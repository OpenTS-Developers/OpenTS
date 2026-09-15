---
key: Group
scope: taskforce
label: TaskForce recruitment group
see_also: ["system:ai-team-production"]
when_omitted:
  kind: value
  value: "-1"
---

Only the TeamTypes that use this TaskForce read its value, and only when they leave their own [`Group`](/keys/group/#scope-teamtype) at `-1`. Setting it gives every TeamType built on it one [recruitment group](/systems/ai-team-production/#recruitment). Any of those TeamTypes overrides it by naming a group of its own.
