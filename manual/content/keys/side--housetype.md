---
key: Side
scope: housetype
label: Country side
see_also: [Multiplay, Crew, Technician, ActsLike]
when_omitted:
  kind: inherited
  note: The side whose [Sides] entry lists this country, or no side at all when none does.
---

```ini title="rules.ini"
[Nod]
Side=Nod
```

The `[Sides]` section is read first and points every country it lists at its side; this key is read afterwards and places a country that no `[Sides]` entry lists. A country the section placed keeps that side: a differing value here is logged and ignored, so the roster the sides are enumerated from is the one the rules declared, and the raw Tiberian Sun rules' `[Nod] Side=GDI` leaves Nod on its own side. A country left with no side at all is not an error — that is what the stock civilian and mutant countries would carry if `[Sides]` did not list them.

The side is a coarser grouping than the country. The side of the country the local player plays decides which art and interface archives a game is presented with and, short of a mission's [`SpeechSide=`](/keys/speechside/), its voices; a musical score restricted with [`Side=`](/keys/side/#scope-themes) is offered only while that country belongs to the side, and the score screen draws its artwork from it. The side of the country a house [acts as](/keys/actslike/) supplies the computer's [base building](/systems/ai-base-building/), its [hunter-seeker](/keys/hunterseeker/#scope-side) and the side an [AI trigger](/mapping/ai-triggers/) may be restricted to. And whether an object's survivor is a [`Crew`](/keys/crew/) or a [`Technician`](/keys/technician/) turns on whether its owner's country has a side at all.

A value naming no declared side is logged and ignored, and the country keeps the side it had.
