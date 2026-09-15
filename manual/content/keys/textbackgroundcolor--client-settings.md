---
key: TextBackgroundColor
scope: client-settings
label: Chat text background
see_also: [MessageDelay, IncomingMessage]
when_omitted:
  kind: value
  value: "12"
---

The palette index drawn behind every glyph of the in-game message list and of the line being typed. `12` is black, which is the value the CnCNet client's option for a black chat background writes. `0` draws nothing. The game controls dialog does not offer the setting, but it writes the figure back to `sun.ini` with the rest of the options. A value set by hand therefore survives the dialog. [In-game chat](/systems/chat/) owns the list the setting is drawn on.

```ini title="sun.ini"
[Options]
TextBackgroundColor=0
```
