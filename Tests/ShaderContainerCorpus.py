"""Parse tracked shader payloads with the production CPU parser (requires PyYAML)."""
import argparse
import json
from pathlib import Path
import subprocess
import yaml


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("executable", type=Path)
    parser.add_argument("--logs", type=Path, required=True)
    args = parser.parse_args()
    executable = args.executable.resolve()
    directory = args.logs.resolve()
    directory.mkdir(parents=True, exist_ok=True)
    root = Path(__file__).resolve().parents[1]
    paths = subprocess.run(["git", "ls-files", "-z", "--", "*.gasset", "*.shader",
                            ":!GEngine/GEngine-ThirdParty/**"], cwd=root,
                           check=True, capture_output=True).stdout.decode("utf-8").split("\0")
    manifest = []
    for fixture in ("PipelineParserInvalidState.gasset", "PipelineParserInvalidStructure.gasset"):
        name = "Example/RenderPipeline/Assets/Shaders/" + fixture
        if name not in paths and (root / name).is_file():
            paths.append(name)
    for name in filter(None, paths):
        path = root / name
        if path.suffix == ".gasset":
            with path.open("rb") as source:
                if b"shader" not in source.read(512):
                    continue
            asset = yaml.safe_load(path.read_text(encoding="utf-8-sig"))
            if not isinstance(asset, dict) or asset.get("type") != "shader":
                continue
            payload = asset["payload"]
            if asset.get("payloadEncoding") != "text" or not isinstance(payload, str):
                raise RuntimeError(f"Unsupported shader corpus encoding: {name}")
        else:
            payload = path.read_text(encoding="utf-8-sig")
        # Registered old GLSL containers/fragments have never used Shader/Pass/Program.
        legacy = payload.lstrip().startswith("#Name") or payload.lstrip().startswith("GE_MATERIAL_DEFINE;")
        reject = legacy or path.name in ("PipelineParserInvalidState.gasset", "PipelineParserInvalidStructure.gasset")
        target = directory / f"{len(manifest):04d}.shader"
        target.write_bytes(payload.encode("utf-8"))
        manifest.append({"source": name, "payload": str(target), "reject": reject})
    if not manifest:
        raise RuntimeError("No tracked shader payloads found")
    (directory / "manifest.json").write_text(json.dumps(manifest, indent=2), encoding="utf-8")
    arguments = [str(executable)]
    for item in manifest:
        if item["reject"]:
            arguments.append("--reject-file")
        arguments.append(item["payload"])
    result = subprocess.run(arguments,
                            cwd=root, capture_output=True, timeout=60)
    (directory / "corpus.log").write_bytes(result.stdout + result.stderr)
    if result.returncode:
        raise RuntimeError(f"Corpus failed; see {directory / 'corpus.log'} and manifest.json")
    rejected = sum(item["reject"] for item in manifest)
    print(f"PASS {len(manifest) - rejected} current containers, {rejected} explicit legacy/invalid rejections; {directory / 'corpus.log'}")


if __name__ == "__main__":
    main()
