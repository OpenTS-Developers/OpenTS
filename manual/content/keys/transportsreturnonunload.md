---
key: TransportsReturnOnUnload
summary: Releases each transport from the team at unload and sends it on the Move mission with whatever return point its record holds.
see_also: [Passengers, "system:ai-team-execution"]
when_omitted:
  kind: value
  value: "no"
---

The behavior belongs to the team rather than to the transport type. The team keeps one return point per transport in a shared record field, and three steps govern what that field holds.

1. Each time the team coordinates a move toward a target, every member whose type sets [`Passengers`](/keys/passengers/) above zero and has no return point yet stamps the cell it is standing in. The record survives later passes within the same script mission, because a member that already holds one is passed over. A reinforcement therefore records its entry position on its first move.
2. When the team advances to the next line of its script, including the advance into the Unload mission, it wipes every member's record and drops its own move target.
3. Once an [Unload](/mapping/missions/tmission-unload/) mission has emptied every transport, each transport is dropped from the team, given its record as destination, and put on the Move mission. Its record is cleared so a later trip can take a fresh one.

Unload itself records nothing: the move coordination that stamps records needs a team target, and the unload mission leaves the team with none. The approach stamps from step 1 are therefore gone by the time step 3 reads the record. Normally the record is empty at release, and the transport is sent on the Move mission with no destination. Whatever another order wrote into the shared field between the wipe and the release is what the transport receives instead.

The unload mission's own argument (the number on its script line) still decides what becomes of the members that have no passengers. It has no say over the transports: they are released whichever setting it names.
