"""The hand-written schema tables match replay.proto and launch.proto."""

import os
import re
import unittest

from _support import PROTO_DIR, cncreplay as c

SCALARS = c.SCALAR_TYPES


class ProtoError(Exception):
    pass


def tokenize(text):
    text = re.sub(r'/\*.*?\*/', ' ', text, flags=re.S)
    text = re.sub(r'//[^\n]*', ' ', text)
    return re.findall(r'"(?:[^"\\]|\\.)*"|[A-Za-z_][A-Za-z0-9_.]*|-?[0-9]+|[{}=;,\[\]()<>]', text)


def parse_proto(text):
    """Parse the subset of proto3 the two files use.

    Returns (package, messages, enums). A message is
    {'fields': [(number, name, type, label, oneof)], 'reserved': [(lo, hi)]}
    with `type` as written; an enum is a list of (name, value).
    """
    toks = tokenize(text)
    pos = 0

    def peek():
        return toks[pos] if pos < len(toks) else None

    def take(expected=None):
        nonlocal pos
        if pos >= len(toks):
            raise ProtoError('unexpected end of file')
        tok = toks[pos]
        if expected is not None and tok != expected:
            raise ProtoError('expected %r, found %r' % (expected, tok))
        pos += 1
        return tok

    package = None
    messages, enums = {}, {}

    def parse_field(label, oneof):
        type_ = take()
        name = take()
        take('=')
        number = int(take())
        if peek() == '[':
            raise ProtoError('field options are not supported')
        take(';')
        return (number, name, type_, label, oneof)

    def parse_enum():
        name = take()
        take('{')
        values = []
        while peek() != '}':
            vname = take()
            take('=')
            values.append((vname, int(take())))
            take(';')
        take('}')
        return name, values

    while pos < len(toks):
        tok = take()
        if tok == 'syntax':
            take('=')
            if take() != '"proto3"':
                raise ProtoError('not proto3')
            take(';')
        elif tok == 'package':
            package = take()
            take(';')
        elif tok == 'import':
            take()
            take(';')
        elif tok == 'enum':
            name, values = parse_enum()
            enums[name] = values
        elif tok == 'message':
            name = take()
            take('{')
            fields, reserved = [], []
            while peek() != '}':
                t = peek()
                if t == 'reserved':
                    take()
                    while True:
                        lo = int(take())
                        hi = lo
                        if peek() == 'to':
                            take()
                            hi = int(take())
                        reserved.append((lo, hi))
                        if peek() == ',':
                            take()
                            continue
                        break
                    take(';')
                elif t == 'oneof':
                    take()
                    oname = take()
                    take('{')
                    while peek() != '}':
                        fields.append(parse_field('', oname))
                    take('}')
                elif t in ('optional', 'repeated'):
                    take()
                    fields.append(parse_field(t, None))
                elif t in ('message', 'enum', 'map', 'extensions', 'option'):
                    raise ProtoError('%s inside a message is not supported' % t)
                else:
                    fields.append(parse_field('', None))
            take('}')
            messages[name] = {'fields': fields, 'reserved': reserved}
        else:
            raise ProtoError('unexpected token %r' % tok)
    return package, messages, enums


def resolve(package, type_, messages, enums):
    if type_ in SCALARS:
        return type_
    full = type_ if '.' in type_ else package + '.' + type_
    return full


def schema_from_proto(texts):
    """Messages and enums of several proto files, with full type names."""
    messages, enums = {}, {}
    for text in texts:
        package, msgs, ens = parse_proto(text)
        for name, values in ens.items():
            enums[package + '.' + name] = list(values)
        for name, m in msgs.items():
            messages[package + '.' + name] = {
                'fields': sorted((n, fname, resolve(package, t, msgs, ens), label, oneof)
                                 for n, fname, t, label, oneof in m['fields']),
                'reserved': sorted(m['reserved']),
            }
    return messages, enums


def schema_from_tables():
    messages = {}
    for name, spec in c.MESSAGES.items():
        messages[name] = {
            'fields': sorted((f.number, f.name, f.type, f.label, f.oneof) for f in spec.fields),
            'reserved': sorted(spec.reserved),
        }
    enums = {name: list(values) for name, values in c.ENUM_TABLE.items()}
    return messages, enums


def differences(proto, tables):
    pm, pe = proto
    tm, te = tables
    out = []
    for name in sorted(set(pm) | set(tm)):
        if name not in tm:
            out.append('message %s is in the proto files and not in the tables' % name)
            continue
        if name not in pm:
            out.append('message %s is in the tables and not in the proto files' % name)
            continue
        a, b = pm[name], tm[name]
        if a['reserved'] != b['reserved']:
            out.append('%s reserved ranges: proto %r, tables %r' % (name, a['reserved'], b['reserved']))
        fa = {f[0]: f for f in a['fields']}
        fb = {f[0]: f for f in b['fields']}
        for n in sorted(set(fa) | set(fb)):
            if fa.get(n) != fb.get(n):
                out.append('%s field %d: proto %r, tables %r' % (name, n, fa.get(n), fb.get(n)))
    for name in sorted(set(pe) | set(te)):
        if pe.get(name) != te.get(name):
            out.append('enum %s: proto %r, tables %r' % (name, pe.get(name), te.get(name)))
    return out


def read(name):
    with open(os.path.join(PROTO_DIR, name), encoding='utf-8') as fh:
        return fh.read()


class SchemaTest(unittest.TestCase):
    def test_tables_match_proto_files(self):
        proto = schema_from_proto([read('replay.proto'), read('launch.proto')])
        self.assertEqual(differences(proto, schema_from_tables()), [])

    def test_every_type_is_known(self):
        proto = schema_from_proto([read('replay.proto'), read('launch.proto')])
        for name, m in proto[0].items():
            for f in m['fields']:
                self.assertTrue(f[2] in SCALARS or f[2] in proto[0] or f[2] in proto[1],
                                '%s.%s has unknown type %s' % (name, f[1], f[2]))

    def test_a_changed_proto_is_detected(self):
        replay = read('replay.proto')
        tables = schema_from_tables()
        launch = read('launch.proto')
        cases = [
            ('uint32 skipped                  = 1;', 'uint32 skipped                  = 11;'),
            ('optional fixed32 hash           = 2;', 'fixed32 hash           = 2;'),
            ('sint32 due     = 3;', 'int32 due     = 3;'),
            ('repeated uint32 defeated        = 10;', 'repeated uint32 defeat        = 10;'),
            ('OUTCOME_DRAW    = 3;', 'OUTCOME_DRAW    = 4;'),
            ('reserved 100 to 199;\n}\n\nmessage FileHash', 'reserved 100 to 198;\n}\n\nmessage FileHash'),
        ]
        for old, new in cases:
            self.assertIn(old, replay)
            changed = replay.replace(old, new, 1)
            proto = schema_from_proto([changed, launch])
            self.assertNotEqual(differences(proto, tables), [], 'change not detected: ' + new)

    def test_parser_refuses_unsupported_syntax(self):
        with self.assertRaises(ProtoError):
            parse_proto('syntax = "proto3"; message A { message B {} }')
        with self.assertRaises(ProtoError):
            parse_proto('syntax = "proto3"; message A { uint32 x = 1 [packed = false]; }')


if __name__ == '__main__':
    unittest.main()
