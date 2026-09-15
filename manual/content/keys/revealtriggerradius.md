---
key: RevealTriggerRadius
summary: Radius in cells that the reveal-around-waypoint trigger action uncovers.
see_also: ["system:map-visibility"]
when_omitted:
  kind: value
  value: "5"
---

[Reveal around waypoint...](/mapping/actions/taction-reveal-some/) is the only reader. The reveal is centered on the waypoint's own ground level, raised by the bridge height when the waypoint lies under a bridge. The [height test](/keys/revealbyheight/) that blocks a reveal behind high ground applies, so a waypoint at the foot of a cliff uncovers less than the radius names.

The value is a plain count of cells and shares the sight scan's cap of ten cells, so a larger value uncovers ten. The action does nothing once the player has already been given full vision.
