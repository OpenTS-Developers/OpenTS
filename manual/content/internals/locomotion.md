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

`TechnoTypeClass::Locomotor` stores the class identifier used to create a type's ordinary locomotor; the [`Locomotor`](/keys/locomotor/) assignment selects it. Concrete `FootClass` constructors create that locomotor, call `Link_To_Object`, and assign it to `FootClass::Locomotion`.

Movement, destination, layer, occupation, and locomotor-specific drawing queries go through the current interface. Code must therefore inspect the runtime `Locomotion` pointer when temporary locomotion is possible; the type's `Locomotor` identifier describes the ordinary implementation, not necessarily the one currently in control.

## Piggybacking

`IPiggyback` lets one locomotor take control while retaining the previous locomotor for restoration. Only the drive, walk, and drop-pod locomotors implement `IPiggyback`, and `LocomotionClass` supplies none of the four methods, `Begin_Piggyback`, `End_Piggyback`, `Is_Ok_To_End`, and `Is_Piggybacking`. A new locomotor that is to carry another must write all four.

| Operation | State transition |
| --- | --- |
| `Begin_Piggyback(previous)` | Stores `previous` inside the new locomotor and takes ownership of it. Answers `false` and leaves `previous` with the caller when there is nothing to store or the carrier already holds one. |
| Replace `FootClass::Locomotion` | Makes the new locomotor the object's active movement interface. The new locomotor must already be linked to the same object. |
| `End_Piggyback()` | Hands the carried locomotor back to the caller and releases its hold on it. Answers with an empty pointer when no locomotor was carried. |

`FootClass::Link_DropPod` applies this sequence with the drop pod's ballistic locomotor: it retains the passenger's current locomotor through `Begin_Piggyback`, then installs the ballistic interface. Drop-pod touchdown assigns the locomotor `End_Piggyback` returns back to `FootClass::Locomotion`, when the pod carried one, before attempting ground placement.

Callers that perform opportunistic restoration first read `Is_Ok_To_End`, which reports whether the locomotor may be handed back yet. `Is_Piggybacking` reports whether anything is carried at all. Every carrier refuses while the object is still moving or while it carries nothing. The drive locomotor also refuses while it is locked, and the walk locomotor while its own movement pass is running. The drop-pod touchdown path calls `End_Piggyback` directly at ground contact, because a pod always reports itself moving and its own `Is_Ok_To_End` would never agree.

## Persistence identity

`FootClass::Serialize` writes the active locomotor as a record of its own, headed by its class identifier, and recreates it from that identifier when loading. A piggyback-capable locomotor writes whether it carries another locomotor and serializes that nested locomotor when present. A save made during a temporary movement state therefore retains both the active locomotor and the one to restore.

`Class_ID` identifies the active locomotor implementation, and the carried locomotor keeps its own. These identities are distinct while a temporary locomotor is in control.
