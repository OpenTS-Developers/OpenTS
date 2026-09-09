---
title: Add the LegacyDialogs setting
category: feature
release: 0.2.0
targets:
- type: key
  id: LegacyDialogs
  effect: added
credit:
- ZivDero
---

`sun.ini` gains `LegacyDialogs` under `[Options]`. Set to `yes`, it opens the Win32 dialog for every screen that also has an RmlUi document; left out or set to `no`, those screens use their documents. The key is read at startup with the other options and written back when the settings are saved.
