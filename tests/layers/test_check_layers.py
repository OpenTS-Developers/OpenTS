#!/usr/bin/env python3
"""Tests for the layers check."""

from __future__ import annotations

import contextlib
import importlib.util
import io
import struct
import sys
import tempfile
import unittest
from pathlib import Path

SCRIPT = Path(__file__).with_name("check_layers.py")
SPEC = importlib.util.spec_from_file_location("opents_check_layers", SCRIPT)
assert SPEC is not None and SPEC.loader is not None
CHECK = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = CHECK
SPEC.loader.exec_module(CHECK)

TABLE = """\
exclude = ["language"]

[directories]
ui = "ui"

[layers.core]
uses = []
files = ["crc"]

[layers.sim]
uses = ["core"]
files = ["house", "techno"]

[layers.ui]
uses = ["core", "sim"]
files = []
"""


def make_object(defined: list[str], undefined: list[str], *, bigobj: bool = False, statics: list[str] = ()) -> bytes:
    """Builds a COFF object holding only a symbol table."""
    strings = b""
    records = b""
    layout = "<8sIiHBB" if bigobj else "<8sIhHBB"
    entries = [(name, 1, 2) for name in defined] + [(name, 0, 2) for name in undefined]
    entries += [(name, 1, 3) for name in statics]
    for name, section, storage in entries:
        encoded = name.encode("latin-1")
        if len(encoded) <= 8:
            raw = encoded.ljust(8, b"\0")
        else:
            raw = struct.pack("<II", 0, 4 + len(strings))
            strings += encoded + b"\0"
        records += struct.pack(layout, raw, 0, section, 0, storage, 0)
    table = struct.pack("<I", 4 + len(strings)) + strings
    if bigobj:
        header = struct.pack("<HHHHI", 0, 0xFFFF, 2, 0x8664, 0) + CHECK.BIGOBJ_CLASS_ID
        header += struct.pack("<IIIIIII", 0, 0, 0, 0, 0, 56, len(entries))
    else:
        header = struct.pack("<HHIIIHH", 0x8664, 0, 0, 20, len(entries), 0, 0)
    return header + records + table


class ObjectReaderTests(unittest.TestCase):
    def test_reads_short_and_long_names(self) -> None:
        data = make_object(["short", "?Long_Name@House@@QAEXXZ"], ["?Other@@YAXXZ"])
        symbols = CHECK.read_object_symbols(data)
        self.assertEqual(symbols.defined, {"short", "?Long_Name@House@@QAEXXZ"})
        self.assertEqual(symbols.undefined, {"?Other@@YAXXZ"})

    def test_reads_the_bigobj_layout(self) -> None:
        data = make_object(["?Defined@@YAXXZ"], ["?Wanted@@YAXXZ"], bigobj=True)
        symbols = CHECK.read_object_symbols(data)
        self.assertEqual(symbols.defined, {"?Defined@@YAXXZ"})
        self.assertEqual(symbols.undefined, {"?Wanted@@YAXXZ"})

    def test_skips_static_symbols(self) -> None:
        symbols = CHECK.read_object_symbols(make_object(["kept"], [], statics=["hidden"]))
        self.assertEqual(symbols.defined, {"kept"})

    def test_rejects_an_object_without_a_symbol_table(self) -> None:
        data = struct.pack("<HHHHI", 0, 0xFFFF, 1, 0x8664, 0) + bytes(44)
        with self.assertRaises(CHECK.CheckError):
            CHECK.read_object_symbols(data)


class EdgeTests(unittest.TestCase):
    def test_ignores_symbols_several_objects_define(self) -> None:
        objects = {
            "a": CHECK.ObjectSymbols({"inline"}, {"only_b"}),
            "b": CHECK.ObjectSymbols({"inline", "only_b"}, set()),
            "c": CHECK.ObjectSymbols(set(), {"inline", "missing"}),
        }
        self.assertEqual(dict(CHECK.find_edges(objects)), {("a", "b"): {"only_b"}})


class CheckTests(unittest.TestCase):
    def setUp(self) -> None:
        self.temp = tempfile.TemporaryDirectory()
        self.root = Path(self.temp.name)
        self.code = self.root / "code"
        self.objects = self.root / "objects"
        (self.code / "ui").mkdir(parents=True)
        (self.code / "language").mkdir()
        self.objects.mkdir()
        (self.root / "layers.toml").write_text(TABLE, encoding="utf-8")
        for name in ("crc.cpp", "house.cpp", "techno.cpp", "ui/uimenu.cpp", "language/skipped.cpp"):
            (self.code / name).write_text("", encoding="utf-8")
        self.write("crc", ["?Crc@@YAXXZ"], [])
        self.write("house", ["?House@@YAXXZ"], ["?Crc@@YAXXZ"])
        self.write("techno", ["?Techno@@YAXXZ"], ["?House@@YAXXZ"])
        self.write("uimenu", ["?Menu@@YAXXZ"], ["?House@@YAXXZ"])

    def tearDown(self) -> None:
        self.temp.cleanup()

    def write(self, stem: str, defined: list[str], undefined: list[str]) -> None:
        (self.objects / f"{stem}.obj").write_bytes(make_object(defined, undefined))

    def run_check(self, *extra: str) -> tuple[int, str]:
        output = io.StringIO()
        with contextlib.redirect_stdout(output), contextlib.redirect_stderr(output):
            code = CHECK.main([
                "--objects", str(self.objects),
                "--table", str(self.root / "layers.toml"),
                "--baseline", str(self.root / "baseline.txt"),
                "--source-root", str(self.code),
                *extra,
            ])
        return code, output.getvalue()

    def test_allowed_references_pass(self) -> None:
        self.assertEqual(self.run_check(), (0, ""))

    def test_new_wrong_direction_reference_fails(self) -> None:
        self.write("house", ["?House@@YAXXZ"], ["?Menu@@YAXXZ"])
        code, output = self.run_check()
        self.assertEqual(code, 1)
        self.assertIn("new: sim/house -> ui/uimenu", output)
        self.assertIn("?Menu@@YAXXZ", output)

    def test_baseline_reference_passes_and_its_removal_fails(self) -> None:
        self.write("house", ["?House@@YAXXZ"], ["?Menu@@YAXXZ"])
        self.assertEqual(self.run_check("--update")[0], 0)
        self.assertEqual(self.run_check()[0], 0)

        self.write("house", ["?House@@YAXXZ"], [])
        code, output = self.run_check()
        self.assertEqual(code, 1)
        self.assertIn("gone: sim/house -> ui/uimenu", output)
        self.assertEqual(self.run_check("--stale", "allow")[0], 0)

    def test_source_without_a_layer_fails(self) -> None:
        (self.code / "radar.cpp").write_text("", encoding="utf-8")
        code, output = self.run_check()
        self.assertEqual(code, 2)
        self.assertIn("radar.cpp has no layer", output)

    def test_table_entry_without_a_source_fails(self) -> None:
        (self.code / "techno.cpp").unlink()
        code, output = self.run_check()
        self.assertEqual(code, 2)
        self.assertIn("lists 'techno'", output)

    def test_source_without_an_object_fails(self) -> None:
        (self.objects / "techno.obj").unlink()
        code, output = self.run_check()
        self.assertEqual(code, 2)
        self.assertIn("no object for: techno", output)

    def test_object_without_a_source_is_ignored(self) -> None:
        self.write("removed", [], ["?Menu@@YAXXZ"])
        self.assertEqual(self.run_check()[0], 0)

    def test_listed_file_that_is_not_an_object_is_ignored(self) -> None:
        listing = self.root / "objects.txt"
        paths = [self.objects / "house.res", *sorted(self.objects.glob("*.obj"))]
        (self.objects / "house.res").write_bytes(b"not an object")
        listing.write_text("\n".join(str(path) for path in paths), encoding="utf-8")
        output = io.StringIO()
        with contextlib.redirect_stdout(output), contextlib.redirect_stderr(output):
            code = CHECK.main([
                "--objects-list", str(listing),
                "--table", str(self.root / "layers.toml"),
                "--baseline", str(self.root / "baseline.txt"),
                "--source-root", str(self.code),
            ])
        self.assertEqual((code, output.getvalue()), (0, ""))

    def test_without_a_baseline_it_reports_and_passes(self) -> None:
        self.write("house", ["?House@@YAXXZ"], ["?Menu@@YAXXZ"])
        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            code = CHECK.main([
                "--objects", str(self.objects),
                "--table", str(self.root / "layers.toml"),
                "--source-root", str(self.code),
                "--list",
            ])
        self.assertEqual(code, 0)
        self.assertIn("sim/house -> ui/uimenu", output.getvalue())
        self.assertIn("1 wrong-direction references: sim -> ui 1", output.getvalue())

    def test_files_entry_overrides_the_directory(self) -> None:
        table = TABLE.replace('files = ["crc"]', 'files = ["crc", "uimenu"]')
        (self.root / "layers.toml").write_text(table, encoding="utf-8")
        code, output = self.run_check()
        self.assertEqual(code, 1)
        self.assertIn("new: core/uimenu -> sim/house", output)


if __name__ == "__main__":
    unittest.main()
