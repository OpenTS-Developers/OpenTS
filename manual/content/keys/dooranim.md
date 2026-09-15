---
key: DoorAnim
summary: The shape file drawn over a structure as its factory door works.
see_also: ["DoorStages", "DamagedDoor", "UnderDoorAnim", "DeployingAnim", "DeployTime", "WeaponsFactory", "system:production"]
when_omitted:
  kind: value
  value: ""
  note: No door shape is loaded and none is drawn.
---

The value is a filename without its extension, and only a non-empty value is taken. The named `<value>.SHP` is loaded with the rules, after the name has been rewritten for the scenario theater on the structure-art convention. The rewrite compares the name's second letter with the letter of every theater the game lists, without regard to case. A name whose second letter matches any listed theater's letter has that letter replaced with the current theater's letter: `T` in temperate and `A` in snow. A name whose second letter matches no listed theater's letter is loaded as written. A theater declared without a letter leaves names as written. It is fetched again, under the theater then in force, when a saved game is restored, and on a [`NewTheater=yes`](/keys/newtheater/) type as each theater is set up.

The shape is drawn only while the structure is unloading: the sequence a [`WeaponsFactory=yes`](/keys/weaponsfactory/) structure runs while a finished vehicle leaves. It is drawn in the pass that draws over the structure, so it covers both the structure and the vehicle standing in the doorway. [`DoorStages`](/keys/doorstages/) covers which frame the door's progress selects.

```ini title="art.ini"
[MYWEAP] ; example war factory, drawn from its own Image ID
DoorAnim=GAWEAP_D  ; loaded as GTWEAP_D.SHP in temperate
DoorStages=9
UnderDoorAnim=GAWEAP_1
```

Its depth bias is a fixed five pixels toward the viewer, and it is drawn at the cell's own lighting level, so [`ExtraLight`](/keys/extralight/) does not reach it.
