---
title: Porting a Vinifera mod
summary: The edits a mod written for Vinifera needs before OpenTS reads its files the way Vinifera did.
category: compatibility-migration
source_files:
  - code/uicontrol.cpp
  - code/init.cpp
  - code/rules.cpp
  - code/side.cpp
  - code/techtype.cpp
  - code/aitrig.cpp
  - code/campaign.cpp
  - code/tutorial.cpp
  - code/themeini.cpp
  - code/vocini.cpp
  - code/spawnerconfig.cpp
related:
  - type: using
    id: configuration-files
  - type: format
    id: ui-ini
  - type: format
    id: sound-ini
  - type: format
    id: theme-ini
---

Many OpenTS features follow Vinifera and read the same keys, so most settings a Vinifera mod writes for those features work unchanged. The tables below list the settings that need an edit, grouped by file. Each row links the page that gives the accepted values and defaults.

The tables list features that OpenTS implements differently from Vinifera or will not implement. A Vinifera feature OpenTS has not ported has no row; OpenTS ignores its keys.

## UI.INI

| In the Vinifera file | Change for OpenTS |
| --- | --- |
| `UnitGroupNumberOffset` and the seven other control group number keys under `[Ingame]` | Move them to `[Pips]`. See [Control group numbers](/formats/ui-ini/#control-group-numbers). |
| `InfantryHealthBarPos` and `UnitHealthBarPos` under `[Ingame]` | Swap the two numbers. Vinifera reads the vertical offset first; see [Health bar positions](/formats/ui-ini/#health-bar-positions). If the file leaves out `UnitHealthBarPos`, set `UnitHealthBarPos=-16,-25` to match Vinifera's default. |
| A `UIOVERRIDES.INI` file | OpenTS does not read it, and it reads only one copy of `UI.INI`. Put a complete `UI.INI`, with the override keys added, in the side's archive, and ship no loose `UI.INI`, which would be used instead. See [When the file is read](/formats/ui-ini/#when-the-file-is-read). |

## rules.ini

| In the Vinifera file | Change for OpenTS |
| --- | --- |
| A side whose section leaves out its power plants or hunter-seeker | Set [`RegularPowerPlant`](/keys/regularpowerplant/), [`AdvancedPowerPlant`](/keys/advancedpowerplant/), [`PowerTurbine`](/keys/powerturbine/) and [`HunterSeeker`](/keys/hunterseeker/#scope-side) in that side's section, unless it is the first side and its name contains `GDI` or the second side and its name does not. Vinifera gives a side whose name contains `GDI`, in capitals, the GDI plants and hunter-seeker from `[General]`, and every other side the Nod ones. OpenTS gives the GDI ones to the first side in `[Sides]`, the Nod ones to the second, and none to the rest. |
| `BuildOffAlly` under `[MultiplayerDefaults]`, or `EligibleForAllyBuilding` on a structure type | Not read. Vinifera lets a player build next to an ally's construction yards, and next to other structure types only with `EligibleForAllyBuilding=yes`. OpenTS lets any of the ally's structures count. Set [`BuildOffAllyAnyStructure=no`](/keys/buildoffallyanystructure/) to keep construction yards only. The launch file's [`BuildOffAlly`](/formats/spawn-ini/#the-options-every-house-plays-under) turns the feature on. |
| `AutoDeployMCV` under `[MultiplayerDefaults]` | Not read. Set `AutoDeployMCV=yes` in the launch file's [`[Settings]`](/formats/spawn-ini/#the-options-every-house-plays-under) instead. Unlike Vinifera's key, it also deploys the computer houses' base units, and it works in a match that starts with more than one unit. |
| `ImmuneToEMP=no` on a type with `IsCoreDefender=yes` | Remove the line to keep the type immune. Vinifera keeps a core defender immune whatever this key says, while OpenTS reads `no` as [vulnerable](/keys/immunetoemp/). |

## AI.INI

| In the Vinifera file | Change for OpenTS |
| --- | --- |
| An AI trigger's side field, other than `0` | Replace it with the position in `[Sides]`, counted from one, of the side that the intended country belongs to. Vinifera reads the field as a country's position in `[Houses]`, also counted from one; the two agree when every country has the same position in `[Houses]` as its side has in `[Sides]`. A trigger can no longer be limited to one country of a side that has several. See [AI triggers](/mapping/ai-triggers/). |

## Campaigns and maps

| In the Vinifera file | Change for OpenTS |
| --- | --- |
| `IntroMovie` in a campaign's section of `BATTLE.INI` | Not read. Name the movie `INTR<n>.VQA`, where `n` is the campaign's `CD=`. Every campaign whose `CD=` is below `2` plays an introduction, including one that played none in Vinifera because it was not an original campaign; see [The campaign level number](/systems/campaign-progression/#the-campaign-level-number). |
| A `[Tutorial]` key in `TUTORIAL.INI` or a scenario that is not a whole number | OpenTS skips the line. Number it, and name that number in the Text Trigger action; see [TUTORIAL.INI](/formats/tutorial-ini/). |

## THEME.INI

| In the Vinifera file | Change for OpenTS |
| --- | --- |
| `FadeOutSeconds` under `[General]` | Rename it to [`FadeOut`](/keys/fadeout/). OpenTS uses this fade for every stop and track change; Vinifera used it only when the music stopped, and cut to a queued track at once unless crossfading was on. |
| `CrossFading=yes` and `CrossFadeSeconds` under `[General]` | Replace both with [`CrossFade`](/keys/crossfade/) set to the `CrossFadeSeconds` value, or to `10` if the file leaves it out. OpenTS crossfades only into a queued track; Vinifera also faded in a track started at once. |
| A `[Themes]` label in `THEME01.INI` that `THEME.INI` also uses | Give it a label `THEME.INI` does not use. In OpenTS a reused label replaces the `THEME.INI` entry; see [THEME.INI](/formats/theme-ini/). |
| `Side=` for one track in both `THEME.INI` and `THEME01.INI` | Write the full list of sides in `THEME01.INI`. Vinifera adds the two lists together; OpenTS uses the `THEME01.INI` list. |
| A track's `Volume` written with `%`, or above `1` | Convert it to a fraction: `50%` becomes `0.5`. OpenTS ignores a value with `%` and reads a value above 1 as a percentage, where Vinifera plays it at full loudness. See [`Volume`](/keys/volume/#scope-themes). |

## SOUND.INI

| In the Vinifera file | Change for OpenTS |
| --- | --- |
| A `[SoundList]` label in `SOUND01.INI` that `SOUND.INI` also uses | Give it a label `SOUND.INI` does not use. In OpenTS a reused label replaces the `SOUND.INI` entry; see [`[SoundList]`](/formats/sound-ini/#soundlist). |
| `Volume` or `MinVolume` written with `%`, or above `1` | Convert it to a fraction: `50%` becomes `0.5`. OpenTS ignores a value with `%` and reads a value above 1 as a percentage, where Vinifera plays it at full loudness. See [`Volume`](/keys/volume/#scope-sounds). |
| `Delay` in a sound section | Vinifera reads the numbers as seconds. OpenTS reads whole numbers as milliseconds, so write `Delay=2.0,5.0` or `Delay=2000,5000` for two to five seconds. The silence also falls elsewhere: Vinifera waits after every sample unless `Control=` has `PREDELAY`, and then only before the first. OpenTS waits only between the cycles of a looping sound, and with `PREDELAY` once before the first sample as well. See [`Delay`](/keys/delay/). |
| `Limit=0` | Vinifera never plays the sound; OpenTS plays any number of copies. Set `Volume=0` to silence it. See [`Limit`](/keys/limit/). |
| `VShift` | Vinifera adds a random share of full loudness, so the sound plays louder. OpenTS only makes a sound quieter and has no setting that raises it at random. Remove the key or rewrite it as a quieter range; see [`VShift`](/keys/vshift/). |
| No `Range` in `[Defaults]` | Add `Range=10` to `[Defaults]` to keep Vinifera's default of 10 cells; OpenTS defaults to 28. See [`Range`](/keys/range/#scope-sounds). |
| No `Limit` in `[Defaults]` | Add `Limit=5` to `[Defaults]` to keep Vinifera's default of five copies at once; OpenTS defaults to 3. See [`Limit`](/keys/limit/). |
| Keys in `[Defaults]` other than `Limit`, `Range`, `Priority`, `Volume` and `MinVolume` | Vinifera ignores them, and OpenTS applies them to every sound that does not set them. Remove those you do not want applied; see [`[Defaults]`](/formats/sound-ini/#defaults). |
| A sound sample or music file present as both `.FLAC` and `.WAV` or `.OGG`, such as `GUN5.FLAC` and `GUN5.WAV` | Keep one. Vinifera uses the `.FLAC` file, and OpenTS the `.WAV` or `.OGG` file; see [How a sample is found](/formats/aud/#how-a-sample-is-found). |
