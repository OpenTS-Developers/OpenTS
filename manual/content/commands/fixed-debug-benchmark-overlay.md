---
command_id: fixed:debug-benchmark-overlay
---

The key is read from the message pump before any dialog sees it, so it works while a menu dialog or one of its controls has focus as well as in play, and the key release is swallowed with the press so the game never sees either. The first press creates the Dear ImGui context and shows the frame benchmark window; each later press hides or shows it, and the window's own close button hides it too.

The window reports the logic frames and the presents of the last second, the frame number, the present interval, and the frame benchmarks the monochrome Events page shows, as a share of the frame and as an average in microseconds when the processor speed could be measured, in ticks otherwise. The five counters the engine never starts are marked. While the monochrome display is off, the window resets the benchmarks once a second and shows the second just gone; while that display is on, the Events page keeps its reset and the window shows the live running averages. A button resets on demand and a switch opens the Dear ImGui demo window.

The window takes the mouse only while the pointer is over it and the keyboard only while one of its fields has focus; everything beside it reaches the game and the test document. A visible menu dialog takes the mouse over its own area before the shell sees it, so over a dialog the window answers the pointer only where it covers the frame beside the dialog. Each toggle writes the renderer's live texture and buffer counts to the debug log.

[Developer mode and diagnostics](/systems/developer-mode/) covers the flag that arms the keys handled directly in code.
