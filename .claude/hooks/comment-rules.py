#!/usr/bin/env python3
"""Comment-style guard for agent edits to engine source.

Claude Code runs this after every Edit and Write; it acts only on C and C++
files under code/. When the edit touches comment lines it returns the comment
sections of code/AGENTS.md as additional context (in full once per session, a
short pointer afterwards), so the rules are present at the moment comments
are being written rather than only at the start of the session. When the
comment lines the edit added break those rules — new Westwood/doxygen
decoration, /// prose without XML tags, narration of the edit, verbosity, a
comment that restates the adjacent code — it reports the findings back to the
agent as a blocking reason so they are fixed immediately. A per-session store
keeps a finding from repeating once it has been reported.

code/AGENTS.md stays the single owner of the rules; the reminder is extracted
from it at run time and the findings point back to it.

Any failure exits quietly. A missing reminder or check is better than a
blocked edit.
"""

import difflib
import hashlib
import json
import re
import subprocess
import sys
import tempfile
from pathlib import Path

SECTIONS = ("Comments",)
SUFFIXES = (".h", ".hpp", ".hh", ".inl", ".c", ".cpp", ".cc")

ROOT = Path(__file__).resolve().parent.parent.parent
RULES = ROOT / "code" / "AGENTS.md"

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
    "the", "a", "an", "to", "of", "and", "or", "for", "is", "are", "this",
    "that", "it", "its", "in", "on", "at", "with", "we", "if", "then",
    "when", "as", "be", "by", "from", "into", "not", "do", "does", "up",
}


def resolve_target(payload):
    path = (payload.get("tool_input") or {}).get("file_path")
    if not path:
        return None
    try:
        resolved = Path(path).resolve()
    except OSError:
        return None
    if resolved.suffix.lower() not in SUFFIXES:
        return None
    if (ROOT / "code") not in resolved.parents:
        return None
    return resolved


def git_show_head(path):
    try:
        rel = path.relative_to(ROOT).as_posix()
        out = subprocess.run(
            ["git", "-C", str(ROOT), "show", f"HEAD:{rel}"],
            capture_output=True,
            timeout=10,
        )
        if out.returncode == 0:
            return out.stdout.decode("utf-8", errors="replace")
    except Exception:
        pass
    return ""


def edit_pairs(payload, target):
    """Yield (old_text, new_text) for each edit the tool call performed."""
    ti = payload.get("tool_input") or {}
    if payload.get("tool_name") == "Write":
        return [(git_show_head(target), ti.get("content", ""))]
    if isinstance(ti.get("edits"), list):
        return [
            (e.get("old_string", ""), e.get("new_string", ""))
            for e in ti["edits"]
            if isinstance(e, dict)
        ]
    return [(ti.get("old_string", ""), ti.get("new_string", ""))]


def added_mask(old_text, new_text):
    """Return (new_lines, per-line added flags, removed old lines)."""
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


def trailing_comment(i, raw):
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
    return {"kind": kind, "parts": [(i, raw, text)], "code_idx": i, "code": code}


def comment_segments(lines):
    """Group the comment lines of `lines` into segments.

    Each segment: kind (line, xml, block, trailing, trailing_xml), parts as
    (index, raw line, comment text), code_idx of the adjacent code line when
    one directly follows, and for trailing kinds the code on the same line.
    """
    segs = []
    group = None
    block = None

    def close_group(code_idx=None):
        nonlocal group
        if group is not None:
            group["code_idx"] = code_idx
            segs.append(group)
            group = None

    for i, raw in enumerate(lines):
        s = raw.strip()
        if block is not None:
            block["parts"].append((i, raw, s.lstrip("*").strip()))
            if "*/" in s:
                segs.append(block)
                block = None
            continue
        if s.startswith("///"):
            kind, text = "xml", s[3:].strip()
        elif s.startswith("//"):
            kind, text = "line", s[2:].strip()
        elif s.startswith("/*"):
            close_group()
            body = s[2:].split("*/")[0].strip()
            block = {"kind": "block", "parts": [(i, raw, body)], "code_idx": None}
            if "*/" in s[2:]:
                segs.append(block)
                block = None
            continue
        else:
            if s == "":
                close_group()
            else:
                close_group(code_idx=i)
                seg = trailing_comment(i, raw)
                if seg is not None:
                    segs.append(seg)
            continue
        if group is not None and group["kind"] == kind and group["parts"][-1][0] == i - 1:
            group["parts"].append((i, raw, text))
        else:
            close_group()
            group = {"kind": kind, "parts": [(i, raw, text)], "code_idx": None}
    close_group()
    if block is not None:
        segs.append(block)
    return segs


def code_words(line):
    words = set()
    for ident in re.findall(r"[A-Za-z_][A-Za-z0-9_]*", line):
        for part in re.split(r"_+", ident):
            for m in re.findall(r"[A-Z]+(?![a-z])|[A-Z][a-z]*|[a-z]+|\d+", part):
                words.add(m.lower())
    return words


def comment_tokens(text):
    return [
        w.lower()
        for w in re.findall(r"[A-Za-z]{2,}", text)
        if w.lower() not in STOPWORDS
    ]


def check_segment(seg, lines, mask):
    added_parts = [p for p in seg["parts"] if mask[p[0]]]
    if not added_parts:
        return []
    fully = all(mask[p[0]] for p in seg["parts"])
    text_added = " ".join(p[2] for p in added_parts).strip()
    text_all = " ".join(p[2] for p in seg["parts"]).strip()
    excerpt = re.sub(r"\s+", " ", re.sub(r"[*/]{2,}", " ", text_added or text_all)).strip()[:70]
    kind = seg["kind"]
    msgs = []

    if kind == "block" and any(
        WESTWOOD_OPEN.match(raw) or WESTWOOD_CONT.match(raw)
        for _i, raw, _t in added_parts
    ):
        msgs.append(
            "uses Westwood/doxygen `**` decoration; new prose takes `//` or a plain `/* */` block"
        )
    if kind == "xml" and fully and not XML_TAG.search(text_all):
        msgs.append("`///` without XML doc tags; ordinary prose takes `//`")
    if kind == "trailing_xml":
        msgs.append("new trailing `///`; a trailing comment takes `//`")
    if NARRATION.search(text_added) or LEADING_EDIT_VERB.match(text_added):
        msgs.append(
            "reads as narration of the edit; describe the code as it stands, or delete it"
        )
    if fully:
        words = len(text_all.split())
        sentences = len(re.findall(r"[.!?]+(?:\s|$)", ABBREVIATIONS.sub("", text_all)))
        if kind == "xml":
            if words > 90:
                msgs.append("long for XML documentation; tighten each element to a concise sentence")
        elif words > 32 or sentences > 2:
            msgs.append("too long; one concise sentence usually suffices")
    if fully and kind in ("line", "trailing") and len(seg["parts"]) <= 2:
        code_text = seg.get("code")
        if code_text is None and seg.get("code_idx") is not None:
            code_text = lines[seg["code_idx"]]
        if code_text:
            tokens = comment_tokens(text_all)
            words = code_words(code_text)
            if len(tokens) >= 2 and sum(1 for t in tokens if t in words) / len(tokens) >= 0.75:
                msgs.append("restates the adjacent code; delete it")
    return [(excerpt, m) for m in msgs]


def seg_key(target, seg):
    text = " ".join(p[2] for p in seg["parts"])
    base = f"{target.as_posix().lower()}|{seg['kind']}|{text}"
    return hashlib.sha1(base.encode("utf-8", "replace")).hexdigest()


def is_commentish(line):
    s = line.strip()
    return s.startswith(("//", "/*", "* ", "*/", "**")) or s == "*"


def comment_sections(text):
    """Return the SECTIONS headings of code/AGENTS.md and their bodies."""
    kept = []
    keeping = False
    for line in text.splitlines():
        if line.startswith("## "):
            keeping = line[3:].strip() in SECTIONS
        elif line.startswith("# "):
            keeping = False
        if keeping:
            kept.append(line)
    return "\n".join(kept).strip()


def session_store(payload):
    sid = re.sub(r"[^A-Za-z0-9_-]", "_", str(payload.get("session_id") or "default"))[:64]
    path = Path(tempfile.gettempdir()) / f"opents-comment-guard-{sid}.json"
    try:
        state = json.loads(path.read_text())
        if not isinstance(state, dict):
            state = {}
    except Exception:
        state = {}
    state.setdefault("reported", [])
    state.setdefault("rules_sent", False)
    return path, state


def display_path(target):
    try:
        return target.relative_to(ROOT).as_posix()
    except ValueError:
        return str(target)


def rules_reminder(rules_sent):
    if rules_sent:
        return (
            "This edit touches comment lines; the comment rules in code/AGENTS.md"
            " (\"## Comments\") govern it even where the surrounding lines"
            " disagree."
        )
    try:
        rules = comment_sections(RULES.read_text(encoding="utf-8"))
    except OSError:
        return None
    if not rules:
        return None
    return (
        "This edit touches comment lines. The comment rules for code/, from"
        " code/AGENTS.md, govern it even where the surrounding lines"
        " disagree.\n\n" + rules
    )


def run(payload, target):
    store_path, state = session_store(payload)
    seen = set(state["reported"])
    touched = False
    findings = []
    for old, new in edit_pairs(payload, target):
        lines, mask, removed = added_mask(old, new)
        segs = comment_segments(lines)
        if not touched:
            touched = any(
                mask[p[0]] for seg in segs for p in seg["parts"]
            ) or any(is_commentish(line) for line in removed)
        if not any(mask):
            continue
        for seg in segs:
            msgs = check_segment(seg, lines, mask)
            if not msgs:
                continue
            key = seg_key(target, seg)
            if key in seen:
                continue
            seen.add(key)
            findings.extend(msgs)
    if not touched and not findings:
        return None

    result = {"suppressOutput": True}
    if findings:
        bullets = "\n".join(f'- "{ex}" — {m}' for ex, m in findings[:8])
        result["decision"] = "block"
        result["reason"] = (
            f"Comment check on {display_path(target)}: comments this edit added break"
            " the comment rules of code/AGENTS.md.\n"
            f"{bullets}\n"
            "Most edits need no comment. Fix each flagged comment now, usually by"
            " deleting it or cutting it to one concise sentence stating only what the"
            " code cannot show. Keep it unchanged only if it is genuinely required as"
            " written."
        )
    if touched:
        reminder = rules_reminder(state["rules_sent"])
        if reminder:
            result["hookSpecificOutput"] = {
                "hookEventName": "PostToolUse",
                "additionalContext": reminder,
            }
            state["rules_sent"] = True

    state["reported"] = sorted(seen)
    try:
        store_path.write_text(json.dumps(state))
    except OSError:
        pass
    return result


def main():
    try:
        payload = json.load(sys.stdin)
    except (json.JSONDecodeError, ValueError):
        return
    target = resolve_target(payload)
    if target is None:
        return
    try:
        result = run(payload, target)
    except Exception:
        return
    if result is not None:
        json.dump(result, sys.stdout)


if __name__ == "__main__":
    main()
