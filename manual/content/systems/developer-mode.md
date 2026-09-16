---
title: Developer mode and diagnostics
summary: Compiles the engine's cheat keys, diagnostic displays and assertions into the Debug configuration, and arms the keys handled in code with any command line argument.
category: tools-diagnostics
keys:
  - Cell
  - CheckHeap
  - Coord
  - Frame
  - Inert
  - MovieTime
  - PrintCRC
  - Target
  - Type
related:
  - type: using
    id: project-status
---

The engine's diagnostic surface is split between the two build configurations. A Release build has the code recognizer on the main menu, the version dialog, the multiplayer statistics file and the launch options that are not guarded. The debug log is written in both configurations. The keys handled directly in code, the monochrome pages and the assertions exist only where the Debug configuration defines `_DEBUG`. [The command reference](/commands/) records which build each command, control and launch option belongs to.

## Arming the debug keys

The keys handled directly in code are gated by two flags, and no in-game control raises either one. The loop that walks the command line raises both at the top of every pass, above the tests that identify the argument. The flags therefore depend on whether an argument was supplied, rather than on which argument it was.

:::caution[Any argument at all arms the code-handled keys]
A Debug build started with a windowed-mode option, a resolution, or a map name has exactly the same keys live as one started with the playtest option. Started with no argument, both flags stay down and every key handled in code is inert, including the ones that grant money and force a win. [The fixed controls](/commands/fixed-controls/) record which flag each of those keys answers.
:::

## What a Debug build allocates

A Debug build allocates the counters behind the events page during startup, one for each step the page times. They count in processor time-stamp ticks of sixteen cycles each, and the events page and the benchmark overlay both draw from them.

## The debug log

Every diagnostic line the engine emits goes to [the debug log](/using/debug-logging/) and to the attached debugger, in either configuration. That log is opened once, on the first line that needs it, and the handle stays open between writes, so each line reaches it as it is reported.

A Debug build also opens a console window titled `Debug Console` as part of that setup; a Release build opens it only on request. A second reporting path writes text with no time stamp and reaches the same destinations; it is what the main menu uses to echo characters as they are typed.

## Assertions

Assertions use the C runtime's `assert`, so they are live wherever `NDEBUG` is undefined, which is the Debug configuration. A failed assertion ends the run through the C runtime's abort path, and the crash reporter records it like any other fault. There is no continue path.

## The benchmark overlay

A Debug build with the debug keys armed shows a frame benchmark window on [F6](/commands/fixed-debug-benchmark-overlay/) and hides it on the next press or through the window's own close button. The window is drawn by Dear ImGui over the presented frame, and follows the frame's position and scale. It reports the logic frames and the presents of the last second, the frame number, the present interval, and the frame benchmarks the Events page shows, as a share of the frame and an average in microseconds when the processor speed could be measured, in ticks otherwise. The five counters the engine never starts are marked as such.

While the monochrome display is off, the window resets the benchmarks once a second and shows the second just gone; while that display is on, the Events page keeps its reset and the window shows the live running averages, so the two never take samples from each other. A button resets on demand. The window takes the mouse only while the pointer is over it and the keyboard only while one of its fields has focus; everything beside it reaches the game. A visible menu dialog takes the mouse over its own area before the shell sees it, so over a dialog the window answers the pointer only where it covers the frame beside the dialog. A switch opens the Dear ImGui demo window, which exercises the renderer.

## The monochrome pages

The monochrome display is a four-page text surface driven through a monochrome display device rather than drawn on screen. Enabling it only raises a flag; nothing verifies that the device is there. The first screen clear the device refuses lowers the flag again. On a machine without that device, the pages switch themselves off on the first diagnostic pass and stay off until something enables them again.

While it is enabled the pages refresh once a second, and the page in view is the one that draws, with the single exception the table names.

| Page | What it draws |
| --- | --- |
| Object | A dump of the last selected object, cleared and redrawn each pass |
| House | A dump of that object's owning house, drawn only where that object is an infantry, a vehicle, an aircraft or a structure |
| Stress | The logic layer's own dump, which is written every pass whether or not the page is in view |
| Events | The frame benchmarks, as a percentage of frame time and an average duration for each tracked process |

The stress page has no body of its own beyond the logic dump. The benchmark figures are reset after every pass, so the events page reports the second just gone rather than a running total. The rules and scenario timings are the two exceptions, and the reset leaves them alone.

## Motion capture

A Debug build binds a key that raises the motion-capture flag. The routine that flag gates would grab the client area into an off-screen surface once per frame and hold one surface per captured frame. The sequence holds [`MovieTime`](/keys/movietime/) minutes' worth of frames, and when it is full the routine writes the whole thing out as numbered `cap0000.pcx` files and switches itself off.

Nothing calls that routine in either configuration. The key raises a flag that reaches nothing, no frame is ever captured, and no value of `MovieTime` changes anything.

## Multiplayer statistics

An Internet session writes `mpstats.txt` when its game loop finishes, in either configuration. The file reports the frame count, the average frame rate, the largest look-ahead the session reached, the latency and game-speed settings, and each local address. Each connected player then gets a block with that player's address, maximum and maximum-average round trip, resend count, frame-sync and command-count stalls, and both the absolute and percentage packet loss.

## The sync dump

A desynchronized network game writes an [out-of-sync report](/using/out-of-sync-reports/) into the `Debug` folder beside the executable, and the [`PrintCRC`](/keys/printcrc/) playback trap writes one when it reaches its frame. Both are the same report, which holds the build, the session identity and seed, the recent frame-checksum ring and the offending event. It also holds bounded histories of the random draws, targeting, missions, facings, animations and events leading up to the divergence, plus per-object and per-heap checksums keyed to each object's stable identifier.

Seven `sun.ini` settings exist to diagnose a desynchronized game, and only [`PrintCRC`](/keys/printcrc/) acts. The other six are read from the file and reach nothing further. [`Frame`](/keys/frame/), [`Type`](/keys/type/#scope-multiplayer-settings), [`Coord`](/keys/coord/), [`Target`](/keys/target/) and [`Cell`](/keys/cell/) describe an object for a per-frame hunt whose body is compiled into neither configuration. [`CheckHeap`](/keys/checkheap/) raises a flag no reader reads.

`CheckHeap` sits in `[MultiPlayer]`; the rest sit in `[SyncBug]`, which is read only while a recorded game is being played back. Both blocks are read as a session outside a campaign is set up, so a campaign game reads none of the seven.

## Crash reporting

An unhandled exception writes a folder of its own under `Exceptions`, beside the executable and named for the time of the crash. The folder holds a minidump, a readable `except.txt` report, and a copy of the end of that run's debug log, so reporting a crash means attaching one folder. The report names the fault and the address involved, and identifies the crash site by function, file and line. It also holds two independently derived call stacks, the registers, the loaded modules and a scan of the stack. Addresses are resolved against the symbol file shipped beside the executable rather than against whatever directory the game was launched from.

The handler goes in as the process-wide unhandled-exception filter before the window, sound and renderer exist. A crash during startup and a crash on any thread are both reported. A debugger attached to the process sees the exception first and the handler never runs, which is why no option is offered to stand it down. Crash folders older than thirty days are removed at startup, and the copied portion of the log is capped at 256 KiB.

## The Release configuration's own surface

### The main-menu code recognizer

The classic main menu accumulates alphanumeric keystrokes into a buffer of up to 31 characters. It flips a flag the moment a recognized code appears anywhere inside what has been typed. Any non-alphanumeric character clears the buffer, and so does a successful match. Each code is a toggle, so typing it a second time flips the flag back.

| Code | What it flips |
| --- | --- |
| `PENGO` | The visceroid art replacement |
| `THETEAM` | A skirmish-only rules overlay read from `TMCJ4F.INI` |

Neither is marked as surviving into multiplayer, so starting a network game clears both. Both exist only as codes; no setting reaches them.

### The version dialog

The version dialog reports the title, the game and internal version names, and a build line. That line is labeled by configuration and names the commit the build was made from, the branch it sat on and that commit's date. The dialog also reports the CPU vendor and the version of the language resource library. It opens from the classic main menu's [Ctrl+V](/commands/fixed-main-menu-version/) and from the menu entry, and is drawn as an RmlUi document from the `ui` directory. OK, Enter and Escape close the document. When its document, style sheet or font fails to load, the game logs the file name and closes the screen at once. [UI files](/systems/ui-files/) covers the directory.

## Toggles that reach nothing

Several diagnostic switches survive as flags that no reader reads, so invoking them changes only the flag.

- The frame-rate toggle raises a flag with no reader anywhere in the engine; no counter is drawn.
- The cell-icon overlay toggle raises a flag with no reader and forces a full redraw, which is the only visible consequence.
- The passability flag has neither a writer nor a reader.
- The map-checking launch option reaches its per-frame check, but the routine that check calls has no body and always reports success, so no map is examined.
- [The debug special dialog](/commands/fixed-debug-special-dialog/) asks for a dialog that is not compiled, and outside a campaign or skirmish game the request is never cleared.
