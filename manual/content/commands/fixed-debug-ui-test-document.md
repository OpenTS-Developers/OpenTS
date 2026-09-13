---
command_id: fixed:debug-ui-test-document
---

The key is read from the message pump before any dialog sees it, so it works while a menu dialog or one of its controls has focus as well as in play, and the key release is swallowed with the press so the game never sees either. The first press loads `test.rml` from the `ui` directory beside the executable and shows it; each later press hides or shows the same document. The document is a panel in the top left corner of the frame with a button that hides it, drawn by the renderer over the presented frame at the window's resolution and following the frame's position and scale.

A click on the panel is consumed before it reaches the game; a click on the frame beside it reaches the game as before. The panel is positioned away from the centered menu dialogs because a visible dialog takes the clicks over its own area first.

The document needs the shipped `OpenSans.ttf`. When the font failed to load at startup, the key writes a line to the debug log and shows nothing. Each load, show, and hide also logs the renderer's live texture and buffer counts.

[Developer mode and diagnostics](/systems/developer-mode/) covers the flag that arms the keys handled directly in code.
