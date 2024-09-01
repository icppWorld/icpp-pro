"""Utility functions """

import os
import stat
from typing import Callable


def remove_readonly(func: Callable[[str], None], path: str, _: None) -> None:
    """Make file writeable, then try again to apply func"""
    os.chmod(path, stat.S_IWRITE)  # pylint: disable = no-member
    func(path)
