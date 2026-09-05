---
title: Out-of-sync reports
summary: A multiplayer game that goes out of sync writes a checksum report into a folder beside the executable, so two players' reports can be compared to find where they diverged.
category: troubleshooting
source_files:
  - code/syncreport.cpp
  - code/syncrec.cpp
  - code/syncrechook.cpp
  - code/queue.cpp
related:
  - type: using
    id: debug-logging
  - type: using
    id: crash-reports
  - type: system
    id: out-of-sync-recovery
  - type: key
    id: PrintCRC
---

## What an out-of-sync report is

Every machine in a network game checksums its own copy of the game each frame and sends that
value to the others. When a machine receives a checksum that disagrees with the one it computed
for the same frame, the two games have diverged: they are out of sync, and from that point they
are playing different games. The engine writes a report describing its own state at that frame so
the divergence can be tracked down, and only then asks the players what to do;
[out-of-sync recovery](/systems/out-of-sync-recovery/) owns the dialog that follows.

## Where it is written

The report is written into the `Debug` folder beside the executable, the same folder the debug
log uses, named for the local player's house number and the frame it was written on:

```
Debug/SYNC_H0_02-09-2026_18-42-07_F1530.LOG
```

Each frame produces at most one report however many players disagreed on it, a player already
reported is not reported again, and a single game writes at most a few, so a game that diverges
and is continued does not fill the folder. Reports older than thirty days are removed at startup.
If the `Debug` folder cannot be created the report falls back to a `SYNC<n>.TXT` in the working
directory.

Two players reporting the same divergence usually name different frames in their file names, since
each compares against the delay the other reported. Match a pair by the session identity, seed and
checksum ring inside the files rather than by their names.

The [playback trap](/keys/printcrc/) writes the same report without a network game.

## What it holds

The report opens with the build, the local player, and the keys that let two players' reports be
matched: the session identity, the random seed, and the frame. It then records the connection
statistics, the recent frame-checksum ring newest first, one block per player whose checksum
disagreed carrying that player's name and both sides' values, and bounded newest-first histories
of the moments before the divergence: the random
draws, target assignments, mission orders, facing assignments, animation creations and events. It
closes with the state of every house's objects keyed by each object's stable identifier and a
per-heap checksum table.

Each history line records the frame and the call site that produced it. The call site is printed
as an offset within the game image, the address it maps to in the build's map file, and, when the
matching `.pdb` sits beside the executable, the function and source line the call came from. The
offset is what two peers running the same build always agree on, so a history that diverges points
at the exact call that first differed whether or not symbols were present to name it.

The report does not draw from the game's random generator while it is written, so reading it does
not itself perturb a game that is still running, and the random history therefore stops at the
divergence frame.

## Comparing two players' reports

Two reports describe the same divergence only when their session identity, seed and frame agree;
check those first. With a matched pair, compare the frame-checksum rings to find the first frame
that differs, then compare the object tables for that frame. Each object line carries its stable
identifier rather than a heap slot, so the same object can be found in both reports and the field
that differs between them located.

## Forcing a divergence on purpose

[`-DESYNCTEST=<frame>`](/using/command-line/desync-test) corrupts this machine's own checksum
once, so the report can be checked on a given pair of machines without waiting for a real
divergence. Every machine in the session sees the mismatch and writes a report, which is what
makes a pair to compare. Arming it on one machine is enough.

## Before sharing a report

A report describes a network game, so it names the other players as they appeared in the lobby. It
does not carry network addresses. Read it before attaching it to a public bug report, the same as
a [debug log](/using/debug-logging/#before-sharing-a-log).
