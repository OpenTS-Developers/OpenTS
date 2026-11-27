---
key: Radar
summary: Whether the structure can supply the player's radar map.
see_also: ["system:power"]
when_omitted:
  kind: value
  value: "no"
---

A `Radar=yes` structure gives the local player the radar map. The map is up while all of these hold:

- no ion storm is running;
- the player's house has at least as much power output as drain;
- the house owns a `Radar=yes` structure that is switched on, on the map and not being sold.

[`FreeRadar=yes`](/keys/freeradar/) replaces the third test; the storm and power tests still apply. [`FreeRadarOnLowPower=yes`](/keys/freeradaronlowpower/) also drops the power test, and it does nothing without `FreeRadar=yes`. In a campaign, a radar structure the player has not discovered does not count. A player who has been given the whole map keeps the radar whatever the three tests say, as [observers and coach mode](/systems/observers/) describes.

A stunned radar structure supplies nothing; the house checks its other radar structures instead. [Radar](/systems/power/#radar) gives the full test.

`Radar=yes` also makes the structure a target for spies. When a spy enters an enemy's `Radar=yes` structure, everything that enemy's objects see is also uncovered for the spy's house. [Who looks, and when](/systems/map-visibility/#who-looks-and-when) covers how long that lasts.
