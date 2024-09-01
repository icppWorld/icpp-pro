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
import icpp.version_rust
import icpp.version_mingw64
import icpp.version_ic_wasi_polyfill
import icpp.version_wasi2ic
import icpp.version_ic_wasm

# Version of package
__version__ = icpp.version.__version__

# Version of wasi-sdk we will install in ~/.icpp
__version_wasi_sdk__ = icpp.version_wasi_sdk.__version__

# Version of rust we will install in ~/.icpp
__version_rust__ = icpp.version_rust.__version__

# Version of mingw64 we will install in ~/.icpp (Windows only)
__version_mingw64__ = icpp.version_mingw64.__version__

# Version of ic-wasi-polyfill we will install in ~/.icpp
__version_ic_wasi_polyfill__ = icpp.version_ic_wasi_polyfill.__version__

# Version of wasi2ic we will install in ~/.icpp
__version_wasi2ic__ = icpp.version_wasi2ic.__version__

# Version of ic-wasm we will install in ~/.icpp
__version_ic_wasm__ = icpp.version_ic_wasm.__version__
