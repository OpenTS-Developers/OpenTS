"""Wire codec: canonical encoding, decoder rules, and round trips."""

import random
import unittest
import zlib

from _support import cncreplay as c

H = bytes.fromhex


class VarintTest(unittest.TestCase):
    def test_edge_values(self):
        for value, enc in ((0, '00'), (1, '01'), (127, '7f'), (128, '8001'), (300, 'ac02'),
                           (c.U32, 'ffffffff0f'), (c.U64, 'ffffffffffffffffff01')):
            self.assertEqual(c.encode_varint(value), H(enc))
            self.assertEqual(c.decode_varint(H(enc), 0, len(H(enc))), (value, len(H(enc))))

    def test_refusals(self):
        with self.assertRaises(c.Truncated):
            c.decode_varint(H('80'), 0, 1)
        with self.assertRaises(c.MalformedError) as cm:
            c.decode_varint(H('ffffffffffffffffff02'), 0, 10)
        self.assertEqual(cm.exception.code, 'varint_overflow')
        with self.assertRaises(c.MalformedError):
            c.decode_varint(H('8080808080808080808000'), 0, 11)
        with self.assertRaises(ValueError):
            c.encode_varint(-1)

    def test_non_minimal_varint_is_read(self):
        self.assertEqual(c.decode_message('cncreplay.Point', H('088100')), {'x': -1})

    def test_zigzag(self):
        for v, z in ((0, 0), (-1, 1), (1, 2), (-2, 3), ((1 << 31) - 1, c.U32 - 1), (-(1 << 31), c.U32)):
            self.assertEqual(c.zigzag_encode(v, 32), z)
            self.assertEqual(c.zigzag_decode(z), v)
        self.assertEqual(c.zigzag_encode(-(1 << 63), 64), c.U64)
        with self.assertRaises(ValueError):
            c.zigzag_encode(1 << 31, 32)


class CanonicalBytesTest(unittest.TestCase):
    def enc(self, name, msg):
        return c.encode_message(name, msg).hex()

    def test_launch(self):
        self.assertEqual(self.enc('cncreplay.Launch', {'seed': 0}), '1500000000')
        self.assertEqual(self.enc('cncreplay.Launch', {}), '')
        msg = {'kind': 2, 'seed': 0x12345678,
               'players': [{'house': 1, 'country': -1, 'allies': [0, 2]}]}
        self.assertEqual(self.enc('cncreplay.Launch', msg),
                         '0802' '1578563412' '1a08' '0801' '2801' '52020002')

    def test_fields_in_ascending_number_whatever_the_dict_order(self):
        msg = {'anonymous': True, 'kind': 1, 'seed': 1}
        self.assertEqual(self.enc('cncreplay.Launch', msg), '0801' '1501000000' '2801')

    def test_implicit_zero_is_left_out_and_optional_zero_is_written(self):
        self.assertEqual(self.enc('cncreplay.Player', {'house': 0, 'name': '', 'human': False}), '')
        self.assertEqual(self.enc('cncreplay.Player', {'ai_level': 0, 'resolved_start': 0}), '4800' '6800')
        self.assertEqual(self.enc('cncreplay.HouseSample', {'credits': 0, 'defeated': False}), '1000')
        self.assertEqual(self.enc('cncreplay.RecordingSummary', {'last_frame': 0}), '0800')
        self.assertEqual(self.enc('cncreplay.Frame', {'hash': 0}), '1500000000')

    def test_oneof_member_is_written_when_zero(self):
        self.assertEqual(self.enc('cncreplay.GameOption', {'key': 'A', 'flag': False}), '0a0141' '1800')
        self.assertEqual(self.enc('cncreplay.GameOption', {'key': 'A', 'number': 0}), '0a0141' '1000')
        self.assertEqual(self.enc('cncreplay.GameOption', {'key': 'A', 'text': ''}), '0a0141' '2200')
        with self.assertRaises(ValueError):
            c.encode_message('cncreplay.GameOption', {'number': 1, 'flag': True})

    def test_empty_message_fields(self):
        # optional message set with an empty encoding: written with length 0
        self.assertEqual(self.enc('cncreplay.Frame', {'view': {'camera': {}}}), '2202' '0a00')
        self.assertEqual(self.enc('cncreplay.Frame', {'view': {'selection': {'ids': []}}}), '2202' '1200')
        # plain message field with an empty encoding: left out
        self.assertEqual(self.enc('cncreplay.Frame', {'view': {}}), '')
        self.assertEqual(self.enc('cncreplay.HouseStatistics', {'house': 1, 'final': {'house': 0}}), '0801')
        # oneof message member set and empty: written
        self.assertEqual(self.enc('cnclaunch.LaunchRequest', {'verify': {}}), '2a00')
        # repeated message entries are always written
        self.assertEqual(self.enc('cncreplay.Frame', {'events': [{}]}), '1a00')

    def test_packed_repeated(self):
        self.assertEqual(self.enc('cncreplay.Frame', {'defeated': [1, 300]}), '5203' '01ac02')
        self.assertEqual(self.enc('cncreplay.Frame', {'defeated': []}), '')
        self.assertEqual(self.enc('cncreplay.Frame', {'income': [{'house': 1, 'kind': 2, 'amount': 25}]}),
                         '4206' '080110021832')
        self.assertEqual(self.enc('cncreplay.RecordingInfo', {'income_kinds': [{'number': 2, 'id': 'Crate'}]}),
                         '7a09' '08021205' '4372617465')
        self.assertEqual(self.enc('cncreplay.HouseStatistics', {'crates': [{'id': 'Money', 'count': 2}]}),
                         '6209' '0a05' '4d6f6e6579' '1002')

    def test_bytes_and_optional_bytes(self):
        self.assertEqual(self.enc('cncreplay.Event', {'payload': b''}), '')
        self.assertEqual(self.enc('cncreplay.Event', {'sealed_payload': b''}), '3a00')
        self.assertEqual(self.enc('cncreplay.Event', {'type': 4, 'due': -2, 'payload': b'\x01'}),
                         '0804' '1803' '220101')

    def test_extreme_integers(self):
        self.assertEqual(self.enc('cncreplay.Income', {'amount': -(1 << 63)}), '18' + 'ff' * 9 + '01')
        self.assertEqual(self.enc('cncreplay.RecordingSummary', {'ended_at': c.U64}), '10' + 'ff' * 9 + '01')
        self.assertEqual(self.enc('cncreplay.PlayerColor', {'rgb': 0x00112233}), '1533221100')
        with self.assertRaises(ValueError):
            c.encode_message('cncreplay.Seal', {'through': 1 << 32})

    def test_checkpoint_at_frame_zero_has_no_frame_field(self):
        body = c.encode_message('cncreplay.Checkpoint', c.checkpoint(0, b'abc'))
        self.assertEqual(body[0], 0x15)
        self.assertEqual(body, H('15') + (zlib.crc32(b'abc')).to_bytes(4, 'little') + H('1a03616263'))
        self.assertEqual(c.encode_message('cncreplay.Checkpoint', c.checkpoint(5, b''))[:2], H('0805'))

    def test_strings_are_stored_as_given(self):
        decomposed = 'é'
        self.assertEqual(self.enc('cncreplay.FileHash', {'path': decomposed}), '0a03' + ' 65cc81'.strip())

    def test_unknown_fields_are_merged_in_number_order(self):
        msg = {'skipped': 1, 'defeated': [2],
               '_unknown': [(50, H('9003') + H('07')), (150, H('b209') + H('00'))]}
        self.assertEqual(self.enc('cncreplay.Frame', msg), '0801' '520102' '900307' 'b20900')


class DecoderRulesTest(unittest.TestCase):
    def dec(self, name, hexstr, limits=None):
        data = H(hexstr)
        return c.decode_message(name, data, 0, len(data), limits)

    def code(self, name, hexstr, limits=None):
        with self.assertRaises(c.MalformedError) as cm:
            self.dec(name, hexstr, limits)
        return cm.exception.code

    def test_group_wire_types_are_malformed(self):
        self.assertEqual(self.code('cncreplay.Launch', '9b03' '9c03'), 'group_wire_type')
        self.assertEqual(self.code('cncreplay.Launch', '0b0c'), 'group_wire_type')
        self.assertEqual(self.code('cncreplay.Launch', '9c03'), 'group_wire_type')

    def test_invalid_wire_types_and_tags(self):
        self.assertEqual(self.code('cncreplay.Launch', '0e'), 'bad_wire_type')
        self.assertEqual(self.code('cncreplay.Launch', '9f03'), 'bad_wire_type')
        self.assertEqual(self.code('cncreplay.Launch', '0000'), 'bad_tag')
        self.assertEqual(self.code('cncreplay.Launch', '8080808010'), 'bad_tag')

    def test_known_field_with_another_wire_type_is_skipped(self):
        # skipped (uint32) as length-delimited, hash (fixed32) as a varint, defeated as fixed32
        msg = self.dec('cncreplay.Frame', '0a00' '1001' '5501000000' '0803')
        self.assertEqual(msg['skipped'], 3)
        self.assertNotIn('hash', msg)
        self.assertNotIn('defeated', msg)
        self.assertEqual([n for n, _ in msg['_unknown']], [1, 2, 10])
        self.assertEqual(c.encode_message('cncreplay.Frame', msg).hex(), '0803' '0a00' '1001' '5501000000')
        # a group wire type on a known field stays malformed
        self.assertEqual(self.code('cncreplay.Frame', '0b0c'), 'group_wire_type')
        # the skipped value must still be whole
        with self.assertRaises(c.Truncated):
            self.dec('cncreplay.Frame', '0a05')

    def test_truncation(self):
        for name, hexstr in (('cncreplay.Launch', '1a'), ('cncreplay.Launch', '1a05616263'),
                             ('cncreplay.Launch', '15000000'), ('cncreplay.Launch', '08'),
                             ('cncreplay.Launch', '0880'), ('cncreplay.Frame', '1a0208'),
                             ('cncreplay.Frame', '9003'), ('cncreplay.Frame', '9103000000')):
            with self.assertRaises(c.Truncated, msg=hexstr):
                self.dec(name, hexstr)

    def test_values_that_do_not_fit_the_type_are_narrowed(self):
        big = lambda v: c.encode_varint(v).hex()
        self.assertEqual(self.dec('cncreplay.Seal', '08' + big(1 << 32)), {'through': 0})
        self.assertEqual(self.dec('cncreplay.Seal', '08' + big((1 << 32) + 5)), {'through': 5})
        self.assertEqual(self.dec('cncreplay.Player', '28' + big((1 << 32) + 3)), {'country': -2})
        self.assertEqual(self.dec('cncreplay.Player', '1802'), {'human': True})
        self.assertEqual(self.dec('cncreplay.Player', '18' + big(1 << 40)), {'human': True})
        self.assertEqual(self.dec('cncreplay.Launch', '08' + big(1 << 31)), {'kind': -(1 << 31)})
        self.assertEqual(self.dec('cncreplay.Launch', '08' + big((1 << 32) + 1)), {'kind': 1})
        self.assertEqual(self.dec('cncreplay.RecordingSummary', '10' + big(c.U64)), {'ended_at': c.U64})

    def test_negative_enum_value_is_read(self):
        self.assertEqual(self.dec('cncreplay.Launch', '08' + 'ff' * 9 + '01'), {'kind': -1})
        self.assertEqual(c.encode_message('cncreplay.Launch', {'kind': -1}), H('08' + 'ff' * 9 + '01'))

    def test_last_value_counts(self):
        self.assertEqual(self.dec('cncreplay.Launch', '0801' '0802'), {'kind': 2})
        # a message field given twice is replaced, not merged
        self.assertEqual(self.dec('cncreplay.Frame', '2204' '0a020802' '2204' '0a021004'),
                         {'view': {'camera': {'y': 2}}})

    def test_last_oneof_member_counts(self):
        self.assertEqual(self.dec('cncreplay.GameOption', '1002' '1801'), {'flag': True})

    def test_packed_and_unpacked_repeated_fields(self):
        self.assertEqual(self.dec('cncreplay.Frame', '520201' '02' '5003'), {'defeated': [1, 2, 3]})
        self.assertEqual(self.dec('cncreplay.Frame', '5200'), {'defeated': []})
        self.assertEqual(self.code('cncreplay.Frame', '520280'), 'truncated')

    def test_private_income_fields_are_kept(self):
        data = '4209' '080110021832' 'b0092a'
        msg = self.dec('cncreplay.Frame', data)
        self.assertEqual(msg['income'][0]['kind'], 2)
        self.assertEqual([n for n, _ in msg['income'][0]['_unknown']], [150])
        self.assertEqual(c.encode_message('cncreplay.Frame', msg).hex(), data)

    def test_unknown_fields_are_kept_byte_for_byte(self):
        data = '0801' '900307' 'b209020102' 'f50601020304' 'f9060102030405060708'
        msg = self.dec('cncreplay.Frame', data)
        self.assertEqual([n for n, _ in msg['_unknown']], [50, 150, 110, 111])
        self.assertEqual(c.encode_message('cncreplay.Frame', msg).hex(),
                         '0801' '900307' 'f50601020304' 'f9060102030405060708' 'b209020102')

    def test_invalid_utf8_is_malformed(self):
        self.assertEqual(self.code('cncreplay.Player', '1202c328'), 'invalid_utf8')

    def test_string_limit(self):
        limits = c.Limits(string=3)
        self.assertEqual(self.dec('cncreplay.Player', '1203616263', limits), {'name': 'abc'})
        self.assertEqual(self.code('cncreplay.Player', '120461626364', limits), 'limit_string')
        long_name = c.encode_message('cncreplay.Player', {'name': 'x' * 1025})
        with self.assertRaises(c.MalformedError):
            c.decode_message('cncreplay.Player', long_name, 0, len(long_name), c.DEFAULT_LIMITS)


class RoundTripTest(unittest.TestCase):
    def test_every_message_round_trips(self):
        rng = random.Random(20261116)
        for name in sorted(c.MESSAGES):
            for _ in range(40):
                msg = c.random_message(name, rng)
                data = c.encode_message(name, msg)
                back = c.decode_message(name, data)
                self.assertEqual(c.encode_message(name, back), data, name)
                self.assertEqual(c.decode_message(name, c.encode_message(name, back)), back, name)

    def test_decoded_values_equal_the_input(self):
        msg = {'house': 3, 'credits': -5, 'power_output': 0, 'vehicles': c.U32, 'score': -(1 << 63),
               'defeated': True}
        self.assertEqual(c.decode_message('cncreplay.HouseSample', c.encode_message('cncreplay.HouseSample', msg)), msg)

    def test_proto_json_round_trip(self):
        rng = random.Random(7)
        for name in sorted(c.MESSAGES):
            for _ in range(20):
                data = c.encode_message(name, c.random_message(name, rng))
                obj = c.to_proto_json(name, c.decode_message(name, data))
                self.assertEqual(c.encode_message(name, c.from_proto_json(name, obj)), data, name)

    def test_proto_json_forms(self):
        obj = c.to_proto_json('cncreplay.RecordingInfo',
                              {'map_sha256': b'\xff\x00', 'recorded_at': 5, 'recorder': 0})
        self.assertEqual(obj, {'recordedAt': '5', 'recorder': 0, 'mapSha256': '/wA='})
        msg = c.from_proto_json('cncreplay.Launch', {'kind': 'GAME_KIND_CAMPAIGN', 'anonymous': True,
                                                     'seed': '0'})
        self.assertEqual(msg, {'kind': 1, 'anonymous': True, 'seed': 0})
        with self.assertRaises(ValueError):
            c.from_proto_json('cncreplay.Launch', {'nope': 1})
        with self.assertRaises(ValueError):
            c.from_proto_json('cncreplay.Launch', {'kind': 'GAME_KIND_NONE'})


class Utf8ReplacementTest(unittest.TestCase):
    def test_each_ill_formed_subpart_becomes_one_replacement(self):
        self.assertEqual(c.sanitize_utf8(b'a\xffb'), 'a�b')
        self.assertEqual(c.sanitize_utf8(b'\xc3('), '�(')
        self.assertEqual(c.sanitize_utf8(b'\xe2\x82'), '�')
        self.assertEqual(c.sanitize_utf8(b'\xc0\xaf'), '��')
        self.assertEqual(c.sanitize_utf8(b'\xed\xa0\x80'), '���')
        self.assertEqual(c.sanitize_utf8('café'.encode()), 'café')


class DeflateTest(unittest.TestCase):
    def test_portable_deflate_inflates_with_zlib(self):
        rng = random.Random(3)
        for n in (0, 1, 2, 3, 258, 259, 5000, 70000):
            data = bytes(rng.choice(b'ab\x00\x01xyz') for _ in range(n))
            d = c.PortableDeflater()
            out = d.compress(data[:n // 3]) + d.flush() + d.flush() + d.compress(data[n // 3:]) + d.finish()
            self.assertEqual(zlib.decompress(out, -15), data)

    def test_sync_flush_ends_on_a_byte_boundary_with_an_empty_stored_block(self):
        d = c.PortableDeflater()
        d.compress(b'hello')
        out = d.flush()
        self.assertTrue(out.endswith(b'\x00\x00\xff\xff'))
        z = zlib.decompressobj(-15)
        self.assertEqual(z.decompress(out), b'hello')


if __name__ == '__main__':
    unittest.main()
