---
key: ConcreteWalls
summary: The walls a computer house builds its base perimeter from, in order of preference.
see_also: ["system:ai-base-building"]
when_omitted:
  kind: value
  value: ""
---

The first entry the country this house acts as may own is the type every wall node of a [planned perimeter](/systems/ai-base-building/#walls-and-gates) is created from, and it is the type the defense planner clears away when a base defense placed along a finished wall takes a cell a wall was already planned for.

The engine builds those nodes from that entry without checking that the list held one that country may own.
