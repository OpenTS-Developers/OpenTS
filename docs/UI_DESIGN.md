# UI system design

Status: proposal. Nothing here is implemented, built, or measured. Source
inspection and upstream documentation inform it. This page owns the proposed
UI architecture and migration; [Building OpenTS](BUILDING.md) owns build
support and [Project direction](DIRECTION.md) the wider architecture.

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
through a pointer stored in `DWL_USER` by its `WM_COMMAND` handler and read by
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
- No RmlUi render effects (filters, layers, shaders, box shadows) and no
  clip mask in the first renderer. The renderer implements the eight
  required methods; the rest stays default until a screen needs it.
- No arbitrary layering of native and GPU UI. The coexistence rule under
  [Input and focus](#input-and-focus) is the whole policy.
- No user UI scale setting yet. Documents follow the frame scale.
- The exception and assertion dialogs stay plain Win32. They must work when
  the renderer is the thing that failed.

## Architecture

Three parts, from the bottom up.

The **UI shell** is one module that owns the RmlUi context, the ImGui
context, the bgfx overlay pass, the input hook, and the modal runner. It is
the only code that includes RmlUi or ImGui headers, the way `bgfxbackend.cpp`
is the only code that includes bgfx.

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
- Renderer handles stay inside `code/ui/uirender.cpp`.
- The shell knows which presentation owns a region and an input scope. It
  does not know production rules, save semantics, or option behavior.
- Existing callers keep their screen functions; composition sits behind
  them.

A read-only screen needs a data builder and a close result. A presenter with
actions is added only where a screen has real state transitions.

### Code layout

New files live in `code/ui/`. The recursive glob in `code/CMakeLists.txt`
picks them up, and the directory lets the RmlUi, ImGui, and bgfx include
paths be scoped to the files that need them, as `bgfxbackend.cpp` is scoped
today.

| File | Holds |
| --- | --- |
| `uishell.h`, `uishell.cpp` | init and shutdown, resize, input hook, tick, overlay render entry, modal runner, selector |
| `uirender.cpp` | RmlUi render interface and the ImGui renderer on bgfx; the only UI file that includes bgfx |
| `uisystem.cpp` | RmlUi system interface: time, logging to `DebugString`, cursor, clipboard, string translation |
| `uifile.cpp` | RmlUi file interface over `CCFileClass` |
| `uitexture.cpp` | image decoding, SHP and PCX conversion, surface-backed textures |
| `uiscreen.h`, `uirmlview.h` | presenter, intent, and result contracts; the RmlUi view base |
| `uidev.cpp` | ImGui context and developer overlays |
| one file per screen | presenter, view-model binding, and the RmlUi view glue |

Shipped UI files (documents, styles, images, the font) live in `ui/` at the
repository root. The build copies the tree beside the executable as it copies
`Language.dll`, and the client package ships it.

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
methods:

| Capability | Behavior |
| --- | --- |
| Compiled geometry | Static vertex and index buffers, since RmlUi 6 compiles geometry once and re-submits it; order preserved; released on request; never dependent on transient memory from a previous frame. |
| Textures | RGBA8, premultiplied alpha as the interface specifies, created and released explicitly, cached by source string, sized for the 32-bit process. |
| Blending | `ONE, INV_SRC_ALPHA`; vertex colors follow the same premultiplied contract with no double premultiplication. |
| Scissor | `bgfx::setScissor` in physical target coordinates, intersected with the viewport, empty regions handled. |
| Projection | The overlay view's orthographic transform; no game-image filter state inherited. |
| Reset and resize | Target-dependent resources recreated, viewport and scissor refreshed, a full redraw requested; existing documents redraw without reload. |

The program is the embedded imgui vertex and fragment shader that
`bgfxbackend.cpp` already carries. Its attributes (position, texture
coordinate, color) match RmlUi's vertex and ImGui's vertex, each with its own
layout. Clip masks, transforms, layers, filters, and shaders are deferred;
shipped documents stay within a declared profile (text, images, ordinary
layout, borders, basic decorators), and a document check enforces it.

### Invalidation

`Video_Present_If_Dirty` grows a second dirty flag for the overlay: a present
happens when either flag is set, but the texture upload happens only when the
frame is dirty. RmlUi has no "needs redraw" query, so the shell marks the
overlay dirty on every tick that a document is visible or an ImGui window is
open, and the present pacing caps the rate. Closing or hiding a document also
marks the overlay dirty so its pixels disappear. A visible menu at 4K then
costs a few draw calls per refresh, not a 16 MB upload. Invalidation raised
during a present is kept for the next one rather than cleared with the
current frame.

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
before `Map.Message_Handler`:

```cpp
if (UI_Handle_Window_Message(hwnd, message, wParam, lParam)) {
    return(0);
}
```

Placing it after the routing keeps legacy child windows working under video
scaling; placing it before the keyboard handler keeps consumed input out of
the `KN_` queue. The hook covers mouse, wheel, key, and text messages only.
Activation, size, paint, transport, and system messages continue on their
paths. Forwarded or re-targeted messages are delivered to a toolkit once.

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
   game keeps tracking the cursor. A button or wheel message is consumed when
   RmlUi reports that the mouse is interacting with an element (its mouse
   functions return `false` for that). Keys and text are consumed when an
   element stopped their propagation, or whenever the focused element is a
   text field. Documents that float over the game mark their body
   `pointer-events: none` so empty space passes through.
4. A button press that a toolkit consumed sets mouse capture on `MainWindow`
   until the release, and the owner of a press owns its release: crossing a
   region or opening a modal in between completes or cancels that gesture
   without activating the newly focused screen.

Gameplay code that polls `Down` still sees held keys; eligibility is applied
at the consumers, `GScreenClass::Input` and the gadget and scroll paths, not
by falsifying physical state.

### Focus, cursor, clipboard, text

The shell clears the keyboard queue when a modal document opens or closes,
after marking the screen closing so the pump inside `Keyboard->Clear()`
cannot re-enter it. Focus loss cancels capture, drags, and composition;
focus return does not replay held keys as presses. Cursor requests from RmlUi
(`pointer`, `text`) map to `Win_Cursor_Set` and the previous request is
restored on close. The clipboard interface uses the Win32 clipboard.

Text input arrives as `WM_CHAR` with surrogate pairs joined. Consuming a
physical key never suppresses the text message it generates. Editable
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

class UIRmlViewClass {                            // uirmlview.h: owns the document
    public:
        UIRmlViewClass(UIPresenterClass & presenter, char const * document);
        virtual void Bind(Rml::DataModelConstructor & model) = 0;   // view-model fields and events
        virtual void Sync(void) = 0;                                // dirty what Execute changed
};
```

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
opens the legacy view where one exists. After activation, a view failure
recreates presentation against the surviving presenter state and never
replays accepted intents.

## Scheduling

Three kinds of work keep their owners: game behavior and command production
run at their existing call points with their existing gates; toolkit input,
layout, and animation run at the shell's service points on the application
thread; GPU submission runs in the presenter. UI animation uses wall-clock
time and never reads or advances deterministic game timers.

A migrated dialog driver keeps its shape. `Run_Modal` is the RmlUi twin of
the `Dialog_Message_Handler` loop:

```cpp
UIResult UI_Run_Modal(UIScreen & screen);
// each pass:
//   Windows_Message_Handler();
//   Main_Loop() in a network session, else Call_Back();   -- same test as today
//   context->Update();
//   execute the screen's queued intents;
//   mark the overlay dirty, Video_Present_If_Dirty();
// until the screen has a result or Main_Loop reports the game ended.
```

The result carries the game-ended flag the way `Dialog_Message_Handler`
returns `true`, so callers keep their logic. Wrappers keep their service
paths: the main menu keeps title-screen maintenance, an in-game screen keeps
the guarded multiplayer pump, lobby and loading flows keep their own work.

Event handlers never act directly. A toolkit event queues an intent, and the
runner executes the queue after `Context::Update` returns. RmlUi gives no
guarantee about re-entering `Update` from its own event dispatch, so a nested
modal (options opening a message box) starts from the queue, one level up,
where `Run_Modal` nests cleanly; and the legacy code already works this way,
`WM_COMMAND` writing `rc` for the driver to act on after the pump. A modal
document is shown with RmlUi's modal flag, which keeps other documents from
taking focus; blocking the game's input is the shell's job through the hook.
Paint handlers and the pump never drain intents, advance game logic, or
update the context; a nested update or present request is recorded and
served at the next safe point.

Non-modal documents are updated by a `UI_Tick` call in `Main_Loop` next to
`Map.Input` and rendered by every present.

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
`ui/` directory or from a mix. The run directory's `ui/` is added to the
`CDFileClass` search paths; the existing order then applies: user path,
current directory, search paths, mix files. A mod overrides a document by
placing a file earlier in that order or by shipping it in a mix. The `ui/`
directory on disk is a packaging convenience, not part of the lookup key.
The adapter validates sizes, reads, and seeks; RmlUi uses `size_t` where the
engine uses `int`, and a clamped seek must not look like success. A missing
required document, style, or font fails preparation with the name reported.

### Images

Images resolve by extension. PNG and TGA decode through `bimg_decode`, which
is already vendored and needs only linking. PCX goes through `Read_PCX_File`
with the palette named in the source string. SHP frames use a
`name.shp#frame` form with an optional palette, decoded to RGBA with index
zero transparent. Surfaces the engine draws at runtime (the map preview, the
desync host icons, a progress bar) reach a document through a `<surface>`
custom element bound to a named provider; the shell re-uploads the texture
when the provider marks it dirty. Original game art stays local runtime data
outside version control; documents receive artwork identities, never engine
pointers.

### Fonts

Fonts use RmlUi's FreeType engine with an OFL sans-serif shipped in `ui/`.
The legacy dialogs already draw with a system TrueType face, so this changes
nothing about their look. RmlUi uses one font engine per process, installed
with `SetFontEngineInterface` before `Rml::Initialise`, and the built-in
engine is not reachable from a custom one. In-game text that must match the
bitmap fonts, needed only by the post-migration sidebar view, has two routes:
convert the game's `.fnt` faces to TrueType at build time, or write a bitmap
engine over `WWFontClass` data as RmlUi's `bitmap_font` sample does and
commit every document to bitmap faces. That choice waits for that view.

### Strings

Engine strings become UTF-8 through the process active code page declared
in `sun.manifest`, a separate change that is a prerequisite for every RmlUi
screen that shows text. With it, every narrow Win32 API, including the
`LoadString` behind `Fetch_String`, yields UTF-8 bytes, and the shell copies
a string out of the `Fetch_String` cache and hands it to RmlUi unchanged.
Text typed into a field goes into engine buffers unchanged. What the
transition does not remove: fixed-size engine buffers, packet fields, and
file names are sized in bytes, so a field's character limit is a byte limit
and truncation never splits a sequence; and `WWFontClass` indexes glyphs by
byte, which bounds in-game text to the range the transition supports.

Documents reference strings by name: `[[TXT_OK]]`. RmlUi passes every text
node through `SystemInterface::TranslateString`, where the shell maps the
name to its identifier. The names are `#define`s in `language.h`, so a CMake
script generates the name table into the build's generated directory; no
hand-maintained list. Dynamic text, including player and map names and error
strings, is inserted as text, never as markup.

## Configuration

One transitional key in `SUN.INI`, named by the change that introduces it,
returns every migrated screen to its legacy view while that view exists.
Defaults are decided per screen family in code, so a family switches to RmlUi
by default when its evidence is in without a key per family. The key is
deleted with OwnerDraw. There is no build option: RmlUi and ImGui are always
compiled and linked, so one configuration matrix carries the evidence.
`Options` reads and writes the key where it handles `[Video]` today, and the
key gets a manual page. A sidebar view key follows the sidebar view.

## Dear ImGui

ImGui is vendored as a submodule, compiled into Debug and Release, and
rendered by a small bgfx adapter in `uirender.cpp` that reuses the same
program and view setup as the RmlUi renderer, on `VIEW_DEV`. Its platform
adapter feeds it input through the shell hook and follows the pinned
version's backend contract for texture creation and destruction. Overlays are
armed by the developer-mode flags the manual documents; tool visibility and
frame rate never touch deterministic state. The first uses are single-window
diagnostics such as frame benchmarks, network statistics, and object and
house inspectors. A player-facing feature may choose an ImGui view through
the same screen contract; it then meets the same coexistence, input, and
evidence rules as an RmlUi view. Docking, extra native viewports, and editor
architecture are separate work.

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
presentation cannot lose one. The screen exposes phase, progress, status, and
the operations the loader supports; no cancellation is added to a loader that
cannot cancel. Loading stays on its thread with explicit cooperative service
points that drain nothing unrelated while scenario objects are being
replaced, and the first paint happens before long work begins.

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
| Build | 32-bit MSVC with the static CRT for every new dependency. |

## Dependencies

Additions, as submodules under `thirdparty/` pinned at tested tags like bgfx,
built static with the static CRT that `thirdparty/CMakeLists.txt` forces:

| Project | License | Notes |
| --- | --- | --- |
| RmlUi 6.x | MIT | `RMLUI_FONT_ENGINE=freetype`, no samples, no backends, static |
| FreeType 2.13 | FTL | bzip2, PNG, HarfBuzz, and Brotli disabled; aliased as `Freetype::Freetype` for RmlUi's find |
| Dear ImGui | MIT | core sources compiled into a small target; no bundled backends |

`THIRD_PARTY_NOTICES.md`, `thirdparty/licenses/`, and the packaging license
copy grow by the same three entries. CI already checks out submodules
recursively. The build stamp step gains the string-name generator, and
`bimg_decode` loses `EXCLUDE_FROM_ALL` and is linked. Dependency upgrades are
separate changes.

## Migration plan

Each step is one pull request unless noted, builds and runs on its own, and
leaves the game playable. A behavior-heavy screen takes two changes: the
first extracts its behavior behind the presenter with the legacy view still
selected and classifies as preserved; the second adds the RmlUi view. A leaf
takes one. Sizes are rough: S under a day of focused work, M a few days, L a
week or more. The order is bottom-up because of the coexistence rule: a
screen migrates only after every screen it can open has migrated.

Prerequisite: the UTF-8 transition lands before step 3. Steps 1 and 2 need no
text beyond an ASCII test document.

1. **Dependencies** (S). Submodules, CMake, notices, `BUILDING.md`. No engine
   code uses them. Evidence: Debug and Release build.
2. **Shell** (M). Everything in the code-layout table except screens, the
   backend split, the input hook, resize handling, the `ui/` copy step, the
   file interface with mix resolution, and a Debug-only test document toggled
   by a developer key. Evidence: the test document renders over the main menu
   and in game at several resolutions and scale modes; clicks on it are
   consumed; clicks beside it reach the game; legacy dialogs still open and
   close; repeated open and close leaks nothing.
3. **Version dialog** (S, leaf). The integration pilot: fonts, clipping,
   mapping, dismissal by mouse and keyboard, focus return, UI-only redraw,
   resize, preparation failure. The main menu keeps hiding around it.
4. **Modal runner and message boxes** (M, leaf). `WWMessageBox::Process` and
   `OwnerDraw::Custom_Message_Box` behind the kill switch, preserving button
   order, default button, Escape, the no-button case, return mappings, and
   session-end interruption. Evidence includes the multiplayer cases where
   `Main_Loop` runs under the box.
5. **Sound** (M, two changes). The behavior pilot: volumes, eligible themes,
   selection, availability, shuffle and repeat, immediate previews, play and
   stop, both templates, frontend and in-game service paths.
6. **Progress and wait** (S, leaf). `IDD_PROGRESS_WAIT`, the saving and
   loading boxes in `savemgr.cpp`, the `<surface>` element, milestone effects
   moved out of drawing.
7. **Options family** (L, two changes each). Main options, display with its
   timed rollback, game controls (three variants), keyboard with the hotkey
   capture control, the display-mode confirmation, abort and surrender.
   Evidence: settings round-trip through `SUN.INI` unchanged.
8. **Main menu family** (M). `IDD_MAIN_MENU`, campaign choice, game type,
   multiplayer game selection. The `NewMenuClass` drivers keep their loops.
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

ImGui overlays (S each) can follow step 2: frame benchmarks first, then what
a developer needs next. GadgetClass screens, MSEngine screens, and the
credits are unscheduled.

## Validation and evidence

A `tests/uishell` CTest target links RmlUi core, FreeType, `uiscreen.h`, the
string table, and the screen presenters with a recording render interface
and a null system interface. It runs without game assets:

- Load every shipped document and fail on a parse error or a property
  outside the declared profile.
- Bind a presenter, drive it with `Context::ProcessMouseButtonDown` on a
  known element, and assert the queued intent and result; drive the same
  actions through the legacy adapter and assert the same ordered service
  calls.
- Reject stale intents after close, suspension, scenario generation change,
  and catalog removal.
- Scan shipped documents for `[[TXT_*]]` names and check each exists in the
  generated table.
- Round-trip the coordinate mapping at integer and fractional scales, with
  letterboxing, resize, outside input, and captured release.

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

## Documentation

- This page owns the architecture and is updated as steps land.
- `docs/BUILDING.md` lists the new submodules.
- `THIRD_PARTY_NOTICES.md` and the packaging license copy gain the three
  projects.
- The manual gains a systems page for the UI files (where they live, the
  override order, the string reference syntax), a key page for the kill
  switch, and a change record per migrated screen. The sidebar page changes
  when its view key lands.

## Open decisions

- The shipped font.
- The kill-switch key name, fixed by the change that introduces it.
- The in-game text route for the sidebar view: TrueType conversions of the
  game fonts or a bitmap font engine for every document.
- The document and binding versioning rules for mods, fixed with the first
  supported override package.
