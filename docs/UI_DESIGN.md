# UI system design

This page owns the UI architecture: the RmlUi shell that runs the game's
screens, the Dear ImGui overlays beside it, and the screens still drawn by
the older systems. [Building OpenTS](BUILDING.md) owns build support and
[Project direction](DIRECTION.md) the wider architecture.

## Where the UI stands today

The screens are RmlUi documents run by the shell this page describes. Three
older systems remain beside them. They share the software frame and the
keyboard queue but nothing else.

| System | Files | Used by | Draws into |
| --- | --- | --- | --- |
| GadgetClass | `gadget.cpp`, `control.cpp`, `toggle.cpp`, `list.cpp`, `edit.cpp`, `slider.cpp`, ... | sidebar, radar, tactical buttons, message list, checklist, mission restate | `LogicalSurface` (`SidebarSurface`, `HiddenSurface`) |
| MSEngine | `msengine.cpp`, `msanim.cpp`, `grphmenu.cpp` | graphic menu, map select, score screens, WDT screens, credits | `AlternateSurface`, `HiddenSurface` |
| Bespoke | `progress.cpp`, `score.cpp`, `movies.cpp` | loading screen, score, movies | `HiddenSurface` |

A fourth system, OwnerDraw, drew the dialogs and is gone. It was the largest
and the least portable: each dialog a real Win32 child window of `MainWindow`
created from a resource template, every control subclassed and painting into
`AlternateSurface` itself, 53 templates in `language.rc` carrying 325
`CONTROL` entries, and `Heal_Dialog_Controls` repainting every child window
after each `Update_Visible_Surface` so that a dialog kept up with the game
frame. `windlg.cpp`, `msgroute.cpp`, the modeless dialog list, the accelerator
table and the 53 templates went with it; `ownrdraw.cpp` went from 7,019 lines
to the 918 that outlived it, the remapped bitmap-font drawer the briefing and
the restate screen use, the GDI text the credits put on a surface, the surface
cache, the blend masks, the hotkey spelling and the mouse capture the graphic
menus take. The string tables stay, and so do the `IDD_` and `IDC_` numbers in
`language.h`, which are the library's index.

The frame path is simple and already hardware-presented. The game draws 16-bit
pixels into system-memory surfaces. `Video_Present` hands `VisibleSurface` to
`Backend_Present`, which uploads it as one texture, draws one quad with the
embedded `vs_ocornut_imgui` program on view `VIEW_PRESENT` (with a
`VIEW_PRESCALE` pass for the pixel-art filter), and calls `bgfx::frame()`.
bgfx runs single-threaded because a present happens at whatever depth the
engine has reached; `_Presenting` guards re-entry and defers a resize that
arrives inside one. Presents are paced to the refresh interval and happen only
when the frame is dirty. The mouse pointer is a hardware Win32 cursor built
from the game's shapes, so it never touches a surface. The window is
per-monitor DPI aware, and `VideoScaleInfo` records where the logical frame
lands in the physical client area.

Input reaches the game one way. `MainWindow` is the only window, so
`Windows_Procedure` sees every message: it maps a mouse position into the
frame's own pixels where the frame is drawn scaled, then offers the message to
the shell's hook, then to `Map.Message_Handler`, then to its own switch, then
to `Keyboard->Message_Handler`, which packs keys and mouse buttons with their
position into the `KN_` queue. Gadgets poll that queue in
`GadgetClass::Input`; `WWKeyboardClass::Down` reads `GetAsyncKeyState`, so
withholding a queued key does not hide a held key from polling code.
`Windows_Message_Handler` takes the developer keys before it dispatches, so
one works wherever the message was headed.

Every screen owns its loop. In game, `Main_Loop` runs input, logic, and
render. A screen's driver spins on `UIShellClass::Run_Modal`, whose service
pass pumps messages and then runs `Main_Loop` in a network session or
`Call_Back` otherwise, so multiplayer keeps stepping under a screen; the
dialog drivers spun on `OwnerDraw::Dialog_Message_Handler` the same way.
MSEngine screens spin on `Engine.Wait_Delay`. `RestateMission` mixes gadgets
with MSEngine. `Keyboard->Clear()`, which every driver calls around a screen,
pumps Windows messages through `Fill_Buffer_From_System`, so cleanup can
re-enter UI code.

Nearly every legacy flow hid or destroyed its parent before opening a child:
the main menu around the version dialog, the options driver ending the main
dialog before a sub-dialog, in-game options around save and load, skirmish
around the scenario picker. A screen now stays open and hides itself under the
one it raises, which is what the shell's hide-parent rule under
[Scheduling](#scheduling) does; the lobby still reads an explicit phase rather
than a window stack to know which of its three it is standing in.

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

Three consequences shaped the design. A new UI had to fit the blocking-loop
shape, or every driver would have been rewritten in one change; the loop shape
is fine and the screen bodies were the problem. Anything drawn into the
software frame sits under anything drawn by bgfx. Input must be claimed before
it enters the `KN_` queue.

## Goals and limits

Goals:

- Replace OwnerDraw with RmlUi documents, one screen per change, with the
  legacy screen available behind a switch until OwnerDraw was retired.
- Make screens interchangeable at the screen level: a model or presenter that
  knows nothing about the toolkit, and one view per toolkit.
- Keep the subsystem portable by preparation, the approach
  [Project direction](DIRECTION.md) sets for the engine. Screen behavior,
  the screen contract, input ownership, coordinate mapping, and the render
  checks carry no operating system type, so a later port replaces the host
  and the platform edge rather than the screens.
- Leave GadgetClass and MSEngine in place; migrate them later through the same
  screen contract when a screen is worth it. The sidebar follows now that the
  Win32 dialogs are gone, as a player-selectable alternative to the gadget
  sidebar.
- Add Dear ImGui on the same shell for developer tooling, available to a
  player-facing feature if one wants it.
- Keep modding open: documents, styles, and images load through the game's
  file system with mix-file support from the first screen.

Limits, chosen to keep the work bounded:

- No widget-level abstraction across GadgetClass, RmlUi, and ImGui. Views are
  whole documents.
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
and only `code/ui/` sources include RmlUi or ImGui; the `toolkitheaders` CTest
check enforces both. `bgfxbackend.cpp` is the only other code that includes
bgfx, by convention rather than by the check.

A **screen** is a presenter plus a view. The presenter is a plain C++ object:
it holds a view-model struct, answers queries, and executes actions. It never
sees an `HWND`, a `Surface`, an `Rml::Element`, or an ImGui call. A view
renders the view-model and turns user actions into intents. The RmlUi view is
a document with a data model; an ImGui view is possible for a feature that
wants it. A screen returns a result the way a dialog returned `rc`.

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
`UIShell.Tick()`. What the shell needs from the program around it comes
through `UIShellHostClass` (`uihost.h`), so a harness builds its own
`UIShellClass` over a host and interfaces it controls and never links the
engine. A modal screen's engine entry calls `UI_Run_Modal(view)` from
`uienginehost.h`, which runs the screen on `UIShell` with `UI_Service_Game`
as the service pass.

### Code layout

Sources live under `code/ui/`, grouped by what they may include. The
recursive glob in `code/CMakeLists.txt` picks them up; the per-file
properties carry only the shader headers, the image decoder header, and the
bgfx debug define, as `bgfxbackend.cpp`'s do today. The library headers reach
the whole target through the linked targets, so the containment below is a
rule the tree follows, not a build boundary.

| Directory | Holds |
| --- | --- |
| `code/` | `bgfxviews.hh`, the view ids the presenter and the overlays share; `_ui.h`, `_ui.cpp`, the shell's one instance `UIShell` under the underscore-file convention for globals |
| `code/ui/` | the shell and the toolkit-free contracts: `uishell.h`, `uishell.cpp` (`UIShellClass`: init and shutdown, resize, input hook, developer-key intercept, tick, overlay render entry, modal runner; its toolkit interfaces are injected, so a test builds its own instance); `uihost.h` (`UIShellHostClass`, what the shell needs from the program around it: the window, frame, keyboard queue, strings and log); `uienginehost.h`, `uienginehost.cpp` (the engine's host and the game-service pass a modal runs with; the only shell file that includes engine headers); `uiscreen.h`, `uiscreen.cpp` (presenter, intent, result, clock); `uiview.h` (`UIViewClass`, the view the shell runs); `uiinput.hh`, `uiinput.h`, `uiinput.cpp` (who owns each held key and button, and the UTF-8 decoding of a narrow window's text); `uiunicode.h`, `uiunicode.cpp` (strict UTF-8 and UTF-16 conversion for the clipboard); `uicoord.h` (the pointer mapping from client pixels into the overlay); `uireveal.h`, `uireveal.cpp` (the schedule a dialog opens on; toolkit-free, so the harness runs it); `uipreview.h`, `uipreview.cpp` (the map preview pictures the engine drew, read for the `<surface>` element) |
| `code/ui/rml/` | the RmlUi adapters, the only headers that include a toolkit: `rmlsystem` (system interface: time, logging through the host, string translation, the pointer request, the clipboard), `rmlfile` (file interface over `CCFileClass`), `rmlrender` (render interface and the ImGui renderer on bgfx; with `bgfxbackend.cpp` the only files that include bgfx), `rmlrenderbase.cpp` (the failure latch, and the layer, filter and shader calls the renderer declines; built into the harness), `rmltexture` (image files: PCX, PNG and TGA today, with SHP and engine surfaces described under [Assets](#assets-and-strings)), `rmlimage` (turning PCX bytes into indices and RGBA, a frame surface's 565 pixels into RGBA, and the fit of a picture in a box), `rmlsurface` (the `<surface>` element, for a picture the engine drew while the game ran), `rmlfontsheet` (measuring and coloring the dialog art's bitmap font), `rmlfontfon` (reading the `FNT` strikes out of the raster face the dialogs drew with; all three toolkit-free, so the harness runs them), `rmlfont` (the font engine over the sheets and the strikes, forwarding every other family to the engine RmlUi made), `rmlkeys` (virtual keys, `KeyIdentifier`, `KEYBOARD.INI` numbers), `rmlview` (`UIRmlViewClass`, the RmlUi view base), `rmlrendermath` (the checks the renderer makes before it draws: index ranges, byte counts, scissors; toolkit-free, so the harness runs them) |
| `code/ui/dev/` | `uidev.h`, `uidev.cpp`: the ImGui context, its input feed, and the developer overlays |
| `code/ui/screens/<name>/` | one family each for `abort`, `campaign`, `desync`, `display`, `gamectrl`, `gameopt`, `keyboard`, `mainopt`, `mapgen`, `menu`, `msgbox`, `netlobby`, `reconnect`, `savegame`, `scenario`, `skirmish`, `sound`, `version`, `waitbox`: `ui<name>.h` (presenter, service and state declarations, view factory, engine entry), `ui<name>.cpp` (presenter and RmlUi view; built into the test), `ui<name>dlg.cpp` (engine service and entry, which the test cannot link); the wait box family carries the `UIWaitBoxClass` the save, load and progress code shows, and the reconnect family the notice the frame-sync wait stands beside the game |
| `tests/uishell/`, `tests/uilogic/` | the two harnesses under [Validation](#validation-and-evidence); `cmake/CheckToolkitHeaders.cmake` is the containment check they run beside |

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
if (Backend_Present(pixels, ...)) {   // VIEW_PRESCALE, VIEW_PRESENT
    UIShell.Render_Overlay();         // VIEW_UI, then VIEW_DEV
}
Backend_End_Frame();                  // bgfx::frame()
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

While OwnerDraw existed, an RmlUi or ImGui document could be shown only while
every legacy dialog was hidden or destroyed, and a legacy dialog only while no
overlay document was visible. Both halves followed from the survey: legacy
pixels are under the overlay, and a visible legacy window takes the mouse
before the shell sees it. The rule set the migration order, since a screen
migrated only after every screen it could open had, and debug assertions in
the dialog layer and the shell's show path enforced it. One presentation is
left, so nothing enforces it now, and what remains of the policy is that
native and GPU UI are never layered.

### Hook and priority

The shell gets a hook in `Windows_Procedure` after the frame-pixel mapping and
before `Map.Message_Handler`. The mapping rewrites a scaled position into the
frame's own pixels for the game, so the hook receives the position as Windows
delivered it:

```cpp
if (UIShell.Handle_Window_Message(hwnd, message, wParam, client_lparam)) {
    return(0);
}
```

The overlay lays itself out in the window's pixels, which is why it takes the
position unmapped; placing the hook before the keyboard handler keeps consumed
input out of the `KN_` queue. The hook covers mouse, wheel, key, and text
messages, and watches capture and activation changes to end the presses it
owns without consuming them. Size, paint, transport, and system messages continue on their
paths. A developer key is intercepted earlier still, in
`Windows_Message_Handler` rather than in the hook; it only records a request
that the next tick executes.

Priority follows scope and capture, not toolkit:

1. Application lifetime handling: activation, shutdown.
2. The active exclusive modal.
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
        virtual bool Prepare(UIShellClass & shell) = 0;   // load; false means it cannot open
        virtual void Show(bool modal) = 0;
        virtual void Hide(void) = 0;
        virtual void Release(void) = 0;
        virtual void Sync(void) = 0;                      // presenter changes into the view
        virtual UIPresenterClass & Presenter(void) const = 0;
};

class UIRmlViewClass : public UIViewClass {       // rml/rmlview.h: owns the document
    public:
        UIRmlViewClass(UIPresenterClass & presenter, char const * document, char const * model);
        virtual bool Bind(Rml::DataModelConstructor & model) = 0;   // view-model fields and events
        virtual void Sync(void) override = 0;                       // dirty what Execute changed
};
```

A screen's factory, `UI_<Name>_View(presenter)`, returns a
`std::unique_ptr<UIViewClass>`, so the engine entry that builds the presenter
and runs the view includes no RmlUi header. The shell runs any `UIViewClass`;
the RmlUi view is the only implementation.

The view-model is a struct of plain values and vectors that RmlUi's data
binding renders; the document uses `data-model`, `data-value`, `data-for`,
and `data-event-click="queue('ok')"`. Intents are small tagged values holding
identities and copied data, never DOM pointers, borrowed buffers, `HWND`s, or
unprotected engine pointers. The presenter copies what it needs out of
`Options`, `Session`, or the scenario into the view-model and writes back on
accept, which is what the dialog procedures did with `TempOptions`. A
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
cancelled, session ended, and failed to open. A screen answers with values of
its own rather than the `IDOK` and `IDCANCEL` its dialog returned; a caller
that still needs a Win32 number maps it at the call. Settings that apply
immediately do not gain an apply-and-cancel transaction.

An entry builds its presenter, runs its view and answers:

```cpp
int WWMessageBox::_Process(const char * msg, int defresponse, const char * b1txt, ...) {
    return(UI_Message_Box(msg, defresponse, b1txt, b2txt, b3txt));
}
```

Preparation (documents, bindings, resources, host scope) completes before a
view becomes interactive. A screen the shell cannot prepare, because it did
not start, the shipped font is missing or a document will not parse, names the
reason in the log and answers as a missing dialog template did: `-1` from a
message box, cancel from a choice. From the main menu that is `SEL_EXIT`, so
a broken install leaves with its reason recorded rather than hanging. After
activation, a view failure recreates presentation against the surviving
presenter state and never replays accepted intents.

## Scheduling

Three kinds of work keep their owners: game behavior and command production
run at their existing call points with their existing gates; toolkit input,
layout, and animation run at the shell's service points on the application
thread; GPU submission runs in the presenter. UI animation uses wall-clock
time and never reads or advances deterministic game timers.

A migrated dialog driver keeps its shape. `Run_Modal` is the RmlUi twin of
the `Dialog_Message_Handler` loop:

```cpp
UIResult UIShellClass::Run_Modal(UIViewClass & view, UIServiceCallback const & service, bool hideparent = false);
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

A screen raised over another hides it. `hideparent` takes the screen below
down for the child's passes and shows it again when the child closes, without
a reveal, so a screen that raises one stays open behind it rather than closing
and opening again. The five flows that inherited the Win32 hide-and-reopen run
the child from the presenter's service inside the parent's pass: the skirmish
setup around the map dialog, the map dialog around the generator, the in-game
options and the generator around their saved-game dialogs, and the lobby around
the map dialog. The lobby's join, its name check and its refusals to start act
from the service the same way, so the screen stays up with its chat line and
its pick until a packet or the host's Go moves the flow on. A message box still
nests visibly, as the Win32 boxes did over their dialogs.

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

Non-modal documents are updated by a `UIShell.Tick` call in `Main_Loop` next
to `Map.Input`, and by the modal runner's own pass, so a notice stays alive
under a screen, and are rendered by every present. A notice a caller shows
while it works goes through `Show_Modeless`, `Refresh` and `Hide_Modeless`,
which tick and present at once because such a caller pumps nothing. That present ignores the interval
between frames: the caller gets no second chance, so a notice raised soon
after the last frame would otherwise never be drawn at all.

A screen over a running game is paced by that game. Its runner gets one pass
per `Main_Loop`, so in a network match, where the loop is held to the session's
frame rate, the band it opens through and every control under the pointer
followed the game at thirty passes a second; the Win32 dialog never showed this
because Windows repainted its controls from their own messages rather than from
the game's loop. The game already spends that time idle in two waits, the
frame's own in `Sync_Delay` and the one for the other players' packets in
`Wait_For_Players`, and both are given to the shown screen instead. A pass taken
there advances only what the screen looks like: the band, the layout and one
present. It drains no intents, so a press still waits for the runner rather than
executing engine work from inside a packet wait.

A match decided while a screen is up is not finished under it. `Main_Loop` sees
the win or loss flag with a screen shown, runs nothing further and answers that
the game ended; every runner on the stack closes on that answer, one per pass,
and the outer game loop's next `Main_Loop` call finds the flags still set and
runs `Do_Win` or `Do_Lose` before any further frame. The score screen and the
movie therefore never run inside a service pass, where the shown screen would
hold the mouse and key messages they poll for. The `-TIME=` tournament score
screen is the one presentation still drawn from inside a frame.

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
texture so the art loads again. So does a side mounting its archives, the
other moment a name can start answering with different bytes. SHP frames are
not decoded yet; they are for the sidebar view, in a `name.shp#frame` form
with an optional palette and index zero transparent.

A picture the engine drew while the game ran reaches a document through the
`<surface>` element, which is handed its pixels rather than naming a file. The
map preview is the only such picture: the desync dialog's host marker is
`wolhost.pcx` through the ordinary image path, and a progress bar needs no
picture at all. The pixels travel as bytes in the presenter's state, the way
every other engine fact reaches a screen, so images have one route rather than
a second of their own. The element widens a frame surface's five and six bit
channels by repeating their top bits, places the picture in proportion and
centered in its box, resamples it there by taking, for each pixel, the one
under its own middle, which is where the stretch GDI gave the dialog layer
landed, and holds it in a callback
texture, so the release of every texture that follows a change of frame scale
regenerates it. The picture is not magnified with the rest of the art, because
it is already drawn at the size it is shown. The centering is a deliberate
departure: the layer scaled a preview in thousandths and then halved both the
frame and the picture in whole pixels, which left the picture a pixel short of
the frame with the gap all on one side. The
box a screen gives it is the frame's rect rather than the room inside the
frame's line, because a frame draws its line corner to corner inclusive, a
pixel outside the rect it was given. A map carrying no preview leaves the element
drawing nothing rather than failing its screen. Original
game art stays local runtime data outside version control; documents receive
artwork identities, never engine pointers.

Colors are the art's own. The original drew into five bits of red and blue and
six of green, so its pixels carried whatever that frame made of them; the kit
draws in eight and keeps each picture's palette as the file holds it. A color
can therefore differ from the original's by a level or two, and a blended pixel
by more, since the original blended in sixteen bits.

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
edge than its width alone would, and halved the room around a centered one in
whole pixels where RmlUi rounds a half up; a sheet gives a right-aligned box
a pixel of right padding, and the kit makes a centered box a pixel narrower,
which brings both to the layer's pixel.

The sheets are read at the first screen and again whenever a side mounts its
archives. A face keeps its own copy of them, so the faces cut from the sheets
that went are let go with them. A side mounting its archives drops the parsed
style sheets and templates too, and because a document keeps the sheet it was
built from, only a screen opened afterwards reads the markup again.

`dlg-sans` is the face the Win32 dialogs asked GDI for, and it is the same
raster face rather than a stand-in for one. They asked for "MS Sans Serif",
which GDI answers from `sserife.fon`: a sixteen-bit image whose resources are
`FNT` strikes, one per point size, each holding a width and a one-bit picture
for every character. `rmlfontfon` reads the strikes and the font engine draws
them, so the letters on a screen are the letters the layer drew. The shell
asks the host where the file is rather than naming a Windows path itself.

Windows cuts that face once per code page, and the shell folds six of the cuts
into the one family: `sserife.fon` Western, then Central European, Cyrillic,
Greek, Turkish and Baltic. A strike names its page in the `dfCharSet` byte at
offset 0x55, so the reader hands back code points rather than bytes and the
cuts merge on the code point. That takes a strike from 219 characters to 469.
The order is fixed and the first cut to carry a character keeps it, so the
family comes out the same whichever cuts a machine has; a machine with only
the Western one is exactly where it was. Cuts are matched by height, because
`sserifee.fon` disagrees with the rest about the point size of three of its
six strikes, and a cut whose baseline sits elsewhere at a matching height, or
whose height no other cut has, is left out rather than drawn off the line.

Hebrew, Arabic and Thai are cut too, as `ssee1255.fon`, `ssee1256.fon` and
`ssee874.fon`, and are deliberately left out. Each needs layout the engine does
not do: Hebrew needs the bidirectional algorithm, Arabic needs that and
contextual joining, and Thai needs its marks stacked over the base rather than
advanced past. Their glyphs alone would draw in the wrong order, disconnected,
or beside the letter they belong over, which is worse than the question mark
they draw as now. The 120 dpi cuts are left out as well: their heights are a
different set and only the 13 and 16 pixel strikes are ever asked for.

A strike is drawn at the size it was cut, so the family answers only where the
frame is at one to one. At any other scale `dlg-sans` falls through to
`micross.ttf`, Microsoft Sans Serif, which is the TrueType successor to this
same face and is registered for the family whether the strikes answered or
not. `BitmapSystemFont` under `[Options]` turns the strikes off altogether and
leaves the outline face answering at every size; it defaults to `yes`. The
shell starts before the settings are read, so it asks again before each screen
shows rather than settling the question once.

A strike has one size, so a document names the height of the one it wants: the
layer asks GDI for a character height of twelve for a list row and fourteen for
everything else, and GDI answers the first from the thirteen pixel strike and
the second from the sixteen, so a list row is `font-size: 13dp` and everything
else `16dp`. The frame's own scaling is divided back out before the strikes are
searched, so the height a document names is the height that is looked for
whatever the frame does.

A merged strike is too big for a fixed grid, so its glyphs are laid into rows
one strike tall and the atlas is sized to what they come to, which at the
largest strike is smaller than the sixteen by sixteen grid it replaced despite
carrying more than twice the characters. The magnification is brought back at
bake time where the frame would otherwise ask for an atlas past what a renderer
takes.

The layer drew a strike's cell at the top of the rect it was given rather than
on a line of its own, so every box drawing this face gives `line-height` the
strike's height: the cell then sits on the box's top and whatever the box has
over falls below it, where a taller line would halve the room and leave the
cell half a pixel out. A glyph is one bit, so one atlas of coverage serves
every color and the quads carry the color, where the shaded family needs an
atlas per color.

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
`⌊1.63y⌋` down, rounded rather than floored for a check box, and measures
`⌊1.5w⌋ + 1` by `⌊1.63h⌋ + 1`: `Resize_Dialog` adds the pixel to every
control it carries over. The two factors are `300/400` and `163/200` in
`windlg.cpp`, where a dialog unit is two pixels wide and two tall before the
scaling; `1.625` is close enough to agree on many controls and wrong on about
one row in six, so it is worth taking the factor from the code. A dialog's
own size takes no such pixel, and the options menu is not carried over at all.
A control's width is its outer width, borders included, because that is what
the template measures. A `dlgsys` caption is drawn from the top of its box
with no leading, so a caption's line height is the glyph's height and the box
carries the row's height; a check box label is the same. A disabled button's
wash covers the skin. The rule is close but not exact: the
game draws a few dialogs and controls a pixel from where it puts them, with no
pattern that the measurements support, so every screen is measured against its
Win32 dialog at 3840x2160 and its sheet follows the game, saying so where it
departs. The display list is a pixel shorter than its template; the front-end
sound dialog, the campaign chooser and the three saved-game dialogs are each a
pixel wider; the in-game options menu is a pixel shorter; the keyboard screen's
combo box and list sit a pixel lower and wider; the abort question's buttons
are a pixel narrower; the in-game menu's Save button sits a row lower than
the column around it, and its Internet arrangement's Load button does the same;
the skirmish setup is a pixel wider than its
template's unit and a half with the frame around its settings a pixel taller
still; the classic main menu and the random map generator are each a pixel
taller than the factor gives; the generator's preview frame is a pixel
wider and a pixel taller than its group box; and the generator's Map Height
caption and its Time of Day box each sit a row higher than the factor gives,
with its two size boxes a pixel wider. Five screens carry a second arrangement: the sound
options and the game controls each have a frontend template and an in-game one,
the in-game options menu has a campaign arrangement, a network one and an
Internet one with two bars under a group box, the random map generator has the
base game's, the expansion's and a tour territory's, the
saved-game screen has one for each of loading, saving and deleting, all chosen
by a class the document sets from its model, and the message box places its
buttons in the three slots its template holds, which come out as a row spread
across the content. Where a screen departs from its template it says so in its own
sheet: the game controls make room for a difficulty row in a game, which no
template of the game's own offers.

The out-of-sync screen is the one that does not take its template's size. At
the 540 by 430 of `IDD_DESYNC_HOST` and `IDD_DESYNC_WAIT` the layer cuts the
players caption and the load button off under the frame and runs the last line
of prose over the paragraph above it, so the screen is laid out in the 640 by
391 frame the lobbies and the skirmish setup use, which is wide enough for the
prose to wrap inside its own column. Two things follow. The prose is four
paragraphs parted by less than a line rather than four controls at fixed
places, so the last of them cannot land on the one above it however long a
translation runs; and the countdown to a load has no row of its own, taking
the room the two decisions give up, since both are disabled by the time it
runs. The player list keeps the layer's three columns at the layer's widths,
marker, name and standing, with the six pixels it leaves between the last two,
and a name too long for its column is cut off there as the layer cut it.

The chrome is `Draw_Dialog_Back`'s composition: the 640 by 400 wallpaper
centered on the frame in whole pixels and cut off at the dialog's edges, black
beyond it; a 24-wide bar tiled down each edge with a corner over each end; and
a glow inside the bars that the original drew as sixteen one-pixel rings of
white, alpha 96 falling by 6 a ring. The glow is the one piece of art the kit
ships, `ui/glow.png`, because sixteen bordered boxes each round to the pixel
grid on their own at any scale but one to one, and gap and double up. Every
other picture is the player's own and is not shipped; each control has a plain
form underneath, a fill where a picture would be, so a screen stays usable
without it, and that form is what the harness renders.

The controls carry the drawing code's own metrics, and every rule in
`kit.rcss` records the measurement it came from, so the geometry is documented
beside the CSS rather than here. What reaches a screen's own sheet:

- `OD_Draw_Rect` draws a frame a pixel outside the control's rect, so a framed
  control is sized to the rect its template gives and its frame hangs over a
  negative margin; a screen spaces one from its neighbour on the neighbour's
  side. A list and a scroll bar are the exception, because the layer insets
  their rows by the frame and draws it on the rect's own edge, so a list is
  its template's rect, frame and all.
- A track bar shows its value in a fifty wide trough unless the dialog turned
  that off, which the game controls do and the sound options do not, so a
  document puts the trough after the bar and gives the bar the rest. Where a
  template gives its bar fewer rows than the trough's picture has, as the
  network lobbies do, the trough is placed over the bar's own last fifty
  columns and sorted behind it, because the layer drew the frame over the
  field rather than beside it.
- A disabled control takes a half-black wash. A caption cannot take one, since
  text draws after every decorator, so it carries the color the wash would
  have left it at; sheet text comes out a step darker again, because the
  sheets shade a glyph rather than filling it.
- A screen's sheet that still needs a visual rule of its own means the kit is
  missing a control.

Two things the kit cannot hold are the view's. A list's scroll-bar grip is as
long as the layer made it, the travel less a fifth of it for each natural-log
step of the rows left over and never under fourteen, which the view sets each
pass in place of the share of the rows in view RmlUi would give it. The
wallpaper is placed against the dialog's middle for the same reason: it is one
picture on the frame rather than on the screen.

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

One screen opens at a time. Only the screen a band belongs to can finish it, so a
screen raised over one still opening puts that one out whole first and takes the
band with it when it closes. The dialogs never interleaved two either: their
reveal was a loop that ran to its end before anything else was shown.

### Strings

Engine strings are UTF-8 through the process active code page declared in
`sun.manifest`. Every narrow Win32 API, including
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

One key in `SUN.INI` under `[Options]`. `BitmapSystemFont` decides whether the
dialog face may be drawn from the bitmap strikes at all, and defaults to
`yes`; it is read once at startup and has no screen of its own. `Options`
reads and writes it with its other `[Options]` settings, and it has a manual
page. There is no build option: RmlUi and ImGui are always compiled and
linked, so one configuration matrix carries the evidence. A sidebar view key
follows the sidebar view.

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

The sidebar is scheduled now that the Win32 dialogs are gone. Two pieces of
work exist, in order: a toolkit-neutral split of `SidebarClass` into model
and view with the gadget view as the only view, and later an RmlUi view
covering the whole sidebar column (radar, credits, power, strips, and mode
buttons) that a player selects instead of the gadget view. No presentation
bridge and no tooltip adapter are built, because no legacy dialog is left to
coexist with.

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
presentation cannot lose one. `ProgressScreenClass::Advance_Milestone` notes a
threshold crossing and plays its sound when the progress moves, and the paint
draws the text still owed.
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
| Gameplay | Command meaning, eligibility, ordering, and side-effect ownership preserved; presenters use the same calls the dialog procedures used. |
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
| The shell's window message hook | The structural item. Messages become a neutral event at the platform edge, which rewrites one signature and the body behind it |
| The window handle on `UIShellHostClass` | Three uses: two identity comparisons and the clipboard's owner. An opaque handle would serve, and `uihost.h` would stop pulling `win.h` into everything that includes it |
| Key mapping in `code/ui/rml/rmlkeys.cpp` | Not only code. `KEYBOARD.INI` stores Windows virtual key numbers, so the mapping is also a data-format boundary |
| The clipboard in `rmlsystem.cpp` and the conversions in `uiunicode.cpp` | Replaceable in place; `tests/uishell` already covers the behavior |
| The developer overlay's input entry points | They follow whatever event type the hook adopts |

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

## What is left

The migration went one screen per change and bottom-up, since the coexistence
rule meant a screen could migrate only after every screen it could open had.
Three things it did not reach:

- The sidebar, which [Sidebar](#sidebar) covers. It is scheduled now that the
  dialogs are gone.
- The other GadgetClass screens, the MSEngine screens and the credits, which
  are unscheduled and keep their own drawing and their own loops beside the
  shell, as [Where the UI stands today](#where-the-ui-stands-today) records.
- `IDD_EXCEPTION`. The crash reporter shows it after `Release_Display`
  through `DialogBoxParam` rather than through the dialog layer, so it stays
  Win32 and keeps working when the thing it reports on is the renderer.

Further ImGui overlays are added as a developer wants them; the frame
benchmark window is the only one so far.

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
  known element, and assert the queued intent and result.
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

The network lobbies were driven on 14 September 2026 by two copies of the game
on one machine, each bound to its own port: one hosted, the other found the
game, joined it, was given a free color because its own was taken, accepted
the host's settings, and the two started a game that ran in step on the same
seed. The host reached the map dialog from its lobby and came back with the
mission it chose.

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
| Skirmish setup, through to a game that deploys its units | yes | not yet |
| Multiplayer map dialog, opened from the skirmish setup and from a host lobby | yes | not yet |
| Network browser, host lobby and guest lobby, between two copies | yes | not yet |
| Main menu, game type and multiplayer game, with the graphic menu hooked out | yes | not yet |
| Random map generator, opened from the map dialog, in the base game's arrangement and a tour territory's | yes | not yet |
| In-game options menu, the campaign arrangement and the Internet one | yes | not yet |
| Out-of-sync screen, the master's form and the waiting one, opened on a key from a skirmish | yes | not yet |
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

The tour's arrangement of the generator was measured through a temporary hook
that hands the generator a fabricated territory, because the tour itself
cannot be reached.

Still owed: every screen whose behavior changed after that pass, which is the
five hide-parent flows and the screens the review fixes touched; the progress
document, which belongs to the multiplayer loading, map generation, and file
transfer paths; the multiplayer cases where `Main_Loop` runs under a message
box; a map carrying no preview of its own; the lobby paths the two-copy run
did not reach, which are the kick button, a rejected join, a host that
disbands its game, and more than two players; the out-of-sync screen and the
frame-sync notice under a session that has really gone out of step or stalled,
which needs the impaired-link rig rather than the key the screen was
photographed through, and with them the countdown to a load, which no
photographed run reached; the generator's own save, load and delete; and, for
each screen, what the paragraph above requires of its own change.

## Documentation

- This page owns the architecture.
- `docs/BUILDING.md` lists the submodules, the harnesses, and where the `ui/`
  directory lands beside the executable.
- `THIRD_PARTY_NOTICES.md` and the packaging license copy carry the three
  projects.
- The manual's systems page for the UI files owns where they live, the
  override order and the string reference syntax; one change record covers
  the replacement. The sidebar page changes when its view key lands.

## Open decisions

- The in-game text route for the sidebar view: TrueType conversions of the
  game fonts or a bitmap font engine for every document.
- The document and binding versioning rules for mods, fixed with the first
  supported override package.
