---
key: Stray
summary: Distance in cells a team member may drift from the team before it is ordered back.
see_also: [CloseEnough, GuardSlower, "system:ai-team-execution"]
when_omitted:
  kind: value
  value: "2"
  note: "512 leptons."
---

The figure is written in cells, with fractions accepted, and multiplied by 256 as it is read. Every test that holds a team's formation together applies this tolerance. A member that has drifted farther than this is ordered back rather than left where it is. The distance is measured from the team's center, from the nearest other member, or from the destination the team is moving to. Until that member is back, the team does not count as gathered or as having arrived. An aircraft is allowed three times the distance, and one team mission measures twice it. [Team execution](/systems/ai-team-execution/#keeping-the-members-together) sets out the individual tests and which of the two distances each one measures.

One test outside teams reads it. An infantryman or vehicle that starts a path search toward a temporarily blocked cell heads for a passable cell nearby instead, but only while it is farther from that cell than its check distance. A team member measures that distance against this figure, and every other object measures it against [`CloseEnough`](/keys/closeenough/).
