---
title: Survive a variable change while a tag has lost its trigger
category: fix
release: 0.2.0
targets:
- type: system
  id: trigger-springing
  effect: changed
credit: [gibbo101]
---

A tag whose trigger has been destroyed keeps a null trigger when the destroyed one had no linked trigger to take its place. A trigger action that set a local or global variable then restarted every tag's timed events through that null trigger and crashed the game. The reset now skips a tag without a trigger, so the action completes and the other tags' timers restart as before.
