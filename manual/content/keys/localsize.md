---
key: LocalSize
summary: The part of the playfield that is visible and playable, in cells.
see_also: ["system:map-visibility", Size]
when_omitted:
  kind: value
  value: "the value of [Map] Size after clipping"
---

The rectangle is clipped into the playfield, and its origin is pushed to at least two cells in on both axes. Its far edges are then pulled back two cells horizontally and six vertically. Everything outside the result is the border a scenario keeps around its playable ground.

The clipped rectangle is read all over the engine: it bounds scrolling, off-map deletion, reinforcement entry, crate and drop-pod placement and more. Three of its uses shape what a player sees. Every object is locked or unlocked against it as it changes, and only a locked object reveals terrain, so an object that has never been inside the playable area sees nothing. The radar picture is fitted to the cells inside it rather than to the whole playfield. The shroud-seeking [team mission](/mapping/missions/tmission-goto-shroud/) and the [Reveal zone of waypoint...](/mapping/actions/taction-reveal-zone/) action both consider only cells inside it.

A player-controlled vehicle, infantryman or aircraft that becomes locked as the rectangle is set takes a look immediately, so widening the playable area reveals ground at once rather than on the object's next move. A structure is not given that look.
