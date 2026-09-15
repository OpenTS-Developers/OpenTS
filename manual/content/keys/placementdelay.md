---
key: PlacementDelay
summary: Minutes a factory waits before trying again when the object it finished cannot leave yet.
see_also: ["system:ai-base-building"]
when_omitted:
  kind: value
  value: ".05"
---

```ini title="rules.ini"
[General]
PlacementDelay=.25 ; a quarter of a minute, or 225 game frames
```

The delay is set only on the temporary-blockage outcome, and the value is converted to whole game frames as it is set. An object whose exit is permanently obstructed is abandoned immediately, and one that leaves cleanly is handed to the house at once. Only production attached to a building runs through this timer, which is how a computer house builds. A player's production is placed from the sidebar instead. For a structure, the blockage that starts the wait is [an allied vehicle, infantry or aircraft standing in the placement zone](/systems/ai-base-building/#choosing-a-spot).
