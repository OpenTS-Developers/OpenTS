---
key: RevealTriggerRadius
summary: Default radius in cells for the reveal-around-waypoint trigger action.
see_also: ["system:map-visibility"]
when_omitted:
  kind: value
  value: "5"
---

The [Reveal around waypoint...](/mapping/actions/taction-reveal-some/) trigger action uses this value as its radius when the radius is set at `0` or below. In that case, the action uncovers the ground within this many cells of its waypoint for every human player who has not already been given full vision. A value above `10` acts as `10`, the most any sight scan reaches, and `0` uncovers nothing.

The reveal is centered on the waypoint's ground level, raised by the bridge height when the waypoint lies under a bridge. With [`RevealByHeight=yes`](/keys/revealbyheight/), cells behind high ground stay covered unless the action line skips the height test, so a waypoint at the foot of a cliff uncovers less than the radius names.
