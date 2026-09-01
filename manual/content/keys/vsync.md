---
key: VSync
summary: Whether the renderer waits for the display's refresh before showing a finished frame.
when_omitted:
  kind: value
  value: "no"
---

The game already presents no more often than the display refreshes, whatever this setting says: it measures the refresh rate and skips a present that would arrive early, leaving the frame marked so the next one carries the newest picture. Switching this on additionally makes each present *wait* for the display, which is what removes the torn seam that can appear across a scrolling map.

The wait has a cost. A present that blocks holds up the game loop that asked for it, and that shows up as added lag between moving the mouse and seeing the result. The pointer itself is unaffected either way, because the system draws it over the game's picture rather than in it.

The setting is read before the renderer starts and is applied when it does, so a change to it takes effect the next time the game is launched.
