---
format_id: ui-ini
title: UI.INI
summary: Sets how a selected object's order lines, a firing vehicle's sighting laser, control group numbers and health bars are drawn.
kind: file
source_files:
  - code/uicontrol.cpp
  - code/init.cpp
filenames:
  - UI.INI
key_scopes:
  - file: ui.ini
    section:
      kind: literal
      name: Ingame
  - file: ui.ini
    section:
      kind: literal
      name: Pips
related:
  - type: system
    id: action-lines
  - type: format
    id: opents-ini
  - type: using
    id: configuration-files
---

The file is optional and every key has a default, so a file names only what it changes. The order-line, sighting-laser and health-bar keys go in `[Ingame]`, and the control group number keys go in `[Pips]`. The values below are examples.

```ini title="UI.INI"
[Ingame]
AlwaysShowActionLines=yes
MovementLineDashed=yes
MovementLineColor=0,255,0
NavComQueueLineThick=yes
InfantryHealthBarPos=-5,-20

[Pips]
UnitWithPipGroupNumberOffset=-4,-15
```

Write a color as three numbers from 0 to 255 for red, green and blue, separated by commas. A color value that does not start with three comma-separated numbers keeps the default. Anything after the third number is ignored. [Action lines](/systems/action-lines/) explains what each line is and when it is drawn.

## Control group numbers

An object whose pips are drawn also shows its group number when its group is one of the ten the player's control groups use. The number is the group's key, `1` to `9`, or `0` for the tenth group. Eight `[Pips]` keys set where it is printed, one pair for each kind of object:

| Object | Without pips | With pips |
| --- | --- | --- |
| Vehicle | [`UnitGroupNumberOffset`](/keys/unitgroupnumberoffset/) | [`UnitWithPipGroupNumberOffset`](/keys/unitwithpipgroupnumberoffset/) |
| Infantry | [`InfantryGroupNumberOffset`](/keys/infantrygroupnumberoffset/) | [`InfantryWithPipGroupNumberOffset`](/keys/infantrywithpipgroupnumberoffset/) |
| Structure | [`BuildingGroupNumberOffset`](/keys/buildinggroupnumberoffset/) | [`BuildingWithPipGroupNumberOffset`](/keys/buildingwithpipgroupnumberoffset/) |
| Aircraft | [`AircraftGroupNumberOffset`](/keys/aircraftgroupnumberoffset/) | [`AircraftWithPipGroupNumberOffset`](/keys/aircraftwithpipgroupnumberoffset/) |

An object uses the "with pips" key when its type's pip row, which [`MaxPips`](/keys/maxpips/) describes, has a length other than zero. A type with [`PipScale=Ammo`](/keys/pipscale/) and the default unlimited [`Ammo`](/keys/ammo/) counts as having pips even though it draws none.

Each value is two whole numbers, a horizontal and a vertical offset in pixels, separated by a comma. Positive values move the number right and down. The offset is measured from the point the object's pips are laid out from, and the number's top-left corner is placed there:

- for a structure, or a vehicle with [`IsCoreDefender=yes`](/keys/iscoredefender/#scope-unittype), that point is the left-hand corner of its footprint at ground level;
- for any other object, it is 10 pixels left of and 10 pixels below the object's position.

The defaults are `-4,-4` without pips and `-4,-8` with pips. A value that does not start with two comma-separated numbers keeps the default, and anything after the second number is ignored.

Vinifera reads the same eight keys from `[Ingame]`, so move them to `[Pips]` when reusing a Vinifera file. Setting all eight to `-8,-33` gives the position Twisted Insurrection uses.

## Health bar positions

Infantry, vehicles and aircraft show their health as a row of pips. Two `[Ingame]` keys set where that row begins:

| Object | Key |
| --- | --- |
| Infantry | [`InfantryHealthBarPos`](/keys/infantryhealthbarpos/) |
| Vehicle or aircraft | [`UnitHealthBarPos`](/keys/unithealthbarpos/) |

A structure, and a vehicle with [`IsCoreDefender=yes`](/keys/iscoredefender/#scope-unittype), draw their health as a pip bar along the near edge of their footprint instead and use neither key.

Each value is two whole numbers, a horizontal and a vertical offset in pixels, separated by a comma. Positive values move the row right and down, while negative values move it left and up. The offset is measured from the object's position on screen. The first pip is centered on that offset point, and each later pip is two pixels further right, so the offset moves the whole row. A value that does not start with two comma-separated numbers keeps the default, and anything after the second number is ignored.

## When the file is read

The game reads the file at startup, after it registers the game archives. It reads the file again each time a scenario or saved game loads, after mounting the archives of the player's side, so a copy inside a side's archive applies to games played as that side. It reads the file once more when a match ends and the menus return, after mounting the [no-side archives](/formats/mix/#theater-side-and-speech-archives). Each read starts from the defaults, so only the keys in the copy read last take effect.

The file is opened through the game's file search, so it may be a loose file in any folder the game searches or be inside an archive. A loose copy is used ahead of an archived one. [OPENTS.INI](/formats/opents-ini/#the-order-files-are-searched-for-in) lists the folders and the order they are searched in, and its [`[Files]` section](/formats/opents-ini/#the-files-it-reads) can change the file name.
