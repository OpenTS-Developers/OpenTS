---
title: Crash reports
summary: A crash writes a minidump, a readable report, and the end of that run's debug log into a folder of its own beside the executable.
category: troubleshooting
source_files:
  - code/except.cpp
  - code/startup.cpp
  - code/init.cpp
related:
  - type: using
    id: debug-logging
  - type: command
    id: launch:exception-test
---

## Where a crash is written

A crash writes its files into a folder named for the time it happened and the process it came
from, under an `Exceptions` folder beside the executable:

```
Exceptions/exception-20260817-061945-7420/
```

| File | Holds |
| --- | --- |
| `except.txt` | The readable report |
| `minidump.dmp` | A summary dump, opened on the thread that crashed |
| `debug-tail.log` | The last 256 KiB of that run's debug log |
| `fulldump.dmp` | The whole address space, written only when asked for from the dialog |

Report a crash by attaching the whole folder. If the folder cannot be created, the files are
written beside the executable instead.

Folders older than thirty days are deleted at startup. That is separate from the debug log's
own two weeks, since a crash folder is worth keeping longer than an ordinary run.

## What the report holds

The report opens with a header naming the build and the run: the time of the crash, the
version, the time the executable was linked, whether it is a release or debug build, its path,
the command line it was started with, and the thread that faulted, marked when that thread is
the main one. A line about symbols appears only when they are missing or do not match.

What follows is the machine state at the moment of the fault, in this order:

- The exception, its name, and a sentence saying what that fault means. An error raised by the
  engine itself carries its own message here.
- The crash site, by function, source file, and line.
- The registers.
- Two separate call stacks, one read from the saved frame pointers and one reconstructed from
  the symbol file. A stack too damaged for the second still yields the first.
- The loaded modules and the address range each occupies.
- How much memory was free: physical, page file, and address space.
- A raw scan of the stack, marking the values that could be code addresses.

Each section is written under its own guard, so a section that faults while being written
leaves a note in its place rather than costing the rest of the report.

## Addresses and symbols

The engine ships a symbol file next to the executable and points the crash handler at that
directory, rather than at whatever folder the game was launched from, so a report from a
player's machine reads the same as one from a developer's.

A report made without a usable symbol file still identifies every address by module and offset.
The header distinguishes the two reasons: no symbol handler could be started, or no symbol file
matching this executable was found.

## What is covered

- A crash on any thread, not only the main one.
- A crash from the first instruction of the program. The handler chain goes on before anything
  that can fail, so a fault during startup is reported even when the window, sound, and
  renderer are not up yet.
- A stack overflow. The reporting path needs guaranteed stack room to run in, which threads
  must ask for; an overflow on a worker or an operating system callback thread is reported on a
  best-effort basis.
- A call to a pure virtual function, a rejected argument to a runtime function, a `terminate`
  call from the C++ runtime, and an aborted run.
- An unrecoverable error the engine reported itself, whose message appears in the report.

## The crash dialog

The dialog shows the report and the folder the crash was saved to, in a fixed-pitch face that
keeps the register columns and the stack scan aligned. `Save full dump` writes `fulldump.dmp`,
which contains the whole address space and is much larger than the summary dump; it is worth
saving for a crash the report cannot explain. `Debug` breaks into an attached debugger. `Quit`
ends the process, as does closing the dialog.

## Raising a crash on purpose

[`-EXCEPTIONTEST=<fault>`](/using/command-line/exception-test) raises a chosen fault on purpose,
so crash reporting can be checked on a given machine without waiting for a real crash.

## Before sharing a folder

`debug-tail.log` is part of a debug log and carries whatever that log did, including player
names and network addresses from multiplayer sessions. Read it before attaching the folder to a
public bug report; see [Debug logs and console](/using/debug-logging/) for what a log records.
