# The replay format layer

`code/replay/` writes and reads replay files as
[REPLAY-FORMAT.md](REPLAY-FORMAT.md) defines them. The game does not call it
yet: no recorder or playback uses it. This document owns the choices the layer
makes where the format leaves them to an implementation, and what each part
of the layer promises its callers. The format and the meaning of every field
are in the specification.

## Files

| File | Content |
| --- | --- |
| `replay.hh` | Section tags, codecs, the seal version and interval, the flush cadence, the section 10 limits as `LimitsType`, and the enumerations of `replay.proto` |
| `replayfields.hh` | The field number of every field of `replay.proto` |
| `replaymessages.h`, `.cpp` | One struct per message, with `Encode`, `Decode`, `Check_Limits`, `Check_Order`, `Check_Income_Kinds`, `IncomeKindSet`, and `Clean_Text` |
| `replayseal.h`, `.cpp` | `S0`, the canonical bytes of a frame, the seal chain, and the seal verifier |
| `replaywriter.h`, `.cpp` | The container writer |
| `replayreader.h`, `.cpp` | The container reader, the frame stream reader, `Verify_Seals`, and `Verify_File` |

The layer builds on two files outside `code/replay/`:

| File | Content |
| --- | --- |
| `code/proto/message.h` | What the message structs share: the fields a decoder keeps without reading, the string and player limits as `LimitsType`, and `RuleError` and `RuleBreak` |
| `code/proto/messagecodec.h` | The encoder and decoder helpers that the files defining message structs share; nothing else includes it |

The Protocol Buffers wire encoding lives in `code/proto/` and raw deflate
streams in `code/`, because they are not specific to replays.
`code/proto/wire.h` declares the varint, zigzag, and fixed-value functions,
the field writer, and a bounded field reader in namespace `ProtoWire`.
`code/deflate.h` declares `DeflaterClass` and `InflaterClass`.
`replaymessages.h` declares the `ProtoWire` names in `Replay`, so replay code
uses them without a prefix.

The layer uses no engine types. Besides the standard library it uses
`code/proto/wire.cpp`, `code/deflate.cpp`, `code/sha256.cpp`, `CRC::Memory`
from `code/crc.cpp`, the UTF-8 checks of `code/utf8.cpp`, the `FileClass`
interface of `code/wwfile.h`, and, through `code/deflate.cpp`, the miniz copy
described in [Building OpenTS](../BUILDING.md). `tests/layers/layers.toml`
places the `replay` and `proto` directories and the `deflate` file in the
`core` layer.

## Messages

Each message of `replay.proto` is a struct in namespace `Replay::Message`,
named exactly as the schema names the message, such as
`Replay::Message::Frame` and `Replay::Message::PlayerMessage`. The codec
functions are declared in the same namespace, where argument-dependent lookup
finds them, and namespace `Replay` also declares the public ones, so
`Replay::Decode` names them too.

`Encode` writes the canonical bytes of section 1 of the specification: fields
in ascending number, zeros left out except in `optional` fields and `oneof`
members, repeated numbers packed, and varints in their shortest form.

`Decode` accepts any valid encoding and replaces the struct it is given. When
a non-repeated field appears more than once, the last one counts. For a
message field the last one replaces the earlier one whole; it is not merged
with it as a protobuf library would. Repeated numbers are read packed or
unpacked. Wire types 3, 4, 6, and 7, a field number of 0 or past 2^29 - 1, a
length past the end of the input, a varint longer than ten bytes, and a string
field that is not valid UTF-8 make the whole message malformed.

Every struct keeps the fields its decoder did not know in `Unknown`, with
their tags, and `Encode` writes them back among the known fields in ascending
field number. A known field that arrives with another wire type is kept there
too, as protobuf parsers keep it. Engine-private fields from 100 to 199 are
written and read through `Unknown`. This layer defines none; the recorder
adds the ones the specification's OpenTS notes name.

`Encode` writes strings as given and does not check them. `Clean_Text` is for
callers that fill a message from engine text: it replaces each maximal invalid
UTF-8 sequence with U+FFFD, as Python's and .NET's decoders do, and cuts the
result on a character boundary at a byte limit.

`Check_Limits` applies the section 10 limits to a decoded or prepared message:
string lengths, the player, house, and income kind counts, the events in a
record, and each payload and sealed payload. `Check_Order` applies the orders
the format sets: players by house, options by key in byte order, allies and
defeated houses ascending without repeats, a house never listed among its
allies, colors by number, and seals by `through`.

`Check_Income_Kinds` answers `INVALID` when two entries of
`RecordingInfo.income_kinds` have the same `number` or the same `id`, or an
entry has an empty `id`. `IncomeKindSet` is built from a `RecordingInfo`, and
its `Check` answers `INVALID` when an `Income` entry of a record uses a
`kind` that the `RecordingInfo` does not list. The writer and `FrameReader`
each build one from `INFO` before the first record.

`Check_Limits`, `Check_Order`, `Check_Income_Kinds`, and `IncomeKindSet::Check`
return a `RuleBreak` for the first rule a message breaks, or one whose `Error`
is `NONE`. A `RuleBreak` names the field by its message in the schema and its
number, such as `Frame` and 3 for `Frame.events`, and gives one of these
errors:

| `Error` | Rule broken |
| --- | --- |
| `TOO_LONG` | A string, payload, or sealed payload is longer than its limit |
| `TOO_MANY` | A repeated field has more entries than its limit |
| `NOT_ASCENDING` | A list is out of the order the format sets, or repeats a value |
| `INVALID` | An income kind entry or an `Income` entry fails `Check_Income_Kinds` or `IncomeKindSet::Check` |

`Decode` with a `LimitsType` applies the same limits while it reads
`RecordingInfo`, `Launch`, `Frame`, or `Statistics`. It checks every
occurrence of a string or payload, including one a later occurrence replaces,
and it refuses a player, house, event, or income kind past its count limit
before decoding it, so a forged count allocates nothing past the limit. It
returns false and fills a `RuleBreak` with `TOO_LONG` or `TOO_MANY` and the
field past its limit. On a malformed message it returns false and leaves the
`Error` at `NONE`.

## Writing

`Writer` writes one recording into a `FileClass` that the caller opened for
writing and left empty:

1. `Begin` takes `INFO`, `LNCH`, `INIT`, and optionally `MAP`, checks them, and
   holds them. It writes `seal_version` 1 into `INFO`, because it computes
   version 1 seals.
2. The first `Add_Frame` that produces a record, or `Close`, writes the
   preamble, the held sections, and an open `FRMS` header together, and
   flushes the file.
3. `Add_Frame` takes a frame number and its record. It computes `skipped`,
   fills `seals` with the interval seals that belong on the record, and writes
   the record into the frame stream. A record with nothing in it is not written.
4. `Add_Checkpoint` compresses a checkpoint at once and holds it until `Close`.
   Checkpoint frames must ascend. A recording that is never closed has no
   checkpoints.
5. `Close` takes the summary and, optionally, the statistics. It ends the frame
   stream, writes the raw size and then the stored size into the `FRMS`
   header, flushes, and appends the checkpoints, `STAT`, and `END`. The writer
   fills `RecordingSummary.seals`, which then lists every interval seal and the
   final seal.

`INFO`, `LNCH`, and `END` are stored uncompressed, so a listing reads them
without inflating. `MAP`, `INIT`, `CKPT`, `STAT`, and the frame stream are raw
deflate. The writer sync-flushes the frame stream and calls `FileClass::Flush`
after every 60 records and when `Flush` is called, so a crash loses at most the
records since the last flush.

Each call answers one of:

| Result | When |
| --- | --- |
| `OK` | The call was carried out |
| `REFUSED` | The input breaks a limit, an ordering rule, or an income kind rule, a checkpoint frame does not ascend, or records were written and the summary has no `last_frame` or one before the last record. Nothing was written and recording continues |
| `STATE` | The call does not fit the recording's state, such as a second `Begin`, a frame after `Close`, or a frame number at or before the last record's |
| `FAILED` | A write, seek, or flush of the file failed, the file is not open, a write would end past 2 GiB, or memory ran out. The writer has stopped, and this and every later call write nothing |

No call throws. Once the first sections are in the file, a failure leaves a
replay without `END` that has at least the records up to the last flush.

The writer writes the `FRMS` sizes by seeking back to the header, and seeks to
the end of what it wrote before it appends again. `FileClass` positions are
`int`, so a write that would end past 2 GiB fails. Open the file under its
final name, since an unclosed file is still a replay.

A synthetic recording with a 32-bit hash on every frame and an order every
50 frames stores its frame stream in about 6 bytes a frame: about 1.3 MB an
hour at 60 frames a second. No recorded game has been measured.

## Reading

`Reader::Open` reads the preamble and every section header from a `FileClass`
that the caller opened for reading, one small read per section, and checks the
section structure. It answers one of:

| Result | When |
| --- | --- |
| `OK` | The file can be read |
| `NOT_REPLAY` | The file is shorter than the preamble or does not start with `CNCR` |
| `VERSION` | The container version is not 1 |
| `MALFORMED` | A section other than `FRMS` is open, a closed section has an open raw size, a stored section's two sizes differ, the sections break the table of section 2, or `END ` uses a codec other than 0 or 1 |
| `LIMIT` | A section's raw size is past the limit of its tag, for any codec |
| `IO` | The file could not be read up to the size it reported |
| `MEMORY` | Memory ran out |

The reader seeks and reads only within the size the file reports, so a cut or
malformed file causes no failed seek or read. A file that is not open reads as
empty, and `Open` answers `NOT_REPLAY`. A file past 2 GiB cannot be read,
because `FileClass` sizes and positions are `int`.

No call of the reader throws. When memory runs out, a `Reader` call answers
`MEMORY`, `FrameReader::Next` fails with `Error` set to `MEMORY`, and
`Verify_Seals` and `Verify_File` report `MALFORMED` with `Error` set to
`MEMORY`.

The structure check requires `INFO` first and `LNCH` second, one `INIT`, at
most one each of `MAP`, `FRMS`, `STAT`, and `END`, `MAP` and `INIT` before
`FRMS`, `CKPT` and `STAT` after it, `FRMS` whenever `END` is present, and no
section after `END`. A section with a shared tag the reader does not know, an
engine-private section, and a section with a codec it does not know are
skipped by their stored size, but each still counts for the order: none may
come before `LNCH` or after `END`. A known tag counts for the order and number
of sections with any codec, so a second `MAP` is malformed even when its codec
is unknown. `Find` does not return a section with an unknown codec, and
`Unknown_Codec` reports that `INIT` or `FRMS` has one. A file cut before its
`INIT` ends is malformed, as the specification says.

Truncation follows section 2 of the specification. A section header cut short
is ignored, and so is a closed section whose body runs past the end of the
file, except `FRMS`, which is then read as an open section. A raw size is
checked against its limit before anything is allocated for the section.
`Read_Body` refuses a deflated body whose stream ends before its stored size,
or that inflates to more or fewer bytes than its raw size.

`FrameReader::Next` returns `CUT` on every call when the file has no `FRMS`
with a codec the reader knows. Otherwise it returns the records in order with
their frame numbers, and then one of the steps below. Its first call then
reads `INFO` with `Read_Info` for the income kind table, and when that read
fails, the step is `FAILED` with the result of `Read_Info` in `Error`.

| Step | When |
| --- | --- |
| `END` | The zero length that ends the stream. A closed stream has nothing after it, its deflate stream ends at the stored size, and its inflated size matches the raw size |
| `CUT` | An open stream ran out of input, or its deflate data turned invalid. A final record cut short is discarded |
| `FAILED` | `Error` says `MALFORMED` for a broken record, a closed stream with invalid deflate data, a closed stream that ends without its terminator, or one with a byte after it; `MALFORMED` also for a record whose `defeated` list is not strictly ascending or that has income of a kind `INFO` does not list; `LIMIT` for a record past 1 MiB or past a limit of its contents; `IO` when the file cannot be read; and `MEMORY` when memory runs out |

A closed `FRMS` whose stored size runs past the end of the file is read as an
open one, so invalid deflate data in it also ends the stream with `CUT`. Bytes
inflated after the terminator of a closed stream fail it as soon as the reader
reaches them, and are not kept.

`Read_Info`, `Read_Launch`, and `Read_Statistics` decode with the reader's
limits and answer `LIMIT` for a value past one. `Read_Info` also answers
`MALFORMED` when `Check_Income_Kinds` refuses the income kind table.
`Read_Checkpoint` refuses a checkpoint whose state does not match its
`crc32`. `Read_Checkpoint_Frame` inflates only the first bytes of a
checkpoint and reads an absent `frame` as 0. It gives the decoded frame only
for a checkpoint that starts with its one `frame` field, as a canonical
writer writes it, so `Open` does not use it to check the order of
checkpoints. `Check_Checkpoints` reads every checkpoint in
full and answers `MALFORMED` when a state does not match its `crc32` or the
frames do not ascend, judged on the decoded `frame`: its last occurrence,
narrowed to 32 bits.

`Verify_Seals` reads `INFO`, `LNCH`, `INIT`, the frame stream, and `END `,
recomputes the chain, compares it with every seal in the records and in
`END `, and checks the checkpoints as `Check_Checkpoints` does. It answers one
of:

| Status | When |
| --- | --- |
| `VERIFIED` | Every seal in the file matches, and each interval seal is on the first record after its interval |
| `MISMATCH` | A seal has another value, or `END ` lists a seal the chain does not give; `FirstMismatch` gives the first such `through` |
| `NOT_CHECKED` | The file is not verified: its `seal_version` is not 1, or `INIT` or `FRMS` uses a codec this reader does not know, so its seals are not compared |
| `MALFORMED` | A record follows `last_frame`, or the summary has no `last_frame` and there are records (`LateRecord` gives the frame); an interval seal is missing from the first record after its interval or is on another record, or `END ` lists fewer seals than the chain gives (`FirstMisplaced`); a checkpoint fails `Check_Checkpoints`; or the file cannot be read, with the read error in `Error` |

The verifier counts the intervals that end before a record, or before
`last_frame`, before it seals them. When a record or `END ` has fewer seals
than that count, the file is malformed and the verifier stops without sealing
those intervals, so a forged frame number cannot make it seal millions of
them.

A malformed file is reported as `MALFORMED` even when its `seal_version`, or
the codec of `INIT` or `FRMS`, is unknown. An unclosed file is checked as far as its frame stream goes.

`Verify_Seals` reads `LNCH` only as bytes and does not read `MAP ` or `STAT`,
so a file it reports `VERIFIED` can still be malformed, for example by a
`Player.country_name` that is not valid UTF-8. `Verify_File` gives the result
of section 11 of the specification. It calls `Open`, reads `INFO` with
`Read_Info`, `LNCH` with `Read_Launch`, the `MAP ` body with `Read_Body`, and
`STAT` with `Read_Statistics`, and then calls `Verify_Seals`. The first read
that fails decides the result; otherwise the seal report decides it:

| Status | When |
| --- | --- |
| `NOT_REPLAY` | `Open` answers `NOT_REPLAY` or `VERSION` |
| `MALFORMED` | A read answers `MALFORMED`, `IO`, or `MEMORY`, or `Verify_Seals` reports `MALFORMED` with an `Error` other than `LIMIT`. `Error` gives the read result |
| `LIMIT` | A read answers `LIMIT`, or `Verify_Seals` reports `MALFORMED` with `Error` set to `LIMIT`. With the default `LimitsType` the file is malformed |
| `MISMATCH` | `Verify_Seals` reports `MISMATCH`; `Seals` is its report |
| `NOT_VERIFIED` | `Verify_Seals` reports `NOT_CHECKED` |
| `INCOMPLETE` | `Verify_Seals` reports `VERIFIED` and the file has no `END ` |
| `COMPLETE` | `Verify_Seals` reports `VERIFIED` and the file has `END ` |

`Seals` is empty when a read failed before `Verify_Seals` ran.

## Checks

`tests/replay` is a doctest harness that compiles the layer with
`proto/wire.cpp`, `deflate.cpp`, `sha256.cpp`, `crc.cpp`, `utf8.cpp`,
`rawfile.cpp`, `ramfile.cpp`, `bfiofile.cpp`, `file.cpp`, `file_posix.cpp`,
and `file_win.cpp`, and links bimg for miniz. It reads no game data. It
covers:

- the SHA-256 test vectors of FIPS 180-2 and the CRC-32 check value;
- `Flush` on `RawFileClass` and on `BufferIOFileClass` with and without a
  cache, a cache turned on after the file opened, and a cache whose write to
  the file fails, and `RAMFileClass` over a fixed buffer, growing, and
  appending its own bytes;
- the wire encoding at its edge values and every malformed form it refuses;
- the field numbers and enumerations against `docs/replay/replay.proto`;
- canonical bytes against hand-assembled hex for each kind of field, and
  2,000 random frames through an encode and decode;
- deflate round trips, sync flushes, and a stream cut at every byte;
- seal vectors computed with Python's `hashlib` from the byte layouts of
  section 7, including an empty interval, a final seal on an interval
  boundary, and a verifier that finds an edited record;
- the writer's section order, flush cadence, refusals, and a file whose
  write, seek, or flush fails at every call, with each file it leaves read
  back;
- the reader on a file cut at every byte, the limits, forged sizes and counts,
  the income kind rules, malformed containers, bytes after a
  closed stream's terminator, invalid deflate data at the end of an unfinished
  stream, checkpoints out of order, an allocation failure inside each reader
  call, and 3,000 mutated files;
- the test vectors in `tests/replay/vectors/`: each file's `S0`, seals, and
  canonical bytes, and the result `Verify_File` gives it, against the
  manifest.

The case `replay.synthetic replay files are written for other readers` writes
seven recordings to `tests/replay/out/` under the build directory, for readers
in other languages: `minimal.cncr`, `three-intervals.cncr`,
`interval-boundary.cncr`, `embedded-map.cncr`, `checkpoint-frame0.cncr`,
`sealed-payload.cncr`, and `unclosed.cncr`. Run it after building the
`ReplayTest` target:

```powershell
ctest --test-dir build -C Debug -R "replay.synthetic"
```
