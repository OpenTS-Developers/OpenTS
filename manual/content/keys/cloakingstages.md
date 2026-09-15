---
key: CloakingStages
summary: The number of fade stages an object's cloak is measured against.
see_also: [CloakingSpeed, "system:cloaking"]
when_omitted:
  kind: value
  value: "9"
---

```ini title="rules.ini"
[General]
CloakingStages=9
```

The engine divides the stage an object has reached by this figure and picks its appearance from fixed fractions of the result. Raising the value stretches every band of the fade over more stages rather than adding bands. It is also where the reappearing fade begins: an object forced out of a cloak restarts one stage below this figure and steps down to zero. [Hiding an object](/systems/cloaking/#the-four-states) covers the bands themselves and where each fade ends.

The figure is used as a divisor with no zero guard, so `CloakingStages=0` divides by zero. A structure fades through fifteen fixed translucency steps instead and never reads the value.
