"""Run native FrameGraphTriangle foundation and matched lifetime regressions."""
import argparse
import csv
import os
from pathlib import Path
import re
import shutil
import subprocess
import time

from D3D12QueueRegression import sample


def run_case(executable, assets, directory, frames, api, dedicated, layer, replacement, msaa=False):
    directory.mkdir(parents=True, exist_ok=True)
    shutil.copytree(assets / "Assets", directory / "Assets", dirs_exist_ok=True)
    config = (assets / "Config.ini").read_text(encoding="utf-8-sig")
    config, count = re.subn(r"GraphicsAPI: [123]", f"GraphicsAPI: {api}", config)
    if count != 1:
        raise RuntimeError("Expected exactly one backend selection in the fixture")
    (directory / "Config.ini").write_text(config, encoding="utf-8")
    environment = dict(os.environ)
    for name in ("GENGINE_EXAMPLE_SECONDS", "GENGINE_EXAMPLE_FRAME_LIMIT",
                 "GENGINE_EXAMPLE_NO_REPLACEMENT", "GENGINE_EXAMPLE_SYNTHETIC_RESIZE",
                 "GENGINE_EXAMPLE_DISCARD_RECORDING", "GENGINE_EXAMPLE_IMGUI",
                 "GENGINE_EXAMPLE_RESOURCE_CHECK", "GENGINE_VULKAN_EXCLUSIVE_TRANSFER_IMAGES",
                 "GENGINE_VULKAN_EXCLUSIVE_COMPUTE_BUFFERS", "GENGINE_EXAMPLE_MSAA_CHECK"):
        environment.pop(name, None)
    environment.update(GENGINE_EXAMPLE_FRAME_LIMIT=str(frames),
                       GENGINE_VULKAN_DEDICATED_QUEUES=str(int(dedicated)),
                       GENGINE_D3D12_DEBUG_LAYER=str(int(layer)),
                       GENGINE_D3D12_INFO_QUEUE_LOGGER="1",
                       GENGINE_D3D12_LIFETIME_TRACE="1",
                       GENGINE_D3D12_LIVE_OBJECTS="1")
    if not replacement:
        environment["GENGINE_EXAMPLE_NO_REPLACEMENT"] = "1"
    if msaa:
        environment["GENGINE_EXAMPLE_MSAA_CHECK"] = "1"
    samples = []
    timed_out = False
    started = time.monotonic()
    log_path = directory / "runtime.log"
    with log_path.open("wb") as output:
        process = subprocess.Popen([str(executable)], cwd=directory, env=environment,
                                   stdout=output, stderr=subprocess.STDOUT)
        try:
            while process.poll() is None:
                elapsed = time.monotonic() - started
                if elapsed > 120:
                    timed_out = True
                    break
                counters = sample(process)
                if counters:
                    samples.append((elapsed, *counters))
                time.sleep(0.2)
        finally:
            if process.poll() is None:
                process.kill()
            process.wait()
    with (directory / "memory.csv").open("w", newline="", encoding="utf-8") as output:
        writer = csv.writer(output)
        writer.writerow(("seconds", "handles", "private_bytes"))
        writer.writerows(samples)
    log = log_path.read_text(encoding="utf-8", errors="replace")
    if timed_out:
        raise RuntimeError(f"Timed out agent-started run; samples and log retained: {directory}")
    if process.returncode or "Foundation attachment preflight passed:" not in log:
        raise RuntimeError(f"Foundation startup failed (exit {process.returncode:#x}): {log_path}")
    completed = f"FrameGraphTriangle completed {frames} frames; requesting normal shutdown."
    if msaa and log.count("MSAA matrix passed:") != 3:
        raise RuntimeError(f"MSAA pixel/readback checks incomplete: {log_path}")
    if msaa and (log.count("MSAA cross-frame LOAD passed at frame") != 4 or
                 "MSAA numeric depth passed:" not in log):
        raise RuntimeError(f"MSAA depth/cross-frame LOAD checks incomplete: {log_path}")
    if log.count("readback passed after 119 frames.") != 3 or completed not in log:
        raise RuntimeError(f"GPU readback/shutdown incomplete: {log_path}")
    if "D3D12 Error:" in log or "Validation layer:" in log or "GL_INVALID_" in log or (msaa and "D3D12 Warning:" in log):
        raise RuntimeError(f"Native validation diagnostic: {log_path}")
    logger_active = "Starting D3D12 debug info queue logger thread." in log
    if api == 3 and logger_active != layer:
        raise RuntimeError(f"D3D12 layer control was not applied: {log_path}")
    duration = time.monotonic() - started
    steady = [row for row in samples if 3 <= row[0] <= duration - 1 and row[1] > 0]
    summary = ""
    if len(steady) > 1:
        summary = (f"; handles {steady[0][1]}->{steady[-1][1]}, "
                   f"private bytes {steady[0][2]}->{steady[-1][2]}")
    print(f"PASS {directory.name}: {frames} frames, preflight + three readbacks, "
          f"exit 0, {time.monotonic() - started:.1f}s{summary}", flush=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("executable", type=Path, help="Native FrameGraphTriangle.exe (not a packaged entry)")
    parser.add_argument("--logs", required=True, type=Path)
    parser.add_argument("--mode", choices=("matrix", "lifetime"), default="matrix")
    parser.add_argument("--frames", type=int)
    parser.add_argument("--msaa", action="store_true", help="Exercise format queries and MSAA MRT pixel readbacks")
    parser.add_argument("--backend", choices=("OpenGL", "Vulkan", "D3D12", "VulkanDedicated"))
    args = parser.parse_args()
    executable = args.executable.resolve()
    if not executable.is_file():
        raise RuntimeError(f"Missing executable: {executable}")
    frames = args.frames if args.frames is not None else (120 if args.mode == "matrix" else 30000)
    if not 120 <= frames <= 60000:
        raise RuntimeError("Choose 120 through 60000 frames for bounded diagnostics")
    assets = Path(__file__).resolve().parents[1] / "Example/FrameGraphTriangle"
    if args.mode == "matrix":
        cases = [("OpenGL", 1, False, False, True), ("Vulkan", 2, False, False, True),
                 ("D3D12", 3, False, True, True), ("VulkanDedicated", 2, True, False, True)]
    else:
        cases = [(f"layer-{layer}-replacement-{replacement}", 3, False, bool(layer), bool(replacement))
                 for layer in (0, 1) for replacement in (0, 1)]
    for name, api, dedicated, layer, replacement in cases:
        if args.backend and args.mode == "matrix" and name != args.backend:
            continue
        run_case(executable, assets, args.logs.resolve() / name,
                 frames, api, dedicated, layer, replacement, args.msaa)


if __name__ == "__main__":
    main()
