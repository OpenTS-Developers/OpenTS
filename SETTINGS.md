# siliconsun settings

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
