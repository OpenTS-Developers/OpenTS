---
command_id: StopObject
---

Stops each selected object that can move or fire and puts it on guard where it stands, whatever order it had. It drops its destination, its target and any waypoints still queued. A unit on area guard therefore no longer walks back to its guard spot, a hunting unit stops looking for targets across the map, a harvester stops working, and a transport that is unloading keeps the passengers still aboard. A vehicle that has already started turning to deploy still deploys.

Aircraft drop their destination and target but keep their order. Three kinds of object ignore the command: an object docked at a structure, such as a harvester unloading at a refinery; a structure being built or sold; and an object off a bridge or ramp whose cell is hidden on screen behind higher ground in front of it.
