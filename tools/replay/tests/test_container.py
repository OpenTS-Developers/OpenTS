"""Container reading, truncation, limits, malformed files, and seals."""

import hashlib
import random
import struct
import unittest
from unittest import mock

from _support import cncreplay as c, sample_file, sample_writer


def section_end(data, tag):
    r = c.read_replay(data, check=False)
    for s in r.sections:
        if s.tag == tag:
            return s.offset + c.SECTION_HEADER.size + len(s.stored)
    raise KeyError(tag)


def frames_of(r):
    return [(rec.frame, c.encode_message('cncreplay.Frame', rec.msg)) for rec in r.records]


def written(w):
    return [(f, c.encode_message('cncreplay.Frame', m)) for f, m in w.records]


def codes(r):
    return [p.code for p in r.errors]


class CutAtEveryOffsetTest(unittest.TestCase):
    def check_cuts(self, data, full_frames, require_complete=True):
        init_end = section_end(data, b'INIT')
        last_count = 0
        for cut in range(len(data) + 1):
            r = c.read_replay(data[:cut])
            got = frames_of(r)
            self.assertEqual(got, full_frames[:len(got)], 'cut %d' % cut)
            self.assertGreaterEqual(len(got), last_count, 'cut %d' % cut)
            last_count = len(got)
            if cut < 8:
                self.assertEqual(r.exit_code, 2, 'cut %d' % cut)
            elif cut < init_end:
                self.assertEqual(r.exit_code, 1, 'cut %d' % cut)
                self.assertEqual(set(codes(r)), {'missing_section'}, 'cut %d' % cut)
            elif cut < len(data) or not require_complete:
                self.assertEqual(r.exit_code, 3, 'cut %d: %s' % (cut, [str(p) for p in r.problems]))
            else:
                self.assertEqual(r.exit_code, 0)

    def test_stored_sections(self):
        w, data = sample_file(codec=0, frms_codec=0)
        self.check_cuts(data, written(w))

    def test_deflated_sections_with_a_flush_per_record(self):
        w, data = sample_file(codec=1, frms_codec=1, flush_every=1)
        self.check_cuts(data, written(w))

    def test_open_frame_stream(self):
        w = sample_writer(frms_codec=1, flush_every=2)
        data = w.open_bytes()
        self.check_cuts(data, written(w), require_complete=False)
        self.assertEqual(len(c.read_replay(data).records), len(w.records))

    def test_open_frame_stream_stored(self):
        w = sample_writer(frms_codec=0)
        data = w.open_bytes(raw_size_written=True)
        self.check_cuts(data, written(w), require_complete=False)

    def test_records_of_every_flush_before_the_cut_survive(self):
        w = sample_writer(frms_codec=1, flush_every=1)
        data = w.open_bytes()
        head = len(w.head()) + c.SECTION_HEADER.size
        stored, _ = w.frms_stored(terminate=False)
        flushes = [i for i in range(len(stored) - 3) if stored[i:i + 4] == b'\x00\x00\xff\xff']
        for k, end in enumerate(flushes, 1):
            r = c.read_replay(data[:head + end + 4])
            self.assertEqual(len(r.records), k)


class ContainerRulesTest(unittest.TestCase):
    def setUp(self):
        self.w, self.data = sample_file()
        self.r = c.read_replay(self.data)

    def parts(self):
        r = c.read_replay(self.data, check=False)
        return [[s.tag, s.codec, s.stored, s.raw_size] for s in r.sections]

    def assemble(self, parts, stored_sizes=None):
        out = bytearray(c.MAGIC + struct.pack('<I', 1))
        for i, (tag, codec, stored, raw) in enumerate(parts):
            size = len(stored) if stored_sizes is None or stored_sizes[i] is None else stored_sizes[i]
            out += c.section_header(tag, codec, size, raw) + stored
        return bytes(out)

    def test_sample_is_valid(self):
        self.assertEqual(self.r.status, 'complete', [str(p) for p in self.r.problems])
        self.assertEqual(self.r.warnings, [])
        self.assertEqual(len(self.r.checkpoints), 2)
        self.assertEqual(self.r.map_body, b'map bytes')

    def test_preamble(self):
        self.assertEqual(c.read_replay(b'').status, 'unreadable')
        self.assertEqual(c.read_replay(b'CNCR\x01\x00\x00').status, 'unreadable')
        self.assertEqual(c.read_replay(b'CNCR\x01\x00\x00\x00').exit_code, 1)
        self.assertEqual(c.read_replay(b'RCNC' + self.data[4:]).exit_code, 2)
        self.assertEqual(c.read_replay(self.data[:4] + b'\x00\x00\x00\x00' + self.data[8:]).exit_code, 2)

    def test_unknown_sections_and_codecs_are_skipped(self):
        p = self.parts()
        p.insert(2, [b'priv', 0, b'abc', 3])
        p.insert(3, [b'Xnew', 0, b'abc', 3])
        p.insert(4, [b'opaq', 5, b'????', 99])
        r = c.read_replay(self.assemble(p))
        self.assertEqual(r.status, 'complete', [str(x) for x in r.problems])

    def test_private_section_before_lnch_is_refused(self):
        p = self.parts()
        p.insert(1, [b'priv', 0, b'abc', 3])
        self.assertIn('section_order', codes(c.read_replay(self.assemble(p))))

    def test_order_and_occurrence(self):
        p = self.parts()
        tags = [x[0] for x in p]
        frms = tags.index(b'FRMS')
        moved = p[:frms] + [p[frms + 1]] + [p[frms]] + p[frms + 2:]       # CKPT before FRMS
        self.assertIn('section_order', codes(c.read_replay(self.assemble(moved))))
        dup = p[:3] + [p[tags.index(b'INIT')]] + p[3:]
        self.assertIn('duplicate_section', codes(c.read_replay(self.assemble(dup))))
        ck = [x for x in p if x[0] == b'CKPT']
        swapped = [x for x in p if x[0] != b'CKPT']
        swapped[tags.index(b'FRMS') + 1:tags.index(b'FRMS') + 1] = ck[::-1]
        self.assertIn('checkpoint_order', codes(c.read_replay(self.assemble(swapped))))
        same = [x for x in p if x[0] != b'CKPT']
        same[tags.index(b'FRMS') + 1:tags.index(b'FRMS') + 1] = [ck[0], ck[0]]
        self.assertIn('checkpoint_order', codes(c.read_replay(self.assemble(same))))
        no_frms = [x for x in p if x[0] != b'FRMS']
        r = c.read_replay(self.assemble(no_frms))
        self.assertIn('missing_section', codes(r))

    def test_open_section_rules(self):
        p = self.parts()
        frms = [x[0] for x in p].index(b'FRMS')
        head = p[:frms + 1]
        # stored all ones, raw real: the writer stopped while closing
        data = self.assemble(head, [None] * frms + [c.ONES])
        r = c.read_replay(data)
        self.assertEqual(r.status, 'incomplete')
        # an open STAT is refused
        stat = [x for x in p if x[0] == b'STAT'][0]
        data = self.assemble(head + [stat], [None] * (frms + 1) + [c.ONES])
        self.assertIn('open_section', codes(c.read_replay(data)))

    def test_closed_frame_stream_rules(self):
        p = self.parts()
        frms = [x[0] for x in p].index(b'FRMS')
        stream = p[frms][2]
        p[frms][2] = stream + b'\x05'
        p[frms][3] += 1
        self.assertIn('data_after_end', codes(c.read_replay(self.assemble(p))))
        p[frms][2] = stream[:-1]
        p[frms][3] = len(stream) - 1
        r = c.read_replay(self.assemble(p))
        self.assertEqual(codes(r), ['frms_unterminated'])
        self.assertEqual(len(r.records), len(self.w.records))

    def test_deflate_errors(self):
        p = self.parts()
        info = p[0][2]
        good = c.deflate(info, 'portable')
        p[0] = [b'INFO', 1, good, len(info)]
        self.assertEqual(c.read_replay(self.assemble(p)).status, 'complete')
        p[0] = [b'INFO', 1, good + b'\x00', len(info)]
        self.assertIn('inflate_error', codes(c.read_replay(self.assemble(p))))
        p[0] = [b'INFO', 1, good[:-2], len(info)]
        self.assertIn('inflate_error', codes(c.read_replay(self.assemble(p))))
        p[0] = [b'INFO', 1, b'\xff' * 10, len(info)]
        self.assertIn('inflate_error', codes(c.read_replay(self.assemble(p))))
        p[0] = [b'INFO', 1, good, len(info) + 1]
        self.assertIn('raw_size_mismatch', codes(c.read_replay(self.assemble(p))))

    def test_open_stream_with_a_corrupt_tail_keeps_the_records_before_it(self):
        w = sample_writer(frms_codec=1, flush_every=1)
        data = w.open_bytes() + b'\x00' * 64
        r = c.read_replay(data)
        self.assertEqual(r.status, 'incomplete')
        self.assertEqual(len(r.records), len(w.records))
        self.assertIn('frms_unreadable_tail', [p.code for p in r.warnings])

    def test_map_path_and_hash_checks(self):
        w = c.ReplayWriter({'engine': 'x', 'map_path': '../maps/a.map', 'map_sha256': bytes(32)},
                           {'seed': 0}, b'i', b'map')
        r = c.read_replay(w.close(summary={}))
        self.assertEqual(r.status, 'complete')
        self.assertEqual({p.code for p in r.warnings}, {'map_path_refused', 'map_sha256_mismatch'})

    def test_lnch_writer_rules_are_warnings(self):
        launch = {'players': [{'house': 2}, {'house': 1, 'allies': [1]}],
                  'options': [{'key': 'b', 'number': 1}, {'key': 'a'}]}
        r = c.read_replay(c.ReplayWriter({'engine': 'x'}, launch, b'i').close(summary={}))
        self.assertEqual(r.status, 'complete')
        self.assertEqual({p.code for p in r.warnings}, {'seed_absent', 'order', 'option_value'})

    def test_noncanonical_lnch_is_a_warning(self):
        p = self.parts()
        lnch = p[1][2]
        p[1] = [b'LNCH', 0, lnch + b'\x08\x80\x00', len(lnch) + 3]
        r = c.read_replay(self.assemble(p))
        self.assertIn('noncanonical', [x.code for x in r.warnings])


class NotVerifiedTest(unittest.TestCase):
    def file(self, seal_version=1, init_codec=0, frms_codec=0, close=True):
        w = sample_writer()
        w.info['seal_version'] = seal_version
        w.info_body = c.encode_message('cncreplay.RecordingInfo', w.info)
        data = w.close(summary={}) if close else w.open_bytes()
        r = c.read_replay(data, check=False)
        out = bytearray(data[:8])
        for s in r.sections:
            codec = {b'INIT': init_codec, b'FRMS': frms_codec}.get(s.tag, s.codec)
            size = c.ONES if s.is_open else len(s.stored)
            raw = c.ONES if s.raw_size == c.ONES else s.raw_size
            out += c.section_header(s.tag, codec, size, raw) + s.stored
        return c.read_replay(bytes(out))

    def test_unknown_or_absent_seal_version(self):
        for version in (0, 2):
            r = self.file(seal_version=version)
            self.assertEqual((r.status, r.exit_code), ('unverified', 4))
            self.assertEqual([p.code for p in r.unverified], ['unknown_seal_version'])
            self.assertEqual(r.errors, [])
            self.assertEqual(r.seal_results, [])
        self.assertEqual(self.file(seal_version=2, close=False).exit_code, 4)

    def test_unknown_codec_on_init_or_frms(self):
        for kw in ({'init_codec': 7}, {'frms_codec': 7}):
            r = self.file(**kw)
            self.assertEqual(r.exit_code, 4, kw)
            self.assertEqual([p.code for p in r.unverified], ['unknown_codec'])

    def test_a_failed_check_outranks_not_verified(self):
        w = sample_writer()
        w.info['seal_version'] = 2
        w.info_body = c.encode_message('cncreplay.RecordingInfo', w.info)
        r = c.read_replay(w.close(summary={'last_frame': 3}))
        self.assertEqual(r.exit_code, 1)

    def test_unknown_codec_on_info_is_malformed(self):
        w, data = sample_file()
        bad = bytearray(data)
        bad[12:16] = (7).to_bytes(4, 'little')
        self.assertEqual(c.read_replay(bytes(bad)).exit_code, 1)


class LimitsTest(unittest.TestCase):
    def test_default_limits_are_section_10(self):
        L = c.DEFAULT_LIMITS
        self.assertEqual(L.section_raw, {b'INFO': 1 << 20, b'LNCH': 1 << 20, b'END ': 1 << 20,
                                         b'MAP ': 32 << 20, b'INIT': 32 << 20, b'STAT': 32 << 20,
                                         b'CKPT': 64 << 20})
        self.assertEqual((L.record, L.events, L.payload, L.string, L.players, L.houses, L.income_kinds),
                         (1 << 20, 16384, 4096, 1024, 64, 64, 4096))

    def test_section_raw_size_is_refused_before_inflating(self):
        w, data = sample_file()
        for tag, limit in c.DEFAULT_LIMITS.section_raw.items():
            r = c.read_replay(data, check=False)
            out = bytearray(data[:8])
            for s in r.sections:
                stored = s.stored
                if s.tag == tag:
                    out += c.section_header(tag, 1, 4, limit + 1) + b'\xff\xff\xff\xff'
                else:
                    out += c.section_header(s.tag, s.codec, len(stored), s.raw_size) + stored
            if tag not in [s.tag for s in r.sections]:
                continue
            got = codes(c.read_replay(bytes(out)))
            self.assertIn('limit_section', got, tag)
            self.assertNotIn('inflate_error', got, tag)

    def test_scaled_limits_at_and_over(self):
        def file_with(**kw):
            launch = {'seed': 0, 'players': kw.get('players', [])}
            w = c.ReplayWriter({'engine': 'x'}, launch, b'i')
            w.add(0, hash=1, frame_micros=1, events=kw.get('events', []))
            stats = {'houses': kw.get('houses', [])}
            return w.close(summary={}, statistics=stats)

        cases = [
            ('events', Limits(events=3), {'events': [{}] * 3}, {'events': [{}] * 4}, 'limit_events'),
            ('payload', Limits(payload=8), {'events': [{'payload': bytes(8)}]},
             {'events': [{'payload': bytes(9)}]}, 'limit_payload'),
            ('sealed', Limits(payload=8), {'events': [{'sealed_payload': bytes(8)}]},
             {'events': [{'sealed_payload': bytes(9)}]}, 'limit_payload'),
            ('players', Limits(players=2), {'players': [{'house': 0}, {'house': 1}]},
             {'players': [{'house': 0}, {'house': 1}, {'house': 2}]}, 'limit_players'),
            ('houses', Limits(houses=2), {'houses': [{}, {}]}, {'houses': [{}, {}, {}]}, 'limit_houses'),
        ]
        for label, limits, at, over, code in cases:
            r = c.read_replay(file_with(**at), limits)
            self.assertEqual(r.status, 'complete', (label, [str(p) for p in r.problems]))
            self.assertIn(code, codes(c.read_replay(file_with(**over), limits)), label)

    def test_record_limit(self):
        w = c.ReplayWriter({'engine': 'x'}, {'seed': 0}, b'i')
        w.add(0, hash=1, frame_micros=1, events=[{'payload': bytes(40)}])
        data = w.close(summary={})
        size = len(c.encode_message('cncreplay.Frame', w.records[0][1]))
        self.assertEqual(c.read_replay(data, Limits(record=size)).status, 'complete')
        self.assertIn('limit_record', codes(c.read_replay(data, Limits(record=size - 1))))

    def test_income_kind_limit(self):
        kinds = [{'number': 1, 'id': 'a'}, {'number': 2, 'id': 'b'}, {'number': 3, 'id': 'c'}]
        w = c.ReplayWriter({'engine': 'x', 'income_kinds': kinds}, {'seed': 0}, b'i')
        w.add(0, hash=1, frame_micros=1, income=[{'kind': 3, 'amount': 5}])
        data = w.close(summary={})
        self.assertEqual(c.read_replay(data, Limits(income_kinds=3)).status, 'complete')
        self.assertEqual(codes(c.read_replay(data, Limits(income_kinds=2))), ['limit_income_kinds'])


Limits = c.Limits


class SealTest(unittest.TestCase):
    def test_s0_definition(self):
        lnch, init = b'\x15\x00\x00\x00\x00', b'state'
        expected = hashlib.sha256(b'CNCR' + struct.pack('<II', 1, 6) + b'opents' + struct.pack('<I', 3)
                                  + hashlib.sha256(lnch).digest() + hashlib.sha256(init).digest()).digest()
        self.assertEqual(c.compute_s0(1, 'opents', 3, lnch, init), expected)

    def test_defeated_houses_out_of_order(self):
        w = c.ReplayWriter({'engine': 'x'}, {'seed': 0}, b'i')
        w.add(0, hash=1, frame_micros=1, defeated=[3, 1])
        r = c.read_replay(w.close(summary={}))
        self.assertEqual(codes(r), ['defeated_order'])
        self.assertTrue(all(s['ok'] for s in r.seal_results))
        self.assertEqual(c.canonical_bytes(0, {'defeated': [3, 1]}),
                         c.canonical_bytes(0, {'defeated': [1, 3]}))

    def test_income_uses_only_kinds_info_lists_once(self):
        def read(kinds, *records):
            info = {'engine': 'x', 'income_kinds': [{'number': n, 'id': i} for n, i in kinds]}
            w = c.ReplayWriter(info, {'seed': 0}, b'i')
            for frame, used in enumerate(records):
                w.add(frame, hash=frame + 1, frame_micros=1 if frame == 0 else None,
                      income=[{'kind': n, 'amount': 5} for n in used])
            return c.read_replay(w.close(summary={}))

        r = read([(1, 'Crate'), (2, 'Trigger'), (3, 'Sold')], [1], [], [2, 1])
        self.assertEqual(r.status, 'complete', [str(p) for p in r.problems])
        self.assertEqual(codes(read([], [1])), ['income_kind_unlisted'])
        self.assertEqual(codes(read([(1, 'Crate')], [1], [2])), ['income_kind_unlisted'])
        self.assertEqual(codes(read([(1, 'Crate'), (1, 'Sold')], [1])), ['income_kind_number_twice'])
        self.assertEqual(codes(read([(1, 'Crate'), (2, 'Crate')], [1])), ['income_kind_id_twice'])
        self.assertEqual(codes(read([(1, '')], [1])), ['income_kind_id_empty'])

    def test_record_after_last_frame_with_nothing_sealed(self):
        w = c.ReplayWriter({'engine': 'x'}, {'seed': 0}, b'i')
        w.add(0, hash=1, frame_micros=1)
        w.add(9, view={'camera': {'x': 1}})
        r = c.read_replay(w.close(summary={'last_frame': 4}))
        self.assertEqual(codes(r), ['record_after_last_frame'])

    def test_canonical_bytes(self):
        rec = {'hash': 0xAABBCCDD, 'events': [
            {'type': 1, 'house': 2, 'due': -1, 'payload': b'xy'},
            {'type': 9, 'payload': b'local', 'local': True},
            {'type': 3, 'payload': b'machine', 'sealed_payload': b'', 'role': 1}],
            'defeated': [4], 'samples': [{'house': 0}]}
        expected = (struct.pack('<IB', 7, 1) + struct.pack('<I', 0xAABBCCDD) + struct.pack('<I', 2)
                    + struct.pack('<IIiI', 1, 2, -1, 2) + b'xy' + struct.pack('<IIiI', 3, 0, 0, 0)
                    + struct.pack('<I', 1) + struct.pack('<I', 4))
        self.assertEqual(c.canonical_bytes(7, rec), expected)
        self.assertIsNone(c.canonical_bytes(7, {'events': [{'local': True}], 'samples': [{}]}))
        self.assertEqual(c.canonical_bytes(0, {'defeated': [1]}),
                         struct.pack('<IBII', 0, 0, 0, 1) + struct.pack('<I', 1))

    def test_empty_intervals_seal_the_previous_seal_alone(self):
        s0 = bytes(32)
        seals = c.seal_chain(s0, [], 2000)
        s1 = hashlib.sha256(s0).digest()
        s2 = hashlib.sha256(s1).digest()
        s3 = hashlib.sha256(s2).digest()
        self.assertEqual(seals, [(899, s1), (1799, s2), (2000, s3)])
        self.assertEqual(c.seal_chain(s0, [], None), [])
        self.assertEqual(c.seal_chain(s0, [], 899), [(899, s1)])
        self.assertEqual(c.seal_chain(s0, [], 0), [(0, s1)])

    def test_copies_that_stop_on_different_frames_agree_on_interval_seals(self):
        a = sample_writer()
        b = sample_writer()
        b.add(1500, hash=5)
        sa, sb = a.final_seals(902), b.final_seals(1600)
        self.assertEqual(sa[0], sb[0])
        self.assertNotEqual(sa[-1], sb[1])
        ra = c.read_replay(a.close(summary={}, last_frame=902))
        rb = c.read_replay(b.close(summary={}, last_frame=1600))
        self.assertEqual((ra.status, rb.status), ('complete', 'complete'))
        res = c.compare_replays(ra, rb)
        self.assertTrue(res['same_game'])
        self.assertIsNone(res['first_frame'])

    def test_frame_carried_seals_and_summary(self):
        w = sample_writer()
        record_901 = w.records[3][1]
        self.assertEqual([s['through'] for s in record_901['seals']], [899])
        r = c.read_replay(w.close(summary={}))
        self.assertEqual([(s['through'], s['where'], s['ok']) for s in r.seal_results],
                         [(899, 'frame 901', True), (899, 'summary', True), (902, 'final', True)])

    def test_summary_seal_list_errors(self):
        w = sample_writer()
        seals = [{'through': t, 'hash': h} for t, h in w.final_seals(902)]
        for bad, code in ((seals[:1], 'missing_seal'), (seals + [seals[-1]], 'unexpected_seal'),
                          ([seals[0], dict(seals[1], through=901)], 'seal_through')):
            r = c.read_replay(sample_writer().close(summary={'seals': bad}))
            self.assertIn(code, codes(r))

    def test_seal_on_a_non_boundary_frame_record(self):
        w = c.ReplayWriter({'engine': 'x'}, {'seed': 0}, b'i', auto_seals=False)
        w.add(0, hash=1, frame_micros=1, seals=[{'through': 5, 'hash': bytes(32)}])
        self.assertIn('unexpected_seal', codes(c.read_replay(w.close(summary={}))))

    def test_any_changed_shared_byte_breaks_a_seal(self):
        _, data = sample_file()
        r = c.read_replay(data)
        rng = random.Random(5)
        lnch = [s for s in r.sections if s.tag == b'LNCH'][0]
        start = lnch.offset + c.SECTION_HEADER.size
        for _ in range(20):
            i = start + rng.randrange(len(lnch.stored))
            bad = bytearray(data)
            bad[i] ^= 1 << rng.randrange(8)
            got = c.read_replay(bytes(bad))
            self.assertEqual(got.exit_code, 1)


def sec(tag, body, codec=0, stored=None, raw=None):
    return c.section_header(tag, codec, len(body) if stored is None else stored,
                            len(body) if raw is None else raw) + body


def hand(*sections):
    return c.MAGIC + struct.pack('<I', 1) + b''.join(sections)


INFO = sec(b'INFO', bytes.fromhex('6801'))          # seal_version 1
LNCH = sec(b'LNCH', bytes.fromhex('1500000000'))    # seed 0
INIT = sec(b'INIT', b'')
FRMS = sec(b'FRMS', b'\x00')
END = sec(b'END ', b'')


class HandBuiltFileTest(unittest.TestCase):
    def test_minimal_file_is_complete(self):
        r = c.read_replay(hand(INFO, LNCH, INIT, FRMS, END))
        self.assertEqual(r.status, 'complete', [str(p) for p in r.problems])

    def test_closed_frame_stream_needs_its_end_marker_and_nothing_after(self):
        for frms in (sec(b'FRMS', b''), sec(b'FRMS', b'\x03\x00', codec=1, raw=0)):
            self.assertEqual(codes(c.read_replay(hand(INFO, LNCH, INIT, frms, END))), ['frms_unterminated'])
        frms = sec(b'FRMS', b'\x00\x01')
        self.assertEqual(codes(c.read_replay(hand(INFO, LNCH, INIT, frms, END))), ['data_after_end'])
        open_frms = sec(b'FRMS', b'', stored=c.ONES, raw=c.ONES)
        self.assertEqual(c.read_replay(hand(INFO, LNCH, INIT, open_frms)).status, 'incomplete')

    def test_invalid_deflate_in_the_frame_stream(self):
        r = c.read_replay(hand(INFO, LNCH, INIT, sec(b'FRMS', b'\x07', codec=1, stored=c.ONES, raw=c.ONES)))
        self.assertEqual(r.status, 'incomplete')
        self.assertIn('frms_unreadable_tail', [p.code for p in r.warnings])
        # A closed FRMS whose stored size runs past the end of the file is read as an open one.
        w = sample_writer(frms_codec=1, flush_every=1)
        stored, raw = w.frms_stored(terminate=False)
        data = w.head() + c.section_header(b'FRMS', 1, len(stored) + 100, raw + 1) + stored + b'\x07'
        r = c.read_replay(data)
        self.assertEqual(r.status, 'incomplete')
        self.assertEqual(len(r.records), len(w.records))
        self.assertIn('frms_unreadable_tail', [p.code for p in r.warnings])
        # A closed FRMS that lies whole in the file is malformed.
        r = c.read_replay(hand(INFO, LNCH, INIT, sec(b'FRMS', b'\x07', codec=1, raw=1), END))
        self.assertEqual(codes(r), ['inflate_error'])

    def test_deflate_stream_must_end_at_the_stored_size(self):
        r = c.read_replay(hand(INFO, LNCH, sec(b'INIT', b'\x03\x00\x00', codec=1, raw=0), FRMS, END))
        self.assertEqual(codes(r), ['inflate_error'])
        r = c.read_replay(hand(INFO, LNCH, sec(b'INIT', b'\x03\x00', codec=1, raw=0), FRMS, END))
        self.assertEqual(r.status, 'complete')

    def test_codec0_sizes_hold_for_skipped_sections(self):
        for skipped in (sec(b'FUTR', b'', raw=1), sec(b'priv', b'ab', raw=1)):
            r = c.read_replay(hand(INFO, LNCH, skipped, INIT, FRMS, END))
            self.assertEqual(codes(r), ['size_mismatch'])
        r = c.read_replay(hand(INFO, LNCH, INIT, sec(b'INIT', b'', raw=1), FRMS, END))
        self.assertEqual(sorted(codes(r)), ['duplicate_section', 'size_mismatch'])

    def test_raw_size_limit_holds_for_an_unknown_codec(self):
        limit = 32 << 20
        r = c.read_replay(hand(INFO, LNCH, sec(b'INIT', b'', codec=9, raw=limit), FRMS, END))
        self.assertEqual((r.exit_code, codes(r)), (4, []))
        r = c.read_replay(hand(INFO, LNCH, sec(b'INIT', b'', codec=9, raw=limit + 1), FRMS, END))
        self.assertEqual((r.exit_code, codes(r)), (1, ['limit_section']))
        r = c.read_replay(hand(INFO, LNCH, sec(b'MAP ', b'', codec=2, raw=limit + 1), INIT, FRMS, END))
        self.assertEqual(codes(r), ['limit_section'])

    def test_section_place_and_count_whatever_the_codec(self):
        for tail in (sec(b'FUTR', b''), sec(b'priv', b''), sec(b'xtra', b'', codec=7)):
            r = c.read_replay(hand(INFO, LNCH, INIT, FRMS, END, tail))
            self.assertEqual(codes(r), ['section_order'], tail[:4])
        r = c.read_replay(hand(INFO, sec(b'FUTR', b''), LNCH, INIT, FRMS, END))
        self.assertIn('section_order', codes(r))
        map2 = sec(b'MAP ', b'', codec=2)
        r = c.read_replay(hand(INFO, LNCH, INIT, FRMS, map2, END))
        self.assertEqual(codes(r), ['section_order'])
        r = c.read_replay(hand(INFO, LNCH, map2, map2, INIT, FRMS, END))
        self.assertEqual(codes(r), ['duplicate_section'])

    def test_limits_hold_at_every_occurrence(self):
        info = bytes.fromhex('68010a8108') + b'a' * 1025 + bytes.fromhex('0a00')
        r = c.read_replay(hand(sec(b'INFO', info), LNCH, INIT, FRMS, END))
        self.assertEqual(codes(r), ['limit_string'])
        for field in (4, 7):        # payload, sealed_payload
            event = (c._tag(field, 2) + c.encode_varint(9) + bytes(9)
                     + c._tag(field, 2) + c.encode_varint(1) + b'x')
            frame = c._tag(3, 2) + c.encode_varint(len(event)) + event
            stream = c.encode_varint(len(frame)) + frame + b'\x00'
            r = c.read_replay(hand(INFO, LNCH, INIT, sec(b'FRMS', stream)), c.Limits(payload=8))
            self.assertEqual(codes(r), ['limit_payload'], field)
        r = c.read_replay(hand(INFO, sec(b'LNCH', b'\x1a\x00' * 524288), INIT, FRMS, END))
        self.assertEqual(codes(r), ['limit_players'])

    def test_checkpoint_frame_is_the_decoded_value(self):
        def frames(*bodies):
            r = c.read_replay(hand(INFO, LNCH, INIT, FRMS, *[sec(b'CKPT', bytes.fromhex(b)) for b in bodies],
                                   END))
            return [ck.get('frame', 0) for _, ck in r.checkpoints], codes(r)

        self.assertEqual(frames('088180808010'), ([1], []))
        self.assertEqual(frames('08010802', '0802'), ([2, 2], ['checkpoint_order']))
        self.assertEqual(frames('15000000000801', '15000000000802'), ([1, 2], []))


class WriterTest(unittest.TestCase):
    def test_an_empty_record_is_left_out(self):
        w = c.ReplayWriter({'engine': 'x'}, {'seed': 0}, b'i')
        w.add(0, {})
        w.add(1, hash=1)
        stored, raw = w.frms_stored(terminate=True)
        self.assertEqual(stored.hex(), '070801150100000000')
        r = c.read_replay(w.close(summary={}))
        self.assertEqual(r.status, 'complete', [str(p) for p in r.problems])
        self.assertEqual([x.frame for x in r.records], [1])


class SealLengthTest(unittest.TestCase):
    def read_counting_seals(self, data):
        with mock.patch.object(c, 'seal_chain', wraps=c.seal_chain) as chain:
            r = c.read_replay(data)
        self.assertTrue(all(call.args[2] < 10 * c.SEAL_INTERVAL for call in chain.call_args_list),
                        chain.call_args_list)
        return r

    def test_a_huge_skipped_is_one_missing_seal(self):
        w = c.ReplayWriter({'engine': 'x'}, {'seed': 0}, b'i', auto_seals=False)
        w.add(c.U32, hash=1, frame_micros=1)
        r = self.read_counting_seals(w.close(summary={'seals': []}))
        self.assertEqual([(p.code, p.section) for p in r.errors], [('missing_seal', 'FRMS'),
                                                                   ('missing_seal', 'END ')])

    def test_a_huge_last_frame_is_one_missing_seal(self):
        w = c.ReplayWriter({'engine': 'x'}, {'seed': 0}, b'i')
        r = self.read_counting_seals(w.close(summary={'last_frame': c.U32, 'seals': []}))
        self.assertEqual(codes(r), ['missing_seal'])

    def test_a_record_short_of_seals_still_checks_the_others(self):
        w = c.ReplayWriter({'engine': 'x'}, {'seed': 0}, b'i', auto_seals=False)
        w.add(0, hash=1, frame_micros=1)
        bad = bytearray(w.final_seals(899)[0][1])
        bad[0] ^= 1
        w.add(950, hash=2, seals=[{'through': 899, 'hash': bytes(bad)}])
        w.add(2000, hash=3)
        r = self.read_counting_seals(w.close(summary={}))
        self.assertEqual([(p.code, p.frame) for p in r.errors if p.section == 'FRMS'],
                         [('seal_mismatch', 950), ('missing_seal', 2000)])


class FuzzTest(unittest.TestCase):
    def test_mutations_never_raise(self):
        rng = random.Random(1234)
        bases = [sample_file()[1], sample_file(1, 1, 1)[1], sample_writer(frms_codec=1).open_bytes()]
        for i in range(4000):
            data = bytearray(rng.choice(bases))
            for _ in range(rng.randint(1, 4)):
                op = rng.randrange(4)
                pos = rng.randrange(len(data))
                if op == 0:
                    data[pos] ^= 1 << rng.randrange(8)
                elif op == 1:
                    data.insert(pos, rng.randrange(256))
                elif op == 2:
                    del data[pos]
                else:
                    data[pos:pos + 8] = struct.pack('<Q', rng.choice([c.ONES, 1 << 40, 0, 7]))
            r = c.read_replay(bytes(data))
            self.assertIn(r.exit_code, (0, 1, 2, 3, 4))
            c.listing(r)
            c.dump_view(r)


if __name__ == '__main__':
    unittest.main()
