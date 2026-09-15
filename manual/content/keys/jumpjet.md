---
key: JumpJet
summary: Marks the InfantryType as a jumpjet, so it routes, is targeted and dies as an air unit.
see_also: [Locomotor, MovementZone, InfantryExplode, "system:ion-storms", "system:target-selection"]
when_omitted:
  kind: value
  value: "no"
---

The flag does not supply the locomotor. [`Locomotor=`](/keys/locomotor/) picks that, and the two only combine into flight when both are present: a soldier counts as flying at a given moment when its type sets this flag *and* the jumpjet locomotor is the one currently in control.

Movement is planned as though the type's [`MovementZone`](/keys/movementzone/) were the infantry zone, whatever it actually says. An order may name any cell at all: an unreachable one is answered with the nearest reachable cell rather than refused, and the cursor is left blank instead of falling back to the cannot-move form. A move order over a cell beside a tunnel entrance offers no action.

Before each leg, a jumpjet chooses between flying and walking. It flies when **any of** the following holds:

- it is already off the ground;
- the destination lies in a different infantry-zone region;
- the trip spans 12 cells or more;
- either end is outside the playable area;
- a trial ground route between the two cells comes back longer than 15 cells.

It walks a trip of one cell or none, and, for a house under player control, a trip that starts beside a tunnel entrance to the north or west. An [ion storm](/systems/ion-storms/#jumpjet-infantry) refuses flight outright. Choosing to walk piggybacks a walking locomotor on top of the jumpjet one; choosing to fly ends that piggyback.

Being airborne infantry changes two more things. A jumpjet is offered to the flying-layer passes of a [threat scan](/systems/target-selection/#picking-the-winner) alongside aircraft. An explosion whose center sits above ground level gathers it alongside the aircraft within one cell of that center, which is how a shot fired into the air reaches it at all. And a killed jumpjet plays `[AudioVisual] InfantryExplode` at its position and is removed at once, with no death animation and no corpse.
