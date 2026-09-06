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

`FootClass::Locomotion` owns the current `ILocomotion` locomotor for one mobile runtime instance. The locomotor is a separate object linked to the `FootClass`; it is not a behavioral base class of `FootClass`.

## Object locomotion

`TechnoTypeClass::Locomotor` stores the CLSID used to create a type's ordinary locomotor. Concrete `FootClass` constructors create that locomotor, call `Link_To_Object`, and assign it to `FootClass::Locomotion`.

Movement, destination, layer, occupation, and locomotor-specific drawing queries go through the current interface. Code must therefore inspect the runtime `Locomotion` pointer when temporary locomotion is possible; the type's `Locomotor` CLSID describes the ordinary implementation, not necessarily the one currently in control.

## Piggybacking

`IPiggyback` lets one locomotor take control while retaining the previous locomotor for restoration.

| Operation | State transition |
| --- | --- |
| `Begin_Piggyback(previous)` | Takes ownership of `previous` and stores it inside the new locomotor. Answers `false`, destroying what it was handed, when nothing was passed or the slot is already occupied. |
| Replace `FootClass::Locomotion` | Makes the new locomotor the object's active movement interface. The new locomotor must already be linked to the same object. |
| `End_Piggyback()` | Hands the stored locomotor back to the caller and empties the piggyback slot. Answers nothing when no locomotor was stored. |

`FootClass::Link_DropPod` applies this sequence with the ballistic locomotor: it retains the passenger's current locomotor through `Begin_Piggyback`, then installs the ballistic interface. Drop-pod touchdown assigns the locomotor `End_Piggyback` returns back to `FootClass::Locomotion`, when the pod carried one, before attempting ground placement.

Callers that perform opportunistic restoration first consult `Is_Ok_To_End`. The drop-pod touchdown path calls `End_Piggyback` directly at ground contact because its descent state already establishes the transition.

## Persistence identity

`FootClass::Serialize` writes the active locomotor as a record of its own, headed by its class identifier, and recreates it from that identifier when loading. A piggyback-capable locomotor writes whether it carries another locomotor and serializes that nested locomotor when present. A save made during a temporary movement state therefore retains both the active locomotor and the one to restore.

`GetClassID` identifies the active locomotor implementation. `Piggyback_CLSID` returns the carried locomotor's `GetClassID` while piggybacking and the active locomotor's ID otherwise. These identities are distinct while a temporary locomotor is in control.
