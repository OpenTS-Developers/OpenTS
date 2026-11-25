---
key: Engineer
scope: global-rules
label: Survivor engineer type
see_also: ["system:capture"]
when_omitted:
  kind: value
  value: none
---

A structure whose type sets [`Factory=BuildingType`](/keys/factory/) can produce the named InfantryType as a [survivor](/systems/capture/#survivors). In the shipped rules that structure is the construction yard. Each survivor such a structure produces has a one-in-four chance of being this type. Every other survivor is the ordinary crew or technician type, and no other structure produces this type.

The roll applies whether the structure is destroyed or sold, with two limits on a sale:

- A sale hands back at most one engineer, however many survivors leave. Destruction has no such limit.
- A structure whose type names an [`UndeploysInto`](/keys/undeploysinto/) vehicle produces no survivors when sold. The shipped construction yard names one, so selling it returns no engineer.

A structure that has ever [changed hands](/systems/capture/#what-changes-hands) never produces this type for the rest of the match. A captured construction yard therefore never returns an engineer.

A side can name its own engineer type for structures of houses acting as its countries. [`Engineer`](/keys/engineer/#scope-side) covers that scope.

:::danger[Name a type here if a buildings factory can be sold]
With no type named here or by the owner's side, a destroyed structure produces no survivor on that one-in-four roll. Selling the structure crashes the game whenever the roll comes up. This applies to any [`Crewed=yes`](/keys/crewed/), `Factory=BuildingType` structure that names no `UndeploysInto` vehicle.
:::
