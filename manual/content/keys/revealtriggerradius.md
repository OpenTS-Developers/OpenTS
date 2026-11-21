---
key: RevealTriggerRadius
summary: Radius in cells that the reveal-around-waypoint trigger action uncovers.
see_also: ["system:map-visibility"]
when_omitted:
  kind: value
  value: "5"
---

Only the [Reveal around waypoint...](/mapping/actions/taction-reveal-some/) trigger action uses this value. The action uncovers the ground within this many cells of its waypoint for every human player who has not already been given full vision. A value above `10` uncovers that far, and `0` uncovers nothing.

The reveal is centered on the waypoint's ground level, raised by the bridge height when the waypoint lies under a bridge. With [`RevealByHeight=yes`](/keys/revealbyheight/), cells behind high ground stay covered, so a waypoint at the foot of a cliff uncovers less than the radius names.
