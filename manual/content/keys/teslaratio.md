---
key: TeslaRatio
summary: Parsed Tesla coil share of a computer base that the engine never uses.
no_effect: true
see_also: ["system:ai-base-building", TeslaLimit]
when_omitted:
  kind: value
  value: ".16"
---

No BuildingType is treated as a Tesla coil anywhere in the engine. What makes a structure behave like one is an electric primary weapon, not any flag on the building type; the charge-up looks for that weapon. Defensive structures are picked by [the defense planner](/systems/ai-base-building/#base-defenses) from the anti-air, anti-armor and anti-infantry values each type is given as the types load.
