---
title: Repair the Direct3D 11 swap-chain fallback
category: fix
release: 0.3.0
targets:
- type: key
  id: Renderer
  effect: changed
credit:
- ZivDero
- jampai
---

When the first Direct3D 11 swap chain fails, the game retries with a scaling mode that Windows 7 supports. This retry can let the game start with Direct3D 11 on Windows 7 with the Platform Update installed.
