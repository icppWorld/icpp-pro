"""The wasi-sdk download retries a transient network error.

Regression: a connection that dropped during the download, including midway
through the streamed body, failed `icpp install-wasi-sdk` on the first try.
"""

from pathlib import Path

import pytest
import requests

# The command modules import the CLI app from icpp.__main__, which imports them
# back, so the entry point has to load first, as it does when icpp runs.
import icpp.__main__  # pylint: disable=unused-import
from icpp import commands_install_wasi_sdk


def http_error(status: int) -> requests.HTTPError:
    """An HTTPError as raise_for_status raises it."""
    response = requests.Response()
    response.status_code = status
    return requests.HTTPError(response=response)


def fake_download(
    monkeypatch: pytest.MonkeyPatch, errors: list[Exception]
) -> list[Path]:
    """Makes download_wasi_sdk raise these errors in turn, then succeed."""
    calls: list[Path] = []

    def download(fpath: Path) -> None:
        calls.append(fpath)
        if len(calls) <= len(errors):
            raise errors[len(calls) - 1]

    monkeypatch.setattr(commands_install_wasi_sdk, "download_wasi_sdk", download)
    monkeypatch.setattr(commands_install_wasi_sdk, "RETRY_PAUSE_SECONDS", 0)
    return calls


def test__recovers_from_a_dropped_connection(
    monkeypatch: pytest.MonkeyPatch, tmp_path: Path
) -> None:
    """A mid-download drop and a 503 are retried, then the download succeeds."""
    calls = fake_download(
        monkeypatch, [requests.exceptions.ChunkedEncodingError(), http_error(503)]
    )
    commands_install_wasi_sdk.download_wasi_sdk_with_retries(tmp_path / "sdk.tar.gz")
    assert len(calls) == 3


def test__a_404_is_not_retried(monkeypatch: pytest.MonkeyPatch, tmp_path: Path) -> None:
    """A missing release will not appear on a retry."""
    calls = fake_download(monkeypatch, [http_error(404)])
    with pytest.raises(requests.HTTPError):
        commands_install_wasi_sdk.download_wasi_sdk_with_retries(
            tmp_path / "sdk.tar.gz"
        )
    assert len(calls) == 1


def test__raises_once_the_retries_are_used_up(
    monkeypatch: pytest.MonkeyPatch, tmp_path: Path
) -> None:
    """A connection that keeps failing is tried 1 + DOWNLOAD_RETRIES times."""
    retries = commands_install_wasi_sdk.DOWNLOAD_RETRIES
    calls = fake_download(monkeypatch, [requests.ConnectionError()] * (retries + 1))
    with pytest.raises(requests.ConnectionError):
        commands_install_wasi_sdk.download_wasi_sdk_with_retries(
            tmp_path / "sdk.tar.gz"
        )
    assert len(calls) == retries + 1
