"""Utility functions """

import os
import shutil
import stat
import sys
from pathlib import Path
from typing import Any, Callable, Union


def remove_readonly(func: Callable[..., Any], path: str, _exc_info: Any) -> None:
    """shutil.rmtree onerror / onexc callback: clear the readonly bit and retry.

    The third argument is unused here, so the same callable is valid for
    both signatures:
      - 3.10/3.11 onerror : (func, path, sys.exc_info() tuple)
      - 3.12+    onexc   : (func, path, exception_instance)
    Typed as Any so we don't have to import TracebackType conditionally.
    """
    os.chmod(path, stat.S_IWRITE)  # pylint: disable = no-member
    func(path)


def rmtree_force(path: Union[str, Path]) -> None:
    """Like shutil.rmtree, but clears readonly bits on the way down and
    silences the Python 3.12 deprecation of `onerror`.

    `shutil.rmtree`'s `onerror` parameter was deprecated in 3.12 in favor
    of `onexc` (with a different 3rd-arg signature). pylint 3.x running
    against Python 3.13's stdlib stubs flags `onerror=` calls as
    `W4903 deprecated-argument`. To keep CI clean across the
    Python 3.10–3.14 matrix we branch by version here. `remove_readonly`
    above ignores the 3rd argument so the same callback works for both.
    """
    # On each Python version one branch below is "right" and the other is
    # statically unreachable. Pylint nonetheless analyzes both with the
    # local stdlib stubs and would flag whichever branch doesn't match —
    # `unexpected-keyword-arg` for `onexc` on 3.10/3.11, and
    # `deprecated-argument` for `onerror` on 3.12+. Both `disable-next`
    # comments are needed; the inactive one is silenced by
    # `useless-suppression` in .pylintrc. Note: `disable-next=` is used
    # rather than a free-standing `disable=` because the latter's
    # line-scope rules vary across pylint versions and let the warning
    # leak through in CI on Python 3.12+.
    if sys.version_info >= (3, 12):
        # pylint: disable-next=unexpected-keyword-arg
        shutil.rmtree(path, onexc=remove_readonly)
    else:
        # pylint: disable-next=deprecated-argument
        shutil.rmtree(path, onerror=remove_readonly)
