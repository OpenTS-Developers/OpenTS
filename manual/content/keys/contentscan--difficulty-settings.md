---
key: ContentScan
scope: difficulty-settings
label: Difficulty cargo scan
see_also: ["system:difficulty", "system:target-selection", IQ]
when_omitted:
  kind: value
  value: "no"
  note: The difficulty block is re-read from fixed defaults whenever its section is present, so a later file that contains the section without this key restores no rather than keeping the earlier value.
---

The flag is one of two ways into a test that adds the worth of everything riding inside a transport to the transport's own worth as a target. This one opens it for every house in the slot; [the threshold in `[IQ]`](/keys/contentscan/#scope-global-rules) opens it for a house clever enough. The two are alternatives, so either on its own is sufficient.

Neither is reached. The test sits inside the routine that puts a figure on an object as a target. The only thing that calls that routine is the pass in which a computer house picks a building to aim a superweapon at, and nothing invokes that pass. What a computer house does weigh when it chooses a target is traced under [target selection](/systems/target-selection/), and cargo has no part in it.
