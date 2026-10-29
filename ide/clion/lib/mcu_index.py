"""Turn a build's compiler invocations into a CMake "index" target for CLion.

Make-driven SDKs (ch32fun, py32f0-template, bouffalo, k230 RT-Smart apps, arduino-cli) do not
describe their sources to CMake. To make CLion's code insight see the exact flags the SDK uses,
the project's CMakeLists runs this script at configure time. It either
  * runs the SDK's make in dry-run mode (make -n -B -w ...) and parses the compiler lines, or
  * reads a compile_commands.json the SDK tool wrote (arduino-cli, bouffalo's inner CMake),
and writes a .cmake file that defines an OBJECT library (EXCLUDE_FROM_ALL, never built) whose
sources carry the per-file compile options. The real firmware is still built by the SDK's make.

Run with the host python in isolated mode (python3 -I). Standard library only.
"""
import argparse
import json
import os
import re
import shlex
import subprocess
import sys

SRC_EXT = (".c", ".cc", ".cpp", ".cxx", ".c++", ".ino")
TAKES_ARG = {"-o", "-MF", "-MT", "-MQ", "-x", "-T", "-L", "-l", "-Xlinker", "-Xassembler", "-u", "-e"}
PATH_ARG = {"-I", "-isystem", "-iquote", "-idirafter", "-include", "-imacros"}


def classify(tokens, cwd):
    """Return (sources, flags) of one compiler command line (tokens without the compiler)."""
    sources, flags = [], []
    i = 0
    while i < len(tokens):
        t = tokens[i]
        nxt = tokens[i + 1] if i + 1 < len(tokens) else None
        if t in PATH_ARG and nxt is not None:
            flags += [t, os.path.realpath(os.path.join(cwd, nxt))]
            i += 2
            continue
        for p in ("-I", "-isystem", "-iquote", "-idirafter"):
            if t.startswith(p) and len(t) > len(p) and not t.startswith("-include"):
                flags.append(p + os.path.realpath(os.path.join(cwd, t[len(p):])))
                break
        else:
            if t in TAKES_ARG:
                i += 2
                continue
            if t.startswith(("-D", "-U", "-std=", "-m", "-f", "-O", "-g", "-nostdinc", "--sysroot")) \
                    or (t.startswith("-W") and not t.startswith(("-Wl,", "-Wa,", "-Wp,"))):
                if not t.startswith(("-MD", "-MMD", "-MP")):
                    flags.append(t)
            elif not t.startswith("-") and t.lower().endswith(SRC_EXT):
                sources.append(os.path.realpath(os.path.join(cwd, t)))
        i += 1
    return sources, flags


def from_make(cmd, cwd, compiler_re, env):
    out = subprocess.run(cmd, cwd=cwd, env=env, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                         text=True, timeout=600)
    entries, dirs = [], [cwd]
    enter = re.compile(r"make(?:\[\d+\])?: Entering directory ['`](.*)'")
    leave = re.compile(r"make(?:\[\d+\])?: Leaving directory")
    for line in out.stdout.splitlines():
        m = enter.search(line)
        if m:
            dirs.append(m.group(1))
            continue
        if leave.search(line):
            if len(dirs) > 1:
                dirs.pop()
            continue
        try:
            toks = shlex.split(line)
        except ValueError:
            continue
        # a recipe may be "cd x && gcc ..." or "@gcc ..."; find the compiler token
        for k, tok in enumerate(toks):
            if re.search(compiler_re, os.path.basename(tok.lstrip("@"))):
                entries.append((toks[k + 1:], dirs[-1]))
                break
    if not entries:
        sys.stderr.write("mcu_index: no compiler lines found in dry run (exit %d):\n%s\n"
                         % (out.returncode, out.stdout[-2000:]))
    return entries


def from_compdb(path, compiler_re):
    with open(path) as f:
        db = json.load(f)
    entries = []
    for e in db:
        toks = e.get("arguments") or shlex.split(e["command"])
        if toks and re.search(compiler_re, os.path.basename(toks[0])):
            toks = toks[1:]
        # the file itself is in the command line; classify() collects it
        entries.append((toks, e.get("directory", os.path.dirname(path))))
    return entries


def cmake_quote(s):
    return '"' + s.replace("\\", "\\\\").replace('"', '\\"').replace("$", "\\$").replace(";", "\\;") + '"'


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--out", required=True, help="cmake file to write")
    ap.add_argument("--target", default="firmware-index")
    ap.add_argument("--compiler", default=r"(gcc|g\+\+|cc|c\+\+)$", help="regex on the compiler basename")
    ap.add_argument("--cwd", default=os.getcwd())
    ap.add_argument("--compdb", help="read this compile_commands.json instead of running a dry run")
    ap.add_argument("--map", action="append", default=[],
                    help="SRC=WRAPPER: index SRC through a generated wrapper TU (e.g. Blink.ino.cpp=Blink.ino)")
    ap.add_argument("--exclude", action="append", default=[], help="regex of sources to leave out")
    ap.add_argument("cmd", nargs="*", help="dry-run command (after --)")
    a = ap.parse_args()

    if a.compdb:
        entries = from_compdb(a.compdb, a.compiler)
    else:
        entries = from_make(a.cmd, a.cwd, a.compiler, os.environ.copy())

    per_src = {}
    for toks, cwd in entries:
        srcs, flags = classify(toks, cwd)
        for s in srcs:
            if any(re.search(x, s) for x in a.exclude):
                continue
            per_src.setdefault(s, flags)

    # --map: replace a generated source (arduino's Blink.ino.cpp) by a wrapper that #includes the
    # user's file, so CLion resolves the user's file in the right context
    outdir = os.path.dirname(os.path.abspath(a.out))
    for m in a.map:
        gen_re, user_file = m.split("=", 1)
        for s in list(per_src):
            if re.search(gen_re, s):
                flags = per_src.pop(s)
                w = os.path.join(outdir, "index-" + os.path.basename(user_file) + ".cpp")
                with open(w, "w") as f:
                    f.write("// generated by mcu/clion/lib/mcu_index.py: gives %s its compile context\n"
                            "#include <Arduino.h>\n#include \"%s\"\n" % (user_file, user_file))
                per_src[w] = flags

    srcs = [s for s in sorted(per_src) if os.path.exists(s)]
    with open(a.out, "w") as f:
        f.write("# generated by mcu/clion/lib/mcu_index.py at configure time; do not edit\n")
        if not srcs:
            f.write("message(WARNING \"mcu_index: no sources found for %s\")\n" % a.target)
            return 0
        f.write("add_library(%s OBJECT EXCLUDE_FROM_ALL\n" % a.target)
        for s in srcs:
            f.write("  %s\n" % cmake_quote(s))
        f.write(")\n")
        for s in srcs:
            f.write("set_source_files_properties(%s TARGET_DIRECTORY %s PROPERTIES COMPILE_OPTIONS %s)\n"
                    % (cmake_quote(s), a.target, cmake_quote(";".join(per_src[s]))))
    print("mcu_index: %d sources -> %s (target %s)" % (len(srcs), a.out, a.target))
    return 0


if __name__ == "__main__":
    sys.exit(main())
