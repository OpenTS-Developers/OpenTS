---
format_id: multiplayer-rules
title: MPLAYER.INI
summary: Rules read over the others in every game that is not a campaign, so multiplayer and skirmish can differ from the single-player balance.
kind: file
source_files:
  - code/rules.cpp
  - code/init.cpp
  - code/deploymentconfig.cpp
filenames:
  - MPLAYER.INI
  - MPLAYERFS.INI
related:
  - type: format
    id: rules-registries
  - type: format
    id: opents-ini
---

A rules file read only when the game is not a campaign. It accepts everything
`RULES.INI` accepts, and is processed the same way, so a deployment can give
multiplayer its own balance without a second copy of the whole rules file.

```ini title="mplayer.ini"
[155mm]
Damage=115
ROF=150
```

Both files are optional, and the game starts without them.

## When they are read

`MPLAYER.INI` is read in every game type except a campaign, which includes
skirmish against the computer. `MPLAYERFS.INI` is read on the same terms, and
only while Firestorm is enabled.

They sit at the end of the rules layering, after the expansion and the
translated rules and before the scenario:

1. the selected `RULE*.INI`
2. `LANGRULE.INI`
3. `FIRESTRM.INI`, when Firestorm is enabled
4. `LANGFS.INI`
5. `MPLAYER.INI`, outside a campaign
6. `MPLAYERFS.INI`, outside a campaign and with Firestorm enabled
7. the scenario's own overrides

A map is therefore still the last word: a scenario that sets one of these
values wins, as it does over every other rules layer.

The files are named in [`OPENTS.INI`](/formats/opents-ini/#the-files-it-reads)
by `MultiplayerRules=` and `MultiplayerRulesExpansion=`, and are searched for in
the same order as everything else the game opens. Unlike the rules, they are not
gathered by a wildcard: the game opens the name it is given and no other.

## Write only keys the rules also write

The rules are rebuilt from the files each time a scenario loads, but a setting
no file names keeps whatever it last held. So a key written **only** here keeps
its multiplayer value into the next campaign played without restarting the game.

Write a key here only where `RULES.INI` writes it too. Then the campaign load
reads the rules value back over it and the two game types stay separate.

The expansion rules and a map's own overrides carry the same trap. It is easier
to walk into here, because these files exist to hold values that differ from the
rules.

## What they cannot do

- **Declare a theater:** `[Theaters]` is read once as the game starts, because a
  theater's position is the number maps, saves and the multiplayer checksum all
  carry. [Rules registration](/formats/rules-registries/) covers why.
- **Resize the heaps:** `[Maximums]` is read from the rules alone, before any
  game type is known.
- **Add a country a player can pick:** the list of countries a player chooses
  from is built from the rules alone, so a country declared only here is missing
  from it. The country itself does exist once the scenario loads, and a map may
  place houses of it. `FIRESTRM.INI` has the same limit.
- **Seed the lobby defaults from the expansion:** `[MultiplayerDefaults]` in
  `MPLAYER.INI` sets the starting credits, unit count, tech level and match
  options a host sees, because that file is read before the menus. The addon is
  not chosen until later, so `MPLAYERFS.INI` is too late to change them, exactly
  as `FIRESTRM.INI` is.

Anything else `RULES.INI` can do, these can. New object types registered here
exist for the multiplayer game and are saved with it, so a saved game restores
them whether or not the file is still on disk.

## Every player needs the same copy

Both files count toward the rules checksum a host compares against each joining
player, alongside `RULES.INI` and `FIRESTRM.INI`. A player whose copy differs,
including one who has the file where the host does not, is refused at the lobby
rather than left to desync later.

A file absent from every machine changes no checksum.
