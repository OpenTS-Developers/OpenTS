---
key: AllowShroudedSubteranneanMoves
summary: Whether a subterranean unit accepts a click on an object standing under the shroud, and follows a rally point into the shroud.
see_also: ["system:map-visibility", MoveToShroud]
when_omitted:
  kind: value
  value: "no"
---

A subterranean unit is one whose type takes the [`Subterannean` movement zone](/reference/enums/movement-zone/). At the default, a move click that would send one onto a shrouded object is swallowed: the click is consumed, no mission is assigned, and the unit stands still. Setting the key to `yes` lets the same click through to the ordinary move handling.

An aircraft is swallowed at the same test and has no equivalent setting. The rule covers a click on an object; a click on shrouded ground is governed by [`MoveToShroud`](/keys/movetoshroud/) instead.

At `no`, a player's subterranean unit leaving a factory also ignores a rally point that is still shrouded for the player, as [rally points](/systems/production/#rally-points) describes.
