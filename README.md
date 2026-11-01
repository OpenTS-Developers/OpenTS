# siliconsun

siliconsun is an Apple Silicon client of *Command & Conquer: Tiberian Sun*,
and a modern Windows x64 client of the same engine - [OpenTS](https://github.com/OpenTS-Developers/OpenTS).

OpenTS rebuilds the Tiberian Sun engine as a standalone program instead of
patching the retail executable. siliconsun keeps that engine, adds an arm64
macOS build, and adds some QoL fixes.

What the engine runs, and how its Windows build works, is in the
[OpenTS guide](https://opents-developers.github.io/OpenTS/).
This file covers installing siliconsun and the two release clients.
[SETTINGS.md](SETTINGS.md) covers the settings this client adds.

---

> a very proud screenshot of the first successful menu build on MacBook Air M5 (macOS 27)

<img src="menu.png" alt="Tiberian Sun main menu running on Apple Silicon" width="900">

> Tiberian Sun - GDI Mission 1 (Reinforce Phoenix Base)
<img src="gameplay.png" alt="Tiberian Sun in-game on Apple Silicon" width="900">

## Install

Download the release for your platform. The game ships without Tiberian Sun's
data, so a launch whose data directory holds no `TIBSUN.MIX` of at least 10 MB
shows a setup screen first.

Choose **Download** to fetch the freeware files, or **Folder** to copy them
from an existing install:

- Tiberian Sun game data and music are always included.
- The campaign cinematics are optional.
- Firestorm game data and music are optional.
- The Firestorm cinematics are optional and also include Firestorm.

The screen shows the size of each part and the speed of a running transfer.
A partial download is kept and resumed, and a finished one with the wrong
checksum is removed. A finished setup offers Restart or Quit, and the next
launch starts the game.

The files are written into the data directory. They are Electronic Arts'
freeware release and are not covered by the siliconsun license. A `-DATADIR`
path that does not exist is refused, and headless or spawned runs do not show
the screen.

### Windows

`siliconsun-<tag>-x64.zip` holds `SUN.EXE`, `Language.dll`, `ui`, `Game.pdb`,
and the license notices. Unzip it into the Steam Tiberian Sun folder to
replace the original executable, or into any folder and run
`SUN.EXE -DATADIR=<path>` from there. The Steam Deck runs this build under
Proton 11.

> [!WARNING]
> Back up `Saved Games` before you replace `SUN.EXE`. OpenTS and the original
> game do not load each other's saves, and there is no converter. The guide's
> [compatibility and saves](https://opents-developers.github.io/OpenTS/using/compatibility-and-saves/)
> page explains the rest.

### macOS

`siliconsun-<tag>-macos.zip` holds `siliconsun.app` and the license notices.
It is an arm64 application. Finder and the Dock show the siliconsun icon.

The published app is signed with a Developer ID and notarized, so Gatekeeper
opens it normally. A copy that carries an ad-hoc signature, such as a local
build you share with someone, stays blocked until you run the commands below
from the folder that holds the app:

```bash
xattr -dr com.apple.quarantine siliconsun.app
open siliconsun.app
```

The app reads game data from `~/Games/OpenTS`, and an interactive launch
creates that folder when it is missing. Settings and saves go to
`~/Library/Application Support/siliconsun`. Replacing the app leaves both
folders in place. To use other folders:

```bash
open siliconsun.app --args -DATADIR="$HOME/Games/OpenTS" -USERDIR="$HOME/Library/Application Support/siliconsun"
```

The shell expands `$HOME` before the game sees the paths.

## siliconsun settings

Player options live in `SUN.INI`. [SETTINGS.md](SETTINGS.md) lists the keys this
client adds or changes, with the default for each and how it differs from
vanilla Tiberian Sun.

Double-click select, campaign intros, the now-playing line, Discord status,
skirmish persistence, and the skirmish-default override start on.

## Steam install

`.agents/skills/steam-release/` rebuilds the x64 Release and copies it onto the
Steam Tiberian Sun install on Windows, replacing `SUN.EXE`, `Language.dll`,
`Game.pdb`, and `ui/`. A build that is not x64 is refused.

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .agents\skills\steam-release\scripts\deploy.ps1
```

Back up `Saved Games` first, for the same reason as the download.

## Building

On macOS, with Apple Clang and Ninja on arm64:

```bash
brew install cmake ninja
git submodule update --init --recursive
cmake -S . -B build/macos -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build/macos
build/macos/bin/GameD -DATADIR="$HOME/Games/OpenTS"
```

The binary is `build/macos/bin/GameD`. On Windows, follow the guide's
[build and run](https://opents-developers.github.io/OpenTS/using/build-and-run/)
page and pass `-A x64`.

## Discord status

The game talks to the Discord desktop client on this computer over that
client's local IPC channel. There is no server, and no Discord library is in
the build. A normal build and a release build use application ID
`1565832094893211698`, set by the CMake cache variable
`SILICONSUN_DISCORD_APPLICATION_ID`. Set the variable to `0` and the game never
contacts Discord, even when `DiscordPresence=yes`. A build directory keeps the
value it was configured with until you configure again.

The large image is the Rich Presence art asset whose key is `logo`. Discord
stores that key in lowercase.

## Working on this fork

`main` holds this fork's commits on top of `upstream/main`. Update it by
rebasing, so the local commits stay one linear run:

```bash
git remote add upstream https://github.com/OpenTS-Developers/OpenTS.git   # first time only
git fetch upstream
git rebase upstream/main
```

Hide the paths this fork does not use instead of deleting them. A deletion
commit removes only the files that existed when you made it, and upstream adds
files under `manual/` in almost every commit, so a deleted tree partly returns
on the next rebase.

```powershell
git sparse-checkout init --cone
git sparse-checkout set .agents .claude .cursor .github .vscode Run cmake code docs tests thirdparty ui workers
```

That hides `manual/`: the Astro site, its pages, the change records, and the
tools behind them. The files stay tracked, so
`git show main:manual/changes/<name>.md` reads one, and writing a new record
needs `git sparse-checkout add manual` first. `git sparse-checkout disable`
restores the whole tree.

Renaming or deleting a path that upstream still edits conflicts the next time
upstream changes it. Put product naming in text and artwork instead:
`README.md`, the window title, `project()` in `CMakeLists.txt`, and the icon.
Paths under `code/`, `docs/`, `tests/`, and `manual/` keep their upstream
names.

A path that `git ls-tree upstream/main -- <path>` does not list belongs to this
fork. These are the main ones:

| Path | Owner |
| --- | --- |
| `workers/`, `.agents/skills/`, `README.md`, `SETTINGS.md`, `.github/workflows/engine-release.yml` | this fork |
| `code/content*`, `code/discord*`, `code/nowplaying*`, `code/platform.*`, `code/ui/screens/install/`, `code/resources/app-icon/siliconsun.*` | this fork |
| `manual/`, `docs/`, `tests/`, the other `.github/workflows/` | upstream |

## License

This fork keeps the upstream license: GPL-3.0-or-later, with the additional
Section 7 terms for material derived from Electronic Arts source. See
[LICENSE.md](LICENSE.md).
