---
key: Landable
summary: Keeps an aircraft under the player's control instead of letting it enter as an unselectable loaner that may leave the map.
see_also: ["Selectable", "Carryall", "Camera"]
when_omitted:
  kind: value
  value: "no"
---

Clearing the flag marks every instance of the type a loaner as it enters the map: an aircraft the engine treats as borrowed rather than the player's own. [`Selectable=no`](/keys/selectable/#scope-aircrafttype) and a primary weapon with [`Camera=yes`](/keys/camera/) mark it the same way, and any one of the three is enough.

A player who can give it orders cannot select a loaner, neither by clicking it nor by holding shift. It stays outside that player's control while it flies. It is permitted to leave the map, which an ordinary object is not. A train, an object already on the retreat mission and a team on its way out are the only other exceptions. Once it is off, an aircraft with no target and no team is deleted. Idle and with no team, it never settles into guard like an ordinary aircraft. Carrying passengers it unloads first, empty it takes the retreat mission, and an armed one with ammunition left goes hunting instead. Running dry of ammunition puts it on the retreat mission as well, and it drops its target the moment it does. In a campaign it is also excused the rule that keeps a player's own aircraft out of shrouded cells.

Those retreat assignments are where it stops. The retreat mission has a routine of its own, and the routine does nothing at all. A loaner told to retreat picks no edge cell and flies nowhere. The permission to leave the map, and the deletion once it is off, apply only when something else carries it there.

Set the flag and none of that applies: the aircraft is selectable and may not leave the map. Idle in the air, an armed one looks for a free bay among its [`Dock`](/keys/dock/) buildings to settle on. It falls back to a nearby landing zone when it finds none. An unarmed one makes for a landing zone instead. An aircraft transport in a reinforcement team whose script includes an unload mission is marked a loaner when it is created, whatever this flag says.
