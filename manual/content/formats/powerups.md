---
format_id: powerups
title: Crate powerups
summary: Sets each crate result's share of the random crate draw, the animation it plays, and the number its effect uses.
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
syntax: "<crate result>=<shares>,<AnimType ID>,<data>"
fields:
  - { position: 1, label: Shares, value: Integer weight in the crate draw, required: true }
  - { position: 2, label: Animation, value: AnimType ID played where the crate was collected, required: false, note: "`<none>` for no animation" }
  - { position: 3, label: Data, value: Number the result's effect uses, required: false }
source_files:
  - code/rules.cpp
  - code/const.cpp
  - code/cell.cpp
---

The keys of `[Powerups]` are the result names on the [crate result](/reference/enums/crate/) page. The game reads only those names, so any other key, including a misspelled result name, has no effect. The section cannot add a new crate result. [Crates](/systems/crates/#what-each-result-does) describes what each result does with its number.

```ini title="rules.ini"
[Powerups]
Money=55,MONEY,2000      ; MONEY is an AnimType registered in rules.ini
Napalm=25,<none>,600
Reveal=8,REVEAL
```

Each rules file that has a `[Powerups]` section applies it in the [order the rules files are read](/formats/multiplayer-rules/#when-they-are-read). A map's section is applied after all of them, so a map can reweight the draw for games on that map.

Crate settings are not reset between games. At each scenario load the rules files' `[Powerups]` sections are applied again, and any value they leave unwritten keeps what the previous game used, including a value an earlier map set. A rules file with a `[Powerups]` section writes every share, so with one present a map's shares apply only to games on that map. Two values a map sets can still carry into later games until you quit:

- a result's animation, when each rules file with a `[Powerups]` section lists that result with only a share
- a result's number, unless a rules file's entry for that result has all three fields

When no rules file has a `[Powerups]` section, every value a map sets carries into later games.

## Reading an entry

Each field an entry writes replaces that result's current value. Fields the entry leaves off keep their current values, so `Reveal=8,REVEAL` above keeps the number `Reveal` already had.

The animation field takes an AnimType ID. `<none>`, `none`, and an ID that matches no registered AnimType all give no animation. The AnimType must be registered by the same rules file or an earlier one; an ID that only a later file registers gives no animation.

The data field is a decimal number. Written with a percent sign, it is divided by 100, so `50%` gives `0.5`.

:::caution[Leave no field empty]
Two commas in a row count as one, so an empty field moves every later field one place to the left. In `Money=40,,500`, `500` is read as the animation ID. It matches no AnimType, so `Money` gets no animation, and its number stays unchanged.
:::

## Crates the section leaves out

A `[Powerups]` section with at least one entry resets every crate result it does not list to a share of `0` with no animation. Only the number of an omitted result keeps its current value.

List every result you want to keep in each `[Powerups]` section you write. For example, a map section with two entries takes the other seventeen results out of the draw in that map's games.

## How the shares are used

Shares are relative weights and need not add up to 100. Outside a campaign, each collected crate draws one result, and a result's chance of being drawn is its share divided by the total of all shares. The drawn result can then be replaced, as [Crates](/systems/crates/#outside-a-campaign) describes.

A share of `0` keeps a result out of the random draw, but a campaign crate or a replacement after the draw can still give it. Campaign crates ignore the shares: each gives the result that the rules name for its crate overlay, as [Crates](/systems/crates/#in-a-campaign) lists.

:::danger[Keep the total of the shares above zero]
If the shares add up to `0` or less, crates collected outside a campaign can draw a result past the end of the result list. Unless that crate is replaced by a free MCV, it has no effect, and its animation is read from past the end of the animation settings, which can play an unrelated animation or crash the game.

When every share is `0`, about half of the crates collected outside a campaign draw `Money` and the other half run past the end. A section that sets `0` for every result it lists causes this, because the results it leaves out are also set to `0`. A negative share can also bring the total to `0` or less while other shares are above zero.
:::
