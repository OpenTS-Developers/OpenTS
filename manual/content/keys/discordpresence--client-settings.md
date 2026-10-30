---
key: DiscordPresence
scope: client-settings
label: Discord status
summary: Shows the current activity on the local Discord desktop client.
when_omitted:
  kind: value
  value: "yes"
---

`DiscordPresence=yes` shows the current activity on the Discord desktop client running on this machine. The profile says whether you are in the menus, a skirmish, a campaign mission, or a multiplayer game. A scenario adds that scenario's name. A scenario with no name adds the file name, without its extension. GDI or Nod is shown when you are playing that side. A skirmish also shows how many computer opponents it has. Discord counts the time from when that activity started. The large image is the application's art asset `logo`.

While the menus stay up, the clock keeps running. Starting a scenario, restarting it, or loading a save starts that clock from the new start.

`no` does not open a connection to Discord. A Discord client that is not running, and a build configured with application ID `0`, also leave the game running with no message. Under Proton, the Linux Discord client usually cannot see the Windows build.

The key is read from `[siliconsun]` in `SUN.INI` at startup. No dialog offers it. Leaving the options menu writes the value currently in force.
