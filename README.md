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

The keys below are the ones this client adds or changes. A missing
`[siliconsun]`, `TextBackgroundColor`, or `CursorScale` key keeps the default
in the table. A `[Skirmish]` key is absent until the skirmish screen writes
it, and a missing one keeps the value already chosen. `yes`, `true`, and `1`
turn a switch on. `no`, `false`, and `0` turn it off. `off` leaves the
default in place. Double-click select, campaign intros, the now-playing line,
Discord status, skirmish persistence, and the skirmish-default override start on.

Leaving the options menu writes the `[siliconsun]` keys, `TextBackgroundColor`,
and `CursorScale` back, with a switch stored as `yes` or `no`. Closing the
skirmish screen writes `[Skirmish]`, and only while `PersistSkirmishSettings`
is on. Turning that switch off leaves a `[Skirmish]` section already in the
file unread. Turning it back on restores that section.

A saved setup wins over the `SkirmishDefault*` values. With nothing saved,
`OverrideSkirmishDefaults=yes` uses those values instead of the engine
defaults. Every number is then pulled into the range the screen offers:
credits from 0 to the rules' maximum money, unit count from 1 to 10, tech
level from 1 to 10, game speed from 0 to 6, AI players from 1 to 7, and AI
level from 0 to 2.

| Setting | Default | What it does | Vanilla Tiberian Sun |
| --- | --- | --- | --- |
| `[siliconsun]` `DoubleClickSelectType` | `yes` | Double-clicking one of your own units selects every on-screen unit of that type, the same selection as Select Same Type. The first click still selects only that unit. A double-click on an enemy, on empty ground, or under fog stays an ordinary click. | A double-click stays an ordinary click. |
| `[siliconsun]` `CampaignIntroMovie` | `yes` | A campaign introduction plays from that campaign's movie archive. Disc 0 prefers `MOVIES01.MIX` and disc 1 prefers `MOVIES02.MIX`. It still plays only when the first mission starts, not on a restart, and only when that campaign's disc number is below 2. `INTR<n>.VQA` plays when that file is present; otherwise `INTRO.VQA` plays. | With both campaigns installed, Nod plays the GDI introduction. Every campaign takes `INTRO.VQA` from the first movie archive. |
| `[siliconsun]` `NowPlaying` | `yes` | While a scenario is running, the bottom left shows `Now Playing:` and the track title for the same time as an alert, in your color. On macOS, while the window is focused, the media next and previous keys skip tracks and the system shows that title. Those keys do the same on Windows when the game receives them. `no` leaves the line off and leaves the media keys unused. | There is no now-playing line, and the media keys do nothing. |
| `[siliconsun]` `DiscordPresence` | `yes` | While the Discord desktop client is running on this machine, your profile shows the menus, a skirmish, a campaign mission, or a multiplayer game. A scenario adds the map name. GDI or Nod is shown for those sides, and a skirmish also shows how many AI opponents it has. The clock runs from when that activity started. The large image is the art asset `logo`. `no` does not contact Discord. If Discord is not running, the game continues and shows no error. Under Proton, the Linux Discord client usually cannot see the Windows build. | There is no Discord status. |
| `[siliconsun]` `PersistSkirmishSettings` | `yes` | The skirmish screen reopens on the map and options it held when it last closed, including when you back out. Those values are the `[Skirmish]` keys. A saved map that is gone falls back to the first scenario. `no` opens from the defaults every time and does not write `[Skirmish]`. | The screen opens from the engine defaults every time. The setup is not kept in `SUN.INI`. |
| `[siliconsun]` `OverrideSkirmishDefaults` | `yes` | A skirmish with nothing saved starts from the `SkirmishDefault*` values. A saved setup still wins while persistence is on. `no` uses the engine defaults. | The screen starts from the engine defaults. There is no override key. |
| `[siliconsun]` `SkirmishDefaultCredits` | `5000` | Starting credits when the override is on and nothing is saved. | The screen starts from `[MultiplayerDefaults] Money` in the rules. The engine uses 3000 when that key is absent. |
| `[siliconsun]` `SkirmishDefaultUnitCount` | `1` | Starting unit count, from 1 to 10. | The screen starts from `[MultiplayerDefaults] UnitCount`. The engine uses 10 when that key is absent. |
| `[siliconsun]` `SkirmishDefaultTechLevel` | `10` | Starting tech level, from 1 to 10. | Same starting level as `[MultiplayerDefaults] TechLevel` when the rules omit it, which is 10. |
| `[siliconsun]` `SkirmishDefaultGameSpeed` | `3` | Same scale as `GameSpeed`: 0 is the fastest and 6 is the slowest. The skirmish slider shows 6 minus this number, so 3 is shown as 3, 0 as 6, and 6 as 0. | The stored speed starts at 0, the fastest, and the slider shows 6. |
| `[siliconsun]` `SkirmishDefaultAIPlayers` | `1` | Starting number of AI opponents, from 1 to 7. | The screen shows at least one. A stored 0 is raised to 1. |
| `[siliconsun]` `SkirmishDefaultAILevel` | `0` | AI difficulty. 0 is the easiest, 1 is normal, and 2 is the hardest. | The engine starts at 1, normal. |
| `[siliconsun]` `SkirmishDefaultBases` | `yes` | Bases start on. | The screen starts from `[MultiplayerDefaults] Bases`, which is yes when that key is absent. |
| `[siliconsun]` `SkirmishDefaultCrates` | `yes` | Crates start on. | The screen starts from `[MultiplayerDefaults] Crates`, which is yes when that key is absent. |
| `[siliconsun]` `SkirmishDefaultFog` | `no` | Fog of war starts off. | The engine starts with fog off. |
| `[siliconsun]` `SkirmishDefaultBridges` | `yes` | Destroyable bridges start on. | The screen starts from `[MultiplayerDefaults] BridgeDestruction`, which is yes when that key is absent. |
| `[siliconsun]` `SkirmishDefaultRedeploy` | `no` | The re-deployable MCV starts off. | The engine starts with re-deploy off. |
| `[siliconsun]` `SkirmishDefaultShortGame` | `yes` | Short game starts on. | The engine starts with short game off. |
| `[siliconsun]` `SkirmishDefaultMultiEngineer` | `no` | Multi-engineer starts off. | The engine starts with multi-engineer off. |
| `[Skirmish]` `Map` | absent | The scenario file the screen had selected. A missing file, or a name that is not in the list, selects the first scenario. | Vanilla does not store it. |
| `[Skirmish]` `Credits` | absent | The credits the screen last held. A missing key keeps the value already chosen. | Vanilla does not store it. |
| `[Skirmish]` `UnitCount` | absent | The unit count the screen last held. A missing key keeps the value already chosen. | Vanilla does not store it. |
| `[Skirmish]` `TechLevel` | absent | The tech level the screen last held. A missing key keeps the value already chosen. | Vanilla does not store it. |
| `[Skirmish]` `GameSpeed` | absent | The stored game speed, on the same scale as `SkirmishDefaultGameSpeed`. A missing key keeps the value already chosen. | Vanilla does not store it. |
| `[Skirmish]` `AIPlayers` | absent | The number of AI opponents the screen last held. A missing key keeps the value already chosen. | Vanilla does not store it. |
| `[Skirmish]` `AIDifficulty` | absent | The AI level the screen last held: 0 easiest, 1 normal, 2 hardest. The screen labels this AI Level. A missing key keeps the value already chosen. | Vanilla does not store it. |
| `[Skirmish]` `Bases` | absent | Bases, as the screen last left them. A missing key keeps the value already chosen. | Vanilla does not store it. |
| `[Skirmish]` `Crates` | absent | Crates, as the screen last left them. A missing key keeps the value already chosen. | Vanilla does not store it. |
| `[Skirmish]` `FogOfWar` | absent | Fog of war, as the screen last left it. A missing key keeps the value already chosen. | Vanilla does not store it. |
| `[Skirmish]` `BridgeDestroy` | absent | Destroyable bridges, as the screen last left them. A missing key keeps the value already chosen. | Vanilla does not store it. |
| `[Skirmish]` `MCVRedeploy` | absent | The re-deployable MCV, as the screen last left it. A missing key keeps the value already chosen. | Vanilla does not store it. |
| `[Skirmish]` `ShortGame` | absent | Short game, as the screen last left it. A missing key keeps the value already chosen. | Vanilla does not store it. |
| `[Skirmish]` `MultiEngineer` | absent | Multi-engineer, as the screen last left it. A missing key keeps the value already chosen. | Vanilla does not store it. |
| `[Options]` `TextBackgroundColor` | `0` | The palette index drawn behind in-game message text and the now-playing line. `0` draws none. `12` is black. | Vanilla draws messages with no box and has no key for one. OpenTS uses `12`, black, when the key is missing. A file that still says `12` keeps that box; set it to `0`. No key named `TransparentMessageText` is read. |
| `[Video]` `CursorScale` | `0` | `0` grows the pointer with the picture. On Windows, a display scale larger than that growth is used instead, so a 3840 by 2160 picture on a display set to 200% gets a double-size pointer. A positive number is a fixed multiple, at most 8. A negative number keeps the artwork's size. | Vanilla has no key, and the pointer stays at the size of its artwork. OpenTS already grows a `0` pointer with the picture. The Windows display scale is the part this client adds. |

The display options also list the primary display's width and height divided
by 2, 3, 4, 5, and 6, when that size is still at least 640 by 400. On Windows,
when the display scale is above 1, they also list the size that scale
corresponds to. A 1280 by 800 panel includes 640 by 400, and a 3840 by 2160
panel at 200% includes 1920 by 1080. Choosing one renders at that size. There
is no key for the list. The size you keep is written as `ScreenWidth` and
`ScreenHeight` under `[Video]`. Vanilla lists the modes the display reports.
These sizes are added on top of that list, and a mode still has to fall
between 640 by 400 and 4096 by 4096.

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

A published release includes `siliconsun-<tag>-macos.zip`: `siliconsun.app` and
the license notices. It is an arm64 application. Finder shows the siliconsun
icon, and the Dock shows that icon while the game is running.

Gatekeeper opens a notarized app. It blocks an ad-hoc signed app until the
quarantine attribute is removed:

```bash
xattr -dr com.apple.quarantine siliconsun.app
open siliconsun.app
```

Run those commands from the unzipped folder.

Game data is read from `~/Games/OpenTS`. That folder has to exist already; the
retail install script puts it there. Settings and saves go to
`~/Library/Application Support/siliconsun`, and saves are in `Saved Games`
inside it. Replacing the app leaves that folder in place.

The `ui` directory is `siliconsun.app/Contents/Resources/ui`. Debug logs and
crash reports are written under `siliconsun.app/Contents/MacOS`. Replacing
the app removes those logs and reports.

To point this launch at other folders:

```bash
open siliconsun.app --args -DATADIR="$HOME/Games/OpenTS" -USERDIR="$HOME/Library/Application Support/siliconsun"
```

The shell expands `$HOME` before the game sees the paths.

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

## Discord status

The game talks to the Discord desktop client on this computer over that
client's local IPC channel. There is no server, no Discord library in the
build, and the application ID is public. The release zips do not add a
runtime file for it.

A normal build and a release build use application ID `1565832094893211698`.
That number is the CMake cache variable `SILICONSUN_DISCORD_APPLICATION_ID`.
CMake remembers a value you pass, so a later configure keeps it. A configure
that does not pass the variable uses this ID. Set the variable to `0` and the
game never contacts Discord, even when `DiscordPresence=yes`. A build
directory configured while the variable was `0` keeps that cached value until
you configure again.

The large image is the Rich Presence art asset whose key is `logo`. Discord
stores that key in lowercase.

## Building on Windows

The Windows client published here is 64-bit (x64). Follow the guide's
[build and run](https://opents-developers.github.io/OpenTS/using/build-and-run/)
page and pass `-A x64`.

## License

This fork keeps the upstream license: GPL-3.0-or-later, with the additional
Section 7 terms for material derived from Electronic Arts source. See
[LICENSE.md](LICENSE.md).
