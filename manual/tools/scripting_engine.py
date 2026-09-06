"""
Map-scripting reference extractor: trigger actions, trigger events, and team
script missions, with the engine's own editor labels, descriptions, parameter
types and attach targets.

Sources (all in code/):
    taction.hh / taction.cpp   -- trigger actions: enum, _ActionText
                                  {Name, Description} table, Action_Needs(),
                                  Attaches_To()
    tevent.hh  / tevent.cpp    -- trigger events: enum, _EventText,
                                  Event_Needs(), Attaches_To()
    tmission.hh / tmission.cpp -- team script missions: enum, TMissions[]
                                  (editor names), TMissionsHelp[]
                                  (descriptions), TeamMission_Needs()

OpenTS realigned the historically desynchronized _ActionText tail with the
current enum. ACTION_LABEL_FIXES below is therefore empty; the mechanism stays
so a future desync can be hand-mapped again, and a count mismatch flags every
row needs_review.

Usage:
    python manual/tools/scripting.py --report
    python manual/tools/scripting.py --out <file.yaml>
"""

import os
import re

import yaml

CODE_DIR = os.path.normpath(
    os.path.join(os.path.dirname(__file__), "..", "..", "code"))


# ---------------------------------------------------------------------------
# The hand-verified fix for the _ActionText tail (see module docstring).
# enum index -> table index to take the text from, or None for "no text".
# ---------------------------------------------------------------------------

ACTION_LABEL_FIXES = {}

# Table texts that correspond to no current action. Empty since the OpenTS
# realignment.
ACTION_ORPHAN_TEXTS = []


# ---------------------------------------------------------------------------
# Semantic parameters
# ---------------------------------------------------------------------------

# NeedType is shared by trigger actions, trigger events and team missions. Keep
# this mapping explicit: an engine addition must fail generation until its
# ordered, modder-facing payload has been reviewed.
NEED_PARAMETERS = {
    "NEED_NONE": [],
    "NEED_THEME": [{"name": "Theme", "type": "theme"}],
    "NEED_MOVIE": [{"name": "Movie", "type": "movie"}],
    "NEED_SOUND": [{"name": "Sound", "type": "sound"}],
    "NEED_SPEECH": [{"name": "EVA speech", "type": "eva-speech"}],
    "NEED_INFANTRY": [{"name": "Infantry type", "type": "infantry-type"}],
    "NEED_UNIT": [{"name": "Unit type", "type": "unit-type"}],
    "NEED_AIRCRAFT": [{"name": "Aircraft type", "type": "aircraft-type"}],
    "NEED_STRUCTURE": [{"name": "Building type", "type": "building-type"}],
    "NEED_WAYPOINT": [{"name": "Waypoint", "type": "waypoint"}],
    "NEED_NUMBER": [{"name": "Number", "type": "integer"}],
    "NEED_TRIGGER": [{"name": "Trigger", "type": "trigger"}],
    "NEED_TEAM": [{"name": "Team", "type": "team"}],
    "NEED_HOUSE": [{"name": "House", "type": "house"}],
    "NEED_TIME": [{"name": "Duration", "type": "frames"}],
    "NEED_QUARRY": [{"name": "Target category", "type": "quarry"}],
    "NEED_BOOL": [{"name": "Boolean", "type": "boolean"}],
    "NEED_SPECIAL": [{"name": "Superweapon", "type": "superweapon"}],
    "NEED_MISSION": [{"name": "Mission", "type": "mission"}],
    "NEED_HEX_NUMBER": [{"name": "Cell offset", "type": "hex-number"}],
    "NEED_SCRIPT": [{"name": "Script", "type": "script"}],
    "NEED_RECT": [{"name": "Rectangle", "type": "rectangle"}],
    "NEED_GLOBAL": [{"name": "Global variable", "type": "global-variable"}],
    "NEED_ANIM_AND_LOCATION": [
        {"name": "Animation", "type": "animation"},
        {"name": "Waypoint", "type": "waypoint"},
    ],
    "NEED_WEAPON_AND_LOCATION": [
        {"name": "Weapon", "type": "weapon"},
        {"name": "Waypoint", "type": "waypoint"},
    ],
    "NEED_LIGHT_BEHAVIOR": [
        {"name": "Spotlight behavior", "type": "spotlight-behavior"},
    ],
    "NEED_TAG": [{"name": "Tag", "type": "tag"}],
    "NEED_METEOR_AND_LOCATION": [
        {"name": "Meteor size", "type": "meteor-size"},
        {"name": "Waypoint", "type": "waypoint"},
    ],
    "NEED_EVENT_AND_LOCATION": [
        {"name": "Radar event", "type": "radar-event"},
        {"name": "Waypoint", "type": "waypoint"},
    ],
    "NEED_LOCAL": [{"name": "Local variable", "type": "local-variable"}],
    "NEED_SPEED_AND_LOCATION": [
        {"name": "Speed", "type": "camera-speed"},
        {"name": "Waypoint", "type": "waypoint"},
    ],
    "NEED_SHOWER_AND_LOCATION": [
        {"name": "Meteor shower", "type": "meteor-shower"},
        {"name": "Waypoint", "type": "waypoint"},
    ],
    "NEED_VELOCITY": [{"name": "Velocity", "type": "camera-speed"}],
    "NEED_FLOAT": [{"name": "Number", "type": "floating-point"}],
    "NEED_TEAM_AND_LOCATION": [
        {"name": "Team", "type": "team"},
        {"name": "Waypoint", "type": "waypoint"},
    ],
    "NEED_PARTICLE_AND_LOCATION": [
        {"name": "Particle system", "type": "particle-system"},
        {"name": "Waypoint", "type": "waypoint"},
    ],
    "NEED_BUILDING_ATTACK": [
        {"name": "Building type", "type": "building-type"},
        {"name": "Target property", "type": "target-property"},
    ],
    "NEED_SPLIT": [{"name": "Group", "type": "split-group"}],
    "NEED_SOUND_AND_LOCATION": [
        {"name": "Sound", "type": "sound"},
        {"name": "Waypoint", "type": "waypoint"},
    ],
    "NEED_TEAM_AND_TIME": [
        {"name": "Team", "type": "team"},
        {"name": "Duration", "type": "frames"},
    ],
    "NEED_ANIM": [
        {"name": "Animation", "type": "animation"},
        {"name": "Loop count", "type": "integer"},
    ],
    "NEED_TALK_BUBBLE": [{"name": "Talk bubble", "type": "talk-bubble"}],
    "NEED_HOUSE_AND_CREDITS": [
        {"name": "House", "type": "house"},
        {"name": "Credits", "type": "integer"},
    ],
    "NEED_STRUCTURE_PLACEMENT": [
        {"name": "House", "type": "house"},
        {"name": "Building type", "type": "building-type"},
        {"name": "Force placement", "type": "boolean"},
        {"name": "Waypoint", "type": "waypoint"},
    ],
}


# Scenario action/event rows contain fixed positional payloads, but publishing
# those raw formats as a separate reference made a simple editing task look far
# more complicated than it is. Keep the placement rules explicit here and emit
# one ready-to-read template with each generated record instead.
#
# Action tuple: (serialized need code, primary payload, rectangle fields,
# final waypoint/time payload). None means the writer's zero/empty sentinel.
ACTION_EXAMPLE_LAYOUTS = {
    "NEED_ANIM_AND_LOCATION": (0, "Animation", None, "Waypoint"),
    "NEED_BOOL": (0, "Boolean", None, None),
    "NEED_EVENT_AND_LOCATION": (0, "Radar event", None, "Waypoint"),
    "NEED_FLOAT": (0, "Number", None, None),
    "NEED_GLOBAL": (0, "Global variable", None, None),
    "NEED_HOUSE": (0, "House", None, None),
    "NEED_HOUSE_AND_CREDITS": (0, "House", ("Credits", None, None, None), None),
    "NEED_LIGHT_BEHAVIOR": (0, "Spotlight behavior", None, None),
    "NEED_LOCAL": (0, "Local variable", None, None),
    "NEED_METEOR_AND_LOCATION": (0, "Meteor size", None, "Waypoint"),
    "NEED_MISSION": (0, "Mission", None, None),
    "NEED_MOVIE": (0, "Movie", None, None),
    "NEED_NONE": (0, None, None, None),
    "NEED_NUMBER": (0, "Number", None, None),
    "NEED_PARTICLE_AND_LOCATION": (0, "Particle system", None, "Waypoint"),
    "NEED_QUARRY": (0, "Target category", None, None),
    "NEED_RECT": (0, None, ("Rectangle X", "Rectangle Y", "Rectangle width", "Rectangle height"), None),
    "NEED_SHOWER_AND_LOCATION": (0, "Meteor shower", None, "Waypoint"),
    "NEED_SOUND": (0, "Sound", None, None),
    "NEED_SOUND_AND_LOCATION": (0, "Sound", None, "Waypoint"),
    "NEED_SPECIAL": (0, "Superweapon", None, None),
    "NEED_SPEECH": (0, "EVA speech", None, None),
    "NEED_SPEED_AND_LOCATION": (0, "Speed", None, "Waypoint"),
    "NEED_STRUCTURE_PLACEMENT": (0, "House", ("Building type", "Force placement", None, None), "Waypoint"),
    "NEED_TAG": (3, "Tag", None, None),
    "NEED_TEAM": (1, "Team", None, None),
    "NEED_TEAM_AND_LOCATION": (1, "Team", None, "Waypoint"),
    "NEED_TEAM_AND_TIME": (4, "Team", None, "Duration"),
    "NEED_THEME": (0, "Theme", None, None),
    "NEED_TRIGGER": (2, "Trigger", None, None),
    "NEED_WAYPOINT": (0, None, None, "Waypoint"),
    "NEED_WEAPON_AND_LOCATION": (0, "Weapon", None, "Waypoint"),
}

# Event tuple: (serialized need code, payload). Only team references use the
# named-object representation; every other event stores a numeric value.
EVENT_EXAMPLE_LAYOUTS = {
    "NEED_AIRCRAFT": (0, "Aircraft type"),
    "NEED_GLOBAL": (0, "Global variable"),
    "NEED_HOUSE": (0, "House"),
    "NEED_INFANTRY": (0, "Infantry type"),
    "NEED_LOCAL": (0, "Local variable"),
    "NEED_NONE": (0, None),
    "NEED_NUMBER": (0, "Number"),
    "NEED_STRUCTURE": (0, "Building type"),
    "NEED_TEAM": (1, "Team"),
    "NEED_UNIT": (0, "Unit type"),
    "NEED_WAYPOINT": (0, "Waypoint"),
}


def placeholder(name):
    return f"<{name}>" if name else None


def ini_example(kind, index, need):
    if kind == "actions":
        if need not in ACTION_EXAMPLE_LAYOUTS:
            raise ValueError(f"action {index}: no INI example layout for {need}")
        code, primary, rectangle, final = ACTION_EXAMPLE_LAYOUTS[need]
        rect = [placeholder(name) or "0" for name in rectangle] if rectangle else ["0"] * 4
        fields = ["1", str(index), str(code), placeholder(primary) or "0", *rect,
                  placeholder(final) or ""]
        return {"section": "[Actions]", "line": "<TriggerID>=" + ",".join(fields)}

    if need not in EVENT_EXAMPLE_LAYOUTS:
        raise ValueError(f"event {index}: no INI example layout for {need}")
    code, payload = EVENT_EXAMPLE_LAYOUTS[need]
    fields = ["1", str(index), str(code), placeholder(payload) or "0"]
    return {"section": "[Events]", "line": "<TriggerID>=" + ",".join(fields)}


def validate_example_maps(actions, events):
    for label, records, layouts in (
            ("action", actions, ACTION_EXAMPLE_LAYOUTS),
            ("event", events, EVENT_EXAMPLE_LAYOUTS)):
        observed = {record["need"] for record in records}
        mapped = set(layouts)
        if observed != mapped:
            raise ValueError(
                f"{label} INI example mapping is not exhaustive "
                f"(missing={sorted(observed - mapped)}, stale={sorted(mapped - observed)})")



def validate_parameter_map():
    declared = set(re.findall(r"^[ \t]*(NEED_[A-Z_0-9]+)[ \t]*,", read("need.hh"), re.M))
    mapped = set(NEED_PARAMETERS)
    if declared != mapped:
        missing = sorted(declared - mapped)
        stale = sorted(mapped - declared)
        raise ValueError(
            "NeedType parameter mapping is not exhaustive"
            f" (missing={missing}, stale={stale})")


def need_fields(enum_name, needs, default, kind=None, index=None):
    need = needs.get(enum_name, default)
    if need not in NEED_PARAMETERS:
        raise ValueError(f"{enum_name}: unmapped need token {need!r}")
    fields = {
        "need": need,
        "parameters": [dict(parameter) for parameter in NEED_PARAMETERS[need]],
    }
    if kind in ("actions", "events"):
        fields["ini_example"] = ini_example(kind, index, need)
    return fields


# ---------------------------------------------------------------------------
# Parsing
# ---------------------------------------------------------------------------

def read(name):
    return open(os.path.join(CODE_DIR, name), encoding="latin-1").read()


def parse_enum(text, prefix):
    """Ordered enum names, with any trailing comment as a hint.

    Whitespace inside the pattern is [ \\t] only -- \\s would cross the newline
    after an empty trailing comment and swallow the next enum line.
    """
    out = []
    for m in re.finditer(
            r"^[ \t]*(%s_[A-Z_0-9]+)[ \t]*(?:=[ \t]*-?\w+)?[ \t]*,?[ \t]*"
            r"(?://[ \t]*(.*))?$" % prefix,
            text, re.M):
        name = m.group(1)
        if name.endswith(("_COUNT", "_FIRST", "_NONE")) and name != prefix + "_NONE":
            continue
        if name in (prefix + "_COUNT", prefix + "_FIRST"):
            continue
        out.append({"enum": name, "hint": (m.group(2) or "").strip() or None})
    return out


def parse_text_table(text, table_re):
    """[{name, description}] from a {Name, Description} struct array."""
    m = re.search(table_re, text, re.S | re.M)
    if not m:
        return []
    entries = []
    for em in re.finditer(r'\{\s*"((?:[^"\\]|\\.)*)"\s*,\s*"((?:[^"\\]|\\.)*)"\s*\}',
                          m.group(1)):
        entries.append({
            "name": em.group(1).replace('\\"', '"'),
            "description": em.group(2).replace('\\"', '"'),
        })
    return entries


def parse_string_array(text, array_re):
    """["..."] from a plain char const * array."""
    m = re.search(array_re, text, re.S | re.M)
    if not m:
        return []
    return [s.replace('\\"', '"')
            for s in re.findall(r'"((?:[^"\\]|\\.)*)"', m.group(1))]


def parse_switch_map(text, func_re, case_prefix, ret_re, accumulate=False):
    """
    {enum: value} from a `switch` where each case group falls through to one
    return: case TACTION_A: case TACTION_B: return(NEED_X);

    With accumulate, a name matched by more than one switch keeps every value
    joined by "|" instead of only the last. Attaches_To is several switches
    ORing flags into one accumulator, so an event reachable from two of them
    attaches to both.
    """
    m = re.search(func_re, text, re.S | re.M)
    if not m:
        return {}, None
    body = m.group(1)
    result = {}
    pending = []
    default = None
    in_default = False
    for line in body.splitlines():
        cm = re.match(r"\s*case\s+(%s_[A-Z_0-9]+)\s*:" % case_prefix, line)
        if cm:
            pending.append(cm.group(1))
        if re.match(r"\s*default\s*:", line):
            in_default = True
        rm = re.search(ret_re, line)
        if rm:
            value = rm.group(1).strip()
            for name in pending:
                if accumulate and name in result:
                    seen = result[name].split("|")
                    if value not in seen:
                        seen.append(value)
                    result[name] = "|".join(seen)
                else:
                    result[name] = value
            if in_default:
                default = value
                in_default = False
            pending = []
    return result, default



# ---------------------------------------------------------------------------
# Extraction per kind
# ---------------------------------------------------------------------------

def extract_actions():
    hh = read("taction.hh")
    cpp = read("taction.cpp")
    enums = parse_enum(hh, "TACTION")
    table = parse_text_table(cpp, r"_ActionText\[TACTION_COUNT\] = \{(.*?)^\};")
    needs, needs_default = parse_switch_map(
        cpp, r"NeedType\s+(?:TActionClass::)?Action_Needs[^{]*\{(.*?)^\}",
        "TACTION", r"return\s*\(\s*(NEED_[A-Z_0-9]+)\s*\)")
    attaches, attach_default = parse_switch_map(
        cpp, r"AttachType\s+(?:\w+::)?Attaches_To\s*\(\s*TActionType[^{]*\{(.*?)^\}",
        "TACTION", r"attach\s*=\s*(?:AttachType\s*\()?\s*(?:attach\s*\|\s*)?([A-Z_| \t]+?)\s*\)?\s*;",
        accumulate=True)

    records = []
    mismatch = len(enums) != len(table) - len(ACTION_ORPHAN_TEXTS)
    for i, e in enumerate(enums):
        if i in ACTION_LABEL_FIXES:
            ti = ACTION_LABEL_FIXES[i]
            text = table[ti] if ti is not None else None
            status = "verified-mapping"
        else:
            text = table[i] if i < len(table) else None
            status = "needs_review" if mismatch else "generated"
        records.append({
            "id": e["enum"],
            "kind": "trigger-action",
            "index": i,
            "editor_name": text["name"] if text else None,
            "editor_description": text["description"] if text else None,
            **need_fields(e["enum"], needs, needs_default, "actions", i),
            "attaches_to": attaches.get(e["enum"], attach_default),
            "status": status if text else "needs_label",
        })

    orphans = [{"table_index": ti, **table[ti]} for ti in ACTION_ORPHAN_TEXTS]
    return records, orphans


def extract_events():
    hh = read("tevent.hh")
    cpp = read("tevent.cpp")
    enums = parse_enum(hh, "TEVENT")
    table = parse_text_table(cpp, r"_EventText\[TEVENT_COUNT\] = \{(.*?)^\};")
    needs, needs_default = parse_switch_map(
        cpp, r"NeedType\s+(?:\w+::)?Event_Needs[^{]*\{(.*?)^\}",
        "TEVENT", r"return\s*\(\s*(NEED_[A-Z_0-9]+)\s*\)")
    attaches, attach_default = parse_switch_map(
        cpp, r"AttachType\s+(?:\w+::)?Attaches_To\s*\(\s*TEventType[^{]*\{(.*?)^\}",
        "TEVENT", r"attach\s*=\s*(?:AttachType\s*\()?\s*(?:attach\s*\|\s*)?([A-Z_| \t]+?)\s*\)?\s*;",
        accumulate=True)

    records = []
    mismatch = len(enums) != len(table)
    for i, e in enumerate(enums):
        text = table[i] if i < len(table) else None
        records.append({
            "id": e["enum"],
            "kind": "trigger-event",
            "index": i,
            "editor_name": text["name"] if text else None,
            "editor_description": text["description"] if text else None,
            **need_fields(e["enum"], needs, needs_default, "events", i),
            "attaches_to": attaches.get(e["enum"], attach_default),
            "status": ("needs_review" if mismatch else "generated")
                      if text else "needs_label",
        })
    return records


def extract_missions():
    hh = read("tmission.hh")
    cpp = read("tmission.cpp")
    # TMISSION_NONE is -1, so TMissions[0] belongs to the first REAL mission --
    # drop NONE from the indexed list entirely (it is not placeable anyway).
    enums = [e for e in parse_enum(hh, "TMISSION")
             if e["enum"] != "TMISSION_NONE"]
    names = parse_string_array(cpp, r"char const \* (?:const )?TMissions\[[^\]]*\] = \{(.*?)^\};")
    helps = parse_string_array(cpp, r"char const \* (?:const )?TMissionsHelp\[[^\]]*\] = \{(.*?)^\};")
    needs, needs_default = parse_switch_map(
        cpp, r"NeedType\s+TeamMission_Needs[^{]*\{(.*?)^\}",
        "TMISSION", r"return\s*\(\s*(NEED_[A-Z_0-9]+)\s*\)")

    records = []
    mismatch = len(enums) != len(names)
    for i, e in enumerate(enums):
        records.append({
            "id": e["enum"],
            "kind": "team-mission",
            "index": i,
            "editor_name": names[i] if i < len(names) else None,
            "editor_description": helps[i] if i < len(helps) else None,
            **need_fields(e["enum"], needs, needs_default),
            "status": ("needs_review" if mismatch else "generated")
                      if i < len(names) else "needs_label",
        })
    return records
