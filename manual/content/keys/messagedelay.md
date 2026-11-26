---
key: MessageDelay
summary: How long most on-screen messages stay visible.
when_omitted:
  kind: value
  value: ".6"
---

Each whole unit keeps a message on screen for 14.4 seconds of real time, so the default `.6` shows a message for about 8.6 seconds and `1` for 14.4 seconds. Game speed does not change it. A message leaves sooner when the list of messages is full, because each new message then removes the oldest one.

It sets how long these messages stay visible:

- the low power warning;
- alliance and defeat announcements;
- text shown by a [Text Trigger](/mapping/actions/taction-text-trigger/) action that does not set a positive display time;
- notices about saving the game, such as the autosave announcement;
- the difficulty announcement at the start of a campaign mission;
- notices during network play, such as a player leaving or changing the game speed;
- chat that players type.

Some other notices have fixed lifetimes instead, among them the ion storm warnings.
