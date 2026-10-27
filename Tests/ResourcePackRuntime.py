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
    directory = args.logs.resolve()
    directory.mkdir(parents=True, exist_ok=True)
    environment = dict(os.environ, GENGINE_EXAMPLE_FRAME_LIMIT="120",
                       GENGINE_EXAMPLE_RESOURCE_CHECK="1")
    environment.pop("GENGINE_EXAMPLE_SECONDS", None)
    try:
        for backend, api in [("OpenGL", 1), ("Vulkan", 2), ("D3D12", 3)]:
            variant = []
            for name, payload in items:
                if name == b"Config.ini":
                    payload, count = re.subn(rb"GraphicsAPI: [123]", f"GraphicsAPI: {api}".encode(), payload)
                    if count != 1:
                        raise RuntimeError("Fixture config must select exactly one backend")
                variant.append((name, payload))
            replace_pack(pack, encode(variant))
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
            if (session / "Assets").exists() or not (session / "Cache/Shaders").is_dir():
                raise RuntimeError(f"{backend}: extracted assets or missing writable cache")
            print(f"PASS {backend}: 120 frames, three readbacks, image/FBX/background reads, no Assets extraction")
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
    finally:
        replace_pack(pack, original)


if __name__ == "__main__":
    main()
