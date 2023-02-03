"""Utility to run 'cmd' from python as a subprocess"""

import subprocess
import re
from pathlib import Path
from typing import Optional


def escape_ansi(line: Optional[str]) -> Optional[str]:
    """Removes ansi escape characters that come back from the terminal"""
    if line is None:
        return None

    ansi_regex = (
        r"\x1b("
        r"(\[\??\d+[hl])|"
        r"([=<>a-kzNM78])|"
        r"([\(\)][a-b0-2])|"
        r"(\[\d{0,2}[ma-dgkjqi])|"
        r"(\[\d+;\d+[hfy]?)|"
        r"(\[;?[hf])|"
        r"(#[3-68])|"
        r"([01356]n)|"
        r"(O[mlnp-z]?)|"
        r"(/Z)|"
        r"(\d+)|"
        r"(\[\?\d;\d0c)|"
        r"(\d;\dR))"
    )
    ansi_escape = re.compile(ansi_regex, flags=re.IGNORECASE)
    return ansi_escape.sub("", line)


def run_shell_cmd(
    cmd: str,
    capture_output: bool = False,
    print_captured_output: bool = False,
    cwd: Optional[Path] = None,
) -> str:
    """Runs 'cmd' with following behavior, so we can
    use it in our sequential CI/CD pipeline:

    Example usage:
        import sys
        import subprocess
        cmd = "...some command..."
        try:
            run_shell_cmd(cmd)
        except subprocess.CalledProcessError as e:
            sys.exit(e.returncode)
        except Exception as e:
            typer.echo("ERROR: -----") # optional explanation
            sys.exit(1)

    Parameters:
        capture_output:
            False (default) - Returns ""
            True            - Returns captured stdout+stderr
                              Blocks! Cannot be used to start a background process

        print_captured_output:
            Only used when capture_output=True
            False (default) - Does NOT print output if all runs ok
                              If an error occurs, it will print it
            True            - Prints output while running

    Explanation:
        - Runs `cmd` as a shell command, as if run from the command line
        - Runs from the current folder
          (You can specify an alternative folder with 'cwd=...' argument)
        - Prints the combined stdout & stderr, while it is running

        - If no error occurs:
          - if capture_output=True, returns the captured combination of stdout & stderr
          - else, returns ""
        - If an error occurs:
          - Throws a subprocess.CalledProcessError as e:
            return e.returncode
    """
    if cwd is None:
        cwd = Path(".")

    capture_stdout = ""

    if capture_output is False:
        subprocess.run(cmd, shell=True, check=True, text=True, cwd=cwd)
    else:
        # These will block & cannot be used to start a background process from make.
        if print_captured_output is False:
            # This will capture_output and:
            # - return it if there is no error
            # - includes it in a possibly raised error
            p_1 = subprocess.run(
                cmd,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                shell=True,
                check=False,
                text=True,
                cwd=cwd,
            )
            if p_1.returncode != 0:
                # typer.echo(p_1.stdout)
                raise subprocess.CalledProcessError(
                    returncode=p_1.returncode,
                    cmd=p_1.args,
                    output=escape_ansi(p_1.stdout),
                    stderr=escape_ansi(p_1.stderr),
                )
            capture_stdout += p_1.stdout
        else:
            # This prints output while running, and we capture it as well & return it
            # https://stackoverflow.com/a/28319191/5480536
            with subprocess.Popen(
                cmd,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                shell=True,
                bufsize=1,
                universal_newlines=True,
                cwd=cwd,
            ) as p_2:
                if p_2.stdout is not None:
                    for line in p_2.stdout:
                        print(line, end="")
                        capture_stdout += line
            if p_2.returncode != 0:
                raise subprocess.CalledProcessError(
                    returncode=p_2.returncode,
                    cmd=p_2.args,
                    output=escape_ansi(p_2.stdout),  # type: ignore
                    stderr=escape_ansi(p_2.stderr),  # type: ignore
                )
    return capture_stdout
