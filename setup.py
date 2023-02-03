"""A setuptools based setup module.

See:
https://packaging.python.org/guides/distributing-packages-using-setuptools/
https://github.com/pypa/sampleproject
"""
import sys
import pathlib
from setuptools import setup, find_packages  # type: ignore

here = pathlib.Path(__file__).parent.resolve()

# Get the long description from the README file
long_description = (here / "README.md").read_text(encoding="utf-8")

# Get the version number
# pylint: disable = wrong-import-position, no-member
sys.path.append(str(here / "src/icpp"))
import version  # type: ignore

setup(
    # This is the name of your project. The first time you publish this
    # package, this name will be registered for you. It will determine how
    # users can install this project, e.g.:
    #
    # $ pip install icpp
    #
    # And where it will live on PyPI: https://pypi.org/project/icpp/
    #
    # There are some restrictions on what makes a valid project name
    # specification here:
    # https://packaging.python.org/specifications/core-metadata/#name
    name="icpp",  # Required
    #
    # Versions should comply with PEP 440:
    # https://www.python.org/dev/peps/pep-0440/
    #
    # For a discussion on single-sourcing the version across setup.py and the
    # project code, see
    # https://packaging.python.org/en/latest/single_source_version.html
    version=version.__version__,  # Required
    #
    # This is a one-line description or tagline of what your project does. This
    # corresponds to the "Summary" metadata field:
    # https://packaging.python.org/specifications/core-metadata/#summary
    description="Modern C++ for the Internet Computer.",  # Optional
    #
    # This is an optional longer description of your project that represents
    # the body of text which users will see when they visit PyPI.
    #
    # Often, this is the same as your README, so you can just read it in from
    # that file directly (as we have already done above)
    #
    # This field corresponds to the "Description" metadata field:
    # https://packaging.python.org/specifications/core-metadata/#description-optional
    long_description=long_description,  # Optional
    #
    # Denotes that our long_description is in Markdown; valid values are
    # text/plain, text/x-rst, and text/markdown
    #
    # Optional if long_description is written in reStructuredText (rst) but
    # required for plain-text or Markdown; if unspecified, "applications should
    # attempt to render [the long_description] as text/x-rst; charset=UTF-8 and
    # fall back to text/plain if it is not valid rst" (see link below)
    #
    # This field corresponds to the "Description-Content-Type" metadata field:
    # https://packaging.python.org/specifications/core-metadata/#description-content-type-optional
    long_description_content_type="text/markdown",  # Optional (see note above)
    #
    # This should be a valid link to your project's main homepage.
    #
    # This field corresponds to the "Home-Page" metadata field:
    # https://packaging.python.org/specifications/core-metadata/#home-page-optional
    url="https://github.com/icppWorld/icpp",  # Optional
    #
    # This should be your name or the name of the organization which owns the project.
    author="icpp team",  # Optional
    #
    # This should be a valid email address corresponding to the author listed above.
    author_email="icpp@icpp.world",  # Optional
    #
    # Classifiers help users find your project by categorizing it.
    #
    # For a list of valid classifiers, see https://pypi.org/classifiers/
    classifiers=[  # Optional
        # How mature is this project? Common values are
        #   1 - Planning
        #   2 - Pre-Alpha
        #   3 - Alpha
        #   4 - Beta
        #   5 - Production/Stable
        "Development Status :: 3 - Alpha",
        # Pick your license as you wish
        "License :: OSI Approved :: GNU General Public License v3 or later (GPLv3+)",
        # Specify the Python versions you support here. In particular, ensure
        # that you indicate you support Python 3. These classifiers are *not*
        # checked by 'pip install'. See instead 'python_requires' below.
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.8",
        "Programming Language :: Python :: 3.9",
        "Programming Language :: Python :: 3.10",
        "Programming Language :: Python :: 3.11",
        "Programming Language :: Python :: 3 :: Only",
    ],
    #
    # This field adds keywords for your project which will appear on the
    # project page. What does your project relate to?
    #
    # Note that this is a list of additional keywords, separated
    # by commas, to be used to assist searching for the distribution in a
    # larger catalog.
    keywords="Internet Computer, C++",  # Optional
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
    # Specify which Python versions you support. In contrast to the
    # 'Programming Language' classifiers above, 'pip install' will check this
    # and refuse to install the project if the version does not match. See
    # https://packaging.python.org/guides/distributing-packages-using-setuptools/#python-requires
    python_requires=">=3.8",
    #
    # This field lists other packages that your project depends on to run.
    # Any package you put here will be installed by pip when your project is
    # installed, so they must be valid existing projects.
    #
    # For an analysis of "install_requires" vs pip's requirements files see:
    # https://packaging.python.org/en/latest/requirements.html
    install_requires=[
        "typer[all]",
        "tomli",
        "requests",
        "enlighten",
        "pytest",
    ],  # Optional
    #
    # List additional groups of dependencies here (e.g. development
    # dependencies). Users will be able to install these using the "extras"
    # syntax, for example:
    #
    #   $ pip install "sampleproject[dev]"
    #
    # Similar to `install_requires` above, these must be valid existing
    # projects.
    extras_require={  # Optional
        "dev": [
            "black",
            "pylint==2.13.9",
            "mypy",
            "build",
            "twine",
            "mkdocs",
            "types-requests",
        ],
    },
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
            "canisters/greet/native/*",
            "canisters/greet/src/*",
            "canisters/greet/test/*",
            "canisters/greet/dfx.json",
            "canisters/greet/icpp.toml",
            "canisters/greet/README.md",
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
    # To provide executable scripts, use entry points in preference to the
    # "scripts" keyword. Entry points provide cross-platform support and allow
    # `pip` to create the appropriate form of executable for the target
    # platform.
    #
    # For example, the following would provide a command called `icpp` which
    # executes the function `main` from this package when invoked:
    # NOTE: you can create multiple entry_points, for example, when distributing
    #       more than one packages, you can create an entry_point for each package
    entry_points={  # Optional
        "console_scripts": [
            "icpp=icpp.__main__:main",
        ],
    },
    #
    # List additional URLs that are relevant to your project as a dict.
    #
    # This field corresponds to the "Project-URL" metadata fields:
    # https://packaging.python.org/specifications/core-metadata/#project-url-multiple-use
    #
    # Examples listed include a pattern for specifying where the package tracks
    # issues, where the source is hosted, where to say thanks to the package
    # maintainers, and where to support the project financially. The key is
    # what's used to render the link text on PyPI.
    ##    project_urls={  # Optional
    ##        'Bug Reports': 'https://github.com/pypa/sampleproject/issues',
    ##        'Funding': 'https://donate.pypi.org',
    ##        'Say Thanks!': 'http://saythanks.io/to/example',
    ##        'Source': 'https://github.com/pypa/sampleproject/',
    ##    },
    #
)
