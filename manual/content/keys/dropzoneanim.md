---
key: DropZoneAnim
summary: The flare that marks a reinforcement drop zone and lights the ground around itself.
see_also: [DropZoneRadius, "system:map-visibility"]
when_omitted:
  kind: value
  value: none
---

```ini title="rules.ini"
[AudioVisual]
DropZoneAnim=MYBEACON ; an AnimType registered in [Animations]
```

The [Drop Zone Flare (waypoint)](/mapping/actions/taction-dz/) trigger action creates one at the ground height of its waypoint, or on the deck when the cell has a bridge. It marks the flare inert, which withholds the animation's own damage, its start sound and any Tiberium chain reaction it would set off.

Two further behaviors are keyed to the animation type itself rather than to the action:

- Every animation of this type reveals the ground around itself to the local player as it is created, out to [`DropZoneRadius`](/keys/dropzoneradius/). The reveal is one-shot and ignores whose trigger dropped the flare.
- Every animation of this type standing over a structure marks itself for deletion on its next pass. That is what makes a flare vanish when a player builds on the spot it marks.

:::caution[Reusing the animation elsewhere spreads both behaviors]
The two extra behaviors compare the animation's type against this setting. Naming a type that is already used for something else gives every copy of it the map reveal and the build-over deletion. Give the flare a type of its own.
:::

:::danger[An unset animation crashes the game when the action fires]
The trigger action creates the flare without first checking that a type was named, so a scenario that uses the action against a rules layer that leaves this key unset crashes the game at that trigger.
:::
