---
key: TeslaCharge
summary: Sound a structure makes as its electric weapon begins charging.
see_also: [TeslaZap, Charges, TurretChargeAnimRate]
when_omitted:
  kind: value
  value: none
---

```ini title="rules.ini"
[AudioVisual]
TeslaCharge=TESLCHG ; a sound ID registered in SOUND.INI
```

The sound is played from the structure's own position on the pass that starts the charge, and one charge covers one shot. The charge starts only when all of the following hold:

- the structure has a primary weapon marked [`Charges=yes`](/keys/charges/);
- its build-up is finished;
- it has a target;
- its house is at full power;
- it is switched on;
- its reload delay has passed; and
- the type's `Primary=` or `Secondary=` weapon holds a legal firing solution on that target.

Nothing about the sound is tied to the player: an enemy structure charging within earshot is heard.

Losing power, being switched off or losing the target discharges the turret in silence, and a structure that is already charged does not sound again until it has fired and started over.
