---
key: Renderer
summary: Which graphics interface the game draws through, as a number.
when_omitted:
  kind: value
  value: "0"
  note: Zero lets the renderer choose the interface it considers best for the machine.
---

The game asks the graphics card for one of several interfaces. The setting exists so that a driver problem can be worked around without a new build:

| Value | Interface |
| --- | --- |
| `0` | Chosen automatically |
| `1` | Direct3D 11 |
| `2` | Direct3D 12 |
| `3` | Vulkan |
| `4` | OpenGL |

A value outside this range is treated as `0`. Naming an interface the machine cannot provide is a startup failure rather than a fallback: the game reports that it could not start the display and closes, so an interface that turns out to be missing has to be undone by editing the file again.

The setting is read before the renderer starts, so a change takes effect the next time the game is launched.
