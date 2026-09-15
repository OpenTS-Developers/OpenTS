---
key: ROF
scope: difficulty-settings
label: Difficulty reload multiplier
see_also: ["system:difficulty"]
when_omitted:
  kind: value
  value: "1"
  note: The difficulty block is re-read from fixed defaults whenever its section is present, so a later file that contains the section without this key restores 1 rather than keeping the earlier value.
---

`[Easy]`, `[Normal]` and `[Difficult]` each set their own multiplier, and a house takes the one for [the difficulty slot it is assigned](/systems/difficulty/#from-the-setting-to-a-slot). The delay an object of that house waits before its next shot is the weapon's own [`ROF`](/keys/rof/#scope-weapontype) multiplied by this value, plus a random 0 to 2 frames. It multiplies the delay rather than the rate, so a figure above 1 fires more slowly.

Three paths never reach it:

- a building holding more than one round of ammunition waits a flat frame instead;
- a shot within a weapon's burst uses that burst's own delay;
- a sonic weapon, or one driven by spark, fire or railgun particles that already has its particle system attached, returns the weapon's `ROF` unmodified.

The product is worked out once, when the house is given its slot, alongside the [country's own multiplier](/keys/rof/#scope-housetype) outside campaign games. In a campaign game the country's figure is dropped and this one stands alone.

```ini title="rules.ini"
[Difficult]
ROF=1.2
```
