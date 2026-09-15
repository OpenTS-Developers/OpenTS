---
key: MessageDelay
summary: Minutes a message stays on screen.
when_omitted:
  kind: value
  value: ".6"
---

The value is the lifetime given to the messages the engine posts itself: the low power warning, alliance and defeat announcements, text sent by a trigger action, and the notices raised during network play. Chat that a player types during a network game has the same lifetime, not a fixed one. Some other engine notices have fixed lifetimes of their own instead, among them the ion storm warnings. The value does not govern how often a message is raised, only how long each one stays visible.
