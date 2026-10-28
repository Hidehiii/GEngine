"""Exercise consecutive GPU graphs with the real D3D12 debug layer enabled."""
import argparse
import ctypes
from ctypes import wintypes
import os
from pathlib import Path
import re
import subprocess
import time


class MemoryCounters(ctypes.Structure):
    _fields_ = [("cb", wintypes.DWORD), ("PageFaultCount", wintypes.DWORD)] + [
        (name, ctypes.c_size_t) for name in (
            "PeakWorkingSetSize", "WorkingSetSize", "QuotaPeakPagedPoolUsage",
            "QuotaPagedPoolUsage", "QuotaPeakNonPagedPoolUsage", "QuotaNonPagedPoolUsage",
            "PagefileUsage", "PeakPagefileUsage", "PrivateUsage")]


def sample(process):
    handle = wintypes.HANDLE(int(process._handle))
    count = wintypes.DWORD()
    memory = MemoryCounters()
    memory.cb = ctypes.sizeof(memory)
    ok = ctypes.windll.kernel32.GetProcessHandleCount(handle, ctypes.byref(count))
    ok = ok and ctypes.windll.psapi.GetProcessMemoryInfo(handle, ctypes.byref(memory), memory.cb)
    return (count.value, memory.PrivateUsage) if ok else None


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("executable", type=Path)
    parser.add_argument("--logs", required=True, type=Path)
    parser.add_argument("--stress-seconds", type=int, default=15)
    args = parser.parse_args()
    if not 4 <= args.stress_seconds <= 45:
        raise RuntimeError("Choose 4 through 45 seconds for bounded stress")
    root_entry = args.executable.resolve()
    binary = root_entry.parent / "Runtime" / root_entry.name
    if not binary.is_file():
        raise RuntimeError("Pass a packaged FrameGraphTriangle root entry")
    config = root_entry.parent / "Config/Config.ini"
    original = config.read_bytes()
    changed, count = re.subn(rb"GraphicsAPI: [123]", b"GraphicsAPI: 3", original)
    if count != 1:
        raise RuntimeError("Expected one external backend setting")
    directory = args.logs.resolve()
    directory.mkdir(parents=True, exist_ok=True)
    base = dict(os.environ, GENGINE_D3D12_DEBUG_LAYER="1", GENGINE_D3D12_INFO_QUEUE_LOGGER="1")
    for name in ("GENGINE_EXAMPLE_SECONDS", "GENGINE_EXAMPLE_FRAME_LIMIT",
                 "GENGINE_EXAMPLE_NO_REPLACEMENT", "GENGINE_EXAMPLE_SYNTHETIC_RESIZE",
                 "GENGINE_EXAMPLE_DISCARD_RECORDING"):
        base.pop(name, None)
    cases = [("replacement", {"GENGINE_EXAMPLE_FRAME_LIMIT": "120"}),
             ("no-replacement", {"GENGINE_EXAMPLE_FRAME_LIMIT": "120", "GENGINE_EXAMPLE_NO_REPLACEMENT": "1"}),
             ("discard-resize", {"GENGINE_EXAMPLE_FRAME_LIMIT": "120", "GENGINE_EXAMPLE_DISCARD_RECORDING": "1",
                                  "GENGINE_EXAMPLE_SYNTHETIC_RESIZE": "1"}),
             ("stress", {"GENGINE_EXAMPLE_SECONDS": str(args.stress_seconds)})]
    try:
        config.write_bytes(changed)
        for name, overrides in cases:
            samples = []
            started = time.monotonic()
            path = directory / f"{name}.log"
            with path.open("wb") as output:
                process = subprocess.Popen([str(binary)], cwd=directory, env=dict(base, **overrides),
                                           stdout=output, stderr=subprocess.STDOUT)
                try:
                    while process.poll() is None:
                        elapsed = time.monotonic() - started
                        if elapsed > 60:
                            subprocess.run(["taskkill", "/PID", str(process.pid), "/T", "/F"],
                                           capture_output=True, check=False)
                            raise RuntimeError(f"{name}: timed out; possible unsignaled dependency")
                        counters = sample(process)
                        if counters:
                            samples.append((elapsed, *counters))
                        time.sleep(0.1)
                finally:
                    if process.poll() is None:
                        process.kill()
                    process.wait()
            text = path.read_text(encoding="utf-8", errors="replace")
            if process.returncode or "Starting D3D12 debug info queue logger thread." not in text:
                raise RuntimeError(f"{name}: runtime/debug layer failed; see {path}")
            if "D3D12 Error:" in text or "D3D12 Warning:" in text or text.count("readback passed after 119 frames.") != 3:
                raise RuntimeError(f"{name}: debug diagnostic or readback failure; see {path}")
            if name == "discard-resize" and ("Discarded a graphics UAV recording" not in text or
                                            text.count("Synthetic D3D12 resize requested") != 2):
                raise RuntimeError("Discarded-recording/resize scenario did not execute")
            (directory / f"{name}-memory.csv").write_text(
                "seconds,handles,private_bytes\n" + "".join(f"{t:.3f},{h},{m}\n" for t, h, m in samples), encoding="utf-8")
            steady = [value for value in samples if value[0] >= 3]
            summary = f"; steady handles {min(v[1] for v in steady)}..{max(v[1] for v in steady)}, private bytes {steady[0][2]} -> {steady[-1][2]}" if steady else ""
            print(f"PASS {name}: three readbacks, no D3D12 warnings/errors{summary}", flush=True)
    finally:
        config.write_bytes(original)


if __name__ == "__main__":
    main()
