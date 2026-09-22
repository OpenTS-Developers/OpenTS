---
title: In-game chat
summary: "Who an in-game message reaches, which keys start one, how a line is shown, and how the message list is drawn."
category: multiplayer-networking
keys:
  - MessageDelay
  - IncomingMessage
  - TextBackgroundColor
related:
  - type: system
    id: observers
  - type: format
    id: keyboard-ini
  - type: command
    id: ChatToAll
  - type: command
    id: ChatToAllies
  - type: command
    id: fixed:multiplayer-message
---

Players can chat only in a game against other machines. A player types a line into the editor above the message list, then presses Enter to send it or Escape to discard it. The key that opened the editor decides who receives the line, and the editor's prompt names them, for example `To All:` or `To Team:`.

| Line | Opened by | Reaches |
| --- | --- | --- |
| To everyone | [`ChatToAll`](/commands/chattoall/) (Enter unless rebound), or the [function key](/commands/fixed-multiplayer-message/) whose number matches [`Players`](/keys/players/), such as `F8` when `Players` is `8` | Every player and observer |
| To the team | [`ChatToAllies`](/commands/chattoallies/), which is Backspace unless rebound | Every player the sender has allied with |
| To one player | A [function key](/commands/fixed-multiplayer-message/) from `F1` up, one key for each other machine in the match | The player or observer at that machine only |
| To the observers | [`ChatToAllies`](/commands/chattoallies/) pressed by an observer | Every other observer |

A team line follows the sender's alliances only. It reaches a player the sender has allied with even if that player has not allied back.

An observer has no allies, so a team line never reaches an observer. When an observer presses the team key, the line goes to the other observers instead.

## Who may open a line

- A player still in the game may open any line except the one to the observers.
- A defeated player may only open a line to everyone, and the other keys do nothing. In a match with [coach mode](/systems/observers/#coach-mode), a defeated player keeps the team and private lines as well.
- An observer may open a line to everyone or to the other observers. The private keys do nothing.

## Delivery

The sender's game sends a line only to the machines the table above lists for it, and the receiving game checks the same rule again before showing it. A receiving game shows a team line only when the sender has allied with the receiving player, and an observers line only when the receiving player is an observer.

A receiving game drops any line from a machine that holds no [seat](/formats/spawn-ini/#who-is-playing) in the match.

Chat does not travel with the game's orders. A line is shown as soon as it arrives and has no effect on play.

## What is shown

A line reads `Name: text` in the sender's color. A tag after the name shows who the line was for, as in `Name [to team]: text`:

| Line | Tag |
| --- | --- |
| To everyone | none |
| To the team | `[to team]` |
| To the observers | `[to observers]` |
| To one player | `[to Name]`, with the recipient's name |

In a match launched with [`QuickMatch=yes`](/formats/spawn-ini/#what-a-player-is-shown), every name is shown as the player's number instead.

The sender's screen shows each sent line with the same name and tag its recipients see. A team line appears on the sender's screen even when no ally is there to receive it.

The message list sits at the top of the tactical view and shows up to six rows. It also carries the game's other notices, such as a player's defeat. When the list is full, a new row pushes out the oldest one. A line too wide for the view wraps onto extra rows.

Each row added to the list plays [`IncomingMessage`](/keys/incomingmessage/) and stays on screen for [`MessageDelay`](/keys/messagedelay/) minutes.

[`TextBackgroundColor`](/keys/textbackgroundcolor/) draws a color behind every glyph of the list and of the editor.
