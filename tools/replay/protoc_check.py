#!/usr/bin/env python3
"""Check cncreplay's codec against protoc.

Usage: protoc_check.py --protoc PATH [--samples N] [--vectors DIR]

For every message body in the test vectors and for N random values of every
message in replay.proto and launch.proto, the script checks that:

- protoc --decode accepts the bytes cncreplay wrote;
- protoc --encode, given the value in protobuf text format, writes the same
  bytes as cncreplay;
- protoc --decode followed by protoc --encode gives the bytes back.

When the google.protobuf package is installed, it also loads the schemas
from a descriptor set and checks cncreplay's proto3 JSON mapping of launch
messages against json_format.

This is a development check.
"""

from __future__ import annotations

import argparse
import json
import os
import random
import subprocess
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import cncreplay as c  # noqa: E402

REPO = os.path.abspath(os.path.join(HERE, '..', '..'))
PROTO_PATHS = ['--proto_path=' + os.path.join(REPO, 'docs', name) for name in ('replay', 'launch')]


def _read(path):
    with open(path, 'rb') as fh:
        return fh.read()


def proto_file(name):
    return 'launch.proto' if name.startswith('cnclaunch.') else 'replay.proto'


def text_escape(data):
    out = []
    for b in data:
        if b in (0x22, 0x5C):
            out.append('\\' + chr(b))
        elif 0x20 <= b < 0x7F:
            out.append(chr(b))
        else:
            out.append('\\%03o' % b)
    return '"' + ''.join(out) + '"'


def to_text(name, msg, indent=0):
    """Protobuf text format for a message dict, or None when it has fields
    text format cannot express (unknown fields)."""
    if msg.get('_unknown'):
        return None
    pad = '  ' * indent
    lines = []
    spec = c.MESSAGES[name]
    for f in spec.fields:
        if f.name not in msg or msg[f.name] is None:
            continue
        value = msg[f.name]
        values = value if f.repeated else [value]
        for v in values:
            if f.kind == 'message':
                body = to_text(f.type, v, indent + 1)
                if body is None:
                    return None
                if not f.repeated and not f.has_presence and not c.encode_message(f.type, v):
                    continue
                lines.append('%s%s {\n%s%s}' % (pad, f.name, body, pad))
                continue
            if not f.repeated and not f.has_presence and c._is_default(v):
                continue
            if f.kind == 'enum':
                s = c.ENUM_VALUES[f.type].get(v, str(v))
            elif f.type == 'bool':
                s = 'true' if v else 'false'
            elif f.type in ('string', 'bytes'):
                s = text_escape(v.encode('utf-8') if isinstance(v, str) else v)
            else:
                s = str(v)
            lines.append('%s%s: %s' % (pad, f.name, s))
    return ''.join(line + '\n' for line in lines)


class Protoc:
    def __init__(self, path):
        self.path = path
        self.calls = 0

    def run(self, mode, name, data):
        self.calls += 1
        proc = subprocess.run(
            [self.path] + PROTO_PATHS + ['--%s=%s' % (mode, name), proto_file(name)],
            input=data, capture_output=True)
        return proc.returncode, proc.stdout, proc.stderr.decode('utf-8', 'replace')


def has_unknown(name, msg):
    if msg.get('_unknown'):
        return True
    for f in c.MESSAGES[name].fields:
        if f.kind == 'message' and msg.get(f.name) is not None:
            values = msg[f.name] if f.repeated else [msg[f.name]]
            if any(has_unknown(f.type, v) for v in values):
                return True
    return False


def check_body(protoc, name, body, label, failures, expect_text=True):
    code, out, err = protoc.run('decode', name, body)
    if code != 0:
        failures.append('%s: protoc --decode=%s refused cncreplay bytes: %s' % (label, name, err.strip()))
        return
    msg = c.decode_message(name, body)
    if c.encode_message(name, msg) != body:
        # A non-canonical body, such as a narrowed value, cannot be written back byte for byte.
        return
    if not has_unknown(name, msg):
        # protoc prints unknown fields by number and cannot read them back.
        code, again, err = protoc.run('encode', name, out)
        if code != 0 or again != body:
            failures.append('%s: protoc decode then encode does not give the bytes back' % label)
    if not expect_text:
        return
    text = to_text(name, msg)
    if text is None:
        return
    code, enc, err = protoc.run('encode', name, text.encode('utf-8'))
    if code != 0:
        failures.append('%s: protoc --encode refused the text form: %s' % (label, err.strip()))
    elif enc != body:
        failures.append('%s: protoc --encode gives %s, cncreplay %s' % (label, enc.hex(), body.hex()))


def check_vectors(protoc, vectors, failures):
    count = 0
    for fname in sorted(os.listdir(vectors)):
        path = os.path.join(vectors, fname)
        if fname.endswith('.bin'):
            name = 'cnclaunch.Result' if 'result' in fname else 'cnclaunch.Request'
            check_body(protoc, name, _read(path), fname, failures)
            count += 1
            continue
        if not fname.endswith('.cncr'):
            continue
        r = c.read_replay(_read(path))
        if r.exit_code not in (0, 3):
            continue
        for sec in r.sections:
            if sec.message is not None:
                check_body(protoc, c.SECTION_MESSAGES[sec.tag], sec.body,
                           '%s %s' % (fname, sec.name), failures)
                count += 1
        for rec in r.records:
            _, p = c.decode_varint(r.frms, rec.offset, len(r.frms))
            body = r.frms[p:p + rec.size]
            check_body(protoc, 'cncreplay.Frame', body, '%s frame %d' % (fname, rec.frame), failures)
            count += 1
    return count


def check_refusals(protoc, vectors, failures, notes):
    """Malformed inputs: say what protoc does with them."""
    r = c.read_replay(_read(os.path.join(vectors, 'malformed_invalid_utf8.cncr')), check=False)
    body = [s for s in r.sections if s.tag == b'LNCH'][0].body
    code, _, err = protoc.run('decode', 'cnclaunch.Game', body)
    notes.append('invalid UTF-8 in Player.name: protoc --decode exit %d (%s)'
                 % (code, (err.strip().splitlines() or ['no message'])[-1]))
    r = c.read_replay(_read(os.path.join(vectors, 'malformed_group_wire_type.cncr')), check=False)
    body = [s for s in r.sections if s.tag == b'LNCH'][0].body
    code, out, err = protoc.run('decode', 'cnclaunch.Game', body)
    notes.append('start/end group pair for unknown field 50 in LNCH: protoc --decode exit %d%s'
                 % (code, ', parsed as an unknown group field' if code == 0 else ''))
    code, out, err = protoc.run('decode', 'cncreplay.Frame', bytes.fromhex('1500000000') + b'\x0a\x00')
    ours = c.decode_message('cncreplay.Frame', bytes.fromhex('1500000000') + b'\x0a\x00')
    notes.append('Frame.skipped with wire type 2: protoc --decode exit %d; cncreplay skips it too '
                 '(unknown fields %r)' % (code, [n for n, _ in ours.get('_unknown', [])]))
    check_narrowing(protoc, failures, notes)
    # A repeated non-repeated message field: the format says the last value counts, and protobuf
    # merges the values.
    data = bytes.fromhex('2204' '0a02' '0802' '2204' '0a02' '1004')
    code, out, err = protoc.run('decode', 'cncreplay.Frame', data)
    notes.append('Frame.view given twice ({camera {x:1}} then {camera {y:2}}): protoc reads %r; '
                 'cncreplay keeps the last value only: %r'
                 % (out.decode().replace('\n', ' ').strip(),
                    c.to_display('cncreplay.Frame', c.decode_message('cncreplay.Frame', data), False)))


NARROWING = (
    ('cnclaunch.Player', 'house (uint32) = 2^32', 1, 1 << 32),
    ('cnclaunch.Player', 'house (uint32) = 2^32 + 5', 1, (1 << 32) + 5),
    ('cnclaunch.Player', 'country (sint32) = 2^32 + 3', 5, (1 << 32) + 3),
    ('cnclaunch.Player', 'human (bool) = 2', 3, 2),
    ('cnclaunch.Player', 'human (bool) = 2^40', 3, 1 << 40),
    ('cnclaunch.Game', 'kind (enum) = 2^31', 1, 1 << 31),
    ('cnclaunch.Game', 'kind (enum) = 2^32 + 1', 1, (1 << 32) + 1),
    ('cncreplay.Seal', 'through (uint32) = 2^64 - 1', 1, c.U64),
)


def check_narrowing(protoc, failures, notes):
    """A varint too large for its field: cncreplay must read what protoc reads."""
    for name, label, number, value in NARROWING:
        data = c.encode_varint(number << 3) + c.encode_varint(value)
        code, out, err = protoc.run('decode', name, data)
        if code != 0:
            failures.append('narrowing %s: protoc --decode refused it: %s' % (label, err.strip()))
            continue
        code, theirs, err = protoc.run('encode', name, out)
        mine = c.encode_message(name, c.decode_message(name, data))
        if theirs != mine:
            failures.append('narrowing %s: protoc reads %r (%s), cncreplay %s'
                            % (label, out.decode().strip(), theirs.hex(), mine.hex()))
        else:
            notes.append('narrowing %s: protoc and cncreplay read %r'
                         % (label, c.to_display(name, c.decode_message(name, data), False)))


def check_random(protoc, samples, seed, failures):
    rng = random.Random(seed)
    count = 0
    for name in sorted(c.MESSAGES):
        for i in range(samples):
            msg = c.random_message(name, rng)
            body = c.encode_message(name, msg)
            check_body(protoc, name, body, '%s random %d' % (name, i), failures)
            back = c.decode_message(name, body)
            if c.encode_message(name, back) != body:
                failures.append('%s random %d: cncreplay round trip changes the bytes' % (name, i))
            count += 1
    return count


def check_json(protoc_path, samples, seed, failures, notes):
    try:
        from google.protobuf import descriptor_pb2, descriptor_pool, json_format, message_factory
    except ImportError:
        notes.append('google.protobuf not installed: JSON mapping not checked')
        return 0
    with tempfile.TemporaryDirectory() as tmp:
        out = os.path.join(tmp, 'set.pb')
        subprocess.run([protoc_path] + PROTO_PATHS + ['--include_imports',
                        '--descriptor_set_out=' + out, 'replay.proto'], check=True)
        fds = descriptor_pb2.FileDescriptorSet.FromString(_read(out))
    pool = descriptor_pool.DescriptorPool()
    for f in fds.file:
        pool.Add(f)
    rng = random.Random(seed)
    count = 0
    for name in sorted(c.MESSAGES):
        cls = message_factory.GetMessageClass(pool.FindMessageTypeByName(name))
        for i in range(samples):
            msg = c.random_message(name, rng)
            body = c.encode_message(name, msg)
            ours = c.to_proto_json(name, c.decode_message(name, body))
            theirs = json_format.MessageToDict(cls.FromString(body))
            if ours != theirs:
                failures.append('%s random %d: JSON differs\n  cncreplay %s\n  protobuf  %s'
                                % (name, i, json.dumps(ours, sort_keys=True), json.dumps(theirs, sort_keys=True)))
            parsed = cls()
            json_format.Parse(json.dumps(ours), parsed)
            if parsed.SerializeToString(deterministic=True) != body:
                failures.append('%s random %d: protobuf encodes cncreplay JSON differently' % (name, i))
            if c.encode_message(name, c.from_proto_json(name, theirs)) != body:
                failures.append('%s random %d: cncreplay encodes protobuf JSON differently' % (name, i))
            count += 1
    return count


def main(argv=None):
    parser = argparse.ArgumentParser(description='Check cncreplay against protoc.')
    parser.add_argument('--protoc', default=os.environ.get('PROTOC'), required=os.environ.get('PROTOC') is None)
    parser.add_argument('--samples', type=int, default=5, help='random values per message')
    parser.add_argument('--seed', type=int, default=1)
    parser.add_argument('--vectors', default=os.path.join(REPO, 'tests', 'replay', 'vectors'))
    args = parser.parse_args(argv)
    protoc = Protoc(args.protoc)
    failures, notes = [], []
    n_vec = check_vectors(protoc, args.vectors, failures)
    n_rand = check_random(protoc, args.samples, args.seed, failures)
    check_refusals(protoc, args.vectors, failures, notes)
    n_json = check_json(args.protoc, args.samples * 4, args.seed + 1, failures, notes)
    print('vector bodies: %d, random values: %d, JSON values: %d, protoc runs: %d'
          % (n_vec, n_rand, n_json, protoc.calls))
    for n in notes:
        print('note: ' + n)
    for f in failures:
        print('FAIL: ' + f)
    print('%d failure(s)' % len(failures))
    return 1 if failures else 0


if __name__ == '__main__':
    sys.exit(main())
