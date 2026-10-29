"""Calls a function referenced by a string"""

import importlib
import os
import sys


def call_function(full_function_name: str) -> None:
    """Calls a function from name

    full_function_name must use the regular python packaging convention

    Example:

    For a python file on disk with:

    |- scripts
    |  |- __init__.py        # 'scripts' is a package
    |  |- my_module.py       # a module in the package
    |  |  def my_function(): # that contains a function

    full_function_name = scripts.my_module.my_function
                         <package>.<module>.<function>

    """

    if full_function_name is not None and full_function_name.strip() != "":
        module_name, function_name = full_function_name.rsplit(".", 1)
        # The project folder is on the path only for this import, so a file in
        # the project can't shadow a module that icpp itself imports
        project_folder = os.getcwd()
        sys.path.insert(1, project_folder)
        try:
            module = importlib.import_module(module_name)
        finally:
            sys.path.remove(project_folder)
        function = getattr(module, function_name)
        function()
