---
key: DiscordPresence
scope: client-settings
label: Discord status
summary: Shows the current activity on the local Discord desktop client.
when_omitted:
  kind: value
  value: "yes"
---

`DiscordPresence=yes` shows the current activity on the Discord desktop client running on this machine. In the menus the profile says `In menus`. In a game the first line names the game, `Tiberian Sun` or `Tiberian Sun - Firestorm`, and the match: `Campaign`, or the sides written largest first, such as `1v1`, `1v1v1`, `2v2`, or `2v1v1`. The second line is the side and the scenario name, as in `GDI - Map Name`. A scenario with no name uses the file name, without its extension. A side is the houses allied with each other in both directions, leaving out observers, defeated houses and houses of a passive country. When the houses still playing are one side, or fewer than two, a skirmish says `Skirmish` and a network game says `Multiplayer`. Discord counts the time from when that activity started. The large image is the application's art asset `logo`.

While the menus stay up, the clock keeps running. Starting a scenario, restarting it, or loading a save starts that clock from the new start.

`no` does not open a connection to Discord. A Discord client that is not running, and a build configured with application ID `0`, also leave the game running with no message. Under Proton, the Linux Discord client usually cannot see the Windows build.

The key is read from `[siliconsun]` in `SUN.INI` at startup. No dialog offers it. Leaving the options menu writes the value currently in force.
