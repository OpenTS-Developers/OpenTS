---
key: IonCannonDamage
summary: The raw damage an ion cannon blast applies, and the strength ceiling the computer rates a target against.
see_also: [IonCannonWarhead, "system:superweapons"]
when_omitted:
  kind: value
  value: "700"
---

The blast applies this figure through [`IonCannonWarhead`](/keys/ioncannonwarhead/) with no source. The warhead's [`Verses`](/keys/verses/) table, its spread falloff and [`Immune=yes`](/keys/immune/) all apply, and nothing is credited with the kills. The same figure sizes the light flash when that warhead is declared bright. A cell under a bridge is detonated twice at full strength, once at bridge height and once at ground level.

The value is read a second time by the computer's [target rating](/systems/superweapons/#the-computers-use): a candidate earns its premium rating only while its current strength is at or below this figure. Raising the damage therefore widens the set of objects the computer considers worth a strike, as well as making the strike hurt more.
