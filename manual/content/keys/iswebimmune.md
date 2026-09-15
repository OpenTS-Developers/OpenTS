---
key: IsWebImmune
summary: Keeps a webbing warhead from pinning the soldier in place.
see_also: [Webby, WebDuration, WebbedInfantry, "system:target-selection"]
when_omitted:
  kind: value
  value: "no"
---

A [`Webby=yes`](/keys/webby/) warhead that lands on an ordinary soldier does no damage at all. It zeroes the figure, puts the soldier into the struggling animation for [`WebDuration`](/keys/webduration/) frames (15 to the second) give or take the warhead's variation, and springs the paralyzed trigger event on whatever tag the soldier holds. An immune soldier takes none of that. The whole branch is skipped, so the hit is applied as ordinary damage through the warhead's [`Verses`](/keys/verses/) percentages instead, and the soldier keeps moving.

```ini title="rules.ini"
[MYCYBORG] ; example InfantryType
IsWebImmune=yes
```

Immunity also changes what shoots at the soldier. An object choosing between a web primary and a second weapon treats an immune soldier as something it cannot web and reaches for the second slot, exactly as it would for a vehicle. [An empty second slot is a hazard](/systems/target-selection/#which-weapon-the-score-assumes) on any object with a web weapon.
