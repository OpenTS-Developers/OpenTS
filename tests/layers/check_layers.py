#!/usr/bin/env python3
"""Reports the engine's link-level references that point against the layer table.

Reads the symbol table of each engine object, finds every reference from one
source file to a symbol another source file defines, and reports the
references whose direction layers.toml does not allow. Given a baseline file,
it fails on any such reference the file does not list.
"""

from __future__ import annotations

import argparse
import struct
import sys
import tomllib
from collections import defaultdict
from dataclasses import dataclass, field
from pathlib import Path

HERE = Path(__file__).resolve().parent
SOURCE_ROOT = HERE.parent.parent / "code"

BIGOBJ_CLASS_ID = bytes.fromhex("c7a1bad1eebaa94baf20faf66aa4dcb8")
STORAGE_EXTERNAL = 2


class CheckError(Exception):
    pass


@dataclass
class ObjectSymbols:
    defined: set[str] = field(default_factory=set)
    undefined: set[str] = field(default_factory=set)


def read_object_symbols(data: bytes, name: str = "object") -> ObjectSymbols:
    """Returns the external symbols a COFF object defines and the ones it leaves undefined."""
    if len(data) < 20:
        raise CheckError(f"{name}: too short to be an object file")
    sig1, sig2 = struct.unpack_from("<HH", data, 0)
    if sig1 == 0 and sig2 == 0xFFFF:
        if len(data) < 56 or data[12:28] != BIGOBJ_CLASS_ID:
            raise CheckError(
                f"{name}: not a COFF object; objects built with whole program "
                "optimization carry no symbol table"
            )
        table, count = struct.unpack_from("<II", data, 48)
        record = struct.Struct("<8sIiHBB")
    else:
        table, count = struct.unpack_from("<II", data, 8)
        record = struct.Struct("<8sIhHBB")

    strings = table + count * record.size
    if strings > len(data):
        raise CheckError(f"{name}: symbol table runs past the end of the file")

    result = ObjectSymbols()
    index = 0
    while index < count:
        raw, value, section, _type, storage, aux = record.unpack_from(data, table + index * record.size)
        index += 1 + aux
        if storage != STORAGE_EXTERNAL:
            continue
        if raw[:4] == b"\0\0\0\0":
            start = strings + struct.unpack_from("<I", raw, 4)[0]
            end = data.index(b"\0", start)
            symbol = data[start:end].decode("latin-1")
        else:
            symbol = raw.rstrip(b"\0").decode("latin-1")
        if section > 0 or (section == 0 and value != 0):
            result.defined.add(symbol)
        elif section == 0:
            result.undefined.add(symbol)
    return result


@dataclass
class LayerTable:
    uses: dict[str, set[str]]
    file_layer: dict[str, str]
    directory_layer: dict[str, str]
    excluded: list[str]


def load_table(path: Path) -> LayerTable:
    with path.open("rb") as handle:
        raw = tomllib.load(handle)
    layers = raw.get("layers", {})
    uses = {name: set(body.get("uses", [])) for name, body in layers.items()}
    for name, allowed in uses.items():
        for other in allowed:
            if other not in uses:
                raise CheckError(f"{path.name}: layer '{name}' uses unknown layer '{other}'")
    file_layer: dict[str, str] = {}
    for name, body in layers.items():
        for stem in body.get("files", []):
            if stem in file_layer:
                raise CheckError(f"{path.name}: '{stem}' is in both '{file_layer[stem]}' and '{name}'")
            file_layer[stem] = name
    directory_layer = dict(raw.get("directories", {}))
    for directory, name in directory_layer.items():
        if name not in uses:
            raise CheckError(f"{path.name}: directory '{directory}' names unknown layer '{name}'")
    return LayerTable(uses, file_layer, directory_layer, list(raw.get("exclude", [])))


def assign_sources(table: LayerTable, source_root: Path) -> dict[str, str]:
    """Maps each engine source stem to its layer. Raises when the table and the sources disagree."""
    problems: list[str] = []
    result: dict[str, str] = {}
    seen: dict[str, str] = {}
    for path in sorted(source_root.rglob("*.cpp")):
        relative = path.relative_to(source_root).as_posix()
        if any(relative.startswith(prefix + "/") for prefix in table.excluded):
            continue
        stem = path.stem.lower()
        if stem in seen:
            problems.append(f"{relative} and {seen[stem]} share a name; the table cannot tell them apart")
            continue
        seen[stem] = relative
        by_directory = None
        parent = path.parent.relative_to(source_root).as_posix()
        while parent not in ("", "."):
            if parent in table.directory_layer:
                by_directory = table.directory_layer[parent]
                break
            parent = parent.rpartition("/")[0]
        by_file = table.file_layer.get(stem)
        if by_file:
            result[stem] = by_file
        elif by_directory:
            result[stem] = by_directory
        else:
            problems.append(f"{relative} has no layer; add '{stem}' to a files list in layers.toml")
    for stem, layer in sorted(table.file_layer.items()):
        if stem not in seen:
            problems.append(f"layers.toml lists '{stem}' under '{layer}', but no such source exists")
    if problems:
        raise CheckError("\n".join(problems))
    return result


def object_stem(path: Path) -> str | None:
    """Returns the source stem an object file was built from, or None for any other file."""
    name = path.name.lower()
    for suffix in (".cpp.obj", ".cpp.o", ".obj", ".o"):
        if name.endswith(suffix):
            return name[: -len(suffix)]
    return None


def find_edges(objects: dict[str, ObjectSymbols]) -> dict[tuple[str, str], set[str]]:
    """Returns {(from, to): symbols} for references to a symbol exactly one object defines.

    A symbol several objects define is an inline function or a template instance, which says
    nothing about which source file depends on which.
    """
    definers: dict[str, list[str]] = defaultdict(list)
    for stem, symbols in objects.items():
        for symbol in symbols.defined:
            definers[symbol].append(stem)
    edges: dict[tuple[str, str], set[str]] = defaultdict(set)
    for stem, symbols in objects.items():
        for symbol in symbols.undefined:
            owners = definers.get(symbol)
            if owners and len(owners) == 1 and owners[0] != stem:
                edges[stem, owners[0]].add(symbol)
    return edges


def find_violations(
    edges: dict[tuple[str, str], set[str]], layer_of: dict[str, str], table: LayerTable
) -> dict[str, set[str]]:
    """Returns {baseline line: symbols} for every reference the table does not allow."""
    violations: dict[str, set[str]] = {}
    for (source, target), symbols in edges.items():
        if source not in layer_of or target not in layer_of:
            continue
        a, b = layer_of[source], layer_of[target]
        if a != b and b not in table.uses[a]:
            violations[f"{a}/{source} -> {b}/{target}"] = symbols
    return violations


def read_baseline(path: Path) -> set[str]:
    if not path.exists():
        return set()
    lines = path.read_text(encoding="utf-8").splitlines()
    return {line.strip() for line in lines if line.strip() and not line.startswith("#")}


BASELINE_HEADER = """\
# References between engine source files that point against the layer order in
# layers.toml, one line per pair of files.
"""


def write_baseline(path: Path, lines: set[str]) -> None:
    path.write_text(BASELINE_HEADER + "".join(line + "\n" for line in sorted(lines)), encoding="utf-8", newline="\n")


def collect_objects(args: argparse.Namespace) -> list[Path]:
    paths: list[Path] = []
    if args.objects_list:
        text = Path(args.objects_list).read_text(encoding="utf-8")
        paths += [Path(line.strip()) for line in text.splitlines() if line.strip()]
    if args.objects:
        directory = Path(args.objects)
        if not directory.is_dir():
            raise CheckError(f"{directory} is not a directory")
        paths += sorted(directory.glob("*.obj")) + sorted(directory.glob("*.o"))
    if not paths:
        raise CheckError("no object files given; pass --objects <dir> or --objects-list <file>")
    return paths


def summarize(violations: dict[str, set[str]]) -> str:
    pairs: dict[str, int] = defaultdict(int)
    for line in violations:
        source, _, target = line.partition(" -> ")
        pairs[f"{source.split('/')[0]} -> {target.split('/')[0]}"] += 1
    return ", ".join(f"{pair} {count}" for pair, count in sorted(pairs.items(), key=lambda item: -item[1]))


def run(args: argparse.Namespace) -> int:
    table = load_table(Path(args.table))
    layer_of = assign_sources(table, Path(args.source_root))

    objects: dict[str, ObjectSymbols] = {}
    missing: list[str] = []
    for path in collect_objects(args):
        stem = object_stem(path)
        if stem not in layer_of:
            continue
        if not path.exists():
            missing.append(str(path))
            continue
        objects[stem] = read_object_symbols(path.read_bytes(), path.name)
    if missing:
        raise CheckError("object files not built yet:\n  " + "\n  ".join(missing[:10]))
    unbuilt = sorted(set(layer_of) - set(objects))
    if unbuilt:
        raise CheckError(
            "no object for: " + ", ".join(unbuilt[:10]) + ("..." if len(unbuilt) > 10 else "")
            + "\nBuild the engine before running the check."
        )

    violations = find_violations(find_edges(objects), layer_of, table)
    if not args.baseline:
        if args.update:
            raise CheckError("--update needs --baseline <file>")
        if args.list:
            for line in sorted(violations):
                print(line)
        print(f"{len(objects)} objects, {len(violations)} wrong-direction references: {summarize(violations)}")
        return 0
    baseline_path = Path(args.baseline)

    if args.update:
        write_baseline(baseline_path, set(violations))
        print(f"wrote {len(violations)} lines to {baseline_path}")
        return 0

    baseline = read_baseline(baseline_path)
    added = sorted(set(violations) - baseline)
    removed = sorted(baseline - set(violations))
    if args.summary:
        print(f"{len(objects)} objects, {len(violations)} wrong-direction references: {summarize(violations)}")

    for line in added:
        symbols = sorted(violations[line])
        shown = ", ".join(symbols[:3]) + (f" and {len(symbols) - 3} more" if len(symbols) > 3 else "")
        print(f"new: {line}\n     {shown}")
    if args.stale == "allow":
        removed = []
    for line in removed:
        print(f"gone: {line}")
    if added:
        print(
            f"\n{len(added)} new reference(s) point against the layer order. Remove the dependency, "
            "or move the file to the layer it belongs in."
        )
    if removed:
        print(f"\n{len(removed)} baseline line(s) no longer apply. Run the check with --update and commit the result.")
    return 1 if added or removed else 0


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--objects", help="directory holding the engine's object files")
    parser.add_argument("--objects-list", help="file listing one object path per line")
    parser.add_argument("--table", default=str(HERE / "layers.toml"))
    parser.add_argument("--baseline", help="file listing the accepted references; compare against it")
    parser.add_argument("--list", action="store_true", help="without a baseline, print every reference")
    parser.add_argument("--source-root", default=str(SOURCE_ROOT))
    parser.add_argument("--update", action="store_true", help="rewrite the baseline from the current objects")
    parser.add_argument(
        "--stale",
        choices=("fail", "allow"),
        default="fail",
        help="what to do when baseline lines no longer apply; an optimized build drops some references",
    )
    parser.add_argument("--summary", action="store_true", help="with a baseline, also print the count per layer pair")
    args = parser.parse_args(argv)
    try:
        return run(args)
    except CheckError as error:
        print(error, file=sys.stderr)
        return 2


if __name__ == "__main__":
    sys.exit(main())
