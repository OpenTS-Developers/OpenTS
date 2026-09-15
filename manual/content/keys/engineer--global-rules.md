---
key: Engineer
scope: global-rules
label: Survivor engineer type
see_also: ["system:capture"]
when_omitted:
  kind: value
  value: none
---

The named InfantryType is offered on a one-in-four roll for each survivor a structure produces, and only for a structure whose own production kind is buildings: the construction yard in the shipped rules. Every other survivor falls through to the ordinary crew or technician type. That crew roll is the only place the value is read. It applies both to a destroyed structure and to a sold one. A sale hands back at most one engineer, however many survivors leave.

A structure that has ever [changed hands](/systems/capture/#what-changes-hands) is excluded from the roll for the rest of the match. A captured construction yard therefore never returns an engineer.
