---
key: Tunnels
summary: The tile set that supplies the four road tunnel mouths, one per facing.
see_also: [TrackTunnels, DirtTunnels, DirtTrackTunnels, MovementZone]
when_omitted:
  kind: value
  value: "-1"
  note: No tile set is bound to the role.
---

Laying one of these pieces down builds a tunnel. As a cell's terrain is worked out, a cell whose tile reports the [`Tunnel` land type](/reference/enums/land-type/) and that has no tunnel yet is matched against the four tunnel roles in turn: this one, then [`TrackTunnels`](/keys/tracktunnels/), then [`DirtTunnels`](/keys/dirttunnels/), then [`DirtTrackTunnels`](/keys/dirttracktunnels/). The first role whose four pieces contain the tile wins. A tunnel is then created at that cell. The piece's place in the set names the facing it is entered from: the first piece from the east, the second from the south, the third from the west, the fourth from the north.

The land type comes from the artwork rather than from this key, so a set named here whose tiles do not report `Tunnel` builds nothing at all. The artwork gate also settles the unresolved role: a theater with no tile reporting `Tunnel` land matches nothing, so nothing is misread. Where tiles do report `Tunnel` land and the role is unresolved, the theater's first three tiles match as its second, third, and fourth pieces, entered from the south, west, and north. What the key settles is which of the four facings each piece of the set stands for.

A tunnel joins the ground on either side of it. Once the cell has tunnel cells on both sides along one axis, the entrance and the exit are registered as a crossing between two otherwise unconnected pieces of ground. Route-finding uses it the way it uses a [bridge](/keys/bridgeset/) span.
