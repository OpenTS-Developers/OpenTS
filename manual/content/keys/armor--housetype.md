---
key: Armor
scope: housetype
label: Armor bias
see_also: ["system:difficulty", Verses]
when_omitted:
  kind: value
  value: "1.0"
---

Each country sets its own divisor, and a house takes the one belonging to the country it was created under. Outside a campaign game that is the country chosen for it in the session setup. Damage arriving at that house's objects is divided by this value, so a figure above 1 in one country's section and not another's makes everything that country fields harder to kill, whatever it is built from.

The divisor is multiplied with [the difficulty section's](/keys/armor/#scope-difficulty-settings) once, [when the house is given its difficulty slot](/systems/difficulty/#how-the-figures-are-combined). A campaign game drops it: nothing a country section says about armor reaches a campaign mission. Because the two are multiplied before anything divides by the result, a `0` in either section does the same thing. The division behind every hit then yields a figure its whole-number conversion cannot hold, and the floor that follows sets each hit to a single point of damage. That division runs before [the warhead's Verses table](/keys/verses/) is read, so the table's percentage and the distance falloff still act on the one point that reaches them.
