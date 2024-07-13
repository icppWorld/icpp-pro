"""Provides decorators for the icpp commands

When creating a new decorator, add it to `icpp/__init__.py`,
so they become available in the package namespace, and you
can then use them as follows:

```
import icpp

@icpp.requires_wasi_sdk()
@app.command()
def build:
    ...
```

See: https://realpython.com/primer-on-python-decorators/
     https://mypy.readthedocs.io/en/stable/generics.html#declaring-decorators
"""

import sys
import platform
import shutil
from typing import Any, Callable, TypeVar
from functools import wraps
import typer
from icpp import config_default
from icpp.commands_install_wasi_sdk import is_wasi_sdk_installed, install_wasi_sdk
from icpp.commands_install_rust import is_rust_installed, install_rust
from icpp.commands_install_mingw64 import is_mingw64_installed, install_mingw64


F = TypeVar("F", bound=Callable[..., Any])

OS_SYSTEM = platform.system()
OS_PROCESSOR = platform.processor()


def requires_wasi_sdk() -> Callable[[F], F]:
    """Decorates a command that requires the wasi-sdk compiler.

    If the wasi-sdk is not installed, it will do that first.
    """

    def decorator(f: F) -> Any:
        @wraps(f)
        def decorated(*args: Any, **kwargs: Any) -> Any:
            if not is_wasi_sdk_installed():
                typer.echo("The wasi-sdk compiler is not installed. Let's do this first.")
                install_wasi_sdk()

            return f(*args, **kwargs)

        return decorated

    return decorator


def requires_native_compiler() -> Callable[[F], F]:
    """Decorates a command that requires a native compiler.

    If the native compiler is not installed:
    - Windows: installs mingw64
    - Other systems: exits
    """

    def decorator(f: F) -> Any:
        @wraps(f)
        def decorated(*args: Any, **kwargs: Any) -> Any:
            if OS_SYSTEM == "Windows" and not is_mingw64_installed():
                typer.echo("The MinGW-w64 compiler is not installed. Let's do this first.")
                install_mingw64()

            exit_if_native_compiler_not_installed()
            return f(*args, **kwargs)

        return decorated

    return decorator


def exit_if_native_compiler_not_installed() -> None:
    """Exit if `clang` or `clang++` not available"""
    no_c = shutil.which(config_default.NATIVE_C) is None
    no_cpp = shutil.which(config_default.NATIVE_CPP) is None
    if no_c or no_cpp:
        typer.echo("--")
        typer.echo("ERROR: The native compiler is not installed")
        if no_c:
            try:
                typer.echo(f"       (✗) could not find '{config_default.NATIVE_C}'")
            except UnicodeEncodeError:
                typer.echo(f"       (ERROR) could not find '{config_default.NATIVE_C}'")
        else:
            try:
                typer.echo(f"       (✔) found '{config_default.NATIVE_C}'")
            except UnicodeEncodeError:
                typer.echo(f"       (OK) found '{config_default.NATIVE_C}'")
        if no_cpp:
            try:
                typer.echo(f"       (✗) could not find '{config_default.NATIVE_CPP}'")
            except UnicodeEncodeError:
                typer.echo(
                    f"       (ERROR) could not find '{config_default.NATIVE_CPP}'"
                )
        else:
            try:
                typer.echo(f"       (✔) found '{config_default.NATIVE_CPP}'")
            except UnicodeEncodeError:
                typer.echo(f"       (OK) found '{config_default.NATIVE_CPP}'")
        sys.exit(1)


def requires_rust() -> Callable[[F], F]:
    """Decorates a command that requires rust.

    If rust is not installed, it will do that first.
    """

    def decorator(f: F) -> Any:
        @wraps(f)
        def decorated(*args: Any, **kwargs: Any) -> Any:
            # Rust requires the native clang compiler to be installed
            if OS_SYSTEM == "Windows" and not is_mingw64_installed():
                typer.echo("The MinGW-w64 compiler is not installed. Let's do this first.")
                install_mingw64()

            exit_if_native_compiler_not_installed()

            # Now we can install rust
            if not is_rust_installed():
                typer.echo("The rust compiler and dependencies are not installed. Let's do this first.")
                install_rust()

            return f(*args, **kwargs)

        return decorated

    return decorator
