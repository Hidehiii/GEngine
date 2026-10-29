"""Run an exported FrameGraphTriangle against each backend without changing source config."""
import argparse
import hashlib
import os
from pathlib import Path
import re
import struct
import subprocess


def entries(data):
    if data[:8] != b"GEPAK001":
        raise RuntimeError("Unexpected archive version")
    position = 12
    result = []
    for _ in range(struct.unpack_from("<I", data, 8)[0]):
        path_size, size = struct.unpack_from("<IQ", data, position)
        position += 44
        name = data[position:position + path_size]
        position += path_size
        payload = data[position:position + size]
        position += size
        result.append((name, payload))
    if position != len(data):
        raise RuntimeError("Fixture archive bounds mismatch")
    return result


def encode(items):
    result = b"GEPAK001" + struct.pack("<I", len(items))
    for name, payload in items:
        result += struct.pack("<IQ", len(name), len(payload))
        result += hashlib.sha256(payload).digest() + name + payload
    return result


def replace_pack(path, data):
    temporary = path.with_suffix(".runtime-test")
    temporary.write_bytes(data)
    temporary.replace(path)


def run(executable, directory, environment):
    process = subprocess.Popen([str(executable)], cwd=directory, env=environment,
                               stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    try:
        output, _ = process.communicate(timeout=60)
    except subprocess.TimeoutExpired:
        subprocess.run(["taskkill", "/PID", str(process.pid), "/T", "/F"],
                       capture_output=True, check=False)
        process.communicate()
        raise RuntimeError("Owned runtime test exceeded 60 seconds")
    return process.returncode, output.decode("utf-8", errors="replace")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("executable", type=Path)
    parser.add_argument("--logs", required=True, type=Path)
    args = parser.parse_args()
    executable = args.executable.resolve()
    pack = executable.parent / "Data" / (executable.stem + ".gpak")
    original = pack.read_bytes()
    items = entries(original)
    if any(name.lower() == b"config.ini" for name, _ in items):
        raise RuntimeError("Config.ini must not be packed into gpak")
    config = executable.parent / "Config" / "Config.ini"
    original_config = config.read_bytes()
    directory = args.logs.resolve()
    directory.mkdir(parents=True, exist_ok=True)
    environment = dict(os.environ, GENGINE_EXAMPLE_FRAME_LIMIT="120",
                       GENGINE_EXAMPLE_RESOURCE_CHECK="1")
    environment.pop("GENGINE_EXAMPLE_SECONDS", None)
    state = executable.parent
    try:
        for backend, api in [("OpenGL", 1), ("Vulkan", 2), ("D3D12", 3)]:
            variant, count = re.subn(rb"GraphicsAPI: [123]", f"GraphicsAPI: {api}".encode(), original_config)
            if count != 1:
                raise RuntimeError("External config must select exactly one backend")
            config.write_bytes(variant)
            code, output = run(executable, directory, environment)
            (directory / f"{backend}.log").write_text(output, encoding="utf-8")
            if code or output.count("readback passed after 119 frames.") != 3:
                raise RuntimeError(f"{backend}: exit {code}; see {directory / (backend + '.log')}")
            if "Resource archive check passed at shutdown" not in output:
                raise RuntimeError(f"{backend}: archive shutdown check missing")
            match = re.search(r"archive=true cwd=(.+)", output)
            if not match:
                raise RuntimeError(f"{backend}: archive read probe missing")
            session = Path(match.group(1).strip())
            cache = session / "Cache/Shaders" / str(api)
            if (session / "Assets").exists() or not cache.is_dir() or session != state:
                raise RuntimeError(f"{backend}: extracted assets or missing writable cache")
            cached = {path.relative_to(cache): path.read_bytes() for path in cache.rglob("*") if path.is_file()}
            if not cached:
                raise RuntimeError(f"{backend}: shader cache directory is empty")
            code, warm = run(executable, directory, environment)
            (directory / f"{backend}-warm.log").write_text(warm, encoding="utf-8")
            after = {path.relative_to(cache): path.read_bytes() for path in cache.rglob("*") if path.is_file()}
            if code or warm.count("readback passed after 119 frames.") != 3 or warm.count(" from cache.") < 2 or cached != after:
                raise RuntimeError(f"{backend}: second launch failed cache reuse")
            print(f"PASS {backend}: external config, cold/warm 120 frames, three readbacks, unchanged reused cache ({len(cached)} files)")
        changed_include = [(name, payload.replace(
                                b"payload: |\n",
                                b"payload: |\n    // persistent cache dependency probe\n", 1)
                            if name.endswith(b"FrameGraphColorLogic.gasset") else payload)
                           for name, payload in items]
        replace_pack(pack, encode(changed_include))
        code, output = run(executable, directory, environment)
        (directory / "changed-include.log").write_text(output, encoding="utf-8")
        if code or "Shader cache is invalid" not in output or output.count("readback passed after 119 frames.") != 3:
            raise RuntimeError("Changed include did not invalidate persistent shader cache")
        print("PASS changed HLSL include invalidates its shader cache")
        corrupt = bytearray(original)
        corrupt[-1] ^= 1
        replace_pack(pack, corrupt)
        code, output = run(executable, directory, environment)
        (directory / "corrupt.log").write_text(output, encoding="utf-8")
        if code != 1 or "Resource checksum mismatch" not in output or "Creating window" in output:
            raise RuntimeError("Corrupt pack did not fail before Application")
        pack.unlink()
        code, output = run(executable, directory, environment)
        (directory / "missing.log").write_text(output, encoding="utf-8")
        if code != 1 or "Required resource pack is missing" not in output:
            raise RuntimeError("Missing pack did not fail before Application")
        print("PASS startup rejection: corrupt/missing packs fail before Application")
        replace_pack(pack, original)
        config.unlink()
        code, output = run(executable, directory, environment)
        (directory / "missing-config.log").write_text(output, encoding="utf-8")
        if code != 1 or "External Config/Config.ini is missing" not in output or "Creating window" in output:
            raise RuntimeError("Missing external config did not fail before Application")
        print("PASS missing external config rejection; original config/pack restored")
    finally:
        replace_pack(pack, original)
        config.write_bytes(original_config)


if __name__ == "__main__":
    main()
