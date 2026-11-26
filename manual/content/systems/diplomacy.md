---
title: Diplomacy
summary: "A panel over the game that lists every player with their kills, alliances, shared control, chat settings and connection, and lets the player change their side of each."
category: multiplayer-networking
keys: []
related:
  - type: command
    id: Diplomacy
  - type: command
    id: ToggleAlliance
  - type: command
    id: GrantControl
  - type: system
    id: chat
  - type: system
    id: shared-control
  - type: system
    id: observers
  - type: system
    id: reconnect-dialog
---

The [`Diplomacy`](/commands/diplomacy/) command opens the diplomacy panel in a skirmish or a game against other machines. It opens nothing in a campaign or while another screen or menu is up.

The game keeps running while the panel is up. A click outside the panel goes to the map, and keys other than Tab and Escape go to the game as usual. While a chat line is being typed, every key goes to the line.

The panel closes on Tab, Escape, its Close button, or the `Diplomacy` command. It also closes when the in-game menu or the [reconnect dialog](/systems/reconnect-dialog/) opens, when the game ends, and when a saved game loads.

## What it lists

The Players group lists the local player first, then every other house whose country has [`Multiplay=yes`](/keys/multiplay/), computer players included. Observers are listed in a group of their own. Each name is shown as it is in [chat](/systems/chat/), in the house's color, and a defeated house's name is grey.

**Kills** counts the units and buildings the house has destroyed, whoever owned them.

The line at the top shows the map's name and the time played. In a game against other machines it also shows the connection quality the game has settled on.

The panel reads the game four times a second.

## Alliance

**Yours** is the local player's alliance with that player. Ticking it allies them, and clearing it ends the alliance on both sides, which also ends any [shared control](/systems/shared-control/) between the two. The change takes effect a moment later, when every machine carries out the order. Until then the box shows the requested state and cannot be clicked.

The box can be clicked only when all of these hold:

- Alliances are allowed in the match and the match does not fix them.
- The local player has not been defeated.
- The other house is a human player.
- For a new alliance, at least one enemy would remain.

In a skirmish every other house is a computer player, so no box can be clicked. Resting the pointer on a box that cannot be clicked names the reason.

**Theirs** shows whether that player has allied with the local player. Alliances go one way, so either side can be allied without the other.

## Shared control

The Control columns appear only in a game against other machines with [shared control](/systems/shared-control/) on. **Give** grants that player command of the local player's units, or takes the grant back, under the same rules as [`GrantControl`](/commands/grantcontrol/). It can be clicked only while the two players are allied both ways. **Given** shows whether that player has granted control to the local player.

## Chat

The Chat columns appear only in a game against other machines, and only for human players who are still connected.

- **Send** chooses who receives the local player's lines. A player whose box is cleared does not receive lines to everyone, to the team or to the observers. Private lines sent with a function key are not affected. A player who does receive a line sees its usual tag, so a line sent to some players still reads as a line to everyone.
- **Mute** hides that player's lines on the local player's screen, including on the out-of-sync screen.

Both settings stay on the local machine. Every player starts included and unmuted at the start of each game and after each load.

## Connection

**Latency** is the round trip, in milliseconds, between the local machine and that player's machine. It is measured from the replies to the game's own packets, so it includes the time each machine takes to answer and reads higher than a ping. It shows `-` for the local player, in a skirmish, and until the connection has been measured. It turns yellow above 150 milliseconds and red above 300.

A red `!` after the latency marks a player the local machine's game has halted for, waiting on that player's frames or orders, for at least one second in the last 30 seconds. Resting the pointer on it shows how long. The line at the bottom names the player waited on most. Each machine counts its own waits, so players can see different marks.
