# Replay file format

This document defines version 1 of the CNCR replay container and version 1
of its seal chain. A C++ implementation in `code/replay/`, described in
[The replay format layer](FORMAT-LAYER.md), and a Python reference reader
implement it. Section 13 lists where the schema, the test vectors, and the
reference reader are.

CNCR is one replay format for every engine that runs a lockstep C&C
simulation. A reader that knows nothing about the engine that wrote a file
can do all of the following:

- list a replay: who played, on which map, when, and for how long;
- walk every frame and every event without a per-engine length table;
- verify the seal chain and compare two copies of one game;
- read the statistics.

Only three things need engine knowledge: what an event's payload means, what
is in the simulation start state, and what is in a checkpoint. The file
stores each of them as opaque bytes with a length.

## 1. Conventions

- All fixed-width integers are little-endian.
- All strings are UTF-8 without a terminator. A writer converts names and
  chat from the engine's encoding and replaces each maximal byte sequence
  that has no valid conversion with one U+FFFD. A string field that is not
  valid UTF-8 makes the file malformed; a generated parser refuses it too.
- A replay file has the extension `.cncr`, whichever engine wrote it.
- "Engine" is the program that ran the simulation and wrote the file.
- "Copy" is one player's recording of a game. A game with four recording
  players has four copies.
- "Shared" data is identical in every copy of one game. "Local" data belongs to
  the copy's recording player.
- House numbers are the engine's house indices. Events, samples, and statistics
  all use the same numbers, and `Launch.players[].house` maps them to players.
- Color numbers are the engine's player-color numbering. Every color field
  in a file uses that one numbering, and `RecordingInfo.colors` gives the
  RGB value of each number the file uses.

### Record encoding

Every structured record uses the Protocol Buffers wire encoding (proto3).
`replay.proto`, next to this document, is the schema. The message blocks
below repeat it with different comments. No library is required: a writer
needs a varint encoder and a length prefix, and a reader can use a generated
parser or read the fields by hand.

Schema rules:

- Field numbers 1 to 99 are shared and assigned only by this document.
- Field numbers 100 to 199 are engine-private in every message the schema
  reserves them in. Their meaning depends on `RecordingInfo.engine`, and a
  reader for another engine skips them. Each engine publishes a table of the
  numbers, types, and meanings it uses. A private field in a shared section
  is part of that section's bytes, so it has the same value on every
  machine. A message that does not reserve the range has no private fields.
- Field numbers from 200 are reserved for this document.
- Only `GameKind` has engine values, from 100. Every value of the other enums
  is defined in this document.
- A field with explicit presence is marked `optional`. For other scalar
  fields an absent field and a zero value mean the same.
- No field is floating point.

A writer encodes each set of values one way:

- It emits fields in ascending field number and emits a non-repeated field
  at most once.
- It leaves out a scalar field whose value is zero, false, or empty, unless
  the field is `optional`. It writes the chosen member of a `oneof` even
  when its value is zero. It packs repeated numeric fields.
- It leaves out a plain non-repeated message field whose encoding would be
  empty. An `optional` or `oneof` message field that is set is written even
  when its content is empty, with a length of zero. One that is not set is
  left out.
- It encodes every varint and length in the fewest bytes. It stores strings
  as given, without Unicode normalization.

Two writers of the same engine build therefore produce the same bytes for the
same values, which the seal chain relies on.

A reader follows these rules:

- It accepts fields in any order and skips field numbers it does not know.
- It skips a field whose number it knows and whose wire type differs from
  the schema's, as it skips an unknown field. A generated parser does the
  same.
- When a non-repeated field appears more than once, the last occurrence
  counts. For a message field, the last occurrence replaces the earlier ones
  whole, without merging their fields.
- It narrows a value too large for its field's type as protobuf does: a
  32-bit field keeps the low 32 bits, and a bool is true for any non-zero
  value. The file is accepted.
- Wire types 3 and 4 (groups) make the file malformed.

A tool that rewrites a file keeps every field it does not know in a shared
section, byte for byte.

## 2. Container

A file is an 8-byte preamble followed by sections.

```
offset size
0      4    magic "CNCR" (43 4E 43 52)
4      4    container version, 1
```

The container version changes only when the preamble or the section header
changes. Adding sections, fields, or codecs does not change it.

Each section is a 24-byte header followed by its body:

```
offset size
0      4    tag, four ASCII characters
4      4    codec: 0 = stored, 1 = raw deflate (RFC 1951, no zlib wrapper)
8      8    stored size: bytes of body in the file
16     8    raw size: bytes of body after decompression
```

For codec 0 the two sizes are equal; a closed section whose sizes differ
makes the file malformed, for any tag. For codec 1 the deflate stream of a
closed section ends at the stored size and inflates to the raw size. Stored
bytes after the end of the stream, or another inflated size, make the file
malformed.

A stored size of `FFFFFFFFFFFFFFFF` marks an open section: the writer has
not finished it, and its body runs to the end of the file. The writer sets
its raw size to `FFFFFFFFFFFFFFFF` as well. Only `FRMS` may be open. On a
clean close the writer overwrites the raw size, then the stored size, and
then appends the remaining sections. A crash between the two writes leaves a
real raw size beside an all-ones stored size, so a reader treats a section
whose stored size is all ones as open, with any raw size. A stored size that
is not all ones beside an all-ones raw size makes the file malformed, and so
does an open section other than `FRMS`.

A tag whose first character is an uppercase letter is shared and defined
here. A tag whose first character is a lowercase letter is engine-private.
Engine-private sections come after `LNCH` and before `END `.

A reader skips a section whose tag it does not know, using the stored size.
It also skips a section whose codec it does not know, with these exceptions:

- `INFO`, `LNCH`, and `END ` use codec 0 or 1, so that every reader can list
  a file. Any other codec on one of them makes the file malformed.
- A reader that does not know the codec of `INIT` or `FRMS` cannot check the
  seals, and the file is not verified (section 11).

A skipped section still counts for the table below: its place, its number,
and its raw size limit (section 10) are checked from its header.

### Sections

| Tag | Body | Occurs | Scope | Order |
|---|---|---|---|---|
| `INFO` | `RecordingInfo` | once | local | first |
| `LNCH` | `Launch` | once | shared | second |
| `MAP ` | the map file's bytes | at most once | local | before `FRMS` |
| `INIT` | engine start state, opaque | once | shared | before `FRMS` |
| `FRMS` | frame stream | once | mixed | after the above |
| `CKPT` | `Checkpoint` | any number | local | after `FRMS`, ascending frame |
| `STAT` | `Statistics` | at most once | local | after `FRMS` |
| `END ` | `RecordingSummary` | at most once | local | last |

A section that occurs more often than the table allows, or outside the
place the table gives it, makes the file malformed. No section follows
`END `, and an unknown or engine-private section before `LNCH` makes the
file malformed.

A shared section is byte-identical in every copy. The comparison is made on
the uncompressed body, so two copies may compress a shared section
differently. A local section may differ between copies even where its
content describes the same game.

A file without an `END ` section is incomplete: the game crashed or was
killed while recording. Everything before the end of the frame stream is
still valid.

To list a replay, a reader needs `INFO`, `LNCH`, and `END `. It reaches
`END ` by following the section sizes, which costs one small read per
section. Who won is in `STAT`.

An incomplete file has no recorded length or result. A reader that lists it
inflates `FRMS` to its last complete record. That record's frame number is
the last recorded frame, the sum of `frame_micros` over the frames is the
game's length, and `Frame.defeated` shows which houses were defeated up to
then. The result is unknown.

A writer writes `INFO`, `LNCH`, `MAP `, and `INIT` together, and may hold
them in memory until the first frame record: `Player.house` is known only
once the engine has assigned houses, which can be after recording starts. A
file that lacks `INFO`, `LNCH`, or `INIT` is therefore malformed, and a
verifier does not check its seals.

### Reading an incomplete file

- A section header cut short by the end of the file is ignored.
- A closed section whose stored size runs past the end of the file is
  ignored. `FRMS` is the exception: a reader may read it as far as it goes,
  as it does an open one.
- An open `FRMS` is inflated until the input runs out. A final record that is
  cut short is discarded.
- Deflate data that is invalid, in an open `FRMS` or in one whose stored size
  runs past the end of the file, ends the stream there, as if the input had
  run out: a power failure can leave a tail the writer never wrote. The
  records before it are kept. In a closed `FRMS` that lies whole in the file,
  invalid deflate data makes the file malformed.
- A writer flushes the deflate stream with a sync flush at least once per 60
  frame records and passes the flushed bytes to the file, so that at most
  that much is lost.

## 3. `INFO`

Facts about this copy and about the program that wrote it.

```proto
message RecordingInfo {
  string engine           = 1;   // registered engine ID, section 10
  string engine_version   = 2;   // build string, for display and diagnostics
  uint32 data_version     = 3;   // version of this engine's opaque payloads, from 1
  string package          = 4;   // game package name
  string package_version  = 5;   // game package version
  repeated FileHash files = 6;   // game files the simulation depends on
  uint64 recorded_at      = 7;   // Unix seconds when recording started
  optional uint32 recorder = 8;  // index into Launch.players of the recording player
  string map_name         = 9;   // this machine's name for the map
  string map_path         = 10;  // where this machine found the map, relative to the game directory
  bytes  map_sha256       = 11;  // SHA-256 of the map file's bytes as this machine read them
  string game_mode        = 12;  // this machine's name for the game mode
  uint32 seal_version     = 13;  // version of the seal chain's definition, 1
  repeated PlayerColor colors = 14; // RGB value of each color number the file uses
  repeated IncomeKind income_kinds = 15; // the kinds of payment an Income entry may name

  reserved 100 to 199;
}

message FileHash {
  string path = 1;               // relative to the game directory, '/' separators
  string hash = 2;               // "<algorithm>:<lowercase hexadecimal digest>"
}

message PlayerColor {
  uint32  number = 1;            // a color number, in the numbering of Player.color
  fixed32 rgb    = 2;            // 0x00RRGGBB, the color the engine shows for it
}

message IncomeKind {
  uint32 number = 1;             // the number Income.kind uses in this file
  string id     = 2;             // the engine's name for the kind of payment
}
```

`data_version` covers event payloads, `INIT`, engine-private fields, and the
definition of `Frame.hash`. An engine raises it when any of those change
meaning. A reader that decodes them checks `engine` and `data_version` first.
Versions start at 1, and an absent value means the version is unknown.

`data_version` describes the layout of the file and does not say which
builds can play the file back: a build that computes the game differently
diverges from a recording with the same `data_version`. An engine that tells
such builds apart writes a build identity in an engine-private field.
That identity also decides whether a build can load a checkpoint's `state`.

`map_name` and `game_mode` are the names this machine's launcher gave for the
map and the game mode. A launcher may translate them, so copies of one game
may have different names. When the launcher gives no map name, the engine
writes the map's own name. `game_mode` has no such source and may be empty.
No seal covers either name, and nothing compares them between copies.

`seal_version` names the definition of `S0`, the canonical bytes, and the
seal interval in section 7. This document defines version 1, and a change to
any of the three raises it. A verifier that does not know a file's
`seal_version`, version 0 included, does not compare its seals, and the file
is not verified (section 11).

`map_path` is where this machine found the map: a path relative to the game
directory, with `/` separators. For a map the engine read from a game archive
it is the name the engine opened the map by. A writer leaves the field empty
when the launcher's path is absolute or leads outside the game directory, so
that a file passed around has no folder names from the player's machine. A
reader refuses a path that is absolute or has a `..` segment.

`map_sha256` is the SHA-256 of the map file's bytes as the game read them,
before parsing. When the file has a `MAP ` section, it is the SHA-256 of that
section's body.

Neither field is part of the shared setup, and both can differ between
copies: one map may be at different paths on two machines, or two machines
may have map files that differ in bytes and load to the same game.
`map_sha256` is one copy's statement, and no seal covers it. A reader that
needs to know the map compares it across copies. Playback may use the path
to find a map that is not embedded, and a launcher may use the hash to find
the map in its library.

`files` lets a launcher warn before playback that the local game files differ
from the recorded ones. It does not affect parsing. `FileHash.hash` names its
algorithm, as in `sha256:9f86d081...`, because launchers already keep digests
of their files and do not all use one algorithm. A reader compares two hashes
only when their algorithms match, and compares paths without regard to ASCII
case.

`colors` lists each color number that a player in `LNCH`, a house in `STAT`,
or a message uses, in ascending order, with the RGB value the engine shows
for it. A reader draws colors from this table and needs no rules file.

`income_kinds` gives the `id` of every kind number that an `Income` entry in
the file uses (section 6). The numbers have no meaning outside the file. Two
entries with the same `number` or the same `id`, or an entry with an empty
`id`, make the file malformed.

`package`, `package_version`, and `files` describe the installation, which
the engine cannot detect. The launcher passes them to the engine when it asks
for a recording.

`recorder` is absent when the recording was made by a pure observer process
that is not in the player list.

## 4. `LNCH`

`LNCH` describes the game that was started: what a launcher decided before
the game ran, with nothing that depends on how the game was told about it and
nothing that identifies a machine.

`LNCH` is shared: every copy of a game stores the same bytes. It therefore
lists players in ascending `house` order, and it has no addresses, ports,
tunnels, file paths, or playback settings. Each value in it comes from the
game's shared setup, never from a setting that only one machine has.

```proto
message Launch {
  GameKind kind               = 1;
  optional fixed32 seed       = 2;   // always set
  repeated Player players     = 3;   // ascending house
  repeated GameOption options = 4;   // ascending key
  bool anonymous              = 5;   // players were shown substitute names

  reserved 100 to 199;
}

enum GameKind {
  GAME_KIND_UNKNOWN     = 0;
  GAME_KIND_CAMPAIGN    = 1;     // one machine, a mission
  GAME_KIND_SKIRMISH    = 2;     // one machine, against AI players
  GAME_KIND_MULTIPLAYER = 3;     // several machines in lockstep
}

message Player {
  uint32 house        = 1;       // house index the engine assigned
  string name         = 2;       // for an AI, the name the launcher gave it, or empty
  bool   human        = 3;
  bool   observer     = 4;
  sint32 country      = 5;       // requested country index, -1 for random
  string country_name = 6;       // rules ID of the country played
  sint32 color        = 7;       // requested color number, -1 for random
  sint32 start        = 8;       // requested start position, from 0, -1 for random
  optional sint32 ai_level = 9;  // requested AI difficulty, in the engine's numbering
  repeated uint32 allies = 10;   // other houses the launch allied this one with, ascending
  optional sint32 resolved_country = 11; // what the game assigned
  optional sint32 resolved_color   = 12;
  optional sint32 resolved_start   = 13;

  reserved 100 to 199;
}

message GameOption {
  string key = 1;                // the engine's option name
  oneof value {
    sint64 number = 2;
    bool   flag   = 3;
    string text   = 4;
  }
}
```

`kind` is the session the engine set up, which playback has to set up again.
It does not say how the players found each other: a LAN game, a lobby game,
and a ladder match are all `GAME_KIND_MULTIPLAYER`. Where an engine plays
differently depending on that, it records the difference as an option, such
as a ladder flag or fixed alliances. Engines define further kinds as values
from 100.

The engine writes `LNCH` from its session state. A launcher is one source of
that state, and a game started from the engine's menus has the same section.
A launcher that hands the engine a `Launch` message uses the same schema in
a different form, which the engine's launch interface defines.

`seed` is always set, so a seed of 0 is written.

`options` lists every setting that changes the simulation and is not a
property of one player: starting credits, unit count, tech level, game
speed, crates, short game, and so on. Keys are the engine's option names,
written once each in ascending byte order. A reader shows them as key and
value and needs no per-engine table.

`options` has everything the engine's playback needs to set the game up
again, including each option the engine applied at its default. It has
nothing that differs between machines, such as addresses, ports, file paths,
and per-machine preferences. A key the engine does not use is not written,
so that two builds whose simulations are the same write the same bytes.

A setting that changes the simulation and belongs to one player goes in an
engine-private `Player` field when no shared field exists for it. Playback
reads it as it reads `options`.

`country` is a position, from 0, in the engine's country list, the list a
seat chooses from: `GDI` and `Nod` in TS, `Americans` and `Russians` in YR.
The factions that countries belong to, such as `Allies`, are not in that
list. `country_name` is the rules ID of the country played. `color` is a
color number (section 1). `start` counts the map's start positions from 0.

`country`, `color`, and `start` are the values asked for. Playback hands
them back to the engine, and where a value is -1 the engine makes the same
random draws it made in the game, which keeps every later random number the
same. A launcher that chooses a value, instead of asking for -1, writes the
value it chose.

`resolved_country`, `resolved_color`, and `resolved_start` are the values the
game assigned. The engine has them, like `house`, once it has set the game
up, which is before it writes the first frame record, so an incomplete file
still has them. They are for readers, and playback does not read them. A
resolved field is written whether the request named the value or asked for
-1. It is absent only when the game assigned none, as with the start
position of an observer.

`ai_level` is the difficulty asked for a computer player, in the engine's
numbering. When it is absent the player plays at the game's default
difficulty. It is absent for a human player.

`name` is the name the launcher gave. For a computer player the launcher left
unnamed it is empty: an engine never writes a name it chose, because engines
pick such names from a translated string table.

`anonymous` is set when the game showed substitute names in place of the
players' names, as a quick match does. `Player.name` is still the real name.
A viewer of such a file shows `HouseStatistics.name` and
`PlayerMessage.sender`, which are the names as they were shown.

The engine always writes `LNCH`, including `house`, the resolved fields, and
`country_name`. `LNCH` has no file name, path, or hash for the map, because
one map may be a different file on each machine. `INIT` identifies what was
loaded. The names of the map and the game mode are in `INFO` (section 3).

`allies` lists the other houses the launch allied this player with, in
ascending order. A house never lists itself, and alliances the map sets are
not listed. A reader that shows teams groups players by `allies`.

Map-defined houses that no lobby slot controls, such as campaign AI houses,
are not players. They appear in `Statistics.houses`.

## 5. `MAP ` and `INIT`

`MAP ` is the map file's bytes as the game read them. The launcher decides
whether it is present, because only the launcher knows which maps every
viewer has. When the launcher does not say, the writer embeds a map that was
a loose file and leaves out a map it read from a game archive.

`MAP ` is a local section. Copies of one game may differ in whether they
have it, and in its bytes where two machines had map files that differ and
load to the same game. No seal covers it.

Playback looks for the map in this order: the `MAP ` section, a path the
launcher gives when it asks for playback, and `RecordingInfo.map_path`. It
may try first a source whose SHA-256 is `RecordingInfo.map_sha256`, and a
reader that uses `MAP ` checks that hash before anything else. Playback loads
a source and compares the start state with `INIT` before frame 0. When they
differ it tries the next source, because a file at a recorded path may have
been replaced since the game. When every source it found differs, the map or
the game files are not the recorded ones: playback says so before the first
frame and does not present the result as verified. When no source yields a
map, playback refuses and reports that the map is missing.

`INIT` identifies the start state, and every recording has one. It includes
a digest of what was loaded: a hash of the loaded game state where the engine
can compute one, and otherwise the SHA-256 of the map's bytes. The state hash
differs when the wrong map or the wrong game files were loaded. The map's
SHA-256 differs only for the wrong map.

`INIT` also includes any data the engine needs to put the simulation into
its exact starting state beyond `LNCH` and the map, such as a random number
generator table captured after the scenario loaded. Its layout is defined by
the engine and `data_version`. `INIT` is shared, so an engine that stores the
map's SHA-256 there requires every machine to have the same map bytes.

## 6. `FRMS`: the frame stream

The body is a sequence of records. Each record is a varint byte length
followed by a `Frame` message of that length. A length of zero ends the
stream. A closed `FRMS` ends with that zero length and has nothing after it.
A closed `FRMS` that ends without it, has a byte after it, or ends inside a
record makes the file malformed.

```proto
message Frame {
  uint32 skipped                  = 1;  // frames with no record since the previous record
  optional fixed32 hash           = 2;  // engine state hash for this frame
  repeated Event events           = 3;  // in the order the engine executed them
  ViewState view                  = 4;  // local
  repeated PlayerMessage messages = 5;  // local
  optional uint32 frame_micros    = 6;  // nominal frame length in microseconds, 0 = uncapped
  repeated HouseSample samples    = 7;
  repeated Income income          = 8;
  repeated Seal seals             = 9;
  repeated uint32 defeated        = 10; // houses defeated on this frame, ascending

  reserved 100 to 199;
}
```

The frame number of a record is the previous record's frame number, plus
one, plus `skipped`. The record before the first counts as frame -1. A
record's frame number is the engine's frame counter, which is 0 on the first
frame the scenario simulates.

A writer writes a record for a frame when any of its fields would be
non-empty, and never writes an empty `Frame`. An engine that hashes every
frame writes a record for every frame.

`hash` is the engine's synchronization hash, taken at the same point of the
frame on every machine. Playback recomputes it and compares. An engine may
store fewer bits of its hash, zero-extended. Its table of private fields
gives the number of bits, and playback compares that many.

`frame_micros` is the nominal length of this frame and the frames after it:
1,000,000 divided by the frame rate the engine paced the frame to, rounded
down, and 0 when the rate is uncapped. In a lockstep game that is the rate
the machines agreed on, which can be lower than the rate of the speed
setting. It is written on the first record and on each record whose value
differs from the previous one. Readers use it to turn frame numbers into game
time without a per-engine speed table. Real pacing varies with lag, and
playback does not read this field.

`defeated` lists the houses the simulation marked defeated during this
frame, as seen at the point `hash` is taken, in ascending order. A house
appears once, on the frame its state changed. A record that lists defeated
houses out of ascending order, or lists a house twice, makes the file
malformed. The list is shared and the seals cover it, so two copies with
matching seals agree on which houses were defeated and when.

Some inputs change the simulation without passing through the event queue.
In TS and YR the game's speed setting changes animation rates, and selecting
a unit whose tag waits for it springs a trigger. An engine records each such
input either as an event or in an engine-private `Frame` field that playback
applies before the frame's logic runs.

### Events

```proto
message Event {
  uint32 type    = 1;            // engine event type number
  uint32 house   = 2;            // house that issued it
  sint32 due     = 3;            // scheduled frame minus this record's frame
  bytes  payload = 4;            // engine payload for this type
  bool   local   = 5;            // true when other copies may not have this event
  EventRole role = 6;
  optional bytes sealed_payload = 7; // the bytes the seals cover, when not payload

  reserved 100 to 199;
}

enum EventRole {
  EVENT_ROLE_NONE       = 0;
  EVENT_ROLE_CHAT_PROOF = 1;     // payload is a chat fingerprint, section 8
}
```

A record lists the events the engine executed on that frame, which is not
always the frame they were scheduled for. An event the engine rescheduled is
recorded where it ran. An event that never reached execution, such as one
that arrived too late, is not recorded. An event the engine took up on its
frame is recorded when playback must inject it to reproduce the frame,
whether the engine carried it out or refused it. An engine may leave out an
event that every machine dropped before it took effect. Every copy must make
the same choice, because the seals cover the result. Whether an event was
refused is not sealed. An engine whose playback needs that fact writes it in
an engine-private field.

`payload` is the type's network payload. Its meaning depends on
`RecordingInfo.engine` and `RecordingInfo.data_version`. The file stores the
payload's byte count in front of it, as the wire encoding does for every
`bytes` field, so a reader that does not know the engine skips the payload
without knowing the type's size.

`local` separates two kinds of events:

- An event with `local` unset is shared. Every copy of the game has it on
  the same frame, in the same order, with the same `type`, `house`, and
  sealed bytes. Orders the players gave are shared.
- An event with `local` set differs between machines, such as per-peer frame
  heartbeats. An engine writes the local events its playback needs and those
  its published readers use, and may leave out the rest. The engine defines
  which types are local: a timing change that every machine executes on the
  same frame is shared.

Both kinds stay in one list, because an engine can depend on the order
between them.

`payload` is always the bytes playback needs. An event's sealed bytes are
`sealed_payload` when that field is present and `payload` otherwise. An
engine writes `sealed_payload` only when its machines have different bytes
for the same order. In YR, a group order reaches receivers with the first
unit's `Follow` target copied to every unit, while the sender keeps each
unit's target, and the game reads that target when the order executes. The
YR writer writes this machine's bytes in `payload` and the form every machine
agrees on in `sealed_payload`.

The seal chain covers shared events only, by their sealed bytes.

### View

```proto
message ViewState {
  optional Point camera        = 1; // when it changed
  optional Selection selection = 2; // when it changed

  reserved 100 to 199;
}

message Point {
  sint32 x = 1;
  sint32 y = 2;
}

message Selection {
  repeated uint32 ids = 1;       // engine object IDs
}
```

The recording player's camera position and selected objects, in engine
units. Each is written on the first record and on each record where it
changed.

### Messages

```proto
message PlayerMessage {
  uint32 house               = 1;   // sender
  string sender              = 2;   // sender name as shown
  string text                = 3;
  sint32 color               = 4;   // color number the line was shown in
  MessageScope scope         = 5;
  repeated uint32 recipients = 6;   // house indices, when the sender said
  bytes  salt                = 7;   // chat fingerprint salt, section 8

  reserved 100 to 199;
}

enum MessageScope {
  MESSAGE_SCOPE_UNKNOWN = 0;
  MESSAGE_SCOPE_ALL     = 1;
  MESSAGE_SCOPE_TEAM    = 2;
  MESSAGE_SCOPE_PRIVATE = 3;
  MESSAGE_SCOPE_OBSERVERS = 4;
}
```

`messages` lists chat lines only. An engine that sends other things beside
chat, such as map beacons or taunts, records them in engine-private `Frame`
fields.

Messages do not travel through the lockstep event queue, so a copy has only
the messages its recording player sent or received. A message that arrives
while the simulation is not advancing is recorded with the frame the engine
is waiting on.

Readers treat every string and index in a message as untrusted.

### Samples and income

```proto
message HouseSample {
  uint32 house                     = 1;
  optional sint64 credits          = 2;
  optional sint64 stored_value     = 3;
  optional sint64 spent            = 4;
  optional sint64 earned           = 5;
  optional sint32 power_output     = 6;
  optional sint32 power_drain      = 7;
  optional uint32 vehicles         = 8;
  optional uint32 infantry         = 9;
  optional uint32 aircraft         = 10;
  optional uint32 buildings        = 11;
  optional sint64 army_value       = 12;
  optional sint64 building_value   = 13;
  optional uint32 units_killed     = 14;
  optional uint32 buildings_killed = 15;
  optional uint32 units_lost       = 16;
  optional uint32 buildings_lost   = 17;
  optional uint32 units_built      = 18;
  optional uint32 buildings_built  = 19;
  optional sint64 score            = 20;
  bool   defeated                  = 21;

  reserved 100 to 199;
}

message Income {
  uint32 house  = 1;
  uint32 kind   = 2;             // a number from RecordingInfo.income_kinds
  sint64 amount = 3;             // greater than zero

  reserved 100 to 199;
}
```

Samples are taken at the same point of the frame as `hash`, for every house.
The first sample is on frame 0. After that the interval is the writer's
choice; once per 60 frames is the suggested default.

Every metric is `optional`. A writer sets each metric it tracks in every
sample, zero included, and leaves out a metric it does not track. An absent
metric is not available for that game.

| Metric | Meaning |
|---|---|
| `credits` | Money the house can spend now, without stored resources |
| `stored_value` | Value of the resources the house has in storage and has not been paid for. Resources carried by harvesters are not counted |
| `spent` | Total paid out since the start. A refund does not reduce it |
| `earned` | Total of the house's `Income` entries since the start, except entries whose kind is in the `Starting` group |
| `power_output`, `power_drain` | Power the house produces and uses now |
| `vehicles`, `infantry`, `aircraft`, `buildings` | Objects the house owns now, each counted once, in the category of its type |
| `army_value`, `building_value` | Sum of the type cost of the objects the house owns now that are not buildings, and of its buildings |
| `units_killed`, `buildings_killed` | Objects of other houses that this house destroyed. Units are infantry, vehicles, and aircraft. A capture is not a kill |
| `units_lost`, `buildings_lost` | This house's objects that another house destroyed. A sale or a capture is not a loss |
| `units_built`, `buildings_built` | Objects the house's factories produced |
| `score` | The engine's running score, which can differ from the value its score screen shows at the end |
| `defeated` | The simulation has marked the house defeated |

An engine whose counter differs from a definition converts it or leaves the
metric out.

`Income` records one house's payments of one kind during the frame, summed.
A record has one entry per house and `kind`, on the frame the money was paid.
Money paid while the scenario was set up is on frame 0. Money taken from a
house is spending and has no entry.

`Income.kind` is a number from `RecordingInfo.income_kinds` (section 3), and
a reader identifies the kind by the `id` given there. An entry whose `kind`
is not in that list makes the file malformed. A writer lists the kinds its
build can pay, so a kind may be listed and never used.

An `id` is the engine's name for the kind, and the engine's notes (section
12) list the names it writes. An engine that cannot name the kind of a
payment, because it records only the address of the code that paid it,
writes one kind for such payments and keeps the address in an
engine-private `Income` field. Two ids are the same only when their bytes
are the same. Part of an `id` up to its first `/` is its group, and an `id`
without a `/` is its group. An engine uses the groups below for these kinds
of payment, so that a reader can add up income across engines without
knowing each engine's names. A kind that fits none of them has a group of
the engine's choosing.

| Group | Payment |
|---|---|
| `Harvested` | Resources a harvester delivered |
| `Building` | Money a building produced |
| `Sold` | An object the house gave up for money: one it sold, or a unit it fed to a grinder |
| `Refund` | Spending that was paid back |
| `Crate` | Money from a crate |
| `Stolen` | Money or stored resources taken from another house |
| `Trigger` | Credits a map trigger gave |
| `Starting` | Starting money paid through the same path as income |

Samples and income are shared in content, but copies are not required to
sample on the same frames or to give a kind the same number. The seal chain
does not cover them.

## 7. Seals

A seal chain lets anyone check that two copies record the same game, and
lets a ladder keep a few kilobytes of evidence after the files are gone. It
does not stop a player from editing a file: whoever edits a file can
recompute its chain. The recomputed chain stops matching every other copy at
the first changed interval.

The seals cover the shared events, the frame hashes, and the houses defeated.
They do not cover `STAT` or `END `, so an edited `outcome` leaves every seal
intact. A reader that needs a result it can check reads it from
`Frame.defeated` and the alliances, in a file whose seals match another
copy's, or plays the game back.

```proto
message Seal {
  uint32 through = 1;            // last frame the seal covers
  bytes  hash    = 2;            // 32 bytes
}
```

The chain starts from the shared setup:

```
S0 = SHA-256( "CNCR" || u32 seal_version
              || u32 byte length of engine || engine
              || u32 data_version
              || SHA-256(LNCH body) || SHA-256(INIT body) )
```

`seal_version`, `engine`, and `data_version` are the `RecordingInfo` values.
They decide what the sealed bytes mean, and every machine of one game has the
same ones, so a copy with another value stops matching. Bodies are hashed
uncompressed. `S0` does not cover `MAP `, so it does not depend on whether
the copy embeds the map.

Each seal covers the frames after the previous seal's `through` (or from
frame 0) up to and including this seal's `through`:

```
Sn = SHA-256( Sn-1 || canonical bytes of each covered frame, in order )
```

A frame has canonical bytes when its record has a `hash`, at least one
shared event, or a defeated house. They are, with fixed-width little-endian
integers:

```
u32 frame
u8  1 if the record has a hash, else 0
u32 hash                         only when present
u32 count of shared events
per shared event, in record order:
        u32 type, u32 house, s32 due, u32 length of the sealed bytes, the sealed bytes
u32 count of defeated houses
per defeated house, in ascending order:
    u32 house
```

`role` and local events are not covered.

An interval with no canonical frames is still sealed, over the previous seal
alone.

All copies of a game must place interval boundaries on the same frames, so
the interval is a constant of the format: 900 frames. Seal number k, counting
from 1, has `through = 900k - 1`.

Each interval seal goes in `Frame.seals` of the first record after its
interval, so an incomplete file still has every seal up to the last flush.
A record that follows the end of several intervals has their seals in
ascending order. An interval seal that is missing, or that is on any other
record, makes the file malformed even when its value is right.

On a clean close the writer lists every seal in `RecordingSummary.seals`,
ending with a final seal whose `through` is `RecordingSummary.last_frame`.
The final seal appears only there. When `last_frame` is the last frame of an
interval, that interval's seal is the final seal and no other is added. A
summary that lists fewer seals than the chain has up to `last_frame` makes
the file malformed. A
recording in which the simulation ran no frame has no seals.

Two copies agree on every seal whose `through` is `900k - 1`. A final seal
that falls elsewhere covers part of an interval, and each recording can end
on a different frame. A verifier recomputes the chain from the file's
contents and compares it with the seals in the file and with other copies.
It compares `LNCH` first and reports the field that differs, because any
difference there changes every seal.

## 8. Chat fingerprints

Chat is not part of the lockstep stream, so a recorded chat line cannot be
checked against other copies. An engine may make it checkable:

1. The sender picks a random 16-byte salt and sends it with the chat message.
2. The sender queues a shared event with `role = EVENT_ROLE_CHAT_PROOF` whose
   payload is the first 16 bytes of
   `SHA-256("chat-proof-1" || salt || u32 sender house || UTF-8 text)`.
3. Each copy records the message with its `salt`.

Any copy's chat line can then be checked against the fingerprint event in any
other copy. A copy that did not receive the message has the fingerprint but
not the salt, so the text cannot be learned from that copy.

Executing the event changes nothing in the simulation. The seal chain already
covers it, so a copy with a fingerprint removed or altered stops matching the
other copies.

## 9. `CKPT`, `STAT`, and `END `

### Checkpoints

```proto
message Checkpoint {
  uint32  frame = 1;             // written first
  fixed32 crc32 = 2;             // of state
  bytes   state = 3;             // engine save data, opaque

  reserved 100 to 199;
}
```

One `CKPT` section per checkpoint, in ascending frame order, with no two on
the same frame. A checkpoint lets playback jump to `frame` without simulating
from the start. `state` is the game as it stands before frame `frame` runs,
and playback continues with the record for `frame`. Its layout is defined by
the engine, and the engine's build identity (section 3) says whether a given
build can load it. `crc32` is the CRC-32 that zlib computes.

`frame` is the first field in the body so that a reader can inflate a few
bytes of each section to find the checkpoint it wants. A checkpoint at frame
0 has no `frame` field, as the encoding rules leave out a zero.

Checkpoints follow `FRMS`, so a writer holds them until the file closes. A
recording that ends in a crash has none.

Loading engine save data from an untrusted file is only as safe as the
engine's loader. The checksum detects corruption and nothing more. Playback
may ignore every checkpoint.

A verifier decodes every checkpoint by the rules of section 1, so `frame` is
the last occurrence of the field wherever it sits in the body. Checkpoints
out of ascending frame order, two on one frame, and a `state` that does not
match `crc32` each make the file malformed.

How many checkpoints to keep is the writer's policy. Ladder uploads and
common chat services cap files near 20 MB.

### Statistics

```proto
message Statistics {
  repeated ObjectType types   = 1;
  repeated HouseStatistics houses = 2;

  reserved 100 to 199;
}

enum ObjectCategory {
  OBJECT_CATEGORY_UNKNOWN  = 0;
  OBJECT_CATEGORY_BUILDING = 1;
  OBJECT_CATEGORY_INFANTRY = 2;
  OBJECT_CATEGORY_VEHICLE  = 3;
  OBJECT_CATEGORY_AIRCRAFT = 4;
}

message ObjectType {
  ObjectCategory category = 1;
  uint32 index         = 2;      // the engine's type index within the category
  string id            = 3;      // rules ID
  string name          = 4;      // display name
  sint32 cost          = 5;
  string icon          = 6;      // icon file name, optional
  bool   naval         = 7;
  bool   unscored      = 8;      // left out of the engine's score
  bool   insignificant = 9;      // never counted by the engine's tallies

  reserved 100 to 199;
}

message HouseStatistics {
  uint32 house           = 1;
  string name            = 2;
  string country_name    = 3;
  sint32 color           = 4;    // color number
  sint32 start           = 5;    // -1 when the house was never placed
  repeated uint32 allies = 6;    // other houses allied at the end, ascending
  Outcome outcome        = 7;
  LeaveReason leave      = 8;
  HouseSample final      = 9;
  repeated TypeCount counts = 10;
  repeated HouseKills kills = 11;
  repeated CrateCount crates = 12; // crates the house picked up

  reserved 100 to 199;
}

enum Outcome {
  OUTCOME_UNKNOWN = 0;
  OUTCOME_WON     = 1;
  OUTCOME_LOST    = 2;
  OUTCOME_DRAW    = 3;
}

enum LeaveReason {
  LEAVE_REASON_NONE         = 0;
  LEAVE_REASON_RESIGNED     = 1;
  LEAVE_REASON_DISCONNECTED = 2;
}

message TypeCount {
  CountKind kind    = 1;
  ObjectCategory category = 2;
  uint32 index      = 3;         // ObjectType.index
  uint32 count      = 4;
}

message CrateCount {
  string id    = 1;              // the engine's name for the crate type
  uint32 count = 2;
}

enum CountKind {
  COUNT_KIND_UNKNOWN   = 0;
  COUNT_KIND_BUILT     = 1;
  COUNT_KIND_KILLED    = 2;
  COUNT_KIND_LOST      = 3;
  COUNT_KIND_CAPTURED  = 4;
  COUNT_KIND_REMAINING = 5;
}

message HouseKills {
  uint32 house     = 1;          // the other house
  uint32 units     = 2;          // of theirs this house destroyed
  uint32 buildings = 3;
}
```

`types` lists the type tables in the engine's order, so that type indices
in counts, and in event payloads for readers that decode them, resolve to
names without a rules file. `STAT` is written when the file closes, so an
incomplete file has no type names.

`unscored` marks a type the engine leaves out of its score. `insignificant`
marks a type the engine's tallies never count. A writer that counts such a
type anyway sets the flag, so that a reader can leave those entries out.

`country_name`, `color`, and `start` in `HouseStatistics` are the values the
game assigned, in the terms of the `Player` fields. `start` is -1 for a house
the game never placed. `allies` lists the other houses this house was allied
with when the recording ended, in ascending order, and never the house
itself.

`outcome` is per house. Houses that take no part in the game, such as a
neutral house, are left out of the rules below and get `OUTCOME_UNKNOWN`, as
does every observer.

- `OUTCOME_WON`: the simulation decided the game, the house was not defeated,
  and every other undefeated house is its ally.
- `OUTCOME_LOST`: the house was defeated, or left, before the recording
  ended. A defeated member of a winning alliance is `OUTCOME_LOST`.
- `OUTCOME_DRAW`: the simulation decided the game, the house was not
  defeated, and some other undefeated house is not its ally.
- `OUTCOME_UNKNOWN`: the recording ended before the game was decided, and the
  house was neither defeated nor gone.

A reader that scores teams combines houses through `allies`.

`leave` comes from state every machine shares, the departure and surrender
events, and never from this machine's view of a connection, so copies agree
on it.

`final` is a last sample, taken when the recording ended. `counts` has one
entry for each kind and type with a count above zero:

| Kind | Counts |
|---|---|
| `COUNT_KIND_BUILT` | Objects of the type that the house's factories produced |
| `COUNT_KIND_KILLED` | Objects of the type, owned by other houses, that this house destroyed |
| `COUNT_KIND_LOST` | This house's objects of the type that were destroyed |
| `COUNT_KIND_CAPTURED` | Buildings of the type that the house captured |
| `COUNT_KIND_REMAINING` | Objects of the type that the house owned when the recording ended |

`kills` counts, for each other house, that house's objects which this house
destroyed, with units and buildings as in `HouseSample`.

`crates` counts the crates the house picked up, with one entry for each crate
type with a count above zero. `id` is the engine's name for the crate type.

The file does not include the engine's end-of-game statistics packet. In TS
and YR that packet includes each player's network address and details of the
recording machine, and a replay is passed around more freely than the packet
is. An engine that needs the packet in its files writes it in an
engine-private field.

### Summary

```proto
message RecordingSummary {
  optional uint32 last_frame     = 1; // last frame the simulation ran
  uint64 ended_at                = 2; // Unix seconds when recording stopped
  optional uint32 desync_frame   = 3; // first frame whose hash differed between machines
  bool   completed               = 4; // the simulation decided the game
  repeated Seal seals            = 5; // every seal, the final one last
  optional uint64 elapsed_micros = 6; // play time in real microseconds, pauses excluded
  optional uint64 game_micros    = 7; // sum of frame_micros over the frames that ran

  reserved 100 to 199;
}
```

`last_frame` is the last frame the simulation ran, whether or not that frame
has a record. It is absent when no frame ran. A frame record for a frame
after `last_frame`, or any frame record in a file whose `END ` has no
`last_frame`, makes the file malformed.

`desync_frame` is the first frame whose hash differed between machines, which
is earlier than the frame on which the engine noticed.

`completed` is true when the recording player's game ended by a win, a loss,
or a draw that the simulation decided, a time limit included. It is false
when the player quit, restarted, or loaded a save, and when the game stopped
on a lost connection or a desync.

`elapsed_micros` is the engine's play clock in microseconds of real time,
with time spent paused left out. Lag counts, because the clock runs in real
time. In TS and YR the clock is `Scen->ElapsedTimer`, which ticks every 16
ms, so the value is the tick count times 16,000. The score screens of those
games count 60 ticks as a second and so show about 4% more. An engine without
a working clock leaves the field out.

The original TS and YR engines reset the clock when a saved game is loaded,
so a game resumed from a save reports only the time since the load. OpenTS
keeps the clock across a load.

`game_micros` is the sum of `frame_micros` over every frame that ran: the
game's length at its nominal speed. A writer adds it up as it records. A
launcher shows `elapsed_micros` as the replay's length when it is present,
and `game_micros` otherwise.

`ended_at` is only a timestamp. `ended_at - RecordingInfo.recorded_at` also
counts pauses, so it is not the game's length.

## 10. Engine IDs and limits

Engine IDs are short lowercase strings, registered here:

| ID | Engine |
|---|---|
| `opents` | OpenTS |
| `yrpp-spawner` | CnCNet YR spawner |

Readers treat every size and count as untrusted. These limits are part of the
format. A writer does not exceed them, and a value past its limit makes the
file malformed. The limits apply to every occurrence of a field, including
one a later occurrence replaces, and to the raw size of a section the reader
skips:

| Item | Limit |
|---|---|
| Raw size of `INFO`, `LNCH`, `END ` | 1 MiB each |
| Raw size of `MAP `, `INIT`, `STAT` | 32 MiB each |
| Raw size of one `CKPT` | 64 MiB |
| One `Frame` record | 1 MiB |
| Events in one record | 16,384 |
| One event payload | 4,096 bytes |
| One string | 1,024 bytes |
| Players, houses | 64 |
| Income kinds | 4,096 |

## 11. Reading results

A reader that reads a whole file and checks its seals reports one of these
results:

| Result | When |
|---|---|
| Not a CNCR file | The file is shorter than the 8-byte preamble, its first four bytes are not `CNCR`, or its container version is not 1 |
| Malformed | The file breaks a rule that this document says makes a file malformed, or a section body or record cannot be decoded: a field runs past the end of its message, a varint is longer than ten bytes, or a record is cut short inside a closed `FRMS` |
| Seal mismatch | A seal in a record or in `END ` differs from the chain the verifier computes from the file (section 7) |
| Not verified | The verifier does not know the file's `seal_version`, version 0 included, or the codec of its `INIT` or `FRMS` section. Its seals are neither confirmed nor refuted, their placement is not checked, and the file is not malformed for that reason |
| Incomplete | The file has no `END ` section, and every seal in it matches the chain |
| Complete and verified | The file has an `END ` section, and every seal in its records and in `END ` matches the chain |

When several results apply, the reader reports the first of these that
applies: malformed or seal mismatch, then not verified, then incomplete.

## 12. Engine notes

These notes say how each registered engine fills the fields whose content
the format leaves to the engine. No engine records games in this format yet,
so the notes define what an engine writes once it does.

### OpenTS

- `Frame.hash` is the engine's 32-bit synchronization hash.
- `INIT` includes a 32-bit hash of the loaded game state, taken once before
  the first frame's logic runs, over every object, type, and cell and the
  rules. Machines may have map files that differ in bytes and load to the
  same state.
- An event's `payload` is the first `EventLength[type]` bytes of the event.
  OpenTS writes no `sealed_payload`: a group order travels compressed only
  when every order in the group is equal apart from its unit, so receivers
  rebuild the sender's values.
- `Frame.view.camera` is the position of the center of the view in tactical
  map pixels, which depend on the tile size. `Selection.ids` lists each
  selected object's target value, whose top 8 bits are the object's kind.
- In a skirmish or multiplayer game, a player's house number is its position
  in `Launch.players`. In a campaign the map creates the houses.
- For a `country`, `color`, or `start` request of -1, the engine draws a value,
  for a human or a computer player, and writes it in the resolved field.
- A color number is the color's position in the rules' `[Colors]` list.
- `ai_level` is 0 for the hardest computer player, 1 for medium, and 2 for
  the easiest. A larger value plays as 2. When the field is absent the
  game's `AIDifficulty` option sets the player's difficulty.
- `Launch.options` includes `PlayMoviesInMultiplayer` and, for a multiplayer
  game, `SessionType`, which records whether the game was a LAN or an
  internet game. At the start an internet game clears fixed alliances and a
  LAN game does not.
- `LNCH` records a player's real name. In a quick match the engine shows
  "Player N" in its place and sets `Launch.anonymous`. `PlayerMessage.sender`
  and `HouseStatistics.name` are the name as it was shown, which in a quick
  match is that text in this machine's language.
- In a game started through `SPAWN.INI`, `UIMapName` fills
  `RecordingInfo.map_name` and `UIGameMode` fills `game_mode`.
- In a game started through `SPAWN.INI`, a human player's `Side=` fills
  `Player.country`, and a computer player's entry in `[HouseCountries]`
  fills it for that player.
- When the launcher gives no map name, `RecordingInfo.map_name` is the
  map's `[Basic] Name=`, or for a campaign mission the name the engine shows
  for it.
- A campaign mission is identified by the scenario name in
  `RecordingInfo.map_path`.
- `ObjectType.insignificant` is the rules' `Insignificant` flag.
- `CrateCount.id` is the name of the crate type's entry in the rules'
  `[Powerups]` section, such as `Money` or `Veteran`.
- OpenTS writes a build identity in an engine-private `INFO` field. The
  identity changes whenever the simulation or the save layout changes.
  Playback on a build with another identity warns first and reports a hash
  mismatch as a different build.
- OpenTS writes no chat fingerprints.
- OpenTS accepts launch requests in the format of `launch.proto`, beside this
  document. Its `Play.launch` field is the `Launch` message before the engine
  has filled in what it assigns, and `launch.proto` lists the differences.

OpenTS writes these `IncomeKind.id` values:

| `id` | Payment |
|---|---|
| `Harvested` | A harvester unloaded at a refinery |
| `Building/Cash` | A building paid its `ProduceCashAmount` |
| `Building/Startup` | A building paid its `ProduceCashStartup` to the house that captured it |
| `Sold/Building` | A building was sold |
| `Sold/Upgrade` | A building's upgrade was sold |
| `Sold/Unit` | A unit was sold at a repair depot |
| `Refund/Production` | Production was cancelled |
| `Refund/Undeploy` | A sold building's unit could not be placed on the map |
| `Refund/FreeUnit` | A building's free unit could not be placed on the map |
| `Crate` | A crate gave money |
| `Stolen/Storage` | A captured building's stored resources went to the capturing house |
| `Starting/Multiplier` | A computer player's credit multiplier was applied at the start |
| `Starting/CarryOver` | Money was brought over from the previous campaign mission |
| `Trigger` | A trigger action gave credits |

## 13. Reference files

| Path | Content |
|---|---|
| `docs/replay/REPLAY-FORMAT.md` | This specification |
| `docs/replay/replay.proto` | The schema of every record |
| `docs/replay/launch.proto` | The schema of the launch request |
| `tests/replay/vectors/` | Test vectors, listed in its manifest `vectors.json` and generated by `tools/replay/make_vectors.py` |
| `tools/replay/cncreplay.py` | The Python reference reader |

The test vectors cover `S0` for a known set of sections, a chain of three
intervals, a final seal on an interval boundary, a minimal complete file, a
string with an invalid UTF-8 sequence, a checkpoint at frame 0, a set
`optional` message field with an empty encoding, one chat fingerprint, one
group order with a `sealed_payload`, and a file for each rule that makes a
file malformed or not verified.
