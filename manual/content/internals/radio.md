---
title: Radio contact protocol
summary: Synchronous object messages, reciprocal contact state, docking flows, and teardown rules.
category: simulation-systems
source_files:
  - code/radio.hh
  - code/radio.h
  - code/radio.cpp
  - code/object.cpp
  - code/techno.cpp
  - code/foot.cpp
  - code/unit.cpp
  - code/aircraft.cpp
  - code/building.cpp
  - code/swizzle.h
---

`RadioClass` provides synchronous messages between mission-capable map objects. It is used for docking, loading, repair, production, transport, and related movement coordination. It is not a queue and it does not represent network or voice communication.

## Contact state

Each `RadioClass` contains one non-owning `RadioClass*` contact. The pointer is the default destination for `Transmit_Message`; an explicit destination can receive a message without becoming the contact.

The table gives that pointer, the debug history kept beside it, and the four calls that read or change them. What to take from the last two rows is that delivery is synchronous: the receiver runs inside the sender's own call.

| Member or interface | Contract |
| --- | --- |
| `Radio` | Current non-owning contact, or null. Established contacts are expected to be reciprocal. |
| `In_Radio_Contact()` | Tests whether `Radio` is non-null. |
| `Contact_With_Whom()` | Returns the contact as `TechnoClass*`. |
| `Old[3]` | Debug history updated by `RadioClass::Receive_Message`. Only consecutive duplicates are suppressed; `A, B, A` retains both `A` entries. A derived handler that does not call the base does not update this history. |
| `Transmit_Message(...)` | Calls the receiver immediately and returns its `RadioMessageType` response. |
| `Receive_Message(...)` | Handles a message in the receiver's virtual override chain. |

The call may nest further radio calls before it returns. No message is deferred to a later frame.

## Dispatch rules

`Transmit_Message(message, param, to)` applies these rules in order:

1. A null `to` is replaced with the current contact.
2. If neither exists, the result is `RADIO_STATIC`.
3. Sending `RADIO_OVER_OUT` to the current contact clears the sender's pointer before the receiver is called.
4. `RADIO_HELLO` first sends `RADIO_OVER_OUT` to the sender's old contact. It then calls the proposed receiver. The sender stores the new contact only when the receiver returns `RADIO_ROGER`.
5. Every other message calls `to->Receive_Message(...)` without changing the sender's contact.

The sender passed to `Receive_Message` is `Dynamic_Cast<TechnoClass*>(this)`. A non-techno sender therefore reaches the receiver as null.

### `RADIO_HELLO` acceptance

The base `RadioClass::Receive_Message` accepts `RADIO_HELLO` only when all of the following are true:

- the receiver's `Strength` is nonzero;
- the receiver has no contact, or its contact is already the sender;
- the sender is a non-null `TechnoClass`;
- the receiver reports `Is_Techno()`;
- the sender's house treats the receiver as an ally; and
- the receiver's house treats the sender as an ally.

On acceptance, the receiver stores the sender and returns `RADIO_ROGER`. A nonzero receiver that fails another predicate returns `RADIO_NEGATIVE`. A zero-strength receiver bypasses the `RadioClass` HELLO branch and the base chain returns `RADIO_STATIC`. `Transmit_Message` normalizes every non-`RADIO_ROGER` HELLO response to `RADIO_NEGATIVE`. The sender stores the receiver only after `RADIO_ROGER`, which produces the reciprocal pair.

## Messages and responses

`RadioMessageType` contains both requests and responses. The receiver may return a general response or another protocol message that directs the caller's next step. The table groups that vocabulary by the job each group does, and names representative messages rather than every one of them; the enum comments in `radio.hh` are the complete vocabulary.

| Group | Representative messages | Purpose |
| --- | --- | --- |
| Contact and synchronization | `RADIO_HELLO`, `RADIO_OVER_OUT`, `RADIO_TETHER`, `RADIO_UNTETHER` | Establish, end, or tighten a relationship between two objects. |
| General responses | `RADIO_STATIC`, `RADIO_ROGER`, `RADIO_NEGATIVE`, `RADIO_CANT`, `RADIO_ALL_DONE` | Report absence, acceptance, refusal, failure, or completion. |
| Docking and cargo | `RADIO_CAN_LOAD`, `RADIO_DOCKING`, `RADIO_MOVE_HERE`, `RADIO_IM_IN`, `RADIO_UNLOAD`, `RADIO_UNLOADED` | Coordinate passengers, transports, harvesters, depots, helipads, and production exits. |
| Service and production | `RADIO_BUILDING`, `RADIO_COMPLETE`, `RADIO_REPAIR`, `RADIO_RELOAD`, `RADIO_PREPARED` | Advance construction, repair, and rearming protocols. |
| Combat and display | `RADIO_ATTACK_THIS`, `RADIO_REDRAW` | Assign a target or invalidate overlapping graphics. |

Concrete receiver implementations define which messages are valid for a particular class.

## Receiver overrides

Messages are handled from the concrete receiver toward the base classes. `UnitClass`, `AircraftClass`, `BuildingClass`, `FootClass`, and `TechnoClass` process their own protocol messages and delegate unhandled messages to `BASECLASS::Receive_Message`.

Some handled cases call the base implementation before applying class-specific effects. That base call preserves shared behavior such as radio history, contact teardown, tethering, or `ObjectClass` handling of `RADIO_REDRAW`. An override that returns without handling a message or delegating it changes the protocol for every base-class message.

## Parameter channel

The message parameter is an `int&`. It is used for both input and output:

- `FootClass` returns its current `NavCom` through `RADIO_NEED_TO_MOVE`.
- `RADIO_MOVE_HERE` interprets the value as an `ObjectClass*`.
- `RADIO_ATTACK_THIS` interprets it as an `AbstractClass*`.
- Building and aircraft handlers place cell or object pointers in it before sending a movement request.

:::danger[The parameter is untyped]
The parameter is an `intptr_t`, wide enough to carry a pointer, and it is never saved or transmitted. What it holds is decided per message and checked by nothing, so a sender and a receiver that disagree go wrong silently.
:::

The overload without an explicit parameter passes the global `LParam` by reference. It is appropriate only for messages that do not consume or modify the parameter. Parameterized or nested protocols should use an explicit local value; otherwise a nested call can overwrite state shared with its caller.

## Refinery docking trace

The refinery/harvester path demonstrates explicit destinations, reciprocal contact, return messages, and pointer parameters:

1. A unit on `MISSION_ENTER` sends `RADIO_DOCKING` directly to the target `BuildingClass`. No contact is required for this first call.
2. `BuildingClass::Receive_Message` rejects an off building. Otherwise, if the building has no contact, it sends `RADIO_HELLO` back to the unit; `RADIO_DOCKING` itself then returns `RADIO_ROGER`. Service eligibility is handled separately by `RADIO_CAN_LOAD`.
3. After contact is established, the building sends `RADIO_NEED_TO_MOVE`. `FootClass` returns its current navigation target through `param` and answers whether a new movement order can be accepted.
4. When `IsDockUnload` or `IsWeeder` is true, the building puts the docking `CellClass*` in `param` and sends `RADIO_MOVE_HERE`.
5. `FootClass` casts `param` back to an object pointer. It assigns that destination and returns `RADIO_ROGER`, or returns `RADIO_YEA_NOW_WHAT` when it already occupies the requested cell.
6. When the unit is in position, the building sends `RADIO_TETHER` followed by `RADIO_BACKUP_NOW`. The unit begins the refinery backup maneuver.

This protocol is spread across `UnitClass`, `FootClass`, `TechnoClass`, and `BuildingClass`. Altering one response requires tracing the callers that branch on it.

## Teardown, persistence, and synchronization

The cleanup paths are not interchangeable:

| Path | Effect |
| --- | --- |
| `RADIO_OVER_OUT` | Negotiated teardown. The sender clears `Radio` only when the explicit or default destination equals its current contact; the receiver clears only when the sender is its current contact. |
| `Limbo()` | Sends `RADIO_OVER_OUT` before the base limbo transition when the object is not already in limbo. |
| `Detach(target, all)` | Emergency pointer cleanup. It clears `Radio` only when it matches `target` and `all` is true; no message is sent. |
| Destructor | Performs no contact negotiation. Earlier lifecycle paths must already have removed the reference. |
| `Serialize()` | Serializes `Radio` as a swizzled pointer so the saved contact is remapped on load. |
| `Compute_CRC()` | Adds the contact's engine ID and RTTI to the synchronization checksum. |

A new exit path must either negotiate `RADIO_OVER_OUT` while both endpoints are valid or participate in the engine's detach sweep. A new persistent relationship also requires load swizzling and deterministic-state review.
