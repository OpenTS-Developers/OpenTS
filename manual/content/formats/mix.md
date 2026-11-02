---
format_id: mix
title: MIX archives
summary: Stores named game files in CRC-indexed archive members.
kind: binary
extensions:
  - .MIX
role: archive
source_files:
  - code/mixfile.h
  - code/mixfile.cpp
  - code/ccfile.cpp
  - code/init.cpp
---

Once the game mounts a MIX archive, each of its members opens by name like a file on disk. A loose file with the same name takes precedence over the member, so a loose file can override archive content. The exception is a file the game reads only from cached archives, which [Caching](#caching) lists. Archive members are read-only; a file opened for writing is always a file on disk.

## Mounting and search order

The game searches archives in the order it mounted them. The first archive that holds a member of the requested name supplies it, so an archive mounted earlier overrides that name in every archive mounted after it. Names are matched without regard to case.

Startup mounts these archives in this order:

| Order | Archive | Required | Cached |
| --- | --- | --- | --- |
| 1 | `PATCH.MIX` | No | No |
| 2 | `PCACHE.MIX` | No | Yes |
| 3 | `EXPAND99.MIX` down to `EXPAND00.MIX` | No | No |
| 4 | `ECACHE99.MIX` down to `ECACHE00.MIX` | No | Yes |
| 5 | `TIBSUN.MIX` | No | No |
| 6 | `CACHE.MIX` | Yes | Yes |
| 7 | `LOCAL.MIX` | No | No |
| 8 | `CONQUER.MIX` | Yes | Yes |
| 9 | Every `MAPS*.MIX`, in alphabetical order | No | No |
| 10 | `MULTI.MIX` | No | No |
| 11 | `SOUNDS01.MIX` | Where the Firestorm expansion is installed | Yes, when audio is available |
| 12 | `SOUNDS.MIX` | Yes | Yes, when audio is available |
| 13 | `SCORES.MIX`, then `SCORES01.MIX` | No | No |
| 14 | Every `MOVIES*.MIX`, in alphabetical order | No | No |

`PATCH.MIX` and the `EXPAND` archives are mounted only as loose files, never as members of another archive.

If a required archive is missing or cannot be cached, the game stops during startup. An optional archive that is missing is skipped, and later lookups do not search it. A deployment can therefore keep the maps and the multiplayer content loose or in other archives.

A music track or movie whose file is not found in any archive or folder is skipped.

### Theater, side and speech archives

Archives mounted after startup are searched after every startup archive.

When a scenario or saved game uses a different theater from the one last loaded, the game drops the previous theater's archives and mounts the new theater's. These are `<Root>.MIX` and `<Suffix>.MIX`, both cached, then `<IsoRoot>.MIX`, not cached. [`Root`](/keys/root/), [`Suffix`](/keys/suffix/#scope-theater) and [`IsoRoot`](/keys/isoroot/) are set per theater.

Each time a scenario or saved game loads, the game drops the previous side's archives and mounts those of the player's side. The two-digit side number `<nn>` is the side's position in the side list: `01` for the first side, `02` for the second. The archives are mounted in this order:

1. `E99SC<nn>.MIX` down to `E00SC<nn>.MIX`, cached, only while an expansion is enabled.
2. `SIDEC<nn>.MIX`, cached. Required.
3. `E99SNC<nn>.MIX` down to `E00SNC<nn>.MIX`, not cached, only while an expansion is enabled.
4. `SIDENC<nn>.MIX`, not cached. Optional.
5. In a campaign only, one CD archive, not cached. Optional.

For the CD archive, a campaign mission first tries `E<xx>SCD<nn>.MIX` while an expansion is enabled, where `<xx>` is the mission's [`RequiredAddOn`](/keys/requiredaddon-scenarios/) number: a Firestorm mission tries `E01SCD01.MIX` or `E01SCD02.MIX`. Without that archive it mounts `SIDECD<nn>.MIX`, so an installation that keeps the expansion's members in the base archive still plays the expansion's campaign.

A campaign that finds neither CD archive still starts. The stock CD archives hold the score screen's picture and movie and the map selection's artwork, palettes and voice lines, so these must then come from another archive or folder:

- A score picture or score movie that is not found is left out.
- Map selection between missions cannot open without its artwork and palettes. The game reports "Unable to initiate Map Selection!" and replays the mission just won, unless that mission sets [`SkipMapSelect=yes`](/keys/skipmapselect/).

If a required side archive is missing, the game mounts the first side's archives instead. If those are missing too, the scenario or saved game fails to load, and a scenario reports that it cannot be read.

Speech archives follow the same pattern for the player's side, or in a campaign for the mission's [`SpeechSide`](/keys/speechside/): `E<xx>VOX<nn>.MIX` for each enabled expansion, then `SPEECH<nn>.MIX`, which is required. A missing `SPEECH<nn>.MIX` falls back to the first side's in the same way. None of the speech archives is cached.

## Caching

A cached archive holds all of its member data in memory from the moment it is cached. An uncached archive stays on disk, and a member is read from it when opened. The tables and lists above say which archives the game caches. Every archive's index is read when the archive is mounted, whether or not it is cached.

Whether an archive is cached decides how its members can be read:

- Many files are read only from cached archives. They include the shapes of object and animation types that are not [demand-loaded](/keys/demandload/), the game fonts, the theater palettes and the mouse cursor. A loose copy of such a file, or a copy in an archive mounted without caching, is not found.
- Any file the game opens by name can come from either kind of archive.

To replace a file of the first kind, put the replacement in `PCACHE.MIX` or an `ECACHE` archive. Do not also put a copy in an uncached archive that the game searches earlier: `PATCH.MIX` for a file in `PCACHE.MIX`, or `PATCH.MIX` or any `EXPAND` archive for a file in an `ECACHE` archive. The game finds that uncached copy first and treats the file as missing.

An archive may carry a digest of its member data. The game checks it when caching the archive and leaves the archive uncached if the digest does not match.

## File layout

An archive starts with a header, then its index, then the member data:

| Part | Contents |
| --- | --- |
| Flags | Optional. Two zero bytes, then a 16-bit field: bit 0 set means a digest follows the data, bit 1 set means the header and index are encrypted. An archive that does not start with two zero bytes has neither. |
| Header | The number of members, a 16-bit integer, then the size of the member data in bytes, a 32-bit integer. |
| Index | One 12-byte entry per member: the checksum of the member's name, the member's offset from the start of the data, and its size in bytes, each a 32-bit integer. |
| Data | The members' contents. Member data is never encrypted. |
| Digest | Present only when flag bit 0 is set. A 20-byte SHA-1 digest of the data. |

The checksum is computed from the member's name in upper case, without a path. Keep the index sorted by checksum in ascending order, compared as signed 32-bit integers. The game finds a member by binary search over the index, so an entry out of order may not be found. The order of the members in the data does not matter.

:::danger[Do not mount an empty or truncated archive]
Keep every archive the game mounts at least as long as its header and index. If the file is shorter, for example an empty file, the missing member count, data size and index entries are taken from leftover memory. A negative member count crashes the game when it mounts the archive. Otherwise the archive joins the search with meaningless entries, and a request that matches one of them reads data from a wrong position.
:::
