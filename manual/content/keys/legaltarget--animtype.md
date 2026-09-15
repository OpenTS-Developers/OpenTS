---
key: LegalTarget
scope: animtype
label: Animation targetability
no_effect: true
see_also: ["system:target-selection"]
when_omitted:
  kind: value
  value: "no"
---

Every path that reads the flag is closed to an animation. The automatic scans weigh only objects a house owns and can be shot at, and an animation is neither. The cursor test takes the nearest object on the screen's click list, or the cell's occupier when nothing there is close enough. An animation never enters either list: the occupier list holds footprint types only, an AnimType not being one. An animation attached to a unit or a structure moves and draws with it and still cannot be pointed at; the object underneath answers for both.
