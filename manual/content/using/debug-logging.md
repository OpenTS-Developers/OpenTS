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

Every run, in Release and Debug builds alike, writes a new log to a `Debug` folder in the user directory. The file is named for the local time the process started:

```
Debug/DEBUG_17-08-2026_06-00-35.LOG
```

Because each run gets its own file, the log from a run that crashed is still there after the next launch. When two processes start within the same second, the second adds its process id to the name.

When the user directory is not specified, i.e. no [`-USERDIR`](/using/command-line/#user-directory) command-line option is provided, the log is placed into a platform-dependent default directory. For Windows, this is the directory that holds the executable. On Linux, it is `$XDG_STATE_HOME/OpenTS/` (or `$HOME/.local/state/OpenTS/` in case `$XDG_STATE_HOME` is not specified), and on Mac, it is `$HOME/Library/Logs/OpenTS/`. In case the game doesn't have write access to this directory (especially on Windows), the [`-USERDIR`](/using/command-line/#user-directory) command-line option should be used.

At startup, the game deletes logs last written more than 14 days ago. A single log stops growing at 64 MB, and its last line then says that the size limit was reached.

## What the log opens with

Every log opens with the OpenTS wordmark and a banner that identifies the build. For example:

```
Version  : OpenTS 0.2.0 (x86 release build)
Commit   : f6842d2c on main (modified)
Committed: 2026-08-17 05:30:39
Started  : 2026-08-17 06:00:35
System   : Windows 10.0.26200
Codepage : ANSI 65001, OEM 437
Options  : -XC
```

- `Version` gives the platform, `x86` or `x64`, and whether this is a release or debug build.
- `modified` means the build was made from a working copy with edits, so it does not exactly match the named commit.
- `Codepage` shows the text code pages Windows gave the game. The game asks for UTF-8, which is code page 65001. Windows versions before Windows 10 version 1903 ignore that request.
- `Options` lists the launch options the game was started with.

## Reading the rest

Each line after the banner starts with the time it was written:

```
[06:00:35.412] Video: renderer is Direct3D 11
```

Some lines are written in several parts while the engine works through a step. Such a line carries one time, from when its first part was written:

```
[06:19:45.401] Bootstrap..... PATCH.MIX EXPAND03.MIX CACHE.MIX ...OK
```

## Opening the console

Debug builds always open the console. Release builds open it when [`-XC`](/using/command-line/#console-debug) is passed. The console opens before the game writes its first log line, so it shows the whole log, including startup. It keeps about 4,000 lines of scrollback.

The console's close button is disabled, because closing a console window ends the program that owns it. Close the game instead.

The console also shows text that a windowed program has no other place to display, such as the help that `-?` prints. In a Release build, pass `-XC` together with `-?` to read that help.

After printing the help, and when it rejects an option or cannot use a directory named on the command line, the game waits for a keypress in the console before it exits, so the text stays readable.

## Before sharing a log

A log describes what the engine did, and in multiplayer that includes other people. Expect to find player names as typed in the lobby and the network addresses of the machines in the game. Read a log before attaching it to a public bug report. The same applies to an [out-of-sync report](/using/out-of-sync-reports/).
