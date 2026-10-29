#!/usr/bin/env python3
"""instantiate.py <sdk> <template|auto> <dest> --name N [--target T]

Copy mcu/templates/<sdk>/<template>/ to <dest>, substituting {{NAME}}, {{TARGET}}, {{BOARD}},
{{SDK}}, {{MCU_ROOT}} and every key of template.toml [vars] (value for the chosen target, or a
plain string) in file contents and file names. template.toml itself is not copied.
Template "auto": the first minimal / minimal-* / *-minimal template (plain "minimal" first) whose targets include T
(without --target: "minimal", else the first such template, at its default_target); used by `mcu new`.
Refuses to overwrite: exits 2 if <dest> already exists. Run with host python: python3 -I instantiate.py ...
"""
import argparse
import os
import shutil
import sys
import tomllib

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(os.path.dirname(HERE))  # 32_01_mcu/ (templates/ lives here)
# the mcu tool dir = {{MCU_ROOT}} (SDK plugins under it): ROOT itself when reached through tool/templates
TOOL = ROOT if os.path.isfile(os.path.join(ROOT, "mcu")) else os.path.join(ROOT, "tool")


def load_toml(path: str) -> dict:
    with open(path, "rb") as handle:
        return tomllib.load(handle)


def load_vars(meta: dict, target: str, name: str) -> dict[str, str]:
    """Substitution table: the built-in placeholders plus template.toml [vars] resolved for the target."""
    values = {"NAME": name, "TARGET": target, "SDK": meta["sdk"], "MCU_ROOT": TOOL,
              "BOARD": (meta.get("boards") or {}).get(target, target)}
    for key, value in (meta.get("vars") or {}).items():
        if isinstance(value, dict):  # per-target value
            if target not in value:
                sys.exit(f"instantiate: var {key} has no value for target {target}")
            value = value[target]
        values[key] = str(value)
    return values


def is_minimal(name: str) -> bool:
    return name == "minimal" or name.startswith("minimal-") or name.endswith("-minimal")


def pick(sdk: str, target: str | None) -> str:
    """Name of the minimal template of <sdk> for <target> ("minimal" preferred), or exit with the candidates."""
    base = os.path.join(ROOT, "templates", sdk)
    entries = os.listdir(base) if os.path.isdir(base) else []
    names = sorted(n for n in entries if is_minimal(n) and os.path.isfile(os.path.join(base, n, "template.toml")))
    names.sort(key=lambda n: n != "minimal")
    for name in names:
        if target is None or target in load_toml(os.path.join(base, name, "template.toml")).get("targets", []):
            return name
    sys.exit(f"instantiate: no minimal template of {sdk} supports target {target!r} (templates: {names})")


def subst(text: str, values: dict[str, str]) -> str:
    for key, value in values.items():
        text = text.replace("{{" + key + "}}", value)
    return text


def copy_file(src_path: str, dst_path: str, values: dict[str, str]) -> None:
    """Copy one file, substituting placeholders in UTF-8 text; binary files are copied unchanged."""
    with open(src_path, "rb") as handle:
        raw = handle.read()
    try:
        text = raw.decode("utf-8")
    except UnicodeDecodeError:
        with open(dst_path, "wb") as handle:
            handle.write(raw)
    else:
        with open(dst_path, "w", encoding="utf-8") as handle:
            handle.write(subst(text, values))
    shutil.copymode(src_path, dst_path)


def copy_template(src: str, dest: str, values: dict[str, str]) -> None:
    for directory, _, files in os.walk(src):
        rel = os.path.relpath(directory, src)
        out_dir = dest if rel == "." else os.path.join(dest, subst(rel, values))
        os.makedirs(out_dir)
        for file_name in files:
            if rel == "." and file_name == "template.toml":
                continue
            copy_file(os.path.join(directory, file_name), os.path.join(out_dir, subst(file_name, values)), values)


def main() -> None:
    parser = argparse.ArgumentParser(description="Create a project from an mcu template (refuses to overwrite).")
    parser.add_argument("sdk", help="SDK name, e.g. stm32 (a folder under templates/)")
    parser.add_argument("template", help="template folder name, or 'auto' for the minimal template of the target")
    parser.add_argument("dest", help="new project directory (must not exist)")
    parser.add_argument("--name", required=True, help="project name substituted for {{NAME}}")
    parser.add_argument("--target", help="target/board (default: the template's default_target)")
    args = parser.parse_args()
    if args.template == "auto":
        args.template = pick(args.sdk, args.target)
    src = os.path.join(ROOT, "templates", args.sdk, args.template)
    meta_path = os.path.join(src, "template.toml")
    if not os.path.isfile(meta_path):
        sys.exit(f"instantiate: no template {args.sdk}/{args.template} ({meta_path})")
    meta = load_toml(meta_path)
    target = args.target or meta.get("default_target")
    if target not in meta.get("targets", []):
        sys.exit(f"instantiate: target {target!r} not in {meta.get('targets')}")
    if os.path.lexists(args.dest):
        print(f"instantiate: refusing to overwrite existing {args.dest}", file=sys.stderr)
        sys.exit(2)
    copy_template(src, args.dest, load_vars(meta, target, args.name))
    print(f"instantiate: {args.sdk}/{args.template} -> {args.dest} (name={args.name}, target={target})")


if __name__ == "__main__":
    main()
