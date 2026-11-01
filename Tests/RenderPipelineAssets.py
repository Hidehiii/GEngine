"""Run native pipeline asset selection, rollback and GPU lifetime checks."""
import argparse
import csv
import os
from pathlib import Path
import re
import shutil
import subprocess
import time

from D3D12QueueRegression import sample


def run(executable, directory, frames, api, dedicated, stack=False, post=False):
    assets = Path(__file__).resolve().parents[1] / "Example/RenderPipeline"
    directory.mkdir(parents=True, exist_ok=True)
    shutil.copytree(assets / "Assets", directory / "Assets", dirs_exist_ok=True)
    config, changed = re.subn(r"GraphicsAPI: [123]", f"GraphicsAPI: {api}",
                             (assets / "Config.ini").read_text(encoding="utf-8-sig"))
    if changed != 1:
        raise RuntimeError("Expected one backend selection")
    if post:
        config, budgets = re.subn(r"CommandBufferCount: \d+", "CommandBufferCount: 12", config)
        if budgets != 1:
            raise RuntimeError("Expected one command capacity setting")
    (directory / "Config.ini").write_text(config, encoding="utf-8")
    env = dict(os.environ)
    env.update(GENGINE_RENDER_PIPELINE_FRAME_LIMIT=str(frames), GENGINE_RENDER_PIPELINE_ASSET_CHECK="1",
               GENGINE_D3D12_DEBUG_LAYER="1", GENGINE_D3D12_INFO_QUEUE_LOGGER="1",
               GENGINE_D3D12_LIFETIME_TRACE="1", GENGINE_D3D12_LIVE_OBJECTS="1",
               GENGINE_VULKAN_DEDICATED_QUEUES=str(int(dedicated)))
    env.pop("GENGINE_RENDER_PIPELINE_POST_CHECK", None)
    if post:
        env.pop("GENGINE_RENDER_PIPELINE_ASSET_CHECK", None)
        env.pop("GENGINE_RENDER_PIPELINE_STACK_CHECK", None)
        env["GENGINE_RENDER_PIPELINE_POST_CHECK"] = "1"
    elif stack:
        env.pop("GENGINE_RENDER_PIPELINE_ASSET_CHECK", None)
        env["GENGINE_RENDER_PIPELINE_STACK_CHECK"] = "1"
    else:
        env.pop("GENGINE_RENDER_PIPELINE_STACK_CHECK", None)
    for key in ("GENGINE_VULKAN_EXCLUSIVE_TRANSFER_IMAGES", "GENGINE_VULKAN_EXCLUSIVE_COMPUTE_BUFFERS"):
        env.pop(key, None)
    started = time.monotonic()
    samples = []
    timed_out = False
    path = directory / "runtime.log"
    with path.open("wb") as output:
        process = subprocess.Popen([str(executable)], cwd=directory, env=env,
                                   stdout=output, stderr=subprocess.STDOUT)
        try:
            while process.poll() is None:
                if time.monotonic() - started > 120:
                    timed_out = True
                    break
                counters = sample(process)
                if counters:
                    samples.append((time.monotonic() - started, *counters))
                time.sleep(0.2)
        finally:
            if process.poll() is None:
                process.kill()
            process.wait()
    with (directory / "memory.csv").open("w", newline="", encoding="utf-8") as output:
        writer = csv.writer(output)
        writer.writerow(("seconds", "handles", "private_bytes"))
        writer.writerows(samples)
    text = path.read_text(encoding="utf-8", errors="replace")
    switches = (frames - 1) // 15
    pixels = switches + (4 if stack or post else 2)
    if timed_out or process.returncode:
        raise RuntimeError(f"Run failed: timeout={timed_out}, exit={process.returncode:#x}: {path}")
    required = ("Pipeline asset preflight and rollback passed.",
                "Pipeline asset registry snapshot passed.",
                f"RenderPipeline completed {frames} frames; requesting normal shutdown.")
    if stack:
        required = ("Camera stack preflight passed: 16 cases.",
                    f"RenderPipeline completed {frames} frames; requesting normal shutdown.")
    if post:
        required = ("Post-process preflight passed: 18 cases.",
                    "Post-process pass reuse passed.",
                    f"RenderPipeline completed {frames} frames; requesting normal shutdown.")
    if any(marker not in text for marker in required):
        raise RuntimeError(f"Missing acceptance marker: {path}")
    if not stack and not post and (text.count("Pipeline asset rejection passed:") != 18 or
            text.count("Pipeline asset switch passed at frame") != switches or
            text.count("recorded outgoing target submitted.") != (switches + 1) // 2 or
            text.count("recorded outgoing target discarded/reset.") != switches // 2 or
            text.count("Pipeline asset pixels passed:") != pixels):
        raise RuntimeError(f"Incomplete rollback/switch/pixel checks: {path}")
    if stack and (text.count("Camera stack binding rejection passed:") != 3 or
                  text.count("Camera stack live binding passed at frame") != 2 or
                  text.count("Camera stack replacement passed at frame") != switches or
                  text.count("Camera stack pixels passed:") != pixels):
        raise RuntimeError(f"Incomplete camera stack checks: {path}")
    if post:
        replacements = sum(tick % 8 != 5 for tick in range(1, switches + 1))
        if (text.count("Post-process live rejection passed:") != 4 or
                text.count("Post-process live settings passed at frame") != 2 or
                text.count("Post-process replacement passed at frame") != replacements or
                text.count("Post-process pixels passed:") != pixels):
            raise RuntimeError(f"Incomplete post-process checks: {path}")
    diagnostics = ("D3D12 Error:", "D3D12 Warning:", "Validation layer:", "GL_INVALID_", "validation layers unavailable")
    if any(marker in text for marker in diagnostics):
        raise RuntimeError(f"Native validation diagnostic: {path}")
    if api == 3 and "Starting D3D12 debug info queue logger thread." not in text:
        raise RuntimeError(f"D3D12 debug logger was not enabled: {path}")
    duration = time.monotonic() - started
    steady = [row for row in samples if 3 <= row[0] <= duration - 1 and row[1] > 0]
    trend = ""
    if len(steady) > 1:
        trend = f"; handles {steady[0][1]}->{steady[-1][1]}, private {steady[0][2]}->{steady[-1][2]}"
    print(f"PASS {directory.name}: frames={frames}, switches={replacements if post else switches}, pixels={pixels}, "
          f"mode={'post' if post else 'stack' if stack else 'asset'}, exit=0, {duration:.1f}s{trend}", flush=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("executable", type=Path)
    parser.add_argument("--logs", required=True, type=Path)
    parser.add_argument("--frames", type=int, default=120)
    modes = parser.add_mutually_exclusive_group()
    modes.add_argument("--stack", action="store_true", help="Run P2 camera composition instead of P1 asset mode")
    modes.add_argument("--post", action="store_true", help="Run P3 HDR composition and ordered effects")
    parser.add_argument("--backend", choices=("OpenGL", "Vulkan", "D3D12", "VulkanDedicated"))
    args = parser.parse_args()
    executable = args.executable.resolve()
    if not executable.is_file() or not 120 <= args.frames <= 6000 or args.frames % 15:
        raise RuntimeError("Use a native executable and 120..6000 frames divisible by 15")
    for name, api, dedicated in (("OpenGL",1,False),("Vulkan",2,False),("D3D12",3,False),("VulkanDedicated",2,True)):
        if not args.backend or name == args.backend:
            run(executable,args.logs.resolve() / name,args.frames,api,dedicated,args.stack,args.post)


if __name__ == "__main__":
    main()
