---
title: Out-of-sync reports
summary: A network game that goes out of sync writes a checksum report beside the executable, so two players' reports can be compared to find where they diverged.
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
for the same frame, the two games have diverged. From that point on they are out of sync and
playing different games. The engine writes a report describing its own state at that frame, so
the divergence can be tracked down. Only then does it ask the players what to do, and
[out-of-sync recovery](/systems/out-of-sync-recovery/) owns the dialog that follows.

## Where it is written

The report is written into the `Debug` folder beside the executable, the same folder the debug
log uses. The name gives the local player's house number and the frame it was written on. The
local date and time sit between them, in day-month-year order:

```
Debug/SYNC_H0_02-09-2026_18-42-07_F1530.LOG
```

A machine writes at most one report per frame, however many players disagreed on it. It never
reports a player it has already reported, and it stops after three reports in one game, so a game
that diverges and is continued does not fill the folder. A report is deleted once it is more than
thirty days old; the folder is swept each time a new report is written. If the `Debug` folder
cannot be created, the report falls back to a `SYNC<n>.TXT` in the working directory, where
`<n>` is the same house number.

Two players reporting the same divergence usually name different frames in their file names, since
each compares against the delay the other reported. Match a pair by the session identity, seed and
checksum ring inside the files rather than by their names.

A diverged network game writes the report on its own, with nothing to arm first. The
[playback trap](/keys/printcrc/) is how the same report is produced without one.

## What it holds

The report is a plain text file. Most lines are a `Label: value` pair, and each section is
introduced by a heading.

The report opens with the build, the local player, and the keys that let two players' reports be
matched: the session identity and the random seed. It then records the connection statistics and
the recent frame checksums, newest first.

The state of every house's objects follows, keyed by each object's stable identifier. After that
come one block for each player whose checksum disagreed, giving that player's name and both sides'
values. Then come bounded newest-first histories of the moments before the divergence: the random
draws, target assignments, mission orders, facing assignments, animation creations and events. The
file closes with a per-heap checksum table.

Each history line records the frame and the call site that produced it. The call site is printed
as an offset within the game image, and as the address that offset maps to in the build's map
file. When the matching `.pdb` sits beside the executable, the function and source line the call
came from are printed too. Two peers running the same build agree on the offset, so a history that
diverges points at the exact call that first differed, whether or not symbols were present to name
it. A call from outside the game image keeps only the low bits of its address, which does not
identify it on another machine.

The report is written without drawing from the game's random generator, so writing it does not
itself perturb a game that is still running. The histories in it stop at the frame it was written
at.

## Comparing two players' reports

Two reports describe the same divergence only when their `Session identity:` and `Seed:` lines
agree; check those first. With a matched pair, compare the frame-checksum rings to find the first
frame that differs, then compare the object tables. Each object line gives its stable
identifier rather than a heap slot, so the same object can be found in both reports. The field
that differs between them can then be located.

## Forcing a divergence on purpose

Passing [`-DESYNCTEST=<frame>`](/using/command-line/desync-test) on the command line corrupts this
machine's own checksum once, so the report can be checked on a given pair of machines without
waiting for a real divergence. Every machine in the session sees the mismatch and writes a report,
which is what makes a pair to compare. Arming it on one machine is enough.

## Before sharing a report

A report describes a network game, so it names the other players as they appeared in the lobby. It
does not contain network addresses. Read it before attaching it to a public bug report, the same as
a [debug log](/using/debug-logging/#before-sharing-a-log).
