# The saved game format

A saved game is one `.SAV` file written by `code/savefile.cpp` and read back by
it. This document owns the layout. Where the files live, how they are named,
and when they are written is on the manual's
[save games page](../manual/content/formats/save-games.md).

Every integer is little-endian. Offsets are from the start of the file.

## Header

| Offset | Size | Field |
| --- | --- | --- |
| 0 | 4 | Signature, the bytes `OTSV` |
| 4 | 2 | Format version, currently 1 |
| 6 | 2 | Flags; bit 0 set when the content is LZO-compressed |
| 8 | 4 | Length of the field table |
| 12 | 4 | Offset of the content |
| 16 | 4 | Stored length of the content |
| 20 | 4 | Uncompressed length of the content |
| 24 | 4 | CRC-32 of the stored content |
| 28 | 4 | CRC-32 of the first 28 bytes of the header, continued over the field table |

The header is 32 bytes, the field table follows it directly, and the content
follows the table directly. The content offset is recorded rather than assumed
so a later format version can put something between the two; this version
refuses a file whose offset says otherwise. A field table is refused above
1 MiB, since a listing is a dozen short fields.

Both checksums are the CRC-32 of IEEE 802.3, polynomial `0xEDB88320`
reflected, initial value and final complement of all ones, as PNG and gzip
use it. The header checksum continues over the field table so a listing can
verify what it shows without reading the content.

## Field table

The fields are what the load dialog lists a save by. Each is:

| Size | Field |
| --- | --- |
| 2 | Identifier |
| 2 | Kind: 1 string, 2 integer, 3 file time |
| 4 | Length of the value |
| | The value: string bytes without a terminator, a 4-byte integer, or an 8-byte `FILETIME` |

The identifiers are the `PIDSI_` values in `code/savever.h`, the same ones the
compound-document property set carried before this format. A field holds at
most 64 KiB. A reader takes the
first field that matches both identifier and kind and ignores the rest, so a
field it does not know costs nothing. A string longer than the buffer it is
read into is cut on a character boundary, so a shortened description stays
UTF-8. `SaveVersionInfo` in `code/savever.cpp` is the only writer and reader.

## Content

The content is the game state: the bytes `Put_All` in `code/saveload.cpp`
writes through `SaveStreamClass`, compressed as one block with LZO1X-1 when
that makes it smaller, and stored as it is otherwise. The reader checks the
stored length and checksum before decompressing, and refuses a block that does
not expand to exactly the recorded length. An uncompressed length above 256 MiB
is refused before anything is allocated for it.

The block is decompressed through `lzo1x_decompress_safe`, which stops at the
end of the output buffer, so a block forged to expand past the recorded length
is refused rather than written past it. The records after the header are still
read into live objects, so treat a save file from an untrusted source as
untrusted input.

### Object records

The state is a sequence of values and object records in the order `Put_All`
names them. An object record is:

| Size | Field |
| --- | --- |
| 16 | The class identifier of the object |
| 4 | Length of the record body |
| | The body: the swizzle identity, then the members the class's `Serialize` names |

The class identifier is the `CLSID` the object's `GetClassID` reports, the
same one registered with the class factory in `code/startup.cpp` and, for a
locomotor, named by the `Locomotor=` key. The reader creates the object
through `CoCreateInstance`, hands it the stream, and then checks that it
consumed exactly the recorded length. A record that comes up short or long
fails the load with the object's type and offset in the debug log, which is
what a member added to one build and not the other looks like. A vector of
objects is a 4-byte count followed by that many records, and a locomotor
nested inside a unit's record is a record of its own. A count that the bytes
remaining in the content could not hold fails the load before anything is
allocated for it.

The body is what each class's `Serialize` produces, member by member, in host
byte order. It is not described here; the classes are the description.

The swizzle identity and every pointer member travel as four bytes. The save
numbers the objects it meets rather than writing the address one sat at, so
the body depends neither on the pointer width of the build that wrote it nor
on where the objects were in memory.

## Versions

Two numbers gate a save. The format version in the header says how to parse
the file, and a reader refuses a version above its own. The header flags are
gated the same way: a reader refuses a file with a flag bit it does not know,
so a later version can mark content it stores differently without moving the
format version. The internal version in the field table,
`PIDSI_INTERNAL_VER`, is `ExpectedGameVersion`, the packed project version,
and a save whose value differs from the running build's is not offered to the
player. The format version moves only when the layout in this document
changes; the internal version moves with every release.

## What the reader refuses

`SaveFileClass::Read` and `Read_Fields` answer one of:

| Result | When |
| --- | --- |
| `RESULT_MISSING` | No file under that name |
| `RESULT_NOT_A_SAVE` | The first bytes are not the signature |
| `RESULT_UNSUPPORTED_VERSION` | A format version above the reader's, or a header flag it does not know |
| `RESULT_CORRUPT` | A length, checksum or compressed block that does not add up, including a truncated file, a forged block, a field table above 1 MiB, a content offset that does not follow the table, or a content length above 256 MiB |
| `RESULT_NO_MEMORY` | A file within those limits that the process cannot hold |

`Read` judges the header before it reads or allocates anything else, so a file
of any size costs the reader no more than the limits above allow, and
`Read_Fields` reads the header and the table only, so listing a folder never
allocates for a file's content.

`Load_Game` reads and checks the whole file before it tears down the running
game, so a refused file costs nothing.

A save written before this format is an OLE compound document, which begins
with a signature of its own, so the reader answers `RESULT_NOT_A_SAVE` and the
load dialog leaves the file out of its list. Nothing converts those files.

## Writing

`SaveFileClass::Write` builds the whole image in memory, writes it to the
target name with `.tmp` appended, flushes and closes it, and then moves it over
the target with `MoveFileExA` and `MOVEFILE_REPLACE_EXISTING`. A save
interrupted at any point leaves the previous file untouched under its name,
and at most a `.tmp` beside it, which the next successful save replaces.
The reader's limits bind the writer too: content above 256 MiB, a field above
64 KiB or a table above 1 MiB is refused with `RESULT_TOO_LARGE` before
anything is written, so a save this build writes is one it reads, and the
file on disk is left as it was.

## Checks

`tests/save` builds `code/savefile.cpp` against the vendored LZO library and
covers the round trip, the fields-only read, replacement of an existing file
and of a stale `.tmp`, and each refusal above, including a later version, an
unknown flag, a file cut at every boundary, a byte flipped in the header, the
table and the content, a field table above its limit, a gap before the
content, a block that ends before or expands past its declared length, and a
write above each limit that leaves the earlier save in place. It reads no game
data.
