"""The library cache reuses a library only when every input is identical, never
a stale or corrupt one, and it can never fail a build."""

# pylint: disable=missing-function-docstring, redefined-outer-name

import json
import os
import shutil
import stat
import subprocess
import threading
from pathlib import Path
from typing import Dict, List

import pytest

from icpp import config_default, library_cache

LIB = "__ic_candid__"
LIB_FILE = f"{LIB}.a"
TARGET = "native"


def write_sources(src: Path, header_value: int = 1) -> List[str]:
    src.mkdir(parents=True, exist_ok=True)
    (src / "lib.h").write_text(
        f"constexpr int VALUE = {header_value};\nint a();\nint b();\n"
    )
    (src / "a.cpp").write_text('#include "lib.h"\nint a() { return VALUE; }\n')
    (src / "b.cpp").write_text('#include "lib.h"\nint b() { return VALUE + 1; }\n')
    return [str(src / "a.cpp"), str(src / "b.cpp")]


def commands(src: Path, flags: str = "-O0") -> Dict[str, str]:
    return {
        "cpp": f"clang++ -std=c++17 {flags} -I{src}",
        "c": "clang",
        "ar": "ar qc",
    }


def build(sources: List[str], cmds: Dict[str, str], build_path: Path) -> None:
    """Builds like the library builders do: objects with -MD, then an archive."""
    if build_path.exists():
        shutil.rmtree(build_path)
    build_path.mkdir(parents=True)
    for source in sources:
        name = Path(source).stem
        subprocess.run(
            f"{cmds['cpp']} -c {source} -o {name}.o {library_cache.dep_flags(name)}",
            shell=True,
            check=True,
            cwd=build_path,
        )
    subprocess.run(f"ar qc {LIB_FILE} *.o", shell=True, check=True, cwd=build_path)


@pytest.fixture(autouse=True)
def cache_root(tmp_path: Path, monkeypatch: pytest.MonkeyPatch) -> Path:
    monkeypatch.setattr(config_default, "ICPP_ROOT", tmp_path / "icpp-root")
    monkeypatch.delenv("ICPP_LIBRARY_CACHE", raising=False)
    return tmp_path / "icpp-root" / "cache" / "libraries" / TARGET


def built_and_published(tmp_path: Path) -> tuple[List[str], Dict[str, str], str, Path]:
    src = tmp_path / "src"
    sources = write_sources(src)
    cmds = commands(src)
    build_path = tmp_path / "build"
    build(sources, cmds, build_path)
    key = library_cache.compute_key(TARGET, LIB, cmds, sources)
    library_cache.publish(TARGET, key, LIB_FILE, build_path, cmds)
    return sources, cmds, key, build_path


def test__key_is_stable_and_follows_every_input(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    """Same inputs, same key; a source byte, a flag, the compiler or the
    icpp-pro version changes it."""
    src = tmp_path / "src"
    sources = write_sources(src)
    cmds = commands(src)
    key = library_cache.compute_key(TARGET, LIB, cmds, sources)
    assert key == library_cache.compute_key(TARGET, LIB, cmds, sources)

    keys = {key}
    keys.add(library_cache.compute_key(TARGET, LIB, commands(src, "-O2"), sources))
    other_compiler = dict(cmds, cpp=cmds["cpp"].replace("clang++", "clang"))
    keys.add(library_cache.compute_key(TARGET, LIB, other_compiler, sources))
    keys.add(library_cache.compute_key("wasm32-wasi", LIB, cmds, sources))
    monkeypatch.setattr(library_cache, "__version__", "0.0.0-test")
    keys.add(library_cache.compute_key(TARGET, LIB, cmds, sources))
    monkeypatch.undo()
    source = Path(sources[0])
    source.write_text(source.read_text(encoding="utf-8") + " ", encoding="utf-8")
    keys.add(library_cache.compute_key(TARGET, LIB, cmds, sources))
    assert len(keys) == 6


def test__a_hit_restores_the_same_library(tmp_path: Path) -> None:
    """A published library is restored byte for byte."""
    _, _, key, build_path = built_and_published(tmp_path)
    target = tmp_path / "other-canister"
    target.mkdir()
    assert library_cache.restore(TARGET, key, LIB_FILE, target)
    assert (target / LIB_FILE).read_bytes() == (build_path / LIB_FILE).read_bytes()


def test__the_manifest_records_the_headers_read(
    tmp_path: Path, cache_root: Path
) -> None:
    """The header comes from the -MD dependency files, also system headers."""
    _, _, key, _ = built_and_published(tmp_path)
    deps = json.loads((cache_root / key / "manifest.json").read_text())["deps"]
    assert str((tmp_path / "src" / "lib.h").resolve()) in deps


def test__a_changed_header_is_a_miss(
    tmp_path: Path, capsys: pytest.CaptureFixture[str]
) -> None:
    """A header is not in the key, but a change to it still makes a miss."""
    sources, cmds, key, _ = built_and_published(tmp_path)
    write_sources(tmp_path / "src", header_value=2)
    assert key == library_cache.compute_key(TARGET, LIB, cmds, sources)
    assert not library_cache.restore(TARGET, key, LIB_FILE, tmp_path)
    assert "a dependency changed" in capsys.readouterr().out


def test__a_deleted_dependency_is_a_miss(
    tmp_path: Path, capsys: pytest.CaptureFixture[str]
) -> None:
    _, _, key, _ = built_and_published(tmp_path)
    (tmp_path / "src" / "lib.h").unlink()
    assert not library_cache.restore(TARGET, key, LIB_FILE, tmp_path)
    assert "a dependency is gone" in capsys.readouterr().out


@pytest.mark.parametrize("damage", ["truncate_lib", "garble_manifest", "no_manifest"])
def test__a_corrupt_entry_is_a_miss_and_is_replaced(
    tmp_path: Path, cache_root: Path, damage: str
) -> None:
    """A damaged entry is never used; the next publish replaces it."""
    _, cmds, key, build_path = built_and_published(tmp_path)
    entry = cache_root / key
    if damage == "truncate_lib":
        (entry / LIB_FILE).write_bytes((entry / LIB_FILE).read_bytes()[:100])
    elif damage == "garble_manifest":
        (entry / "manifest.json").write_text("{not json")
    else:
        (entry / "manifest.json").unlink()
    restored = tmp_path / "restored"
    restored.mkdir()
    assert not library_cache.restore(TARGET, key, LIB_FILE, restored)
    library_cache.publish(TARGET, key, LIB_FILE, build_path, cmds)
    assert library_cache.restore(TARGET, key, LIB_FILE, restored)
    assert (restored / LIB_FILE).read_bytes() == (build_path / LIB_FILE).read_bytes()


def test__parallel_publishes_leave_one_valid_entry(
    tmp_path: Path, cache_root: Path
) -> None:
    """Builds that publish the same key at once: one entry, no leftovers."""
    src = tmp_path / "src"
    sources = write_sources(src)
    cmds = commands(src)
    build_path = tmp_path / "build"
    build(sources, cmds, build_path)
    key = library_cache.compute_key(TARGET, LIB, cmds, sources)
    threads = [
        threading.Thread(
            target=library_cache.publish, args=(TARGET, key, LIB_FILE, build_path, cmds)
        )
        for _ in range(8)
    ]
    for t in threads:
        t.start()
    for t in threads:
        t.join()
    assert [p.name for p in cache_root.iterdir()] == [key]
    assert library_cache.restore(TARGET, key, LIB_FILE, tmp_path)


def test__off_switch(
    tmp_path: Path, cache_root: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    """ICPP_LIBRARY_CACHE=0: no lookup, nothing written."""
    for value in ("0", "false", "No", "off"):
        monkeypatch.setenv("ICPP_LIBRARY_CACHE", value)
        src = tmp_path / "src"
        sources = write_sources(src)
        assert library_cache.begin(
            TARGET, LIB, commands(src), sources, LIB_FILE, tmp_path
        ) == (False, None)
    assert not cache_root.exists()
    monkeypatch.setenv("ICPP_LIBRARY_CACHE", "1")
    assert library_cache.enabled()


def test__only_the_icpp_candid_library_is_cached(tmp_path: Path) -> None:
    src = tmp_path / "src"
    sources = write_sources(src)
    assert library_cache.begin(
        TARGET, "my_lib", commands(src), sources, "my_lib.a", tmp_path
    ) == (False, None)


def test__a_cache_that_cannot_be_written_never_fails_a_build(
    tmp_path: Path, cache_root: Path, capsys: pytest.CaptureFixture[str]
) -> None:
    """A file where the target folder should be: a warning, no exception."""
    src = tmp_path / "src"
    sources = write_sources(src)
    cmds = commands(src)
    build_path = tmp_path / "build"
    build(sources, cmds, build_path)
    cache_root.parent.mkdir(parents=True)
    cache_root.write_text("not a folder", encoding="utf-8")
    hit, key = library_cache.begin(TARGET, LIB, cmds, sources, LIB_FILE, build_path)
    assert not hit and key is not None
    library_cache.publish(TARGET, key, LIB_FILE, build_path, cmds)
    assert "WARNING: library cache: cannot store the library" in capsys.readouterr().out
    assert cache_root.read_text(encoding="utf-8") == "not a folder"


def test__the_cache_is_private(tmp_path: Path, cache_root: Path) -> None:
    """Folders 0700, files 0600, whatever the umask."""
    old = os.umask(0o022)
    try:
        _, _, key, _ = built_and_published(tmp_path)
    finally:
        os.umask(old)
    entry = cache_root / key
    for folder in (cache_root.parent.parent, cache_root.parent, cache_root, entry):
        assert stat.S_IMODE(folder.stat().st_mode) == 0o700, folder
    for file in entry.iterdir():
        assert stat.S_IMODE(file.stat().st_mode) == 0o600, file


@pytest.mark.parametrize("where", ["entry", "library", "target_folder"])
def test__a_symlink_is_never_trusted(
    tmp_path: Path, cache_root: Path, where: str, capsys: pytest.CaptureFixture[str]
) -> None:
    """An entry, a library or a cache folder that is a symlink: a miss."""
    _, _, key, _ = built_and_published(tmp_path)
    elsewhere = tmp_path / "elsewhere"
    if where == "entry":
        shutil.move(str(cache_root / key), elsewhere)
        (cache_root / key).symlink_to(elsewhere)
    elif where == "library":
        shutil.move(str(cache_root / key / LIB_FILE), elsewhere)
        (cache_root / key / LIB_FILE).symlink_to(elsewhere)
    else:
        shutil.move(str(cache_root), elsewhere)
        cache_root.symlink_to(elsewhere)
    restored = tmp_path / "restored"
    restored.mkdir()
    assert not library_cache.restore(TARGET, key, LIB_FILE, restored)
    assert "is a symlink" in capsys.readouterr().out
    assert not (restored / LIB_FILE).exists()


@pytest.mark.parametrize("mode", [0o720, 0o702])
def test__an_entry_writable_by_others_is_never_trusted(
    tmp_path: Path, cache_root: Path, mode: int, capsys: pytest.CaptureFixture[str]
) -> None:
    """Group- or world-writable: someone else could have swapped it."""
    _, _, key, _ = built_and_published(tmp_path)
    os.chmod(cache_root / key, mode)
    assert not library_cache.restore(TARGET, key, LIB_FILE, tmp_path)
    assert "is writable by others" in capsys.readouterr().out


def test__a_failed_copy_leaves_no_partial_library(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch, capsys: pytest.CaptureFixture[str]
) -> None:
    """A copy that fails halfway (e.g. a full disk) is removed: the build that
    follows appends to an existing archive (`ar qc`) instead of replacing it."""
    _, _, key, _ = built_and_published(tmp_path)

    def failing_copy(_src: Path, dst: Path) -> None:
        Path(dst).write_bytes(b"half a library")
        raise OSError(28, "No space left on device")

    monkeypatch.setattr(shutil, "copy2", failing_copy)
    restored = tmp_path / "restored"
    restored.mkdir()
    assert not library_cache.restore(TARGET, key, LIB_FILE, restored)
    assert "No space left on device" in capsys.readouterr().out
    assert not (restored / LIB_FILE).exists()


def test__a_library_swapped_during_the_copy_is_never_used(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch, capsys: pytest.CaptureFixture[str]
) -> None:
    """The copy in the canister folder is hashed again; a mismatch: no copy."""
    _, _, key, _ = built_and_published(tmp_path)

    def swapping_copy(_src: Path, dst: Path) -> None:
        Path(dst).write_bytes(b"not the library that was validated")

    monkeypatch.setattr(shutil, "copy2", swapping_copy)
    restored = tmp_path / "restored"
    restored.mkdir()
    assert not library_cache.restore(TARGET, key, LIB_FILE, restored)
    assert "changed while it was copied" in capsys.readouterr().out
    assert not (restored / LIB_FILE).exists()


def test__a_missing_compiler_means_no_cache(
    tmp_path: Path, capsys: pytest.CaptureFixture[str]
) -> None:
    src = tmp_path / "src"
    sources = write_sources(src)
    cmds = dict(commands(src), cpp="no-such-compiler -O0")
    assert library_cache.begin(TARGET, LIB, cmds, sources, LIB_FILE, tmp_path) == (
        False,
        None,
    )
    assert "cannot compute the key" in capsys.readouterr().out


def test__prune_keeps_the_newest_entries(
    cache_root: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    """Beyond MAX_ENTRIES the least recently used go; the new one stays."""
    monkeypatch.setattr(library_cache, "MAX_ENTRIES", 3)
    cache_root.mkdir(parents=True)
    for i in range(6):
        entry = cache_root / f"key{i}"
        entry.mkdir()
        (entry / "manifest.json").write_text("{}")
        os.utime(entry / "manifest.json", (1000 + i, 1000 + i))
    (cache_root / ".tmp-old").mkdir()
    os.utime(cache_root / ".tmp-old", (0, 0))
    library_cache.prune(TARGET, keep="key0")
    assert sorted(p.name for p in cache_root.iterdir()) == ["key0", "key4", "key5"]


def test__prune_respects_the_size_limit(
    cache_root: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    monkeypatch.setattr(library_cache, "MAX_BYTES", 2500)
    cache_root.mkdir(parents=True)
    for i in range(4):
        entry = cache_root / f"key{i}"
        entry.mkdir()
        (entry / "lib.a").write_bytes(b"x" * 1000)
        (entry / "manifest.json").write_text("{}")
        os.utime(entry / "manifest.json", (1000 + i, 1000 + i))
    library_cache.prune(TARGET, keep="key3")
    assert sorted(p.name for p in cache_root.iterdir()) == ["key2", "key3"]


def test__parse_dep_file() -> None:
    """Make syntax: line continuations, escaped spaces and $$, relative paths."""
    text = "a.o: /x/a.cpp /x/my\\ dir/h.h \\\n  rel/b.h /x/c$$.h\n"
    assert library_cache.parse_dep_file(text, Path("/base")) == [
        Path("/x/a.cpp"),
        Path("/x/my dir/h.h"),
        Path("/base/rel/b.h").resolve(),
        Path("/x/c$.h"),
    ]
