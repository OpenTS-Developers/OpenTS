---
key: LegacyDialogs
summary: Selects the Win32 dialog over the RmlUi document for the screens that have both.
when_omitted:
  kind: value
  value: "no"
---

`LegacyDialogs=yes` under `[Options]` returns every screen that has an RmlUi document to its Win32 dialog. No screen has one yet, so the key changes nothing until the first screen migrates.

The key is read with the other `[Options]` settings when the game starts and written back with them when the settings are saved, so a value written by hand survives the options dialogs.
