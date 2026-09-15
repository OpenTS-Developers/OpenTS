---
key: Color
scope: tiberium
label: Tiberium remap
see_also: ["system:tiberium", "Debris"]
when_omitted:
  kind: value
  value: "0"
  note: The first color scheme in the loaded list.
---

The value is a color scheme name, matched against the loaded schemes without regard to letter case. A scheme with only one intensity level is skipped during that match. A name matching no remaining scheme leaves the type on the scheme it already had.

The chosen scheme recolors the type's overlay wherever it is drawn, the cell animation the overlay type declares as the overlay is created, and the [`Debris`](/keys/debris/) animation a chain reaction leaves.
