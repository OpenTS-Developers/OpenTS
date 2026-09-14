# UI system design

Status: under implementation. Steps 1 through 7 of the
[migration plan](#migration-plan) have landed: the dependencies, the RmlUi
shell, the Dear ImGui overlays, the version dialog, the message boxes, the
sound options, the progress and wait boxes, and the options family (game
controls, display, mode confirmation, keyboard, options menu). Steps 8
onward are not yet implemented, built, or measured; source inspection and
upstream documentation inform them. This page owns the UI architecture and
migration; [Building OpenTS](BUILDING.md) owns build support and
[Project direction](DIRECTION.md) the wider architecture.

## Where the UI stands today

OpenTS has four UI systems plus a few bespoke screens. They share the software
frame and the keyboard queue but nothing else.

| System | Files | Used by | Draws into |
| --- | --- | --- | --- |
| OwnerDraw | `ownrdraw.cpp` (7,009 lines), `windlg.cpp`, `msgloop.cpp`, 53 templates in `language.rc` | main menu, options, skirmish, load and save, lobbies, desync, map generator, WDT, message boxes, progress wait | `AlternateSurface`, then `VisibleSurface` |
| GadgetClass | `gadget.cpp`, `control.cpp`, `toggle.cpp`, `list.cpp`, `edit.cpp`, `slider.cpp`, ... | sidebar, radar, tactical buttons, message list, checklist, mission restate | `LogicalSurface` (`SidebarSurface`, `HiddenSurface`) |
| MSEngine | `msengine.cpp`, `msanim.cpp`, `grphmenu.cpp` | graphic menu, map select, score screens, WDT screens, credits | `AlternateSurface`, `HiddenSurface` |
| Bespoke | `progress.cpp`, `score.cpp`, `movies.cpp` | loading screen, score, movies | `HiddenSurface` |

OwnerDraw is the largest and the least portable. Each dialog is a real Win32
child window of `MainWindow`, created from a resource template by
`CreateDialogIndirectParam`. Every control is subclassed; its window procedure
paints into `AlternateSurface` and blits the result into `VisibleSurface`
itself. `Draw_Dialog_Back` composes `dbak6440.pcx`, the side bars, and sixteen
glow passes into a cached surface and assumes 640x400 art centered on the
screen. Text is GDI "MS Sans Serif" at 14 and 12 pixels through `WS_Get_Font`,
plus the `dlgsys` remap sheets for list text. Tooltips save and restore the
pixels under them. `Heal_Dialog_Controls` forces every child window to repaint
after each `Update_Visible_Surface`, so a dialog repaints once per game frame.
The templates hold 322 `CONTROL` entries: 103 owner-draw buttons, 40 track
bars, 26 combo boxes, 26 list boxes, 23 edit boxes, and 45 check boxes. Two
entry APIs share one dialog stack, `g_Dialogs` in `windlg.cpp`:
`OwnerDraw::Begin_Dialog` and the `WS_` family the network lobbies use.

The frame path is simple and already hardware-presented. The game draws 16-bit
pixels into system-memory surfaces. `Video_Present` hands `VisibleSurface` to
`Backend_Present`, which uploads it as one texture, draws one quad with the
embedded `vs_ocornut_imgui` program on view `VIEW_PRESENT` (with a
`VIEW_PRESCALE` pass for the pixel-art filter), and calls `bgfx::frame()`.
bgfx runs single-threaded because presents happen from inside dialog paint
handlers; `_Presenting` guards the recursion. Presents are paced to the refresh
interval and happen only when the frame is dirty. The mouse pointer is a
hardware Win32 cursor built from the game's shapes, so it never touches a
surface. The window is per-monitor DPI aware, and `VideoScaleInfo` records
where the logical frame lands in the physical client area.

Input reaches a control by one of two routes. Windows delivers a mouse message
to the visible child window under the cursor, so a legacy dialog receives its
own messages and `MainWindow`'s procedure never sees them. For `MainWindow`,
`Windows_Procedure` first runs `Route_Mouse_Message`, which under video scaling
re-targets a mouse message to the visible child under the scaled cursor, then
`Map.Message_Handler`, then its own switch, then `Keyboard->Message_Handler`,
which packs keys and mouse buttons with their position into the `KN_` queue.
Gadgets poll that queue in `GadgetClass::Input`; `WWKeyboardClass::Down`
reads `GetAsyncKeyState`, so withholding a queued key does not hide a held
key from polling code. `Windows_Message_Handler` runs `IsDialogMessage` for
every tracked dialog before dispatch, which takes Tab, Enter, Escape, and
arrows, and runs `Message_Intercept_Handler` only after that.

Every screen owns its loop. In game, `Main_Loop` runs input, logic, and
render. A dialog driver spins on `OwnerDraw::Dialog_Message_Handler`, which
pumps messages and then runs `Main_Loop` in a network session or `Call_Back`
otherwise, so multiplayer keeps stepping under a dialog. The lobbies use
`WS_Wait_Dialog` with a callback. MSEngine screens spin on `Engine.Wait_Delay`.
`RestateMission` mixes gadgets with MSEngine. A dialog's result is written
through a pointer stored in `DWLP_USER` by its `WM_COMMAND` handler and read by
the driver after the pump returns. `Keyboard->Clear()`, which every driver
calls around a dialog, pumps Windows messages through
`Fill_Buffer_From_System`, so cleanup can re-enter UI code.

Nearly every legacy flow hides or destroys its parent before opening a child:
the main menu hides around the version dialog, the options driver ends the
main dialog before a sub-dialog, in-game options hide around save and load,
skirmish hides around the scenario picker. The lobby keeps its host and
game-list dialogs alive together.

Facts elsewhere in the tree that bind the design:

- `Fetch_String` loads through the ANSI `LoadString` into a 128-slot reused
  cache and returns a pointer into it. String identifiers are `#define`s in
  `language.h`; no name table exists. The engine builds with `_MBCS` and the
  resource script at code page 1252.
- `CDFileClass::Has_Directory` treats `\`, `/`, and `:` as directory marks,
  and such names skip the user-path redirect; a bare name is tried in the
  user path, the current directory, the search paths, and then the mix files.
- `Map.Input`, and with it `SidebarClass::AI`, is polled from `Main_Loop` and
  from the network and timer waits in `queue.cpp`.
- `FactoryClass::Has_Changed` clears `IsDifferent`, which `FactoryClass::Serialize`
  writes. `StripClass::Serialize` writes `IsScrolling`, `Flasher`, `Scroller`,
  `Slid`, and `LastSlid` beside `TopIndex` and `Buildables`.
- `SidebarClass::Reposition_Sidebar` registers the cameo tooltips itself,
  independent of gadget registration; `CCToolTip` paints into game surfaces.
- `ProgressScreenClass::Set_Progress_Percent` sends `WM_PAINT` synchronously,
  and `Display_Progress` plays the milestone sound from the draw path.

Three consequences shape the design. A new UI must fit the blocking-loop
shape, or every driver has to be rewritten in the same change; the loop shape
is fine and the screen bodies are the problem. Anything drawn into the
software frame sits under anything drawn by bgfx, and a visible legacy window
takes the mouse before the shell can see it. Input must be claimed before it
enters the `KN_` queue, and legacy child windows must keep receiving their own
messages until they are gone.

## Goals and limits

Goals:

- Replace OwnerDraw with RmlUi documents, one screen per change, with the
  legacy screen available behind a switch until OwnerDraw is retired.
- Make screens interchangeable at the screen level: a model or presenter that
  knows nothing about the toolkit, and one view per toolkit.
- Keep the subsystem portable by preparation, the approach
  [Project direction](DIRECTION.md) sets for the engine. Screen behavior,
  the screen contract, input ownership, coordinate mapping, and the render
  checks carry no operating system type, so a later port replaces the host
  and the platform edge rather than the screens.
- Leave GadgetClass and MSEngine in place; migrate them later through the same
  screen contract when a screen is worth it. The sidebar follows only after
  the Win32 dialogs are gone, as a player-selectable alternative to the
  gadget sidebar.
- Add Dear ImGui on the same shell for developer tooling, available to a
  player-facing feature if one wants it.
- Keep modding open: documents, styles, and images load through the game's
  file system with mix-file support from the first screen.

Limits, chosen to keep the work bounded:

- No widget-level abstraction across GadgetClass, OwnerDraw, RmlUi, and
  ImGui. Views are whole documents.
- No scripting layer (RmlUi's Lua plugin), no reactive framework, no global
  message bus, no runtime plugin system.
- No RmlUi render effects (filters, layers, shaders, box shadows). The
  renderer implements the eight required methods, transforms, and clip masks;
  the rest stays default until a screen needs it.
- No arbitrary layering of native and GPU UI. The coexistence rule under
  [Input and focus](#input-and-focus) is the whole policy.
- No user UI scale setting yet. Documents follow the frame scale.
- No platform abstraction layer. Windows is the only supported target, so the
  shell's message hook and its host interface are written in Win32 terms. A
  seam designed against one platform would be guesswork; the coupling is kept
  where a port can find it instead, under [Portability](#portability).
- The exception and assertion dialogs stay plain Win32. They must work when
  the renderer is the thing that failed.

## Architecture

Three parts, from the bottom up.

The **UI shell** is the `code/ui/` module: a `UIShellClass` object that owns
the RmlUi context, the injected toolkit interfaces, the bgfx overlay pass,
the input hook, and the modal runner, beside the ImGui context its developer
module holds. Only its `code/ui/rml/` headers include RmlUi, ImGui, or bgfx,
the way `bgfxbackend.cpp` is the only other code that includes bgfx; the
`toolkitheaders` CTest check enforces that.

A **screen** is a presenter plus a view. The presenter is a plain C++ object:
it holds a view-model struct, answers queries, and executes actions. It never
sees an `HWND`, a `Surface`, an `Rml::Element`, or an ImGui call. A view
renders the view-model and turns user actions into intents. The RmlUi view is
a document with a data model; the legacy view is the existing dialog
procedure wrapped so it reads and writes the same presenter; an ImGui view is
possible for a feature that wants it. A screen returns a result the way a
dialog returns `rc` today.

The **toolkits** are RmlUi, preferred for player-facing screens; the existing
systems for screens not yet migrated; and ImGui, primarily for tools.

Dependency rules:

- Presenter headers contain no `HWND`, control IDs, `GadgetClass`, RmlUi,
  ImGui, or bgfx types.
- Views use their toolkit directly. There is no shared widget API.
- RmlUi data bindings and document nodes stay inside the RmlUi view.
- Renderer handles stay inside `code/ui/rml/rmlrender.cpp`.
- Headers outside `code/ui/rml/` include no toolkit header, and sources
  outside `code/ui/` include no RmlUi or ImGui header; the `toolkitheaders`
  CTest check enforces both.
- The shell knows which presentation owns a region and an input scope. It
  does not know production rules, save semantics, or option behavior.
- Existing callers keep their screen functions; composition sits behind
  them.

A read-only screen needs a data builder and a close result. A presenter with
actions is added only where a screen has real state transitions.

### Shell object

`UIShellClass` (`uishell.h`) holds the shell's state: the RmlUi context, the
injected system, file and render interfaces, the re-entry guards, the work
deferred while the context runs, the modal stack and the modeless list. The
engine's one instance is `UIShell`, declared `extern` in `code/_ui.h` and
defined in `code/_ui.cpp` over `UI_Engine_Host()`; callers write
`UIShell.Tick()` or `UIShell.Use_Rml()`. What the shell needs from the program
around it comes through `UIShellHostClass` (`uihost.h`), so a harness builds
its own `UIShellClass` over a host and interfaces it controls and never links
the engine. A modal screen's engine entry calls `UI_Run_Modal(view)` from
`uienginehost.h`, which runs the screen on `UIShell` with `UI_Service_Game`
as the service pass.

### Code layout

Sources live under `code/ui/`, grouped by what they may include. The
recursive glob in `code/CMakeLists.txt` picks them up; the per-file
properties carry only the shader headers, the image decoder header, and the
bgfx debug define, as `bgfxbackend.cpp`'s do today. The library headers reach
the whole target through the linked targets, so the containment below is a
rule the tree follows, not a build boundary.

| Directory | Holds | Status |
| --- | --- | --- |
| `code/` | `bgfxviews.hh`, the view ids the presenter and the overlays share; `_ui.h`, `_ui.cpp`, the shell's one instance `UIShell` under the underscore-file convention for globals | landed |
| `code/ui/` | the shell and the toolkit-free contracts: `uishell.h`, `uishell.cpp` (`UIShellClass`: init and shutdown, resize, input hook, developer-key intercept, tick, overlay render entry, modal runner, selector; its toolkit interfaces are injected, so a test builds its own instance); `uihost.h` (`UIShellHostClass`, what the shell needs from the program around it: the window, frame, keyboard queue, dialogs, strings and log); `uienginehost.h`, `uienginehost.cpp` (the engine's host and the game-service pass a modal runs with; the only shell file that includes engine headers); `uiscreen.h`, `uiscreen.cpp` (presenter, intent, result, clock); `uiview.h` (`UIViewClass`, the view the shell runs); `uiinput.hh`, `uiinput.h`, `uiinput.cpp` (who owns each held key and button, and the UTF-8 decoding of a narrow window's text); `uiunicode.h`, `uiunicode.cpp` (strict UTF-8 and UTF-16 conversion for the clipboard); `uicoord.h` (the pointer mapping from client pixels into the overlay); `uireveal.h`, `uireveal.cpp` (the schedule a dialog opens on; toolkit-free, so the harness runs it) | landed |
| `code/ui/rml/` | the RmlUi adapters, the only headers that include a toolkit: `rmlsystem` (system interface: time, logging through the host, string translation, the pointer request, the clipboard), `rmlfile` (file interface over `CCFileClass`), `rmlrender` (render interface and the ImGui renderer on bgfx; with `bgfxbackend.cpp` the only files that include bgfx), `rmltexture` (image files: PCX, PNG and TGA today, with SHP and engine surfaces described under [Assets](#assets-and-strings)), `rmlimage` (turning PCX bytes into indices and RGBA), `rmlfontsheet` (measuring and coloring the dialog art's bitmap font; both toolkit-free, so the harness runs them), `rmlfont` (the font engine over those sheets, forwarding every other family to the engine RmlUi made), `rmlkeys` (virtual keys, `KeyIdentifier`, `KEYBOARD.INI` numbers), `rmlview` (`UIRmlViewClass`, the RmlUi view base), `rmlrendermath` (the checks the renderer makes before it draws: index ranges, byte counts, scissors; toolkit-free, so the harness runs them) | landed |
| `code/ui/dev/` | `uidev.h`, `uidev.cpp`: the ImGui context, its input feed, and the developer overlays | landed with the frame benchmark window |
| `code/ui/screens/<name>/` | one family each for `version`, `msgbox`, `waitbox`, `sound`, `gamectrl`, `display`, `keyboard`, `mainopt`: `ui<name>.h` (presenter, service and state declarations, view factory, engine entry), `ui<name>.cpp` (presenter and RmlUi view; built into the test), `ui<name>dlg.cpp` (engine service and entry, which the test cannot link) | landed; the sound, game controls, keyboard and display Win32 dialogs drive the same presenter as a second view, and the wait box family carries the `UIWaitBoxClass` the save, load and progress code shows |
| `tests/uishell/`, `tests/uilogic/` | the two harnesses under [Validation](#validation-and-evidence); `cmake/CheckToolkitHeaders.cmake` is the containment check they run beside | landed |

Shipped UI files (documents, styles, images, the font) live in `ui/` at the
repository root. The build places the tree beside the executable, at
`<build directory>/bin/<configuration>/ui/`, and the client package ships it.

## Rendering

RmlUi and ImGui render as GPU overlays on top of the presented frame, at the
physical resolution of the window. `Backend_Present` splits in two:
`Backend_Present` submits the frame quad as now but no longer calls
`bgfx::frame()`; a new `Backend_End_Frame` does. `video.cpp` calls the shell's
render between them:

```cpp
Backend_Present(pixels, ...);   // VIEW_PRESCALE, VIEW_PRESENT
UI_Render_Overlay();            // VIEW_UI, then VIEW_DEV
Backend_End_Frame();            // bgfx::frame()
```

`Backend_End_Frame` runs whether or not `Backend_Present` succeeded; it ends
a frame only when one was begun, so a refused present leaves nothing pending.
No other code begins or ends a bgfx frame. The view identifiers move from
`bgfxbackend.cpp` into a small shared header so both translation units agree
on the order. The overlay views use the frame destination rectangle from
`Video_Get_Scale_Info` as their viewport and an orthographic transform of the
destination size, so UI coordinates are physical pixels relative to the
frame's top-left corner. Draw order is the software frame and its scaling
passes, RmlUi documents in the context's document order, ImGui, then the
hardware cursor.

One RmlUi context holds every document. A second context is justified only by
an independent coordinate space or lifetime. Data-model names are unique
among live screens, binding storage is owned by the view and outlives the
model, and a model is removed before its storage is destroyed.

### Renderer

The render interface is a bgfx implementation of RmlUi's eight required
methods, plus `SetTransform`, `EnableClipMask`, and `RenderToClipMask`:

| Capability | Behavior |
| --- | --- |
| Compiled geometry | Static vertex and index buffers, since RmlUi 6 compiles geometry once and re-submits it; order preserved; released on request; never dependent on transient memory from a previous frame. Indices are checked against the vertex count and sizes are checked before the copy; without 32-bit indices, a fragment over 65536 vertices is refused rather than truncated. |
| Textures | RGBA8, premultiplied alpha as the interface specifies, created and released explicitly, cached by source string. Each edge is at most the smaller of the device limit and 4096, and a source must hold exactly width times height times four bytes. A document's textures wrap rather than clamp, which is what the tiled decorators repeat through, and are sampled with the filter the frame underneath was magnified by. RmlUi keeps a one-pixel gutter around every glyph, so text is unaffected; a magnified image's outer edge blends half a texel of its opposite edge, as RmlUi's own GL3 backend does. Dear ImGui's atlas keeps its clamp. |
| Blending | `ONE, INV_SRC_ALPHA`; vertex colors follow the same premultiplied contract with no double premultiplication. |
| Scissor | `bgfx::setScissor` in physical target coordinates, rounded outward to whole pixels, intersected with the viewport, empty regions handled. |
| Transform | RmlUi's matrix and bgfx's are both four columns with the translation last, so the sixteen floats pass between them untouched; each draw submits the transform applied to the fragment's own translation. RmlUi sends none until a document has one and dedupes identity, so an untransformed document never reaches it. The view projection is unchanged, and its zero-to-thousand depth range clips a three-dimensional transform that pushes a vertex behind the near plane; a two-dimensional one keeps every vertex at zero. |
| Clip mask | The back buffer's own stencil, which bgfx attaches unless a caller asks for a depth-only format. A mask that starts over writes zero through a view-sized shape first, because bgfx clears a view only before its first draw, then writes one under the geometry with nothing reaching the color buffer; a narrowing mask counts up instead, and the draws that follow pass where the stencil equals what the last mask left. An inverted mask writes the same shape and tests against the untouched target. Every test carries a read mask, since bgfx reads none by default and an equality test against nothing passes everywhere. |
| Limits | 64 MiB per geometry or texture, 128 MiB of live geometry and 128 MiB of live textures, two draw calls short of the device's frame limit, and clip masks nested no deeper than the stencil's eight bits count. The first refusal is latched with its reason; the shell clears the latch before preparing a document and reads it after, so a document the renderer could not draw whole opens its Win32 view instead. |
| Projection | The overlay view's orthographic transform; no game-image filter state inherited. |
| Reset and resize | Target-dependent resources recreated, viewport and scissor refreshed, a present without an upload requested; existing documents redraw without reload. |

The program is bgfx's embedded debug-draw texture shader pair
(`vs_debugdraw_fill_texture`, `fs_debugdraw_fill_texture`). The imgui pair the
frame quad uses multiplies by the view projection alone and drops the model
matrix, which is where each compiled fragment's per-draw translation travels;
the debug-draw pair multiplies by the model, view, and projection product. Its
attributes (position, texture coordinate, color) match RmlUi's vertex and
ImGui's vertex, each with its own layout. Layers, filters, and shaders are
deferred, and with them `box-shadow`, which RmlUi renders into a layer and
saves as a texture: with `SaveLayerAsTexture` returning nothing the shadow
draws as an untextured white quad and its generation callback runs again every
frame, which is the one effect that fails visibly rather than quietly. Shipped
documents stay within a declared profile (text, images, ordinary layout,
borders, basic decorators, transforms, and clipping), and a document check
enforces it. A document that reaches a deferred effect anyway, as a mod's may,
draws without it: the renderer latches the refusal with one logged reason and
otherwise behaves as RmlUi's defaults do.

### Invalidation

The presenter keeps a game mark, an overlay mark and whether the renderer
holds an uploaded frame (`VideoDirtyStateClass`, `code/videodirty.h`). A
present happens when either mark is set; the frame is uploaded only when the
game mark is set or the renderer has never received it. RmlUi has no "needs
redraw" query, so the shell marks the overlay dirty on every tick that a
document is visible or an ImGui window is open, and the present pacing caps
the rate. Closing or hiding a document also marks the overlay dirty so its
pixels disappear. A visible menu at 4K then costs a few draw calls per
refresh, not a 16 MB upload.

A present consumes both marks first, so invalidation raised while it runs is
kept for the next one rather than cleared with the current frame. A present
the renderer refuses restores what it consumed and is retried as at least an
overlay present. A minimized window presents nothing and keeps its marks for
the restore. A resize arriving inside a present is applied after it. A window
resize or refresh-rate change marks only the overlay, since the renderer
keeps the uploaded frame across a reset; `Video_Present_Count` and
`Video_Frame_Upload_Count` on the developer overlay show a drag-resize
presenting without uploading.

Movies keep their own presenter path; the shell renders nothing while a movie
plays.

## Coordinates

Three spaces exist and the shell owns every conversion between them:

| Space | Purpose |
| --- | --- |
| Native client pixels | Window messages and the drawable size. |
| Game logical coordinates | Existing surfaces, tactical input, legacy geometry. |
| UI coordinates | RmlUi and ImGui layout inside the overlay viewport. |

| Quantity | Value |
| --- | --- |
| Context dimensions | `DestWidth` by `DestHeight` from `VideoScaleInfo`, physical pixels. |
| Document origin | `DestX`, `DestY` in the client area. |
| Density-independent pixel ratio | `min(ScaleX, ScaleY)`; one authored `dp` is one game logical unit. |
| Pointer input to the overlay | Client pixels minus the destination origin; never divided by the ratio. |
| Pointer input to the game | `((x - DestX) / ScaleX, (y - DestY) / ScaleY)`, only for consumers that are eligible. |
| Wheel position | Arrives in screen space; converted to client space once. |

A document authored at a legacy dialog's logical size therefore appears at
the same on-screen size while text is rasterized at physical resolution. The
presenter fits uniformly and truncates the destination extents, so `ScaleX`
and `ScaleY` can differ by less than a pixel across the frame; the uniform
ratio serves everything inside a document. A document whose edge must meet a
software-drawn edge, such as the future sidebar meeting the tactical
viewport, gets its outer bounds from the exact mapping, both edges rounded as
the presenter rounds, and receives them as physical pixels; only its interior
is authored in `dp`. Letterbox space outside the viewport is inactive for UI
and never becomes an edge click through clamping; a captured release is still
delivered there. `Video_Set_Mode` and `Video_On_Resize` notify the shell so
the context, mapping, clipping, and cursor scale change together.

## Input and focus

### Coexistence rule

An RmlUi or ImGui document may be shown only while every legacy dialog is
hidden or destroyed. A legacy dialog may be shown only while no overlay
document is visible. Both halves follow from the survey: legacy pixels are
under the overlay, and a visible legacy window takes the mouse before the
shell sees it. `Windows_Message_Handler` skips hidden dialogs in its
`IsDialogMessage` loop so a hidden parent cannot take Tab, Enter, or Escape
from an overlay child. Debug assertions in `OwnerDraw::Begin_Dialog`,
`OwnerDraw::Display_Dialog`, `WS_Create_Dialog`, and the shell's show path
enforce the rule. The legacy flows already satisfy it except the lobby, which
migrates as one family.

### Hook and priority

The shell gets a hook in `Windows_Procedure` after `Route_Mouse_Message` and
before `Map.Message_Handler`. The router rewrites a position into the frame's
own pixels, so the hook receives the position as Windows delivered it:

```cpp
if (UI_Handle_Window_Message(hwnd, message, wParam, client_lparam)) {
    return(0);
}
```

Placing it after the routing keeps legacy child windows working under video
scaling; placing it before the keyboard handler keeps consumed input out of
the `KN_` queue. The hook covers mouse, wheel, key, and text messages, and
watches capture and activation changes to end the presses it owns without
consuming them. Size, paint, transport, and system messages continue on their
paths. Forwarded or re-targeted messages are delivered to a toolkit once. A
developer key is intercepted earlier still, in `Windows_Message_Handler`
ahead of the dialog loop, so it works whichever window has focus; it only
records a request that the next tick executes.

Priority follows scope and capture, not toolkit:

1. Application lifetime handling: activation, shutdown.
2. The active exclusive modal, legacy or overlay.
3. An ImGui window that owns focus or capture, per ImGui's capture flags.
4. A HUD document for its region, focused field, or capture.
5. Gameplay input that remains eligible.

The rules the hook applies, in order:

1. If ImGui wants the mouse or keyboard, ImGui takes the message. ImGui is
   fed input first and its capture flags decide suppression; capture is not
   a filter on delivery.
2. If a modal document is shown, RmlUi takes every mouse and key message.
   This mirrors `IgnoreInput` around a legacy dialog and composes with the
   scenario's own input locks rather than replacing them.
3. Otherwise mouse moves are always delivered and never consumed, so the
   game keeps tracking the cursor. A button press is owned by whoever takes
   it: ImGui, RmlUi when it reports the mouse interacting with an element
   (its mouse functions return `false` for that), or the game. A key press
   is owned the same way, by RmlUi when an element stopped its propagation
   or the focused element is a text field; a wheel message is consumed when
   RmlUi consumed it, and text when RmlUi consumed it. Documents that float
   over the game mark their body `pointer-events: none` so empty space
   passes through.
4. The owner of a press owns its release, wherever the release lands. A
   press a toolkit owns sets mouse capture on `MainWindow` until the last
   such button is up. A screen closing, another window taking the capture,
   or the window losing focus suppresses what is held: the toolkits are told
   their presses ended and the releases are swallowed rather than handed to
   the game as the end of a press it never saw. What is physically held as a
   modal screen opens, or as focus returns while something is shown, is
   suppressed the same way. A suppressed key or button is forgotten once the
   system reports it up, so a release that went to another window cannot
   keep the shell active. The five mouse buttons and both wheel axes are
   routed; the modifier keys are read from the keyboard state and never
   owned.

Gameplay code that polls `Down` still sees held keys; eligibility is applied
at the consumers, `GScreenClass::Input` and the gadget and scroll paths, not
by falsifying physical state.

### Focus, cursor, clipboard, text

The shell clears the keyboard queue when a modal document opens and again
after it is released, so the pump inside `Keyboard->Clear()` meets either
the shown screen or the ownership table, never a screen mid-teardown. Focus
loss cancels capture, drags, and composition; focus return does not replay
held keys as presses. While the pointer is the documents', because a screen
is shown, a document holds a press, or the pointer is over an element that
takes it, the shell answers `WM_SETCURSOR`: a document's request (`text`,
`pointer`, `move`, `not-allowed`) shows the matching system pointer, and
otherwise the window's arrow, which is what the Win32 dialogs show. The
game's own shape stays captured under a screen and is blank in the
frontend, so the answer is never left to it. The game's pointer returns
when the pointer is no longer the documents'. The clipboard interface
exchanges Unicode text with the Win32 clipboard and refuses malformed text
rather than repairing it.

Text arrives as `WM_CHAR`. The main window is a narrow window, so under the
UTF-8 code page each message carries one byte and the shell decodes the
sequence, replacing a malformed one with U+FFFD; under another code page it
joins a lead byte with its trail byte. A Unicode window would deliver UTF-16
units, which the shell pairs, and a lone surrogate becomes U+FFFD. Consuming
a physical key never suppresses the text message it generates. Editable
screens ship only after Tab and Shift+Tab, Enter and Escape, repeat,
modifiers, paste, dead keys, and IME composition have been exercised for the
supported languages; the read-only pilot proves none of that.

## Screens

The screen contract is two small classes. The presenter is toolkit-free; a
view binds it:

```cpp
class UIPresenterClass {                          // uiscreen.h: no toolkit types
    public:
        void Queue(UIIntent const & intent);              // from any view's events
        void Drain(void);                                 // owner's safe point: Execute each, in order
        virtual void Execute(UIIntent const & intent) = 0;
        virtual void Refresh(void) = 0;                   // engine state into the view-model
        std::optional<UIResult> Result;
};

class UIViewClass {                               // uiview.h: no toolkit types
    public:
        virtual bool Prepare(UIShellClass & shell) = 0;   // load; false falls back
        virtual void Show(bool modal) = 0;
        virtual void Hide(void) = 0;
        virtual void Release(void) = 0;
        virtual void Sync(void) = 0;                      // presenter changes into the view
        virtual UIPresenterClass & Presenter(void) const = 0;
};

class UIRmlViewClass : public UIViewClass {       // rml/rmlview.h: owns the document
    public:
        UIRmlViewClass(UIPresenterClass & presenter, char const * document);
        virtual void Bind(Rml::DataModelConstructor & model) = 0;   // view-model fields and events
        virtual void Sync(void) override = 0;                       // dirty what Execute changed
};
```

A screen's factory, `UI_<Name>_View(presenter)`, returns a
`std::unique_ptr<UIViewClass>`, so the engine entry that builds the presenter
and runs the view includes no RmlUi header. The shell runs any `UIViewClass`;
the RmlUi view is the only implementation today, and the Win32 dialogs that
drive a presenter do so from their dialog procedures rather than as views.

The view-model is a struct of plain values and vectors that RmlUi's data
binding renders; the document uses `data-model`, `data-value`, `data-for`,
and `data-event-click="queue('ok')"`. Intents are small tagged values holding
identities and copied data, never DOM pointers, borrowed buffers, `HWND`s, or
unprotected engine pointers. The presenter copies what it needs out of
`Options`, `Session`, or the scenario into the view-model and writes back on
accept, which is what the dialog procedures do today with `TempOptions`. A
query never clears an engine dirty flag, advances a timer, consumes a factory
notification, or emits an event; where an existing getter has such an effect,
it stays on the behavior path and a separate query is added.

Executing an intent:

1. Verify the screen and the scenario or session are still alive and the
   originating scope is still eligible; a screen carries a lifetime token and
   a scenario generation for this.
2. Resolve the supplied identity against current state.
3. Apply the feature's existing validation, feedback, and rejection at their
   existing boundaries; styling adds no new restriction.
4. Use the current service or command path with its ordering and effects.
5. Refresh the view-model or produce a result.

Intents are executed in order. When a scope is suspended or loses
eligibility, its undrained intents are discarded, not replayed; accepted
effects are not undone. Production clicks and commands are never coalesced.

Each screen defines its result and, where relevant, distinguishes accepted,
cancelled, session ended, and failed to open. A wrapper maps these onto the
existing return values, including `IDOK` and `IDCANCEL`. Settings that apply
immediately do not gain an apply-and-cancel transaction.

The legacy view for a migrated screen is the existing driver behind a
selector:

```cpp
int WWMessageBox::Process(...) {
    if (UI_Use_Rml()) return(UI_Message_Box(...));
    // existing OwnerDraw path, deleted with OwnerDraw
}
```

Selection is latched at screen entry or at scenario load, never mid-gesture.
Preparation (documents, bindings, resources, host scope) completes before a
view becomes interactive; a preparation failure reports the resource and
opens the legacy view where one exists. A screen asked to open while a Win32
dialog is visible opens its legacy view too, so the coexistence rule holds
until that dialog migrates. After activation, a view failure recreates
presentation against the surviving presenter state and never replays
accepted intents.

## Scheduling

Three kinds of work keep their owners: game behavior and command production
run at their existing call points with their existing gates; toolkit input,
layout, and animation run at the shell's service points on the application
thread; GPU submission runs in the presenter. UI animation uses wall-clock
time and never reads or advances deterministic game timers.

A migrated dialog driver keeps its shape. `Run_Modal` is the RmlUi twin of
the `Dialog_Message_Handler` loop:

```cpp
UIResult UIShellClass::Run_Modal(UIViewClass & view, UIServiceCallback const & service);
// each pass:
//   service();   -- the engine passes UI_Service_Game: Windows_Message_Handler(),
//                   then Main_Loop() in a network session, else Call_Back();
//                   a test passes whatever it wants pumped
//   refresh the presenter, execute the screen's queued intents, sync the model;
//   context->Update();
//   mark the overlay dirty, Video_Present_If_Dirty();
// until the screen has a result or the service reports the game ended.
```

The service pass is injected rather than written into the runner, so the
shell includes no game-loop header and the harness drives a modal without
the engine; `UI_Run_Modal(view)` in `uienginehost.h` is the engine's entry
and binds `UI_Service_Game`.

The result carries the game-ended flag the way `Dialog_Message_Handler`
returns `true`, so callers keep their logic. Wrappers keep their service
paths: the main menu keeps title-screen maintenance, an in-game screen keeps
the guarded multiplayer pump, lobby and loading flows keep their own work.

Event handlers never act directly. A toolkit event queues an intent, and the
runner executes the queue before the `Context::Update` that pushes the model
into the documents, so the push carries what the player just changed. A push
that lagged behind the queue would write the old level onto a slider, whose
own change event would then queue that level after the player's. RmlUi gives
no guarantee about re-entering `Update` from its own event dispatch, so a
nested modal (options opening a message box) starts from the queue, one level
up, where `Run_Modal` nests cleanly; and the legacy code already works this
way, `WM_COMMAND` writing `rc` for the driver to act on after the pump. A
modal document is shown with RmlUi's modal flag, which keeps other documents
from taking focus; blocking the game's input is the shell's job through the
hook.
Paint handlers and the pump never drain intents, advance game logic, or
update the context; a nested update or present request is recorded and
served at the next safe point.

Non-modal documents are updated by a `UI_Tick` call in `Main_Loop` next to
`Map.Input`, and by one at the end of each pass of the legacy dialog driver
so that a document stays alive under a menu, and are rendered by every
present. A notice a caller shows while it works goes through
`Show_Modeless`, `Refresh` and `Hide_Modeless`, which tick and present at
once because such a caller pumps nothing. That present ignores the interval
between frames: the caller gets no second chance, so a notice raised soon
after the last frame would otherwise never be drawn at all.

Teardown order: mark the screen closing and invalidate its token, then drop
focus and capture and discard its intents, then detach listeners and data
models and release documents while their storage lives, then remove the
shell registration, and only then clear the keyboard queue or return focus.
Focus returns only to a still-valid owner and never foregrounds the game
while another application is active. A session may end during a multiplayer
modal; closing must not recreate a destroyed HUD or touch a stale scenario
pointer.

## Assets and strings

### Files

The RmlUi file interface is a thin wrapper over `CCFileClass`. Documents,
styles, images, and fonts use flat basenames, and the interface resolves
every relative reference by basename, so the same files load from a loose
`ui/` directory or from a mix. The `ui/` directory beside the executable is
added to the `CDFileClass` search paths as an absolute path, whichever data
directory the deployment names; the existing order then applies: user path,
current directory, search paths, mix files. A mod overrides a document by
placing a file earlier in that order; a copy in a mix is used only when no
loose file exists. The `ui/` directory on disk is a packaging convenience,
not part of the lookup key.
The adapter validates sizes, reads, and seeks; RmlUi uses `size_t` where the
engine uses `int`, and a clamped seek must not look like success. A missing
required document, style, or font fails preparation with the name reported.

### Images

Images resolve by extension. PNG and TGA decode through `stb_image.h`, which
bimg vendors and the texture loader compiles with only those two formats
enabled; `bimg_decode` itself stays out because it would bring the AVIF codecs
and three more decoders along.

PCX decodes through `rmlimage`, which reads an 8-bit run-length file into
palette indices and turns those into premultiplied RGBA. Pure magenta is the
color key the interface art is drawn with, so those pixels come back clear.
The engine's own `Read_PCX_File` is not reused: it allocates a `BSurface`,
converts to display-format pixels, and bounds-checks nothing, while a document
needs RGBA, indices for the bitmap font, and safety against a malformed file.
Keeping the decoding in a file that knows no toolkit or file system is what
lets the harness test it over bytes it builds itself, with no art shipped.

Art the player does not have is told apart from art that is present and will
not decode. A missing image leaves a clear texture and no latched refusal, so
a screen missing a decoration still opens; an unreadable one latches as
before, because that is the document's own fault.

The art is scaled the way the frame is. The pixel art filter magnifies the
frame point for point to the next whole multiple of its size and shrinks that
smoothly to the window, so its pixels stay whole and even at any scale; a
picture drawn by RmlUi at a fractional scale with a linear sampler would blur
instead, and with a point sampler would come out with uneven pixels. So the
host reports that whole multiple as `Art_Magnification`, the renderer keeps
each loaded picture magnified by it while reporting the picture's own size to
RmlUi, and the sheet font magnifies its atlases the same way, so both are
sampled smoothly from whole pixels. The factor is one when the frame is drawn
smoothly or point for point, and a frame that changes it releases every
texture so the art loads again. SHP frames are not decoded
yet; they are for the sidebar view, in a `name.shp#frame` form with an
optional palette and index zero transparent.

Surfaces the engine draws at runtime (the map preview, the
desync host icons, a progress bar) reach a document through a `<surface>`
custom element bound to a named provider; the shell re-uploads the texture
when the provider marks it dirty. Original game art stays local runtime data
outside version control; documents receive artwork identities, never engine
pointers.

Every colour is what the game's 16-bit frame showed of it: the original drew
into five bits of red and blue and six of green, and the frame widens them
again by repeating their top bits, so the loader rounds each picture's palette
the same way, the sheet font rounds its remapped colours, and the kit's own
colours are written as they come out. Blended pixels, the glow and the dims,
can still differ from the original's by a level or two, since the original
blended in sixteen bits and the kit in eight.

### Fonts

A document names one of two families, and never a fallback of its own.

`dlgsys` is the dialog art's own bitmap font: a pair of PCX sheets of 256
cells in Windows-1252 order, one naming a color per pixel and one carrying
coverage, read through `rmlfontsheet`. Its glyphs are shaded rather than flat,
so asking for text in a color moves the whole palette toward that color
instead of tinting a white sheet, and an atlas is baked per color rather than
per draw as the dialog layer rebuilt its table. The sheets have one size, so a
document asking for another gets them magnified; `font-size: 16dp` is that
size, which is how the family scales with the frame. A space, and every code
below it, moves the pen without drawing, as the dialog layer moved over them:
the sheets' space cell is not blank, and drawing it puts a stray line over the
text. The pen starts a pixel left of where the text is placed, as the dialog
layer started it, and the advances are scaled as a sum rather than one by
one, so a run keeps the width the frame's scaling gives it at a scale that is
no whole number. The layer put a right-aligned run a pixel further from the
edge than its width alone would, and halved the room around a centred one in
whole pixels where RmlUi rounds a half up; a sheet gives a right-aligned box
a pixel of right padding, and the kit makes a centred box a pixel narrower,
which brings both to the layer's pixel.

`dlg-sans` is the face the Win32 dialogs asked GDI for. They asked for the
raster "MS Sans Serif"; OpenTS loads its TrueType successor `micross.ttf`,
which ships with the same Windows, scales freely and needs no strike
selection. The shell asks the host where the file is rather than naming a
Windows path itself.

Arimo (OFL 1.1) from Google Fonts ships in `ui/` as `Arimo.ttf` beside its
license text and stands in for either family when the machine has no system
face or the player has no game art, so a stylesheet never has to name a
fallback and the harness renders every document without either.

RmlUi uses one font engine per process. `Rml::Initialise` creates its own only
when none is set, and keeps it alive until shutdown, so OpenTS initialises
first, takes that engine from `Rml::GetFontEngineInterface`, and installs one
that answers for the bitmap families and hands everything else to it. RmlUi
calls `Shutdown` on whichever engine is installed, so the engine it made is
shut down through that forwarding or not at all.

In-game text that must match the game's `.fnt` faces, needed only by the
post-migration sidebar view, still has two routes: convert them to TrueType at
build time, or extend the sheet engine over `WWFontClass` data. That choice
waits for that view.

### Dialog kit

`ui/kit.rcss` and `ui/dialog.rml` reproduce the owner-drawn dialog for every
document: the stylesheet holds the controls in the colors and metrics
`ownrdraw.cpp` draws them with, and the template holds the chrome around a
screen's content. A screen links the kit first and its own sheet after, so
its sheet overrides and holds only where things go, and its body names the
template. Every length is in `dp` and is the original's pixel value, so a
document matches its Win32 dialog at 1:1 and keeps its proportion to the game
frame at any window size; the harness lays the options menu out at ratio 2
and checks its boxes double, which a `px` length in either file fails.

Every screen is on the kit. Each one's sheet gives the dialog the size its
Win32 template comes to, then places the controls down the page in flow with
margins taken from the same template, so a screen holds no visual rule of its
own and can grow. Measured against the Win32 dialogs at 3840x2160, where a
dp is a pixel, a control at unit `x, y, w, h` lands at `⌊1.5x⌋` across and
`⌊1.625y⌋` down, rounded rather than floored for a check box, and measures
`⌊1.5w⌋ + 1` by `⌊1.625h⌋ + 1`: `Resize_Dialog` adds the pixel to every
control it carries over. A dialog's own size takes no such pixel, and the
options menu is not carried over at all. A control's width is its outer
width, borders included, because that is what the template measures. A
`dlgsys` caption is drawn from the top of its box with no leading, so a
caption's line height is the glyph's height and the box carries the row's
height; a check box label is the same. Where the game draws a control a pixel
from where the rule puts it, the sheet follows the game and says so: the
display list is a pixel shorter than its template, the front-end sound
dialog a pixel wider, and the keyboard screen's combo box and list a pixel
lower and wider. Three screens carry a second arrangement: the
sound options and the
game controls each have a frontend template and an in-game one, chosen by a
class the document sets from its model, and the message box places its buttons
in the three slots its template holds, which come out as a row spread across
the content. Where a screen departs from its template it says so in its own
sheet: the game controls make room for a difficulty row in a game, which no
template of the game's own offers.

The kit's tabs are authored from the drawing code and no migrated screen uses
them yet; the network lobbies and the skirmish screens are what will exercise
them.

The chrome is `Draw_Dialog_Back`'s composition: the 640 by 400 wallpaper
centred on the frame in whole pixels and cut off at the dialog's edges, black
beyond it, which the view places against the dialog's middle to the half pixel
a dialog of odd height is off the grid; a
24-wide bar tiled down each edge with a corner over each end; and a glow
inside the bars that the original drew as sixteen one-pixel rings of white,
alpha 96 falling by 6 a ring. The glow is a picture shipped with the kit,
`ui/glow.png`: the sixteen rings drawn once with mitred corners, cut nine
ways by a sprite sheet into corners, edges and a clear middle and drawn with
`tiled-box`, because sixteen bordered boxes each round to the pixel grid on
their own at any scale but one to one and gap and double up, and a gradient
strip per edge overlaps at the corners. It is magnified with the rest of the
art, so the rings stay whole at any scale. Two RmlUi facts shape the rest.
Decorators paint last
to first, so a bar is listed after its corners. Overflow is measured from
static boxes: positioned children are laid out after their container has
measured what it clips, and a relative offset is never counted. So the
wallpaper's box, whose only child is positioned, and the band, whose chrome
sits half a dialog to the left until its offset moves it back, are both told
to clip outright with `clip: always`. The art is the
player's own and is not shipped; each control has a plain form underneath, a
fill where a picture would be, so a screen stays usable without it, and that
form is what the harness renders.

The controls follow the drawing code's metrics: a button is a 24 or 30 tall
skin with a 7-wide left cap, a 10-wide right cap and a middle drawn from the
left cap's end to three short of the right edge, under the right cap, its
`dlgsys` caption three down from the top of a short skin and six from a tall
one, centred in a box two narrower than the skin. The middle is the tile's
own middle: the layer sampled the 177-wide tile from its centre when the run
was narrower and repeated it from the left when wider, so the kit draws the
caps and a centred, clipped tile as three `image` decorators on the box's
border and padding areas, the middle last to lie under the caps, and a
button whose run is wider than the tile carries the class `wide` to repeat
it instead; `tiled-horizontal` is no use here because it always fills its
centre. Pressing shows a skin two down that is two shorter, three for a tall
one, whose right cap is seven wide and keeps its place, so the last three
columns show through, and moves the caption one further down and one across;
the box keeps its size, with the rows the skin gives up as a bottom border
that shows through. Disabled is a half-black wash rather than the skin the
original loads and never draws. Check boxes, edit boxes, lists,
scroll bars, track bars, combo boxes, the progress bar, group boxes, hotkey
fields and tooltips are authored the same way, in the cyan frame
`OD_Draw_Rect` substitutes for white. That frame is drawn a pixel outside the
control's rect, so those controls are sized to the rect their template gives
and their frame hangs over a negative margin; a screen spaces one from its
neighbour on the neighbour's side. A list and a scroll bar are the exception:
the layer insets their rows by the frame and draws it on the rect's own edge,
so a list is its template's rect, frame and all. When a list needs a scroll
bar the original narrows the list by twenty and stands the bar beside it, so
the list's frame closes a pixel before the bar's opens and the two make a
double line, and the bar's other three edges share the list's frame; the kit
stands the bar inside the list's frame, twenty wide, with that double line as
its left border. The list's dim covers its rows only and the bar's track shows
the plain background, so the dim is carried by a box around the rows; and the
grip is as long as the layer made it, the travel less a fifth of it for each
natural-log step of the rows left over, never under fourteen, which the view
sets on the grip each pass rather than the share of the rows in view RmlUi
would give it. RmlUi places a slider's arrows, track and thumb from the
input's border corner rather than its content corner, so in the kit each of
those carries the frame's width as a margin: the track bar's track starts two
in and ends one back, which keeps it a pixel narrower than the bar with the
thumb inside the frame, and the scroll bar's parts start after its two wide
border. A track bar shows its value in a fifty
wide trough unless the dialog turned that off, which the game controls do and
the sound options do not, so a document puts the trough after the bar and
gives the bar the rest. The bar's dim reaches a pixel past its rect, so the
field is a pixel wider and taller than the fifty it is given, and the layer
fills the fifty with the middle of its tile, so the kit's field is the two
ends over a sprite of the tile's centre. A combo box's list is the toolkit's own child element,
placed by it and styled by the kit; a group box's top edge is two line pieces
either side of its caption, because a border cannot be broken, and its
caption row is sixteen tall, the height GDI gave the face, so the frame runs
eight below the caption's top. The glow sits under every control, as the
original painted it into the dialog's background before any control drew. A
screen's sheet that still needs a visual rule means the kit is missing a
control.

Pressing one of those controls sounds the click the dialog layer sounded, and
a disabled control stays silent because that layer never handed it the mouse.
The rules name the sound rather than the shell, so a mod that changes
`GenericClick` changes this one. The view turns a press into the sound,
because what counts as a press is the toolkit's business; the shell only
carries it to the host.

A modal opens as a `Begin_Dialog` dialog did. The screen is laid out whole,
then let out through a band widening from the middle, 12 pixels a side a
frame, with the bar art riding the band's edges and `EMBLEM.AUD` once at
64/255 as it starts. Frame *n* is due at n × (40 − ⌊240(n − 1) / half⌋) ms
from the start, `half` the dialog's half width in original pixels, so a
300-wide dialog opens in 13 frames and 276 ms with the schedule tightening as
it goes. The shell drives the band per modal pass from the host clock through
`UI_Reveal_Width` in `uireveal.cpp`, a pure function the logic harness runs
over the whole curve. A pass opens one step at most, however late it comes:
the original drew every band and only slept while it was ahead, so a slow
frame rate draws the reveal out rather than skipping to its end, and the debug
log records each reveal's passes and duration. The band is a clipped box whose
content is pinned to the middle of it at the width the screen was laid out at,
so no control moves as the band widens and input needs no gate; the original's
queued input reached the same controls after its sleeps. The driver pins it,
reading that width on the first pass before anything is hidden, so a screen
says nothing about the reveal beyond its own size. A document opts out with `reveal="none"`
on its body, and the harness host never animates except in the test that
watches the band.

### Strings

Engine strings are UTF-8 through the process active code page declared in
`sun.manifest`; that transition has landed. Every narrow Win32 API, including
the `LoadString` behind `Fetch_String`, yields UTF-8 bytes, and the shell
copies a string out of the `Fetch_String` cache and hands it to RmlUi
unchanged. Text typed into a field goes into engine buffers unchanged. What
the transition does not remove: fixed-size engine buffers, packet fields, and
file names are sized in bytes, so a field's character limit is a byte limit
and truncation never splits a sequence; and `WWFontClass` indexes glyphs by
byte, which bounds in-game text to the range the transition supports.

Documents reference strings by name: `[[TXT_OK]]`. RmlUi passes every text
node through `SystemInterface::TranslateString`, where the shell maps the
name to its identifier and copies the string out of the `Fetch_String` cache;
an unknown name stays as typed and is logged. The names are `#define`s in
`language.h`, so `cmake/StringTable.cmake` generates the name table into the
build's generated directory at configure time and per build, beside the build
stamp; no hand-maintained list. RmlUi re-parses a translated text node as
markup only when it contains `<`; no engine string does, and one that did
would need its `<` encoded. Dynamic text, including player and map names and
error strings, is inserted as text, never as markup.

## Configuration

One transitional key in `SUN.INI`, `LegacyDialogs` under `[Options]`, returns
every migrated screen to its legacy view while that view exists; it defaults
to `no`. Defaults are decided per screen family in code, so a family switches
to RmlUi by default when its evidence is in without a key per family. The key
is deleted with OwnerDraw. There is no build option: RmlUi and ImGui are always
compiled and linked, so one configuration matrix carries the evidence.
`Options` reads and writes the key with its other `[Options]` settings, a
caller latches `UI_Use_Rml()` at screen entry because `Options` loads after the
shell, and the key has a manual page. A sidebar view key follows the sidebar
view.

## Dear ImGui

ImGui is vendored as a submodule, compiled into Debug and Release, and
rendered by a small bgfx adapter in `rml/rmlrender.cpp` that reuses the RmlUi
renderer's program and view setup, on `VIEW_DEV`, with its own vertex layout
and straight-alpha blending, since ImGui's colors are not premultiplied. Its
geometry travels in transient buffers every frame, and its textures follow the
pinned version's contract: the renderer answers each create, update, and
destroy request and acknowledges it. The glyph atlas is created empty and
filled by updates, because bgfx makes a texture created with pixels immutable
and the atlas grows as glyphs are first drawn. Its platform adapter in
`dev/uidev.cpp` feeds it input through the shell hook ahead of the documents; the
default font is scaled by the frame's dp ratio. The context is created on the
first toggle, so a build whose developer keys never arm allocates nothing.
Overlays are armed by the developer-mode flags the manual documents; tool
visibility and frame rate never touch deterministic state. The first overlay
is the frame benchmark window on F6; network statistics and object and house
inspectors follow the same shape. A player-facing feature may choose an ImGui
view through the same screen contract; it then meets the same coexistence,
input, and evidence rules as an RmlUi view. Docking, extra native viewports,
and editor architecture are separate work.

## Sidebar

The sidebar is scheduled after the Win32 dialogs are gone. Two pieces of
work exist, in order: a toolkit-neutral split of `SidebarClass` into model
and view with the gadget view as the only view, and later an RmlUi view
covering the whole sidebar column (radar, credits, power, strips, and mode
buttons) that a player selects instead of the gadget view. No presentation
bridge and no tooltip adapter are built, because no legacy dialog exists by
then to coexist with.

The current HUD crosses `GScreen`, `Map`, `Display`, `Radar`, `Power`,
`Sidebar`, `Tab`, `Scroll`, and `Mouse`. `SidebarClass` stays as a forwarding
facade for its callers: catalog additions, factory linking, scroll commands,
redraw requests from houses and buildings. The model keeps `Column[]`, the
buildable lists, `TopIndex`, the mode flags, `Serialize`, and gains explicit
actions and queries: select a slot, scroll or page a column, toggle repair,
sell, power, and waypoint modes, and read each slot's identity, cameo, name,
cost, progress, ready state, queue count, and enabled state. The logic in
`SelectClass::Action` and the button handlers moves into those actions; the
gadgets call them.

Invariants the split preserves:

- Every field `Serialize` writes stays in the model, including the scroll
  and flash animation fields, so the save format does not move. Toolkit state
  never enters a serialized class. A save does not select the view and the
  view does not change the save schema.
- Shared behavior, `StripClass::AI`'s factory changes, completion events, and
  announcements, runs at every existing eligible `Map.Input` poll, including
  the network and timer waits, whichever view is selected. It is not one
  update per simulation frame or per toolkit update.
- Only the behavior path calls `Has_Changed`. Snapshots, bindings, rendering,
  and a hidden view never consume it.
- A slot is identified by `(RTTIType, ID)` revalidated at execution, never by
  a captured index; reordering must not redirect a click, and a scenario load
  invalidates old intents even if numbers are reused.
- A dimmed cameo is not a disabled control. Draw code computes darkening
  separately from `SelectClass::Action`; a click can still announce a
  condition or enqueue a request that `HouseClass::Begin_Production` rejects.
  Rejection is not moved earlier.
- The selected view owns tooltip registration for its regions and removes the
  registrations `Reposition_Sidebar` makes for regions it covers.
- Before scenario destruction or load, pending intents and bindings are
  invalidated; after pointer fixup, the selected view is recreated from
  current state.

## Progress and other systems

Progress tracking, clamping, milestone text and sound, and the readiness
queries that `scenario.cpp` consumes move out of the draw path into shared
behavior, so a repaint cannot repeat a milestone sound and a hidden
presentation cannot lose one. The milestone half landed with step 6:
`ProgressScreenClass::Advance_Milestone` notes a threshold crossing and plays
its sound when the progress moves, and the paint draws the text still owed.
The screen exposes phase, progress, status, and the operations the loader
supports; no cancellation is added to a loader that cannot cancel. Loading
stays on its thread with explicit cooperative service points that drain
nothing unrelated while scenario objects are being replaced, and the first
paint happens before long work begins.

MSEngine screens (campaign selection, briefings, score screens) are features
with animation, audio, and navigation. RmlUi can replace their layout and
controls while the existing image, animation, video, and audio services
supply content; their waits, focus pause, and callbacks stay explicit, and
replacing their timing with CSS animation is a deliberate per-screen choice.
A stable bespoke screen may stay bespoke. The message list and restate screen
can follow the screen contract when someone wants them.

## Compatibility

| Boundary | Requirement |
| --- | --- |
| Gameplay | Command meaning, eligibility, ordering, and side-effect ownership preserved; presenters use the same calls the dialog procedures use. |
| Determinism and networking | UI timing stays out of the simulation; sidebar poll placement and serialized reads unchanged; lobby and in-game screens send the same events. |
| Saves and replays | Representations and reconstruction paths unchanged; toolkit objects and view preference never enter game state. |
| Class layout and COM | Presenters and adapters stay outside layout-sensitive structures. |
| Configuration | Existing keys and defaults unchanged; new keys get owning documentation. |
| Localization | The UTF-8 transition owns the encoding change; the UI adds no conversion of its own. |
| Mods and resources | Legacy asset semantics unchanged; document paths, binding names, event names, and the styling profile are experimental until versioned with the first supported override package. |
| Build | Win32 and x64 MSVC with the static CRT for every new dependency; CI builds and tests both platforms. |
| Portability | No new operating system type outside the coupling listed under [Portability](#portability). |

### Portability

Windows is the supported target and the only platform the shell is written
for. Portability is a direction rather than a feature here: preparatory work
does not make another platform supported, and no platform layer is invented
before there is a second platform to validate it against.

These carry no operating system type today, and a port keeps them as they
are: the presenters and their service interfaces, the screen contract, the
view interface, the input ownership table and its text decoding, the pointer
mapping, the render checks, and the bgfx renderer.

The rest is written in Win32 terms. A port pays for it here:

| Coupling | What a port costs |
| --- | --- |
| The shell's window message hook and its pumped-message intercept | The structural item. Messages become a neutral event at the platform edge, which rewrites one signature and the body behind it |
| The window handle on `UIShellHostClass` | Three uses: two identity comparisons and the clipboard's owner. An opaque handle would serve, and `uihost.h` would stop pulling `win.h` into everything that includes it |
| Key mapping in `code/ui/rml/rmlkeys.cpp` | Not only code. `KEYBOARD.INI` stores Windows virtual key numbers, so the mapping is also a data-format boundary |
| The clipboard in `rmlsystem.cpp` and the conversions in `uiunicode.cpp` | Replaceable in place; `tests/uishell` already covers the behavior |
| The developer overlay's input entry points | They follow whatever event type the hook adopts |
| `UIWaitBoxClass`'s window member | The only screen header that reaches Win32, and the one leak in the containment rule |

New UI code adds no operating system type outside that list. A presenter, a
service, or a screen header that needs one has the wrong shape.

## Dependencies

Additions, as submodules under `thirdparty/` pinned at tested tags like bgfx,
built static with the static CRT that `thirdparty/CMakeLists.txt` forces:

| Project | License | Notes |
| --- | --- | --- |
| RmlUi 6.x | MIT | `RMLUI_FONT_ENGINE=freetype`, no samples, no backends, static |
| FreeType 2.14 | FTL | zlib, bzip2, PNG, HarfBuzz, and Brotli disabled, so the gzip module uses the bundled zlib copy; aliased as `Freetype::Freetype` for RmlUi's dependency check |
| Dear ImGui | MIT | core sources compiled into a small target; no bundled backends |

`THIRD_PARTY_NOTICES.md`, `thirdparty/licenses/`, and the packaging license
copy grow by the three projects and the components they bundle: robin_hood
and itlib in RmlUi, zlib in FreeType, and the stb headers in Dear ImGui. CI
already checks out submodules recursively. The build stamp step gains the
string-name generator. Dependency upgrades are separate changes.

## Migration plan

Each step is one pull request unless noted, builds and runs on its own, and
leaves the game playable. A behavior-heavy screen takes two changes: the
first extracts its behavior behind the presenter with the legacy view still
selected and classifies as preserved; the second adds the RmlUi view. A leaf
takes one. Sizes are rough: S under a day of focused work, M a few days, L a
week or more. The order is bottom-up because of the coexistence rule: a
screen migrates only after every screen it can open has migrated.

The UTF-8 transition that step 3 needs has landed. Steps 1 and 2 need no text
beyond an ASCII test document.

1. **Dependencies** (S, landed). Submodules, CMake, notices, `BUILDING.md`,
   and a `tests/uishell` smoke test that links the three libraries. No engine
   code uses them. Evidence: Debug and Release build.
2. **Shell** (M, landed in two changes). Everything in the code-layout table
   except screens, the backend split, the input hook, resize handling, the
   `ui/` copy step, the file interface with mix resolution, and a Debug-only
   test document toggled by F9; then the Dear ImGui context, its renderer, and
   the frame benchmark window toggled by F6. Evidence: the test document
   renders over the main menu and in game at several resolutions and scale
   modes; clicks on it, beside any legacy dialog, are consumed; clicks beside
   it reach the game; legacy dialogs still open and close; repeated open and
   close leaks nothing.
3. **Version dialog** (S, leaf, landed in two changes: the string table, the
   `LegacyDialogs` key and the coexistence checks, then the screen contract,
   the modal runner and the dialog). The integration pilot: fonts, clipping,
   mapping, dismissal by mouse and keyboard, focus return, UI-only redraw,
   resize, preparation failure. The main menu keeps hiding around it.
4. **Message boxes** (M, leaf, landed). The modal runner landed with step 3.
   `WWMessageBox::Process` behind the kill switch, preserving button order,
   default button, Escape, the no-button case, return mappings, and
   session-end interruption; a box raised over a visible Win32 dialog stays a
   Win32 box until that dialog migrates. `OwnerDraw::Custom_Message_Box` is
   the modeless progress box of the save and load flows and moves to step 6.
   Runtime evidence still owed: the multiplayer cases where `Main_Loop` runs
   under the box.
5. **Sound** (M, two changes, landed: the behavior sits behind
   `UISoundPresenterClass` and an engine service, the Win32 dialog drives it
   with the same calls in the same order, and `sound.rml` is the RmlUi view
   with a `data-if` for the in-game half). The behavior pilot: volumes,
   eligible themes, selection, availability, shuffle and repeat, immediate
   previews, play and stop, both templates, frontend and in-game service
   paths. Runtime evidence so far is under
   [What has been exercised](#what-has-been-exercised).
6. **Progress and wait** (S, leaf, two changes, landed: milestone effects
   moved out of drawing, then `UIWaitBoxClass` over `wait.rml` for the saving
   and loading boxes and the progress dialog, with the Win32 boxes kept
   behind it). `IDD_PROGRESS_WAIT`, the saving and loading boxes in
   `savemgr.cpp` with `OwnerDraw::Custom_Message_Box`, the modeless box they
   show. A progress bar needs no engine surface, so the `<surface>` element
   waits for the map preview in step 10. Runtime evidence still owed.
7. **Options family** (L, landed for the frontend and the in-game settings:
   the game controls sit behind `UIGameControlsPresenterClass` and an engine
   service with `gamectrl.rml` covering the three Win32 templates through
   `data-if`; `UIDisplayPresenterClass` hands the caller the mode to try and
   `UIConfirmModePresenterClass` reads a `UIClockClass` and cancels itself at
   the timeout, which replaced the posted `WM_DESTROY`, over `display.rml`
   and `confirm.rml`, the latter counting the seconds down;
   `UIKeyboardPresenterClass` edits a copy of the hotkey table that OK saves
   and Cancel drops, where the Win32 procedure edited the game's table and
   reloaded the file on Cancel, and `keyboard.rml` captures a key through a
   focusable element that `rml/rmlkeys.cpp` turns back into the `KEYBOARD.INI`
   number; the options menu is `mainopt.rml`, placed where the main menu's
   buttons were; surrender runs through the message box screen, while abort
   is `abort.rml` over its own `IDD_MISSION_ABORT` template, whose middle
   answer surrenders rather than restarts outside a campaign mission). The
   Win32 templates remain the fallback view of every one. The
   in-game options menu opens load, save and delete, so it follows step 9.
   Evidence: settings round-trip through `SUN.INI` unchanged.
8. **Main menu family** (M, campaign choice landed as `campaign.rml`, which
   names the difficulty the bar is set to from the start where the Win32
   dialog left its template's caption until the bar first moved).
   `IDD_MAIN_MENU`, game type and multiplayer game selection remain; all
   three appear only where `GMENU.MIX` is missing and the graphic menu cannot
   run, so they are the degraded install's menu rather than the shipped one.
   The `NewMenuClass` drivers keep their loops.
9. **Load, save, delete** (M, two changes).
10. **Skirmish and map selection** (M, two changes). Includes the scenario
    picker templates and the preview surface.
11. **Network lobbies** (L, two changes). Host, guest, game list, the `WS_`
    stack, and `netshare.cpp` as one family; then disconnect, desync, and
    reconnect. Packets unchanged.
12. **Map generator and WDT** (L).
13. **Retire OwnerDraw** (M). Delete `ownrdraw.cpp`, `windlg.cpp`, the
    modeless dialog list, the dialog templates, the kill switch, and the
    coexistence assertions. String tables stay.
14. **Sidebar** (M, then L). The model and view split with the gadget view;
    later the RmlUi view over the whole column and its selection key.

ImGui overlays (S each) follow step 2: the frame benchmarks landed with it,
then what a developer needs next. GadgetClass screens, MSEngine screens, and
the credits are unscheduled.

## Validation and evidence

Three CTest targets run without game assets and build into
`<build directory>/test-bin/`.

`tests/uishell` brings FreeType and Dear ImGui up and down under the engine's
link settings, drives RmlUi core through a recording render interface and a
counting system interface, links the string table and the screen presenters,
and builds `UIShellClass` itself over a host the test controls:

- Load every shipped document from the source tree with the shipped font under
  both family names, which is what a machine with neither the system face nor
  the game's art has,
  show, update, and render it, and fail on a parse error, an RmlUi warning
  or error, a call to a render method the shell leaves at its default, a
  fragment the renderer would refuse, a scissor outside the context, or a
  resource named by anything but a bare file name; after shutdown, every
  compiled geometry and texture has been released.
- Draw a rotated element and a rounded one that clips its content, each
  written in the test rather than shipped, and assert that the transform and
  the clip mask reach the renderer, that no deferred effect is asked for, and
  that the mask is no longer in force afterwards.
- Bind a presenter, drive it with `Context::ProcessMouseButtonDown` on a
  known element, and assert the queued intent and result; drive the same
  actions through the legacy adapter and assert the same ordered service
  calls.
- Reject stale intents after close, suspension, scenario generation change,
  and catalog removal.
- Scan shipped documents for `[[TXT_*]]` names and check each exists in the
  generated table.
- Map client positions into the overlay at integer and fractional scales,
  with letterboxing, exclusive edges, outside input, and the offset a captured
  pointer keeps outside.
- Run the shell: initialize over injected interfaces and again after a
  shutdown; drive a modal with a stub service to each result; consume a press
  pumped while a screen opens; defer a resize arriving inside a render;
  suppress what is held as a screen opens, what a lost capture cancels, and
  what focus return finds held; restore the outer modal's ownership after a
  nested one; take the side buttons and the horizontal wheel under a modal;
  decode two UTF-8 bytes into one character; round the clipboard through
  UTF-16; show and put back the pointer shape; list, unlist and release a
  modeless notice.
- Drive whole screens through the hook, one input mechanism each: a button on
  the options menu, a mode row on the display options, a switch and a
  backwards slider on the game controls, and a captured key on the keyboard
  screen. Each is asserted the whole way, from the window message through the
  intent and the service call to the model that comes back to the document.
  The mode confirmation runs the same way over a clock the test moves,
  because its result comes from a refresh rather than from an intent.
- Open the options menu on the dialog kit: the wallpaper's scissor is the
  menu's own box, the menu and its buttons double at twice the ratio, and
  under the modal runner over the real clock the band starts narrow, widens
  by one step a pass at most around content that never moves, ends open, and
  sounds once.

`tests/uilogic` compiles the toolkit-free state with no UI library: the input
ownership table and its cancellations, the UTF-8 decoder, the renderer's
geometry, size and scissor checks, the model matrix a transformed fragment
draws through and the stencil each clip mask operation asks for, the PCX
decoder over files the test builds itself, the bitmap font's metrics, remap and
atlas over sheets it builds itself, the reveal schedule a pass at a time and
under passes too slow for it, the point for point magnifying of a picture, and
the presenter's marks through consume, restore and reset.

`toolkitheaders` runs `cmake/CheckToolkitHeaders.cmake` over `code/` and
fails on a toolkit or renderer header included outside `code/ui/rml/`, or an
RmlUi or ImGui header included by a source outside `code/ui/`.

Runtime evidence stays per pull request, as `CONTRIBUTING.md` requires: the
screen exercised in single player, skirmish, and a two-instance LAN game
where it can appear during play, at a native and a scaled resolution, with
repeated open and close, focus loss and return, keyboard-only navigation,
session end while open, and no leakage of wheel, edge scroll, or shortcuts
underneath. Editable screens add non-ASCII input, paste, and composition.
Capture paths that read software surfaces omit the overlay; capture after
composition or state the limitation. No performance target is asserted
before measurement; idle CPU, update time, submission cost, and texture and
geometry memory are recorded on an agreed baseline before defaults change.

### What has been exercised

A pass by hand on 13 September 2026 drove the migrated screens from the
frontend on both platforms, and from a skirmish on `Win32`. The two platforms
behaved alike. The debug log names each document as it opens and closes, so
the table below is what the logs of that pass contain.

| Screen | `Win32` | `x64` |
| --- | --- | --- |
| Version | yes | not yet |
| Message box, raised over the keyboard screen by the hotkey reset | yes | yes |
| Sound, frontend and in game | yes | not yet |
| Game controls | frontend and in game | frontend |
| Display, with the mode confirmation, its countdown and its timeout | yes | yes |
| Keyboard, with key capture, reset, and every category | yes | yes |
| Options menu | yes | yes |
| Wait notice, over eleven consecutive quicksaves | yes | not yet |
| Progress, with its bar | never shown | never shown |

The defects the pass found were mostly in the shell rather than in any one
screen: the pointer shape, the absence of a tick while the graphical menu is
up, which left every document and overlay frozen there, and a notice the
present interval could swallow before it was ever drawn. A screen that looks
right is therefore not evidence that the shell is.

The options menu was the first screen on the dialog kit and is the one the kit
was checked against by hand, over the graphical menu at twice the frame scale:
chrome, wallpaper, reveal, and the buttons at rest, pressed and disabled. The
other screens moved onto the kit after it and are owed the same pass; the
harness covers their layout and their input, not how they look.

Still owed: the progress document, which belongs to the multiplayer loading,
map generation, and file transfer paths; the multiplayer cases where
`Main_Loop` runs under a message box; and, for each screen, what the paragraph
above requires of its own change.

## Documentation

- This page owns the architecture and is updated as steps land.
- `docs/BUILDING.md` lists the new submodules, the harnesses, and where the
  `ui/` directory lands beside the executable.
- `THIRD_PARTY_NOTICES.md` and the packaging license copy gain the three
  projects.
- The manual gains a systems page for the UI files (where they live, the
  override order, the string reference syntax), a key page for the kill
  switch, and a change record per migrated screen. The sidebar page changes
  when its view key lands.

## Open decisions

- The in-game text route for the sidebar view: TrueType conversions of the
  game fonts or a bitmap font engine for every document.
- The document and binding versioning rules for mods, fixed with the first
  supported override package.
