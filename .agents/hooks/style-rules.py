#!/usr/bin/env python3
"""Give editing agents the repository's prose and comment rules.

Claude Code and Codex both call this script with a hook payload on stdin.  A
Markdown edit gets the ``## Writing prose`` section of the root ``AGENTS.md``
as nonblocking context.  A C or C++ edit under ``code/`` gets the ``##
Comments`` section of ``code/AGENTS.md`` whenever it touches comments.  The
small set of mechanical comment checks may also block the tool result.

Edit and apply-patch payloads contain both sides of a change, so only newly
added comment text is checked.  A Claude ``Write`` payload does not always
include the previous content.  In that case the hook still sends a reminder
when the written file contains comments, but deliberately skips blocking
checks instead of comparing the file with Git HEAD and misclassifying an
already-dirty working tree.

Runtime failures are fail-open.  ``--check`` is the strict mode for tests: it
fails if a canonical rule section is missing or renamed.
"""

from __future__ import annotations

import argparse
import difflib
import json
import re
import sys
from dataclasses import dataclass, field
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PROSE_RULES = (Path("AGENTS.md"), "Writing prose")
COMMENT_RULES = (Path("code/AGENTS.md"), "Comments")

MARKDOWN_SUFFIXES = {".md", ".mdx"}
SOURCE_SUFFIXES = {".h", ".hpp", ".hh", ".inl", ".c", ".cpp", ".cc"}

WESTWOOD_OPEN = re.compile(r"^\s*/\*\*")
WESTWOOD_CONT = re.compile(r"^\s*\*\*")
XML_TAG = re.compile(r"<[A-Za-z/]")
NARRATION = re.compile(
    r"(?i)\b(?:previously|instead of|no longer|used to\b"
    r"|renamed (?:from|to)\b"
    r"|was (?:added|removed|renamed|moved|changed|replaced)\b"
    r"|has been (?:added|removed|renamed|moved|changed|updated|replaced)\b"
    r"|this (?:change|edit|commit|fix|patch)\b"
    r"|the (?:old|original|previous) (?:code|version|implementation|behaviou?r)\b"
    r")|(?<![Ff]or )\bnow\b"
)
LEADING_EDIT_VERB = re.compile(
    r"(?i)^(?:added|removed|deleted|changed|updated|fixed|moved|renamed|refactored)\b"
)
ABBREVIATIONS = re.compile(r"(?i)\b(?:e\.g\.|i\.e\.|etc\.|vs\.|cf\.)")
STOPWORDS = {
    "the", "a", "an", "to", "of", "and", "or", "for", "is", "are",
    "this", "that", "it", "its", "in", "on", "at", "with", "we", "if",
    "then", "when", "as", "be", "by", "from", "into", "not", "do",
    "does", "up",
}


@dataclass
class EditPair:
    old: str
    new: str
    checkable: bool = True


@dataclass
class FileEdit:
    path: Path
    pairs: list[EditPair] = field(default_factory=list)


def _inside(path: Path, directory: Path) -> bool:
    try:
        path.relative_to(directory)
    except ValueError:
        return False
    return True


def _resolve_path(raw: str, root: Path, cwd: str | None = None) -> Path | None:
    if not raw:
        return None
    raw = raw.strip().strip('"')
    candidate = Path(raw)
    if not candidate.is_absolute():
        base = Path(cwd).resolve() if cwd else root
        candidate = base / candidate
    try:
        resolved = candidate.resolve()
    except OSError:
        return None
    return resolved if _inside(resolved, root.resolve()) else None


def extract_rule_section(root: Path, relative: Path, heading: str) -> str | None:
    """Return one level-two section, including any nested headings."""
    try:
        lines = (root / relative).read_text(encoding="utf-8").splitlines()
    except OSError:
        return None

    wanted = f"## {heading}"
    start = next((i for i, line in enumerate(lines) if line.strip() == wanted), None)
    if start is None:
        return None
    end = len(lines)
    for i in range(start + 1, len(lines)):
        if re.match(r"^#{1,2}\s+", lines[i]):
            end = i
            break
    return "\n".join(lines[start:end]).strip()


def validate_rule_sections(root: Path = ROOT) -> list[str]:
    errors = []
    for relative, heading in (PROSE_RULES, COMMENT_RULES):
        if extract_rule_section(root, relative, heading) is None:
            errors.append(f"{relative.as_posix()} is missing `## {heading}`")
    return errors


def _rule_context(root: Path, rule: tuple[Path, str]) -> str:
    relative, heading = rule
    section = extract_rule_section(root, relative, heading)
    if section is not None:
        return section
    return (
        f"Style reminder unavailable: {relative.as_posix()} is missing the required "
        f"`## {heading}` section. Follow the rest of the repository instructions; "
        "the hook is fail-open, but the hook tests require this section to be restored."
    )


def prose_context(root: Path, paths: list[Path]) -> str:
    context = (
        "This edit changes Markdown. Apply the repository's writing rules; the "
        "examples show direction, not text to copy.\n\n"
        + _rule_context(root, PROSE_RULES)
    )
    manual = (root / "manual").resolve()
    if any(_inside(path, manual) for path in paths):
        context += (
            "\n\nFor files under `manual/`, also follow `manual/AGENTS.md` and the "
            "relevant manual authoring or style guide. Do not apply a root-guide "
            "shortcut to a published manual page."
        )
    return context


def comment_context(root: Path) -> str:
    return (
        "This edit touches C or C++ comments. Apply the `code/` comment rules "
        "even where inherited surrounding comments use another form.\n\n"
        + _rule_context(root, COMMENT_RULES)
    )


def combined_context(root: Path) -> str:
    return (
        "Keep these OpenTS writing rules in context for delegated or continued "
        "work.\n\n"
        + _rule_context(root, PROSE_RULES)
        + "\n\n"
        + _rule_context(root, COMMENT_RULES)
    )


def parse_apply_patch(command: str, root: Path, cwd: str | None = None) -> list[FileEdit]:
    """Extract per-file hunk pairs from an apply_patch command."""
    edits: list[FileEdit] = []
    current: FileEdit | None = None
    old_lines: list[str] = []
    new_lines: list[str] = []
    in_hunk = False

    def flush_hunk() -> None:
        nonlocal old_lines, new_lines, in_hunk
        if current is not None and in_hunk:
            current.pairs.append(EditPair("\n".join(old_lines), "\n".join(new_lines)))
        old_lines = []
        new_lines = []
        in_hunk = False

    def flush_file() -> None:
        nonlocal current
        flush_hunk()
        if current is not None:
            edits.append(current)
        current = None

    header = re.compile(r"^\*\*\* (?:Update|Add|Delete) File:\s*(.+?)\s*$")
    for line in command.splitlines():
        match = header.match(line)
        if match:
            flush_file()
            path = _resolve_path(match.group(1), root, cwd)
            current = FileEdit(path) if path is not None else None
            continue
        if line == "*** End Patch":
            flush_file()
            continue
        if current is None:
            continue
        if line.startswith("@@"):
            flush_hunk()
            in_hunk = True
            continue
        if line.startswith("*** "):
            continue
        if not in_hunk:
            # Add/Delete patches may omit an explicit @@ marker.
            if line.startswith(("+", "-")):
                in_hunk = True
            else:
                continue
        if line.startswith("+"):
            new_lines.append(line[1:])
        elif line.startswith("-"):
            old_lines.append(line[1:])
        elif line.startswith(" "):
            old_lines.append(line[1:])
            new_lines.append(line[1:])
        elif line == "":
            old_lines.append("")
            new_lines.append("")
    flush_file()
    return edits


def _first_string(mapping: object, names: tuple[str, ...]) -> str | None:
    if not isinstance(mapping, dict):
        return None
    for name in names:
        value = mapping.get(name)
        if isinstance(value, str):
            return value
    return None


def payload_edits(payload: dict, root: Path = ROOT) -> list[FileEdit]:
    tool_name = str(payload.get("tool_name") or "")
    tool_input = payload.get("tool_input") or {}
    if not isinstance(tool_input, dict):
        return []
    cwd = payload.get("cwd") if isinstance(payload.get("cwd"), str) else None

    if tool_name.lower() == "apply_patch":
        command = tool_input.get("command")
        return parse_apply_patch(command, root, cwd) if isinstance(command, str) else []

    path = _resolve_path(str(tool_input.get("file_path") or ""), root, cwd)
    if path is None:
        return []
    edit = FileEdit(path)

    if isinstance(tool_input.get("edits"), list):
        for item in tool_input["edits"]:
            if isinstance(item, dict):
                edit.pairs.append(
                    EditPair(str(item.get("old_string") or ""), str(item.get("new_string") or ""))
                )
        return [edit]

    new = _first_string(tool_input, ("new_string", "content", "new_content"))
    old = _first_string(tool_input, ("old_string", "old_content", "previous_content"))
    if old is None:
        old = _first_string(
            payload.get("tool_response"),
            ("old_string", "old_content", "previous_content", "original_content"),
        )

    if new is None:
        return [edit]
    # Edit normally supplies old_string. Write may not; such a replacement is
    # useful for reminder detection but unsafe for blocking checks.
    edit.pairs.append(EditPair(old or "", new, checkable=old is not None))
    return [edit]


def added_mask(old_text: str, new_text: str) -> tuple[list[str], list[bool], list[str]]:
    old_lines = old_text.splitlines()
    new_lines = new_text.splitlines()
    mask = [False] * len(new_lines)
    removed = []
    matcher = difflib.SequenceMatcher(None, old_lines, new_lines, autojunk=False)
    for tag, i1, i2, j1, j2 in matcher.get_opcodes():
        if tag in ("insert", "replace"):
            for j in range(j1, j2):
                mask[j] = True
        if tag in ("delete", "replace"):
            removed.extend(old_lines[i1:i2])
    return new_lines, mask, removed


def trailing_comment(index: int, raw: str) -> dict | None:
    pos = 0
    while True:
        pos = raw.find("//", pos)
        if pos < 0:
            return None
        if pos > 0 and raw[pos - 1] == ":":
            pos += 2
            continue
        if raw[:pos].count('"') % 2 == 0:
            break
        pos += 2
    code = raw[:pos].strip()
    if not code:
        return None
    kind = "trailing_xml" if raw[pos:pos + 3] == "///" else "trailing"
    text = raw[pos:].lstrip("/").strip()
    return {"kind": kind, "parts": [(index, raw, text)], "code_idx": index, "code": code}


def comment_segments(lines: list[str]) -> list[dict]:
    segments = []
    group = None
    block = None

    def close_group(code_idx: int | None = None) -> None:
        nonlocal group
        if group is not None:
            group["code_idx"] = code_idx
            segments.append(group)
            group = None

    for index, raw in enumerate(lines):
        stripped = raw.strip()
        if block is not None:
            block["parts"].append((index, raw, stripped.lstrip("*").strip()))
            if "*/" in stripped:
                segments.append(block)
                block = None
            continue
        if stripped.startswith("///"):
            kind, text = "xml", stripped[3:].strip()
        elif stripped.startswith("//"):
            kind, text = "line", stripped[2:].strip()
        elif stripped.startswith("/*"):
            close_group()
            body = stripped[2:].split("*/")[0].strip()
            block = {"kind": "block", "parts": [(index, raw, body)], "code_idx": None}
            if "*/" in stripped[2:]:
                segments.append(block)
                block = None
            continue
        else:
            if stripped == "":
                close_group()
            else:
                close_group(code_idx=index)
                segment = trailing_comment(index, raw)
                if segment is not None:
                    segments.append(segment)
            continue
        if group is not None and group["kind"] == kind and group["parts"][-1][0] == index - 1:
            group["parts"].append((index, raw, text))
        else:
            close_group()
            group = {"kind": kind, "parts": [(index, raw, text)], "code_idx": None}
    close_group()
    if block is not None:
        segments.append(block)
    return segments


def code_words(line: str) -> set[str]:
    words = set()
    for identifier in re.findall(r"[A-Za-z_][A-Za-z0-9_]*", line):
        for part in re.split(r"_+", identifier):
            words.update(
                match.lower()
                for match in re.findall(r"[A-Z]+(?![a-z])|[A-Z][a-z]*|[a-z]+|\d+", part)
            )
    return words


def comment_tokens(text: str) -> list[str]:
    return [
        word.lower()
        for word in re.findall(r"[A-Za-z]{2,}", text)
        if word.lower() not in STOPWORDS
    ]


def check_segment(segment: dict, lines: list[str], mask: list[bool]) -> list[tuple[str, str]]:
    added_parts = [part for part in segment["parts"] if mask[part[0]]]
    if not added_parts:
        return []
    fully_added = all(mask[part[0]] for part in segment["parts"])
    added_text = " ".join(part[2] for part in added_parts).strip()
    all_text = " ".join(part[2] for part in segment["parts"]).strip()
    excerpt = re.sub(r"\s+", " ", re.sub(r"[*/]{2,}", " ", added_text or all_text)).strip()[:70]
    kind = segment["kind"]
    messages = []

    if kind == "block" and any(
        WESTWOOD_OPEN.match(raw) or WESTWOOD_CONT.match(raw)
        for _index, raw, _text in added_parts
    ):
        messages.append(
            "uses Westwood/doxygen `**` decoration; new prose takes `//` or a plain `/* */` block"
        )
    if kind == "xml" and fully_added and not XML_TAG.search(all_text):
        messages.append("uses `///` without XML tags; ordinary prose takes `//`")
    if kind == "trailing_xml":
        messages.append("uses new trailing `///`; trailing prose takes `//`")
    if NARRATION.search(added_text) or LEADING_EDIT_VERB.match(added_text):
        messages.append("narrates the edit; describe the code as it stands, or delete the comment")
    if fully_added:
        words = len(all_text.split())
        sentences = len(re.findall(r"[.!?]+(?:\s|$)", ABBREVIATIONS.sub("", all_text)))
        if kind == "xml":
            if words > 90:
                messages.append("is long for XML documentation; tighten each element")
        elif words > 32 or sentences > 2:
            messages.append("is too long; one concise sentence usually suffices")
    if fully_added and kind in ("line", "trailing") and len(segment["parts"]) <= 2:
        code_text = segment.get("code")
        if code_text is None and segment.get("code_idx") is not None:
            code_text = lines[segment["code_idx"]]
        if code_text:
            tokens = comment_tokens(all_text)
            words = code_words(code_text)
            if len(tokens) >= 2 and sum(token in words for token in tokens) / len(tokens) >= 0.75:
                messages.append("restates the adjacent code; delete it")
    return [(excerpt, message) for message in messages]


def _line_has_comment(line: str) -> bool:
    stripped = line.strip()
    if stripped.startswith(("//", "/*", "* ", "*/", "**")) or stripped == "*":
        return True
    return trailing_comment(0, line) is not None or "/*" in line or "*/" in line


def inspect_source_edit(edit: FileEdit) -> tuple[bool, list[tuple[str, str]]]:
    touched = False
    findings = []
    for pair in edit.pairs:
        if not pair.checkable:
            touched = touched or any(_line_has_comment(line) for line in pair.new.splitlines())
            continue
        lines, mask, removed = added_mask(pair.old, pair.new)
        segments = comment_segments(lines)
        touched = touched or any(
            mask[part[0]] for segment in segments for part in segment["parts"]
        ) or any(_line_has_comment(line) for line in removed)
        for segment in segments:
            findings.extend(check_segment(segment, lines, mask))
    return touched, findings


def _display_path(path: Path, root: Path) -> str:
    try:
        return path.relative_to(root).as_posix()
    except ValueError:
        return str(path)


def post_tool_result(payload: dict, root: Path = ROOT) -> dict | None:
    edits = payload_edits(payload, root)
    if not edits:
        return None

    markdown = [edit.path for edit in edits if edit.path.suffix.lower() in MARKDOWN_SUFFIXES]
    code_root = (root / "code").resolve()
    comment_paths = []
    findings = []
    for edit in edits:
        if edit.path.suffix.lower() not in SOURCE_SUFFIXES or not _inside(edit.path, code_root):
            continue
        touched, file_findings = inspect_source_edit(edit)
        if touched:
            comment_paths.append(edit.path)
        findings.extend((_display_path(edit.path, root), excerpt, message) for excerpt, message in file_findings)

    contexts = []
    if markdown:
        contexts.append(prose_context(root, markdown))
    if comment_paths:
        contexts.append(comment_context(root))
    if not contexts and not findings:
        return None

    result = {}
    if contexts:
        result["hookSpecificOutput"] = {
            "hookEventName": "PostToolUse",
            "additionalContext": "\n\n".join(contexts),
        }
    if findings:
        bullets = "\n".join(
            f'- {path}: "{excerpt}" — {message}' for path, excerpt, message in findings[:8]
        )
        result["decision"] = "block"
        result["reason"] = (
            "Comments added by this edit break the objective checks derived from "
            f"`code/AGENTS.md`.\n{bullets}\nFix each flagged comment before continuing."
        )
    return result


def handle_payload(payload: dict, root: Path = ROOT) -> dict | None:
    event = payload.get("hook_event_name")
    if event == "PostToolUse":
        return post_tool_result(payload, root)
    if event == "SubagentStart":
        return {
            "hookSpecificOutput": {
                "hookEventName": "SubagentStart",
                "additionalContext": combined_context(root),
            }
        }
    if event == "SessionStart" and payload.get("source") == "compact":
        return {
            "hookSpecificOutput": {
                "hookEventName": "SessionStart",
                "additionalContext": combined_context(root),
            }
        }
    return None


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(add_help=False)
    parser.add_argument("--check", action="store_true")
    args, _unknown = parser.parse_known_args(argv)
    if args.check:
        errors = validate_rule_sections(ROOT)
        if errors:
            print("\n".join(errors), file=sys.stderr)
            return 1
        return 0

    try:
        payload = json.load(sys.stdin)
        result = handle_payload(payload, ROOT)
    except Exception:
        # Hook bugs must not prevent source or documentation edits.
        return 0
    if result is not None:
        json.dump(result, sys.stdout)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
