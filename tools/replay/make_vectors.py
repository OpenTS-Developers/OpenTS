#!/usr/bin/env python3
"""Write the CNCR test vectors and their manifest.

Usage: make_vectors.py [--out DIR]

Writes the files and vectors.json into DIR, by default tests/replay/vectors
in the repository. The output depends only on this script and cncreplay.py:
compressed sections use cncreplay's deflate encoder, not zlib, so two runs
on any machine write the same bytes. Each file is read back with cncreplay and
the script stops with an error when a file does not classify as its manifest
entry says.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import os
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import cncreplay as c  # noqa: E402

REPO = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))
DEFAULT_OUT = os.path.join(REPO, 'tests', 'replay', 'vectors')

MAP_BYTES = b'[Basic]\nName=Vector Map\n'
INIT_BODY = struct.pack('<I', 0x5EED1234) + hashlib.sha256(MAP_BYTES).digest()
RECORDED_AT = 1767225600          # 2026-01-01 00:00:00 UTC
MICROS_15FPS = 66666
MICROS_30FPS = 33333


def _read(path):
    with open(path, 'rb') as fh:
        return fh.read()


def base_info(**changes):
    info = {
        'engine': 'opents',
        'engine_version': 'vectors-1',
        'data_version': 1,
        'package': 'vector-package',
        'package_version': '1.0',
        'files': [{'path': 'rules.ini', 'hash': 'sha256:' + hashlib.sha256(b'rules').hexdigest()}],
        'recorded_at': RECORDED_AT,
        'recorder': 0,
        'map_name': 'Vector Map',
        'game_mode': 'Battle',
        'map_path': 'maps/vector.map',
        'map_sha256': hashlib.sha256(MAP_BYTES).digest(),
        'seal_version': 1,
        'colors': [{'number': 0, 'rgb': 0xE0D838}, {'number': 3, 'rgb': 0x3058C8}],
    }
    info.update(changes)
    return info


def base_launch(**changes):
    launch = {
        'kind': 3,
        'seed': 0x12345678,
        'players': [
            {'house': 0, 'name': 'Alpha', 'human': True, 'country': 0, 'country_name': 'GDI',
             'color': 0, 'start': -1, 'resolved_country': 0, 'resolved_color': 0,
             'resolved_start': 2},
            {'house': 1, 'name': 'Bravo', 'human': True, 'country': -1, 'country_name': 'Nod',
             'color': 3, 'start': 0, 'resolved_country': 1, 'resolved_color': 3,
             'resolved_start': 0},
        ],
        'options': [
            {'key': 'Crates', 'flag': False},
            {'key': 'Credits', 'number': 10000},
            {'key': 'GameSpeed', 'number': 0},
            {'key': 'SessionType', 'text': 'LAN'},
        ],
    }
    launch.update(changes)
    return launch


def writer(info=None, launch=None, init=INIT_BODY, map_body=None, **kw):
    kw.setdefault('deflater', 'portable')
    return c.ReplayWriter(info or base_info(), launch or base_launch(), init, map_body, **kw)


def seals_hex(seals):
    return [{'through': t, 'hash': h.hex()} for t, h in seals]


def canon_hex(w):
    return [{'frame': f, 'bytes': b.hex()} for f, b in w.canon]


def body_of(data, tag):
    r = c.read_replay(data, check=False)
    for sec in r.sections:
        if sec.tag == tag:
            return sec.body
    return None


def exact_info(raw_size):
    """A RecordingInfo of exactly `raw_size` bytes, padded with FileHash entries."""
    head = {'engine': 'opents', 'data_version': 1, 'seal_version': 1}
    remaining = raw_size - len(c.encode_message('cncreplay.RecordingInfo', head))
    # An entry with a path of p bytes, 128 <= p <= 1024, and the hash "sha1:00" is p + 15 bytes.
    sizes = []
    while remaining > 1039:
        take = 1015 if remaining - 1015 >= 143 else remaining - 143
        sizes.append(take)
        remaining -= take
    sizes.append(remaining)
    info = dict(head, files=[{'path': 'f' * (n - 15), 'hash': 'sha1:00'} for n in sizes])
    body = c.encode_message('cncreplay.RecordingInfo', info)
    assert len(body) == raw_size, (len(body), raw_size)
    return info


def _event_entry(n):
    ev = 2 + 1 + len(c.encode_varint(n)) + n          # type = 1, then the payload
    return 1 + len(c.encode_varint(ev)) + ev


def exact_record(size):
    """A Frame record body of exactly `size` bytes: a hash and shared events."""
    by_size = {_event_entry(n): n for n in range(4097)}
    base = {'hash': 1, 'frame_micros': MICROS_30FPS}
    left = size - len(c.encode_message('cncreplay.Frame', base))
    payloads = []
    while left > 2 * _event_entry(4000):
        payloads.append(4000)
        left -= _event_entry(4000)
    if left in by_size:
        payloads.append(by_size[left])
    else:
        for n1 in range(4096, -1, -1):
            rest = left - _event_entry(n1)
            if rest in by_size:
                payloads += [n1, by_size[rest]]
                break
    rec = dict(base, events=[{'type': 1, 'payload': b'x' * n} for n in payloads])
    assert len(c.encode_message('cncreplay.Frame', rec)) == size
    return rec


class Builder:
    def __init__(self, out):
        self.out = out
        self.vectors = []
        self.computations = []
        self.written = []

    def write(self, name, data):
        path = os.path.join(self.out, name)
        with open(path, 'wb') as fh:
            fh.write(data)
        self.written.append(name)

    def add(self, name, data, exit_code, description, expect=None, reason=None, error_code=None):
        r = c.read_replay(data)
        if r.exit_code != exit_code:
            raise SystemExit('%s: expected verify exit %d, got %d: %s'
                             % (name, exit_code, r.exit_code, '; '.join(str(p) for p in r.problems)))
        found = [p.code for p in r.errors + r.unverified]
        if error_code is not None and error_code not in found:
            raise SystemExit('%s: expected %s, got %s' % (name, error_code, found))
        self.write(name, data)
        entry = {
            'file': name,
            'size': len(data),
            'sha256': hashlib.sha256(data).hexdigest(),
            'verify_exit': exit_code,
            'description': description,
        }
        if reason is not None:
            entry['reason'] = reason
            entry['error_code'] = error_code
        if exit_code in (0, 3, 4):
            entry['records'] = len(r.records)
            entry['last_recorded_frame'] = r.last_recorded_frame
            entry['s0'] = r.s0.hex() if r.s0 is not None else None
        if expect:
            entry.update(expect)
        self.vectors.append(entry)
        return r


def build(out):
    b = Builder(out)

    # ---- S0 for a known set of sections
    info = base_info()
    lnch = c.encode_message('cnclaunch.Game', base_launch())
    s0 = c.compute_s0(1, 'opents', 1, lnch, INIT_BODY)
    b.computations.append({
        'name': 's0_base',
        'description': 'S0 of section 7 for the base RecordingInfo values, LNCH body, and INIT body.',
        'seal_version': 1, 'engine': 'opents', 'data_version': 1,
        'lnch': lnch.hex(), 'lnch_sha256': hashlib.sha256(lnch).hexdigest(),
        'init': INIT_BODY.hex(), 'init_sha256': hashlib.sha256(INIT_BODY).hexdigest(),
        'preimage': (b'CNCR' + struct.pack('<II', 1, 6) + b'opents' + struct.pack('<I', 1)
                     + hashlib.sha256(lnch).digest() + hashlib.sha256(INIT_BODY).digest()).hex(),
        's0': s0.hex(),
    })
    s0e = c.compute_s0(1, '', 0, b'', b'')
    b.computations.append({
        'name': 's0_empty',
        'description': 'S0 with an empty engine ID, data_version 0, and empty LNCH and INIT bodies.',
        'seal_version': 1, 'engine': '', 'data_version': 0, 'lnch': '', 'init': '',
        's0': s0e.hex(),
    })

    # ---- minimal complete file
    w = c.ReplayWriter({'engine': 'opents', 'data_version': 1, 'seal_version': 1},
                       {'seed': 0}, b'\x00\x00\x00\x00', deflater='portable')
    w.add(0, hash=0, frame_micros=MICROS_30FPS)
    data = w.close(summary={'ended_at': RECORDED_AT + 1})
    b.add('minimal_complete.cncr', data, 0,
          'Smallest complete file: one record on frame 0 with a zero hash, every section stored. '
          'Game.seed is set to 0 and RecordingSummary.last_frame to 0, so both are written.',
          {'file_hex': data.hex(), 'seals': seals_hex(w.final_seals(0)), 'canonical': canon_hex(w)})

    # ---- no frame ran
    w = c.ReplayWriter({'engine': 'opents', 'data_version': 1, 'seal_version': 1},
                       {'seed': 0}, b'\x00\x00\x00\x00', deflater='portable')
    data = w.close(summary={'ended_at': RECORDED_AT})
    b.add('no_frames.cncr', data, 0,
          'Complete file in which the simulation ran no frame: FRMS holds only the end marker, '
          'last_frame is absent, and there are no seals.', {'seals': []})

    # ---- a chain of three intervals, two of them empty, reached by one skipped jump
    w = writer(info=base_info(income_kinds=[{'number': 1, 'id': 'Harvested'}]),
               codec=1, frms_codec=1, flush_every=2)
    w.add(0, hash=0x11111111, frame_micros=MICROS_15FPS,
          view={'camera': {'x': 1200, 'y': -40}, 'selection': {'ids': [0x01000005, 0x01000006]}},
          samples=[{'house': 0, 'credits': 10000, 'power_output': 0},
                   {'house': 1, 'credits': 10000, 'power_output': 0}])
    w.add(1, hash=0x22222222)
    w.add(2, hash=0x33333333, events=[
        {'type': 4, 'house': 0, 'due': 3, 'payload': bytes.fromhex('0500000001000000')},
        {'type': 31, 'house': 1, 'payload': b'tick', 'local': True}])
    w.add(3, hash=0x44444444, income=[{'house': 0, 'kind': 1, 'amount': 25}])
    w.add(2800, hash=0x55555555, defeated=[1])
    data = w.close(summary={'ended_at': RECORDED_AT + 200, 'completed': True,
                            'elapsed_micros': 190000000},
                   last_frame=2850)
    b.add('chain_three_intervals.cncr', data, 0,
          'Interval seals through 899, 1799, and 2699, then the final seal through 2850. Intervals '
          '2 and 3 hold no canonical frame and are sealed over the previous seal alone. The record '
          'on frame 2800 follows a skipped jump and carries all three interval seals. Every section '
          'uses codec 1, and FRMS is sync-flushed every 2 records.',
          {'seals': seals_hex(w.final_seals(2850)), 'canonical': canon_hex(w),
           'frame_seals': {'2800': [899, 1799, 2699]}})

    # ---- final seal on an interval boundary
    w = writer()
    w.add(0, hash=7, frame_micros=MICROS_30FPS)
    w.add(899, hash=8, frame_micros=MICROS_15FPS)
    data = w.close(summary={'ended_at': RECORDED_AT + 30})
    b.add('final_on_boundary.cncr', data, 0,
          'last_frame is 899, the last frame of interval 1, so RecordingSummary.seals holds that '
          'interval seal as the final seal and nothing else. No frame record carries a seal. '
          'game_micros is 899 * 33333 + 66666.',
          {'seals': seals_hex(w.final_seals(899)), 'game_micros': 899 * MICROS_30FPS + MICROS_15FPS})

    # ---- strings with invalid UTF-8
    raw = b'Bad\xffname\xc3(\xe2\x82'
    clean = c.sanitize_utf8(raw)
    b.computations.append({
        'name': 'utf8_replacement',
        'description': 'A writer replaces each ill-formed UTF-8 sequence with one U+FFFD, '
                       'following the Unicode maximal-subpart practice.',
        'input': raw.hex(), 'output': clean.encode('utf-8').hex(),
    })
    launch = base_launch()
    launch['players'][1]['name'] = clean
    w = writer(launch=launch)
    w.add(0, hash=1, frame_micros=MICROS_30FPS, messages=[
        {'house': 1, 'sender': clean, 'text': c.sanitize_utf8(b'hi \xc0\xafthere'),
         'scope': 1}])
    data = w.close(summary={'ended_at': RECORDED_AT + 1})
    b.add('utf8_replaced.cncr', data, 0,
          'Player 1 name and a chat line written from input with invalid UTF-8, after replacement '
          'as in computation utf8_replacement.',
          {'player_name_hex': clean.encode('utf-8').hex()})
    launch_raw = base_launch()
    launch_raw['players'][1]['name'] = raw
    w = writer(launch=launch_raw)
    w.add(0, hash=1, frame_micros=MICROS_30FPS)
    data = w.close(summary={'ended_at': RECORDED_AT + 1})
    b.add('malformed_invalid_utf8.cncr', data, 1,
          'Player.name in LNCH holds the raw bytes of computation utf8_replacement.',
          reason='a string field that is not valid UTF-8; proto3 parsers refuse it',
          error_code='invalid_utf8')

    # ---- checkpoint at frame 0
    w = writer()
    w.add(0, hash=1, frame_micros=MICROS_30FPS)
    w.add(1, hash=2)
    ck0 = c.checkpoint(0, b'engine save data at frame 0')
    ck1 = c.checkpoint(1, b'engine save data at frame 1')
    data = w.close(summary={'ended_at': RECORDED_AT + 1}, checkpoints=[ck0, ck1])
    b.add('checkpoint_frame0.cncr', data, 0,
          'Two CKPT sections. The one at frame 0 has no frame field, so its body starts with the '
          'fixed32 crc32 field (tag 0x15).',
          {'ckpt0_body': c.encode_message('cncreplay.Checkpoint', ck0).hex(),
           'ckpt1_body': c.encode_message('cncreplay.Checkpoint', ck1).hex()})

    # ---- set optional message fields with empty encodings
    w = writer()
    rec = w.add(0, hash=1, frame_micros=MICROS_30FPS,
                view={'camera': {'x': 0, 'y': 0}, 'selection': {'ids': []}})
    stats = {'houses': [{'house': 0, 'start': 2, 'outcome': 1, 'final': {'house': 0}},
                        {'house': 1, 'start': 0, 'outcome': 2, 'final': {'house': 1, 'credits': 0},
                         'crates': [{'id': 'Money', 'count': 2}]}]}
    data = w.close(summary={'ended_at': RECORDED_AT + 1}, statistics=stats)
    b.add('optional_empty_message.cncr', data, 0,
          'Frame 0 sets ViewState.camera to the point (0, 0) and ViewState.selection to an empty '
          'selection: both are optional and written with length 0, so view encodes as 22 04 0a 00 '
          '12 00. In STAT, house 0 has a final sample whose encoding is empty, which a writer '
          'leaves out because HouseStatistics.final is a plain message field; house 1 has '
          'credits 0 set and picked up two Money crates, written as 62 09 0a 05 4d 6f 6e 65 79 '
          '10 02. In LNCH the option Crates holds the oneof member flag = false, written as 18 00.',
          {'record_body': c.encode_message('cncreplay.Frame', rec).hex(),
           'stat_body': c.encode_message('cncreplay.Statistics', stats).hex()})

    # ---- one chat fingerprint
    salt = bytes(range(16))
    text = 'gg wp \u263a'
    fp = c.chat_fingerprint(salt, 1, text)
    b.computations.append({
        'name': 'chat_fingerprint',
        'description': 'Section 8: the first 16 bytes of SHA-256("chat-proof-1" || salt || '
                       'u32 sender house || UTF-8 text).',
        'salt': salt.hex(), 'house': 1, 'text': text.encode('utf-8').hex(), 'fingerprint': fp.hex(),
    })
    w = writer()
    w.add(0, hash=1, frame_micros=MICROS_30FPS)
    w.add(10, hash=2, messages=[{'house': 1, 'sender': 'Bravo', 'text': text,
                                 'color': 3, 'scope': 1, 'salt': salt}])
    w.add(12, hash=3, events=[{'type': 0x30, 'house': 1, 'payload': fp, 'role': 1}])
    data = w.close(summary={'ended_at': RECORDED_AT + 1})
    b.add('chat_fingerprint.cncr', data, 0,
          'A chat line with a salt on frame 10 and its fingerprint event, role EVENT_ROLE_CHAT_PROOF, '
          'on frame 12. The fingerprint is computation chat_fingerprint.',
          {'chat_proven': 1, 'canonical': canon_hex(w)})

    # ---- one group order with a sealed_payload, as the sender and a receiver have it
    sealed = bytes.fromhex('0a000000' '00' '00' '11223344' '00000000')
    sender = bytes.fromhex('0a000000' '07' '00' '11223344' 'aabbccdd')
    receiver = bytes.fromhex('0a000000' '07' '00' '11223344' '55667788')
    results = []
    for name, payload, info in (
            ('sealed_payload_sender.cncr', sender, base_info(recorder=0)),
            ('sealed_payload_receiver.cncr', receiver,
             base_info(recorder=1, map_name='Carte vectorielle', game_mode='Bataille'))):
        w = writer(info=info)
        w.add(0, hash=1, frame_micros=MICROS_30FPS)
        w.add(5, hash=2, events=[{'type': 0x0B, 'house': 0, 'payload': payload,
                                  'sealed_payload': sealed}])
        data = w.close(summary={'ended_at': RECORDED_AT + 1}, last_frame=5)
        b.add(name, data, 0,
              'A group order whose payload holds the machine\'s own bytes and whose sealed_payload '
              'holds the form every machine agrees on. The sender and receiver copies differ in '
              'payload, in INFO.recorder, and in INFO.map_name and game_mode, which the receiver '
              'holds translated, and have the same S0 and seals.',
              {'seals': seals_hex(w.final_seals(5)), 'canonical': canon_hex(w),
               'event_payload': payload.hex(), 'event_sealed_payload': sealed.hex()})
        results.append(w.final_seals(5))
    assert results[0] == results[1]

    # ---- an unclosed file, as after a crash
    def crash_writer(frames):
        w = writer(frms_codec=1, flush_every=1)
        for f in frames:
            if f == 0:
                w.add(0, hash=0xA0, frame_micros=MICROS_30FPS)
            elif f == 901:
                w.add(f, hash=0xB1, events=[{'type': 4, 'house': 1, 'payload': b'\x01\x02\x03\x04' * 8}])
            else:
                w.add(f, hash=0xA0 + f if f < 900 else 0xB0)
        return w
    frames = [0, 1, 2, 3, 4, 5, 900, 901]
    w = crash_writer(frames)
    open_file = w.open_bytes()
    head = len(w.head()) + c.SECTION_HEADER.size
    # With a sync flush after every record, the stored bytes through record k do not depend on
    # the records after it.
    stored7, _ = crash_writer(frames[:7]).frms_stored(terminate=False)
    assert open_file[head:head + len(stored7)] == stored7
    data = open_file[:head + len(stored7) + 3]
    r = b.add('incomplete_open.cncr', data, 3,
              'FRMS is open: stored and raw size are all ones, codec 1, a sync flush after every '
              'record. The file ends 3 bytes into the flushed block that holds frame 901, so that '
              'record is lost. The record on frame 900 carries the seal through 899, which matches.',
              {'frame_seals': {'900': [899]}})
    assert len(r.records) == 7 and r.last_recorded_frame == 900

    w = writer()
    w.add(0, hash=1, frame_micros=MICROS_30FPS)
    w.add(1, hash=2)
    data = w.open_bytes(raw_size_written=True)
    b.add('incomplete_closing.cncr', data, 3,
          'The writer stopped while closing: FRMS has its real raw size and a stored size that '
          'is still all ones, so it is read as open. The stream has no end marker.',
          {'last_recorded_frame': 1})

    # ---- sections and fields a reader skips
    info = base_info(_unknown=[(100, c._tag(100, 2) + c.encode_varint(5) + b'build')])
    w = writer(info=info, head_private=[(b'opts', b'private bytes', 0), (b'xtra', b'\x00' * 8, 7)])
    w.add(0, hash=1, frame_micros=MICROS_30FPS,
          _unknown=[(50, c._tag(50, 0) + c.encode_varint(3)),
                    (150, c._tag(150, 2) + c.encode_varint(2) + b'\x01\x02')])
    data = w.close(summary={'ended_at': RECORDED_AT + 1},
                   tail_private=[(b'ZZZZ', b'a future shared section', 0)])
    b.add('skipped_parts.cncr', data, 0,
          'A reader skips: private section "opts" (codec 0) and "xtra" (codec 7, unknown) after '
          'LNCH; an unknown shared section "ZZZZ" before END; private field 100 in INFO; unknown '
          'field 50 and private field 150 in a Frame record.')

    # ---- two copies of one game that desync, and a copy of another game
    copies = {}
    for name, h1000 in (('desync_a.cncr', 0xD00D), ('desync_b.cncr', 0xBEEF)):
        w = writer(codec=1, frms_codec=1)
        w.add(0, hash=1, frame_micros=MICROS_30FPS)
        w.add(500, hash=2, events=[{'type': 4, 'house': 0, 'payload': b'order'}])
        w.add(1000, hash=h1000)
        w.add(1001, hash=4)
        data = w.close(summary={'ended_at': RECORDED_AT + 60, 'desync_frame': 1000})
        copies[name] = data
        b.add(name, data, 0,
              'One of two copies of a game that differ in the frame hash on frame 1000. Each copy '
              'is valid on its own. compare reports the same game and the first difference on '
              'frame 1000, interval 2.',
              {'seals': seals_hex(w.final_seals(1001))})
    launch = base_launch()
    launch['players'][1]['color'] = 2
    w = writer(launch=launch)
    w.add(0, hash=1, frame_micros=MICROS_30FPS)
    data = w.close(summary={'ended_at': RECORDED_AT + 60})
    b.add('other_game.cncr', data, 0,
          'Differs from desync_a.cncr only in LNCH players[1].color, so S0 and every seal differ.')

    # ---- limits at the boundary
    w = c.ReplayWriter(exact_info(1 << 20), {'seed': 0}, b'\x00', codec=1, deflater='portable')
    w.add(0, hash=1, frame_micros=MICROS_30FPS)
    b.add('limit_info_at.cncr', w.close(summary={}), 0,
          'INFO with a raw size of exactly 1 MiB (1048576 bytes), codec 1.')
    w = c.ReplayWriter(exact_info((1 << 20) + 1), {'seed': 0}, b'\x00', codec=1, deflater='portable')
    w.add(0, hash=1, frame_micros=MICROS_30FPS)
    b.add('malformed_limit_info.cncr', w.close(summary={}), 1,
          'INFO with a raw size of 1048577 bytes, codec 1.',
          reason='INFO raw size above 1 MiB; a reader refuses it before inflating',
          error_code='limit_section')

    for size, name, exit_code in (((1 << 20), 'limit_record_at.cncr', 0),
                                  ((1 << 20) + 1, 'malformed_limit_record.cncr', 1)):
        w = writer(frms_codec=1)
        w.add(0, exact_record(size))
        data = w.close(summary={})
        assert len(c.encode_message('cncreplay.Frame', w.records[0][1])) == size
        b.add(name, data, exit_code, 'One Frame record of %d bytes, FRMS codec 1.' % size,
              reason=None if exit_code == 0 else 'a Frame record above 1 MiB',
              error_code=None if exit_code == 0 else 'limit_record')

    for count, name, exit_code in ((16384, 'limit_events_at.cncr', 0),
                                   (16385, 'malformed_limit_events.cncr', 1)):
        w = writer(frms_codec=1)
        w.add(0, hash=1, frame_micros=MICROS_30FPS, events=[{'local': True}] * count)
        b.add(name, w.close(summary={}), exit_code,
              'One record with %d local events that are otherwise empty, FRMS codec 1.' % count,
              reason=None if exit_code == 0 else 'more than 16384 events in one record',
              error_code=None if exit_code == 0 else 'limit_events')

    for size, name, exit_code in ((4096, 'limit_payload_at.cncr', 0),
                                  (4097, 'malformed_limit_payload.cncr', 1)):
        w = writer()
        w.add(0, hash=1, frame_micros=MICROS_30FPS, events=[{'type': 1, 'payload': bytes(size)}])
        b.add(name, w.close(summary={}), exit_code, 'One event payload of %d bytes.' % size,
              reason=None if exit_code == 0 else 'an event payload above 4096 bytes',
              error_code=None if exit_code == 0 else 'limit_payload')

    for size, name, exit_code in ((1024, 'limit_string_at.cncr', 0),
                                  (1025, 'malformed_limit_string.cncr', 1)):
        launch = base_launch()
        launch['players'][0]['name'] = 'n' * size
        w = writer(launch=launch)
        w.add(0, hash=1, frame_micros=MICROS_30FPS)
        b.add(name, w.close(summary={}), exit_code, 'Player 0 name of %d bytes.' % size,
              reason=None if exit_code == 0 else 'a string above 1024 bytes',
              error_code=None if exit_code == 0 else 'limit_string')

    for count, name, exit_code in ((64, 'limit_players_at.cncr', 0),
                                   (65, 'malformed_limit_players.cncr', 1)):
        launch = base_launch(players=[{'house': i, 'name': 'P%d' % i, 'human': True}
                                      for i in range(count)])
        w = writer(launch=launch)
        w.add(0, hash=1, frame_micros=MICROS_30FPS)
        b.add(name, w.close(summary={}), exit_code, '%d players in LNCH.' % count,
              reason=None if exit_code == 0 else 'more than 64 players',
              error_code=None if exit_code == 0 else 'limit_players')

    # ---- malformed files
    w = writer()
    w.add(0, hash=1, frame_micros=MICROS_30FPS)
    w.add(950, hash=2)
    good = w.close(summary={'ended_at': RECORDED_AT + 1})
    b.add('base_valid.cncr', good, 0,
          'The valid file several malformed vectors are derived from: records on frames 0 and '
          '950, the second carrying the seal through 899.',
          {'seals': seals_hex(w.final_seals(950))})

    b.add('malformed_short_preamble.cncr', good[:6], 2, 'The first 6 bytes of base_valid.cncr.',
          reason='shorter than the 8-byte preamble', error_code='short_preamble')
    b.add('malformed_bad_magic.cncr', b'CNCX' + good[4:], 2, 'Magic "CNCX".',
          reason='magic is not "CNCR"', error_code='bad_magic')
    b.add('malformed_container_version.cncr', good[:4] + struct.pack('<I', 2) + good[8:], 2,
          'Container version 2.', reason='container version is not 1',
          error_code='container_version')

    def sections_of(data):
        r = c.read_replay(data, check=False)
        return [(s.tag, s.codec, s.stored, s.raw_size) for s in r.sections]

    def assemble(parts):
        out = bytearray(c.MAGIC + struct.pack('<I', 1))
        for tag, codec, stored, raw in parts:
            out += c.section_header(tag, codec, len(stored), raw) + stored
        return bytes(out)

    parts = sections_of(good)
    b.add('malformed_missing_init.cncr', assemble([p for p in parts if p[0] != b'INIT']), 1,
          'base_valid.cncr without its INIT section.',
          reason='INIT is required; seals are not checked', error_code='missing_section')
    b.add('malformed_lnch_first.cncr', assemble([parts[1], parts[0]] + parts[2:]), 1,
          'LNCH before INFO.', reason='INFO must be first and LNCH second', error_code='section_order')
    end = [p for p in parts if p[0] == b'END '][0]
    ck = c.encode_message('cncreplay.Checkpoint', c.checkpoint(0, b'state'))
    b.add('malformed_section_after_end.cncr', assemble(parts + [(b'CKPT', 0, ck, len(ck))]), 1,
          'A CKPT section after END.', reason='END must be the last section',
          error_code='section_order')
    info_part = parts[0]
    bad = bytearray(assemble(parts))
    struct.pack_into('<Q', bad, 8 + 16, info_part[3] + 1)
    b.add('malformed_codec0_sizes.cncr', bytes(bad), 1,
          'INFO with codec 0 and a raw size one larger than its stored size.',
          reason='codec 0 requires equal stored and raw sizes', error_code='size_mismatch')
    bad = bytearray(assemble(parts))
    struct.pack_into('<Q', bad, 8 + 8, c.ONES)
    struct.pack_into('<Q', bad, 8 + 16, c.ONES)
    b.add('malformed_open_info.cncr', bytes(bad), 1, 'INFO marked open.',
          reason='only FRMS may be open', error_code='open_section')
    bad = bytearray(assemble(parts))
    struct.pack_into('<Q', bad, 8 + 16, c.ONES)
    b.add('malformed_raw_size_ones.cncr', bytes(bad), 1,
          'INFO with a real stored size and a raw size of all ones.',
          reason='a stored size that is not all ones comes with a real raw size',
          error_code='open_raw_size')
    info_body = info_part[2]
    b.add('malformed_unknown_codec_info.cncr',
          assemble([(b'INFO', 9, info_body, len(info_body))] + parts[1:]), 1,
          'INFO with codec 9.', reason='INFO, LNCH, and END must use codec 0 or 1',
          error_code='unknown_codec')
    defl = c.deflate(info_body, 'portable')
    b.add('malformed_raw_size.cncr',
          assemble([(b'INFO', 1, defl, len(info_body) - 1)] + parts[1:]), 1,
          'INFO with codec 1 whose raw size is one less than the inflated size.',
          reason='inflated size differs from the raw size', error_code='raw_size_mismatch')
    lnch_body = parts[1][2]
    grp = lnch_body + c._tag(50, 3) + c._tag(50, 4)
    b.add('malformed_group_wire_type.cncr',
          assemble([parts[0], (b'LNCH', 0, grp, len(grp))] + parts[2:]), 1,
          'LNCH ending in a start-group and end-group pair for field 50.',
          reason='wire types 3 and 4 are malformed', error_code='group_wire_type')
    cut = lnch_body + c._tag(3, 2) + c.encode_varint(20) + b'short'
    b.add('malformed_truncated_field.cncr',
          assemble([parts[0], (b'LNCH', 0, cut, len(cut))] + parts[2:]), 1,
          'LNCH ending in a players entry whose length runs past the end of the body.',
          reason='a field runs past the end of its message', error_code='truncated')
    for version, name in ((2, 'unknown_seal_version.cncr'), (0, 'absent_seal_version.cncr')):
        w = writer(info=base_info(seal_version=version))
        w.add(0, hash=1, frame_micros=MICROS_30FPS)
        b.add(name, w.close(summary={}), 4,
              'RecordingInfo.seal_version %s. The file is otherwise valid.'
              % ('2' if version else 'absent (0)'),
              reason='not verified: this reader knows only seal version 1, so the seals are '
                     'neither confirmed nor refuted; the file is not malformed',
              error_code='unknown_seal_version')

    w = writer(auto_seals=False)
    w.add(0, hash=1, frame_micros=MICROS_30FPS)
    w.add(950, hash=2)
    b.add('malformed_missing_frame_seal.cncr', w.close(summary={'ended_at': RECORDED_AT + 1}), 1,
          'base_valid.cncr with the seal through 899 left out of the record on frame 950; the '
          'summary seals are correct.', reason='an interval seal is missing from its frame record',
          error_code='missing_seal')

    w = writer()
    w.add(0, hash=1, frame_micros=MICROS_30FPS)
    real = w.add(950, hash=2)
    bad_seal = bytearray(real['seals'][0]['hash'])
    bad_seal[0] ^= 1
    real['seals'][0]['hash'] = bytes(bad_seal)
    b.add('malformed_frame_seal.cncr', w.close(summary={'ended_at': RECORDED_AT + 1}), 1,
          'The seal through 899 in the record on frame 950 has its first byte changed.',
          reason='an interval seal does not match', error_code='seal_mismatch')

    w = writer(auto_seals=False)
    w.add(0, hash=1, frame_micros=MICROS_30FPS)
    w.add(950, hash=2)
    w.add(1000, hash=3, seals=[{'through': 899, 'hash': w.final_seals(899)[0][1]}])
    b.add('malformed_misplaced_seal.cncr', w.close(summary={'ended_at': RECORDED_AT + 1}), 1,
          'The correct seal through 899 on the record of frame 1000 instead of frame 950.',
          reason='an interval seal is not on the first record after its interval',
          error_code='misplaced_seal')

    w = writer()
    w.add(0, hash=1, frame_micros=MICROS_30FPS)
    w.add(950, hash=2)
    seals = [{'through': t, 'hash': h} for t, h in w.final_seals(950)]
    seals[-1]['hash'] = bytes(32)
    b.add('malformed_final_seal.cncr', w.close(summary={'ended_at': RECORDED_AT + 1, 'seals': seals}),
          1, 'The final seal through 950 is 32 zero bytes.', reason='the final seal does not match',
          error_code='seal_mismatch')

    w = writer()
    w.add(0, hash=1, frame_micros=MICROS_30FPS)
    w.add(4, hash=2)
    b.add('malformed_record_after_last_frame.cncr', w.close(summary={'last_frame': 3}), 1,
          'last_frame is 3 and a record is on frame 4.',
          reason='a frame record after last_frame', error_code='record_after_last_frame')

    w = writer()
    w.add(0, hash=1, frame_micros=MICROS_30FPS)
    ckb = c.checkpoint(0, b'state')
    ckb['crc32'] ^= 1
    b.add('malformed_checkpoint_crc.cncr', w.close(summary={}, checkpoints=[ckb]), 1,
          'A checkpoint whose crc32 does not match its state.',
          reason='checkpoint CRC-32 mismatch', error_code='checkpoint_crc')

    w = writer()
    w.add(0, hash=1, frame_micros=MICROS_30FPS)
    rec_bytes = c.encode_message('cncreplay.Frame', w.records[0][1])
    bad_rec = rec_bytes + c._tag(1, 2) + b'\x00'
    stream = c.encode_varint(len(bad_rec)) + bad_rec + b'\x00'
    p2 = [p if p[0] != b'FRMS' else (b'FRMS', 0, stream, len(stream)) for p in sections_of(w.close(summary={}))]
    b.add('skipped_wire_type.cncr', assemble(p2), 0,
          'The record on frame 0 ends with field 1 (skipped, uint32) in wire type 2 with length 0. '
          'A reader skips it as an unknown field, so skipped reads as absent (0), the record is '
          'on frame 0, and every seal matches.',
          {'record_body': bad_rec.hex()})

    stream = c.encode_varint(len(rec_bytes)) + rec_bytes + c.encode_varint(40) + b'\x15'
    p2 = [p if p[0] != b'FRMS' else (b'FRMS', 0, stream, len(stream)) for p in sections_of(w.close(summary={}))]
    b.add('malformed_record_cut.cncr', assemble(p2), 1,
          'A closed FRMS that fits in the file and ends inside a record of 40 bytes.',
          reason='a record cut short inside a closed section', error_code='record_cut')

    # ---- more malformed files, unknown codecs, and a narrowed value
    w = writer(launch=base_launch(players=[{'house': i, 'name': 'P%d' % i, 'human': True}
                                           for i in range(3)]))
    w.add(0, hash=1, frame_micros=MICROS_30FPS)
    w.add(1, hash=2, defeated=[2, 1])
    data = w.close(summary={})
    b.add('malformed_defeated_order.cncr', data, 1,
          'The record on frame 1 lists defeated houses 2 then 1. The seals are computed over the '
          'canonical bytes, which list the houses in ascending order, and they match.',
          {'canonical': canon_hex(w)},
          reason='Frame.defeated is not in ascending order; the sealed list then has two readings, '
                 'record order and ascending order, so the record is refused',
          error_code='defeated_order')

    # ---- income kinds
    def income_file(kinds, records):
        w = writer(info=base_info(income_kinds=[{'number': n, 'id': i} for n, i in kinds]))
        for frame, used in enumerate(records):
            fields = {'hash': frame + 1}
            if frame == 0:
                fields['frame_micros'] = MICROS_30FPS
            if used:
                fields['income'] = [{'house': 0, 'kind': n, 'amount': 10} for n in used]
            w.add(frame, **fields)
        return w.close(summary={})

    w = writer(info=base_info(income_kinds=[{'number': 1, 'id': 'Harvested'}, {'number': 2, 'id': 'Crate'},
                                            {'number': 3, 'id': 'Sold/Unit'}, {'number': 4, 'id': 'Trigger'}]))
    w.add(0, hash=1, frame_micros=MICROS_30FPS, income=[{'house': 0, 'kind': 1, 'amount': 10}])
    w.add(1, hash=2, income=[{'house': 0, 'kind': 1, 'amount': 10}, {'house': 1, 'kind': 3, 'amount': 750}])
    w.add(2, hash=3, income=[{'house': 1, 'kind': 2, 'amount': 2000,
                              '_unknown': [(150, c._tag(150, 0) + c.encode_varint(0x4F7A20))]}])
    b.add('income_kinds.cncr', w.close(summary={}), 0,
          'INFO lists income kinds 1 "Harvested", 2 "Crate", 3 "Sold/Unit", and 4 "Trigger". Frame 0 '
          'pays kind 1, frame 1 pays kinds 1 and 3, and frame 2 pays kind 2 in an entry with private '
          'field 150. No entry uses kind 4.')
    for name, kinds, records, description, reason, code in (
            ('malformed_income_kind_unlisted.cncr', [(1, 'Crate')], [[1], [2]],
             'INFO lists income kind 1 "Crate". Frame 0 pays kind 1, and frame 1 pays kind 2.',
             'an Income entry uses a kind that RecordingInfo.income_kinds does not list',
             'income_kind_unlisted'),
            ('malformed_income_kind_number_twice.cncr', [(1, 'Crate'), (1, 'Trigger')], [[1]],
             'INFO lists income kind 1 "Crate" and kind 1 "Trigger". Frame 0 pays kind 1.',
             'two RecordingInfo.income_kinds entries have the same number', 'income_kind_number_twice'),
            ('malformed_income_kind_id_twice.cncr', [(1, 'Crate'), (2, 'Crate')], [[1, 2]],
             'INFO lists income kind 1 "Crate" and kind 2 "Crate". Frame 0 pays kinds 1 and 2.',
             'two RecordingInfo.income_kinds entries have the same id', 'income_kind_id_twice'),
            ('malformed_income_kind_id_empty.cncr', [(1, '')], [[1]],
             'INFO lists income kind 1 with an empty id. Frame 0 pays kind 1.',
             'a RecordingInfo.income_kinds entry has an empty id', 'income_kind_id_empty')):
        b.add(name, income_file(kinds, records), 1, description, reason=reason, error_code=code)

    for count, name, exit_code in ((4096, 'limit_income_kinds_at.cncr', 0),
                                   (4097, 'malformed_limit_income_kinds.cncr', 1)):
        kinds = [(n, 'Kind/%04d' % n) for n in range(1, count + 1)]
        b.add(name, income_file(kinds, [[count]]), exit_code,
              'INFO lists %d income kinds, numbered from 1. Frame 0 pays kind %d.' % (count, count),
              reason=None if exit_code == 0 else 'more than 4096 income kinds',
              error_code=None if exit_code == 0 else 'limit_income_kinds')

    w = writer()
    w.add(0, hash=1, frame_micros=MICROS_30FPS)
    w.add(2, hash=2)
    w.add(5, samples=[{'house': 0, 'credits': 100}])
    data = w.close(summary={'last_frame': 3})
    assert c.canonical_bytes(5, w.records[-1][1]) is None
    b.add('malformed_unsealed_after_last_frame.cncr', data, 1,
          'last_frame is 3 and a record on frame 5 holds only a sample: no hash, no shared event, '
          'no defeat. The seals through 3 match, so only the last_frame rule refuses the file.',
          reason='a frame record after last_frame', error_code='record_after_last_frame')

    init = [x for x in parts if x[0] == b'INIT'][0]
    init_start = len(assemble(parts[:[x[0] for x in parts].index(b'INIT')]))
    data = good[:init_start + c.SECTION_HEADER.size + len(init[2]) // 2]
    b.add('malformed_cut_in_init.cncr', data, 1,
          'base_valid.cncr cut in the middle of the INIT body, as a crash before the first flush '
          'leaves it.', reason='INIT runs past the end of the file and is ignored; a file without '
          'INIT is malformed', error_code='missing_section')

    p3 = [list(x) for x in sections_of(good)]
    for x in p3:
        if x[0] == b'INIT':
            x[1] = 9
    b.add('unknown_codec_init.cncr', assemble(p3), 4,
          'base_valid.cncr with INIT marked codec 9.',
          reason='not verified: INIT cannot be decoded, so S0 and the seals cannot be computed; '
                 'a future codec is allowed outside INFO, LNCH, and END',
          error_code='unknown_codec')
    p3 = [list(x) for x in sections_of(good)]
    for x in p3:
        if x[0] == b'FRMS':
            x[1] = 9
    b.add('unknown_codec_frms.cncr', assemble(p3), 4,
          'base_valid.cncr with FRMS marked codec 9.',
          reason='not verified: the frame stream cannot be read, so the seals cannot be compared',
          error_code='unknown_codec')

    launch = base_launch()
    launch['players'][0]['_unknown'] = [(1, c._tag(1, 0) + c.encode_varint(1 << 32))]
    w = writer(launch=launch)
    w.add(0, hash=1, frame_micros=MICROS_30FPS)
    b.add('narrowed_value_range.cncr', w.close(summary={}), 0,
          'Player 0 in LNCH has house (uint32) encoded as the varint 2^32. A reader narrows the '
          'value to 32 bits, as a generated parser does, so house reads as 0 and the file is '
          'accepted. The seals are computed over the LNCH bytes as stored, so they match. '
          'Re-encoding LNCH gives other bytes, so cncreplay warns that LNCH is not canonical.',
          {'narrowed': {'field': 'players[0].house', 'type': 'uint32', 'varint': 1 << 32,
                        'reads_as': 0}})

    w = writer()
    w.add(0, hash=1, frame_micros=MICROS_30FPS)
    w.add(1, hash=2)
    data = w.close(summary={}, checkpoints=[c.checkpoint(1, b'first'), c.checkpoint(1, b'second')])
    b.add('malformed_checkpoint_same_frame.cncr', data, 1, 'Two CKPT sections for frame 1.',
          reason='two checkpoints on the same frame', error_code='checkpoint_order')

    # ---- hand-built files on the smallest valid sections
    def sec(tag, body, codec=0, stored=None, raw=None):
        return c.section_header(tag, codec, len(body) if stored is None else stored,
                                len(body) if raw is None else raw) + body

    def hand(*sections):
        return c.MAGIC + struct.pack('<I', 1) + b''.join(sections)

    h_info = sec(b'INFO', bytes.fromhex('6801'))
    h_lnch = sec(b'LNCH', bytes.fromhex('1500000000'))
    h_init = sec(b'INIT', b'')
    h_frms = sec(b'FRMS', b'\x00')
    h_end = sec(b'END ', b'')
    h_note = ('Built by hand: INFO is 68 01 (seal_version 1), LNCH is 15 00000000 (seed 0), INIT and '
              'END are empty, and FRMS is 00 unless stated. ')
    data = hand(h_info, h_lnch, h_init, h_frms, h_end)
    b.add('hand_minimal.cncr', data, 0,
          h_note + 'The base of the other hand-built vectors. No frame ran, so there are no records '
          'and no seals.', {'file_hex': data.hex()})

    b.add('malformed_frms_bytes_after_terminator.cncr',
          hand(h_info, h_lnch, h_init, sec(b'FRMS', b'\x00\x01'), h_end), 1,
          h_note + 'The closed FRMS body is 00 01: one byte follows the end marker.',
          reason='a closed FRMS holds nothing after its zero-length end marker',
          error_code='data_after_end')
    b.add('malformed_frms_no_terminator.cncr', hand(h_info, h_lnch, h_init, sec(b'FRMS', b''), h_end), 1,
          h_note + 'FRMS is closed, codec 0, with an empty body.',
          reason='a closed FRMS ends without its zero-length end marker', error_code='frms_unterminated')
    b.add('malformed_frms_no_terminator_deflate.cncr',
          hand(h_info, h_lnch, h_init, sec(b'FRMS', b'\x03\x00', codec=1, raw=0), h_end), 1,
          h_note + 'FRMS is closed, codec 1, with raw size 0 and the stored body 03 00, a final empty '
          'block that inflates to nothing.',
          reason='a closed FRMS ends without its zero-length end marker', error_code='frms_unterminated')
    b.add('incomplete_frms_invalid_deflate.cncr',
          hand(h_info, h_lnch, h_init, sec(b'FRMS', b'\x07', codec=1, stored=c.ONES, raw=c.ONES)), 3,
          h_note + 'FRMS is open (stored and raw size all ones), codec 1, and its body is 07, a block '
          'of the reserved type 3. Invalid deflate data in an open FRMS ends the stream as if the '
          'input had run out, so the file is incomplete with no records.')
    b.add('malformed_frms_closed_invalid_deflate.cncr',
          hand(h_info, h_lnch, h_init, sec(b'FRMS', b'\x07', codec=1, raw=1), h_end), 1,
          h_note + 'FRMS is closed, codec 1, raw size 1, and its body is 07, a block of the reserved '
          'type 3. END follows.',
          reason='invalid deflate data in a closed FRMS that lies whole in the file',
          error_code='inflate_error')
    b.add('malformed_deflate_trailing_bytes.cncr',
          hand(h_info, h_lnch, sec(b'INIT', b'\x03\x00\x00', codec=1, raw=0), h_frms, h_end), 1,
          h_note + 'INIT is codec 1, raw size 0, with the stored body 03 00 00: an empty final '
          'block and one byte after it.',
          reason='stored bytes after the end of a closed section\'s deflate stream',
          error_code='inflate_error')
    b.add('malformed_section_after_end_unknown.cncr',
          hand(h_info, h_lnch, h_init, h_frms, h_end, sec(b'FUTR', b'')), 1,
          h_note + 'An empty section with the unknown shared tag "FUTR" follows END.',
          reason='no section follows END, a skipped one included', error_code='section_order')
    b.add('malformed_section_after_end_private.cncr',
          hand(h_info, h_lnch, h_init, h_frms, h_end, sec(b'priv', b'')), 1,
          h_note + 'An empty engine-private section "priv" follows END.',
          reason='no section follows END, a skipped one included', error_code='section_order')
    b.add('malformed_map_unknown_codec_after_frms.cncr',
          hand(h_info, h_lnch, h_init, h_frms, sec(b'MAP ', b'', codec=2), h_end), 1,
          h_note + 'An empty MAP section with codec 2 follows FRMS.',
          reason='MAP comes before FRMS, whatever its codec', error_code='section_order')
    b.add('malformed_second_map_unknown_codec.cncr',
          hand(h_info, h_lnch, sec(b'MAP ', b''), sec(b'MAP ', b'', codec=2), h_init, h_frms, h_end), 1,
          h_note + 'Two empty MAP sections before INIT, the first codec 0 and the second codec 2.',
          reason='MAP occurs at most once, whatever its codec', error_code='duplicate_section')
    b.add('malformed_unknown_tag_codec0_sizes.cncr',
          hand(h_info, h_lnch, sec(b'FUTR', b'', raw=1), h_init, h_frms, h_end), 1,
          h_note + 'After LNCH, a section with the unknown shared tag "FUTR", codec 0, stored size 0, '
          'and raw size 1.',
          reason='codec 0 requires equal stored and raw sizes for every tag, a skipped one included',
          error_code='size_mismatch')
    b.add('malformed_init_unknown_codec_over_limit.cncr',
          hand(h_info, h_lnch, sec(b'INIT', b'', codec=9, raw=(32 << 20) + 1), h_frms, h_end), 1,
          h_note + 'INIT is codec 9, stored size 0, raw size 33554433 (32 MiB + 1).',
          reason='the raw size limit holds for a section whose codec the reader does not know',
          error_code='limit_section')

    for name, bodies, frames, exit_code, description in (
            ('checkpoint_frame_wide.cncr', ['088180808010'], [1], 0,
             'One CKPT whose body is 08 81 80 80 80 10: frame as the varint 2^32 + 1, which narrows '
             'to frame 1. crc32 and state are absent, and the CRC-32 of an empty state is 0, so the '
             'checkpoint is valid.'),
            ('malformed_checkpoint_frame_repeated.cncr', ['08010802', '0802'], [2, 2], 1,
             'Two CKPT sections: 08 01 08 02, whose frame is 2 because the last occurrence counts, '
             'then 08 02. Both have the empty state and crc32 0.'),
            ('checkpoint_frame_last.cncr', ['15000000000801', '15000000000802'], [1, 2], 0,
             'Two CKPT sections whose frame field comes after crc32: 15 00000000 08 01 and '
             '15 00000000 08 02, on frames 1 and 2. A writer puts frame first, but a reader takes '
             'the field wherever it sits.')):
        ckpts = [sec(b'CKPT', bytes.fromhex(x)) for x in bodies]
        b.add(name, hand(h_info, h_lnch, h_init, h_frms, *ckpts, h_end), exit_code, h_note + description,
              {'checkpoint_frames': frames},
              reason=None if exit_code == 0 else 'two checkpoints on the same frame, judged on the '
                                                 'decoded frame values',
              error_code=None if exit_code == 0 else 'checkpoint_order')

    info = bytes.fromhex('68010a8108') + b'a' * 1025 + bytes.fromhex('0a00')
    b.add('malformed_string_limit_replaced.cncr', hand(sec(b'INFO', info), h_lnch, h_init, h_frms, h_end), 1,
          h_note + 'INFO is 68 01 0a 81 08, 1025 bytes of "a", then 0a 00: engine holds 1025 bytes, '
          'and a second, empty engine field replaces it.',
          reason='a string above 1024 bytes, even one a later occurrence replaces',
          error_code='limit_string')
    bomb = b'\x1a\x00' * 524288
    b.add('malformed_players_over_limit_bomb.cncr',
          hand(h_info, sec(b'LNCH', c.deflate(bomb, 'portable'), codec=1, raw=len(bomb)), h_init, h_frms,
               h_end), 1,
          h_note + 'LNCH is codec 1 and inflates to 1a 00 repeated 524288 times: 1 MiB, exactly the '
          'raw size limit, holding 524288 empty players.',
          reason='more than 64 players; a reader can stop counting at the 65th',
          error_code='limit_players')

    record = bytes.fromhex('08ffffffff0f1501000000')
    b.add('malformed_skipped_max.cncr',
          hand(h_info, h_lnch, h_init, sec(b'FRMS', c.encode_varint(len(record)) + record + b'\x00'),
               sec(b'END ', bytes.fromhex('08ffffffff0f'))), 1,
          h_note + 'The only record has skipped = 0xFFFFFFFF and hash 1, so it is on frame 0xFFFFFFFF, '
          'and carries no seal. END has last_frame 0xFFFFFFFF and no seals, so the summary also '
          'lacks its seals. 4772185 intervals end before the record; a verifier can find the '
          'missing seals by counting them, without computing the chain.',
          reason='an interval seal is missing', error_code='missing_seal')
    b.add('malformed_last_frame_max.cncr',
          hand(h_info, h_lnch, h_init, h_frms, sec(b'END ', bytes.fromhex('08ffffffff0f'))), 1,
          h_note + 'END has last_frame 0xFFFFFFFF and no seals; there are no records.',
          reason='RecordingSummary.seals lacks the seals up to last_frame', error_code='missing_seal')

    # ---- launch requests
    request = {
        'version': 1,
        'play': {
            'game': {'kind': 3, 'seed': 0,
                       'players': [{'name': 'Alpha', 'human': True, 'country': 0, 'color': 0, 'start': -1},
                                   {'name': 'Bravo', 'human': True, 'country': -1, 'color': 3, 'start': 1,
                                    'allies': [0]}],
                       'options': [{'key': 'Credits', 'number': 10000}, {'key': 'Crates', 'flag': False}]},
            'map_path': 'maps/vector.map',
            'session': {'local_player': 0, 'host': True, 'map_name': 'Carte', 'game_mode': 'Bataille'},
            'network': {'port': 1234, 'peers': [{'player': 1, 'address': '10.0.0.2', 'port': 1235}],
                        'connect_timeout': 30000},
            'recording': {'enabled': True, 'files': [{'path': 'rules.ini', 'hash': 'sha1:00'}],
                          'embed_map': False},
        },
        'result_path': 'launch-result.bin',
        'request_id': 'vector-1',
    }
    data = c.encode_message('cnclaunch.Request', request)
    text = json.dumps(c.to_proto_json('cnclaunch.Request', request), indent=2) + '\n'
    b.write('launch_play.bin', data)
    b.write('launch_play.json', text.encode('utf-8'))
    result = {'request_id': 'vector-1', 'code': 1,
              'scenarios': [{'map_path': 'maps/vector.map', 'outcome': 1, 'completed': True,
                             'last_frame': 0, 'replay_path': 'C:/Games/Replays/vector.cncr'}]}
    rdata = c.encode_message('cnclaunch.Result', result)
    b.write('launch_result.bin', rdata)
    b.computations.append({
        'name': 'launch_request',
        'description': 'launch_play.json in the proto3 JSON mapping encodes to launch_play.bin. '
                       'Game.seed and Session.local_player are optional and set to 0, so both '
                       'are written; the oneof member Crates.flag = false is written.',
        'json_file': 'launch_play.json', 'binary_file': 'launch_play.bin', 'binary': data.hex(),
    })
    b.computations.append({
        'name': 'launch_result',
        'description': 'A LaunchResult with one scenario; last_frame is optional and set to 0.',
        'binary_file': 'launch_result.bin', 'binary': rdata.hex(),
    })

    # ---- compare expectations
    b.computations.append({
        'name': 'compare_desync',
        'description': 'compare desync_a.cncr desync_b.cncr',
        'a': 'desync_a.cncr', 'b': 'desync_b.cncr', 'same_game': True, 'first_frame': 1000,
        'first_interval': 2, 'exit_code': 1,
    })
    b.computations.append({
        'name': 'compare_sealed_payload',
        'description': 'compare sealed_payload_sender.cncr sealed_payload_receiver.cncr',
        'a': 'sealed_payload_sender.cncr', 'b': 'sealed_payload_receiver.cncr', 'same_game': True,
        'first_frame': None, 'exit_code': 0,
    })
    b.computations.append({
        'name': 'compare_other_game',
        'description': 'compare desync_a.cncr other_game.cncr',
        'a': 'desync_a.cncr', 'b': 'other_game.cncr', 'same_game': False,
        'lnch_difference': 'players[1].color', 'exit_code': 1,
    })
    for comp in b.computations:
        if comp['name'].startswith('compare_'):
            ra = c.read_replay(_read(os.path.join(out, comp['a'])))
            rb = c.read_replay(_read(os.path.join(out, comp['b'])))
            res = c.compare_replays(ra, rb)
            if res['same_game'] != comp['same_game'] or res['exit_code'] != comp['exit_code'] or \
                    ('first_frame' in comp and res['first_frame'] != comp['first_frame']):
                raise SystemExit('%s: unexpected compare result %r' % (comp['name'], res))

    manifest = {
        'format': 'CNCR container version 1, seal version 1',
        'generator': 'tools/replay/make_vectors.py',
        'notes': 'Hashes and byte strings are lowercase hex. verify_exit is the exit code of '
                 'cncreplay.py verify: 0 complete and every seal matches, 1 a seal or structural '
                 'check fails, 2 not a CNCR file, 3 no END section and every seal present matches, '
                 '4 not verified: the file is not malformed but its seal_version, or the codec of INIT '
                 'or FRMS, is unknown, so its seals are neither confirmed nor refuted. error_code '
                 'is the code cncreplay.py reports for a malformed or unverified file.',
        'computations': b.computations,
        'files': b.vectors,
    }
    text = json.dumps(manifest, indent=2, ensure_ascii=True) + '\n'
    b.write('vectors.json', text.encode('ascii'))
    return b.written


def main(argv=None):
    parser = argparse.ArgumentParser(description='Write the CNCR test vectors and vectors.json.')
    parser.add_argument('--out', default=DEFAULT_OUT, help='output folder (default: %(default)s)')
    args = parser.parse_args(argv)
    os.makedirs(args.out, exist_ok=True)
    written = build(args.out)
    print('wrote %d files to %s' % (len(written), args.out))
    return 0


if __name__ == '__main__':
    sys.exit(main())
