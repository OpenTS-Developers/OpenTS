---
title: Debug logs and console
summary: Every run writes a timestamped log beside the executable, and the debug console can be opened to watch that output live.
category: troubleshooting
source_files:
  - code/dbgprint.cpp
  - code/startup.cpp
  - code/init.cpp
related:
  - type: using
    id: developer-build-troubleshooting
  - type: command
    id: launch:console-debug
---

## Where the log is written

Every run writes a log to a `Debug` folder beside the executable, named for the moment the
process started:

```
Debug/DEBUG_17-08-2026_06-00-35.LOG
```

Release and debug builds both write one. Each run gets its own file, so the log from a run
that crashed is still there after the next launch. When two processes start within the same
second, the second one adds its process id to the name.

## What the log opens with

Every log begins with a banner naming the build that wrote it:

```
Version  : OpenTS 0.1.0 (release build)
Commit   : f6842d2c on main (modified)
Committed: 2026-08-17 06:30:39
Started  : 2026-08-17 06:56:51
System   : Windows 10.0.26200
Options  : -XC
```

`modified` means the build was made from a working copy with edits, so it is not exactly the
named commit. `Options` lists the launch options the game was started with.

## Reading the rest

Each line after the banner is stamped with the time it was written:

```
[06:00:35.412] Video: renderer is Direct3D 11
```

Some records are assembled as the engine works through a step, and are stamped once where the
line starts rather than at each addition:

```
[06:19:45.401] Bootstrap..... PATCH.MIX EXPAND03.MIX CACHE.MIX ...OK
```

Logs last written more than fourteen days ago are deleted at startup. A single log stops
growing at 64 MB, and records that it has done so on its last line.

The folder is created next to the executable, so the game needs write access to its own
directory. If the folder or the file cannot be created, the game still runs and the output
still reaches the console and an attached debugger; only the file is missing.

## Opening the console

Debug builds always open the console. Release builds open it when
[`-XC`](/using/command-line/console-debug) is passed, which is recognized early enough that
even messages written during startup appear. The window holds about 4000 lines of scrollback.

Its close button is disabled, because closing a console window terminates the program that
owns it. Close the game itself instead.

The console also carries ordinary program output that a windowed application otherwise has
nowhere to display, such as the `-?` command line help. When the command line is rejected,
the game waits for a keypress before exiting so that the message stays readable.

## Before sharing a log

A log describes what the engine did, and in multiplayer that includes other people. Expect to
find player names as typed in the lobby, and network addresses of the machines in the game.
Read a log before attaching it to a public bug report.
