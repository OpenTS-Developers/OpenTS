---
key: SpeakDelay
summary: Minutes an EVA announcement waits before it may be repeated.
see_also: ["system:power"]
when_omitted:
  kind: value
  value: "2"
---

Four announcements to the local player share this interval: the base is under attack, funds are low, Tiberium storage is nearly full, and [power is low](/systems/power/#player-feedback). The attack warning and the power warning each keep a timer of their own. The money and storage warnings share one, so either of those two silences the other for the interval. Every timer is scaled against the game speed setting, so it runs for roughly the same wall-clock time at any speed.
