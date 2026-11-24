"""Shared paths and builders for the cncreplay tests."""

import os
import sys

TESTS = os.path.dirname(os.path.abspath(__file__))
TOOL = os.path.dirname(TESTS)
REPO = os.path.abspath(os.path.join(TOOL, '..', '..'))
PROTO_DIRS = {'replay.proto': os.path.join(REPO, 'docs', 'replay'),
              'launch.proto': os.path.join(REPO, 'docs', 'launch')}
VECTORS = os.path.join(REPO, 'tests', 'replay', 'vectors')

if TOOL not in sys.path:
    sys.path.insert(0, TOOL)

import cncreplay  # noqa: E402,F401


def sample_writer(codec=0, frms_codec=0, flush_every=60, map_body=b'map bytes', **kw):
    c = cncreplay
    info = {'engine': 'opents', 'engine_version': 'test', 'data_version': 1,
            'recorded_at': 1767225600, 'recorder': 0, 'map_name': 'Test', 'seal_version': 1}
    launch = {'kind': 2, 'seed': 7,
              'players': [{'house': 0, 'name': 'A', 'human': True, 'resolved_country': 0},
                          {'house': 1, 'name': '', 'ai_level': 0, 'resolved_country': 1}],
              'options': [{'key': 'Credits', 'number': 5000}]}
    w = c.ReplayWriter(info, launch, b'init state', map_body, codec=codec, frms_codec=frms_codec,
                       deflater='portable', flush_every=flush_every, **kw)
    w.add(0, hash=0x10, frame_micros=33333, samples=[{'house': 0, 'credits': 5000}])
    w.add(1, hash=0x11, events=[{'type': 3, 'house': 0, 'payload': b'order'},
                                {'type': 9, 'house': 1, 'payload': b'beat', 'local': True}])
    w.add(2, hash=0x12, messages=[{'house': 0, 'sender': 'A', 'text': 'hi'}])
    w.add(901, hash=0x13, defeated=[1])
    w.add(902, hash=0x14)
    return w


def sample_file(codec=0, frms_codec=0, flush_every=60):
    c = cncreplay
    w = sample_writer(codec, frms_codec, flush_every)
    data = w.close(summary={'ended_at': 1767225700, 'completed': True},
                   statistics={'houses': [{'house': 0, 'outcome': 1}, {'house': 1, 'outcome': 2}]},
                   checkpoints=[c.checkpoint(0, b'state 0'), c.checkpoint(901, b'state 901')])
    return w, data
