---
key: DemandLoadBuildup
summary: Loads the structure's construction artwork when each scenario starts, from a loose file or any archive.
see_also: ["Buildup", "DemandLoad"]
when_omitted:
  kind: value
  value: "no"
---

With `DemandLoadBuildup=yes`, the file that [`Buildup=`](/keys/buildup/) specifies is loaded when a scenario starts or a saved game is loaded, rather than with the rules, and stays loaded until the next scenario or saved game is loaded. The file can be loose or in any archive. Without the flag, the file is read from a cached archive with the rules, and again when a scenario starts for a [`Theater=yes`](/keys/theater/) or [`NewTheater=yes`](/keys/newtheater/) type.

This load always looks for the name rewritten for the theater by the structure-art convention, with a `.SHP` extension, whatever the type's theater settings say. A `Theater=yes` structure that sets this flag therefore does not look for a file with the theater's extension. The load also times the animation the ordinary way `Buildup=` describes, spreading [`BuildupTime`](/keys/builduptime/) over the steps. The five-second timing of a `Theater=yes` type does not apply. Every player in a network game loads the file at the same point, so all of them use the same construction animation.
