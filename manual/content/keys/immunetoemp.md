---
key: ImmuneToEMP
summary: Stops an EM pulse from stunning, powering off, downing or destroying objects of this type.
see_also: ["system:emp-pulse", "IsCoreDefender", "EMEffect", "Cyborg"]
when_omitted:
  kind: context-dependent
  note: A BuildingType or UnitType uses its own `IsCoreDefender=` value. Every other type uses `no`.
---

```ini title="rules.ini"
[MYTANK]         ; a UnitType registered in [VehicleTypes]
ImmuneToEMP=yes
```

With `ImmuneToEMP=yes`, an [EM pulse](/systems/emp-pulse/) from any source, including an [`EMEffect=yes`](/keys/emeffect/) warhead, does not affect objects of this type. A vehicle, landed aircraft, cyborg or burrowing object is not stunned, an aircraft taking off or landing does not crash, a structure is not powered off or stunned, and a limpet mine is not destroyed.

An immune object still springs its [Paralyzed](/mapping/events/tevent-paralyzed/) trigger event when [a pulse reaches it](/systems/emp-pulse/#what-a-pulse-reaches).

The key matters only for objects a pulse can reach. Infantry other than [cyborgs](/keys/cyborg/), and aircraft one height level or more above the ground, are never affected, so the key changes nothing for them.

A structure or vehicle type that omits the key is immune when it sets [`IsCoreDefender=yes`](/keys/iscoredefender/), which keeps the stock Core Defender immune. Set `ImmuneToEMP=no` to make such a type vulnerable.
