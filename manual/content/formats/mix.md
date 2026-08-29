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

Registered MIX archives expose their members through the ordinary file layer, so a member is opened by name exactly as a file on disk is. A loose file with the requested name takes precedence over a member in a loaded archive, which permits local override files. Archive members are read-only through this file layer; a file opened for writing is always a real file on disk.

## Mounting and search order

An archive is mounted by name, and the order archives are mounted in is the order they are searched. Startup mounts the patch archives first, then the numbered expansion archives counted down from the highest number present, then the shipped archives; a theater or a side mounts its own later and drops them again when the theater or side changes. The first archive holding a member of the requested name answers for it, so an archive mounted earlier shadows that name in every archive mounted after it.

Startup insists on a few of them: `CACHE.MIX`, and then `CONQUER.MIX`, `SOUNDS.MIX`, `SCORES.MIX`, a movie archive, and `SOUNDS01.MIX` where the expansion is installed. The game stops during startup without one of those. Every other archive is mounted when present and passed over when not, the map and multiplayer archives among them, so a deployment holding the maps and the multiplayer content loose or in archives of its own still starts.

Names are matched without regard to case. Each archive carries an index of its members sorted by a checksum of the member name, and a lookup is a binary search over that index rather than a scan, so nothing depends on the order members were packed in.

Mounting an archive that is not there registers nothing and reports nothing: the object is created, finds no file, and never joins the list of archives to search. A name that no archive holds is not recorded anywhere either — the request falls through to opening a real file of that name, and fails when there is none.

## Caching

An archive can be cached, which reads all of its member data into memory in one operation. The index is read when the archive is mounted whether or not the archive is ever cached; caching concerns only the member data. An archive may carry a message digest, which is checked as the archive is cached and refuses the cache when it does not match, and its index may be encrypted, which the game detects from the start of the file and decrypts as it reads.

Whether an archive is cached decides how its members can be reached:

- A member of a cached archive can be handed out as a pointer straight into the memory the archive is already holding. Nothing is allocated for the member and nothing is copied. Shapes, fonts, palettes and sound samples are fetched this way, so those files have to live in an archive that was cached — a loose file, or a member of an archive that was mounted without being cached, is not found by that path at all.
- Opening a member as a file works either way. From a cached archive the file object becomes a window onto that same memory and a read copies out of it; from an archive that is not cached the archive file itself is opened and every read is biased to the member's position within it.

The memory belongs to the archive rather than to whatever asked for the member, so a pointer fetched the first way stays good only while that archive stays cached and mounted, and handing it back is not something its holder may do. [`FreeBuildup`](/keys/freebuildup/) describes what a type that hands one back costs the rest of the game.

:::danger[An archive too short to hold a header is mounted from uninitialized memory]
The number of members and the size of the data block are taken from the first bytes of the file without testing that any bytes were read. An archive that cannot supply them — an empty file most obviously — is mounted with a member count and an index taken from whatever that memory last held. Allocating an index for an implausible count is not survivable; where the count is small enough to allocate, the archive joins the search carrying meaningless entries, and a request that matches one is handed an offset and a size that describe no file.
:::
