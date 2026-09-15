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

The `[Sides]` section is read first and points every country it lists at its side. This key is read afterwards and places a country that no `[Sides]` entry lists. A country the section placed keeps that side: a differing value here is logged and ignored. The roster the sides are enumerated from is therefore the one the rules declared, and the raw Tiberian Sun rules' `[Nod] Side=GDI` leaves Nod on its own side. A country left with no side at all is not an error. The stock civilian and mutant countries would have none if `[Sides]` did not list them, because their own `Side=` values name sides only `[Sides]` creates.

The side is a coarser grouping than the country. The side of the country the local player plays decides which art and interface archives a game is presented with. These are the numbered `SIDEC01.MIX` and `SIDENC01.MIX`, with `SIDECD01.MIX` added in a campaign game and `02` in place of `01` for the second side. The same side's voices come with them, short of a mission's [`SpeechSide=`](/keys/speechside/). Defeat does not follow the side either: a campaign loss plays the mission's lose movie, and a multiplayer or skirmish loss shows the multiplayer score screen, whose artwork is one fixed file. A musical score restricted with [`Side=`](/keys/side/#scope-themes) is offered only while that country belongs to the side. The side of the country a house [acts as](/keys/actslike/) supplies the computer's [base building](/systems/ai-base-building/), its [hunter-seeker](/keys/hunterseeker/#scope-side) and the side an [AI trigger](/mapping/ai-triggers/) may be restricted to. An object's survivor is decided by its owner's country's side as well. A country with no side always leaves a [`Technician`](/keys/technician/). A country with a side leaves its [`Crew`](/keys/crew/), though a weapon-equipped object has a 15% chance of leaving a technician instead.

A value naming no declared side is logged and ignored, and the country keeps the side it had.
