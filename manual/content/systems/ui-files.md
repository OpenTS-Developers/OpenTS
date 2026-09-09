---
title: UI files
summary: Ships the RmlUi documents, style sheets and font in a `ui` directory beside the executable, resolves them by bare file name through the game's file system, and names engine strings in documents as `[[TXT_NAME]]`.
category: interface-controls
keys:
  - LegacyDialogs
related:
  - type: using
    id: game-data
  - type: format
    id: mix
---

The `ui` directory beside the executable holds the RmlUi documents (`.rml`), their style sheets (`.rcss`), and the Open Sans font `OpenSans.ttf` with its license `OFL.txt`. The build copies the directory beside the executable the way it copies `Language.dll`, and the release package carries it.

## How a file is found

A document names every file it uses by bare file name, and the game adds the `ui` directory to its search paths at startup, so a name resolves in the same order as any other game file: the user path, the current directory, the search paths, then the mix files. A loose copy earlier in that order overrides the shipped file, and a copy inside a mix is used only when no loose file exists. A document, style sheet or font that fails to load fails the screen's preparation; the game logs the file name and opens the screen's Win32 dialog instead.

## Strings

A document names an engine string as `[[TXT_NAME]]`, using the identifier names of the language library. The game replaces the reference with the string of that name as it lays the text out. An unknown name stays as typed, so the mistake shows on screen, and a Debug build logs it.

## Choosing the Win32 dialogs

[`LegacyDialogs`](/keys/legacydialogs/) under `[Options]` returns every screen that has a document to its Win32 dialog. The [version dialog](/systems/developer-mode/#the-version-dialog) and the game's message boxes are the screens with both. A message box raised while a Win32 dialog is on screen, as the options and network dialogs raise theirs, stays a Win32 box, because a visible dialog takes the mouse before a document can. The document keeps the Win32 box's layout: up to three buttons in the same slots, Enter answering with the default button and Escape with the second.
