"""A setuptools based setup module.

See:
https://packaging.python.org/guides/distributing-packages-using-setuptools/
https://github.com/pypa/sampleproject
"""
import sys
import pathlib
from setuptools import setup, find_packages  # type: ignore

here = pathlib.Path(__file__).parent.resolve()

# Get the version number
# pylint: disable = wrong-import-position, no-member
sys.path.append(str(here / "src/icpp"))
import version  # type: ignore

setup(
    #
    # Versions should comply with PEP 440:
    # https://www.python.org/dev/peps/pep-0440/
    #
    # For a discussion on single-sourcing the version across setup.py and the
    # project code, see
    # https://packaging.python.org/en/latest/single_source_version.html
    version=version.__version__,  # Required
    #
    # When your source code is in a subdirectory under the project root, e.g.
    # `src/`, it is necessary to specify the `package_dir` argument.
    package_dir={"": "src"},  # Optional
    #
    # You can just specify package directories manually here if your project is
    # simple. Or you can use find_packages().
    #
    # Alternatively, if you just want to distribute a single Python file, use
    # the `py_modules` argument instead as follows, which will expect a file
    # called `my_module.py` to exist:
    #
    #   py_modules=["my_module"],
    #
    packages=find_packages(where="src"),  # Required
    #
    # This field lists other packages that your project depends on to run.
    # Any package you put here will be installed by pip when your project is
    # installed, so they must be valid existing projects.
    #
    # For an analysis of "install_requires" vs pip's requirements files see:
    # https://packaging.python.org/en/latest/requirements.html
    install_requires=[
        f"icpp-candid=={version.__version__}"
    ],  # Optional
    #
    # If there are data files included in your packages that need to be
    # installed, specify them here.
    # https://setuptools.pypa.io/en/latest/userguide/datafiles.html#package-data
    #
    # (-) add 'py.typed', to play nice with mypy
    #     https://mypy.readthedocs.io/en/latest/installed_packages.html
    #     #making-pep-561-compatible-packages
    # (-) add 'ic/'    , which contains the C++ & Header files of the API
    # (-) add 'native/', which contains the C++ & Header files for a native build,
    #                    with a mock IC, allowing interactive debugging.
    #
    package_data={  # Optional
        "icpp": [
            "canisters/greet/libhello/*",
            "canisters/greet/libworld/*",
            "canisters/greet/native/*",
            "canisters/greet/src/*",
            "canisters/greet/src/vendors/nlohmann/json/json.hpp",
            "canisters/greet/test/*",
            "canisters/greet/dfx.json",
            "canisters/greet/icpp.toml",
            "canisters/greet/icpp-c++17.toml",
            "canisters/greet/README.md",
            "canisters/greet/demo.sh",
            "canisters/greet/demo.ps1",
            "canisters/greet/demo-c++17.sh",
            "canisters/greet/demo-c++17.ps1",
            "ic/*/*.c",
            "ic/*/*.cpp",
            "ic/*/*.h",
            "ic/*/*.hpp",
            "py.typed",
        ],
    },
    #
    # Although 'package_data' is the preferred approach, in some case you may
    # need to place data files outside of your packages. See:
    # http://docs.python.org/distutils/setupscript.html#installing-additional-files
    #
    # In this case, 'data_file' will be installed into '<sys.prefix>/my_data'
    ##    data_files=[('my_data', ['data/data_file'])],  # Optional
    #
)
