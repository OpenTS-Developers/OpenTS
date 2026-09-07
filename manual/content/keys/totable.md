---
key: Totable
summary: Whether a carryall may pick this vehicle up.
see_also: ["Carryall", "Passengers"]
when_omitted:
  kind: value
  value: "yes"
---

```ini title="rules.ini"
[MYBIGTANK] ; a UnitType registered in [VehicleTypes]
Totable=no
```

[`Carryall=yes`](/keys/carryall/) owns the lift. This key decides only which vehicles are eligible for one, and every vehicle is until its own section says otherwise.

`Totable=no` withdraws the tote cursor, so a player cannot ask for the lift, and a move ordered onto the vehicle anyway — by force-move, or by a computer-controlled carryall keeping up with a team-mate — stays a move: the carryall counts the cell as occupied like any other and settles beside it. The refusal is by type and permanent, unlike the situational ones [`Carryall`](/keys/carryall/) lists, which lapse as soon as the vehicle is no longer unloading or tethered.

The flag reaches the lift alone. An aircraft with [`Passengers`](/keys/passengers/) capacity still takes a vehicle into its hold on the ordinary load handshake, and a scenario can still deliver one aboard an air transport as reinforcements.
