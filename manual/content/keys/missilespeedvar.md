---
key: MissileSpeedVar
summary: Parsed fraction that the engine never uses.
no_effect: true
see_also: [MissileROTVar]
when_omitted:
  kind: value
  value: ".25"
---

The name marks it as the counterpart of [`MissileROTVar`](/keys/missilerotvar/): a fluctuation applied to a homing projectile's speed in flight. Nothing reads the stored figure, so no fluctuation is ever applied. A homing projectile's speed climbs toward the firing weapon's [`Speed`](/keys/speed/#scope-weapontype) and holds there, coasting back down by half the BulletType's [`Acceleration`](/keys/acceleration/#scope-bullettype) if it is ever moving faster. That `Acceleration` governs the climb only for a weapon whose `Speed` is `40` or more. Below that the projectile stays in its launch phase and gains one lepton of speed every other frame until it is up to speed.

The matching rate-of-turn setting is read, and it is what gives a missile its weaving flight path.
