---
key: PlayerScatter
summary: Whether the objects a person commands scatter of their own accord.
see_also: ["Scatter", "Incoming"]
when_omitted:
  kind: value
  value: "no"
---

Scattering is the short move an object makes to get out of the way of something. Three decisions read this setting, and it is only one of several ways past each of them.

An object that has just been hit and is allowed to answer the attacker scatters under **All of:**

- it has no target of its own;
- it has no destination of its own;
- **Any of:** this is on, or it has earned the scatter ability.

That one applies to every house alike, computer and player. Earning the scatter ability means the object is a veteran whose type lists the ability under [`VeteranAbilities=`](/keys/veteranabilities/), or an elite whose type lists it under [`VeteranAbilities=`](/keys/veteranabilities/) or [`EliteAbilities=`](/keys/eliteabilities/).

An object that has just been hit and is not allowed to answer scatters under **All of:**

- it is not tethered (docked at a refinery or aircraft pad, or carried by a transport);
- it is standing still;
- it has no target of its own;
- it has no destination of its own;
- it is not an aircraft;
- **Any of:** its house is under computer control, this is on, or it has earned the scatter ability.

A cell warned that a threat is coming scatters its occupants under **Any of:**

- an occupant of that cell is elite;
- the warning was raised as overriding;
- this is on;
- the object has earned the scatter ability;
- the object's house's IQ has reached [`[IQ] Scatter`](/keys/scatter/#scope-global-rules).

The first three of those settle the whole cell at once; the last two are read per object, so one occupant can scatter while its neighbor stays put.

Infantry adds a refusal of its own on top. A soldier belonging to a house a person commands turns a scatter down outright unless **Any of:**

- this is on;
- the soldier has earned the scatter ability;
- the move was forced;
- the warning was overriding;
- the soldier belongs to a team.

Vehicles have no such refusal.

The mission an object is on can forbid scattering before any of this is reached; [`Scatter`](/keys/scatter/#scope-mission-behavior) covers that.
