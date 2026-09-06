---
key: TextBackgroundColor
scope: client-settings
label: Chat text background
see_also: [MessageDelay, IncomingMessage]
when_omitted:
  kind: value
  value: "12"
---

The palette index drawn behind every glyph of the in-game message list and of the line being typed; `12` is black, the value the CnCNet client's option for a black chat background writes, and `0` draws nothing. The game controls dialog does not offer the setting, but it writes the figure back to `sun.ini` with the rest of the options, so a value set by hand survives the dialog. [In-game chat](/systems/chat/) owns the list the setting is drawn on.

```ini title="sun.ini"
[Options]
TextBackgroundColor=0
```
