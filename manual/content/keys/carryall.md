---
key: Carryall
summary: Gives an aircraft a move mission that lifts and sets down whole vehicles.
see_also: ["Landable", "Passengers", "Dock", "Totable"]
when_omitted:
  kind: value
  value: "no"
---

The flag swaps the aircraft's move mission for one built around a single vehicle. Sent to an allied [`Totable=yes`](/keys/totable/) UnitType with nothing already aboard, the carryall opens radio contact, asks for a ride and tells the vehicle to hold still; the vehicle agrees unless it is unloading, already tethered to something, inside a tunnel, or bound for a cell under a bridge it is not climbing onto. The carryall then flies over it, takes it off the map and attaches it as cargo. Sent anywhere while already loaded, it flies there and sets the vehicle down — and if the ground will not take it, the vehicle stays aboard and the carryall goes looking for somewhere else.

That handshake never consults [`Passengers`](/keys/passengers/). A carryall with no passenger capacity at all still lifts vehicles; the capacity setting governs only the ordinary loading a transport does.

```ini title="rules.ini"
[MYLIFTER] ; an AircraftType registered in [AircraftTypes]
Carryall=yes
Landable=yes
```

Under a player, hovering an empty carryall over an allied vehicle turns the cursor into the tote order, unless the vehicle is [`Totable=no`](/keys/totable/). The order is withdrawn again over a vehicle standing on a [`WeaponsFactory=yes`](/keys/weaponsfactory/) building's cell, so a carryall cannot pluck one off the factory it is leaving.

While something is aboard, the cargo is drawn at the carryall's own position with its facing forced to the carryall's each frame, so the vehicle visibly hangs beneath it. A loaded carryall, or one in radio contact, settles 100 leptons above the ground rather than touching down — a cell is 256 — which is the height it works from when it picks up and puts down. An empty one sent to enter a helipad or a repair bay does land properly. Passing below 300 leptons on the way down plays the hard-coded `CARYLAND` animation, unless the type also carries [`IsDropship=yes`](/keys/isdropship/), whose animation is chosen first.
