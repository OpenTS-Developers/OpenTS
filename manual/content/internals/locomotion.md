---
title: Locomotion and piggybacking
summary: Defines runtime locomotor ownership, temporary replacement, restoration, and persistence identity.
category: simulation-systems
source_files:
  - code/iloco.h
  - code/ipiggy.h
  - code/loco.h
  - code/foot.h
  - code/foot.cpp
  - code/techtype.h
  - code/techtype.cpp
  - code/droppod.h
  - code/droppod.cpp
---

`FootClass::Locomotion` is the current `ILocomotion` COM interface for one mobile runtime instance. The locomotor is a separate object linked to the `FootClass`; it is not a behavioral base class of `FootClass`.

## Object locomotion

`TechnoTypeClass::Locomotor` stores the CLSID used to create a type's ordinary locomotor. Concrete `FootClass` constructors create that COM object, call `Link_To_Object`, and assign it to `FootClass::Locomotion`.

Movement, destination, layer, occupation, and locomotor-specific drawing queries go through the current interface. Code must therefore inspect the runtime `Locomotion` pointer when temporary locomotion is possible; the type's `Locomotor` CLSID describes the ordinary implementation, not necessarily the one currently in control.

## Piggybacking

`IPiggyback` lets one locomotor take control while retaining the previous locomotor for restoration.

| Operation | State transition |
| --- | --- |
| `Begin_Piggyback(previous)` | Stores `previous` inside the new locomotor. A null pointer returns `E_POINTER`; an already occupied slot returns `E_FAIL`. |
| Replace `FootClass::Locomotion` | Makes the new locomotor the object's active movement interface. The new locomotor must already be linked to the same object. |
| `End_Piggyback(&FootClass::Locomotion)` | Writes the stored locomotor back into the object member and releases the piggyback slot. No stored locomotor returns `S_FALSE`; a null output pointer returns `E_POINTER`. |

`FootClass::Link_DropPod` applies this sequence with the ballistic locomotor: it retains the passenger's current locomotor through `Begin_Piggyback`, then installs the ballistic interface. Drop-pod touchdown passes the address of `FootClass::Locomotion` to `End_Piggyback` before attempting ground placement.

Callers that perform opportunistic restoration first consult `Is_Ok_To_End`. The drop-pod touchdown path calls `End_Piggyback` directly at ground contact because its descent state already establishes the transition.

## Persistence identity

`FootClass::Serialize` writes the active locomotor as a record of its own, headed by its class identifier, and recreates it from that identifier when loading. A piggyback-capable locomotor writes whether it carries another locomotor and serializes that nested locomotor when present. A save made during a temporary movement state therefore retains both the active locomotor and the one to restore.

`GetClassID` identifies the active locomotor implementation. `Piggyback_CLSID` returns the carried locomotor's `GetClassID` while piggybacking and the active locomotor's ID otherwise. These identities are distinct while a temporary locomotor is in control.
