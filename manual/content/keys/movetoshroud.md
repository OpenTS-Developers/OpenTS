---
key: MoveToShroud
summary: Whether the type accepts an order onto ground that is still under the shroud.
see_also: ["system:map-visibility", Sight, AllowShroudedSubteranneanMoves]
when_omitted:
  kind: value
  value: "yes"
  note: An AircraftType stores no instead, because the aircraft type overwrites the shared default before any rules file is read.
---

An order onto a shrouded cell is refused outright at `no`. At `yes` it is accepted, but only while the cell also lies inside the playable area. Every order but a patrol waypoint is reduced to a plain move: an attack click onto shrouded ground sends the object there rather than making it fire.

The same refusal covers an order onto a shrouded object, and that second test runs only in a campaign.

An aircraft section therefore has to state `MoveToShroud=yes` outright to accept such an order at all.

At `no`, a player's object leaving a factory also ignores a rally point that is still shrouded for the player, as [rally points](/systems/production/#rally-points) describes.
