---
key: Multiplay
summary: Offers the country as a choice when a skirmish or multiplayer game is being set up.
see_also: [MultiplayPassive, Side, Color]
when_omitted:
  kind: value
  value: "no"
---

```ini title="rules.ini"
[GDI]
Multiplay=yes
```

The flag is what keeps a story-only country out of the country box. The skirmish and LAN setup screens each walk the whole country list and offer only the countries carrying it, listing each by its [`Name=`](/keys/name/#scope-aitriggertype); [`Side`](/keys/side/#scope-multiplayer-settings) covers what the choice is stored as.

It is also what puts a house on the in-game player standings panel: the radar's name-and-kills list skips any house whose country lacks the flag, so a house of a country without it plays without ever appearing there.

No other gameplay path consults it. Whether a house takes part in the contest at all is [`MultiplayPassive`](/keys/multiplaypassive/), which is a separate question.
