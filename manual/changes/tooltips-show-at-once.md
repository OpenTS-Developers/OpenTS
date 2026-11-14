---
title: Show the next tooltip at once, beside its control
category: feature
release: 0.2.0
targets:
- type: key
  id: ToolTips
  effect: changed
credit:
- ZivDero
---

Once a tooltip has been shown, moving onto another cameo or control shows its tooltip at once instead of after another half-second rest. A tooltip now stays up while the pointer moves within its control. After a click or the 10-second limit takes a tooltip down, it stays hidden until the pointer leaves the control.

A cameo's or other control's tooltip now sits beside the control, toward the middle of the screen, instead of below the pointer, so it no longer covers the next cameo down. A short tooltip is centered on the control, and a tall one starts level with its top. The power bar's tooltip is level with the pointer. Tooltips over the battlefield keep the old timing and placement.

Long tooltip text now wraps at a fixed width that `gametip.rcss` sets, instead of the width of the sidebar.
