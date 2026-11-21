---
title: Out-of-sync reports
summary: A network game that goes out of sync writes a checksum report into the `Debug` folder, so two players' reports can be compared to find where they diverged.
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

An out-of-sync report is a text file that records one machine's game state on the frame it finds that another player's checksum does not match its own. The two checksums belong to an earlier frame, which the report names. Two players' reports, compared side by side, show where their games diverged.

Each machine in a network game computes a checksum of its game every frame. It sends that checksum to the other machines only on frames that are a multiple of the frame send rate, the number on the report's `FrameSendRate:` line. When a checksum from another player disagrees with the one this machine computed for the same frame, the two games diverged on that frame or earlier. The machine writes its report at that point, before it asks the players what to do. [Out-of-sync recovery](/systems/out-of-sync-recovery/) describes the dialog that follows.

A network game writes the report without any setup. Recording playback can also write a report through the [`PrintCRC`](/keys/printcrc/) trap, which then exits the game; that page gives the frame the report is written on.

## Where it is written

The report goes into the `Debug` folder that holds the [debug log](/using/debug-logging/#where-the-log-is-written): in the user directory with `-USERDIR`, and beside the executable otherwise. The file name gives the local player's house number, the local date and time in day-month-year order, and the frame the report was written on:

```
Debug/SYNC_H0_02-09-2026_18-42-07_F1530.LOG
```

Each machine limits how many reports it writes:

- at most one report per frame, naming every player who disagreed on that frame;
- no second report about a player it has already reported;
- at most three reports per game, counted again from zero when a game starts or a saved game is loaded.

Before it writes a report, the game deletes reports in the folder that are more than thirty days old.

Keep that folder writable. If the report file cannot be created there, no report is written.

## What it holds

The report is a plain text file. Most lines are a `Label: value` pair, and each section starts with a heading line. The sections appear in this order:

1. The build, the local player, and the `Session identity:` and `Seed:` lines that identify the game.
2. The game speed and network settings, and each other player's connection statistics.
3. The frame checksums this machine computed, newest first.
4. Every house, with its name, whether a human plays it, its color, its ID and its house type.
5. Every house's infantry, vehicles (headed `Units`), structures (headed `Buildings`) and aircraft, one line per object with its position, facing, current mission and type. Infantry and vehicle lines add the object's target and destination. Structure and aircraft lines instead identify the object itself: the last six hexadecimal digits of their `Tgt:` field are its unique ID.
6. Every object on each map layer, then every object in the list of objects the game updates each frame.
7. One block for each player whose checksum disagreed, giving that player's name and checksum and the frame the checksum was computed for. The block adds this machine's checksum for the same frame when its `Delay:` value is under 256. A report written by the `PrintCRC` trap has a note in place of these blocks.
8. Histories of the moments before the divergence, each newest first and limited in length: random number draws, executed and queued events, target assignments, mission orders, facing assignments and animation creations.
9. A checksum table. It gives a count and checksum for each list of objects and type definitions, then one row for each house, vehicle, bullet, team, trigger or other object in the game. Each row gives the object's unique ID and a running checksum of that object and every object before it in the same list.

Each history line records the frame it happened on. Target and mission lines name objects by their unique ID, which is the same on every machine that is still in sync. Animation lines do too, except those marked `(local only)`, which exist on one machine only.

Some history lines can differ between machines that are still in sync. Leave them out of a comparison:

- random draws whose two-letter code starts with `N`, which come from a generator used for sounds and other local effects; compare the draws marked `C`;
- the value on a facing line; compare facing lines by frame and call site only;
- animation lines marked `(local only)`.

Every history line except the event lines also records the call site in the game that produced it, as an offset within the game executable. Two machines running the same build print the same offset for the same call, so the first comparable history line that differs points at the call that first diverged. The call site is also printed in two other forms when the information is available:

- In a 32-bit build, the address that offset has in the build's `.map` file.
- When the matching `.pdb` sits beside the executable, the function and source line.

A call from outside the game executable is printed with an `extern:` prefix. That address does not identify the call on another machine.

Writing the report does not draw from the game's random number generator, so it does not change a game that is still running. Its histories end at the frame the report was written.

## Comparing two players' reports

Match reports by their contents. Two reports describe the same divergence only when their `Session identity:` and `Seed:` lines agree. The file names differ in the house number and usually in the time.

With a matched pair, compare the frame checksums to find the first frame whose checksums differ. Then compare the histories around that frame to find the first comparable entry that differs; its call site names the code that diverged.

To find the object whose state differs, compare the checksum table. In each list, the first row whose running checksum differs names that object's unique ID. The object lines earlier in the report can be compared as well, but infantry and vehicle lines carry no ID, so match those objects by house, type and position.

## Forcing a divergence on purpose

[`-DESYNCTEST=<frame>`](/using/command-line/#desync-test) corrupts this machine's checksum once, so the report can be checked on a pair of machines without waiting for a real divergence. Passing it on one machine is enough. When the corrupted checksum is compared, every machine in the session sees a mismatch and writes a report, which gives a pair to compare.

The corrupted checksum is compared only if it belongs to a frame whose checksum is sent, a multiple of the frame send rate. On any other frame, no machine sees the corruption and the test writes no report.

## Before sharing a report

A report names the other players as they appeared in the lobby. It does not contain network addresses. Read it before attaching it to a public bug report, as with a [debug log](/using/debug-logging/#before-sharing-a-log).
