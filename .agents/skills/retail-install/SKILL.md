---
name: Retail install
description: Copy Tiberian Sun and Firestorm data from the freeware GDI, Nod, and Firestorm disc zips, an ISO, or an install folder into ~/Games/OpenTS. On macOS run install-retail.py. On Windows run install-retail.ps1.
---

# Retail install

Copy game data into `~/Games/OpenTS`, then start the macOS build with
`-DATADIR` pointed at that folder.

Download all three freeware discs. Pass Firestorm last so its files win when
names clash. Pages and files are listed at
<https://cnc-comm.com/tiberian-sun/downloads/the-game>.

| Disc | Zip |
| --- | --- |
| GDI | <https://downloads.cnc-comm.com/tiberian-sun/iso/TS_GDI.zip> |
| Nod | <https://downloads.cnc-comm.com/tiberian-sun/iso/TS_Nod.zip> |
| Firestorm | <https://downloads.cnc-comm.com/tiberian-sun/iso/TS_Firestorm.zip> |

A mounted disc, an extracted disc, or a Steam, GOG, or First Decade install
folder is a source too. Do not fetch any other game data.

## macOS

```bash
python3 .agents/skills/retail-install/scripts/install-retail.py \
  ~/Downloads/TS_GDI.zip ~/Downloads/TS_Nod.zip ~/Downloads/TS_Firestorm.zip
```

The script unzips each archive, mounts the ISO with `hdiutil`, and copies the
game files to the target root. It leaves the engine binary in the build tree.

```bash
build/macos/bin/GameD -DATADIR="$HOME/Games/OpenTS"
```

| Flag | Effect |
| --- | --- |
| `--target <dir>` | Destination. Default `~/Games/OpenTS`. Same-name files are replaced when the size differs. |
| `--no-movies` | Skip `MOVIES??.MIX` and loose `.VQA` files. |
| `--no-music` | Skip `SCORES.MIX` and `SCORES01.MIX`. |
| `--no-firestorm` | Skip the expansion archives, including `SOUNDS01.MIX`. |
| `--dry-run` | Print the plan and write nothing to the target. |

Report the per-group counts the script prints. It exits when the sources have
neither `TIBSUN.MIX` nor `CACHE.MIX`, `CONQUER.MIX`, and `SOUNDS.MIX` together,
and when a loose Firestorm layout has `EXPAND01.MIX` without `SOUNDS01.MIX`.
A warning that `EXPAND01.MIX` is missing means the campaign data is probably
still inside the disc installer; pass the folder that setup installs.

## Windows

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .agents\skills\retail-install\scripts\install-retail.ps1 -Source E:\
```

Same file groups. `-TargetDir`, `-NoMovies`, `-NoMusic`, `-NoFirestorm`, and
`-WhatIf` match the macOS flags. The script mounts an ISO only in an elevated
shell and does not unzip. Without `-NoBuild`, an existing x64
`build\x64\bin\Release` is copied beside the data; a Win32 build is refused.
