---
key: Thief
summary: Makes a soldier take a non-allied vehicle it is walking toward.
see_also: ["system:capture"]
when_omitted:
  kind: value
  value: "no"
---

This is not the setting behind the vehicle hijacker; [`VehicleThief=yes`](/keys/vehiclethief/) is, and the two are separate features with separate cursors and separate limits. Nothing offers a cursor for this one. The steal runs on the soldier's own pass whatever mission it is on, and needs only a non-allied vehicle as its movement destination. Further out the soldier keeps re-aiming at the vehicle as it moves. Within half a cell and one height level of it, the vehicle changes hands and the soldier is consumed: it is deleted on the spot and never becomes a passenger. The two features meet in what the steal leaves behind, because the vehicle keeps a record of the type that took it. When that vehicle dies, a new soldier of the recorded type appears at the wreck, owned by whoever holds the vehicle at that moment. It arrives at a random strength between `5` and half its maximum, and only if the wreck cell has room for it.

The setting also [widens what its owner scans for](/systems/target-selection/#what-each-kind-of-object-considers), adding capturable structures and Tiberium processors to the request. The widening changes only which object the scan picks as a target. Reaching one is an ordinary attack: an armed thief fires on the structure, and an unarmed thief can do nothing there. No structure entry, credit theft, or drain follows; only the vehicle steal consumes a soldier.
