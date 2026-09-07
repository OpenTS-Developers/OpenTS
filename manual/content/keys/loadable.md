---
key: Loadable
summary: Lets the player order infantry into a transport that belongs to a team of this type.
see_also: [Passengers, IsVehicleTransport, "system:ai-team-execution", "system:transports"]
when_omitted:
  kind: value
  value: "no"
---

Hovering an infantry over a transport that can take passengers normally offers the enter cursor. When that transport is on a team, the offer depends on this setting: a team of a `Loadable=yes` type lets the order through, and a team of any other type turns the cursor into the refusal form. A transport that is moving is refused either way, and one that is on no team at all is unaffected.

The test reads the transport's team rather than the team of the soldier being ordered aboard, and it runs only where a player-controlled house is looking at an allied object — so it governs what the player may do with a team, not what the team does with itself. A team loading its own members through the [Load onto Transport](/mapping/missions/tmission-load/) mission assigns the enter mission directly and never consults it.
