---
key: C4Warhead
summary: The WarheadType behind damage the engine deals without a weapon to draw one from.
when_omitted:
  kind: value
  value: none
---

The engine reaches for this warhead wherever it applies damage that no weapon produced:

- a demolition charge going off on a structure;
- an infantryman poisoned by Tiberium, or left standing on rock or water;
- a Tiberium field chain-reacting;
- a harvester exploding with its load;
- a laser fence energizing, or being torn down over an occupied cell;
- a firestorm wall closing on something walking into it;
- the [structure damage tick](/systems/power/#the-structure-damage-tick) a house takes while it is short of power.

The warhead's animation lists shape the explosion every case here produces. The demolition charge, the poisoning, the fence, and the firestorm wall pass their damage as forced, which skips the armor table and [`Immune=yes`](/keys/immune/) alike: every object in range takes the full damage. The low-power tick, the chain reaction, and the harvester explosion are not forced. Those three alone answer to the warhead's [`Verses`](/keys/verses/) table and are blocked by immunity. [`Spread`](/keys/spread/) trims the chain reaction and the harvester blast by distance; the tick lands at point-blank range, so nothing trims it.

Leaving the key out leaves the warhead empty. The harvester explosion and the low-power tick then do nothing at all, and a Tiberium chain reaction burns the growth down without damaging anything standing in it. The four forced paths still land their full damage, because forced damage never reads the warhead.
