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

`MPLAYER.INI` is a rules file that applies only outside a campaign. It accepts everything `RULES.INI` accepts, so a deployment can give multiplayer and skirmish their own balance without a second copy of the whole rules file.

```ini title="mplayer.ini"
[155mm]
Damage=115
ROF=150
```

In every game that is not a campaign, the `155mm` weapon uses these two values unless the map sets them. In a campaign it uses its `RULES.INI` values.

`MPLAYERFS.INI` is the expansion's copy. Both files are optional, and the game starts without them. The game reads them once at startup, so an edit to either file takes effect after a restart.

## When they are read

`MPLAYER.INI` applies in every game that is not a campaign, including skirmish against the computer. `MPLAYERFS.INI` applies on the same terms, and only while Firestorm is enabled.

Each time a scenario loads, the game applies the rules files in this order, and a later file overrides an earlier one:

1. `RULES.INI`
2. `LANGRULE.INI`
3. `FIRESTRM.INI`, when Firestorm is enabled
4. `LANGFS.INI`
5. `MPLAYER.INI`, outside a campaign
6. `MPLAYERFS.INI`, outside a campaign and with Firestorm enabled
7. the scenario's overrides

A scenario that sets one of these values therefore overrides both files, as it overrides every other rules file.

`MultiplayerRules=` and `MultiplayerRulesExpansion=` in [`OPENTS.INI`](/formats/opents-ini/#the-files-it-reads) name the two files. The game looks for them in the same folders, in the same order, as every other file it opens. Only the named file is read; no wildcard gathers other multiplayer rules files.

## Write only keys the rules also write

Each time a scenario loads, the game rebuilds its object types from the rules files: vehicles, infantry, aircraft, structures, weapons, warheads, projectiles, superweapons, animations, voxel animations, particles, particle systems, overlays, terrain objects, smudges, countries and the colors in `[Colors]`. A key in one of their sections therefore returns to the value the other rules files give it, or to its default, when a campaign loads.

Other settings are not reset between scenarios. A key in a section such as `[General]`, `[CombatDamage]` or a Tiberium's section keeps the last value any file gave it. If only `MPLAYER.INI` writes such a key, the multiplayer value stays in force in a campaign started later without restarting the game.

Write such a key here only where `RULES.INI` writes it too. The campaign load then reads the `RULES.INI` value back over it, and the two kinds of game stay separate.

The expansion rules and a map's overrides carry the same risk. `MPLAYER.INI` and `MPLAYERFS.INI` meet it most often, because they exist to hold values that differ from the rules.

## What they cannot do

- **Declare a theater or change its settings:** `[Theaters]` and each theater's section are read once at startup, from `RULES.INI` and `FIRESTRM.INI` only. [Rules registration](/formats/rules-registries/) explains why the list of theaters cannot change after that.
- **Set `[Maximums]`:** that section is read only from `RULES.INI`.
- **Add a country a player can pick:** the countries a player chooses from are read from `RULES.INI` alone, so a country declared only here is missing from that list. The country does exist once the scenario loads, and a map may place houses of it. `FIRESTRM.INI` has the same limit.
- **Set the lobby's starting values from the expansion:** `[MultiplayerDefaults]` in `MPLAYER.INI` sets the starting credits, unit count, tech level and match options a host sees, because the game reads that file before the menus. `MPLAYERFS.INI` cannot change them, and neither can `FIRESTRM.INI`, because the game does not know whether Firestorm is enabled until later. The section is still read from every rules file when a scenario loads, so a key that acts during the match, such as [`BuildOffAllyAnyStructure=`](/keys/buildoffallyanystructure/), takes effect from either file.

Anything else `RULES.INI` can do, these files can do. Object types that only these files declare exist in the multiplayer game and are stored in its saves, so loading such a save restores them even when the file is no longer on disk.

## Every player needs the same copy

In a game hosted from the game's own network lobby, the host compares a checksum of its rules with each joining player's. The checksum covers `RULES.INI` and `MPLAYER.INI`, plus `FIRESTRM.INI` and `MPLAYERFS.INI` while Firestorm is enabled. A player whose copy of any of them differs is refused, including a player who has one of these files where the host does not.

A file that is absent from every machine, or that has no sections, changes no checksum.

A game started by an external client does not make this comparison. Give every player the same files there, because a difference is not detected when the game starts.
