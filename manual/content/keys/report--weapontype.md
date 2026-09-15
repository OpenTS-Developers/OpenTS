---
key: Report
scope: weapontype
label: Firing sounds
see_also: ["Anim", "DropPodWeapon"]
when_omitted:
  kind: value
  value: ""
---

Which entry is played at the muzzle is decided once, when the firing object is created, and never again. The object draws a number then and uses it to index the list for the rest of its life. A list of several entries therefore gives one sound per object rather than variety between shots, and two objects of the same type may well pick the same one.

```ini title="rules.ini"
[MyGatling] ; example WeaponType
Report=TSGUN4,CHAINGN1,INFGUN3 ; registered sound names
```

An entry that does not match a registered sound is dropped without complaint, so `Report=none` produces an empty list rather than an error, and a misspelling shortens the list. Writing the key with nothing after the `=` is a different thing: the read finds no value and keeps whatever an earlier rules file set, so an inherited list cannot be cleared that way.

Three other paths play the list, and all three pick from it afresh each time rather than using the object's fixed choice. One is an EM pulse cannon firing at its stored destination. Another is a [`Jellyfish=yes`](/keys/jellyfish/) unit that has just stung something, which plays the list again for every cell of its sweep still to come. The third is a descending drop pod laying [covering fire](/keys/droppodweapon/) on the cell below it.

:::caution[Those three paths pick without checking the list]
The ordinary firing path checks that the list holds something before picking from it. The EM pulse cannon, the jellyfish's sting and the drop pod's covering fire do not. An empty list makes them play the engine's first sound entry, `FIRSTRM1`, the Firestorm defense burning sound, rather than stay silent. A weapon reached by one of those paths still needs at least one entry that resolves to a registered sound.
:::
