---
title: Present the game through bgfx instead of DirectDraw
category: feature
release: 0.1.0
breaking: true
migration:
- Remove `-16` from any shortcut. The option is gone.
- Delete `AllowHiResModes`, `AllowModeToggle` and `VideoBackBuffer` from `sun.ini`, or leave them to be ignored. Size the window with `Fullscreen`, `WindowWidth` and `WindowHeight` instead.
targets:
- type: key
  id: Fullscreen
  effect: added
- type: key
  id: WindowWidth
  effect: added
- type: key
  id: WindowHeight
  effect: added
- type: key
  id: ScaleMode
  effect: added
- type: key
  id: IntegerScaling
  effect: added
- type: key
  id: VSync
  effect: added
- type: key
  id: Renderer
  effect: added
- type: key
  id: CursorScale
  effect: added
- type: key
  id: VideoBackBuffer
  effect: removed
- type: key
  id: AllowHiResModes
  effect: removed
- type: key
  id: AllowModeToggle
  effect: removed
- type: command
  id: launch:high-color
  effect: removed
credit: [ZivDero]
---

The finished picture reaches the screen through bgfx, which draws it with Direct3D, Vulkan, or OpenGL depending on the machine. The game still renders every frame in software exactly as it did, so nothing about how the game looks or plays depends on the graphics card; only the last step, getting that picture in front of the player, has changed. DirectDraw is gone from the engine entirely.

The game no longer changes the desktop's resolution. A full-screen game covers the screen with a borderless window, and the picture it renders is scaled into that window, keeping its shape and adding black bars where the shapes differ. Switching away from the game and back no longer disturbs the rest of the desktop, and a game that stops responding no longer leaves the display in its resolution.

The mouse pointer is now a real system cursor built from the game's own artwork, drawn over the picture by the system rather than into the frame.

The display options screen offers every resolution the display reports between 640 by 400 and 4096 by 4096. `AllowHiResModes` no longer filters that list and has been removed, along with the `HIRES` cheat that used to bypass it. `AllowModeToggle` and its `TOGGLE` cheat are also gone: the game no longer switches resolution between the menus and play, because the picture is scaled to the window instead. `VideoBackBuffer` was already unused.
