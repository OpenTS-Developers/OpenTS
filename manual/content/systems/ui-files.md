---
title: UI files
summary: Holds the RmlUi documents, style sheets, dialog kit and font that draw the game's screens, in a `ui` directory beside the executable.
category: interface-controls
keys: []
related:
  - type: using
    id: game-data
  - type: format
    id: mix
---

The `ui` directory sits beside the executable and holds everything the screens are built from. The build places it where `Language.dll` is written, and the release package carries it.

| File | Role |
| --- | --- |
| `<screen>.rml` | One document per screen, named after it, such as `sound.rml` |
| `<screen>.rcss` | That document's own style sheet |
| `dialog.rml` | The template every document opens inside |
| `kit.rcss` | The dialog kit, which styles the controls |
| `side-<name>.rcss` | Rules every screen takes while that side is the player's, such as `side-gdi.rcss` and `side-nod.rcss` |
| `glow.png` | The glow the template draws around a screen |
| `cursor.png` | The arrow pointer shown over the screens, with its tip at the top-left pixel. [`SystemCursor`](/keys/systemcursor/) replaces it with the Windows pointer |
| `icon.png` | Optional replacement for the running game's window icon |
| `Arimo.ttf` | The face a font family uses when its own face is missing, with its license in `OFL.txt` |

## The dialog kit

`kit.rcss` styles the controls the way the game's dialogs drew them, and `dialog.rml` is the frame around a screen: the wallpaper, the side bars and the glow. A document names `dialog.rml` as its template, which brings in the kit, and also links its own style sheet. The shipped screen sheets only place their controls; how the controls look comes from the kit.

The pictures, and the dialog font used for captions, buttons and message-box text, are the game's interface art, read from its mix files. Where a picture is missing, the control draws a plain fill in its place.

`font-effect: shadow(...)` draws on every font, the dialog font and the dialogs' sans font included, so a style sheet can shadow text that sits over the wallpaper, as in `font-effect: shadow(1dp 1dp #000000);`. The other effects, `outline`, `glow` and `blur`, draw only where a TrueType face such as `Arimo.ttf` supplies the font; on the game's own fonts the text is drawn without them.

The kit gives every layout size in `dp`, RmlUi's scaled pixel, so a screen matches the dialog it replaced at the game's own resolution and grows with the game picture when that is drawn larger.

A screen opens the way the original dialogs did: it slides out from the middle behind a pair of side bars, with the dialog sound once as it starts. Put `reveal="none"` on a document's `<body>` for a screen that should appear at once instead. Of the screens shown beside a running game, only the [diplomacy panel](/systems/diplomacy/) slides in; the reconnect dialog and the save and load notices appear at once.

## Game fonts

Six font families draw text in the fonts the game prints with, read from the font files in its mix archives:

| Family | Font file | Height |
| --- | --- | --- |
| `wwfont-kia6pt` | `KIA6PT.FNT` | 9 |
| `wwfont-6point` | `6POINT.FNT` | 10 |
| `wwfont-8point` | `8POINT.FNT` | 11 |
| `wwfont-editfnt` | `EDITFNT.FNT` | 8 |
| `wwfont-grad6fnt` | `GRAD6FNT.FNT` | 16 |
| `wwfont-12metfnt` | `12METFNT.FNT` | 16 |

Set `font-size` to the height in `dp` to draw one font pixel per `dp`, such as `font-family: wwfont-kia6pt; font-size: 9dp;`. Letters are spaced as the game spaces them. A character the font has no glyph for draws as a question mark. If a font file is missing or cannot be read, its family is not available and the debug log names it.

Each pixel of a glyph is drawn as text, shadow, outline or shade. The text pixels take the `color` property. The shadow and outline pixels are not drawn unless a style sheet colors them. The shade pixels take the text color in `wwfont-editfnt`, `wwfont-grad6fnt` and `wwfont-12metfnt`, as the game prints those fonts, and are not drawn in the other families.

These font effects color the pixels of a game font:

| Effect | Colors |
| --- | --- |
| `ww-shadow(<color>)` | The shadow pixels |
| `ww-outline(<color>)` | The outline pixels |
| `ww-gradient(<color>)` | Every shade pixel |
| `ww-role(<index> <color>)` | The pixels of one pixel value from 2 to 15: 2 is the shadow, 3 the outline, and 4 to 15 the shades |

Where two effects color the same pixels, the later one in the list is used. For example, `font-effect: ww-shadow(#000000), ww-outline(#000000);` draws the font with a black shadow and outline. The ww- effects change nothing on any other font. A `shadow(...)` effect on a game font draws every visible pixel of the text in the shadow's color.

## Your own fonts

A style sheet loads a TrueType or OpenType font with an `@font-face` block, and any document can then name its family:

```css
@font-face {
	font-family: mod-title;
	src: modtitle.ttf;
}

#gametip { font-family: mod-title; font-size: 13dp; }
```

`src` names the font file by bare file name, and the file is [found like any other file here](#how-a-file-is-found). Without `font-weight`, every weight in the file is loaded. `font-weight: bold` or a number such as `600` loads the face as that weight, and `font-style: italic` loads it as italic. A family that has no face loaded draws no text, and a missing font file is named in the debug log.

A face is loaded when the first document that uses the style sheet opens, and it stays loaded until the game exits. Declare the face in every style sheet that uses it; a repeated declaration does not read the file again. A later declaration with the same family, weight and style keeps the first file's face, even when the player's side changes. To give each side its own font, give each side's face its own family name in that side's `side-<name>.rcss`.

The game's own family names cannot be declared: `wwfont-kia6pt`, `wwfont-6point`, `wwfont-8point`, `wwfont-editfnt`, `wwfont-grad6fnt`, `wwfont-12metfnt`, `dlgsys`, `dlg-sans` and `fullfnt`. A block that names one loads nothing, and the debug log records a warning.

## How a file is found

A document names every file it uses by bare file name, and the game adds the `ui` directory to its search folders at startup. Those names then resolve in [the order the game searches](/formats/opents-ini/#the-order-files-are-searched-for-in): a loose copy found earlier in that order wins, and a copy inside a mix archive is used only where no loose file of that name exists. To replace a shipped document, style sheet or picture, put a file of the same name in a folder the game reaches first.

If a required document, template, style sheet or `Arimo.ttf` fails to load, the screen does not open. Opening a menu or modal dialog shows a native error message with the loading failure and instructions to restore the `ui` folder supplied with the build. The debug log records the failure. The classic main menu exits after the message is dismissed; other menus return without a choice.

When a scenario is read or a saved game loads, the game mounts the archives of the player's side. During the match, a side's copy of a name is used over the copy in the game's own archives, but a loose file and a copy in a patch or expansion archive are used over the side's. A side's archives cannot replace a shipped document or style sheet, because those are loose files in the `ui` directory. When the match ends and the menus return, the side's archives move behind the archives mounted at startup, so the menus use those archives' copies again. [MIX archives](/formats/mix/#theater-side-and-speech-archives) gives the full order.

The game reads the pictures, the dialog font and the game fonts again when it mounts a side's archives and when a match ends. Every screen then draws with the copies the mounted archives supply, including a screen already on show, which keeps its layout and style sheets.

To give each side its own version of a picture or font, put a copy in each side's `SIDEC<nn>.MIX` or `SIDENC<nn>.MIX`, and keep the copy for the menus in an archive searched after them, such as `LOCAL.MIX`. Keep the name out of the `ui` directory and the patch and expansion archives; a copy there is used everywhere instead.

## Window icon

Put `icon.png` in the `ui` directory beside the executable to replace the running game's window and Alt+Tab icon. Use a square PNG with transparency, such as 256×256 pixels. No image is bundled, and no INI key is needed. The executable's icon in Explorer and pinned shortcut icons stay unchanged. The Debug Console keeps its own icon, and Windows may still show the built-in icon for a grouped taskbar button.

The game uses the first loose `icon.png` found in [its file search order](/formats/opents-ini/#the-order-files-are-searched-for-in), so a copy in a higher-priority folder overrides the one in `ui`. The icon loads before the window is shown and before MIX archives are mounted, so an archive cannot supply it.

A missing, unreadable or corrupt image keeps the embedded icon. A corrupt first match does not cause the game to try a later copy. Restart the game after replacing or removing the image.

## Game tooltips

`gametip.rml` draws the tooltips over the sidebar and the tactical view, and `gametip.rcss` styles them. The tooltip is the element with `id="gametip"`, drawn by default in `wwfont-kia6pt` in the colors and frame the game drew it in. A style sheet can change its font, colors, border and padding, and the width at which long text wraps.

The game places the box:

- A tooltip of a sidebar cameo, button or other control sits beside the control, on the side facing the middle of the screen. It is centered on the control's height, but a box taller than the control starts level with the control's top. Beside a control taller than it is wide, such as the power bar, it is level with where the pointer was when the tooltip opened, and stays alongside the control. It moves up when it would cross the bottom of the screen.
- A tooltip over the battlefield opens below the pointer and stays inside the battlefield. It moves left to fit, and moves above the pointer when there is no room below.

Text wraps at the box's `max-width`, which `gametip.rcss` sets to `360dp`, or at the width of the screen when that is narrower. A line break in the text or an at sign (`@`) starts a new line; [Line breaks in text](/formats/ini-syntax/#line-breaks-in-text) gives the escapes. The pointer passes through the tooltip to whatever is under it.

## Side style sheets

While a side is the player's, every screen also takes the rules in `side-<name>.rcss`, where `<name>` is the side's name from `[Sides]` in lower case. The sheet is applied after the screen's own style sheets, so its rule wins over a shipped rule with an equally specific selector. A side with no such file keeps the shipped look. The game ships `side-gdi.rcss` and `side-nod.rcss`, which color the objectives screen's text green and red.

The player's side is set when a scenario is read or a saved game loads. It is cleared when the match ends and the menus return, so no side sheet applies in the menus.

The sheet is found like any other file here, so a side's own archive can carry it. A rule in it reaches every screen with a matching element. To limit a rule to one screen, begin its selector with that screen's body, such as `body[data-model="restate"] .line.ink { color: #fc1c1c; }`.

## Strings

Write `[[TXT_NAME]]` in a document for an engine string, using an identifier name that `code/language/language.h` defines, such as `[[TXT_CANCEL]]`. The game substitutes the string as it lays the text out. An unknown name is left as typed, so the mistake shows on screen, and the debug log names it.

## Which screens are documents

These screens are documents:

- the [classic main menu](/systems/developer-mode/#the-main-menu-code-recognizer), the menus under it, and the version screen
- the campaign and multiplayer choosers
- the options menu, with its sound, display, game control and keyboard screens
- the skirmish setup, the map chooser and the random map generator
- the network lobbies
- the load, save and delete lists
- the in-game options, the abort question and the objectives screen
- the out-of-sync screen, the [reconnect dialog](/systems/reconnect-dialog/) and the [diplomacy panel](/systems/diplomacy/)
- the message boxes, and the notices shown while a game saves or loads
- the tooltips over the sidebar and the tactical view

The score screens, the graphical main menu, the campaign map and the sidebar are drawn by the game's older systems and are not documents. The crash report remains a Windows dialog.

The skirmish setup can open with an empty map list. Choose or generate a map before starting a game.
