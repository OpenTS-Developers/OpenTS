---
name: Retail install
description: Copy Tiberian Sun game data from retail discs, ISOs, or an installed copy into a playable folder such as ~/Games/OpenTS, with optional movies, music, and Firestorm.
---

# Retail install

Use this skill when the user wants a playable folder built from game data they
already own: retail discs, disc images, or an installed copy of the game.

## Preconditions

- Stop when the user cannot point at a legitimately owned copy. Do not download
  game data.
- Close the game before writing into an install folder.
- Ask what the source is when the request does not say: a disc in a drive, an
  ISO file, an extracted disc, or an installed copy from Steam, GOG, or The
  First Decade.

## Sources

| Source | Pass to `-Source` |
| --- | --- |
| Disc in a drive | The drive letter, such as `E:\` |
| ISO file | The file path; the script mounts it and dismounts it afterwards. Mounting needs an elevated shell. |
| Extracted disc | The folder that holds `INSTALL\TIBSUN.MIX` |
| Installed copy | Its install folder |

Pass several sources to combine discs, for example the GDI disc, the Nod disc,
and the Firestorm disc. Later sources replace files with the same name, so pass
the Firestorm disc last. Without `-Source`, the script uses every readable disc
drive.

A retail disc keeps the base archives inside `INSTALL\TIBSUN.MIX`; its root
holds the map, movie, music, and mission archives. The script copies that file
as `TIBSUN.MIX`, the name the Steam and First Decade installs use. The game
opens the archives inside it. If the game reports missing data from a disc
install, pass the folder the disc's own setup installs and copy from there.

## Command

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .agents\skills\retail-install\scripts\install-retail.ps1 -Source E:\
```

Defaults: target `~/Games/OpenTS`, the x64 Release in `build\x64\bin\Release`
when it exists, and every file group the source holds.

| Parameter | Effect |
| --- | --- |
| `-Source <path[]>` | Folders or ISO files to read. Defaults to readable disc drives. |
| `-TargetDir <path>` | Destination folder. Defaults to `~/Games/OpenTS`. Same-name files are replaced; only `ui/` is deleted. |
| `-BuildDir <path>` | Build output to copy beside the data. Defaults to `build\x64\bin\Release`. |
| `-NoBuild` | Copy game data only. Run a build with `-DATADIR=<target>` instead. |
| `-NoMovies` | Skip `MOVIES*.MIX` and loose `.VQA` files. The Steam copy of these is 1,399.2 MB. |
| `-NoMusic` | Skip `SCORES.MIX` and `SCORES01.MIX`. The Steam copies are 68.1 MB. |
| `-NoFirestorm` | Skip the expansion archives, including `SOUNDS01.MIX`. |
| `-WhatIf` | Print the copy plan without writing anything. |

## What it copies

- **Core:** `TIBSUN.MIX` or the loose `CACHE.MIX`, `CONQUER.MIX`, and
  `SOUNDS.MIX`; `LOCAL.MIX`, `MULTI.MIX`, `PATCH.MIX`, and `PCACHE.MIX`; the
  `MAPS`, `SIDEC`, `SIDENC`, `SIDECD`, and `SPEECH` archives; the theater
  archives; and loose `.map` files.
- **Movies:** `MOVIES*.MIX` and loose `.VQA` files.
- **Music:** `SCORES.MIX` and `SCORES01.MIX`.
- **Firestorm:** `EXPAND` and `ECACHE` archives, the `E??SC??`, `E??SNC??`,
  `E??SCD??`, and `E??VOX??` archives, `SOUNDS01.MIX`, `GMENU.MIX`, and
  `WDT*.MIX`.

Firestorm data on a disc may sit inside its installer, which the script does not
open. It warns when it finds the Firestorm side archives without `EXPAND01.MIX`;
pass the folder the Firestorm setup installs instead.

[`MIX archives`](../../../manual/content/formats/mix.md) owns the mount order
and states which of these the game requires.

## When it stops

- No source holds `TIBSUN.MIX`, and none holds `CACHE.MIX`, `CONQUER.MIX`, and
  `SOUNDS.MIX` together. Report the source the user passed and ask for the
  installed game folder.
- `EXPAND01.MIX` is copied without `SOUNDS01.MIX` in a loose layout. The game
  stops during startup. Pass the folder that holds `SOUNDS01.MIX`, or use
  `-NoFirestorm`.
- The build output is not x64. Point `-BuildDir` at `build\x64\bin\Release` or
  use `-NoBuild`; do not mix a Win32 `Language.dll` with an x64 executable.

## Verify and report

The script prints the file count and size per group, the skipped or missing
groups, the build files, and the command that starts the game. Report those
values. A copied file set is not runtime evidence: start the game once from the
target and confirm it reaches the main menu.

Saves, settings, and logs are written beside the executable unless the game is
started with `-USERDIR`. Mention this when the target is meant to be read-only
or shared.

## Compatibility

The x64 client cannot read Win32 saves or join a Win32 network game.
[`Building`](../../../docs/BUILDING.md) explains why, and `-NoBuild` lets one data
folder serve both builds. Rerunning the script after an update keeps files
whose size already matches and replaces the rest.
