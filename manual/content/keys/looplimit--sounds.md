---
key: LoopLimit
scope: sounds
label: Loop count alias
see_also: [Loop]
when_omitted:
  kind: value
  value: "0"
---

Another name for [`Loop=`](/keys/loop/), accepted so that sound sections written for Vinifera read unchanged. It is read only when `Loop=` is absent from the section.

```ini title="sound01.ini"
[ALARM]
Control=LOOP
LoopLimit=4
```
