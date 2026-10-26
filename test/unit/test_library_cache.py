"""The library cache reuses a library only when everything that goes into it is
identical, never a stale or corrupt one, and it can never fail a build."""

# pylint: disable=missing-function-docstring, redefined-outer-name, protected-access

import json
import os
import shutil
import stat
import subprocess
import threading
from pathlib import Path
from typing import Dict, List, Tuple

import pytest

from icpp import config_default, library_cache

LIB = "__ic_candid__"
LIB_FILE = f"{LIB}.a"
TARGET = "native"


def write_sources(src: Path, header_value: int = 1) -> List[str]:
    src.mkdir(parents=True, exist_ok=True)
    (src / "lib.h").write_text(
        f"constexpr int VALUE = {header_value};\nint a();\nint b();\n",
        encoding="utf-8",
    )
    (src / "a.cpp").write_text(
        '#include "lib.h"\nint a() { return VALUE; }\n', encoding="utf-8"
    )
    (src / "b.cpp").write_text(
        "#include <lib.h>\n"
        "#if __has_include(<sub/extra.h>)\n#include <sub/extra.h>\n#endif\n"
        "int b() { return VALUE + 1; }\n",
        encoding="utf-8",
    )
    return [str(src / "a.cpp"), str(src / "b.cpp")]


def commands(src: Path, flags: str = "-O0", include: str = "") -> Dict[str, str]:
    return {
        "cpp": f"clang++ -std=c++17 {flags} {include} -I{src}",
        "c": "clang",
        "ar": "ar qc",
    }


def build(sources: List[str], cmds: Dict[str, str], build_path: Path) -> None:
    """Builds like the library builders do: objects, then an archive."""
    if build_path.exists():
        shutil.rmtree(build_path)
    build_path.mkdir(parents=True)
    for source in sources:
        subprocess.run(
            f"{cmds['cpp']} -c {source} -o {Path(source).stem}.o",
            shell=True,
            check=True,
            cwd=build_path,
        )
    subprocess.run(f"ar qc {LIB_FILE} *.o", shell=True, check=True, cwd=build_path)


def key(cmds: Dict[str, str], sources: List[str], cwd: Path) -> str:
    return library_cache.compute_key(TARGET, LIB, cmds, sources, cwd)


@pytest.fixture(autouse=True)
def cache_root(tmp_path: Path, monkeypatch: pytest.MonkeyPatch) -> Path:
    monkeypatch.setattr(config_default, "ICPP_ROOT", tmp_path / "icpp-root")
    monkeypatch.delenv("ICPP_LIBRARY_CACHE", raising=False)
    for name in ("CPATH", "CPLUS_INCLUDE_PATH"):
        monkeypatch.delenv(name, raising=False)
    return tmp_path / "icpp-root" / "cache" / "libraries" / TARGET


def built_and_published(
    tmp_path: Path,
) -> Tuple[List[str], Dict[str, str], str, Path]:
    src = tmp_path / "src"
    sources = write_sources(src)
    cmds = commands(src)
    build_path = tmp_path / "build"
    build(sources, cmds, build_path)
    k = key(cmds, sources, build_path)
    library_cache.publish(TARGET, k, LIB_FILE, build_path, cmds)
    return sources, cmds, k, build_path


# ---------------------------------------------------------------------------
# The key


def test__key_is_stable_and_follows_every_input(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    """Same inputs, same key; a source byte, a flag, the compiler command, the
    compiler identity (same command), the driver resolution, the target or the
    icpp-pro version changes it."""
    src = tmp_path / "src"
    sources = write_sources(src)
    cmds = commands(src)
    k = key(cmds, sources, tmp_path)
    assert k == key(cmds, sources, tmp_path)

    keys = {k, key(commands(src, "-O2"), sources, tmp_path)}
    other = dict(cmds, cpp=cmds["cpp"].replace("clang++", "clang"))
    keys.add(key(other, sources, tmp_path))
    keys.add(library_cache.compute_key("wasm32-wasi", LIB, cmds, sources, tmp_path))
    with monkeypatch.context() as m:
        real = library_cache._compiler_identity
        m.setattr(
            library_cache,
            "_compiler_identity",
            lambda command: dict(real(command), mtime_ns=1),
        )
        keys.add(key(cmds, sources, tmp_path))
    with monkeypatch.context() as m:
        real_driver = library_cache._driver
        m.setattr(
            library_cache,
            "_driver",
            lambda command, language, cwd: real_driver(command, language, cwd)
            + " --sysroot=/another/sdk",
        )
        keys.add(key(cmds, sources, tmp_path))
    with monkeypatch.context() as m:
        m.setattr(library_cache, "__version__", "0.0.0-test")
        keys.add(key(cmds, sources, tmp_path))
    source = Path(sources[0])
    source.write_text(source.read_text("utf-8") + " int c;", encoding="utf-8")
    keys.add(key(cmds, sources, tmp_path))
    assert len(keys) == 8


def test__a_changed_header_changes_the_key(tmp_path: Path) -> None:
    src = tmp_path / "src"
    sources = write_sources(src)
    cmds = commands(src)
    k = key(cmds, sources, tmp_path)
    write_sources(src, header_value=2)
    assert k != key(cmds, sources, tmp_path)


def test__a_header_that_would_now_be_found_first_changes_the_key(
    tmp_path: Path,
) -> None:
    """A new lib.h in an -I folder before src shadows src/lib.h."""
    first, src = tmp_path / "first", tmp_path / "src"
    first.mkdir()
    sources = write_sources(src)
    cmds = commands(src, include=f"-I{first}")
    k = key(cmds, sources, tmp_path)
    (first / "lib.h").write_text(
        "constexpr int VALUE = 7;\nint a();\nint b();\n", encoding="utf-8"
    )
    assert k != key(cmds, sources, tmp_path)


def test__a_new_header_that_satisfies_a_has_include_changes_the_key(
    tmp_path: Path,
) -> None:
    """b.cpp includes <sub/extra.h> only when it exists: adding it later, in a
    subfolder that held nothing the build used, changes what is compiled."""
    src = tmp_path / "src"
    sources = write_sources(src)
    cmds = commands(src)
    (src / "sub").mkdir()
    k = key(cmds, sources, tmp_path)
    (src / "sub" / "extra.h").write_text("int extra();\n", encoding="utf-8")
    assert k != key(cmds, sources, tmp_path)


def test__a_symlinked_header_that_now_points_elsewhere_changes_the_key(
    tmp_path: Path,
) -> None:
    src = tmp_path / "src"
    sources = write_sources(src)
    v1, v2 = tmp_path / "v1", tmp_path / "v2"
    v1.mkdir()
    v2.mkdir()
    (v1 / "lib.h").write_text((src / "lib.h").read_text("utf-8"), encoding="utf-8")
    (v2 / "lib.h").write_text(
        "constexpr int VALUE = 2;\nint a();\nint b();\n", encoding="utf-8"
    )
    (src / "lib.h").unlink()
    (src / "lib.h").symlink_to(v1 / "lib.h")
    cmds = commands(src)
    k = key(cmds, sources, tmp_path)
    (src / "lib.h").unlink()
    (src / "lib.h").symlink_to(v2 / "lib.h")
    assert k != key(cmds, sources, tmp_path)


def test__a_relative_include_path_from_the_environment_is_per_canister(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    """CPATH=../inc points to another folder for each canister: the headers
    found there are in the preprocessed output, so the keys differ."""
    src = tmp_path / "src"
    sources = write_sources(src)
    (src / "lib.h").unlink()
    cmds = commands(src)
    for canister, value in (("a", 1), ("b", 2)):
        (tmp_path / canister / "inc").mkdir(parents=True)
        (tmp_path / canister / "build").mkdir()
        (tmp_path / canister / "inc" / "lib.h").write_text(
            f"constexpr int VALUE = {value};\nint a();\nint b();\n", encoding="utf-8"
        )
    monkeypatch.setenv("CPATH", "../inc")
    assert key(cmds, sources, tmp_path / "a" / "build") != key(
        cmds, sources, tmp_path / "b" / "build"
    )


def test__own_flags_put_the_build_folder_in_the_key(tmp_path: Path) -> None:
    """A canister's own flags may name files that are not in the preprocessed
    text, by a relative path: its build folder is in the key. Without own flags,
    canisters share the key."""
    src = tmp_path / "src"
    sources = write_sources(src)
    a, b = tmp_path / "a" / "build", tmp_path / "b" / "build"
    a.mkdir(parents=True)
    b.mkdir(parents=True)
    cmds = commands(src)

    def own(cwd: Path) -> str:
        return library_cache.compute_key(
            TARGET, LIB, cmds, sources, cwd, own_flags=True
        )

    assert own(a) != own(b)
    assert key(cmds, sources, a) == key(cmds, sources, b)


def test__a_source_that_does_not_preprocess_means_no_cache(
    tmp_path: Path, capsys: pytest.CaptureFixture[str]
) -> None:
    src = tmp_path / "src"
    sources = write_sources(src)
    (src / "lib.h").unlink()
    build_ = library_cache.start(
        TARGET, LIB, commands(src), sources, LIB_FILE, tmp_path
    )
    assert (build_.hit, build_.key) == (False, None)
    assert "cannot compute the key: cannot preprocess" in capsys.readouterr().out


def test__a_missing_compiler_means_no_cache(
    tmp_path: Path, capsys: pytest.CaptureFixture[str]
) -> None:
    src = tmp_path / "src"
    sources = write_sources(src)
    cmds = dict(commands(src), cpp="no-such-compiler -O0")
    build_ = library_cache.start(TARGET, LIB, cmds, sources, LIB_FILE, tmp_path)
    assert (build_.hit, build_.key) == (False, None)
    assert "cannot compute the key" in capsys.readouterr().out


# ---------------------------------------------------------------------------
# Hits, misses and publishing


def test__a_hit_restores_the_same_library(tmp_path: Path) -> None:
    _, _, k, build_path = built_and_published(tmp_path)
    target = tmp_path / "other-canister"
    target.mkdir()
    assert library_cache.restore(TARGET, k, LIB_FILE, target)
    assert (target / LIB_FILE).read_bytes() == (build_path / LIB_FILE).read_bytes()


def test__start_then_publish_then_start_is_a_hit(tmp_path: Path) -> None:
    src = tmp_path / "src"
    sources = write_sources(src)
    cmds = commands(src)
    build_path = tmp_path / "build"
    build_path.mkdir()
    first = library_cache.start(TARGET, LIB, cmds, sources, LIB_FILE, build_path)
    assert not first.hit and first.key is not None
    build(sources, cmds, build_path)
    first.publish()
    second = library_cache.start(TARGET, LIB, cmds, sources, LIB_FILE, build_path)
    assert second.hit and second.key == first.key


def test__an_input_changed_during_the_build_is_not_stored(
    tmp_path: Path, cache_root: Path, capsys: pytest.CaptureFixture[str]
) -> None:
    """The key is computed again before storing: a header edited while the
    library was being built makes it differ, and nothing is stored."""
    src = tmp_path / "src"
    sources = write_sources(src)
    cmds = commands(src)
    build_path = tmp_path / "build"
    build_path.mkdir()
    build_ = library_cache.start(TARGET, LIB, cmds, sources, LIB_FILE, build_path)
    build(sources, cmds, build_path)
    write_sources(src, header_value=9)  # "during the build"
    build_.publish()
    assert "an input changed during the build" in capsys.readouterr().out
    assert not cache_root.exists() or not list(cache_root.iterdir())


@pytest.mark.parametrize("damage", ["truncate_lib", "garble_manifest", "no_manifest"])
def test__a_corrupt_entry_is_a_miss_and_is_replaced(
    tmp_path: Path, cache_root: Path, damage: str
) -> None:
    _, cmds, k, build_path = built_and_published(tmp_path)
    entry = cache_root / k
    if damage == "truncate_lib":
        (entry / LIB_FILE).write_bytes((entry / LIB_FILE).read_bytes()[:100])
    elif damage == "garble_manifest":
        (entry / "manifest.json").write_text("{not json", encoding="utf-8")
    else:
        (entry / "manifest.json").unlink()
    restored = tmp_path / "restored"
    restored.mkdir()
    assert not library_cache.restore(TARGET, k, LIB_FILE, restored)
    assert not (restored / LIB_FILE).exists()
    library_cache.publish(TARGET, k, LIB_FILE, build_path, cmds)
    assert library_cache.restore(TARGET, k, LIB_FILE, restored)
    assert (restored / LIB_FILE).read_bytes() == (build_path / LIB_FILE).read_bytes()


def test__parallel_publishes_leave_one_valid_entry(
    tmp_path: Path, cache_root: Path
) -> None:
    src = tmp_path / "src"
    sources = write_sources(src)
    cmds = commands(src)
    build_path = tmp_path / "build"
    build(sources, cmds, build_path)
    k = key(cmds, sources, build_path)
    threads = [
        threading.Thread(
            target=library_cache.publish, args=(TARGET, k, LIB_FILE, build_path, cmds)
        )
        for _ in range(8)
    ]
    for t in threads:
        t.start()
    for t in threads:
        t.join()
    assert [p.name for p in cache_root.iterdir()] == [k]
    assert library_cache.restore(TARGET, k, LIB_FILE, tmp_path)


def test__off_switch(
    tmp_path: Path, cache_root: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    src = tmp_path / "src"
    sources = write_sources(src)
    for value in ("0", "false", "No", "off"):
        monkeypatch.setenv("ICPP_LIBRARY_CACHE", value)
        build_ = library_cache.start(
            TARGET, LIB, commands(src), sources, LIB_FILE, tmp_path
        )
        assert (build_.hit, build_.key) == (False, None)
        build_.publish()
    assert not cache_root.exists()
    monkeypatch.setenv("ICPP_LIBRARY_CACHE", "1")
    assert library_cache.enabled()


def test__only_the_icpp_candid_library_is_cached(tmp_path: Path) -> None:
    src = tmp_path / "src"
    sources = write_sources(src)
    build_ = library_cache.start(
        TARGET, "my_lib", commands(src), sources, "my_lib.a", tmp_path
    )
    assert (build_.hit, build_.key) == (False, None)


# ---------------------------------------------------------------------------
# Trust


def test__the_cache_is_private(tmp_path: Path, cache_root: Path) -> None:
    """Folders 0700, files 0600, whatever the umask."""
    old = os.umask(0o022)
    try:
        _, _, k, _ = built_and_published(tmp_path)
    finally:
        os.umask(old)
    entry = cache_root / k
    for folder in (cache_root.parent.parent, cache_root.parent, cache_root, entry):
        assert stat.S_IMODE(folder.stat().st_mode) == 0o700, folder
    for file in entry.iterdir():
        assert stat.S_IMODE(file.stat().st_mode) == 0o600, file


@pytest.mark.parametrize("where", ["entry", "library", "target_folder"])
def test__a_symlink_is_never_trusted(
    tmp_path: Path, cache_root: Path, where: str, capsys: pytest.CaptureFixture[str]
) -> None:
    _, _, k, _ = built_and_published(tmp_path)
    elsewhere = tmp_path / "elsewhere"
    if where == "entry":
        shutil.move(str(cache_root / k), elsewhere)
        (cache_root / k).symlink_to(elsewhere)
    elif where == "library":
        shutil.move(str(cache_root / k / LIB_FILE), elsewhere)
        (cache_root / k / LIB_FILE).symlink_to(elsewhere)
    else:
        shutil.move(str(cache_root), elsewhere)
        cache_root.symlink_to(elsewhere)
    restored = tmp_path / "restored"
    restored.mkdir()
    assert not library_cache.restore(TARGET, k, LIB_FILE, restored)
    assert "is a symlink" in capsys.readouterr().out
    assert not (restored / LIB_FILE).exists()


@pytest.mark.parametrize("mode", [0o720, 0o702])
def test__an_entry_writable_by_others_is_never_trusted(
    tmp_path: Path, cache_root: Path, mode: int, capsys: pytest.CaptureFixture[str]
) -> None:
    _, _, k, _ = built_and_published(tmp_path)
    os.chmod(cache_root / k, mode)
    assert not library_cache.restore(TARGET, k, LIB_FILE, tmp_path)
    assert "is writable by others" in capsys.readouterr().out


def test__a_restored_cache_with_open_parent_folders_still_hits(
    tmp_path: Path, cache_root: Path
) -> None:
    """actions/cache (tar) recreates the folders above the target with the umask,
    e.g. 0775: our own folders are made private again, and the entry is used."""
    _, _, k, _ = built_and_published(tmp_path)
    levels = [cache_root.parent.parent, cache_root.parent, cache_root]
    for level in levels:
        os.chmod(level, 0o775)
    assert library_cache.restore(TARGET, k, LIB_FILE, tmp_path)
    for level in levels:
        assert stat.S_IMODE(level.stat().st_mode) == 0o700, level


def test__a_library_swapped_after_the_validation_is_never_used(
    tmp_path: Path, cache_root: Path, capsys: pytest.CaptureFixture[str]
) -> None:
    """The library is hashed while it is copied: a file that no longer matches
    its manifest is removed from the canister folder."""
    _, _, k, _ = built_and_published(tmp_path)
    (cache_root / k / LIB_FILE).write_bytes(b"not the library that was published")
    restored = tmp_path / "restored"
    restored.mkdir()
    assert not library_cache.restore(TARGET, k, LIB_FILE, restored)
    assert "does not match its sha256" in capsys.readouterr().out
    assert not (restored / LIB_FILE).exists()


def test__a_symlinked_entry_is_replaced_and_removed(
    tmp_path: Path, cache_root: Path
) -> None:
    """Publishing over a symlinked entry removes the symlink, not its target."""
    _, cmds, k, build_path = built_and_published(tmp_path)
    elsewhere = tmp_path / "elsewhere"
    shutil.move(str(cache_root / k), elsewhere)
    (cache_root / k).symlink_to(elsewhere)
    library_cache.publish(TARGET, k, LIB_FILE, build_path, cmds)
    assert not (cache_root / k).is_symlink()
    assert sorted(p.name for p in cache_root.iterdir()) == [k]
    assert elsewhere.exists()


# ---------------------------------------------------------------------------
# Never fails a build


def test__a_failed_copy_leaves_no_partial_library(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch, capsys: pytest.CaptureFixture[str]
) -> None:
    """A copy that fails halfway (e.g. a full disk) is removed: the build that
    follows appends to an existing archive (`ar qc`) instead of replacing it."""
    _, _, k, _ = built_and_published(tmp_path)

    def failing_copy(_src: Path, dst: Path) -> str:
        Path(dst).write_bytes(b"half a library")
        raise OSError(28, "No space left on device")

    monkeypatch.setattr(library_cache, "_copy_hashed", failing_copy)
    restored = tmp_path / "restored"
    restored.mkdir()
    assert not library_cache.restore(TARGET, k, LIB_FILE, restored)
    assert "No space left on device" in capsys.readouterr().out
    assert not (restored / LIB_FILE).exists()


def test__a_failing_cleanup_never_escapes_restore(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    _, _, k, _ = built_and_published(tmp_path)

    def failing_copy(_src: Path, _dst: Path) -> str:
        raise OSError(5, "Input/output error")

    def failing_unlink(_self: Path, missing_ok: bool = False) -> None:
        raise PermissionError(13, "Permission denied")

    monkeypatch.setattr(library_cache, "_copy_hashed", failing_copy)
    monkeypatch.setattr(Path, "unlink", failing_unlink)
    assert not library_cache.restore(TARGET, k, LIB_FILE, tmp_path)


def test__start_never_raises(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch, capsys: pytest.CaptureFixture[str]
) -> None:
    src = tmp_path / "src"
    sources = write_sources(src)

    def broken_restore(*_args: object) -> bool:
        raise RuntimeError("unexpected")

    monkeypatch.setattr(library_cache, "restore", broken_restore)
    build_ = library_cache.start(
        TARGET, LIB, commands(src), sources, LIB_FILE, tmp_path
    )
    assert (build_.hit, build_.key) == (False, None)
    assert "cannot use the cache: unexpected" in capsys.readouterr().out


def test__a_cache_that_cannot_be_written_never_fails_a_build(
    tmp_path: Path, cache_root: Path, capsys: pytest.CaptureFixture[str]
) -> None:
    """A file where the target folder should be: a warning, no exception."""
    _, cmds, k, build_path = built_and_published(tmp_path)
    shutil.rmtree(cache_root)
    cache_root.write_text("not a folder", encoding="utf-8")
    library_cache.publish(TARGET, k, LIB_FILE, build_path, cmds)
    assert "WARNING: library cache: cannot store the library" in capsys.readouterr().out
    assert cache_root.read_text(encoding="utf-8") == "not a folder"


# ---------------------------------------------------------------------------
# Pruning


def make_entries(cache_root: Path, sizes: List[int]) -> None:
    """key0 is the oldest, the last one the newest."""
    cache_root.mkdir(parents=True)
    for i, size in enumerate(sizes):
        entry = cache_root / f"key{i}"
        entry.mkdir()
        (entry / "lib.a").write_bytes(b"x" * size)
        (entry / "manifest.json").write_text("{}", encoding="utf-8")
        os.utime(entry / "manifest.json", (1000 + i, 1000 + i))


def test__prune_keeps_the_newest_entries(
    cache_root: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    """Beyond MAX_ENTRIES the least recently used go; `keep` always stays."""
    monkeypatch.setattr(library_cache, "MAX_ENTRIES", 3)
    make_entries(cache_root, [10] * 6)
    (cache_root / ".tmp-old").mkdir()
    os.utime(cache_root / ".tmp-old", (0, 0))
    library_cache.prune(TARGET, keep="key0")
    assert sorted(p.name for p in cache_root.iterdir()) == ["key0", "key4", "key5"]


def test__prune_counts_only_the_entries_it_keeps(
    cache_root: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    """One large entry over the limit is removed; smaller older ones that still
    fit stay."""
    monkeypatch.setattr(library_cache, "MAX_BYTES", 2000)
    make_entries(cache_root, [300, 1500, 400, 500])
    library_cache.prune(TARGET, keep="key3")
    assert sorted(p.name for p in cache_root.iterdir()) == ["key0", "key2", "key3"]


def test__a_stray_file_is_removed(cache_root: Path) -> None:
    cache_root.mkdir(parents=True)
    stray = cache_root / ".stale-file"
    stray.write_text("x", encoding="utf-8")
    os.utime(stray, (0, 0))
    library_cache.prune(TARGET, keep="none")
    assert not stray.exists()


def test__the_manifest_is_small(tmp_path: Path, cache_root: Path) -> None:
    """No dependency lists: the preprocessed output is in the key."""
    _, _, k, _ = built_and_published(tmp_path)
    manifest = json.loads((cache_root / k / "manifest.json").read_text("utf-8"))
    assert set(manifest) == {
        "schema",
        "key",
        "lib_sha256",
        "commands",
        "icpp_pro",
        "icpp_candid",
        "created",
    }
