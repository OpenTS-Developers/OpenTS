---
key: MaxMoney
summary: Upper end of the starting-credits slider on every multiplayer and skirmish setup screen.
see_also: [Money]
when_omitted:
  kind: value
  value: "10000"
---

The figure is spent entirely on the setup screens: skirmish, network and online all build their credits slider from `2500` to this value. Nothing during a match compares a house's balance against it, so it neither caps what a house may accumulate nor limits what [`Money`](/keys/money/) can seed; it limits only what a player can choose.

The lower end is fixed in the engine and this setting cannot move it, so raising the starting money floor is not possible from the rules.
