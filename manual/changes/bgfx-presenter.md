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

The finished picture now reaches the screen through bgfx, which draws it with Direct3D, Vulkan or OpenGL depending on the machine, in place of DirectDraw. The game still renders every frame in software, so nothing about how it looks or plays depends on the graphics card.

A full-screen game no longer changes the desktop's resolution: it covers the screen with a borderless window and scales the picture into it, keeping its shape and adding black bars where the shapes differ. Switching away and back no longer disturbs the desktop, and a game that stops responding no longer leaves the display in its resolution. The mouse pointer is a real system cursor built from the game's own artwork rather than drawn into the frame.

The display options screen offers every resolution the display reports between 640 by 400 and 4096 by 4096, where `AllowHiResModes` and its `HIRES` cheat used to filter that list. The game no longer switches resolution between the menus and play, so `AllowModeToggle` and its `TOGGLE` cheat are gone as well. `VideoBackBuffer` was already unused.
