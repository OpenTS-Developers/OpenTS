# siliconsun

siliconsun is an Apple Silicon client of *Command & Conquer: Tiberian Sun*,
and a modern Windows x64 client of the same engine - [OpenTS](https://github.com/OpenTS-Developers/OpenTS).

### What this is

OpenTS rebuilds the Tiberian Sun engine as a standalone program instead of
patching the retail executable. siliconsun keeps that engine, adds an arm64
macOS build, and adds some QoL fixes.

What the engine runs, and how its Windows build works, is in the
[OpenTS guide](https://opents-developers.github.io/OpenTS/).
This file covers the Apple Silicon build, the two release clients, the
freeware install, and the settings siliconsun adds.

---

> a very proud screenshot of the first successful menu build on MacBook Air M5 (macOS 27)

<img src="menu.png" alt="Tiberian Sun main menu running on Apple Silicon" width="900">

> Tiberian Sun - GDI Mission 1 (Reinforce Phoenix Base)
<img src="gameplay.png" alt="Tiberian Sun in-game on Apple Silicon" width="900">

## siliconsun settings

Player options live in `SUN.INI`. Where that file is written is on the guide's
[configuration files](https://opents-developers.github.io/OpenTS/using/configuration-files/)
and [game data](https://opents-developers.github.io/OpenTS/using/game-data/#keeping-the-data-somewhere-else)
pages.

siliconsun reads its own keys from a `[siliconsun]` section in that file. 
Each key defaults to `yes`. A missing key, or a missing section, leaves the feature on.
`no`, `false`, and `0` turn that one feature off. 
Leaving the options menu writes the section back out.

```ini
[siliconsun]
DoubleClickSelectType=yes
CampaignIntroMovie=yes
```

`DoubleClickSelectType=yes` selects every on-screen unit of one type when you
double-click one of your own, the same selection as Select Same Type.

`CampaignIntroMovie=yes` plays each campaign's own introduction when that
introduction plays at all: the first mission of a campaign whose disc number
is below 2, and not on a restart. Disc 0 prefers `MOVIES01.MIX` and disc 1
prefers `MOVIES02.MIX`, so Nod plays the Nod introduction when both campaigns
are installed.

In-game messages have no background when `TextBackgroundColor` under
`[Options]` is missing or `0`. The guide's
[TextBackgroundColor](https://opents-developers.github.io/OpenTS/keys/textbackgroundcolor/)
page still treats a missing value as `12`, the black box. Here a missing
value is `0`. A saved file that still has `12` keeps that box; set it to `0`.

```ini
[Options]
TextBackgroundColor=0
```

---

## How to get the game data

The guide's [game data](https://opents-developers.github.io/OpenTS/using/game-data/)
page is for a developer build that reads an existing install from `Run/`.
siliconsun ships the engine only. Download the freeware GDI, Nod, and Firestorm
disc images from the
[Command & Conquer Communications Center](https://cnc-comm.com/tiberian-sun/downloads/the-game):

| Disc | Page | File |
| --- | --- | --- |
| GDI | [GDI CD](https://cnc-comm.com/tiberian-sun/downloads/the-game/gdi-disc) | [TS_GDI.zip](https://downloads.cnc-comm.com/tiberian-sun/iso/TS_GDI.zip) |
| Nod | [Nod CD](https://cnc-comm.com/tiberian-sun/downloads/the-game/nod-disc) | [TS_Nod.zip](https://downloads.cnc-comm.com/tiberian-sun/iso/TS_Nod.zip) |
| Firestorm | [Firestorm CD](https://cnc-comm.com/tiberian-sun/downloads/the-game/firestorm-disc) | [TS_Firestorm.zip](https://downloads.cnc-comm.com/tiberian-sun/iso/TS_Firestorm.zip) |

Ask an agent to run the `retail-install` skill, or run the script yourself.
Pass Firestorm last. A mounted disc, an extracted disc, or an existing install
folder is a source too. The files land in `~/Games/OpenTS`.

```bash
python3 .agents/skills/retail-install/scripts/install-retail.py \
  ~/Downloads/TS_GDI.zip ~/Downloads/TS_Nod.zip ~/Downloads/TS_Firestorm.zip
```

`--no-movies`, `--no-music`, and `--no-firestorm` leave those groups out.
`--dry-run` prints the plan. On Windows, run
`.agents/skills/retail-install/scripts/install-retail.ps1` instead.

## Windows download

The x64 release is a drop-in replacement for the Steam install.

> [!WARNING]
> Back up the `Saved Games` folder before you replace `SUN.EXE`. OpenTS and
> the original game do not load each other's saves.
> [Compatibility and save games](https://opents-developers.github.io/OpenTS/using/compatibility-and-saves/)
> covers that, and there is no converter.

A published release includes `siliconsun-<tag>-x64.zip`: `SUN.EXE`, `Language.dll`,
`ui`, `Game.pdb`, and the license notices. Copy `SUN.EXE`, `Language.dll`,
`Game.pdb`, and `ui` into the Steam Tiberian Sun folder, or run
`SUN.EXE -DATADIR=<path>` from the unzipped folder. The Steam Deck and the
Victus both run this build. On the Deck, use Proton 11.

## macOS download

A published release includes `siliconsun-<tag>-macos.zip`: `Game`, `ui`, and the
license notices. It is an unsigned arm64 binary. `ui` has to stay beside
`Game`. After unzipping, clear quarantine and point the binary at the game
data:

```bash
xattr -dr com.apple.quarantine .
./Game -DATADIR="$HOME/Games/OpenTS"
```

Run that from the unzipped folder. The shell expands `$HOME` before the game
sees the path. The folder has to exist already.

## Windows Steam install

Back up `Saved Games` before you deploy, for the same reason as the Windows
download.

`.agents/skills/steam-release/` rebuilds the x64 Release and copies it onto the
Steam Tiberian Sun install on Windows, replacing `SUN.EXE`, `Language.dll`,
`Game.pdb`, and `ui/`. A build that is not x64 is refused. The Steam Deck runs
that Windows build under Proton 11.

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .agents\skills\steam-release\scripts\deploy.ps1
```

---

---

## Building on macOS (Apple Silicon)

The macOS build is the one I develop with. It builds with Apple Clang and
Ninja on arm64. The guide's
[build and run](https://opents-developers.github.io/OpenTS/using/build-and-run/)
page is Visual Studio on Windows, and it has no arm64 macOS path.

1. Install CMake and Ninja:

   ```bash
   brew install cmake ninja
   ```

2. Fetch the submodules:

   ```bash
   git submodule update --init --recursive
   ```

3. Configure and build a Debug binary:

   ```bash
   cmake -S . -B build/macos -G Ninja \
     -DCMAKE_BUILD_TYPE=Debug \
     -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
   cmake --build build/macos
   ```

   The binary is `build/macos/bin/GameD`.

4. Point the game at a Tiberian Sun data directory and run:

   ```bash
   build/macos/bin/GameD -DATADIR="$HOME/Games/OpenTS"
   ```

---

## Building on Windows

The Windows client published here is 64-bit (x64). Follow the guide's
[build and run](https://opents-developers.github.io/OpenTS/using/build-and-run/)
page and pass `-A x64`.

## License

This fork keeps the upstream license: GPL-3.0-or-later, with the additional
Section 7 terms for material derived from Electronic Arts source. See
[LICENSE.md](LICENSE.md).
