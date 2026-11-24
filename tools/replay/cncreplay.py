#!/usr/bin/env python3
"""Read, verify, compare, and write CNCR replay files (.cncr).

This is the reference reader and writer for docs/replay/REPLAY-FORMAT.md,
and for the launch request of docs/replay/launch.proto. It uses the
Python standard library only and needs Python 3.11 or later.

Commands:

  list <file> [--json]        players, map, mode, start time, length, build,
                              and whether the file is complete
  dump <file> [--json] [--full]
                              every section, frame record, event, seal,
                              message, checkpoint, statistic, and summary field
  verify <file> [--json]      check the container, the limits, and the seals
  compare <a> <b> [--json]    say whether two copies record the same game and
                              where their sealed contents first differ
  launch encode <json> <out> [--message NAME]
                              write a binary launch message from its proto3
                              JSON form
  launch decode <file> [--message NAME]
                              print a binary launch message as proto3 JSON

Exit codes of verify: 0 when the file is complete and every seal matches; 1
when a seal or a structural check fails; 2 when the file is not a readable
CNCR file; 3 when the file has no END section and every seal in it matches;
4 when the file is not verified: it is not malformed, but this reader cannot
check its seals, because it does not know the file's seal_version or the
codec of INIT or FRMS.

Exit codes of compare: 0 when both copies record the same game and their
sealed contents agree up to the frame where the shorter copy ends; 1 when
they differ; 2 when either file cannot be read far enough to compute S0.
"""

from __future__ import annotations

import argparse
import base64
import bisect
import datetime
import hashlib
import json
import re
import struct
import sys
import zlib

# ---------------------------------------------------------------------------
# Schema tables, which must match replay.proto and launch.proto.
#
# A field is (number, name, type) or (number, name, type, label). The label is
# 'optional', 'repeated', or 'oneof <name>'. A type without a package prefix
# belongs to the package of the message.

RESERVED_PRIVATE = ((100, 199),)

ENUM_TABLE = {
    'cncreplay.GameKind': (
        ('GAME_KIND_UNKNOWN', 0), ('GAME_KIND_CAMPAIGN', 1),
        ('GAME_KIND_SKIRMISH', 2), ('GAME_KIND_MULTIPLAYER', 3)),
    'cncreplay.EventRole': (
        ('EVENT_ROLE_NONE', 0), ('EVENT_ROLE_CHAT_PROOF', 1)),
    'cncreplay.MessageScope': (
        ('MESSAGE_SCOPE_UNKNOWN', 0), ('MESSAGE_SCOPE_ALL', 1),
        ('MESSAGE_SCOPE_TEAM', 2), ('MESSAGE_SCOPE_PRIVATE', 3),
        ('MESSAGE_SCOPE_OBSERVERS', 4)),
    'cncreplay.ObjectCategory': (
        ('OBJECT_CATEGORY_UNKNOWN', 0), ('OBJECT_CATEGORY_BUILDING', 1),
        ('OBJECT_CATEGORY_INFANTRY', 2), ('OBJECT_CATEGORY_VEHICLE', 3),
        ('OBJECT_CATEGORY_AIRCRAFT', 4)),
    'cncreplay.Outcome': (
        ('OUTCOME_UNKNOWN', 0), ('OUTCOME_WON', 1), ('OUTCOME_LOST', 2),
        ('OUTCOME_DRAW', 3)),
    'cncreplay.LeaveReason': (
        ('LEAVE_REASON_NONE', 0), ('LEAVE_REASON_RESIGNED', 1),
        ('LEAVE_REASON_DISCONNECTED', 2)),
    'cncreplay.CountKind': (
        ('COUNT_KIND_UNKNOWN', 0), ('COUNT_KIND_BUILT', 1),
        ('COUNT_KIND_KILLED', 2), ('COUNT_KIND_LOST', 3),
        ('COUNT_KIND_CAPTURED', 4), ('COUNT_KIND_REMAINING', 5)),
    'cnclaunch.ResultCode': (
        ('RESULT_CODE_UNKNOWN', 0), ('RESULT_CODE_OK', 1),
        ('RESULT_CODE_REFUSED', 2), ('RESULT_CODE_FAILED', 3),
        ('RESULT_CODE_CRASHED', 4), ('RESULT_CODE_UNREADABLE', 5),
        ('RESULT_CODE_MAP_MISSING', 6), ('RESULT_CODE_START_MISMATCH', 7),
        ('RESULT_CODE_HASH_MISMATCH', 8), ('RESULT_CODE_SEAL_MISMATCH', 9)),
    'cnclaunch.GameOutcome': (
        ('GAME_OUTCOME_UNKNOWN', 0), ('GAME_OUTCOME_WON', 1),
        ('GAME_OUTCOME_LOST', 2), ('GAME_OUTCOME_DRAW', 3),
        ('GAME_OUTCOME_ABORTED', 4), ('GAME_OUTCOME_DESYNC', 5),
        ('GAME_OUTCOME_CONNECTION_LOST', 6), ('GAME_OUTCOME_NOT_STARTED', 7)),
    'cnclaunch.NotRecorded': (
        ('NOT_RECORDED_NONE', 0), ('NOT_RECORDED_DISABLED', 1),
        ('NOT_RECORDED_RESUMED', 2), ('NOT_RECORDED_CHEATS', 3),
        ('NOT_RECORDED_TIME_LIMIT', 4), ('NOT_RECORDED_WRITE_FAILED', 5)),
}

MESSAGE_TABLE = {
    # ---- replay.proto: INFO
    'cncreplay.RecordingInfo': (RESERVED_PRIVATE, (
        (1, 'engine', 'string'),
        (2, 'engine_version', 'string'),
        (3, 'data_version', 'uint32'),
        (4, 'package', 'string'),
        (5, 'package_version', 'string'),
        (6, 'files', 'FileHash', 'repeated'),
        (7, 'recorded_at', 'uint64'),
        (8, 'recorder', 'uint32', 'optional'),
        (9, 'map_name', 'string'),
        (10, 'map_path', 'string'),
        (11, 'map_sha256', 'bytes'),
        (12, 'game_mode', 'string'),
        (13, 'seal_version', 'uint32'),
        (14, 'colors', 'PlayerColor', 'repeated'),
        (15, 'income_kinds', 'IncomeKind', 'repeated'))),
    'cncreplay.FileHash': ((), (
        (1, 'path', 'string'),
        (2, 'hash', 'string'))),
    'cncreplay.PlayerColor': ((), (
        (1, 'number', 'uint32'),
        (2, 'rgb', 'fixed32'))),
    'cncreplay.IncomeKind': ((), (
        (1, 'number', 'uint32'),
        (2, 'id', 'string'))),
    # ---- LNCH
    'cncreplay.Launch': (RESERVED_PRIVATE, (
        (1, 'kind', 'GameKind'),
        (2, 'seed', 'fixed32', 'optional'),
        (3, 'players', 'Player', 'repeated'),
        (4, 'options', 'GameOption', 'repeated'),
        (5, 'anonymous', 'bool'))),
    'cncreplay.Player': (RESERVED_PRIVATE, (
        (1, 'house', 'uint32'),
        (2, 'name', 'string'),
        (3, 'human', 'bool'),
        (4, 'observer', 'bool'),
        (5, 'country', 'sint32'),
        (6, 'country_name', 'string'),
        (7, 'color', 'sint32'),
        (8, 'start', 'sint32'),
        (9, 'ai_level', 'sint32', 'optional'),
        (10, 'allies', 'uint32', 'repeated'),
        (11, 'resolved_country', 'sint32', 'optional'),
        (12, 'resolved_color', 'sint32', 'optional'),
        (13, 'resolved_start', 'sint32', 'optional'))),
    'cncreplay.GameOption': ((), (
        (1, 'key', 'string'),
        (2, 'number', 'sint64', 'oneof value'),
        (3, 'flag', 'bool', 'oneof value'),
        (4, 'text', 'string', 'oneof value'))),
    # ---- FRMS
    'cncreplay.Frame': (RESERVED_PRIVATE, (
        (1, 'skipped', 'uint32'),
        (2, 'hash', 'fixed32', 'optional'),
        (3, 'events', 'Event', 'repeated'),
        (4, 'view', 'ViewState'),
        (5, 'messages', 'PlayerMessage', 'repeated'),
        (6, 'frame_micros', 'uint32', 'optional'),
        (7, 'samples', 'HouseSample', 'repeated'),
        (8, 'income', 'Income', 'repeated'),
        (9, 'seals', 'Seal', 'repeated'),
        (10, 'defeated', 'uint32', 'repeated'))),
    'cncreplay.Event': (RESERVED_PRIVATE, (
        (1, 'type', 'uint32'),
        (2, 'house', 'uint32'),
        (3, 'due', 'sint32'),
        (4, 'payload', 'bytes'),
        (5, 'local', 'bool'),
        (6, 'role', 'EventRole'),
        (7, 'sealed_payload', 'bytes', 'optional'))),
    'cncreplay.ViewState': (RESERVED_PRIVATE, (
        (1, 'camera', 'Point', 'optional'),
        (2, 'selection', 'Selection', 'optional'))),
    'cncreplay.Point': ((), (
        (1, 'x', 'sint32'),
        (2, 'y', 'sint32'))),
    'cncreplay.Selection': ((), (
        (1, 'ids', 'uint32', 'repeated'),)),
    'cncreplay.PlayerMessage': (RESERVED_PRIVATE, (
        (1, 'house', 'uint32'),
        (2, 'sender', 'string'),
        (3, 'text', 'string'),
        (4, 'color', 'sint32'),
        (5, 'scope', 'MessageScope'),
        (6, 'recipients', 'uint32', 'repeated'),
        (7, 'salt', 'bytes'))),
    'cncreplay.HouseSample': (RESERVED_PRIVATE, (
        (1, 'house', 'uint32'),
        (2, 'credits', 'sint64', 'optional'),
        (3, 'stored_value', 'sint64', 'optional'),
        (4, 'spent', 'sint64', 'optional'),
        (5, 'earned', 'sint64', 'optional'),
        (6, 'power_output', 'sint32', 'optional'),
        (7, 'power_drain', 'sint32', 'optional'),
        (8, 'vehicles', 'uint32', 'optional'),
        (9, 'infantry', 'uint32', 'optional'),
        (10, 'aircraft', 'uint32', 'optional'),
        (11, 'buildings', 'uint32', 'optional'),
        (12, 'army_value', 'sint64', 'optional'),
        (13, 'building_value', 'sint64', 'optional'),
        (14, 'units_killed', 'uint32', 'optional'),
        (15, 'buildings_killed', 'uint32', 'optional'),
        (16, 'units_lost', 'uint32', 'optional'),
        (17, 'buildings_lost', 'uint32', 'optional'),
        (18, 'units_built', 'uint32', 'optional'),
        (19, 'buildings_built', 'uint32', 'optional'),
        (20, 'score', 'sint64', 'optional'),
        (21, 'defeated', 'bool'))),
    'cncreplay.Income': (RESERVED_PRIVATE, (
        (1, 'house', 'uint32'),
        (2, 'kind', 'uint32'),
        (3, 'amount', 'sint64'))),
    'cncreplay.Seal': ((), (
        (1, 'through', 'uint32'),
        (2, 'hash', 'bytes'))),
    # ---- CKPT
    'cncreplay.Checkpoint': (RESERVED_PRIVATE, (
        (1, 'frame', 'uint32'),
        (2, 'crc32', 'fixed32'),
        (3, 'state', 'bytes'))),
    # ---- STAT
    'cncreplay.Statistics': (RESERVED_PRIVATE, (
        (1, 'types', 'ObjectType', 'repeated'),
        (2, 'houses', 'HouseStatistics', 'repeated'))),
    'cncreplay.ObjectType': (RESERVED_PRIVATE, (
        (1, 'category', 'ObjectCategory'),
        (2, 'index', 'uint32'),
        (3, 'id', 'string'),
        (4, 'name', 'string'),
        (5, 'cost', 'sint32'),
        (6, 'icon', 'string'),
        (7, 'naval', 'bool'),
        (8, 'unscored', 'bool'),
        (9, 'insignificant', 'bool'))),
    'cncreplay.HouseStatistics': (RESERVED_PRIVATE, (
        (1, 'house', 'uint32'),
        (2, 'name', 'string'),
        (3, 'country_name', 'string'),
        (4, 'color', 'sint32'),
        (5, 'start', 'sint32'),
        (6, 'allies', 'uint32', 'repeated'),
        (7, 'outcome', 'Outcome'),
        (8, 'leave', 'LeaveReason'),
        (9, 'final', 'HouseSample'),
        (10, 'counts', 'TypeCount', 'repeated'),
        (11, 'kills', 'HouseKills', 'repeated'),
        (12, 'crates', 'CrateCount', 'repeated'))),
    'cncreplay.TypeCount': ((), (
        (1, 'kind', 'CountKind'),
        (2, 'category', 'ObjectCategory'),
        (3, 'index', 'uint32'),
        (4, 'count', 'uint32'))),
    'cncreplay.CrateCount': ((), (
        (1, 'id', 'string'),
        (2, 'count', 'uint32'))),
    'cncreplay.HouseKills': ((), (
        (1, 'house', 'uint32'),
        (2, 'units', 'uint32'),
        (3, 'buildings', 'uint32'))),
    # ---- END
    'cncreplay.RecordingSummary': (RESERVED_PRIVATE, (
        (1, 'last_frame', 'uint32', 'optional'),
        (2, 'ended_at', 'uint64'),
        (3, 'desync_frame', 'uint32', 'optional'),
        (4, 'completed', 'bool'),
        (5, 'seals', 'Seal', 'repeated'),
        (6, 'elapsed_micros', 'uint64', 'optional'),
        (7, 'game_micros', 'uint64', 'optional'))),

    # ---- launch.proto
    'cnclaunch.LaunchRequest': (RESERVED_PRIVATE, (
        (1, 'version', 'uint32'),
        (2, 'play', 'Play', 'oneof verb'),
        (3, 'load', 'Load', 'oneof verb'),
        (4, 'replay', 'Replay', 'oneof verb'),
        (5, 'verify', 'Verify', 'oneof verb'),
        (6, 'capture', 'Capture', 'oneof verb'),
        (7, 'result_path', 'string'),
        (8, 'channel', 'string'),
        (9, 'request_id', 'string'))),
    'cnclaunch.Play': (RESERVED_PRIVATE, (
        (1, 'launch', 'cncreplay.Launch'),
        (2, 'map_path', 'string'),
        (3, 'session', 'Session'),
        (4, 'network', 'Network'),
        (5, 'recording', 'Recording'),
        (6, 'fixture', 'Fixture'))),
    'cnclaunch.Session': (RESERVED_PRIVATE, (
        (1, 'local_player', 'uint32', 'optional'),
        (2, 'host', 'bool'),
        (3, 'autosave_interval', 'uint32'),
        (4, 'next_campaign_autosave', 'uint32'),
        (5, 'next_skirmish_autosave', 'uint32'),
        (6, 'skip_score_screen', 'bool'),
        (7, 'write_statistics', 'bool'),
        (8, 'game_id', 'uint32'),
        (9, 'map_hash', 'string'),
        (10, 'map_name', 'string'),
        (11, 'game_mode', 'string'),
        (12, 'load_screen', 'LoadScreen'),
        (13, 'difficulty_name', 'string'))),
    'cnclaunch.LoadScreen': (RESERVED_PRIVATE, (
        (1, 'image', 'string'),
        (2, 'x', 'sint32', 'optional'),
        (3, 'y', 'sint32', 'optional'))),
    'cnclaunch.Network': (RESERVED_PRIVATE, (
        (1, 'port', 'sint32'),
        (2, 'peers', 'Peer', 'repeated'),
        (3, 'tunnel', 'Tunnel'),
        (4, 'protocol', 'uint32'),
        (5, 'frame_send_rate', 'uint32'),
        (6, 'max_ahead', 'uint32'),
        (7, 'connect_timeout', 'uint32'),
        (8, 'reconnect_timeout', 'uint32'))),
    'cnclaunch.Peer': (RESERVED_PRIVATE, (
        (1, 'player', 'uint32'),
        (2, 'address', 'string'),
        (3, 'port', 'sint32'))),
    'cnclaunch.Tunnel': (RESERVED_PRIVATE, (
        (1, 'address', 'string'),
        (2, 'port', 'uint32'))),
    'cnclaunch.Recording': (RESERVED_PRIVATE, (
        (1, 'enabled', 'bool', 'optional'),
        (2, 'file_out', 'string'),
        (3, 'package', 'string'),
        (4, 'package_version', 'string'),
        (5, 'files', 'cncreplay.FileHash', 'repeated'),
        (6, 'embed_map', 'bool', 'optional'))),
    'cnclaunch.Fixture': (RESERVED_PRIVATE, (
        (1, 'headless', 'bool'),
        (2, 'stop_frame', 'uint32', 'optional'),
        (3, 'script_path', 'string'),
        (4, 'checkpoint_check', 'CheckpointCheck'))),
    'cnclaunch.CheckpointCheck': (RESERVED_PRIVATE, (
        (1, 'save_frame', 'uint32'),
        (2, 'run_frames', 'uint32'))),
    'cnclaunch.Load': (RESERVED_PRIVATE, (
        (1, 'save', 'string'),
        (2, 'session', 'Session'),
        (3, 'network', 'Network'),
        (4, 'players', 'cncreplay.Player', 'repeated'),
        (5, 'game_speed', 'sint32', 'optional'))),
    'cnclaunch.Replay': (RESERVED_PRIVATE, (
        (1, 'file', 'string'),
        (2, 'watch_player', 'uint32', 'optional'),
        (3, 'free_view', 'bool'),
        (4, 'start_frame', 'uint32'),
        (5, 'map_path', 'string'),
        (6, 'load_screen', 'LoadScreen'))),
    'cnclaunch.Verify': (RESERVED_PRIVATE, (
        (1, 'file', 'string'),
        (2, 'out_path', 'string'),
        (3, 'map_path', 'string'))),
    'cnclaunch.Capture': (RESERVED_PRIVATE, (
        (1, 'map_path', 'string'),
        (2, 'out_path', 'string'),
        (3, 'description_path', 'string'))),
    'cnclaunch.LaunchResult': (RESERVED_PRIVATE, (
        (1, 'request_id', 'string'),
        (2, 'code', 'ResultCode'),
        (3, 'message', 'string'),
        (4, 'scenarios', 'ScenarioResult', 'repeated'),
        (5, 'statistics_path', 'string'),
        (6, 'different_build', 'bool'),
        (7, 'unverified', 'bool'),
        (8, 'mismatch_frame', 'uint32', 'optional'),
        (9, 'output_path', 'string'),
        (10, 'crash_path', 'string'))),
    'cnclaunch.ScenarioResult': (RESERVED_PRIVATE, (
        (1, 'map_path', 'string'),
        (2, 'outcome', 'GameOutcome'),
        (3, 'completed', 'bool'),
        (4, 'last_frame', 'uint32', 'optional'),
        (5, 'replay_path', 'string'),
        (6, 'not_recorded', 'NotRecorded'))),
}

SCALAR_TYPES = frozenset({
    'double', 'float', 'int32', 'int64', 'uint32', 'uint64', 'sint32',
    'sint64', 'fixed32', 'fixed64', 'sfixed32', 'sfixed64', 'bool', 'string',
    'bytes'})
# The schema uses only these scalar types, and the codec supports only these.
CODEC_SCALARS = frozenset({
    'uint32', 'uint64', 'sint32', 'sint64', 'fixed32', 'bool', 'string',
    'bytes'})

WT_VARINT, WT_FIXED64, WT_LEN, WT_START_GROUP, WT_END_GROUP, WT_FIXED32 = 0, 1, 2, 3, 4, 5


def json_name(name):
    """Return the proto3 JSON name of a field, as protoc derives it."""
    out = []
    upper = False
    for ch in name:
        if ch == '_':
            upper = True
        elif upper:
            out.append(ch.upper())
            upper = False
        else:
            out.append(ch)
    return ''.join(out)


class FieldSpec:
    __slots__ = ('number', 'name', 'type', 'label', 'oneof', 'kind', 'json')

    def __init__(self, number, name, type_, label, oneof, kind):
        self.number = number
        self.name = name
        self.type = type_        # scalar name or full message/enum name
        self.label = label       # '', 'optional', 'repeated'
        self.oneof = oneof       # oneof name or None
        self.kind = kind         # 'scalar', 'enum', 'message'
        self.json = json_name(name)

    @property
    def repeated(self):
        return self.label == 'repeated'

    @property
    def has_presence(self):
        """True when an unset field and a zero value are different."""
        return self.label == 'optional' or self.oneof is not None

    @property
    def wire_type(self):
        if self.kind == 'message' or self.type in ('string', 'bytes'):
            return WT_LEN
        if self.type == 'fixed32':
            return WT_FIXED32
        return WT_VARINT

    @property
    def packable(self):
        return self.repeated and self.wire_type != WT_LEN


class MessageSpec:
    __slots__ = ('name', 'package', 'fields', 'by_number', 'by_name', 'by_json',
                 'reserved', 'oneofs')

    def __init__(self, name, reserved, fields):
        self.name = name
        self.package = name.rsplit('.', 1)[0]
        self.reserved = tuple(reserved)
        self.fields = tuple(sorted(fields, key=lambda f: f.number))
        self.by_number = {f.number: f for f in self.fields}
        self.by_name = {f.name: f for f in self.fields}
        self.by_json = {f.json: f for f in self.fields}
        self.oneofs = {}
        for f in self.fields:
            if f.oneof:
                self.oneofs.setdefault(f.oneof, []).append(f)

    def reserves(self, number):
        return any(lo <= number <= hi for lo, hi in self.reserved)


def _resolve(package, type_):
    if type_ in SCALAR_TYPES:
        return type_, 'scalar'
    full = type_ if '.' in type_ else package + '.' + type_
    if full in ENUM_TABLE:
        return full, 'enum'
    if full in MESSAGE_TABLE:
        return full, 'message'
    raise KeyError('unknown type ' + type_)


def _build_schema():
    messages = {}
    for name, (reserved, rows) in MESSAGE_TABLE.items():
        package = name.rsplit('.', 1)[0]
        fields = []
        for row in rows:
            number, fname, type_ = row[:3]
            label = row[3] if len(row) > 3 else ''
            oneof = None
            if label.startswith('oneof '):
                oneof = label[6:]
                label = ''
            full, kind = _resolve(package, type_)
            if kind == 'scalar' and full not in CODEC_SCALARS:
                raise KeyError('codec has no support for ' + full)
            fields.append(FieldSpec(number, fname, full, label, oneof, kind))
        messages[name] = MessageSpec(name, reserved, fields)
    return messages


MESSAGES = _build_schema()
ENUM_NAMES = {name: dict(values) for name, values in ENUM_TABLE.items()}
ENUM_VALUES = {name: {v: k for k, v in values} for name, values in ENUM_TABLE.items()}


# ---------------------------------------------------------------------------
# Errors and limits

class MalformedError(Exception):
    """Input that the format does not allow. `code` is a short stable name."""

    def __init__(self, code, message, offset=None):
        super().__init__(message)
        self.code = code
        self.message = message
        self.offset = offset


class Truncated(MalformedError):
    """Input that ends inside a value."""

    def __init__(self, message, offset=None):
        super().__init__('truncated', message, offset)


class Limits:
    """The format's size limits. A keyword argument replaces the limit it names."""

    def __init__(self, **overrides):
        self.section_raw = {
            b'INFO': 1 << 20, b'LNCH': 1 << 20, b'END ': 1 << 20,
            b'MAP ': 32 << 20, b'INIT': 32 << 20, b'STAT': 32 << 20,
            b'CKPT': 64 << 20,
        }
        self.record = 1 << 20
        self.events = 16384
        self.payload = 4096
        self.string = 1024
        self.players = 64
        self.houses = 64
        self.income_kinds = 4096
        for key, value in overrides.items():
            if not hasattr(self, key):
                raise TypeError('unknown limit ' + key)
            setattr(self, key, value)


DEFAULT_LIMITS = Limits()


# ---------------------------------------------------------------------------
# Wire codec

U32 = (1 << 32) - 1
U64 = (1 << 64) - 1


def encode_varint(value):
    if value < 0 or value > U64:
        raise ValueError('varint out of range: %r' % (value,))
    out = bytearray()
    while value >= 0x80:
        out.append((value & 0x7F) | 0x80)
        value >>= 7
    out.append(value)
    return bytes(out)


def decode_varint(buf, pos, end):
    """Return (value, next position). Raises Truncated or MalformedError."""
    result = 0
    shift = 0
    start = pos
    while True:
        if pos >= end:
            raise Truncated('varint cut short', start)
        b = buf[pos]
        pos += 1
        if shift == 63 and b > 1:
            raise MalformedError('varint_overflow', 'varint exceeds 64 bits', start)
        result |= (b & 0x7F) << shift
        if b < 0x80:
            return result, pos
        shift += 7
        if shift > 63:
            raise MalformedError('varint_overflow', 'varint longer than 10 bytes', start)


def zigzag_encode(value, bits):
    lo, hi = -(1 << (bits - 1)), (1 << (bits - 1)) - 1
    if not lo <= value <= hi:
        raise ValueError('sint%d out of range: %r' % (bits, value))
    return ((value << 1) ^ (value >> (bits - 1))) & ((1 << bits) - 1)


def zigzag_decode(value):
    return (value >> 1) ^ -(value & 1)


def _tag(number, wire_type):
    return encode_varint((number << 3) | wire_type)


def _is_default(value):
    return value is None or value == 0 or value is False or value == '' or value == b''


def _encode_scalar(field, value):
    t = field.type
    if field.kind == 'enum':
        if isinstance(value, str):
            value = ENUM_NAMES[t][value]
        if isinstance(value, bool) or not isinstance(value, int):
            raise ValueError('%s: enum value must be an integer' % field.name)
        if not -(1 << 31) <= value <= (1 << 31) - 1:
            raise ValueError('%s: enum value out of range' % field.name)
        return encode_varint(value & U64)
    if t == 'string':
        if isinstance(value, (bytes, bytearray)):
            data = bytes(value)       # raw bytes, written as given even when not UTF-8
        elif isinstance(value, str):
            data = value.encode('utf-8')
        else:
            raise ValueError('%s: string expected' % field.name)
        return encode_varint(len(data)) + data
    if t == 'bytes':
        if not isinstance(value, (bytes, bytearray)):
            raise ValueError('%s: bytes expected' % field.name)
        return encode_varint(len(value)) + bytes(value)
    if t == 'bool':
        if not isinstance(value, bool) and value not in (0, 1):
            raise ValueError('%s: bool expected' % field.name)
        return b'\x01' if value else b'\x00'
    if isinstance(value, bool) or not isinstance(value, int):
        raise ValueError('%s: integer expected' % field.name)
    if t == 'uint32':
        if not 0 <= value <= U32:
            raise ValueError('%s: uint32 out of range' % field.name)
        return encode_varint(value)
    if t == 'uint64':
        if not 0 <= value <= U64:
            raise ValueError('%s: uint64 out of range' % field.name)
        return encode_varint(value)
    if t == 'sint32':
        return encode_varint(zigzag_encode(value, 32))
    if t == 'sint64':
        return encode_varint(zigzag_encode(value, 64))
    if t == 'fixed32':
        if not 0 <= value <= U32:
            raise ValueError('%s: fixed32 out of range' % field.name)
        return struct.pack('<I', value)
    raise ValueError('unsupported type ' + t)


def encode_message(name, msg):
    """Encode a message dict in the format's canonical form.

    A dict key is a field name. A field with presence is set when its key is
    present with a value other than None. `_unknown` is a list of
    (field number, raw bytes) pairs written back unchanged.
    """
    spec = MESSAGES[name]
    for key in msg:
        if key != '_unknown' and key not in spec.by_name:
            raise ValueError('%s has no field %r' % (name, key))
    for members in spec.oneofs.values():
        chosen = [f.name for f in members if msg.get(f.name) is not None]
        if len(chosen) > 1:
            raise ValueError('%s: more than one member of a oneof set: %s'
                             % (name, ', '.join(chosen)))
    parts = []
    for f in spec.fields:
        if f.name not in msg:
            continue
        value = msg[f.name]
        if value is None:
            continue
        if f.repeated:
            if not value:
                continue
            if f.packable:
                body = b''.join(_encode_scalar(f, v) for v in value)
                parts.append((f.number, _tag(f.number, WT_LEN) + encode_varint(len(body)) + body))
            else:
                chunk = bytearray()
                for v in value:
                    if f.kind == 'message':
                        body = encode_message(f.type, v)
                        chunk += _tag(f.number, WT_LEN) + encode_varint(len(body)) + body
                    else:
                        chunk += _tag(f.number, f.wire_type) + _encode_scalar(f, v)
                parts.append((f.number, bytes(chunk)))
            continue
        if f.kind == 'message':
            body = encode_message(f.type, value)
            if not body and not f.has_presence:
                continue
            parts.append((f.number, _tag(f.number, WT_LEN) + encode_varint(len(body)) + body))
            continue
        if f.kind == 'enum' and isinstance(value, str):
            value = ENUM_NAMES[f.type][value]
        if not f.has_presence and _is_default(value):
            continue
        parts.append((f.number, _tag(f.number, f.wire_type) + _encode_scalar(f, value)))
    for number, raw in msg.get('_unknown', ()):
        parts.append((number, bytes(raw)))
    parts.sort(key=lambda p: p[0])
    return b''.join(p[1] for p in parts)


def _skip_value(buf, pos, end, wire_type):
    if wire_type == WT_VARINT:
        _, pos = decode_varint(buf, pos, end)
        return pos
    if wire_type == WT_FIXED64:
        if pos + 8 > end:
            raise Truncated('fixed64 cut short', pos)
        return pos + 8
    if wire_type == WT_FIXED32:
        if pos + 4 > end:
            raise Truncated('fixed32 cut short', pos)
        return pos + 4
    if wire_type == WT_LEN:
        length, p = decode_varint(buf, pos, end)
        if length > end - p:
            raise Truncated('length-delimited field runs past the end', pos)
        return p + length
    if wire_type in (WT_START_GROUP, WT_END_GROUP):
        raise MalformedError('group_wire_type', 'group wire type %d' % wire_type, pos)
    raise MalformedError('bad_wire_type', 'invalid wire type %d' % wire_type, pos)


def _decode_varint_value(field, raw, offset):
    # A value too large for its type is narrowed as a generated protobuf parser narrows it.
    t = field.type
    if field.kind == 'enum':
        low = raw & U32
        return low - (1 << 32) if low >= 1 << 31 else low
    if t == 'bool':
        return raw != 0
    if t == 'uint64':
        return raw
    if t == 'sint64':
        return zigzag_decode(raw)
    if t == 'uint32':
        return raw & U32
    if t == 'sint32':
        return zigzag_decode(raw & U32)
    raise MalformedError('internal', 'unexpected varint type ' + t, offset)


# The format's limits on repeated message fields and byte fields, keyed by (message, field) and
# naming the Limits attribute. A decode applies them to every occurrence of a field.
_COUNT_LIMITS = {
    ('cncreplay.Launch', 'players'): 'players',
    ('cncreplay.Statistics', 'houses'): 'houses',
    ('cncreplay.Frame', 'events'): 'events',
    ('cncreplay.RecordingInfo', 'income_kinds'): 'income_kinds',
}
_BYTES_LIMITS = {
    ('cncreplay.Event', 'payload'): 'payload',
    ('cncreplay.Event', 'sealed_payload'): 'payload',
}


def _decode_len_value(field, buf, start, end, limits, message=None):
    if field.kind == 'message':
        return decode_message(field.type, buf, start, end, limits)
    key = _BYTES_LIMITS.get((message, field.name))
    if key is not None and limits is not None and end - start > getattr(limits, key):
        raise MalformedError('limit_' + key, '%s: %d bytes exceed the limit of %d'
                             % (field.name, end - start, getattr(limits, key)), start)
    data = bytes(buf[start:end])
    if field.type == 'string':
        if limits is not None and len(data) > limits.string:
            raise MalformedError('limit_string', '%s: string of %d bytes exceeds %d'
                                 % (field.name, len(data), limits.string), start)
        try:
            return data.decode('utf-8')
        except UnicodeDecodeError:
            raise MalformedError('invalid_utf8', '%s: string is not valid UTF-8'
                                 % field.name, start) from None
    return data


def decode_message(name, buf, start=0, end=None, limits=None):
    """Decode a message. Raises MalformedError on input the format refuses.

    Fields absent on the wire are absent from the dict. Unknown fields are kept
    under `_unknown`. When a non-repeated field appears more than once the last
    value counts, for message fields as well.
    """
    spec = MESSAGES[name]
    if end is None:
        end = len(buf)
    msg = {}
    unknown = []
    pos = start
    while pos < end:
        tag_start = pos
        tag, pos = decode_varint(buf, pos, end)
        if tag > U32:
            raise MalformedError('bad_tag', 'field tag exceeds 32 bits', tag_start)
        number, wire_type = tag >> 3, tag & 7
        if number == 0:
            raise MalformedError('bad_tag', 'field number 0', tag_start)
        if wire_type in (6, 7):
            raise MalformedError('bad_wire_type', 'invalid wire type %d' % wire_type, tag_start)
        field = spec.by_number.get(number)
        if field is None:
            pos = _skip_value(buf, pos, end, wire_type)
            unknown.append((number, bytes(buf[tag_start:pos])))
            continue
        if wire_type in (WT_START_GROUP, WT_END_GROUP):
            raise MalformedError('group_wire_type', 'group wire type %d' % wire_type, tag_start)
        expected = field.wire_type
        if wire_type != expected and not (field.packable and wire_type == WT_LEN):
            # A generated parser skips a known field with another wire type, and so does this one.
            pos = _skip_value(buf, pos, end, wire_type)
            unknown.append((number, bytes(buf[tag_start:pos])))
            continue
        if wire_type == WT_VARINT:
            value_start = pos
            raw, pos = decode_varint(buf, pos, end)
            value = _decode_varint_value(field, raw, value_start)
        elif wire_type == WT_FIXED32:
            if pos + 4 > end:
                raise Truncated('fixed32 cut short', pos)
            value = struct.unpack_from('<I', buf, pos)[0]
            pos += 4
        else:
            length, p = decode_varint(buf, pos, end)
            if length > end - p:
                raise Truncated('%s: length runs past the end' % field.name, pos)
            if field.packable:
                values = []
                q, qend = p, p + length
                while q < qend:
                    if expected == WT_FIXED32:
                        if q + 4 > qend:
                            raise Truncated('packed fixed32 cut short', q)
                        values.append(struct.unpack_from('<I', buf, q)[0])
                        q += 4
                    else:
                        vstart = q
                        raw, q = decode_varint(buf, q, qend)
                        values.append(_decode_varint_value(field, raw, vstart))
                msg.setdefault(field.name, []).extend(values)
                pos = p + length
                continue
            value = _decode_len_value(field, buf, p, p + length, limits, name)
            pos = p + length
        if field.repeated:
            values = msg.setdefault(field.name, [])
            values.append(value)
            key = _COUNT_LIMITS.get((name, field.name))
            if key is not None and limits is not None and len(values) > getattr(limits, key):
                raise MalformedError('limit_' + key, '%s: more than %d entries'
                                     % (field.name, getattr(limits, key)), tag_start)
        else:
            if field.oneof:
                for other in spec.oneofs[field.oneof]:
                    msg.pop(other.name, None)
            msg[field.name] = value
    if unknown:
        msg['_unknown'] = unknown
    return msg


def field_value(name, msg, field_name):
    """Return a field's value, or its default when absent (None for presence)."""
    f = MESSAGES[name].by_name[field_name]
    if field_name in msg:
        return msg[field_name]
    if f.repeated:
        return []
    if f.has_presence:
        return None
    if f.kind == 'message':
        return {}
    if f.type == 'string':
        return ''
    if f.type == 'bytes':
        return b''
    if f.type == 'bool':
        return False
    return 0


def enum_name(enum, value):
    return ENUM_VALUES[enum].get(value, value)


def to_display(name, msg, defaults=True):
    """A JSON-ready view of a decoded message: enum names, bytes as hex, and
    fixed32 values as 0x-prefixed hex. With `defaults`, a scalar field without
    presence that is absent is shown with its zero value."""
    spec = MESSAGES[name]
    out = {}
    for f in spec.fields:
        if f.name not in msg or msg[f.name] is None:
            if defaults and f.kind != 'message' and not f.repeated and not f.has_presence:
                msg = dict(msg)
                msg[f.name] = field_value(name, {}, f.name)
            else:
                continue
        value = msg[f.name]

        def one(v, f=f):
            if f.kind == 'message':
                return to_display(f.type, v, defaults)
            if f.kind == 'enum':
                return enum_name(f.type, v)
            if isinstance(v, bytes):
                return v.hex()
            if f.type == 'fixed32':
                return '0x%08x' % v
            return v
        out[f.name] = [one(v) for v in value] if f.repeated else one(value)
    if msg.get('_unknown'):
        out['_unknown'] = [{'field': n, 'hex': raw.hex()} for n, raw in msg['_unknown']]
    return out


def random_message(name, rng, depth=0):
    """A random value of a message."""
    spec = MESSAGES[name]
    msg = {}
    chosen_oneof = {k: rng.choice(v + [None]) for k, v in spec.oneofs.items()}
    for f in spec.fields:
        if f.oneof and chosen_oneof[f.oneof] is not f:
            continue
        if not f.oneof and rng.random() < 0.3:
            continue

        def one():
            if f.kind == 'message':
                return random_message(f.type, rng, depth + 1) if depth < 3 else {}
            if f.kind == 'enum':
                return rng.choice(list(ENUM_VALUES[f.type]))
            t = f.type
            if t == 'bool':
                return rng.random() < 0.5
            if t == 'string':
                return rng.choice(['', 'a', 'Name', 'café', '日本', 'x' * 200, '"\\q'])
            if t == 'bytes':
                return bytes(rng.randrange(256) for _ in range(rng.choice([0, 1, 16, 32, 130])))
            if t in ('uint32', 'fixed32'):
                return rng.choice([0, 1, 127, 128, 300, 16384, U32, rng.randrange(U32 + 1)])
            if t == 'uint64':
                return rng.choice([0, 1, 1 << 35, U64, rng.randrange(U64 + 1)])
            if t == 'sint32':
                return rng.choice([0, -1, 1, -64, 64, -(1 << 31), (1 << 31) - 1])
            if t == 'sint64':
                return rng.choice([0, -1, 1, -(1 << 63), (1 << 63) - 1, -12345678901])
            raise ValueError(t)
        if f.repeated:
            msg[f.name] = [one() for _ in range(rng.choice([0, 1, 3]))]
        else:
            msg[f.name] = one()
    return msg


# ---------------------------------------------------------------------------
# Strings

def sanitize_utf8(data):
    """Return text a writer stores for `data`: each ill-formed sequence becomes
    one U+FFFD, following the Unicode "maximal subpart" practice."""
    return bytes(data).decode('utf-8', errors='replace')


# ---------------------------------------------------------------------------
# Deterministic raw deflate (RFC 1951 fixed-Huffman blocks)

_LEN_BASE = (3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31, 35, 43, 51,
             59, 67, 83, 99, 115, 131, 163, 195, 227, 258)
_LEN_EXTRA = (0,) * 8 + (1,) * 4 + (2,) * 4 + (3,) * 4 + (4,) * 4 + (5,) * 4 + (0,)
_DIST_BASE = (1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193, 257, 385,
              513, 769, 1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577)
_DIST_EXTRA = (0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9,
               10, 10, 11, 11, 12, 12, 13, 13)


class PortableDeflater:
    """Raw deflate whose output depends on nothing but its input, so it does
    not change with the zlib build.

    `flush()` behaves as zlib's Z_SYNC_FLUSH and `finish()` as Z_FINISH.
    """

    WINDOW = 32768
    CHAIN = 16

    def __init__(self):
        self._buf = bytearray()
        self._done = 0
        self._heads = {}
        self._out = bytearray()
        self._acc = 0
        self._nbits = 0

    def _bits(self, value, count):
        self._acc |= value << self._nbits
        self._nbits += count
        while self._nbits >= 8:
            self._out.append(self._acc & 0xFF)
            self._acc >>= 8
            self._nbits -= 8

    def _huff(self, code, length):
        rev = 0
        for _ in range(length):
            rev = (rev << 1) | (code & 1)
            code >>= 1
        self._bits(rev, length)

    def _literal(self, v):
        if v < 144:
            self._huff(0x30 + v, 8)
        elif v < 256:
            self._huff(0x190 + v - 144, 9)
        elif v < 280:
            self._huff(v - 256, 7)
        else:
            self._huff(0xC0 + v - 280, 8)

    def _align(self):
        if self._nbits:
            self._out.append(self._acc & 0xFF)
            self._acc = 0
            self._nbits = 0

    def _block(self, final):
        buf = self._buf
        end = len(buf)
        self._bits(1 if final else 0, 1)
        self._bits(1, 2)
        pos = self._done
        while pos < end:
            best_len, best_dist = 0, 0
            if pos + 3 <= end:
                key = bytes(buf[pos:pos + 3])
                chain = self._heads.get(key, ())
                limit = min(258, end - pos)
                for cand in reversed(chain):
                    dist = pos - cand
                    if dist > self.WINDOW:
                        break
                    n = 3
                    while n < limit and buf[cand + n] == buf[pos + n]:
                        n += 1
                    if n > best_len:
                        best_len, best_dist = n, dist
                        if n == limit:
                            break
            if best_len >= 3:
                if best_len == 258:
                    i = 28
                else:
                    i = bisect.bisect_right(_LEN_BASE, best_len, 0, 28) - 1
                self._literal(257 + i)
                self._bits(best_len - _LEN_BASE[i], _LEN_EXTRA[i])
                d = bisect.bisect_right(_DIST_BASE, best_dist) - 1
                self._huff(d, 5)
                self._bits(best_dist - _DIST_BASE[d], _DIST_EXTRA[d])
                step = best_len
            else:
                self._literal(buf[pos])
                step = 1
            for p in (range(pos, pos + step) if step < 32 else (pos, pos + step - 1)):
                if p + 3 <= end:
                    lst = self._heads.setdefault(bytes(buf[p:p + 3]), [])
                    lst.append(p)
                    if len(lst) > self.CHAIN:
                        del lst[0]
            pos += step
        self._literal(256)
        self._done = end

    def _take(self):
        data = bytes(self._out)
        self._out.clear()
        return data

    def compress(self, data):
        self._buf += data
        return b''

    def flush(self):
        if self._done < len(self._buf):
            self._block(False)
        self._bits(0, 3)
        self._align()
        self._out += b'\x00\x00\xff\xff'
        return self._take()

    def finish(self):
        self._block(True)
        self._align()
        return self._take()


class ZlibDeflater:
    def __init__(self, level=9):
        self._z = zlib.compressobj(level, zlib.DEFLATED, -15, 9)

    def compress(self, data):
        return self._z.compress(data)

    def flush(self):
        return self._z.flush(zlib.Z_SYNC_FLUSH)

    def finish(self):
        return self._z.flush(zlib.Z_FINISH)


def make_deflater(kind):
    if kind == 'portable':
        return PortableDeflater()
    if kind == 'zlib':
        return ZlibDeflater()
    raise ValueError('unknown deflater ' + kind)


def deflate(data, kind='zlib'):
    d = make_deflater(kind)
    return d.compress(data) + d.finish()


# ---------------------------------------------------------------------------
# Seals

SEAL_INTERVAL = 900
SEAL_VERSION = 1


def compute_s0(seal_version, engine, data_version, lnch_body, init_body):
    """The seal chain's start, S0. `engine` is the RecordingInfo.engine string."""
    eng = engine.encode('utf-8') if isinstance(engine, str) else bytes(engine)
    h = hashlib.sha256()
    h.update(b'CNCR')
    h.update(struct.pack('<I', seal_version))
    h.update(struct.pack('<I', len(eng)))
    h.update(eng)
    h.update(struct.pack('<I', data_version))
    h.update(hashlib.sha256(lnch_body).digest())
    h.update(hashlib.sha256(init_body).digest())
    return h.digest()


def sealed_bytes(event):
    if event.get('sealed_payload') is not None:
        return event['sealed_payload']
    return event.get('payload', b'')


def canonical_bytes(frame, record):
    """Canonical bytes of one frame record, or None when it has none."""
    has_hash = record.get('hash') is not None
    shared = [e for e in record.get('events', ()) if not e.get('local')]
    defeated = record.get('defeated', ())
    if not (has_hash or shared or defeated):
        return None
    out = bytearray(struct.pack('<IB', frame, 1 if has_hash else 0))
    if has_hash:
        out += struct.pack('<I', record['hash'])
    out += struct.pack('<I', len(shared))
    for e in shared:
        data = sealed_bytes(e)
        out += struct.pack('<IIiI', e.get('type', 0), e.get('house', 0), e.get('due', 0), len(data))
        out += data
    out += struct.pack('<I', len(defeated))
    for house in sorted(defeated):
        out += struct.pack('<I', house)
    return bytes(out)


def seal_chain(s0, canon, end_frame):
    """Every seal through `end_frame`: the interval seals, then the final seal.

    `canon` is a list of (frame, canonical bytes) in ascending frame order.
    Returns a list of (through, hash). An `end_frame` of None gives no seals.
    """
    if end_frame is None:
        return []
    seals = []
    prev = s0
    idx = 0
    k = 1
    while True:
        through = min(SEAL_INTERVAL * k - 1, end_frame)
        h = hashlib.sha256(prev)
        while idx < len(canon) and canon[idx][0] <= through:
            h.update(canon[idx][1])
            idx += 1
        prev = h.digest()
        seals.append((through, prev))
        if through == end_frame:
            return seals
        k += 1


def interval_seals_before(s0, canon, frame):
    """The interval seals whose `through` is below `frame`, as {through: hash}."""
    k = frame // SEAL_INTERVAL       # intervals fully before `frame`
    if k == 0:
        return {}
    return dict(seal_chain(s0, canon, SEAL_INTERVAL * k - 1))


def chat_fingerprint(salt, house, text):
    """The chat fingerprint: the first 16 bytes of SHA-256("chat-proof-1" || salt ||
    u32 sender house || UTF-8 text)."""
    data = text.encode('utf-8') if isinstance(text, str) else bytes(text)
    return hashlib.sha256(b'chat-proof-1' + bytes(salt) + struct.pack('<I', house) + data).digest()[:16]


# ---------------------------------------------------------------------------
# Container writer

MAGIC = b'CNCR'
CONTAINER_VERSION = 1
ONES = U64
SECTION_HEADER = struct.Struct('<4sIQQ')


def section_header(tag, codec, stored_size, raw_size):
    return SECTION_HEADER.pack(tag, codec, stored_size, raw_size)


def encode_section(tag, body, codec=0, deflater='zlib'):
    if len(tag) != 4:
        raise ValueError('section tag must be 4 bytes')
    stored = body if codec == 0 else deflate(body, deflater)
    return section_header(tag, codec, len(stored), len(body)) + stored


def nominal_micros(records, end_frame):
    """Sum of frame_micros over frames 0..end_frame, or None when no record
    sets it. A value applies from its record to the next change; the first
    value also applies to the frames before its record."""
    changes = [(r[0], r[1]['frame_micros']) for r in records
               if r[1].get('frame_micros') is not None]
    if not changes or end_frame is None or end_frame < 0:
        return None if not changes else 0
    total = 0
    for i, (start, value) in enumerate(changes):
        lo = 0 if i == 0 else start
        hi = changes[i + 1][0] - 1 if i + 1 < len(changes) else end_frame
        hi = min(hi, end_frame)
        if hi >= lo:
            total += (hi - lo + 1) * value
    return total


class ReplayWriter:
    """Builds a CNCR file the way an engine writes one.

    `info` and `launch` are message dicts; `init` and `map_body` are bytes.
    Records are added in ascending frame order with `add`; `skipped` and the
    interval seals are filled in. A record that would encode empty is left
    out, and the next record's `skipped` counts its frame. `close` returns the
    complete file and `open_bytes` the file as a crash would leave it.
    """

    def __init__(self, info, launch, init, map_body=None, *, codec=0, frms_codec=0,
                 deflater='zlib', flush_every=60, head_private=(), auto_seals=True):
        self.info = dict(info)
        self.info.setdefault('seal_version', SEAL_VERSION)
        self.launch = launch
        self.info_body = encode_message('cncreplay.RecordingInfo', self.info)
        self.lnch_body = encode_message('cncreplay.Launch', launch)
        self.init = bytes(init)
        self.map_body = map_body
        self.codec = codec
        self.frms_codec = frms_codec
        self.deflater = deflater
        self.flush_every = flush_every
        self.head_private = list(head_private)
        self.auto_seals = auto_seals
        self.s0 = compute_s0(self.info['seal_version'], self.info.get('engine', ''),
                             self.info.get('data_version', 0), self.lnch_body, self.init)
        self.records = []            # (frame, record dict as written)
        self.canon = []
        self._prev = self.s0
        self._k = 1
        self._pending = []
        self.emitted = []            # interval seals placed in records

    def _finish_interval(self):
        h = hashlib.sha256(self._prev)
        for data in self._pending:
            h.update(data)
        self._prev = h.digest()
        self._pending = []
        seal = (SEAL_INTERVAL * self._k - 1, self._prev)
        self._k += 1
        return seal

    def add(self, frame, record=None, **fields):
        record = dict(record or {}, **fields)
        last = self.records[-1][0] if self.records else -1
        if frame <= last:
            raise ValueError('records must be added in ascending frame order')
        if self.auto_seals:
            new = []
            while SEAL_INTERVAL * self._k - 1 < frame:
                new.append(self._finish_interval())
            if new:
                self.emitted.extend(new)
                record['seals'] = [{'through': t, 'hash': h} for t, h in new] + list(record.get('seals', ()))
        if not encode_message('cncreplay.Frame', record):
            return record           # a writer never writes an empty Frame
        skipped = frame - last - 1
        if skipped:
            record['skipped'] = skipped
        cb = canonical_bytes(frame, record)
        if cb is not None:
            self.canon.append((frame, cb))
            self._pending.append(cb)
        self.records.append((frame, record))
        return record

    def final_seals(self, last_frame):
        """Every seal of RecordingSummary.seals for a game ending on `last_frame`."""
        return seal_chain(self.s0, self.canon, last_frame)

    def head(self):
        out = bytearray(MAGIC + struct.pack('<I', CONTAINER_VERSION))
        out += encode_section(b'INFO', self.info_body, self.codec, self.deflater)
        out += encode_section(b'LNCH', self.lnch_body, self.codec, self.deflater)
        for tag, body, codec in self.head_private:
            out += encode_section(tag, body, codec, self.deflater)
        if self.map_body is not None:
            out += encode_section(b'MAP ', self.map_body, self.codec, self.deflater)
        out += encode_section(b'INIT', self.init, self.codec, self.deflater)
        return bytes(out)

    def record_bytes(self):
        out = []
        for _, record in self.records:
            body = encode_message('cncreplay.Frame', record)
            out.append(encode_varint(len(body)) + body)
        return out

    def frms_stored(self, terminate):
        """Stored FRMS body and its raw size. Without `terminate` the deflate
        stream ends at the last sync flush, as an open section does."""
        records = self.record_bytes()
        raw = b''.join(records) + (b'\x00' if terminate else b'')
        if self.frms_codec == 0:
            return raw, len(raw)
        d = make_deflater(self.deflater)
        out = bytearray()
        for i, rec in enumerate(records, 1):
            out += d.compress(rec)
            if i % self.flush_every == 0:
                out += d.flush()
        if terminate:
            out += d.compress(b'\x00')
            out += d.finish()
        elif len(records) % self.flush_every:
            out += d.flush()
        return bytes(out), len(raw)

    def open_bytes(self, raw_size_written=False):
        """The file as an engine leaves it when it stops before closing."""
        stored, raw = self.frms_stored(terminate=False)
        header = section_header(b'FRMS', self.frms_codec, ONES, raw if raw_size_written else ONES)
        return self.head() + header + stored

    def close(self, summary=None, statistics=None, checkpoints=(), tail_private=(),
              last_frame='auto', terminate=True):
        out = bytearray(self.head())
        stored, raw = self.frms_stored(terminate)
        out += section_header(b'FRMS', self.frms_codec, len(stored), raw) + stored
        for ckpt in checkpoints:
            out += encode_section(b'CKPT', encode_message('cncreplay.Checkpoint', ckpt),
                                  self.codec, self.deflater)
        if statistics is not None:
            out += encode_section(b'STAT', encode_message('cncreplay.Statistics', statistics),
                                  self.codec, self.deflater)
        for tag, body, codec in tail_private:
            out += encode_section(tag, body, codec, self.deflater)
        if summary is not False:
            summary = dict(summary or {})
            if last_frame == 'auto':
                last_frame = self.records[-1][0] if self.records else None
            if 'last_frame' not in summary and last_frame is not None:
                summary['last_frame'] = last_frame
            lf = summary.get('last_frame')
            if 'seals' not in summary:
                summary['seals'] = [{'through': t, 'hash': h} for t, h in self.final_seals(lf)]
            if 'game_micros' not in summary:
                gm = nominal_micros(self.records, lf)
                if gm is not None:
                    summary['game_micros'] = gm
            out += encode_section(b'END ', encode_message('cncreplay.RecordingSummary', summary),
                                  self.codec, self.deflater)
        return bytes(out)


def checkpoint(frame, state):
    msg = {'crc32': zlib.crc32(state) & U32, 'state': state}
    if frame:
        msg['frame'] = frame
    return msg


# ---------------------------------------------------------------------------
# Container reader

SECTION_MESSAGES = {
    b'INFO': 'cncreplay.RecordingInfo',
    b'LNCH': 'cncreplay.Launch',
    b'CKPT': 'cncreplay.Checkpoint',
    b'STAT': 'cncreplay.Statistics',
    b'END ': 'cncreplay.RecordingSummary',
}
SHARED_TAGS = (b'INFO', b'LNCH', b'MAP ', b'INIT', b'FRMS', b'CKPT', b'STAT', b'END ')
ONCE_TAGS = (b'INFO', b'LNCH', b'INIT', b'FRMS')
AT_MOST_ONCE_TAGS = (b'MAP ', b'STAT', b'END ')


def tag_text(tag):
    return ''.join(chr(b) if 32 <= b < 127 else '\\x%02x' % b for b in tag)


class Problem:
    def __init__(self, level, code, message, section=None, frame=None, offset=None):
        self.level = level          # 'error', 'unverified', or 'warning'
        self.code = code
        self.message = message
        self.section = section
        self.frame = frame
        self.offset = offset

    def as_dict(self):
        d = {'level': self.level, 'code': self.code, 'message': self.message}
        if self.section is not None:
            d['section'] = self.section
        if self.frame is not None:
            d['frame'] = self.frame
        if self.offset is not None:
            d['offset'] = self.offset
        return d

    def __str__(self):
        where = []
        if self.section is not None:
            where.append('section ' + self.section)
        if self.frame is not None:
            where.append('frame %d' % self.frame)
        if self.offset is not None:
            where.append('offset %d' % self.offset)
        level = 'not verified' if self.level == 'unverified' else self.level
        return '%s: %s%s' % (level, self.message, (' (' + ', '.join(where) + ')') if where else '')


class Section:
    def __init__(self, tag, codec, stored_size, raw_size, offset):
        self.tag = tag
        self.codec = codec
        self.stored_size = stored_size
        self.raw_size = raw_size
        self.offset = offset            # of the header
        self.is_open = stored_size == ONES
        self.cut = False                # body runs past the end of the file
        self.stored = b''
        self.body = None                # uncompressed body when it was read
        self.message = None

    @property
    def name(self):
        return tag_text(self.tag)

    def as_dict(self):
        return {
            'tag': self.name, 'offset': self.offset, 'codec': self.codec,
            'stored_size': None if self.is_open else self.stored_size,
            'raw_size': None if self.raw_size == ONES else self.raw_size,
            'open': self.is_open, 'cut_short': self.cut,
            'stored_bytes_in_file': len(self.stored),
        }


class FrameRecord:
    __slots__ = ('frame', 'offset', 'msg', 'size')

    def __init__(self, frame, offset, msg, size):
        self.frame = frame
        self.offset = offset            # in the uncompressed frame stream
        self.msg = msg
        self.size = size


class Replay:
    """Everything read from one file. Build it with read_replay()."""

    def __init__(self):
        self.size = 0
        self.readable = False           # the preamble is a CNCR preamble
        self.container_version = None
        self.sections = []
        self.problems = []
        self.info = None
        self.launch = None
        self.lnch_body = None
        self.init_body = None
        self.map_body = None
        self.frms = None
        self.records = []
        self.stream_end = None          # how the frame stream ended
        self.checkpoints = []
        self.statistics = None
        self.summary = None
        self.s0 = None
        self.canon = []
        self.seal_results = []          # dicts: through, where, ok
        self.chat = {'proven': 0, 'unproven': 0}

    def error(self, code, message, **where):
        self.problems.append(Problem('error', code, message, **where))

    def warn(self, code, message, **where):
        self.problems.append(Problem('warning', code, message, **where))

    @property
    def errors(self):
        return [p for p in self.problems if p.level == 'error']

    @property
    def unverified(self):
        return [p for p in self.problems if p.level == 'unverified']

    def not_verified(self, code, message, **where):
        self.problems.append(Problem('unverified', code, message, **where))

    @property
    def warnings(self):
        return [p for p in self.problems if p.level == 'warning']

    @property
    def complete(self):
        return self.summary is not None

    @property
    def status(self):
        if not self.readable:
            return 'unreadable'
        if self.errors:
            return 'failed'
        if self.unverified:
            return 'unverified'
        if not self.complete:
            return 'incomplete'
        return 'complete'

    @property
    def exit_code(self):
        return {'complete': 0, 'failed': 1, 'unreadable': 2, 'incomplete': 3,
                'unverified': 4}[self.status]

    @property
    def last_recorded_frame(self):
        return self.records[-1].frame if self.records else None

    @property
    def end_frame(self):
        """The last frame known to have run: last_frame when the file is
        complete, otherwise the frame of the last complete record."""
        if self.summary is not None:
            return self.summary.get('last_frame')
        return self.last_recorded_frame


def _inflate_closed(stored, raw_size):
    d = zlib.decompressobj(-15)
    try:
        out = d.decompress(stored, min(raw_size + 1, sys.maxsize))
    except zlib.error as exc:
        raise MalformedError('inflate_error', 'deflate data is invalid: %s' % exc) from None
    if len(out) > raw_size or d.unconsumed_tail:
        raise MalformedError('raw_size_mismatch', 'body inflates to more than its raw size %d' % raw_size)
    if not d.eof:
        raise MalformedError('inflate_error', 'deflate stream does not end in the section')
    if d.unused_data:
        raise MalformedError('inflate_error', '%d bytes follow the deflate stream' % len(d.unused_data))
    if len(out) != raw_size:
        raise MalformedError('raw_size_mismatch', 'body inflates to %d bytes, raw size says %d'
                             % (len(out), raw_size))
    return out


def _inflate_partial(stored):
    """Inflate as far as the input goes. Returns (data, ended, error offset,
    bytes after the end of the stream)."""
    d = zlib.decompressobj(-15)
    out = bytearray()
    pos = 0
    chunk_size = 4096
    while pos < len(stored):
        chunk = stored[pos:pos + chunk_size]
        saved = d.copy()
        try:
            out += d.decompress(chunk)
        except zlib.error:
            d = saved
            for i in range(len(chunk)):
                saved = d.copy()
                try:
                    out += d.decompress(chunk[i:i + 1])
                except zlib.error:
                    return bytes(out), False, pos + i, b''
                if d.eof:
                    return bytes(out), True, None, d.unused_data + chunk[i + 1:] + stored[pos + chunk_size:]
            return bytes(out), False, None, b''
        if d.eof:
            return bytes(out), True, None, d.unused_data + stored[pos + chunk_size:]
        pos += chunk_size
    return bytes(out), False, None, b''


def _check_header(replay, sec, limits):
    """Check the sizes of a closed, whole section from its header, for any tag and
    codec. Returns False when the body must not be read: after an error reported here, or
    when the raw size is all ones, which _walk_sections reports."""
    ok = True
    limit = limits.section_raw.get(sec.tag)
    if limit is not None and sec.raw_size != ONES and sec.raw_size > limit:
        replay.error('limit_section', 'raw size %d exceeds the limit of %d' % (sec.raw_size, limit),
                     section=sec.name, offset=sec.offset)
        ok = False
    if sec.codec == 0 and sec.raw_size != ONES and sec.stored_size != sec.raw_size:
        replay.error('size_mismatch', 'stored size %d and raw size %d differ for codec 0'
                     % (sec.stored_size, sec.raw_size), section=sec.name, offset=sec.offset)
        ok = False
    return ok and sec.raw_size != ONES


def _section_body(replay, sec):
    """Uncompressed body of a closed, whole section whose header passed _check_header, or
    None after an error."""
    if sec.codec not in (0, 1):
        return None
    if sec.codec == 0:
        return sec.stored
    try:
        return _inflate_closed(sec.stored, sec.raw_size)
    except MalformedError as exc:
        replay.error(exc.code, exc.message, section=sec.name, offset=sec.offset)
        return None


def _walk_sections(replay, data):
    pos = 8
    while pos < len(data):
        if len(data) - pos < SECTION_HEADER.size:
            replay.warn('cut_header', 'section header cut short by the end of the file; ignored',
                        offset=pos)
            return
        tag, codec, stored, raw = SECTION_HEADER.unpack_from(data, pos)
        sec = Section(tag, codec, stored, raw, pos)
        body_start = pos + SECTION_HEADER.size
        if sec.is_open:
            sec.stored = data[body_start:]
            replay.sections.append(sec)
            if tag != b'FRMS':
                replay.error('open_section', 'only FRMS may be open', section=sec.name, offset=pos)
            return
        if raw == ONES:
            replay.error('open_raw_size', 'raw size is all ones but the stored size is not',
                         section=sec.name, offset=pos)
        if stored > len(data) - body_start:
            sec.cut = True
            sec.stored = data[body_start:]
            if tag == b'FRMS':
                replay.sections.append(sec)
            else:
                replay.warn('section_cut', 'section runs past the end of the file; ignored',
                            section=sec.name, offset=pos)
            return
        sec.stored = data[body_start:body_start + stored]
        replay.sections.append(sec)
        pos = body_start + stored


def _check_order(replay):
    secs = replay.sections
    tags = [s.tag for s in secs]
    for tag in ONCE_TAGS + AT_MOST_ONCE_TAGS:
        if tags.count(tag) > 1:
            replay.error('duplicate_section', 'section occurs %d times' % tags.count(tag),
                         section=tag_text(tag))
    if b'INFO' in tags and tags[0] != b'INFO':
        replay.error('section_order', 'INFO is not the first section', section='INFO')
    if b'LNCH' in tags and (len(tags) < 2 or tags[1] != b'LNCH'):
        replay.error('section_order', 'LNCH is not the second section', section='LNCH')
    frms = tags.index(b'FRMS') if b'FRMS' in tags else None
    end = tags.index(b'END ') if b'END ' in tags else None
    lnch = tags.index(b'LNCH') if b'LNCH' in tags else None
    for i, sec in enumerate(secs):
        t = sec.tag
        if t in (b'MAP ', b'INIT') and frms is not None and i > frms:
            replay.error('section_order', 'section follows FRMS', section=sec.name, offset=sec.offset)
        if t in (b'CKPT', b'STAT') and (frms is None or i < frms):
            replay.error('section_order', 'section does not follow FRMS', section=sec.name,
                         offset=sec.offset)
        if end is not None and i > end:
            replay.error('section_order', 'section follows END', section=sec.name, offset=sec.offset)
        if t not in SHARED_TAGS:
            if lnch is None or i < lnch:
                replay.error('section_order', 'section precedes LNCH', section=sec.name,
                             offset=sec.offset)
            if not (97 <= t[0] <= 122 or 65 <= t[0] <= 90):
                replay.warn('unknown_tag', 'tag is neither shared nor engine-private; skipped',
                            section=sec.name, offset=sec.offset)
    if end is not None and frms is None:
        replay.error('missing_section', 'a complete file has no FRMS section', section='FRMS')


def _parse_frames(replay, stream, complete, limits):
    pos = 0
    prev = -1
    n = len(stream)
    while pos < n:
        start = pos
        try:
            length, pos = decode_varint(stream, pos, n)
        except Truncated:
            if complete:
                replay.error('record_cut', 'frame stream ends inside a record length',
                             section='FRMS', offset=start)
            replay.stream_end = 'cut'
            return
        except MalformedError as exc:
            replay.error(exc.code, 'record length: ' + exc.message, section='FRMS', offset=start)
            replay.stream_end = 'error'
            return
        if length == 0:
            replay.stream_end = 'terminator'
            if pos < n:
                if complete:
                    replay.error('data_after_end', '%d bytes follow the end of the frame stream'
                                 % (n - pos), section='FRMS', offset=pos)
                else:
                    replay.warn('data_after_end', '%d bytes after the end of the frame stream '
                                'were not read' % (n - pos), section='FRMS', offset=pos)
            return
        if length > limits.record:
            replay.error('limit_record', 'frame record of %d bytes exceeds the limit of %d'
                         % (length, limits.record), section='FRMS', offset=start)
            replay.stream_end = 'error'
            return
        if length > n - pos:
            if complete:
                replay.error('record_cut', 'last frame record is cut short', section='FRMS',
                             offset=start)
            replay.stream_end = 'cut'
            return
        try:
            msg = decode_message('cncreplay.Frame', stream, pos, pos + length, limits)
        except MalformedError as exc:
            replay.error(exc.code, 'frame record: ' + exc.message, section='FRMS', offset=start)
            replay.stream_end = 'error'
            return
        frame = prev + 1 + msg.get('skipped', 0)
        if frame > U32:
            replay.error('frame_overflow', 'frame number exceeds 32 bits', section='FRMS', offset=start)
            replay.stream_end = 'error'
            return
        replay.records.append(FrameRecord(frame, start, msg, length))
        prev = frame
        pos += length
    if complete:
        replay.error('frms_unterminated', 'a closed FRMS ends without its zero-length end marker',
                     section='FRMS', offset=n)
        replay.stream_end = 'section_end'
    else:
        replay.stream_end = 'input_end'


def _read_frms(replay, sec, limits):
    complete = not sec.is_open and not sec.cut
    if sec.codec not in (0, 1):
        replay.not_verified('unknown_codec', 'FRMS uses codec %d, which this reader does not know; '
                            'its records and seals are not read' % sec.codec,
                            section='FRMS', offset=sec.offset)
        return
    if complete:
        if sec.codec == 0:
            stream = sec.stored
        else:
            try:
                stream = _inflate_closed(sec.stored, sec.raw_size)
            except MalformedError as exc:
                replay.error(exc.code, exc.message, section='FRMS', offset=sec.offset)
                return
    else:
        if sec.codec == 0:
            stream = sec.stored
        else:
            stream, ended, err, trailing = _inflate_partial(sec.stored)
            if err is not None:
                replay.warn('frms_unreadable_tail', 'deflate data unreadable from stored byte %d; '
                            'the records before it are kept' % err, section='FRMS', offset=sec.offset)
            if trailing:
                replay.warn('data_after_end', '%d bytes follow the deflate stream of an open FRMS'
                            % len(trailing), section='FRMS', offset=sec.offset)
    replay.frms = stream
    _parse_frames(replay, stream, complete, limits)


def read_replay(data, limits=DEFAULT_LIMITS, check=True):
    """Read a whole file from bytes. Never raises on bad input; problems are
    in `Replay.problems` and the classification in `Replay.status`."""
    replay = Replay()
    replay.size = len(data)
    data = bytes(data)
    if len(data) < 8:
        replay.error('short_preamble', 'file is shorter than the 8-byte preamble')
        return replay
    if data[:4] != MAGIC:
        replay.error('bad_magic', 'file does not start with "CNCR"')
        return replay
    replay.container_version = struct.unpack_from('<I', data, 4)[0]
    if replay.container_version != CONTAINER_VERSION:
        replay.error('container_version', 'container version %d is not 1' % replay.container_version)
        return replay
    replay.readable = True
    _walk_sections(replay, data)
    _check_order(replay)

    seen = set()
    for sec in replay.sections:
        whole = not sec.is_open and not sec.cut
        if whole and not _check_header(replay, sec, limits):
            seen.add(sec.tag)
            continue
        if sec.tag == b'FRMS':
            if b'FRMS' not in seen:
                _read_frms(replay, sec, limits)
            seen.add(sec.tag)
            continue
        if not whole:
            continue
        known = sec.tag in SHARED_TAGS
        if not known:
            continue                # skipped by its stored size
        if sec.codec not in (0, 1):
            if sec.tag in (b'INFO', b'LNCH', b'END '):
                replay.error('unknown_codec', 'section uses codec %d; INFO, LNCH, and END must use '
                             'codec 0 or 1' % sec.codec, section=sec.name, offset=sec.offset)
            elif sec.tag == b'INIT':
                replay.not_verified('unknown_codec', 'INIT uses codec %d, which this reader does not '
                                    'know; S0 cannot be computed' % sec.codec,
                                    section=sec.name, offset=sec.offset)
            else:
                replay.warn('unknown_codec', 'section uses codec %d; skipped' % sec.codec,
                            section=sec.name, offset=sec.offset)
            continue
        if sec.tag in seen and sec.tag != b'CKPT':
            continue                # a duplicate, reported by _check_order
        seen.add(sec.tag)
        body = _section_body(replay, sec)
        if body is None:
            continue
        sec.body = body
        if sec.tag == b'INIT':
            replay.init_body = body
            continue
        if sec.tag == b'MAP ':
            replay.map_body = body
            continue
        try:
            msg = decode_message(SECTION_MESSAGES[sec.tag], body, 0, len(body), limits)
        except MalformedError as exc:
            replay.error(exc.code, exc.message, section=sec.name, offset=sec.offset)
            continue
        sec.message = msg
        if sec.tag == b'INFO':
            replay.info = msg
        elif sec.tag == b'LNCH':
            replay.launch = msg
            replay.lnch_body = body
        elif sec.tag == b'CKPT':
            replay.checkpoints.append((sec, msg))
        elif sec.tag == b'STAT':
            replay.statistics = msg
        elif sec.tag == b'END ':
            replay.summary = msg

    for tag, attr in ((b'INFO', 'info'), (b'LNCH', 'launch'), (b'INIT', 'init_body')):
        if getattr(replay, attr) is None and not any(s.tag == tag for s in replay.sections):
            replay.error('missing_section', 'the file has no %s section' % tag_text(tag),
                         section=tag_text(tag))
    if check:
        _check_contents(replay)
    return replay


def read_replay_file(path, limits=DEFAULT_LIMITS):
    with open(path, 'rb') as fh:
        return read_replay(fh.read(), limits)


# ---------------------------------------------------------------------------
# Content and seal checks

def _strictly_ascending(values):
    return all(b > a for a, b in zip(values, values[1:]))


def _check_income_kinds(replay, info):
    """Check the income kind table of INFO and return the set of kind numbers it lists."""
    numbers, ids = set(), set()
    for k in info.get('income_kinds', ()):
        number, kid = k.get('number', 0), k.get('id', '')
        if not kid:
            replay.error('income_kind_id_empty', 'income kind %d has an empty id' % number,
                         section='INFO')
        if number in numbers:
            replay.error('income_kind_number_twice', 'income kind number %d is listed twice' % number,
                         section='INFO')
        elif kid in ids:
            replay.error('income_kind_id_twice', 'income kind id %r is listed twice' % kid,
                         section='INFO')
        numbers.add(number)
        ids.add(kid)
    return numbers


def _check_contents(replay):
    info, launch = replay.info, replay.launch
    kinds = None
    if info is not None:
        kinds = _check_income_kinds(replay, info)
        path = info.get('map_path', '')
        if path and (path.startswith(('/', '\\')) or re.match(r'^[A-Za-z]:', path)
                     or '..' in re.split(r'[\\/]', path)):
            replay.warn('map_path_refused', 'map_path %r is absolute or has a ".." segment; '
                        'a reader does not use it' % path, section='INFO')
        sha = info.get('map_sha256', b'')
        if sha and len(sha) != 32:
            replay.warn('map_sha256_size', 'map_sha256 holds %d bytes, not 32' % len(sha), section='INFO')
        if replay.map_body is not None and len(sha) == 32 and hashlib.sha256(replay.map_body).digest() != sha:
            replay.warn('map_sha256_mismatch', 'map_sha256 is not the SHA-256 of the MAP section',
                        section='INFO')
        colors = [c.get('number', 0) for c in info.get('colors', ())]
        if not _strictly_ascending(colors):
            replay.warn('order', 'colors are not in ascending order', section='INFO')
        bad = [fh.get('path', '') for fh in info.get('files', ())
               if not re.match(r'^[a-z0-9-]+:[0-9a-f]+$', fh.get('hash', ''))]
        if bad:
            replay.warn('file_hash_form', '%d file hash(es) are not "<algorithm>:<lowercase hex>", '
                        'the first for %r' % (len(bad), bad[0]), section='INFO')
        recorder = info.get('recorder')
        if recorder is not None and launch is not None and recorder >= len(launch.get('players', ())):
            replay.warn('recorder_range', 'recorder %d is not an index into Launch.players' % recorder,
                        section='INFO')
    if launch is not None:
        players = launch.get('players', ())
        if launch.get('seed') is None:
            replay.warn('seed_absent', 'Launch.seed is not set', section='LNCH')
        houses = [p.get('house', 0) for p in players]
        if not _strictly_ascending(houses):
            replay.warn('order', 'players are not in ascending house order', section='LNCH')
        keys = [o.get('key', '').encode('utf-8') for o in launch.get('options', ())]
        if not _strictly_ascending(keys):
            replay.warn('order', 'options are not in ascending key order, each key once', section='LNCH')
        for o in launch.get('options', ()):
            if all(o.get(k) is None for k in ('number', 'flag', 'text')):
                replay.warn('option_value', 'option %r has no value' % o.get('key', ''), section='LNCH')
        for p in players:
            allies = p.get('allies', [])
            if not _strictly_ascending(allies) or p.get('house', 0) in allies:
                replay.warn('order', 'allies of house %d are not ascending or list the house itself'
                            % p.get('house', 0), section='LNCH')
        try:
            if encode_message('cncreplay.Launch', launch) != replay.lnch_body:
                replay.warn('noncanonical', 'LNCH is not in the canonical encoding', section='LNCH')
        except ValueError:
            pass
    # Frame stream
    proofs = set()
    seen_defeated = {}
    micros = None
    for i, rec in enumerate(replay.records):
        m = rec.msg
        if i == 0 and m.get('frame_micros') is None:
            replay.warn('frame_micros', 'the first record has no frame_micros', section='FRMS',
                        frame=rec.frame)
        if m.get('frame_micros') is not None:
            if i > 0 and m['frame_micros'] == micros:
                replay.warn('frame_micros', 'frame_micros repeats the previous value', section='FRMS',
                            frame=rec.frame)
            micros = m['frame_micros']
        defeated = m.get('defeated', [])
        if not _strictly_ascending(defeated):
            replay.error('defeated_order', 'defeated houses are not in ascending order, each once',
                         section='FRMS', frame=rec.frame)
        for h in defeated:
            if h in seen_defeated and seen_defeated[h] != rec.frame:
                replay.warn('defeated_twice', 'house %d is listed as defeated on frames %d and %d'
                            % (h, seen_defeated[h], rec.frame), section='FRMS', frame=rec.frame)
            seen_defeated.setdefault(h, rec.frame)
        if kinds is not None:
            for inc in m.get('income', ()):
                if inc.get('kind', 0) not in kinds:
                    replay.error('income_kind_unlisted', 'income uses kind %d, which INFO does not list'
                                 % inc.get('kind', 0), section='FRMS', frame=rec.frame)
        for e in m.get('events', ()):
            if e.get('role') == 1:
                if e.get('local'):
                    replay.warn('local_proof', 'a chat fingerprint event is local', section='FRMS',
                                frame=rec.frame)
                else:
                    proofs.add(e.get('payload', b''))
    for rec in replay.records:
        for msg in rec.msg.get('messages', ()):
            salt = msg.get('salt', b'')
            if not salt:
                continue
            if len(salt) != 16:
                replay.warn('salt_size', 'chat salt holds %d bytes, not 16' % len(salt),
                            section='FRMS', frame=rec.frame)
            fp = chat_fingerprint(salt, msg.get('house', 0), msg.get('text', ''))
            if fp in proofs:
                replay.chat['proven'] += 1
            else:
                replay.chat['unproven'] += 1
                replay.warn('chat_unproven', 'no fingerprint event matches the chat line from house %d'
                            % msg.get('house', 0), section='FRMS', frame=rec.frame)

    # Checkpoints
    prev = None
    for sec, ck in replay.checkpoints:
        frame = ck.get('frame', 0)
        if prev is not None and frame <= prev:
            replay.error('checkpoint_order', 'checkpoint frames are not in ascending order, '
                         'one checkpoint per frame', section='CKPT', frame=frame, offset=sec.offset)
        prev = frame
        if zlib.crc32(ck.get('state', b'')) & U32 != ck.get('crc32', 0):
            replay.error('checkpoint_crc', 'checkpoint state does not match its CRC-32',
                         section='CKPT', frame=frame, offset=sec.offset)

    # Summary consistency
    summary = replay.summary
    last = replay.last_recorded_frame
    if summary is not None:
        lf = summary.get('last_frame')
        if last is not None and (lf is None or last > lf):
            replay.error('record_after_last_frame', 'a record is on frame %d, after last_frame %s'
                         % (last, 'absent' if lf is None else lf), section='END ', frame=last)
        gm = summary.get('game_micros')
        if gm is not None and lf is not None:
            computed = nominal_micros([(r.frame, r.msg) for r in replay.records], lf)
            if computed is not None and computed != gm:
                replay.warn('game_micros', 'game_micros is %d; the frame records give %d'
                            % (gm, computed), section='END ')
    _check_seals(replay)


def _check_seals(replay):
    info = replay.info
    if info is None or replay.lnch_body is None or replay.init_body is None:
        return
    sv = info.get('seal_version', 0)
    if sv != SEAL_VERSION:
        replay.not_verified('unknown_seal_version', 'seal_version %d is not known; the seals are '
                            'neither confirmed nor refuted' % sv, section='INFO')
        return
    replay.s0 = compute_s0(sv, info.get('engine', ''), info.get('data_version', 0),
                           replay.lnch_body, replay.init_body)
    canon = []
    for rec in replay.records:
        cb = canonical_bytes(rec.frame, rec.msg)
        if cb is not None:
            canon.append((rec.frame, cb))
    replay.canon = canon
    if any(p.code == 'record_after_last_frame' for p in replay.errors):
        return
    frms = [s for s in replay.sections if s.tag == b'FRMS']
    if replay.stream_end == 'error' or (frms and replay.frms is None):
        if replay.unverified:
            return
        replay.warn('seals_not_compared', 'the frame stream could not be read to its end; '
                    'seals not compared', section='FRMS')
        return

    # Interval seals in frame records. The chain is built only as far as the seals in the
    # records, so a huge `skipped` or last_frame cannot make it build millions of seals.
    if replay.records:
        carried = sum(len(rec.msg.get('seals', ())) for rec in replay.records)
        built = min(replay.records[-1].frame // SEAL_INTERVAL, carried)
        chain = dict(seal_chain(replay.s0, canon, SEAL_INTERVAL * built - 1)) if built else {}
        prev = -1
        for rec in replay.records:
            found = rec.msg.get('seals', [])
            throughs = [s.get('through', 0) for s in found]
            if not _strictly_ascending(throughs):
                replay.error('seal_order', 'seals in a record are not in ascending order',
                             section='FRMS', frame=rec.frame)
            for s in found:
                t = s.get('through', 0)
                if (t + 1) % SEAL_INTERVAL != 0:
                    replay.error('unexpected_seal', 'a frame record holds a seal through %d, '
                                 'which is not an interval boundary' % t, section='FRMS', frame=rec.frame)
                elif not prev <= t < rec.frame:
                    replay.error('misplaced_seal', 'the seal through %d is not on the first record '
                                 'after its interval' % t, section='FRMS', frame=rec.frame)
                elif t not in chain:
                    pass        # past the built chain only when some record lacks seals, reported below
                elif s.get('hash', b'') != chain[t]:
                    replay.error('seal_mismatch', 'seal through %d does not match: file %s, computed %s'
                                 % (t, s.get('hash', b'').hex(), chain[t].hex()),
                                 section='FRMS', frame=rec.frame)
                    replay.seal_results.append({'through': t, 'where': 'frame %d' % rec.frame, 'ok': False})
                else:
                    replay.seal_results.append({'through': t, 'where': 'frame %d' % rec.frame, 'ok': True})
            first = max(0, prev // SEAL_INTERVAL) + 1
            due = rec.frame // SEAL_INTERVAL - first + 1
            if due > len(found):
                replay.error('missing_seal', 'the record on frame %d carries %d seal(s), but %d interval(s) '
                             'end between the previous record and it' % (rec.frame, len(found), due),
                             section='FRMS', frame=rec.frame)
            else:
                for k in range(first, first + due):
                    t = SEAL_INTERVAL * k - 1
                    if t not in throughs:
                        replay.error('missing_seal', 'the seal through %d is missing from the first record '
                                     'after its interval' % t, section='FRMS', frame=rec.frame)
            prev = rec.frame

    # Every seal in the summary. As above, the chain is built no further than the seals listed.
    summary = replay.summary
    if summary is None:
        return
    last_frame = summary.get('last_frame')
    found = summary.get('seals', [])
    due = 0 if last_frame is None else last_frame // SEAL_INTERVAL + 1
    if due > len(found):
        replay.error('missing_seal', 'RecordingSummary.seals holds %d seal(s); last_frame %d needs %d'
                     % (len(found), last_frame, due), section='END ')
        expected = seal_chain(replay.s0, canon, SEAL_INTERVAL * len(found) - 1) if found else []
        final_index = None
    else:
        expected = seal_chain(replay.s0, canon, last_frame)
        final_index = len(expected) - 1
    for i in range(len(found)):
        t = found[i].get('through', 0)
        if i >= len(expected):
            replay.error('unexpected_seal', 'RecordingSummary.seals holds an extra seal through %d' % t,
                         section='END ', frame=t)
            break
        et, eh = expected[i]
        if t != et:
            replay.error('seal_through', 'RecordingSummary seal %d covers through %d, expected %d'
                         % (i + 1, t, et), section='END ', frame=t)
            break
        ok = found[i].get('hash', b'') == eh
        final = i == final_index
        replay.seal_results.append({'through': t, 'where': 'final' if final else 'summary', 'ok': ok})
        if not ok:
            replay.error('seal_mismatch', '%s seal through %d does not match: file %s, computed %s'
                         % ('final' if final else 'summary', t, found[i].get('hash', b'').hex(), eh.hex()),
                         section='END ', frame=t)


# ---------------------------------------------------------------------------
# list, dump, verify, compare

def _fmt_time(seconds):
    try:
        return datetime.datetime.fromtimestamp(seconds, datetime.timezone.utc).strftime('%Y-%m-%d %H:%M:%S UTC')
    except (OverflowError, ValueError, OSError):
        return str(seconds)


def _fmt_duration(micros):
    if micros is None:
        return 'unknown'
    total = micros // 1000000
    h, rem = divmod(total, 3600)
    m, s = divmod(rem, 60)
    return '%d:%02d:%02d' % (h, m, s) if h else '%d:%02d' % (m, s)


def _short_enum(enum, value):
    """GAME_KIND_SKIRMISH -> 'skirmish'; an unknown value stays a number."""
    name = enum_name(enum, value)
    if isinstance(name, int):
        return name
    prefix = re.sub(r'(?<!^)(?=[A-Z])', '_', enum.split('.')[-1]).upper() + '_'
    return name[len(prefix):].lower() if name.startswith(prefix) else name


def listing(replay):
    """The `list` view as a dict."""
    info = replay.info or {}
    launch = replay.launch or {}
    summary = replay.summary
    records = [(r.frame, r.msg) for r in replay.records]
    outcomes = {}
    if replay.statistics:
        for h in replay.statistics.get('houses', ()):
            outcomes[h.get('house', 0)] = h
    players = []
    for p in launch.get('players', ()):
        house = p.get('house', 0)
        role = 'observer' if p.get('observer') else ('human' if p.get('human') else 'computer')
        entry = {
            'house': house, 'name': p.get('name', ''), 'role': role,
            'country_name': p.get('country_name', ''),
            'color': p.get('resolved_color', p.get('color', 0)),
            'start': p.get('resolved_start'),
            'allies': p.get('allies', []),
        }
        if p.get('ai_level') is not None:
            entry['ai_level'] = p['ai_level']
        if house in outcomes:
            entry['outcome'] = enum_name('cncreplay.Outcome', outcomes[house].get('outcome', 0))
            entry['shown_name'] = outcomes[house].get('name', '')
        players.append(entry)
    if summary is not None:
        last_frame = summary.get('last_frame')
        game_micros = summary.get('game_micros')
        if game_micros is None:
            game_micros = nominal_micros(records, last_frame)
        elapsed = summary.get('elapsed_micros')
    else:
        last_frame = replay.last_recorded_frame
        game_micros = nominal_micros(records, last_frame)
        elapsed = None
    length = elapsed if elapsed is not None else game_micros
    defeated = []
    for f, m in records:
        for h in m.get('defeated', ()):
            defeated.append({'house': h, 'frame': f})
    return {
        'complete': replay.complete,
        'status': replay.status,
        'engine': info.get('engine', ''),
        'engine_version': info.get('engine_version', ''),
        'data_version': info.get('data_version', 0),
        'seal_version': info.get('seal_version', 0),
        'package': info.get('package', ''),
        'package_version': info.get('package_version', ''),
        'recorded_at': info.get('recorded_at', 0),
        'recorded_at_text': _fmt_time(info.get('recorded_at', 0)),
        'kind': _short_enum('cncreplay.GameKind', launch.get('kind', 0)),
        'map_name': info.get('map_name', ''),
        'game_mode': info.get('game_mode', ''),
        'map_path': info.get('map_path', ''),
        'map_embedded': replay.map_body is not None,
        'anonymous': bool(launch.get('anonymous')),
        'recorder': info.get('recorder'),
        'last_frame': last_frame,
        'elapsed_micros': elapsed,
        'game_micros': game_micros,
        'length_micros': length,
        'length_text': _fmt_duration(length),
        'completed': None if summary is None else bool(summary.get('completed')),
        'desync_frame': None if summary is None else summary.get('desync_frame'),
        'players': players,
        'defeated': defeated,
    }


def _print_listing(view, out):
    w = out.write
    w('complete: %s\n' % ('yes' if view['complete'] else 'no (no END section)'))
    w('engine: %s %s (data version %d, seal version %d)\n'
      % (view['engine'], view['engine_version'], view['data_version'], view['seal_version']))
    if view['package']:
        w('package: %s %s\n' % (view['package'], view['package_version']))
    w('recorded: %s\n' % view['recorded_at_text'])
    w('kind: %s\n' % view['kind'])
    w('map: %s\n' % view['map_name'])
    if view['game_mode']:
        w('mode: %s\n' % view['game_mode'])
    w('length: %s, last frame %s\n' % (view['length_text'], view['last_frame']))
    if view['completed'] is not None:
        w('decided: %s\n' % ('yes' if view['completed'] else 'no'))
    if view['desync_frame'] is not None:
        w('desync on frame %d\n' % view['desync_frame'])
    w('players:\n')
    for p in view['players']:
        extra = ''
        if p.get('outcome'):
            extra += ', ' + p['outcome']
        if p['allies']:
            extra += ', allies %s' % ','.join(str(a) for a in p['allies'])
        w('  house %d: %s, %s, %s, color %s%s\n'
          % (p['house'], p['name'] or '(no name)', p['role'], p['country_name'] or '?', p['color'], extra))
    for d in view['defeated']:
        w('house %d defeated on frame %d\n' % (d['house'], d['frame']))


def _hex_limited(data, full):
    if full or len(data) <= 256:
        return data.hex()
    return data[:256].hex() + '... (%d bytes)' % len(data)


def dump_view(replay, full=False):
    sections = []
    for sec in replay.sections:
        d = sec.as_dict()
        if sec.message is not None:
            d['message'] = to_display(SECTION_MESSAGES[sec.tag], sec.message)
            if sec.tag == b'CKPT' and not full:
                state = sec.message.get('state', b'')
                d['message']['state'] = _hex_limited(state, full)
        elif sec.body is not None and sec.tag in (b'INIT', b'MAP '):
            d['sha256'] = hashlib.sha256(sec.body).hexdigest()
            d['body'] = _hex_limited(sec.body, full)
        sections.append(d)
    frames = []
    for rec in replay.records:
        d = {'frame': rec.frame, 'offset': rec.offset, 'size': rec.size}
        d.update(to_display('cncreplay.Frame', rec.msg))
        frames.append(d)
    return {
        'size': replay.size,
        'container_version': replay.container_version,
        'status': replay.status,
        's0': None if replay.s0 is None else replay.s0.hex(),
        'sections': sections,
        'frame_stream_end': replay.stream_end,
        'frames': frames,
        'problems': [p.as_dict() for p in replay.problems],
    }


def _render(value, indent, out, key=None):
    pad = '  ' * indent
    prefix = pad + (key + ':' if key is not None else '')
    if isinstance(value, dict):
        if not value:
            out.write(prefix + ' {}\n')
            return
        if key is not None:
            out.write(prefix + '\n')
            indent += 1
        for k, v in value.items():
            _render(v, indent, out, k)
    elif isinstance(value, list):
        if not value:
            out.write(prefix + ' []\n')
            return
        if all(not isinstance(v, (dict, list)) for v in value):
            out.write(prefix + ' [' + ', '.join(str(v) for v in value) + ']\n')
            return
        out.write(prefix + '\n')
        for v in value:
            out.write('  ' * (indent + 1) + '-\n')
            _render(v, indent + 2, out)
    else:
        out.write(prefix + ' ' + str(value) + '\n')


def verify_view(replay):
    matched = sum(1 for s in replay.seal_results if s['ok'])
    return {
        'status': replay.status,
        'exit_code': replay.exit_code,
        's0': None if replay.s0 is None else replay.s0.hex(),
        'records': len(replay.records),
        'last_frame': replay.end_frame,
        'seals_checked': len(replay.seal_results),
        'seals_matched': matched,
        'seals': replay.seal_results,
        'chat': dict(replay.chat),
        'complete': replay.complete,
        'errors': [p.as_dict() for p in replay.errors],
        'not_verified': [p.as_dict() for p in replay.unverified],
        'warnings': [p.as_dict() for p in replay.warnings],
    }


def _first_difference(name, a, b, path=''):
    spec = MESSAGES[name]
    for f in spec.fields:
        va, vb = field_value(name, a, f.name), field_value(name, b, f.name)
        p = (path + '.' if path else '') + f.name
        if f.repeated:
            for i in range(max(len(va), len(vb))):
                if i >= len(va) or i >= len(vb):
                    return '%s: %d entries against %d' % (p, len(va), len(vb))
                if f.kind == 'message':
                    d = _first_difference(f.type, va[i], vb[i], '%s[%d]' % (p, i))
                    if d:
                        return d
                elif va[i] != vb[i]:
                    return '%s[%d]: %r against %r' % (p, i, va[i], vb[i])
        elif f.kind == 'message' and va is not None and vb is not None:
            d = _first_difference(f.type, va, vb, p)
            if d:
                return d
        elif va != vb:
            return '%s: %r against %r' % (p, va, vb)
    if a.get('_unknown', []) != b.get('_unknown', []):
        return (path or name) + ': fields this reader does not know differ'
    return None


def _first_byte_difference(a, b):
    for i in range(min(len(a), len(b))):
        if a[i] != b[i]:
            return i
    return min(len(a), len(b)) if len(a) != len(b) else None


def _describe_frame_difference(fa, fb):
    if fa is None or fb is None:
        return 'canonical bytes present in only one copy'
    ha = struct.unpack_from('<IB', fa)
    hb = struct.unpack_from('<IB', fb)
    if ha[1] != hb[1]:
        return 'a frame hash is present in only one copy'
    if ha[1] and fa[5:9] != fb[5:9]:
        return 'frame hash %08x against %08x' % (struct.unpack_from('<I', fa, 5)[0],
                                                  struct.unpack_from('<I', fb, 5)[0])
    return 'the shared events or defeated houses differ, from byte %d of the canonical bytes' \
        % _first_byte_difference(fa, fb)


def compare_replays(a, b):
    result = {'same_game': None, 'differences': [], 'first_frame': None, 'first_interval': None,
              'compared_through': None, 'exit_code': 2}
    for label, r in (('a', a), ('b', b)):
        if not r.readable:
            result['differences'].append('%s is not a readable CNCR file' % label)
            return result
        if r.s0 is None:
            why = [p.message for p in r.unverified] or ['it lacks a readable INFO, LNCH, or INIT']
            result['differences'].append('%s: S0 cannot be computed: %s' % (label, why[0]))
            return result
    result['s0'] = {'a': a.s0.hex(), 'b': b.s0.hex()}
    result['same_game'] = a.s0 == b.s0
    if not result['same_game']:
        ia, ib = a.info, b.info
        for key in ('seal_version', 'engine', 'data_version'):
            if field_value('cncreplay.RecordingInfo', ia, key) != field_value('cncreplay.RecordingInfo', ib, key):
                result['differences'].append('RecordingInfo.%s differs' % key)
        if a.lnch_body != b.lnch_body:
            d = None
            if a.launch is not None and b.launch is not None:
                d = _first_difference('cncreplay.Launch', a.launch, b.launch)
            result['differences'].append('LNCH differs: ' + (d or 'from byte %s'
                                                                % _first_byte_difference(a.lnch_body, b.lnch_body)))
        if a.init_body != b.init_body:
            result['differences'].append('INIT differs from byte %d'
                                         % _first_byte_difference(a.init_body, b.init_body))
    ea, eb = a.end_frame, b.end_frame
    end = None if ea is None or eb is None else min(ea, eb)
    result['compared_through'] = end
    if ea != eb:
        result['differences'].append('a ends on frame %s and b on frame %s' % (ea, eb))
    ca = {f: cb for f, cb in a.canon if end is not None and f <= end}
    cb_ = {f: cb for f, cb in b.canon if end is not None and f <= end}
    for f in sorted(set(ca) | set(cb_)):
        if ca.get(f) != cb_.get(f):
            k = f // SEAL_INTERVAL + 1
            result['first_frame'] = f
            result['first_interval'] = k
            result['interval_frames'] = [SEAL_INTERVAL * (k - 1), SEAL_INTERVAL * k - 1]
            result['differences'].append('canonical bytes first differ on frame %d, in interval %d: %s'
                                         % (f, k, _describe_frame_difference(ca.get(f), cb_.get(f))))
            break
    result['exit_code'] = 0 if result['same_game'] and result['first_frame'] is None else 1
    return result


# ---------------------------------------------------------------------------
# proto3 JSON mapping, for launch requests

_INT_RE = re.compile(r'^-?[0-9]+$')
_64_BIT = ('uint64', 'sint64', 'int64', 'fixed64', 'sfixed64')


def to_proto_json(name, msg):
    """Convert a decoded message to its proto3 JSON form."""
    if msg.get('_unknown'):
        numbers = ', '.join(str(n) for n, _ in msg['_unknown'])
        raise ValueError('%s holds fields this tool does not know (%s); the JSON form cannot hold them'
                         % (name, numbers))
    spec = MESSAGES[name]
    out = {}
    for f in spec.fields:
        if f.name not in msg or msg[f.name] is None:
            continue
        value = msg[f.name]

        def one(v, f=f):
            if f.kind == 'message':
                return to_proto_json(f.type, v)
            if f.kind == 'enum':
                return enum_name(f.type, v)
            if f.type == 'bytes':
                return base64.b64encode(v).decode('ascii')
            if f.type in _64_BIT:
                return str(v)
            return v
        if f.repeated:
            if value:
                out[f.json] = [one(v) for v in value]
        elif f.kind == 'message' or f.has_presence or not _is_default(value):
            out[f.json] = one(value)
    return out


def _json_int(value, path):
    if isinstance(value, bool):
        raise ValueError('%s: integer expected' % path)
    if isinstance(value, int):
        return value
    if isinstance(value, float) and value.is_integer():
        return int(value)
    if isinstance(value, str) and _INT_RE.match(value):
        return int(value)
    raise ValueError('%s: integer expected, got %r' % (path, value))


def from_proto_json(name, obj, path=None):
    """Convert a proto3 JSON object to a message dict. Accepts the JSON name
    and the original field name; refuses unknown fields."""
    path = path or name.split('.')[-1]
    if not isinstance(obj, dict):
        raise ValueError('%s: object expected' % path)
    spec = MESSAGES[name]
    msg = {}
    for key, value in obj.items():
        f = spec.by_json.get(key) or spec.by_name.get(key)
        if f is None:
            raise ValueError('%s: no field named %r' % (path, key))
        if f.name in msg:
            raise ValueError('%s: field %s given twice' % (path, f.name))
        if value is None:
            continue
        p = path + '.' + f.json

        def one(v, p=p, f=f):
            if f.kind == 'message':
                return from_proto_json(f.type, v, p)
            if f.kind == 'enum':
                if isinstance(v, str) and not _INT_RE.match(v):
                    if v not in ENUM_NAMES[f.type]:
                        raise ValueError('%s: %r is not a value of %s' % (p, v, f.type))
                    return ENUM_NAMES[f.type][v]
                return _json_int(v, p)
            if f.type == 'bool':
                if not isinstance(v, bool):
                    raise ValueError('%s: true or false expected' % p)
                return v
            if f.type == 'string':
                if not isinstance(v, str):
                    raise ValueError('%s: string expected' % p)
                return v
            if f.type == 'bytes':
                if not isinstance(v, str):
                    raise ValueError('%s: base64 string expected' % p)
                s = v.replace('-', '+').replace('_', '/')
                s += '=' * (-len(s) % 4)
                try:
                    return base64.b64decode(s, validate=True)
                except ValueError:
                    raise ValueError('%s: invalid base64' % p) from None
            return _json_int(v, p)
        if f.repeated:
            if not isinstance(value, list):
                raise ValueError('%s: array expected' % p)
            msg[f.name] = [one(v) for v in value]
        else:
            msg[f.name] = one(value)
    return msg


LAUNCH_MESSAGES = {
    'LaunchRequest': 'cnclaunch.LaunchRequest',
    'LaunchResult': 'cnclaunch.LaunchResult',
}


def _dropped_empty_messages(name, msg, path=''):
    """Paths of plain message fields that are set but encode to nothing."""
    spec = MESSAGES[name]
    found = []
    for f in spec.fields:
        if f.kind != 'message' or f.name not in msg or msg[f.name] is None:
            continue
        p = (path + '.' if path else '') + f.json
        values = msg[f.name] if f.repeated else [msg[f.name]]
        for v in values:
            if not f.repeated and not f.has_presence and not encode_message(f.type, v):
                found.append(p)
            else:
                found.extend(_dropped_empty_messages(f.type, v, p))
    return found


def launch_warnings(name, msg):
    warnings = ['%s is empty and is left out of the encoding' % p for p in _dropped_empty_messages(name, msg)]
    if name == 'cnclaunch.LaunchRequest':
        if msg.get('version', 0) != 1:
            warnings.append('version is %d; the engine knows version 1' % msg.get('version', 0))
        if not any(msg.get(v) is not None for v in ('play', 'load', 'replay', 'verify', 'capture')):
            warnings.append('no verb is set')
    return warnings


# ---------------------------------------------------------------------------
# Command line

def _load(path):
    try:
        with open(path, 'rb') as fh:
            data = fh.read()
    except OSError as exc:
        return None, str(exc)
    return read_replay(data), None


def _emit_json(obj, out):
    out.write(json.dumps(obj, indent=2, ensure_ascii=False) + '\n')


def cmd_list(args, out):
    replay, err = _load(args.file)
    if replay is None:
        sys.stderr.write('%s: %s\n' % (args.file, err))
        return 2
    if not replay.readable:
        sys.stderr.write('%s: %s\n' % (args.file, replay.errors[0].message))
        return 2
    view = listing(replay)
    if args.json:
        view['problems'] = [p.as_dict() for p in replay.problems]
        _emit_json(view, out)
    else:
        _print_listing(view, out)
        for p in replay.errors:
            out.write(str(p) + '\n')
    return 0 if replay.info is not None and replay.launch is not None else 1


def cmd_dump(args, out):
    replay, err = _load(args.file)
    if replay is None:
        sys.stderr.write('%s: %s\n' % (args.file, err))
        return 2
    if not replay.readable:
        sys.stderr.write('%s: %s\n' % (args.file, replay.errors[0].message))
        return 2
    view = dump_view(replay, args.full)
    if args.json:
        _emit_json(view, out)
    else:
        _render(view, 0, out)
    return 1 if replay.errors else 0


def cmd_verify(args, out):
    replay, err = _load(args.file)
    if replay is None:
        sys.stderr.write('%s: %s\n' % (args.file, err))
        return 2
    view = verify_view(replay)
    if args.json:
        _emit_json(view, out)
        return replay.exit_code
    for p in replay.problems:
        out.write(str(p) + '\n')
    status = replay.status
    if status == 'unreadable':
        out.write('%s: not a readable CNCR file\n' % args.file)
    elif status == 'failed':
        out.write('%s: failed, %d error(s)\n' % (args.file, len(replay.errors)))
    elif status == 'unverified':
        out.write('%s: not verified, %s; %d record(s)\n'
                  % (args.file, 'complete' if replay.complete else 'incomplete (no END section)',
                     len(replay.records)))
    else:
        out.write('%s: %s, %d record(s), %d of %d seal(s) match%s\n'
                  % (args.file, 'complete' if status == 'complete' else 'incomplete (no END section)',
                     len(replay.records), view['seals_matched'], view['seals_checked'],
                     '' if status == 'complete' else ', last recorded frame %s' % replay.last_recorded_frame))
    return replay.exit_code


def cmd_compare(args, out):
    a, err = _load(args.a)
    if a is None:
        sys.stderr.write('%s: %s\n' % (args.a, err))
        return 2
    b, err = _load(args.b)
    if b is None:
        sys.stderr.write('%s: %s\n' % (args.b, err))
        return 2
    result = compare_replays(a, b)
    if args.json:
        _emit_json(result, out)
        return result['exit_code']
    if result['same_game'] is None:
        out.write('cannot compare: %s\n' % '; '.join(result['differences']))
        return 2
    out.write('same game: %s\n' % ('yes' if result['same_game'] else 'no'))
    for d in result['differences']:
        out.write(d + '\n')
    if result['first_frame'] is None:
        out.write('canonical bytes agree through frame %s\n' % result['compared_through'])
    return result['exit_code']


def cmd_launch(args, out):
    name = LAUNCH_MESSAGES[args.message]
    if args.action == 'encode':
        try:
            with open(args.text, 'r', encoding='utf-8') as fh:
                obj = json.load(fh)
            msg = from_proto_json(name, obj)
            data = encode_message(name, msg)
        except (OSError, ValueError) as exc:
            sys.stderr.write('%s: %s\n' % (args.text, exc))
            return 1
        for w in launch_warnings(name, msg):
            sys.stderr.write('warning: %s\n' % w)
        with open(args.out, 'wb') as fh:
            fh.write(data)
        return 0
    try:
        with open(args.file, 'rb') as fh:
            data = fh.read()
        msg = decode_message(name, data)
        obj = to_proto_json(name, msg)
    except OSError as exc:
        sys.stderr.write('%s: %s\n' % (args.file, exc))
        return 2
    except (MalformedError, ValueError) as exc:
        sys.stderr.write('%s: %s\n' % (args.file, exc))
        return 1
    for w in launch_warnings(name, msg):
        sys.stderr.write('warning: %s\n' % w)
    _emit_json(obj, out)
    return 0


def build_parser():
    parser = argparse.ArgumentParser(
        prog='cncreplay.py',
        description='Read, verify, and compare CNCR replay files, and convert launch requests.')
    sub = parser.add_subparsers(dest='command', required=True)

    p = sub.add_parser('list', help='show who played, the map, the date, the length, and the build')
    p.add_argument('file')
    p.add_argument('--json', action='store_true', help='print JSON')
    p.set_defaults(func=cmd_list)

    p = sub.add_parser('dump', help='print every section and frame record')
    p.add_argument('file')
    p.add_argument('--json', action='store_true', help='print JSON')
    p.add_argument('--full', action='store_true', help='print INIT, MAP, and checkpoint state in full')
    p.set_defaults(func=cmd_dump)

    p = sub.add_parser('verify', help='check the container and the seal chain',
                       description='Exit codes: 0 complete and every seal matches; 1 a seal or '
                                   'structural check failed; 2 not a readable CNCR file; 3 no END '
                                   'section and every seal present matches; 4 not verified: the '
                                   'file is not malformed, but its seal_version, or the codec of '
                                   'INIT or FRMS, is one this reader does not know.')
    p.add_argument('file')
    p.add_argument('--json', action='store_true', help='print JSON')
    p.set_defaults(func=cmd_verify)

    p = sub.add_parser('compare', help='check whether two copies hold the same game',
                       description='Exit codes: 0 same game and the sealed contents agree up to the '
                                   'end of the shorter copy; 1 they differ; 2 a file cannot be read '
                                   'far enough to compute S0.')
    p.add_argument('a')
    p.add_argument('b')
    p.add_argument('--json', action='store_true', help='print JSON')
    p.set_defaults(func=cmd_compare)

    p = sub.add_parser('launch', help='convert a launch request between proto3 JSON and binary')
    lsub = p.add_subparsers(dest='action', required=True)
    e = lsub.add_parser('encode', help='write the binary form of a JSON launch message')
    e.add_argument('text', help='JSON file in the proto3 JSON mapping')
    e.add_argument('out', help='binary file to write')
    e.add_argument('--message', choices=sorted(LAUNCH_MESSAGES), default='LaunchRequest')
    d = lsub.add_parser('decode', help='print a binary launch message as JSON')
    d.add_argument('file')
    d.add_argument('--message', choices=sorted(LAUNCH_MESSAGES), default='LaunchRequest')
    p.set_defaults(func=cmd_launch)
    return parser


def main(argv=None, out=None):
    out = out or sys.stdout
    args = build_parser().parse_args(argv)
    return args.func(args, out)


if __name__ == '__main__':
    if hasattr(sys.stdout, 'reconfigure'):
        sys.stdout.reconfigure(encoding='utf-8')
    sys.exit(main())
