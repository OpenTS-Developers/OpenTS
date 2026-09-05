---
title: Observers and coach mode
summary: "Seats a house that watches a multiplayer match without playing it, gives such a house the whole map, and decides what a defeated player keeps."
category: multiplayer-networking
keys:
  - MultiplayPassive
  - ShroudGrow
  - ShroudRate
  - FogRate
  - CreditTicks
related:
  - type: format
    id: spawn-ini
  - type: format
    id: save-games
  - type: system
    id: map-visibility
  - type: system
    id: cloaking
  - type: system
    id: multiplayer-score-screen
  - type: system
    id: sidebar
  - type: system
    id: veterancy
  - type: system
    id: power
  - type: system
    id: ion-storms
---

An observer is a seat that watches a skirmish or network game from its first frame. A client marks the seat in [the launch file](/formats/spawn-ini/#who-is-playing); the menu lobbies cannot seat one. The observer's house plays the country the client wrote for it, which the CnCNet client picks at random, so the sidebar art and the EVA voice follow that country. The house starts defeated, is given no start position, construction vehicle or units, and holds no alliances: its own alliance entries and any naming it are ignored, and nobody can ally with it during play. Its view opens on the start position of a random playing house, or on the middle of the map when no house plays.

Starting defeated is what keeps the house out of the contest. It is never tested for defeat, never counted among the houses or people still alive, never asked whether everyone left is allied, and cannot surrender or change an alliance. It is also left out of three places a defeated player still appears in: the [score screen](/systems/multiplayer-score-screen/), the radar pane's name and kill list, and the statistics report. The house survives a [saved game](/formats/save-games/) as an observer, and a launch file that resumes the save must still mark the seat as watching.

## The whole map

An observer is given the whole map, and so is a defeated player outside [coach mode](#coach-mode). The engine keeps one flag for that state, so both are shown the same match:

| Subject | What is shown |
| --- | --- |
| Shroud and fog | Every cell is revealed and unfogged, the stand-ins fogged structures leave behind are discarded, and neither [regrowth pass](/systems/map-visibility/#losing-ground-again) runs again |
| Radar | The pane stays up whatever the [power](/systems/power/#radar) and [ion storm](/systems/ion-storms/#radar) tests say, and shows what the map shows |
| Hidden objects | A cloaked, submerged or subterranean object is drawn shadowy and plotted on the radar as its owner sees it, may be clicked and read, and never raises a [detection](/systems/cloaking/#on-the-radar) event or line |
| Decorations | Condition pips, cargo pips, [rank insignia](/systems/veterancy/#rank-display), the healer's cross, a structure's power figures and the true name in a tooltip are drawn for every house, and a selected factory shows the cameo of what it is building |
| Disguises | Kept, as from every player but the owner |
| Chat | Messages to everyone only, and for an observer to the other observers as well; a private or team message to a player is refused ([in-game chat](/systems/chat/)) |

A player given the whole map owns nothing, so every click is a look rather than an order.

## What an observer hears

EVA speaks most lines to the house they belong to: money, silos, power, a lost unit or structure, a harvester or base under attack, a superweapon, the sidebar, and the house's own defeat or victory. An observer owns nothing and is never given a verdict, so it hears none of them. A line spoken to everyone still reaches it: another player's defeat, a missile launch, an approaching ion storm, and anything a scenario scripts. Radar events follow the same rule, so only scripted ones appear. The text messages announcing a defeat or a departure appear as they do for every player.

## The sidebar and the credit readout

The [sidebar](/systems/sidebar/) stays up with nothing on either strip. Its credit readout shows how long the match has run instead of money, in real seconds while the game is not paused, as `Time:MM:SS` and from the first hour `Time:HH:MM:SS`. It plays no [`CreditTicks`](/keys/creditticks/) sound.

## Coach mode

`CoachMode=yes` in the launch file's `[Settings]` decides what a defeated player keeps. The CnCNet client writes it whenever two or more teams each hold two or more people; it is not a lobby option.

Without coach mode a defeated player is given the whole map, as an observer is, and can only message everyone. With it the player keeps what their allies see: the map stays as it was at defeat, fog stays drawn, the regrowth passes keep running, the radar follows its ordinary tests, hidden objects and decorations stay hidden, and private and team messages stay open, so the player can keep talking to their allies. Either way the player stays in the score screen, the name list and the statistics report.

## When the match ends

A match with at least one person playing ends as it always has: when one side is left, or when nobody playing is left. A match in which every person watches ends only when the last enemy of the surviving houses falls. An allied house lost to a scripted event does not end it, and a match that only ever had one side never ends; an observer leaves it through the menu.

## Seating

Any number of people may watch as long as somebody plays, a computer included; a launch file in which everyone watches and no computer plays is refused. One person playing beside observers is accepted, and nothing can end that match. An observer who leaves is announced like any other player, and nothing is handed to the computer.

:::caution[Reshroud map still darkens a whole-map viewer]
The [Reshroud map](/mapping/actions/taction-reshroud/) trigger action re-shrouds the map for the player at this machine whoever they are, and nothing reveals it again for an observer. A map that uses the action cannot be watched.
:::
