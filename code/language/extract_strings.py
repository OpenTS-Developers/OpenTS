#!/usr/bin/env python3
"""Write a C++ string table from language.h and the STRINGTABLE entries in language.rc."""

import re
import sys


def load_ids(path):
    ids = {}
    pattern = re.compile(r"^#define\s+(\w+)\s+(\d+)\s*$")
    with open(path, encoding="utf-8") as handle:
        for line in handle:
            match = pattern.match(line.strip())
            if match:
                ids[match.group(1)] = int(match.group(2))
    return ids


def unescape(text):
    output = []
    index = 0
    while index < len(text):
        character = text[index]
        if character != "\\":
            output.append(character)
            index += 1
            continue
        index += 1
        if index >= len(text):
            break
        escaped = text[index]
        index += 1
        if escaped == "n":
            output.append("\n")
        elif escaped == "r":
            output.append("\r")
        elif escaped == "t":
            output.append("\t")
        elif escaped in ('"', "\\"):
            output.append(escaped)
        elif escaped == "x":
            digits = ""
            while index < len(text) and text[index] in "0123456789abcdefABCDEF" and len(digits) < 2:
                digits += text[index]
                index += 1
            output.append(chr(int(digits, 16)) if digits else "x")
        else:
            output.append(escaped)
    return "".join(output)


def load_strings(path, ids):
    with open(path, encoding="utf-8") as handle:
        text = handle.read()
    entries = {}
    pattern = re.compile(r"\b([A-Za-z_][A-Za-z0-9_]*)\b\s*\"((?:\\.|[^\"\\])*)\"", re.S)
    for name, literal in pattern.findall(text):
        if name not in ids:
            continue
        entries[ids[name]] = unescape(literal)
    return entries


def cpp_literal(text):
    encoded = text.encode("utf-8")
    parts = ['"']
    for byte in encoded:
        character = chr(byte)
        if character == "\\":
            parts.append("\\\\")
        elif character == '"':
            parts.append('\\"')
        elif character == "\n":
            parts.append("\\n")
        elif character == "\r":
            parts.append("\\r")
        elif character == "\t":
            parts.append("\\t")
        elif 32 <= byte < 127:
            parts.append(character)
        else:
            # Octal escapes end after three digits, unlike \x, which would swallow a
            # following hex digit and corrupt the byte that comes after.
            parts.append("\\%03o" % byte)
    parts.append('"')
    return "".join(parts)


def write_table(path, entries):
    lines = [
        "/* Generated from code/language/language.rc. LoadString reads this table. */",
        "",
        "struct LanguageString",
        "{",
        "\tunsigned Id;",
        "\tchar const * Text;",
        "};",
        "",
        "static LanguageString const LanguageStrings[] = {",
    ]
    for ident in sorted(entries):
        lines.append("\t{ %d, %s }," % (ident, cpp_literal(entries[ident])))
    lines.extend([
        "};",
        "",
        "int Language_Load_String(unsigned id, char * buffer, int size)",
        "{",
        "\tif (buffer == nullptr || size <= 1) return(0);",
        "\tint low = 0;",
        "\tint high = (int)(sizeof(LanguageStrings) / sizeof(LanguageStrings[0])) - 1;",
        "\twhile (low <= high) {",
        "\t\tint mid = low + (high - low) / 2;",
        "\t\tunsigned const found = LanguageStrings[mid].Id;",
        "\t\tif (found == id) {",
        "\t\t\tchar const * text = LanguageStrings[mid].Text;",
        "\t\t\tint length = 0;",
        "\t\t\twhile (text[length] != '\\0') length++;",
        "\t\t\tif (length >= size) length = size - 1;",
        "\t\t\tfor (int index = 0; index < length; index++) buffer[index] = text[index];",
        "\t\t\tbuffer[length] = '\\0';",
        "\t\t\treturn(length);",
        "\t\t}",
        "\t\tif (found < id) low = mid + 1;",
        "\t\telse high = mid - 1;",
        "\t}",
        "\treturn(0);",
        "}",
        "",
    ])
    with open(path, "w", encoding="utf-8") as handle:
        handle.write("\n".join(lines))


def main():
    if len(sys.argv) != 4:
        sys.stderr.write("usage: extract_strings.py language.h language.rc output.cpp\n")
        return 1
    ids = load_ids(sys.argv[1])
    entries = load_strings(sys.argv[2], ids)
    if not entries:
        sys.stderr.write("no strings were read\n")
        return 1
    write_table(sys.argv[3], entries)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
