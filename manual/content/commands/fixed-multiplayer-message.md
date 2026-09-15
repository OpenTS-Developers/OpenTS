---
command_id: fixed:multiplayer-message
---

A function key opens a message editor only while all of these hold:

- the session is neither a campaign nor a skirmish game;
- the key falls in the session player range, which runs from `F1` to `F1` plus the session's player limit less one;
- no message is already being edited.

Past that gate only a network or Internet session opens anything. The last key in the range starts a message to everyone. An earlier key starts a private message to one connection: `F1` names the first connection and each key after it names the next. The message needs a connection at that position, and the player must still be allowed to send a private message. [In-game chat](/systems/chat/) states who may. A message to the team, and a second key for everyone, are commands of their own: [`ChatToAllies`](/commands/chattoallies/) and [`ChatToAll`](/commands/chattoall/).
