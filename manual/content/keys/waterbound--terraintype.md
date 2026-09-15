---
key: WaterBound
scope: terraintype
label: Water-based terrain object
see_also: [Foundation]
when_omitted:
  kind: value
  value: "no"
---

WaterBound picks the movement kind the engine uses when it tests whether a terrain object may stand on a cell. A water-based object is tested for the floating movement kind; every other object is tested for the kind a tracked vehicle uses. Any of the following fails a cell of the object's footprint:

- It lies outside the playable area.
- It holds any overlay.
- Its land type costs nothing for the kind being tested.

The test never reads [`Buildable=`](/keys/buildable/), the flag that admits building foundations.

The engine does not run the test when a scenario places its terrain, so map terrain stands where the map puts it whatever the ground. The test runs when the ground under a placed object changes, which in play happens when a low bridge is destroyed. An object that fails is destroyed with the bridge.
