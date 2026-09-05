---
title: Multiplayer score screen
summary: "Ranks the houses that played a skirmish or network match and shows what each lost, killed, built and scored."
category: multiplayer-networking
keys:
  - MultiplayPassive
  - SkipScore
related:
  - type: system
    id: observers
  - type: format
    id: spawn-ini
---

A skirmish or network match ends at this screen rather than at the campaign one. It appears once the match is decided, to the winner and the loser alike, and the match is over when the player dismisses it: a game the menu started returns to the menu, and one [a client launched](/formats/spawn-ini/) exits. A recorded game being played back never reaches it.

## Who is listed

One row per house that played. A house whose country is [`MultiplayPassive`](/keys/multiplaypassive/) is left out, and so is an [observer](/systems/observers/). A defeated player keeps its row, so a match of four ends with four rows however it went. Computer players are listed like anyone else, under the name the match gave them.

Rows are ranked by rounds won and then by the round's score, so the winner heads the list. Each row is tinted with the house's own colour, and two houses sharing a colour are drawn alike.

## What each column holds

| Column | What it counts |
| --- | --- |
| Losses | The house's own units and structures that were destroyed. |
| Kills | The units and structures the house destroyed, whoever owned them. |
| Economy | What the house still had standing, as a percentage of everything it ever had. A house that was wiped out reads zero. |
| Score | The points the house earned, and for a house left undefeated a bonus of half its opponents' average points, or 100, whichever is larger. |

The three bar columns grow together and the score counts up at the end. A score that would rank below the row beneath it is raised above it before the columns are drawn, so the score column always descends.

The four tallies come from the counts each house keeps for the whole match, and a [saved game](/formats/save-games/) carries all four, so a resumed match is scored on the whole of it rather than on the part played since the load.

## Passing over the screen

A client that keeps its own record of a match writes [`SkipScoreScreen=yes`](/formats/spawn-ini/#what-a-player-is-shown), and the match then ends without the screen: the round is still counted, and an ending movie [the file asked for](/formats/spawn-ini/) still plays. The map's [`SkipScore`](/keys/skipscore/) is a campaign setting and does not reach this screen.
