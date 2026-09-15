---
key: NumPlayers
scope: random-map-generation-2
label: Start positions a lobby counts
see_also: [Width, Height]
when_omitted:
  kind: value
  value: "0"
---

This is a second, separate read of the assignment, made from the multiplayer lobby rather than by the generator. Before a game is allowed to start, the host's chosen scenario file is opened and its `[Waypoints]` section counted for entries `0` through `7`. Only when that count comes to nothing is this assignment read instead, with a fallback of zero. The figure that comes out is compared against the players in the lobby plus the computer players asked for. A game whose total exceeds it is refused with a message that the scenario is too small.

The fallback is what makes a saved map seed usable in a lobby. A seed file holds no waypoints, because the start points do not exist until the map is generated. Without the fallback, every seed file would count as having nowhere to put anyone.
