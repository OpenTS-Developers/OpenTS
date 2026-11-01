# siliconsun settings

Player options live in `SUN.INI`. Where that file is written is on the guide's
[configuration files](https://opents-developers.github.io/OpenTS/using/configuration-files/)
and [game data](https://opents-developers.github.io/OpenTS/using/game-data/#keeping-the-data-somewhere-else)
pages.

Every `[siliconsun]` key below is new; vanilla Tiberian Sun has none of them.
A missing key keeps the default in the table. `yes`, `true`, and `1` turn a
switch on; `no`, `false`, and `0` turn it off; `off` keeps the default. Leaving
the options menu writes these keys, `TextBackgroundColor`, and `CursorScale`
back, with a switch stored as `yes` or `no`.

## Client settings

| Setting | Default | What it does |
| --- | --- | --- |
| `DoubleClickSelectType` | `yes` | Double-clicking your own unit selects every on-screen unit of that type, the same as Select Same Type. The first click selects one. An enemy, empty ground, or fog stays an ordinary click. |
| `CampaignIntroMovie` | `yes` | Each campaign plays its own introduction from its movie archive: `MOVIES01.MIX` for disc 0 and `MOVIES02.MIX` for disc 1. It plays when the first mission starts, not on a restart. `INTR<n>.VQA` is used when present, otherwise `INTRO.VQA`. |
| `NowPlaying` | `yes` | The bottom left shows the track title while a scenario runs. On macOS, the media next and previous keys skip tracks while the window is focused; on Windows they work when the game receives them. |
| `DiscordPresence` | `yes` | Your profile shows the current menu, skirmish, campaign mission, or multiplayer game while the Discord desktop client runs. A scenario adds the map name, and a skirmish adds the AI opponent count. The large image is the `logo` art asset. If Discord is not running, the game continues without an error; under Proton the Linux client usually cannot see the Windows build. |
| `PersistSkirmishSettings` | `yes` | The skirmish screen reopens on the map and options it last held, including after you back out. A saved map that is gone falls back to the first scenario. `no` opens from the defaults and does not write `[Skirmish]`. |
| `OverrideSkirmishDefaults` | `yes` | A skirmish with nothing saved starts from the `SkirmishDefault*` values. A saved setup still wins. |

## Skirmish defaults

Used when `OverrideSkirmishDefaults` is on and nothing is saved. Each value is
pulled into the range the screen offers: credits from 0 to the rules' maximum
money, unit count and tech level from 1 to 10, game speed from 0 to 6, AI
players from 1 to 7, and AI level from 0 to 2.

| Setting | Default | Meaning |
| --- | --- | --- |
| `SkirmishDefaultCredits` | `5000` | Starting credits. |
| `SkirmishDefaultUnitCount` | `1` | Starting unit count. |
| `SkirmishDefaultTechLevel` | `10` | Starting tech level. |
| `SkirmishDefaultGameSpeed` | `3` | Game speed, where 0 is fastest. The slider shows 6 minus this value. |
| `SkirmishDefaultAIPlayers` | `1` | Number of AI opponents. |
| `SkirmishDefaultAILevel` | `0` | AI difficulty: 0 easiest, 1 normal, 2 hardest. |
| `SkirmishDefaultBases` | `yes` | Bases start on. |
| `SkirmishDefaultCrates` | `yes` | Crates start on. |
| `SkirmishDefaultFog` | `no` | Fog of war starts off. |
| `SkirmishDefaultBridges` | `yes` | Destroyable bridges start on. |
| `SkirmishDefaultRedeploy` | `no` | The re-deployable MCV starts off. |
| `SkirmishDefaultShortGame` | `yes` | Short game starts on. |
| `SkirmishDefaultMultiEngineer` | `no` | Multi-engineer starts off. |

## Skirmish memory

With `PersistSkirmishSettings` on, closing the skirmish screen writes
`[Skirmish]`: `Map`, `Credits`, `UnitCount`, `TechLevel`, `GameSpeed`,
`AIPlayers`, `AIDifficulty`, `Bases`, `Crates`, `FogOfWar`, `BridgeDestroy`,
`MCVRedeploy`, `ShortGame`, and `MultiEngineer`. A missing key keeps the value
already chosen, and a map that is gone selects the first scenario. Turning
persistence off leaves an existing `[Skirmish]` section unread, and turning it
back on uses that section again.

## Display

| Setting | Default | What it does |
| --- | --- | --- |
| `[Options]` `TextBackgroundColor` | `0` | Palette index drawn behind in-game messages and the now-playing line. `0` draws none and `12` is black. OpenTS uses `12` when the key is missing, so set `0` to remove the box. |
| `[Video]` `CursorScale` | `0` | `0` grows the pointer with the picture. On Windows, a larger display scale overrides that growth, so a 3840 by 2160 picture at 200% gets a double-size pointer. A positive number is a fixed multiple up to 8; a negative number keeps the artwork's size. |

The display options list the primary display's own size and its whole
fractions: the width and height divided by 2, 3, 4, 5, and 6, kept only when
both divide exactly and the result is still at least 640 by 400. Each entry is
marked `(native)` or `(1/2)` and so on, and a mode still has to fall between
640 by 400 and 4096 by 4096. Choosing one renders at that size and writes
`ScreenWidth` and `ScreenHeight` under `[Video]`.

A first launch with either key missing or set to `-1` chooses the largest
whole fraction whose picture fits within 960 by 540, or the display's own size
when none does. A 1920 by 1080 or 3840 by 2160 display therefore opens at 960
by 540, and a 1280 by 800 display at 640 by 400.
