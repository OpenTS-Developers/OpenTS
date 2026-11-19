"""The committed test vectors regenerate identically and classify as listed."""

import hashlib
import json
import os
import tempfile
import unittest

from _support import TOOL, VECTORS, cncreplay as c

import importlib.util

_spec = importlib.util.spec_from_file_location('make_vectors', os.path.join(TOOL, 'make_vectors.py'))
make_vectors = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(make_vectors)


def read(name):
    with open(os.path.join(VECTORS, name), 'rb') as fh:
        return fh.read()


def manifest():
    with open(os.path.join(VECTORS, 'vectors.json'), encoding='ascii') as fh:
        return json.load(fh)


class RegenerateTest(unittest.TestCase):
    def test_vectors_regenerate_identically(self):
        with tempfile.TemporaryDirectory() as tmp:
            names = make_vectors.build(tmp)
            committed = sorted(n for n in os.listdir(VECTORS) if n != '.gitattributes')
            self.assertEqual(sorted(names), committed)
            for name in names:
                with open(os.path.join(tmp, name), 'rb') as fh:
                    self.assertEqual(fh.read(), read(name), name)


class ManifestTest(unittest.TestCase):
    def setUp(self):
        self.m = manifest()

    def test_every_file_classifies_as_listed(self):
        for entry in self.m['files']:
            data = read(entry['file'])
            self.assertEqual(hashlib.sha256(data).hexdigest(), entry['sha256'], entry['file'])
            r = c.read_replay(data)
            self.assertEqual(r.exit_code, entry['verify_exit'], (entry['file'], [str(p) for p in r.problems]))
            if 'error_code' in entry:
                self.assertIn(entry['error_code'], [p.code for p in r.errors + r.unverified],
                              entry['file'])
            if 'seals' in entry:
                found = [{'through': s.get('through', 0), 'hash': s.get('hash', b'').hex()}
                         for s in (r.summary or {}).get('seals', [])]
                self.assertEqual(found, entry['seals'], entry['file'])
            if 's0' in entry:
                self.assertEqual(r.s0.hex() if r.s0 else None, entry['s0'], entry['file'])
            if 'checkpoint_frames' in entry:
                self.assertEqual([ck.get('frame', 0) for _, ck in r.checkpoints], entry['checkpoint_frames'],
                                 entry['file'])
            if 'file_hex' in entry:
                self.assertEqual(data.hex(), entry['file_hex'], entry['file'])

    def test_s0_computations(self):
        comps = {x['name']: x for x in self.m['computations']}
        base = comps['s0_base']
        s0 = c.compute_s0(base['seal_version'], base['engine'], base['data_version'],
                          bytes.fromhex(base['lnch']), bytes.fromhex(base['init']))
        self.assertEqual(s0.hex(), base['s0'])
        self.assertEqual(hashlib.sha256(bytes.fromhex(base['preimage'])).hexdigest(), base['s0'])
        empty = comps['s0_empty']
        self.assertEqual(c.compute_s0(1, '', 0, b'', b'').hex(), empty['s0'])

    def test_chain_vector_canonical_bytes(self):
        entry = [e for e in self.m['files'] if e['file'] == 'chain_three_intervals.cncr'][0]
        r = c.read_replay(read(entry['file']))
        self.assertEqual([{'frame': f, 'bytes': b.hex()} for f, b in r.canon], entry['canonical'])
        self.assertEqual([s['through'] for s in entry['seals']], [899, 1799, 2699, 2850])
        rec = [x for x in r.records if x.frame == 2800][0]
        self.assertEqual([s['through'] for s in rec.msg['seals']], [899, 1799, 2699])

    def test_final_on_boundary(self):
        r = c.read_replay(read('final_on_boundary.cncr'))
        self.assertEqual([s['through'] for s in r.summary['seals']], [899])
        self.assertTrue(all('seals' not in x.msg for x in r.records))
        self.assertEqual(r.summary['game_micros'], 899 * 33333 + 66666)

    def test_utf8_and_chat_computations(self):
        comps = {x['name']: x for x in self.m['computations']}
        u = comps['utf8_replacement']
        self.assertEqual(c.sanitize_utf8(bytes.fromhex(u['input'])).encode().hex(), u['output'])
        f = comps['chat_fingerprint']
        fp = c.chat_fingerprint(bytes.fromhex(f['salt']), f['house'], bytes.fromhex(f['text']))
        self.assertEqual(fp.hex(), f['fingerprint'])
        self.assertEqual(c.read_replay(read('chat_fingerprint.cncr')).chat, {'proven': 1, 'unproven': 0})

    def test_checkpoint_and_empty_message_bodies(self):
        r = c.read_replay(read('checkpoint_frame0.cncr'))
        entry = [e for e in self.m['files'] if e['file'] == 'checkpoint_frame0.cncr'][0]
        bodies = [s.body.hex() for s in r.sections if s.tag == b'CKPT']
        self.assertEqual(bodies, [entry['ckpt0_body'], entry['ckpt1_body']])
        self.assertTrue(entry['ckpt0_body'].startswith('15'))
        entry = [e for e in self.m['files'] if e['file'] == 'optional_empty_message.cncr'][0]
        self.assertIn('22040a001200', entry['record_body'])

    def test_compare_computations(self):
        for comp in self.m['computations']:
            if not comp['name'].startswith('compare_'):
                continue
            res = c.compare_replays(c.read_replay(read(comp['a'])), c.read_replay(read(comp['b'])))
            self.assertEqual(res['same_game'], comp['same_game'], comp['name'])
            self.assertEqual(res['exit_code'], comp['exit_code'], comp['name'])
            if 'first_frame' in comp:
                self.assertEqual(res['first_frame'], comp['first_frame'], comp['name'])
            if 'first_interval' in comp:
                self.assertEqual(res['first_interval'], comp['first_interval'], comp['name'])
            if 'lnch_difference' in comp:
                self.assertTrue(any(comp['lnch_difference'] in d for d in res['differences']), res)

    def test_launch_vectors(self):
        data = read('launch_play.bin')
        obj = json.loads(read('launch_play.json'))
        msg = c.from_proto_json('cnclaunch.LaunchRequest', obj)
        self.assertEqual(c.encode_message('cnclaunch.LaunchRequest', msg), data)
        self.assertEqual(c.to_proto_json('cnclaunch.LaunchRequest',
                                         c.decode_message('cnclaunch.LaunchRequest', data)), obj)
        result = c.decode_message('cnclaunch.LaunchResult', read('launch_result.bin'))
        self.assertEqual(result['scenarios'][0]['last_frame'], 0)


if __name__ == '__main__':
    unittest.main()
