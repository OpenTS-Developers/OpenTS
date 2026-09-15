---
key: OnTransOnly
summary: Restricts the team's tag to the members that can carry passengers.
see_also: [Tag, Passengers]
when_omitted:
  kind: value
  value: "no"
---

A team built from a TeamType that names a [`Tag`](/keys/tag/) attaches that tag to each member as the member joins. With this set, only a member whose type sets [`Passengers`](/keys/passengers/) above zero is given it; every other member joins holding whatever tag it already had.

A TeamType with no `Tag` is unaffected, since there is nothing to withhold. The narrowing is applied at the moment of joining and nowhere else: a member recruited later is judged by the same test. A member that already holds the team's tag keeps it however this setting changes afterward.
