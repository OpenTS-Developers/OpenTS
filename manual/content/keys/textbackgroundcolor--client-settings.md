---
key: TextBackgroundColor
scope: client-settings
label: Chat text background
see_also: [MessageDelay, IncomingMessage]
when_omitted:
  kind: value
  value: "0"
---

`TextBackgroundColor` is the palette index drawn behind every character of the in-game message list and of the line being typed. `0` draws no background; `12` is black. [In-game chat](/systems/chat/) describes the message list.

No dialog offers the setting, but saving the options writes it back to `sun.ini` unchanged, so a value set by hand is kept.

```ini title="sun.ini"
[Options]
TextBackgroundColor=12
```
