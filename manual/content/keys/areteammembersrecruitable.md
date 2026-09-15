---
key: AreTeamMembersRecruitable
summary: The autocreate-recruitable state written onto each object as it joins a team of this type.
see_also: ["system:ai-team-production", Autocreate, Priority]
when_omitted:
  kind: value
  value: "yes"
---

The value is stamped onto the object at the moment it joins, and it outlives the team. An object released by a team declared `AreTeamMembersRecruitable=no` keeps the cleared state and is refused by every [autocreated team](/systems/ai-team-production/#recruitment) it later comes near. Its ordinary team-recruitable state is untouched, so a team whose TeamType is not marked as autocreated can still take it.
