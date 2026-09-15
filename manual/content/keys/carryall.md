---
key: Carryall
summary: Gives an aircraft a move mission that lifts and sets down whole vehicles.
see_also: ["Landable", "Passengers", "Dock", "Totable"]
when_omitted:
  kind: value
  value: "no"
---

The flag swaps the aircraft's move mission for one built around a single vehicle. Sent to an allied [`Totable=yes`](/keys/totable/) UnitType with nothing already aboard, the carryall opens radio contact and asks the vehicle for a ride. The vehicle refuses when it is unloading, already tethered to something, inside a tunnel, or bound for a cell under a bridge it is not climbing onto. Otherwise it agrees, holds still on the carryall's order, and waits while the carryall flies over it, takes it off the map and attaches it as cargo.

Sent anywhere while already loaded, it flies there and sets the vehicle down. If the ground will not take the vehicle, it stays aboard and the carryall tries somewhere else.

That handshake never reads [`Passengers`](/keys/passengers/). A carryall with no passenger capacity at all still lifts vehicles; the capacity setting governs only the ordinary loading a transport does.

```ini title="rules.ini"
[MYLIFTER] ; an AircraftType registered in [AircraftTypes]
Carryall=yes
Landable=yes
```

Under a player, hovering an empty carryall over an allied vehicle turns the cursor into the tote order, unless the vehicle is [`Totable=no`](/keys/totable/). The order is withdrawn again over a vehicle standing on a [`WeaponsFactory=yes`](/keys/weaponsfactory/) building's cell, so a carryall cannot pluck one off the factory it is leaving.

While something is aboard, the vehicle is drawn at the carryall's own position, its facing forced to the carryall's each frame. It visibly hangs beneath the aircraft. A loaded carryall, or one in radio contact, settles 100 leptons above the ground instead of touching down (a cell is 256). This is the height it works from when it picks up and puts down. An empty one sent to enter a helipad or a repair bay does land properly. Passing below 300 leptons on the way down plays the hard-coded `CARYLAND` animation, unless the type also sets [`IsDropship=yes`](/keys/isdropship/), whose animation is chosen first.
