---
key: Locomotor
summary: The class identifier of the locomotion object created to move each instance of the type.
when_omitted:
  kind: value
  value: "{4A582747-9839-11D1-B709-00A024DDAFD1}"
  note: The teleport locomotor. No stock type names this identifier, and the only stock type that leaves the key out is the crate-goodie `BASEUNIT` shell.
---

The value is a class identifier written in the usual braced form. One locomotion object of the named class is created for each aircraft, infantry and vehicle instance as that instance is created. Everything about how the object travels, whether it drives, walks, hovers, flies, burrows or steps, follows from which class that is. [Locomotion and piggybacking](/internals/locomotion/) explains how a runtime instance can be moved by a different locomotor than its type names. Ten classes are registered:

| Identifier | Movement |
| --- | --- |
| `{4A582741-9839-11D1-B709-00A024DDAFD1}` | Drive: wheeled and tracked ground travel along the cell grid |
| `{4A582742-9839-11D1-B709-00A024DDAFD1}` | Hover: a cushion that slews about far more freely than a tracked hull |
| `{4A582743-9839-11D1-B709-00A024DDAFD1}` | Tunnel: burrows out of sight and travels underground |
| `{4A582744-9839-11D1-B709-00A024DDAFD1}` | Walk: infantry |
| `{4A582745-9839-11D1-B709-00A024DDAFD1}` | Ballistic: the falling [drop pod](/systems/drop-pods/) |
| `{4A582746-9839-11D1-B709-00A024DDAFD1}` | Flyer: aircraft |
| `{4A582747-9839-11D1-B709-00A024DDAFD1}` | Teleport |
| `{55D141B8-DB94-11D1-AC98-006008055BB5}` | Mech: the striding walk of the stock Titan and Juggernaut |
| `{92612C46-F71F-11D1-AC9F-006008055BB5}` | Jumpjet: powered hover flight run from `[JumpjetControls]` |
| `{3DC0B295-6546-11D3-80B0-00902792494C}` | Levitate: the stock jellyfish |

A BuildingType never creates a locomotor, so the setting is inert in a building's section. A reinforcement chain compares the stored identifier without creating anything. The chain burrows onto the map only if every member of it names the tunnel locomotor; a single member without it disqualifies the whole chain. A vehicle leaving a war factory is sorted by the locomotor it is actually using. A drive locomotor is forced onto the exit track. A tunnel locomotor is given a temporary drive locomotor piggybacked on top of it to get clear of the door. Anything else is simply given a destination three cells east and one south.

Text that is not a legal identifier is discarded and the previous value stays in force.

:::danger[An identifier that names no registered class crashes the game]
The identifier is handed straight to the object factory as the aircraft, infantry or vehicle is being constructed. Neither the factory's answer nor the resulting pointer is tested before the object is linked to it. A well-formed identifier that matches none of the ten above therefore brings the game down as the first instance of that type is created. For a type the computer or a map places, that happens while the scenario is still loading.
:::
