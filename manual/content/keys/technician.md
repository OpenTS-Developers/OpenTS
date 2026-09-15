---
key: Technician
summary: The InfantryType a civilian object always produces as its survivor, and an armed object sometimes, in place of the ordinary crew.
see_also: ["system:capture", Crew, Crewed]
when_omitted:
  kind: value
  value: none
---

A [`Crewed=yes`](/keys/crewed/) object whose owning house names no [`Side=`](/keys/side/#scope-housetype) yields this type for every survivor it produces. An object whose house does name one yields it on a fixed 15% roll, and only while the object has a primary weapon. Every country in the shipped rules names a side, so a mod reaches the first case only on a country it leaves without one. Both tests are on the object producing the survivor rather than on the survivor, and both are taken afresh for each one. Everything else falls to [`Crew`](/keys/crew/), and an uncaptured structure whose [`Factory=`](/keys/factory/) builds structures takes a one-in-four `[General]` [`Engineer`](/keys/engineer/#scope-global-rules) roll ahead of both.

A sold structure marks every survivor whose own type is [`Nominal=yes`](/keys/nominal/), whether it came from this type, `Crew` or `Engineer`; a destroyed one marks them only while it has build-up artwork. A marked survivor is displayed under the engine's "Technician" string, is barred from being picked up as a civilian evacuee, and is left out of its house's infantry tally. A vehicle's escaping crew is never marked.

:::danger[Leaving this unset crashes the game on a sale]
The value starts as no type at all, and the loop that produces a sale's survivors reads each pick before testing it for nothing, so the game crashes on the empty pick. The stock rules name `CTECH`. An armed `Crewed=yes` structure reaches this type on 15% of its survivors, and one owned by a house that names no side reaches it on every one. A structure's [survivor count](/keys/survivordivisor/) runs from one to five, and a destroyed structure tests each pick before using it, so only a sale crashes.
:::
