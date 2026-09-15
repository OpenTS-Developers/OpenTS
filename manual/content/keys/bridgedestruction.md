---
key: BridgeDestruction
summary: Starting state of the destroyable-bridges option for a multiplayer or skirmish match.
see_also: [DestroyableBridges, BridgeStrength]
when_omitted:
  kind: value
  value: "yes"
---

The value seeds the match's bridge option as the rules are read, and the setup screen then writes over it. The resulting option replaces the scenario's own [`DestroyableBridges`](/keys/destroyablebridges/) flag once the map has been read. That flag is the first half of the test a blast makes when it reaches a span; [`BridgeStrength`](/keys/bridgestrength/) covers the roll that follows. A campaign game never reads the option and takes the flag from the map instead.

:::caution[A skirmish played with bridges off leaves them off]
The option is copied into the scenario flag only when it is off, and the flag starts on and is not reset between matches. One skirmish with the box cleared therefore leaves bridges indestructible in every later skirmish of the same run of the game, whatever the box says. A network game writes the flag from the agreed option either way, so it both turns the flag off and turns it back on.
:::
