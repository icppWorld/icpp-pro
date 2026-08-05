"""Functions to be used in a pytest test

These helpers drive `icp` (icp-cli), the successor of the deprecated `dfx`.

The `network` argument of every helper is the name of an *environment* in the
project's `icp.yaml`, and is passed to icp as `-e <network>`. Define an
environment per network you test against, e.g.:

    environments:
      - name: local
        network: local
      - name: ic
        network: ic

so that `pytest --network=local` and `pytest --network=ic` keep working.

The identity every helper calls as is named explicitly, with
`pytest --identity <name>` or by exporting ICPP_PRO_TEST_IDENTITY, and is
passed to icp as `--identity <name>`. Pass `identity=` to override it for a
single call. The machine-wide active identity (`icp identity default`) is never
read and never written: it is shared with every other process on the machine,
so using it would clobber the identity you use for mainnet work and leave a run
exposed to anything that changes it mid-flight.
"""

import json
import re
import shlex
import subprocess
import warnings
from pathlib import Path
from typing import Optional, Any, Dict, List, Tuple
import pytest  # pylint: disable=unused-import
import yaml

from icpp.run_shell_cmd import run_shell_cmd

ICP = "icp"

# Names the identity the tests run as, when `pytest --identity <name>` is not
# given.
IDENTITY_ENV_VAR = "ICPP_PRO_TEST_IDENTITY"

# dfx's `--type` / `--output` values, mapped onto the icp-cli equivalents
# (`--args-format` / `--output`). The icp-cli names are passed through, so
# callers can already use them today.
_ARGS_FORMATS = {
    "idl": "candid",
    "candid": "candid",
    "raw": "hex",
    "hex": "hex",
    "bin": "bin",
}
_OUTPUT_FORMATS = {
    "idl": "candid",
    "candid": "candid",
    "pp": "candid",
    "raw": "hex",
    "hex": "hex",
    "text": "text",
    "auto": "auto",
}


# The identity every icp command runs as. This state is deliberately
# process-local: `icp identity default <name>` is machine-wide and persistent,
# so using it to scope an identity to a test would mutate the developer's
# machine - and any other process could change it again mid-run.
_SESSION_IDENTITY: Optional[str] = None
_IDENTITY_OVERRIDE: Optional[str] = None


def set_session_identity(name: str) -> None:
    """Pins the identity for this pytest process. Set once, from a fixture."""
    global _SESSION_IDENTITY  # pylint: disable=global-statement
    _SESSION_IDENTITY = name


def set_identity_override(name: Optional[str]) -> None:
    """Overrides the session identity for one test. `None` clears it."""
    global _IDENTITY_OVERRIDE  # pylint: disable=global-statement
    _IDENTITY_OVERRIDE = name


def _available_identities() -> str:
    """The output of `icp identity list`, to show in an error message."""
    try:
        return run_shell_cmd(
            f"{ICP} identity list", capture_output=True, timeout_seconds=30
        ).rstrip("\n")
    except subprocess.CalledProcessError:
        return "    (could not run `icp identity list`)"


def no_identity_msg() -> str:
    """The 'name your test identity' message, when none has been configured."""
    return (
        "\n\n"
        "ERROR: no test identity configured.\n\n"
        "icpp-pro never uses the machine-wide active identity\n"
        "(`icp identity default`), because any other process can change it\n"
        "while your tests are running.\n\n"
        "Name the identity explicitly:\n\n"
        "    pytest --network=local --identity <name>\n\n"
        "or, once per project:\n\n"
        f"    export {IDENTITY_ENV_VAR}=<name>\n\n"
        "Create one with:\n\n"
        "    icp identity new <name> --storage plaintext\n\n"
        "(icpp-pro exports the key to sign locally, so the identity must not\n"
        " be password protected.)\n\n"
        "Available identities:\n"
        f"{_available_identities()}\n"
    )


def _identity() -> str:
    """The identity to run an icp command as.

    Never reads `icp identity default` - see the comment on _SESSION_IDENTITY.
    """
    name = _IDENTITY_OVERRIDE or _SESSION_IDENTITY
    if name is None:
        pytest.fail(no_identity_msg())
    return name


def _icp_yaml(
    icp_yaml_path: Optional[Path],
    dfx_json_path: Optional[Path],
) -> Optional[Path]:
    """Returns the path of the project's `icp.yaml`, or None if not given.

    `dfx_json_path` is a deprecated alias, kept for one release so test suites
    written against the dfx-based framework keep working. The icp.yaml is
    assumed to sit next to the dfx.json it replaced.
    """
    if dfx_json_path is not None:
        if icp_yaml_path is not None:
            pytest.fail(
                "ERROR: pass either 'icp_yaml_path' or the deprecated "
                "'dfx_json_path', not both."
            )
        warnings.warn(
            "'dfx_json_path' is deprecated, because dfx is deprecated. "
            "Pass 'icp_yaml_path' pointing at your icp.yaml instead.",
            DeprecationWarning,
            stacklevel=3,
        )
        icp_yaml_path = Path(dfx_json_path).parent / "icp.yaml"

    if icp_yaml_path is None:
        return None

    icp_yaml_path = Path(icp_yaml_path)
    if not icp_yaml_path.exists():
        pytest.fail(
            f"ERROR: '{icp_yaml_path}' does not exist.\n"
            f"       icpp-pro uses icp-cli, which reads an 'icp.yaml' project file."
        )

    return icp_yaml_path.resolve()


def _require_icp_yaml(
    icp_yaml_path: Optional[Path],
    dfx_json_path: Optional[Path],
) -> Path:
    """Same as _icp_yaml, but fails the test when no path was given"""
    icp_yaml = _icp_yaml(icp_yaml_path, dfx_json_path)
    if icp_yaml is None:
        pytest.fail("ERROR: 'icp_yaml_path' is required.")
    return icp_yaml


def _canister_names(icp_yaml_path: Path) -> List[str]:
    """Returns the names of the canisters defined in an icp.yaml"""
    with open(icp_yaml_path, "rb") as f:
        data = yaml.safe_load(f)

    canisters = (data or {}).get("canisters") or []
    return [c["name"] for c in canisters if "name" in c]


def _verify_canister_name(
    canister_name: str,
    icp_yaml_path: Optional[Path],
    dfx_json_path: Optional[Path],
) -> Path:
    """Fails the test if the canister is not defined in icp.yaml.

    Returns the path of the icp.yaml (its parent is the icp project root).
    """
    icp_yaml = _require_icp_yaml(icp_yaml_path, dfx_json_path)
    if canister_name not in _canister_names(icp_yaml):
        pytest.fail(
            f"ERROR: canister '{canister_name}' not defined in '{str(icp_yaml)}'"
        )
    return icp_yaml


def _canister_wasm_path(icp_yaml_path: Path, canister_name: str) -> Optional[Path]:
    """Returns the `pre-built` wasm path declared for a canister, if any."""
    with open(icp_yaml_path, "rb") as f:
        data = yaml.safe_load(f)

    for canister in (data or {}).get("canisters") or []:
        if canister.get("name") != canister_name:
            continue
        steps = (canister.get("build") or {}).get("steps") or []
        for step in steps:
            path = step.get("path")
            if path:
                return icp_yaml_path.parent / str(path)
    return None


def _skip_balanced(text: str, i: int) -> int:
    """Given the index of an opening `(`/`{`, returns the index just past its
    matching close, skipping over nested groups and string literals."""
    openers = {"(": ")", "{": "}"}
    stack = [openers[text[i]]]
    i += 1
    while i < len(text) and stack:
        char = text[i]
        if char == '"':
            i += 1
            while i < len(text) and text[i] != '"':
                i += 2 if text[i] == "\\" else 1
        elif char in openers:
            stack.append(openers[char])
        elif char == stack[-1]:
            stack.pop()
        i += 1
    return i


def _is_query_in_did(did_text: str, method: str) -> Optional[bool]:
    """True/False if `method` is/isn't declared `query` in the Candid text.

    Returns None when the method cannot be found, so the caller can fall back
    to an update call.

    A declaration looks like:
        get_count : () -> (nat64) query;
        "quoted name" : (record { a: nat; b: nat }) -> (text);
    The annotation sits between the closing paren of the result type and the
    terminating `;` - which is why the result type has to be skipped as a
    balanced group: it may itself contain `;` inside a `record { ... }`.
    """
    text = re.sub(r"//[^\n]*", "", did_text)

    for match in re.finditer(
        rf'(?:"{re.escape(method)}"|\b{re.escape(method)}\b)\s*:', text
    ):
        arrow = text.find("->", match.end())
        if arrow == -1:
            continue
        i = arrow + 2
        while i < len(text) and text[i].isspace():
            i += 1
        if i < len(text) and text[i] in "({":
            i = _skip_balanced(text, i)
        end = text.find(";", i)
        annotation = text[i : end if end != -1 else len(text)]
        return bool(re.search(r"\b(query|composite_query)\b", annotation))
    return None


# canister -> {method: is_query}. A polling loop hits the same method many
# times; resolving the interface once keeps that cheap.
_CANDID_CACHE: Dict[Tuple[str, str, str], Optional[str]] = {}


def _candid_text(
    project_root: Path,
    icp_yaml_path: Path,
    canister_name: str,
    network: str,
    timeout_seconds: Optional[int],
    identity: str,
) -> Optional[str]:
    """Returns the canister's Candid interface, or None if it cannot be found.

    Looks for the `.did` that `icpp build-wasm` writes next to the wasm first
    (no network needed), then asks the deployed canister for its
    `candid:service` metadata - which works for any canister, however it was
    built.
    """
    key = (str(project_root), canister_name, network)
    if key in _CANDID_CACHE:
        return _CANDID_CACHE[key]

    did_text: Optional[str] = None

    wasm_path = _canister_wasm_path(icp_yaml_path, canister_name)
    if wasm_path is not None:
        did_path = wasm_path.with_suffix(".did")
        if did_path.exists():
            did_text = did_path.read_text(encoding="utf-8")

    if did_text is None:
        try:
            did_text = _run_icp(
                f" canister metadata {canister_name} candid:service "
                f" --environment {network} ",
                project_root,
                timeout_seconds,
                identity=identity,
            )
        except subprocess.CalledProcessError:
            did_text = None

    _CANDID_CACHE[key] = did_text
    return did_text


def _as_candid_arg_tuple(argument: str) -> str:
    """Wraps a bare Candid value in the parentheses that icp-cli requires.

    dfx accepted a bare value, eg. `"a text"`, as a one-element argument list.
    icp-cli's Candid parser insists on the tuple form, `("a text")`. Callers
    that build their argument with `dict_to_candid_text` rely on the lenient
    behaviour, so keep accepting both.
    """
    stripped = argument.strip()
    if stripped.startswith("("):
        return argument
    return f"({stripped})"


def _run_icp(
    args: str,
    project_root: Optional[Path] = None,
    timeout_seconds: Optional[int] = None,
    identity: Optional[str] = None,
) -> str:
    """Runs an icp command against a project & returns its captured output.

    Without a `project_root`, icp finds the project itself by walking up from
    the current working directory.

    `identity` is appended as `--identity <name>`. Pass it only for
    subcommands that accept it: `canister call`, `canister metadata`,
    `canister status` and `deploy` do, `network ping` and `network status`
    do not.
    """
    cmd = f"{ICP} {args} "
    if identity is not None:
        cmd += f"--identity {shlex.quote(identity)} "
    if project_root is not None:
        cmd += f"--project-root-override {str(project_root)} "
    # icp launches an interactive prompt when it is missing an argument. Close
    # stdin so it can never block a test run waiting for input.
    return run_shell_cmd(
        f"{cmd} < /dev/null",
        capture_output=True,
        timeout_seconds=timeout_seconds,
    ).rstrip("\n")


def call_canister_api(
    *,
    canister_name: str,
    canister_method: str,
    icp_yaml_path: Optional[Path] = None,
    canister_argument: Optional[str] = None,
    canister_input: str = "idl",
    canister_output: str = "idl",
    network: str = "local",
    query: Optional[bool] = None,
    quiet: str = "-qq",  # deprecated: dfx only, icp-cli has no verbosity flag
    timeout_seconds: Optional[int] = None,
    identity: Optional[str] = None,
    dfx_json_path: Optional[Path] = None,  # deprecated alias of icp_yaml_path
) -> str:
    """Calls a canister method.

    `identity` names the icp identity to call as. It defaults to the identity
    of the pytest session (`pytest --identity <name>` or the
    ICPP_PRO_TEST_IDENTITY environment variable), which the
    `identity_anonymous` fixture temporarily overrides.

    `query` selects the request type:

    - `None` (default) - look the method up in the canister's Candid interface
      and send a query request if it is declared `query`, else an update
      request. This is what dfx did. Unlike the icp-cli default (always an
      update request), it keeps query methods fast, which matters for tests
      that poll a counter or assert on timing.
    - `True` / `False` - force a query / update request.

    When the interface cannot be resolved, an update request is sent: that is
    valid for query methods too, so the fallback can only cost speed, never
    correctness.
    """
    del quiet  # accepted for backwards compatibility, has no icp-cli equivalent

    identity = identity or _identity()

    icp_yaml = _verify_canister_name(canister_name, icp_yaml_path, dfx_json_path)
    project_root = icp_yaml.parent

    if canister_input not in _ARGS_FORMATS:
        pytest.fail(f"ERROR: unsupported canister_input '{canister_input}'")
    if canister_output not in _OUTPUT_FORMATS:
        pytest.fail(f"ERROR: unsupported canister_output '{canister_output}'")

    argument = "()" if canister_argument is None else canister_argument
    if _ARGS_FORMATS[canister_input] == "candid":
        argument = _as_candid_arg_tuple(argument)

    if query is None:
        did_text = _candid_text(
            project_root, icp_yaml, canister_name, network, timeout_seconds, identity
        )
        query = bool(
            did_text is not None and _is_query_in_did(did_text, canister_method)
        )

    arg = (
        f" canister call "
        f" {canister_name} "
        f" {canister_method} "
        f" '{argument}' "
        f" --environment {network} "
        f" --args-format {_ARGS_FORMATS[canister_input]} "
        f" --output {_OUTPUT_FORMATS[canister_output]} "
    )
    if query:
        arg += " --query "

    try:
        response = _run_icp(arg, project_root, timeout_seconds, identity=identity)
    except subprocess.CalledProcessError as e:
        if _is_not_deployed_error(e.output):
            pytest.exit(_not_deployed_msg(canister_name, network, canister_method))
        response = (
            f"Failed call to api '{canister_method}' of canister '{canister_name}':"
            f"{e.output}"
        )

    return response


def get_canister_url(
    *,
    canister_name: str,
    icp_yaml_path: Optional[Path] = None,
    network: str = "local",
    url_path: Optional[str] = None,
    timeout_seconds: Optional[int] = None,
    identity: Optional[str] = None,
    dfx_json_path: Optional[Path] = None,  # deprecated alias of icp_yaml_path
) -> str:
    """THIS FUNCTION IS DEPRECATED. DO NOT USE...
    use get_canister_url_with_headers instead

    Returns the url for calling a canister as a Web2.0 HTTP server
    """
    print("get_canister_url has been deprecated since dfx 0.14.1")
    print("instead, use: get_canister_url_with_headers")

    canister_id = get_canister_id(
        icp_yaml_path=icp_yaml_path,
        dfx_json_path=dfx_json_path,
        canister_name=canister_name,
        network=network,
        timeout_seconds=timeout_seconds,
        identity=identity,
    )

    if network == "ic":
        url = f"https://{canister_id}"
    else:
        webserver_port = get_local_webserver_port(
            icp_yaml_path=icp_yaml_path,
            dfx_json_path=dfx_json_path,
            network=network,
            timeout_seconds=timeout_seconds,
        )
        url = f"http://localhost:{webserver_port}"

    # Add the path
    if url_path is not None:
        url = f"{url}/{url_path}"

    # For local network, add the canister_id as a query parameter
    if network != "ic":
        url = f"{url}?canisterId={canister_id}"

    return url


def get_canister_url_with_headers(
    *,
    canister_name: str,
    icp_yaml_path: Optional[Path] = None,
    network: str = "local",
    url_path: Optional[str] = None,
    timeout_seconds: Optional[int] = None,
    identity: Optional[str] = None,
    dfx_json_path: Optional[Path] = None,  # deprecated alias of icp_yaml_path
) -> Tuple[str, Optional[Dict[str, str]]]:
    """Returns the url + headers for calling a canister as a Web2.0 HTTP server"""

    canister_id = get_canister_id(
        icp_yaml_path=icp_yaml_path,
        dfx_json_path=dfx_json_path,
        canister_name=canister_name,
        network=network,
        timeout_seconds=timeout_seconds,
        identity=identity,
    )

    # The `raw` subdomain bypasses response certification, which an icpp-pro
    # `http_request` does not provide. Same for the icp-cli gateway as it was
    # for the dfx one:
    # https://forum.dfinity.org/t/upgrading-to-dfx-0-24-1-breaks-the-http-request-endpoint/36709/13?u=icpp # pylint: disable=line-too-long
    headers = None
    if network == "ic":
        url = f"https://{canister_id}.raw.icp0.io"
    else:
        webserver_port = get_local_webserver_port(
            icp_yaml_path=icp_yaml_path,
            dfx_json_path=dfx_json_path,
            network=network,
            timeout_seconds=timeout_seconds,
        )
        url = f"http://127.0.0.1:{webserver_port}"
        headers = {"Host": f"{canister_id}.raw.localhost"}

    # Add the path
    if url_path is not None:
        url = f"{url}/{url_path}"

    return url, headers


def get_canister_id(
    *,
    canister_name: str,
    icp_yaml_path: Optional[Path] = None,
    network: str = "local",
    timeout_seconds: Optional[int] = None,
    identity: Optional[str] = None,
    dfx_json_path: Optional[Path] = None,  # deprecated alias of icp_yaml_path
) -> str:
    """Returns the canister_id of a canister"""

    project_root = _verify_canister_name(
        canister_name, icp_yaml_path, dfx_json_path
    ).parent

    # icp-cli records the deployed ids per environment in its ID store. For a
    # `connected` network that store is persistent (`data/`); for a `managed`
    # network it lives alongside the disposable network state (`cache/`).
    for sub_dir in ("data", "cache"):
        ids_path = project_root / ".icp" / sub_dir / "mappings" / f"{network}.ids.json"
        if ids_path.exists():
            with open(ids_path, "rb") as f:
                ids = json.load(f)
            if canister_name in ids:
                return str(ids[canister_name])

    # Not in the ID store - ask the network. This also covers canisters that
    # were deployed from elsewhere and linked into the project.
    try:
        response = _run_icp(
            f" canister status {canister_name} --environment {network} --json ",
            project_root,
            timeout_seconds,
            identity=identity or _identity(),
        )
    except subprocess.CalledProcessError as e:
        if _is_not_deployed_error(e.output):
            pytest.exit(_not_deployed_msg(canister_name, network))
        pytest.fail(f"Failed to get id of canister '{canister_name}': {e.output}")

    return str(json.loads(response)["id"])


def get_local_webserver_port(
    *,
    icp_yaml_path: Optional[Path] = None,
    network: str = "local",
    timeout_seconds: Optional[int] = None,
    dfx_json_path: Optional[Path] = None,  # deprecated alias of icp_yaml_path
) -> str:
    """Returns the port of the network's HTTP gateway"""

    icp_yaml = _icp_yaml(icp_yaml_path, dfx_json_path)
    project_root = None if icp_yaml is None else icp_yaml.parent

    try:
        response = _run_icp(
            f" network status --environment {network} --json ",
            project_root,
            timeout_seconds,
        )
    except subprocess.CalledProcessError as e:
        pytest.fail(f"Failed to get local network's webserver port: {e.output}")

    gateway_url = json.loads(response)["gateway_url"]
    port = gateway_url.rstrip("/").rsplit(":", maxsplit=1)[-1]
    return str(port)


def flatten_candid_text(candid_text: str) -> str:
    """Collapses the line breaks & indentation of pretty-printed Candid text.

    dfx printed most responses on a single line, icp-cli wraps records, vecs
    and long values over several indented lines. Both are the same Candid
    value, so compare the flattened form when asserting on a response:

        assert flatten_candid_text(response) == '( record { n = 1 : nat }, )'

    Note that this also collapses runs of whitespace *inside* Candid string
    literals, so do not use it when that whitespace is what you are testing.
    """
    return re.sub(r"\s+", " ", candid_text).strip()


def dict_to_candid_text(d: Dict[Any, Any]) -> str:
    """Serializes dict to Candid text to send it over the wire"""
    # pull it through twice, so all " are escaped into \"
    return json.dumps(json.dumps(d))


def network_status(
    network: str,
    icp_yaml_path: Optional[Path] = None,
    dfx_json_path: Optional[Path] = None,  # deprecated alias of icp_yaml_path
) -> str:
    """Returns the network status.

    Without a path, icp finds the project by walking up from the current
    working directory, so run pytest from within your project.
    """
    icp_yaml = _icp_yaml(icp_yaml_path, dfx_json_path)
    project_root = None if icp_yaml is None else icp_yaml.parent

    try:
        response = _run_icp(f" network ping --environment {network} ", project_root)
    except subprocess.CalledProcessError:
        if network == "ic":
            msg = (
                "\n"
                "*******************************\n"
                "*** The IC is not up        ***\n"
                "*** Please try again later  ***\n"
                "*******************************"
            )
        else:
            msg = (
                "\n"
                "*******************************************\n"
                "*** The local network is not up         ***\n"
                "*** Please start it first with:         ***\n"
                f"***  {ICP} network start --background     ***\n"
                "*******************************************"
            )
        pytest.exit(msg)

    return response


# There is deliberately no `get_identity()`. It returned the machine-wide
# active identity (`icp identity default`), which every other process on the
# machine can change at any moment - so anything derived from it, including
# `identity=get_identity()`, is a race. Run `icp identity default` in a shell
# if you want to report it.


def get_principal(identity: Optional[str] = None) -> str:
    """Returns the principal of an identity, without changing anything.

    Defaults to the identity of the pytest session. `icp identity principal`
    is asked for it by name, so the machine-wide active identity is neither
    read nor written.
    """
    name = identity or _identity()
    arg = f"{ICP} identity principal --identity {shlex.quote(name)} "
    try:
        principal = run_shell_cmd(arg, capture_output=True, timeout_seconds=30)
        principal = principal.rstrip("\n")
    except subprocess.CalledProcessError as e:
        pytest.fail(f"ERROR: command {arg} failed with error:\n{e.output}")

    return principal


def _is_not_deployed_error(output: str) -> bool:
    """True if icp failed because the canister has no id on this network yet"""
    return "could not find ID for canister" in output


def _not_deployed_msg(
    canister_name: str,
    network: str,
    canister_method: Optional[str] = None,
) -> str:
    """The 'deploy the canister first' message shown when a call has no target"""
    what = (
        f"Call to api '{canister_method}' of canister '{canister_name}'"
        if canister_method is not None
        else f"Failed to get id of canister '{canister_name}'"
    )
    return (
        "\n\n"
        f"FAIL: {what}\n\n"
        "*******************************************\n"
        "*** Failed to determine id for canister ***\n"
        "*** Deploy the canister first with:     ***\n"
        "*******************************************\n"
        f"  {ICP} deploy --environment {network}"
    )
