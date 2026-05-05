"""Utility functions """

import os
import stat
from typing import Any, Callable


def remove_readonly(func: Callable[..., Any], path: str, _exc_info: Any) -> None:
    """shutil.rmtree onerror callback: clear the readonly bit and retry.

    Signature matches typeshed's expected onerror callable type, which in
    mypy 1.13+ stubs is
        Callable[[Callable[..., Any], str,
                  tuple[type[BaseException], BaseException, TracebackType]],
                 object]
    The third argument (the sys.exc_info tuple) is unused here; we accept
    Any to stay compatible across the Python 3.10-3.14 matrix without
    importing TracebackType conditionally.
    """
    os.chmod(path, stat.S_IWRITE)  # pylint: disable = no-member
    func(path)
