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

A crash writes its files into a new folder under `Exceptions`, in the directory that holds the executable. The folder is named for the local time of the crash and the process id:

```
Exceptions/exception-20260817-061945-7420/
```

| File | Holds |
| --- | --- |
| `except.txt` | The readable report |
| `minidump.dmp` | A memory dump of the process, including the state of the thread that crashed |
| `debug-tail.log` | The last 256 KiB of that run's debug log |
| `fulldump.dmp` | The whole address space, written only when requested from the crash dialog |

Report a crash by attaching the whole folder. If the folder cannot be created, the files are written directly into the executable's directory instead.

When the game runs a [launch request](/formats/launch-request/#the-result-file) that names a result file, a crash also writes that file once the report is written, giving the folder's path.

The folder stays beside the executable when [`-USERDIR`](/using/command-line/#user-directory) names a user directory, so the game needs write access to the executable's directory to save a report.

At startup, the game deletes crash folders last modified more than 30 days ago.

When a debugger is attached to the game, the debugger receives the crash and no folder is written.

## What the report holds

The report opens with a header that identifies the build and the run:

- the local time of the crash;
- the version, the time the executable was linked, and whether it is a Release or Debug build;
- the executable's path and the command line the game was started with;
- the thread that crashed, marked when it is the main thread;
- a `Symbols` line, only when symbols are unavailable or do not match the executable.

The machine state at the moment of the crash follows, in this order:

1. The exception, its name, and a sentence saying what that kind of fault means. An error the engine raised itself adds its message.
2. The crash site, by function, source file, and line, and the code bytes at that address.
3. The registers.
4. Two call stacks. The first needs no symbols: the Win32 build follows the saved frame pointers, and the x64 build uses the executable's unwind tables. The second uses the symbol file, and is skipped when no symbol handler could be started.
5. The loaded modules and the address range each occupies.
6. How much memory was free: physical memory, page file, and address space.
7. A raw scan of the stack, marking the values that could be code addresses.

The report ends by naming the folder and saying whether the minidump and the log tail were written.

If one section faults while it is being written, the section ends with a note and the rest of the report is still written. The exception is a crash while the crash handler is still starting, or on a machine where it could not start its reporting thread. A fault in the report then ends the process, and only the files already written are kept.

## Addresses and symbols

The crash handler looks for the symbol file (`Game.pdb` or `GameD.pdb`) in the executable's directory, whatever directory the game was launched from. Keep the symbol file beside the executable to get function names and source lines in the report.

Without a usable symbol file, the report still identifies every address by module name and offset. The `Symbols` line in the header gives the reason:

- no symbol handler could be started;
- no symbol file matching this executable was found.

## What is covered

- A crash on any thread, not only the main one.
- A crash during startup, from the first step of the game's startup code. A fault is reported before the window, sound, and renderer exist. A fault while global data is being set up, before that step, writes no crash folder.
- A stack overflow on the main thread. An overflow on another thread is reported when enough of that thread's stack is left to run the handler.
- A call to a pure virtual function, a rejected argument to a C runtime function, a `terminate` call from the C++ runtime, and an aborted run.
- An unrecoverable error the engine reported itself. Its message appears in the report.

## The crash dialog

After writing the folder, the game shows the report and the folder's path in a dialog:

- `Save full dump` writes `fulldump.dmp`, which holds the whole address space and is much larger than `minidump.dmp`. Save one for a crash that the report cannot explain.
- `Debug` breaks into an attached debugger. The process ends when the debugger lets it continue. Without a debugger, `Debug` acts like `Quit`.
- `Quit` ends the process, as does closing the dialog.

## Raising a crash on purpose

[`-EXCEPTIONTEST=<fault>`](/using/command-line/#exception-test) raises a chosen fault, so crash reporting can be checked on a given machine without waiting for a real crash.

## Before sharing a folder

Read `debug-tail.log` before attaching the folder to a public bug report. It is the end of the debug log, and can include player names and network addresses from multiplayer sessions; see [Debug logs and console](/using/debug-logging/#before-sharing-a-log).
