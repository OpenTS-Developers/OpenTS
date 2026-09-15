---
key: TrackTunnels
summary: The tile set that supplies the four railway tunnel mouths, one per facing.
see_also: [Tunnels, DirtTunnels, DirtTrackTunnels]
when_omitted:
  kind: value
  value: "-1"
  note: No tile set is bound to the role.
---

The four pieces build a tunnel exactly as the road mouths do. [`Tunnels`](/keys/tunnels/) covers what that means and how the facing is read off a piece's place in the set. This role is tested second of the four, so a tile that also falls inside the road tunnel set is claimed by that one instead.
