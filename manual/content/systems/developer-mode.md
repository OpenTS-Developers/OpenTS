---
title: Developer mode and diagnostics
summary: Compiles the engine's cheat keys, diagnostic displays and logging into the Debug configuration alone, and raises the flags that arm them as soon as the command line carries any argument.
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

The engine's diagnostic surface is split between the two build configurations. A Release build carries the code recognizer on the main menu, the version dialog, the multiplayer statistics file and the launch options that are not guarded. Everything else — the keys handled directly in code, the monochrome pages, the debug log and the assertion report — exists only where the Debug configuration defines `_DEBUG`. [The command reference](/commands/) records which build each command, control and launch option belongs to.

## Arming the debug keys

The keys handled directly in code are gated by two flags, and no in-game control raises either one. The loop that walks the command line raises both at the top of every pass, above the tests that identify the argument, so the flags depend on whether an argument was supplied rather than on which argument it was.

:::caution[Any argument at all arms the code-handled keys]
A Debug build started with a windowed-mode option, a resolution, or a map name has exactly the same keys live as one started with the playtest option. Started with no argument, both flags stay down and every key handled in code is inert, including the ones that grant money and force a win. [The fixed controls](/commands/fixed-controls/) record which flag each of those keys answers.
:::

## What a Debug build allocates

A Debug build creates two extra objects during startup. The first is the scenario editor, which is built whether or not anything ever enters editor state. The second is the set of frame benchmarks, which count in processor time-stamp ticks of sixteen cycles each. A Release build creates neither.

## The debug log

Every diagnostic line a Debug build emits goes to `DEBUG.TXT` beside the executable and to the attached debugger. The file is created fresh the first time a run writes to it and appended for the rest of that run; each line reopens it, seeks to the end, writes and closes again, so the text on disk is current after every line rather than at shutdown.

The console option adds a third destination, a separate window titled `Debug Console`, opened on the first line that reaches it. A quieter reporting path exists alongside the main one and reaches the log and the debugger but never that window; it is what the main menu uses to echo characters as they are typed.

## Assertions

Assertions are live wherever `NDEBUG` is undefined, which is the Debug configuration. A failed assertion does five things in order: it switches the monochrome display on and prints the failure there, writes the same line to the debug log, writes a timestamped copy to `ASSERT.TXT`, raises a message box, and then ends the program. There is no continue path.

:::caution[Each assertion report overwrites the front of the last one]
`ASSERT.TXT` is opened for writing at its start rather than appended, and the new record is written from the beginning of the file. A shorter report therefore leaves the tail of the previous one in place behind it, and the file never accumulates a history.
:::

## The UI test document

A Debug build with the debug keys armed shows an RmlUi test document over the game and its menus on [F9](/commands/fixed-debug-ui-test-document/) and hides it again on the next press. The document is a panel in the top left corner of the frame with a button that closes it. It is drawn by the renderer over the presented frame, so it appears over the main menu as well as in play, and it follows the frame's position and scale in the window. A click on the panel never reaches the game; a click beside it does, and a visible menu dialog takes the clicks over its own area first. The document, its style sheet, and the font come from the `ui` directory beside the executable, and each load, show, and hide writes a line to the debug log with the renderer's texture and buffer counts, so a leak across repeated toggles shows there.

## The benchmark overlay

A Debug build with the debug keys armed shows a frame benchmark window on [F6](/commands/fixed-debug-benchmark-overlay/) and hides it on the next press or through the window's own close button. The window is drawn by Dear ImGui over the presented frame, like the test document, and follows the frame's position and scale. It reports the logic frames and the presents of the last second, the frame number, the present interval, and the frame benchmarks the Events page shows, as a share of the frame and an average in microseconds when the processor speed could be measured, in ticks otherwise. The five counters the engine never starts are marked as such.

While the monochrome display is off, the window resets the benchmarks once a second and shows the second just gone; while that display is on, the Events page keeps its reset and the window shows the live running averages, so the two never take samples from each other. A button resets on demand. The window takes the mouse only while the pointer is over it and the keyboard only while one of its fields has focus; everything beside it reaches the game. A visible menu dialog takes the mouse over its own area before the shell sees it, so over a dialog the window answers the pointer only where it covers the frame beside the dialog. A switch opens the Dear ImGui demo window, which exercises the renderer.

## The monochrome pages

The monochrome display is a four-page text surface driven through a monochrome display device rather than drawn on screen. Enabling it only raises a flag; nothing verifies that the device is there. The first screen clear the device refuses lowers the flag again, so on a machine without that device the pages switch themselves off on the first diagnostic pass and stay off until something enables them again.

While it is enabled the pages refresh once a second, and the page in view is the one that draws — with the single exception the table names.

| Page | What it draws |
| --- | --- |
| Object | A dump of the last selected object, cleared and redrawn each pass |
| House | A dump of that object's owning house, drawn only where that object is an infantry, a vehicle, an aircraft or a structure |
| Stress | The logic layer's own dump, which is written every pass whether or not the page is in view |
| Events | The frame benchmarks, as a percentage of frame time and an average duration for each tracked process |

The stress page has no body of its own beyond the logic dump, and the benchmark figures are reset after every pass, so the events page reports the second just gone rather than a running total. The rules and scenario timings are the two the reset skips.

## Motion capture

A Debug build binds a key that raises the motion-capture flag. The routine that flag gates would grab the client area into an off-screen surface once per frame, hold one surface per captured frame for [`MovieTime`](/keys/movietime/) minutes' worth of frames, and write the whole sequence out as numbered `cap0000.pcx` files before switching itself off.

Nothing calls that routine in either configuration, so the key raises a flag that reaches nothing, no frame is ever captured, and no value of `MovieTime` changes anything.

## Multiplayer statistics

An Internet session writes `mpstats.txt` when its game loop finishes, in either configuration. The file reports the frame count, the average frame rate, the largest look-ahead the session reached, the latency and game-speed settings, and each local address, followed by a block per connected player carrying that player's address, maximum and maximum-average round trip, resend count, frame-sync and command-count stalls, and both the absolute and percentage packet loss.

## The sync dump

A desynchronized network game writes an [out-of-sync report](/using/out-of-sync-reports/) into the `Debug` folder beside the executable, and so does the [`PrintCRC`](/keys/printcrc/) playback trap on reaching its frame. Both are the same report; it carries the build, the session identity and seed, the recent frame-checksum ring, the offending event, bounded histories of the random draws, targeting, missions, facings, animations and events leading up to the divergence, and per-object and per-heap checksums keyed to each object's stable identifier.

Seven `sun.ini` settings exist to diagnose a desynchronized game, and only [`PrintCRC`](/keys/printcrc/) acts. The other six are read from the file and reach nothing further. [`Frame`](/keys/frame/), [`Type`](/keys/type/#scope-multiplayer-settings), [`Coord`](/keys/coord/), [`Target`](/keys/target/) and [`Cell`](/keys/cell/) describe an object for a per-frame hunt whose body is compiled into neither configuration, and [`CheckHeap`](/keys/checkheap/) raises a flag no reader consults.

`CheckHeap` sits in `[MultiPlayer]`; the rest sit in `[SyncBug]`, which is read only while recording playback is armed. Both blocks are read as a session outside a campaign is set up, so a campaign game reads none of the seven.

## Crash reporting

An unhandled exception writes a folder of its own under `Exceptions`, beside the executable and named for the time of the crash. The folder holds a minidump, a readable `except.txt` report, and a copy of the end of that run's debug log, so reporting a crash means attaching one folder. The report names the fault and the address involved, identifies the crash site by function, file and line, and carries two independently derived call stacks, the registers, the loaded modules and a scan of the stack. Addresses are resolved against the symbol file shipped beside the executable rather than against whatever directory the game was launched from.

The handler is installed as the process-wide unhandled-exception filter before the window, sound and renderer exist, so a crash during startup and a crash on any thread are both reported. A debugger attached to the process sees the exception first and the handler never runs, which is why no option is offered to stand it down. Crash folders older than thirty days are removed at startup, and the copied portion of the log is capped at 256 KiB.

## The Release configuration's own surface

### The main-menu code recognizer

The classic main menu accumulates alphanumeric keystrokes into a buffer of up to 31 characters and flips a flag the moment a recognized code appears anywhere inside what has been typed. Any non-alphanumeric character clears the buffer, and so does a successful match. Each code is a toggle, so typing it a second time flips the flag back.

| Code | What it flips |
| --- | --- |
| `PENGO` | The visceroid art replacement |
| `THETEAM` | A skirmish-only rules overlay read from `TMCJ4F.INI` |

Neither is marked as surviving into multiplayer, so starting a network game clears both. Both exist only as codes; no setting reaches them.

### The version dialog

The version dialog reports the title, the game and internal version names, a build line labeled by configuration and naming the commit the build was made from, the branch it sat on and that commit's date, a processor line, and the version of the language resource library. It opens from the classic main menu's [Ctrl+V](/commands/fixed-main-menu-version/) and from the menu entry, and is drawn as an RmlUi document from the `ui` directory unless [`LegacyDialogs`](/keys/legacydialogs/) is `yes`, which keeps the Win32 dialog; both show the same lines. OK, Enter and Escape close the document. When its document, style sheet or font fails to load, the game logs the file name and opens the Win32 dialog instead. [UI files](/systems/ui-files/) covers the directory.

## Toggles that reach nothing

Several diagnostic switches survive as flags that no reader consults, so invoking them changes only the flag.

- The frame-rate toggle raises a flag with no reader anywhere in the engine; no counter is drawn.
- The cell-icon overlay toggle raises a flag with no reader and forces a full redraw, which is the only visible consequence.
- The passability flag has neither a writer nor a reader.
- The map-checking launch option reaches its per-frame check, but the routine that check calls has no body and always reports success, so no map is examined.
- [The debug special dialog](/commands/fixed-debug-special-dialog/) asks for a dialog that is not compiled, and outside a campaign or skirmish game the request is never cleared.

The scenario editor is unreachable for the same reason: the routine that switches editor state on has no caller anywhere. The only other thing that raises the state is the random map generator, which restores it afterward, so the editor's own drawing, input and placement paths never run for a scenario. A map name on the command line therefore skips the main menu and starts that map as a campaign scenario with no campaign attached, rather than opening an editor over it.
