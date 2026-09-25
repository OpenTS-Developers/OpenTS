---
format_id: powerups
title: Crate powerups
summary: Sets each crate type's share of the crate draw, the animation it plays, and the figure its effect uses.
kind: record
files:
  - RULES.INI
  - LANGRULE.INI
  - FIRESTRM.INI
  - LANGFS.INI
  - MPLAYER.INI
  - MPLAYERFS.INI
  - map file
section: Powerups
syntax: "<crate type>=<shares>,<AnimType ID>,<data>"
fields:
  - { position: 1, label: Shares, value: Integer weight in the crate draw, required: true }
  - { position: 2, label: Animation, value: AnimType ID played where the crate was collected, required: false, note: "`<none>` for no animation" }
  - { position: 3, label: Data, value: Figure the crate's own effect interprets, required: false }
source_files:
  - code/rules.cpp
  - code/const.cpp
  - code/cell.cpp
---

The crate types are fixed by the engine, and this section's keys are their names: the result tokens listed on the [crate result](/reference/enums/crate/) page, and no others. Nothing here registers a crate: the section is read by walking the engine's own list of crate types, asking the file for each name in turn. An entry whose key is not one of those names is never looked at, so a misspelled name is inert rather than a new kind of crate, and no message says so.

```ini title="rules.ini"
[Powerups]
Money=55,MONEY,2000      ; MONEY is an AnimType registered in rules.ini
Napalm=25,<none>,600
Reveal=8,REVEAL
```

Every rules layer is offered the section in turn, and the map is offered it after them, so a map can reweight the draw for one scenario.

## Reading an entry

The three fields are read in order and each one that is present replaces what that crate held. An entry that stops early leaves the remaining settings as they stand, which is how the two-field entries above keep their data figures.

The animation is named by its AnimType ID. `<none>`, `none`, and a name that matches no registered AnimType all mean no animation; nothing distinguishes them. An ID that is registered only in a later rules layer counts as unmatched when the section is read. The data figure is read as a real number, and a percent sign anywhere in it divides the figure by one hundred.

:::caution[An empty field shifts the ones after it]
The split hands back the runs of text lying between commas and never an empty run, so two commas together read as one separator and the field between them is not held open. In `Money=40,,500` the second run handed back is therefore `500`, which is taken as the animation name and matches no AnimType, leaving the crate with no animation. Nothing is handed back third, so the data figure is not touched at all.
:::

## Crates the section leaves out

Because the section is read by asking for each crate name, a name it does not have is not left alone. The answer to a name that is not there is the built-in `0,NONE`, which takes that crate out of the draw and clears its animation, leaving only its data figure as it stands.

A layer that contains `[Powerups]` at all therefore has to list every crate it wants kept. Reweighting two crates in a map by writing a section with two entries sets the other seventeen to zero for that scenario.

## How the shares are used

The shares are weights rather than percentages. Collecting a crate adds up the shares of every crate type, then picks a number from `1` to that total. It walks the types adding each share to a running total until the total reaches the number picked, so a crate's chance is its own share over the total of all of them. A share of zero takes a crate out of the draw without removing it from the game: a scenario can still hand it out directly. Crates collected in a campaign game ignore the shares entirely, because those come from the rules settings that name a crate for each crate image, which [Crates](/systems/crates/#in-a-campaign) lists and applies.

:::danger[Shares that add up to zero pick a crate that does not exist]
The draw is not guarded against an empty pool. Where every share is zero, the number is asked for from `1` to `0`. A range given the wrong way round is put back in order rather than refused, so what comes back is `0` or `1` with about even odds. The running total never leaves zero, and the walk stops only once that total has reached the number picked, so a `0` stops at the first crate type and a `1` takes the walk past the last. About half of all pickups outside a campaign game therefore end past the end of the crate list. The effect and animation for that pickup are then read one entry beyond the end of the tables that hold them. The crate does nothing, and the animation number found past the end is looked up in the animation list, which the game may not survive. A section that gives a weight of zero to every crate it names reaches this, because the crates it leaves out are set to zero as they are read.
:::
