---
key: Renderer
summary: Which graphics interface the game draws through, as a number.
when_omitted:
  kind: value
  value: "0"
  note: Zero lets the game choose the interface.
---

The number selects the graphics interface the game asks for when it starts. It exists to work around a driver problem, so leave it at `0` unless a driver needs a specific interface.

| Value | Interface |
| --- | --- |
| `0` | Chosen automatically |
| `1` | Direct3D 11 |
| `2` | Direct3D 12 |
| `3` | Vulkan |
| `4` | OpenGL |

A value outside this range is treated as `0`. Direct3D 11 requires feature level 11.0 or higher. OpenGL requires version 4.3 or higher.

The [debug log](/using/debug-logging/) names the interface that started. Check it to confirm that a requested interface is in use.

If the game cannot start drawing, it reports that it is unable to set the video mode and closes. Set `Renderer` back to `0` in the settings file to undo a value that causes this.

A change takes effect at the next launch.
