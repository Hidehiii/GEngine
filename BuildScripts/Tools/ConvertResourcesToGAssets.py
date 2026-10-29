#!/usr/bin/env python3
import argparse
import base64
from pathlib import Path


RESOURCE_EXTENSIONS = {
    ".png": "texture",
    ".jpg": "texture",
    ".jpeg": "texture",
    ".tga": "texture",
    ".ppm": "texture",
    ".fbx": "mesh",
    ".ttf": "font",
    ".otf": "font",
    ".shader": "shader",
    ".glsl": "shader",
    ".glvk": "shader",
    ".hlsli": "shader-include",
}

TEXT_EXTENSIONS = {".shader", ".glsl", ".glvk", ".hlsli"}
SCENE_EXTENSIONS = {".gescene"}
EXCLUDED_PARTS = {"build", "dist", "GeneratedFiles", "Cache", "Logs", "mono", "Intermediates"}
TEXT_FILE_EXTENSIONS = {
    ".cpp", ".h", ".hpp", ".lua", ".json", ".md", ".txt", ".ini",
    ".gproject", ".gitattributes", ".cmake", ".bat", ".shader", ".glsl",
    ".glvk", ".hlsli", ".gescene", ".gasset", ".gscene",
}


def is_excluded(path):
    return any(part.lower() in EXCLUDED_PARTS for part in path.parts)


def runtime_resource_roots(repo_root):
    roots = []
    for top in ("Example", "Sandbox"):
        base = repo_root / top
        if not base.exists():
            continue
        for directory in base.rglob("Assets"):
            if not is_excluded(directory):
                roots.append(directory)
        for directory in base.rglob("Resources"):
            if not is_excluded(directory):
                roots.append(directory)
    worker_assets = repo_root / "GEngine-Editor" / "Worker" / "Assets"
    if worker_assets.exists():
        roots.append(worker_assets)
    editor_assets = repo_root / "GEngine-Editor" / "Assets"
    if editor_assets.exists():
        roots.append(editor_assets)
    return roots


def project_relative(path, resource_root):
    current = resource_root
    while current.name not in {"Assets", "Resources"}:
        current = current.parent
        if current == current.parent:
            return None
    return path.relative_to(current.parent)


def project_root(resource_root):
    current = resource_root
    while current.name not in {"Assets", "Resources"}:
        current = current.parent
        if current == current.parent:
            return None
    return current.parent


def replacements_for(path, resource_root, target):
    relative = project_relative(path, resource_root)
    target_relative = project_relative(target, resource_root)
    result = []
    pairs = [(path, target)]
    if relative is not None and target_relative is not None:
        pairs.append((relative, target_relative))
    for old, new in pairs:
        old_text = old.as_posix()
        new_text = new.as_posix()
        result.extend([
            (old_text, new_text),
            (old_text.replace("/", "\\"), new_text.replace("/", "\\")),
        ])
    return result


def yaml_text_payload(payload):
    text = payload.replace("\r\n", "\n").replace("\r", "\n")
    if text and not text.endswith("\n"):
        text += "\n"
    lines = [line.rstrip().replace("\t", "    ") for line in text.split("\n")]
    if lines and lines[-1] == "":
        lines.pop()
    body = "\n".join("    " + line for line in lines)
    return "payload: |\n" + body + "\n" if body else "payload: |\n"


def yaml_binary_payload(payload):
    encoded = base64.b64encode(payload).decode("ascii")
    lines = [encoded[index:index + 76] for index in range(0, len(encoded), 76)]
    if not lines:
        return "payload: !!binary \"\"\n"
    return "payload: !!binary |\n" + "\n".join("    " + line for line in lines) + "\n"


def write_gasset(source, target, asset_type, source_format):
    payload = source.read_bytes()
    if source.suffix.lower() in TEXT_EXTENSIONS:
        try:
            text = payload.decode("utf-8")
        except UnicodeDecodeError as error:
            raise ValueError(f"Text resource is not UTF-8: {source}") from error
        payload_section = yaml_text_payload(text)
    else:
        payload_section = yaml_binary_payload(payload)

    document = (
        "schemaVersion: 1\n"
        f"type: {asset_type}\n"
        f"format: {source_format}\n"
        f"payloadEncoding: {'text' if source.suffix.lower() in TEXT_EXTENSIONS else 'base64'}\n"
        + payload_section
    )
    target.write_text(document, encoding="utf-8", newline="\n")
    source.unlink()


def update_text_references(root, replacements):
    ordered = sorted(replacements, key=lambda item: len(item[0]), reverse=True)
    for path in root.rglob("*"):
        if not path.is_file() or is_excluded(path) or path.suffix.lower() not in TEXT_FILE_EXTENSIONS:
            continue
        if "GEngine-ThirdParty" in path.parts or "TODO" in path.parts:
            continue
        try:
            text = path.read_text(encoding="utf-8")
        except (UnicodeDecodeError, OSError):
            continue
        updated = text
        for old, new in ordered:
            updated = updated.replace(old, new)
        if updated != text:
            path.write_text(updated, encoding="utf-8", newline="")


def migrate(repo_root):
    replacements_by_root = {}
    pending = []
    reserved_targets = set()
    for resource_root in runtime_resource_roots(repo_root):
        resource_candidates = []
        for source in sorted(resource_root.rglob("*")):
            if not source.is_file() or is_excluded(source):
                continue
            suffix = source.suffix.lower()
            if suffix in SCENE_EXTENSIONS:
                target = source.with_suffix(".gscene")
                if target.exists():
                    raise FileExistsError(f"Scene target already exists: {target}")
                replacements_by_root.setdefault(project_root(resource_root), []).extend(
                    replacements_for(source, resource_root, target))
                pending.append((source, target, "scene", None, None))
            elif suffix in RESOURCE_EXTENSIONS:
                resource_candidates.append((source, RESOURCE_EXTENSIONS[suffix], suffix[1:]))
        grouped = {}
        for source, asset_type, source_format in resource_candidates:
            grouped.setdefault(source.with_suffix(".gasset"), []).append((source, asset_type, source_format))
        for default_target, group in grouped.items():
            for source, asset_type, source_format in group:
                target = default_target
                if len(group) > 1:
                    base = source.stem + "_" + asset_type.replace("-", "_")
                    target = source.with_name(base + ".gasset")
                    if target in reserved_targets or target.exists():
                        target = source.with_name(base + "_" + source_format + ".gasset")
                if target in reserved_targets or target.exists():
                    raise FileExistsError(f"Asset target already exists: {target}")
                reserved_targets.add(target)
                replacements_by_root.setdefault(project_root(resource_root), []).extend(
                    replacements_for(source, resource_root, target))
                pending.append((source, target, "asset", asset_type, source_format))
    for root, replacements in replacements_by_root.items():
        update_text_references(root, replacements)
    migrated_assets = 0
    migrated_scenes = 0
    for source, target, kind, asset_type, source_format in pending:
        if kind == "scene":
            target.write_bytes(source.read_bytes())
            source.unlink()
            migrated_scenes += 1
        else:
            if target.exists():
                raise FileExistsError(f"Asset target already exists: {target}")
            write_gasset(source, target, asset_type, source_format)
            migrated_assets += 1
    print(f"Migrated {migrated_assets} resources to .gasset and {migrated_scenes} scenes to .gscene.")


def repair_references(repo_root):
    replacements_by_root = {}
    for resource_root in runtime_resource_roots(repo_root):
        root = project_root(resource_root)
        if root is None:
            continue
        for target in sorted(resource_root.rglob("*")):
            if not target.is_file() or is_excluded(target):
                continue
            if target.suffix.lower() == ".gscene":
                old = target.with_suffix(".GEScene")
            elif target.suffix.lower() == ".gasset":
                text = target.read_text(encoding="utf-8")
                format_match = next(
                    (line for line in text.splitlines() if line.startswith("format: ")), None)
                if format_match is None:
                    continue
                old = target.with_suffix("." + format_match[len("format: "):].strip())
            else:
                continue
            replacements = replacements_for(old, resource_root, target)
            absolute = target.as_posix()
            relative = project_relative(target, resource_root)
            if relative is not None:
                replacements.append((absolute, relative.as_posix()))
                replacements.append((absolute.replace("/", "\\"), relative.as_posix().replace("/", "\\")))
            replacements_by_root.setdefault(root, []).extend(replacements)
    for root, replacements in replacements_by_root.items():
        update_text_references(root, replacements)
    print("Repaired project-relative GAsset references.")


def main():
    parser = argparse.ArgumentParser(description="Convert GEngine runtime resources to YAML GAsset files.")
    parser.add_argument("--root", type=Path, default=Path(__file__).resolve().parents[2])
    parser.add_argument("--repair-references", action="store_true")
    arguments = parser.parse_args()
    root = arguments.root.resolve()
    if arguments.repair_references:
        repair_references(root)
    else:
        migrate(root)


if __name__ == "__main__":
    main()
