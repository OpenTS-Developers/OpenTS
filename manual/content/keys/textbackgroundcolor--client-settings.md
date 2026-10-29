---
key: TextBackgroundColor
scope: client-settings
label: Chat text background
see_also: [MessageDelay, IncomingMessage]
when_omitted:
  kind: value
  value: "0"
---

`TextBackgroundColor` is the palette index drawn behind every character of the in-game message list and of the line being typed. `0` draws no background. `12` is black. A missing key is `0`. [In-game chat](/systems/chat/) describes the message list.

Any other index is drawn as written. A saved `SUN.INI` that still has `TextBackgroundColor=12` keeps that black box. Set the key to `0` for no background.

No dialog offers the setting. Leaving the options menu writes the index in force, so a value set by hand is kept.

```ini title="sun.ini"
[Options]
TextBackgroundColor=0
```
