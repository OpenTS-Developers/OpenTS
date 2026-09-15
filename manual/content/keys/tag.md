---
key: Tag
summary: The TagType attached to every member of a team of this type.
see_also: [OnTransOnly, Script, TaskForce]
when_omitted:
  kind: value
  value: none
---

One tag is built from the named TagType as the team itself is created, and that single tag is attached to each member as the member joins. Every member of the team therefore holds the same instance rather than one of its own. Each trigger behind the tag exists once for the team, so a trigger that can fire only once fires for the team as a whole rather than once per member. [`OnTransOnly=yes`](/keys/ontransonly/) narrows the attachment to the members that can carry passengers.

A member removed from the team has the tag detached again, but only while it belongs to a computer house. A member of a human house keeps it, so a reinforcement team's tag stays on the objects after the player takes them over.

The value is matched against the registered TagType IDs. `<none>` and `none` clear the reference, and a name that no `[Tags]` entry registers is not rejected: a fresh TagType is created under that name with no trigger behind it. A typo therefore gives the team a tag that can never spring.
