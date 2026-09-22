---
title: Developer mode and diagnostics
summary: The diagnostic keys, displays, reports and codes each build configuration has, and how a Debug build arms its debug keys.
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

The debug keys, the monochrome pages and assertions exist only in a Debug build. Both configurations write the debug log, crash reports, out-of-sync reports and the multiplayer statistics file, and both have the main-menu codes and the version dialog. [The command reference](/commands/) records which build has each command, control and launch option.

## Arming the debug keys

The debug keys are the fixed in-game keys that only a Debug build has, such as the ones that grant money, force a win or spawn an ORCA. [The fixed controls](/commands/fixed-controls/) list them.

A Debug build arms its debug keys when it is started with at least one command-line argument. A windowed-mode option, a resolution or a data directory arms them, and so does a stray word the game ignores. The [playtest option](/using/command-line/playtest/) arms the same keys and adds nothing else. The Visual Studio project starts the game under the debugger with `-DATADIR=`, so a Debug build launched that way has its debug keys armed.

The [help option](/using/command-line/help/) and an `-X` option with a letter the game does not recognize end the run before the game starts, so they arm nothing.

:::caution[A Debug build started with no arguments has no debug keys]
With no command-line argument, none of the debug keys respond, including the ones that grant money and force a win. No in-game control arms them later.
:::

A debug key acts only while that key is not bound to a keyboard command. A key bound to a command runs the command instead.

:::caution[The debug special dialog key hangs every game except a campaign]
The [debug special dialog key](/commands/fixed-debug-special-dialog/) opens nothing. In a campaign it has no effect. In any other game it pauses the game and never resumes it, unless the local player's game is already ending when the key is pressed.
:::

## The debug log

Both configurations write [the debug log](/using/debug-logging/) and send every line to an attached debugger.

A Debug build also opens a console window titled `Debug Console` that shows the same lines as they are written. A Release build opens it only when started with [`-XC`](/using/command-line/console-debug).

## Assertions

Assertions are compiled only into a Debug build. A failed assertion shows the C runtime's assertion dialog, which offers three choices:

- `Abort` ends the run, and the [crash reporter](/using/crash-reports/) records it as an aborted run.
- `Retry` breaks into an attached debugger. Without one, the game closes and no crash report is written.
- `Ignore` continues past the failed assertion.

## The monochrome pages

The monochrome pages are four pages of diagnostic text for a secondary monochrome display. They need a monochrome display driver, the `\\.\MONO` device. Without that device the pages can still be enabled, but nothing is drawn. If the device is present but refuses to clear a page, the pages switch off.

The [monochrome launch option](/using/command-line/monochrome/) enables the pages. The [monochrome debug key](/commands/fixed-debug-monochrome/) switches them on and off. Separate debug keys step to the [previous](/commands/fixed-debug-previous-page/) and [next](/commands/fixed-debug-next-page/) page.

While the pages are enabled, the page in view is redrawn once a second. The other pages are not drawn, except the stress page, which is updated every second even when another page is in view.

| Page | What it draws |
| --- | --- |
| Object | The diagnostic dump of the selected object. With nothing selected, it shows the last object selected, for as long as that object exists. |
| House | The diagnostic dump of the house that owns the selected or last selected object, when that object is infantry, a vehicle, an aircraft or a structure |
| Stress | The mission timer, the frame rate, and counts of vehicles, infantry, aircraft, structures, terrain objects, projectiles, animations, teams, triggers, trigger types and factories. Scrolling columns log CPU use, cell redraws, route searches, target scans and sidebar redraws once a second. |
| Events | Timings for the steps of a frame: each step's share of frame time and its average duration, plus the rules and scenario timings |

Each time the events page is drawn, it resets every timing except the rules and scenario timings. While the page stays in view, each refresh therefore covers the second just gone. The first refresh after switching to it covers the whole time since it was last drawn.

## Motion capture

The [motion-capture debug key](/commands/fixed-debug-motion-capture/) raises a flag that nothing acts on, so no frame is ever captured. [`MovieTime`](/keys/movietime/) would have set the length of the capture, so no value of it changes anything.

## Multiplayer statistics

A multiplayer game started through the [client launch](/using/command-line/spawn/) writes `mpstats.txt` when it ends, in either configuration. LAN, skirmish and campaign games do not write the file.

The file opens with the frame count, the average frame rate, the largest look-ahead the game reached, the latency and game-speed settings, and each local address. A block for each connected player follows. It gives the player's name and address, maximum and maximum-average round trip, resend count, frame-sync and command-count stalls, and packet loss as a count and as a percentage.

## The sync dump

A network game that goes out of sync writes an [out-of-sync report](/using/out-of-sync-reports/) into the `Debug` folder beside the executable, in either configuration. The [`PrintCRC`](/keys/printcrc/) playback trap writes the same report at a chosen frame and then exits. The report page describes what the report holds.

Seven `sun.ini` settings exist for tracking down a desynchronized game, and only `PrintCRC` has an effect:

- [`Frame`](/keys/frame/), [`Type`](/keys/type/#scope-multiplayer-settings), [`Coord`](/keys/coord/), [`Target`](/keys/target/) and [`Cell`](/keys/cell/) describe an object to watch frame by frame. The code that would watch it is compiled into neither configuration.
- [`CheckHeap`](/keys/checkheap/) raises a flag that nothing reads.

`CheckHeap` belongs in `[MultiPlayer]` and the other six in `[SyncBug]`. Both sections are read only when a skirmish or network game is chosen from the main menu, and `[SyncBug]` only while recording playback is armed. A campaign reads none of the seven. [`PrintCRC`](/keys/printcrc/) describes the one launch in which its value takes effect.

## Crash reporting

Both configurations report crashes. A crash writes a folder under `Exceptions` beside the executable, holding a minidump, a readable report and the end of that run's debug log. [Crash reports](/using/crash-reports/) describes the folder and what the report holds.

While a debugger is attached, the debugger receives the crash instead, and no crash folder is written.

## Main-menu tools

### The main-menu code recognizer

The classic main menu, the plain menu shown when the graphical main menu cannot be built, recognizes two typed codes.

| Code | What it switches |
| --- | --- |
| `PENGO` | Replaces the visceroid art. While it is on, the version number on the title screen ends in `PG`. |
| `THETEAM` | In a skirmish game, reads `TMCJ4F.INI` over the rules when the game starts |

Typing a code switches its effect on, and typing it again switches it off. Each switch plays the [`OptionsChanged`](/keys/optionschanged/) sound.

The menu matches a code anywhere in the typed letters and digits, so `XPENGO` also switches `PENGO`. Any other key clears what has been typed, and so does a match. After 31 characters without a match, the typed text starts over. Each letter and digit typed on this menu is also written to the debug log.

Starting a LAN game switches both codes off. A skirmish game keeps them. No setting reads or saves either code, so both are off each time OpenTS is launched.

### The version dialog

The [version dialog](/commands/fixed-main-menu-version/) shows the title, the game and internal version names, and a build line. The build line says whether the build is Debug or Release, and names the commit the build was made from, the branch it was on and that commit's date. The dialog also shows the CPU vendor and the version of the language resource library.

## Toggles that reach nothing

These Debug controls change nothing beyond what each entry says:

- The [debug icon overlay key](/commands/fixed-debug-icon-overlay/) forces the whole screen to redraw. The overlay it switches is never drawn.
- The [map-checking launch option](/using/command-line/check-map/) runs a check every frame, but the check examines nothing and always passes.
