---
key: BerzerkAllowed
summary: Whether a cyborg turns berserk when a hit takes it below half strength.
see_also: ["Cyborg", "ConditionRed"]
when_omitted:
  kind: value
  value: "no"
---

The hit must leave a [`Cyborg=yes`](/keys/cyborg/) infantryman alive and take it from at or above half its strength to below half. A hit heavy enough to cross [`ConditionRed`](/keys/conditionred/) in the same blow is classed as the graver of the two and does not send the soldier berserk. A single heavy hit that takes a cyborg past both marks at once leaves it sane. The state is set once per soldier and is never cleared.

A berserk soldier stops recognizing its own side. Allied objects are no longer excluded from its target search, and an ally standing in a cell it sweeps is accepted as a target. It is put on the guard-area mission as it turns.

The [Go Berzerk](/mapping/actions/taction-go-berzerk/) trigger action and the [Go Berzerk](/mapping/missions/tmission-berzerk/) team mission set the same state directly. Neither reads this setting, and neither requires the infantry to be a cyborg or to be damaged.
