---
key: SpotlightRadius
summary: Base radius, in leptons, within which a spotlight notices an intruder.
see_also: [SpotlightLocationRadius, SpotlightMovementRadius, HasSpotlight, "system:power"]
when_omitted:
  kind: value
  value: "175"
---

While a beam is nearer its structure than [`SpotlightLocationRadius`](/keys/spotlightlocationradius/), it detects at exactly this radius. Past that distance it gains about seven and a half leptons for every whole tenth of the span up to [`SpotlightMovementRadius`](/keys/spotlightmovementradius/) that it has traveled. The same radius fixes the spread of the two glowing edges drawn from the structure down to the pool of light, so a beam that sees further is a visibly broader one. Once that radius passes the beam's own distance from the structure, the two edges are not drawn at all.

```ini title="rules.ini"
[General]
SpotlightRadius=256  ; one cell
```

A structure has a beam at all only where its type sets [`HasSpotlight=yes`](/keys/hasspotlight/).

Noticing an intruder has one consequence. A sweeping beam whose structure holds a tag springs the [Enemy In Spotlight...](/mapping/events/tevent-enemy-in-spotlight/) and [Enemy In Spotlight... (repeating)](/mapping/events/tevent-enemy-in-spotlight-repeating/) trigger events. They spring for any non-allied infantryman or vehicle standing in the nine cells around the beam, within 30 leptons more than that radius. The scan never looks outside those nine cells, so a radius that reaches past them catches nobody further out. A beam set to circle its structure springs neither. A following beam springs them only on the frame it gives its target up and reverts to sweeping. [Fields, fences and lights](/systems/power/#fields-fences-and-lights) covers the power condition that both the drawing and the scan depend on.

:::caution[The pool of light on the ground does not follow this key]
The images that pool is drawn from are built once when the game starts, before any rules file has been read. They keep the size the built-in figure gave them for the rest of the session. A value written in `[General]` reaches the detection radius and the width of the beam above the ground, and nothing else.
:::
