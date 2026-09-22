---
title: Observers and coach mode
summary: "Observers who watch a skirmish or network game without playing, what a player given the whole map sees, and what coach mode lets a defeated player keep."
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

A client seats an observer by listing the seat under `[IsSpectator]` in [the launch file](/formats/spawn-ini/#who-is-playing). The observer watches from the first frame and never plays. The menu lobbies cannot seat an observer.

Each observer still gets a house. The house uses the country the launch file gives the seat, so the sidebar art and the EVA voice follow that country. It starts defeated and receives no start position, construction vehicle or starting units. The observer's view opens on the start position of a randomly chosen playing house, or on the center of the map when no house has one.

An observer's house holds no alliances. The launch file's alliance entries for the seat are ignored, and so are other seats' entries that name it. No house can ally with it during play, and it cannot ally with another house.

Because the house starts defeated, it never counts among the houses or people still playing when the game decides whether the match is over. It cannot surrender.

An observer does not appear on the [score screen](/systems/multiplayer-score-screen/), in the radar pane's name and kill list, or in the statistics report.

An observer's house is still an observer after a [saved game](/formats/save-games/) is loaded. A launch file that resumes a network game's save must mark the same seats as watching, or the game refuses to load the save.

## The whole map

An observer is given the whole map, and so is a defeated player in a match without [coach mode](#coach-mode). Both see the match the same way:

| Subject | What is shown |
| --- | --- |
| Shroud and fog | Every cell is revealed and unfogged, and the fogged stand-ins for structures are discarded. Neither [regrowth pass](/systems/map-visibility/#losing-ground-again) runs again. |
| Radar | The pane stays up whatever the [power](/systems/power/#radar) and [ion storm](/systems/ion-storms/#radar) tests say, and it shows the whole map. |
| Hidden objects | A cloaked object is drawn shadowy and can be selected. Cloaked and underground objects are plotted on the radar. None of them raises a [detection](/systems/cloaking/#on-the-radar) event. |
| Decorations | Every house's objects show cargo pips, [rank insignia](/systems/veterancy/#rank-display) and the healer's cross. A selected structure that generates power shows its house's power output and drain, and a selected factory shows the cameo of what it is building. A tooltip names an object by its type, not as an enemy. |
| Disguises | They hold, as they do for every player except the owner. |
| Chat | Messages can go to everyone. An observer can also message the other observers. A team or private message cannot be opened ([in-game chat](/systems/chat/)). |

An observer owns nothing, and a defeated player's forces are destroyed at defeat. Clicking an object therefore only selects it for viewing and gives no order.

## What an observer hears

EVA speaks most lines only to the house they concern: money, silos, power, a lost unit or structure, a harvester or base under attack, a superweapon, the sidebar, and the house's own defeat or victory. An observer owns nothing and is never given a verdict, so it hears none of these lines.

Lines spoken to everyone still reach an observer. These include another player's defeat, a missile launch, an approaching ion storm, and anything a scenario scripts. The text messages that announce a defeat or a departure appear as they do for every player.

Radar events follow the same rule: the only ones an observer sees are those a scenario scripts.

## The sidebar and the credit readout

The [sidebar](/systems/sidebar/) stays up, with nothing on either strip. Its credit readout shows how long the match has run instead of money. The time counts real seconds while the game is not paused, and reads `Time:MM:SS`, or `Time:HH:MM:SS` once the match has run an hour. The readout plays no [`CreditTicks`](/keys/creditticks/) sound.

## Coach mode

`CoachMode=yes` in the launch file's `[Settings]` section decides what a defeated player keeps. Only the launch file can turn it on; the menu lobbies have no such option.

Without coach mode, a defeated player is given [the whole map](#the-whole-map), as an observer is, and can only send messages to everyone.

With coach mode, defeat leaves the player's view and chat as they were:

- Nothing is revealed at defeat. The fog stays drawn, the regrowth passes keep running, and the player goes on seeing what their allies reveal under [`AllyReveal=yes`](/keys/allyreveal/).
- The radar goes down with the player's radar structure, unless the map gives everyone radar. An [ion storm](/systems/ion-storms/#radar) still takes it down.
- Hidden enemy objects and enemy decorations stay hidden.
- Team and private messages stay open, so the player can keep talking to their allies.

With or without coach mode, a defeated player keeps their row on the score screen, in the radar pane's name list and in the statistics report.

## When the match ends

A match with at least one person playing ends when only one side is left, or when no person is left playing.

A match in which every person watches ends only when an enemy of a surviving house is defeated and only one side is left. Losing an allied house to a scripted event does not end the match, and a match whose houses are all on one side from the start never ends. An observer leaves such a match through the menu.

## Seating

Any number of people may watch, as long as somebody plays; a computer player counts. The game refuses a launch file in which every person watches and no computer plays.

One person may play alone beside observers. With no opponent, that match ends when the player is defeated.

When an observer [leaves](/systems/leaving-a-match/), the departure is announced as any other player's is. The observer's house has nothing to hand to the computer.

:::caution[Reshroud map hides the map from observers]
[Reshroud map](/mapping/actions/taction-reshroud/) and its [team mission](/mapping/missions/tmission-reshroud/) shroud the whole map for the player at each machine, including an observer and any player given the whole map. After either runs, an observer sees only shroud until a trigger or team mission reveals the map again. Leave both out of a map meant to be watched.
:::
