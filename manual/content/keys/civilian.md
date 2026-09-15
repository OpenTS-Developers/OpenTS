---
key: Civilian
summary: Counts the soldier as a civilian evacuation candidate and drops it from the surviving-infantry report.
see_also: [Nominal, Fraidycat, Disguised]
when_omitted:
  kind: value
  value: "no"
---

A soldier of such a type counts as a civilian evacuation unless it is a technician. Technician is the state an infantry of a [`Nominal=yes`](/keys/nominal/) type takes when a structure throws it out: on every sale, but only on the destruction of a structure that has buildup artwork. Two behaviors hang off that test, and both belong to aircraft alone. The moment one of these soldiers reports itself aboard an aircraft transport, the transport is put on the Retreat mission, whatever it was doing before. For an aircraft that mission is empty: the handler does nothing, so the transport never steers for the map edge, and it carries on with whatever movement it already had. The evacuation is a separate check, and the assignment alone never brings it about. It runs only when an aircraft on Retreat is outside the playable area, and it then deletes every passenger aboard. For each deleted passenger that counts as a civilian evacuation, it sets a flag on the *passenger's* house rather than the transport's. The flag is never cleared again, and nothing acts on it except the [Civilians Evacuated](/mapping/events/tevent-evac-civilian/) trigger event. That event is never announced to a trigger, so it is never satisfied, and its own condition would read the flag on the house owning the trigger rather than on the passenger's house. What a player sees, then, is a transport that goes nowhere new and a trigger that never springs.

The setting has no behavior of its own beyond that. Running from danger is [`Fraidycat=yes`](/keys/fraidycat/) and wearing another type's identity is [`Disguised=yes`](/keys/disguised/); neither follows from this one.

The surviving-infantry figures a multiplayer session reports at its end count every infantry a house still owns except these. A civilian on the map is invisible to that tally.
