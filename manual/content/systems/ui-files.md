---
title: UI files
summary: Ships the RmlUi documents, style sheets, dialog kit and font in a `ui` directory beside the executable, resolves them by bare file name through the game's file system, and names engine strings in documents as `[[TXT_NAME]]`.
category: interface-controls
keys: []
related:
  - type: using
    id: game-data
  - type: format
    id: mix
---

The `ui` directory beside the executable holds the RmlUi documents (`.rml`), their style sheets (`.rcss`), the dialog kit `kit.rcss` with its template `dialog.rml` and its glow picture `glow.png`, and the Arimo font `Arimo.ttf` with its license `OFL.txt`. The build places the directory beside the executable, where `Language.dll` is built, and the release package carries it.

## The dialog kit

`kit.rcss` styles the controls the way the game's own dialogs draw them, and `dialog.rml` is the frame around a screen: the wallpaper, the side bars and the glow. A document links the kit first and its own style sheet after, so its own sheet only says where things go. The pictures are the game's own interface art, read from its mix files; where a picture is missing, the control keeps a plain fill in its place. Every size in the kit is in `dp`, so a screen matches the original dialog at the game's native size and scales with the frame.

A screen opens the way the original dialogs did: it slides out from the middle behind a pair of side bars, with the dialog sound once as it starts. A document that should open at once puts `reveal="none"` on its body.

## How a file is found

A document names every file it uses by bare file name, and the game adds the `ui` directory to its search paths at startup, so a name resolves in the same order as any other game file: the user path, the current directory, the search paths, then the mix files. A loose copy earlier in that order overrides the shipped file, and a copy inside a mix is used only when no loose file exists. A document, style sheet or font that fails to load fails the screen's preparation; the game logs the file name and the screen answers as if the player had backed out of it.

Which archives are mounted changes while the game runs: a side mounts its own when it is prepared and drops them when the side changes. The pictures and the dialog font are read again then, so a side's archives can carry their own interface art. [MIX archives](/formats/mix/) covers which archive answers for a name. An archive mounted at startup still answers first, so a side's copy serves for a name no earlier archive holds.

## Strings

A document names an engine string as `[[TXT_NAME]]`, using the identifier names of the language library. The game replaces the reference with the string of that name as it lays the text out. An unknown name stays as typed, so the mistake shows on screen, and a Debug build logs it.

## Which screens are documents

Every screen the game shows outside a mission is a document: the main menu and the menus under it, the campaign and multiplayer choosers, the options menu with the sound, display, game control and keyboard screens, the skirmish setup and the map dialog, the random map generator, the network lobbies, the load, save and delete lists, the in-game options and the abort question, the out-of-sync screen and the notice a stalled match waits behind, the message boxes, and the notices shown while a game saves or loads. The Win32 dialogs they replaced are gone.

The score screens, the graphic menus, the mission briefing and the sidebar are drawn by the game's older systems and are not documents.
