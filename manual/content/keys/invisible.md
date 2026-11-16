---
key: Invisible
summary: Hides the object from every player except its owner, and keeps it off the radar.
see_also: [InvisibleInGame, "system:cloaking"]
when_omitted:
  kind: value
  value: "no"
---

An `Invisible=yes` object is visible only to the player who owns it. On that player's machine it is drawn normally, cloaked or not. On every other machine, including an ally's, it is not drawn at all, and the tooltip, the cursor and click-selection ignore it.

No machine plots the object on the radar, not even its owner's.

A [hunter-seeker drone](/keys/hunterseeker/#scope-aircrafttype) never chooses it as a target.

A BuildingType with [`InvisibleInGame=yes`](/keys/invisibleingame/) has this key turned on, even when its section sets `Invisible=no`.

Another house's weapons refuse to fire at the object unless a sensor of the attacking house covers its cell. The one exception is the [weapon test for hidden objects](/systems/cloaking/#targeting-a-hidden-object): a shot that would do no damage goes through when the object's owner considers the attacking house allied.
