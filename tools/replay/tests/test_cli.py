"""The command line: output forms and exit codes."""

import contextlib
import io
import json
import os
import tempfile
import unittest

from _support import VECTORS, cncreplay as c


def run(*argv):
    out = io.StringIO()
    with contextlib.redirect_stderr(io.StringIO()):
        code = c.main(list(argv), out)
    return code, out.getvalue()


def vec(name):
    return os.path.join(VECTORS, name)


class CliTest(unittest.TestCase):
    def test_verify_exit_codes(self):
        for name, expected in (('minimal_complete.cncr', 0), ('malformed_final_seal.cncr', 1),
                               ('malformed_bad_magic.cncr', 2), ('incomplete_open.cncr', 3),
                               ('unknown_seal_version.cncr', 4), ('unknown_codec_frms.cncr', 4)):
            code, text = run('verify', vec(name))
            self.assertEqual(code, expected, text)
            code, text = run('verify', '--json', vec(name))
            self.assertEqual(code, expected)
            self.assertEqual(json.loads(text)['exit_code'], expected)

    def test_verify_names_the_failing_seal_and_frame(self):
        code, text = run('verify', '--json', vec('malformed_frame_seal.cncr'))
        err = json.loads(text)['errors'][0]
        self.assertEqual((err['code'], err['frame']), ('seal_mismatch', 950))
        self.assertIn('through 899', err['message'])

    def test_verify_not_verified_output(self):
        code, text = run('verify', vec('unknown_seal_version.cncr'))
        self.assertIn('not verified', text)
        code, text = run('verify', '--json', vec('unknown_seal_version.cncr'))
        view = json.loads(text)
        self.assertEqual((view['status'], view['errors']), ('unverified', []))
        self.assertEqual(view['not_verified'][0]['code'], 'unknown_seal_version')
        code, _ = run('compare', vec('unknown_seal_version.cncr'), vec('desync_a.cncr'))
        self.assertEqual(code, 2)

    def test_verify_missing_file(self):
        code, _ = run('verify', vec('no such file.cncr'))
        self.assertEqual(code, 2)

    def test_list(self):
        code, text = run('list', vec('chain_three_intervals.cncr'))
        self.assertEqual(code, 0)
        self.assertIn('house 1: Bravo', text)
        code, text = run('list', '--json', vec('incomplete_open.cncr'))
        view = json.loads(text)
        self.assertEqual((view['complete'], view['last_frame']), (False, 900))
        self.assertEqual(view['game_micros'], 901 * 33333)
        self.assertEqual([p['name'] for p in view['players']], ['Alpha', 'Bravo'])

    def test_dump(self):
        code, text = run('dump', '--json', vec('sealed_payload_sender.cncr'))
        self.assertEqual(code, 0)
        view = json.loads(text)
        event = view['frames'][1]['events'][0]
        self.assertEqual(event['payload'], '0a000000070011223344aabbccdd')
        self.assertEqual(event['sealed_payload'], '0a00000000001122334400000000')
        code, text = run('dump', vec('chain_three_intervals.cncr'))
        self.assertIn('through: 2699', text)

    def test_compare(self):
        code, text = run('compare', vec('desync_a.cncr'), vec('desync_b.cncr'))
        self.assertEqual(code, 1)
        self.assertIn('frame 1000, in interval 2', text)
        code, text = run('compare', '--json', vec('sealed_payload_sender.cncr'), vec('sealed_payload_receiver.cncr'))
        self.assertEqual((code, json.loads(text)['same_game']), (0, True))
        code, _ = run('compare', vec('malformed_bad_magic.cncr'), vec('desync_a.cncr'))
        self.assertEqual(code, 2)

    def test_launch_encode_and_decode(self):
        with tempfile.TemporaryDirectory() as tmp:
            out = os.path.join(tmp, 'req.bin')
            code, _ = run('launch', 'encode', vec('launch_play.json'), out)
            self.assertEqual(code, 0)
            with open(out, 'rb') as fh, open(vec('launch_play.bin'), 'rb') as ref:
                self.assertEqual(fh.read(), ref.read())
            code, text = run('launch', 'decode', out)
            self.assertEqual(code, 0)
            with open(vec('launch_play.json'), encoding='utf-8') as fh:
                self.assertEqual(json.loads(text), json.load(fh))
            code, text = run('launch', 'decode', '--message', 'LaunchResult', vec('launch_result.bin'))
            self.assertEqual(json.loads(text)['code'], 'RESULT_CODE_OK')
            bad = os.path.join(tmp, 'bad.json')
            with open(bad, 'w', encoding='utf-8') as fh:
                fh.write('{"version": 1, "nonsense": true}')
            code, _ = run('launch', 'encode', bad, out)
            self.assertEqual(code, 1)


if __name__ == '__main__':
    unittest.main()
