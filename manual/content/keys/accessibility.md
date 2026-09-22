---
key: Accessibility
summary: The chance that neighboring pieces of high and low ground are joined by more than one ramp, as a figure from 0 to 100.
see_also: [RegionSize, Ruggedness]
when_omitted:
  kind: value
  value: "0"
  note: Every pair of neighboring regions at different heights gets a single ramp and never more.
---

Once the map is carved into regions, each pair of neighboring regions standing at different heights has ramps cut between them. One ramp is always asked for; the figure is the percentage chance, rolled once per pair, that one or two more are asked for as well. At `0` every such pair gets a single ramp and at `100` almost every pair gets two or three. [Map seed files](/formats/map-seed/) covers the section it is written in.

```ini title="map seed file"
[RandomMap]
Accessibility=75
```

The ramps are cut at cells drawn at random from the higher region's border, and up to a hundred cells are tried per pair. A pair still short of its ramps after fifty tries starts accepting fallback placements, which try the four straight ramp shapes at fixed endpoints rather than hunting for a shape that fits the ground. A pair that ends with no ramp at all marks the region as unconnected, and nothing reads that mark, so the region ships as it stands.

Water is joined differently and takes no notice of the figure. A region of water spans itself with bridges to the dry land on either side, and only where both sides stand at the water's own height.

When a map is [generated from a file](/systems/map-generation/#the-dialog-path-and-the-scenario-path), a value below `0` becomes `0` and one above `100` becomes `100`.
