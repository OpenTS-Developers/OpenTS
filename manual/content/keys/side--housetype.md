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

The value names a side declared in `[Sides]`. A side groups countries: several countries can share one, and a country with no side is valid.

`[Sides]` is read before this key and wins over it. A country that already has a side keeps it, and a `Side=` naming a different side is ignored. The key therefore only places a country that no `[Sides]` entry lists.

A value naming no side that `[Sides]` declares is ignored, and the country keeps the side it had.

## What the player's side decides

The side of the country the local player plays decides:

- which side archives the game mounts, as [Theater, side and speech archives](/formats/mix/#theater-side-and-speech-archives) lists; a side without its own archives uses the first side's;
- which side's voices play, unless a mission sets [`SpeechSide=`](/keys/speechside/);
- which music tracks are offered, when a track is restricted with its own [`Side=`](/keys/side/#scope-themes);
- in skirmish and multiplayer games, which pair of loading pictures appears: the second side's for a country on the second side, the first side's otherwise;
- on the campaign score screen, which side's casualty bars show the player's losses: the first side's bars when the player's country is on the first side, the second side's bars otherwise.

For the archives and voices, a player's country with no side counts as the first side. Such a player is offered no side-restricted track and sees the second side's casualty bars.

## What a house's side decides

The side of the country a house [acts as](/keys/actslike/) decides the computer's [base building](/systems/ai-base-building/), the house's [hunter-seeker](/keys/hunterseeker/#scope-side), and whether an [AI trigger](/mapping/ai-triggers/) restricted to a side is open to that house.

Survivors take their type from that side too. Whether the technician replaces the crew type turns on the side of the owner's own country: a country with no side leaves a [`Technician`](/keys/technician/), while a country with a side leaves the [`Crew`](/keys/crew/) type and gives an armed object a 15% chance of a technician instead. [Survivor types](/systems/capture/#survivor-types) gives the whole order, including the engineer a structure can leave.
