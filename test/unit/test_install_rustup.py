"""install_rustup must fail when the rustup installer cannot be downloaded.

Regression: it ran `curl ... | sh`. A failed download handed sh an empty
script, which exits 0, so the step reported success and `icpp install-rust`
failed one step later on a missing rustup, with the real error hidden.
"""

import subprocess
from pathlib import Path

import pytest

# The command modules import the CLI app from icpp.__main__, which imports them
# back, so the entry point has to load first, as it does when icpp runs.
import icpp.__main__  # pylint: disable=unused-import
from icpp import commands_install_rust


def test__fails_when_the_download_fails(
    monkeypatch: pytest.MonkeyPatch, tmp_path: Path
) -> None:
    """Nothing listens on port 9, so curl fails and the step must too."""
    monkeypatch.setattr(commands_install_rust, "LOG_FILE", tmp_path / "install.log")
    monkeypatch.setattr(
        commands_install_rust, "RUSTUP_INIT_URL", "https://127.0.0.1:9/rustup-init"
    )

    with pytest.raises(subprocess.CalledProcessError) as excinfo:
        commands_install_rust.install_rustup(1, 1)
    assert "curl" in str(excinfo.value.cmd)
