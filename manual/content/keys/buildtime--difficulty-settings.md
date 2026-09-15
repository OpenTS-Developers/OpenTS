---
key: BuildTime
scope: difficulty-settings
label: Difficulty build-time multiplier
when_omitted:
  kind: value
  value: "1"
  note: The difficulty block is re-read from fixed defaults whenever its section is present, so a later file that contains the section without this key restores 1 rather than keeping the earlier value.
---

`[Easy]`, `[Normal]` and `[Difficult]` each set their own multiplier, and a house takes the one for [the difficulty slot it is assigned](/systems/difficulty/#from-the-setting-to-a-slot). Every object that house produces has its build time multiplied by it, alongside the [country's own multiplier](/keys/buildtime/#scope-housetype) outside campaign games and [`GameSpeedBias`](/keys/gamespeedbias/) everywhere.

In a campaign game a house under the player's control takes the slot for the difficulty selected for the mission. Every other house takes the computer slot, so the two sides can be tuned apart.

```ini title="rules.ini"
[Difficult]
BuildTime=0.8
```
