"""
__init__.py.
The double underscores in the filename tell us that this is a special file.
__init__.py represents the root of your package.
It should usually be kept quite simple, but it’s a good place to put package constants,
documentation, and so on.

The special variable __version__ is a convention in Python for adding version numbers
to your package.

Variables defined in __init__.py become available as variables in the package namespace:
```
import icpp
icpp.__version__
```

reference: https://realpython.com/pypi-publish-python-package/
"""
import icpp.version
import icpp.version_wasi_sdk

# Version of package
__version__ = icpp.version.__version__

# Version of wasi-sdk
__version_wasi_sdk__ = icpp.version_wasi_sdk.__version__
