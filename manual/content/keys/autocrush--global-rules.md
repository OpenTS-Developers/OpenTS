---
key: AutoCrush
scope: global-rules
label: IQ threshold
see_also: ["IQ", "Crusher"]
when_omitted:
  kind: value
  value: "2"
---

A computer house whose [`IQ`](/keys/iq/) reaches this level lets its vehicles answer being shot at by driving over the attacker instead of returning fire. The crush needs all of these, in the order the engine tests them:

- the vehicle is on no team, is not tethered, and is not allied to its attacker;
- the vehicle is a [`Crusher=yes`](/keys/crusher/) or has earned the crusher ability;
- the attacker is [`Crushable=yes`](/keys/crushable/#scope-aircrafttype);
- the attacker is no further off than [`Crush`](/keys/crush/);
- the attacker is not a disguised spy.

:::caution[The `[Difficult]` slot never crushes]
A house holding the hardest of the three [difficulty slots](/systems/difficulty/#from-the-setting-to-a-slot) is refused before the IQ is even considered. In a skirmish that slot is what every computer house is given at the Easy setting, so lowering the difficulty switches computer crushing off entirely, whatever this threshold says. Once a session holds more than one person the [easy-game bonus](/systems/difficulty/) moves those houses a slot easier, and they crush again.
:::
