---
key: VehicleThief
summary: Makes a soldier take the vehicle it is sent at, and lets it walk into that vehicle's cell.
see_also: ["system:capture"]
when_omitted:
  kind: value
  value: "no"
---

The capture cursor appears over anything that counts as a vehicle and is not a structure, a landed aircraft included, when it belongs to a different house; a unit type flagged [`NonVehicle=yes`](/keys/nonvehicle/) does not count as a vehicle. The test is house identity, not alliance, so an allied house's vehicle qualifies too. One exception: with the [HarvesterImmune](/keys/harvesterimmune/) truce switched on, a vehicle its list covers gets a select cursor instead. An [`IsTrain=yes`](/keys/istrain/) type is excluded from every cursor, targeting and movement path the feature uses. [Stealing a vehicle](/systems/capture/#stealing-a-vehicle) covers what the arrival does.

Two effects outlive the theft. The stolen vehicle counts against this type's [`BuildLimit`](/keys/buildlimit/) for as long as it lives, so a hijacker limited to one cannot be rebuilt while its prize survives. When that vehicle is destroyed, the hijacker is recreated at the wreck with a strength between 5 hit points and half its maximum, whether or not the vehicle type is [`Crewed`](/keys/crewed/).

The setting also changes what its owner scans for and what it will keep without scanning again, both covered by [target selection](/systems/target-selection/#what-each-kind-of-object-considers).
