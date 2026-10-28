"""Verify editor project settings remain compatible with the native export reader."""
import argparse
import json
from pathlib import Path
import subprocess


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("tool", type=Path)
    parser.add_argument("--logs", type=Path, required=True)
    args = parser.parse_args()
    root = args.logs.resolve()
    root.mkdir(parents=True, exist_ok=True)
    valid = {"schemaVersion": 1, "name": "SettingsProbe"}
    cases = [("legacy", valid, True),
             ("default", dict(valid, settings={"defaultScene": "Assets/Scenes/场景.GEScene"}), True),
             ("empty", dict(valid, settings={"defaultScene": ""}), True),
             ("braces-in-path", dict(valid, settings={"defaultScene": "Assets/Scenes/a}b.GEScene"}), True),
             ("bad-type", dict(valid, settings={"defaultScene": 12}), False),
             ("unknown", dict(valid, settings={"unknown": "x"}), False),
             ("nested", dict(valid, settings={"defaultScene": {"nested": "x"}}), False),
             ("bad-settings", dict(valid, settings="bad"), False),
             ("packaging", dict(valid, packaging={}), False)]
    for name, metadata, accepted in cases:
        directory = root / name
        directory.mkdir(exist_ok=True)
        project = directory / "Probe.gproject"
        project.write_text(json.dumps(metadata, ensure_ascii=False), encoding="utf-8")
        result = subprocess.run([str(args.tool.resolve()), "export-project", "--project", str(project),
                                 "--output", str(root / (name + "-output"))], capture_output=True, timeout=30)
        output = (result.stdout + result.stderr).decode("utf-8", errors="replace")
        (directory / "result.log").write_text(output, encoding="utf-8")
        # Valid metadata reaches source validation; there is deliberately no src directory.
        if result.returncode == 0 or ("Project src/ directory is missing." in output) != accepted:
            raise RuntimeError(f"{name}: unexpected metadata result: {output}")
        print(f"PASS: {name}")


if __name__ == "__main__":
    main()
