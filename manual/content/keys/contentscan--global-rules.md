---
key: ContentScan
scope: global-rules
label: IQ threshold
see_also: ["system:difficulty", "system:target-selection", IQ]
when_omitted:
  kind: value
  value: "4"
---

A house whose [`IQ`](/keys/iq/) reached this level would count the worth of a transport's passengers toward the transport's worth as a target. A house whose [difficulty section](/keys/contentscan/#scope-difficulty-settings) sets the companion flag would do the same. The two are alternatives rather than conditions that both have to hold.

Neither is reached. The comparison sits inside the routine that puts a figure on an object as a target. The only thing that calls that routine is the pass in which a computer house picks a building to aim a superweapon at. No code invokes that pass, so raising a house's intelligence cannot switch the comparison on, and moving the threshold changes nothing at any level.
