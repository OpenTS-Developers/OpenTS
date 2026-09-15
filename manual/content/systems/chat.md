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

In a game against other machines, a player types a line into the editor above the message list and sends it with Enter; Escape drops it. The key that opened the editor decides who the line is for:

| Line | Opened by | Reaches |
| --- | --- | --- |
| To everyone | The last function key of the player range, `F8` with the default player limit, or [`ChatToAll`](/commands/chattoall/), Enter unless rebound | Every player and observer |
| To the team | [`ChatToAllies`](/commands/chattoallies/), Backspace unless rebound | Every house the sender is allied with |
| To one player | `F1` upward, one key per connection | That connection only |
| To the observers | [`ChatToAllies`](/commands/chattoallies/) while the sender is an observer | Every other observer |

Alliances are one-way, so a team line reaches the houses the sender has allied with whether or not they allied back. An observer holds no alliances, so a team line never reaches one and an observer's team key addresses the other observers instead.

## Who may open a line

- A playing house may open every line but the one to the observers.
- A defeated player given the whole map may only open a line to everyone; the other keys do nothing. With [coach mode](/systems/observers/#coach-mode) a defeated player keeps the team and private lines.
- An observer may open a line to everyone or to the other observers; the private keys do nothing.

## Delivery

A line is sent only to the connections its kind allows, and the receiving game applies the same rule again before showing it: a team line is shown only when the sender is allied with the house at that machine, and an observers line only to an observer. A line whose sender is not a seat of the match is dropped. Chat is not part of the frame-locked traffic, so a line arrives whenever it arrives and never touches the simulation.

## What is shown

A line reads `Name: text` in the sender's color. A team line carries the tag `[to team]` after the name, an observers line `[to observers]`, and a private line `[to Name]` with the recipient's name; a line to everyone carries none. The sender's own screen shows the line the same way as it is sent, so a team line with nobody to reach still appears there. Each line added plays [`IncomingMessage`](/keys/incomingmessage/) once and stays for [`MessageDelay`](/keys/messagedelay/) minutes.

The list holds six lines over the tactical view. [`TextBackgroundColor`](/keys/textbackgroundcolor/) draws a color behind every glyph of the list and of the editor.
