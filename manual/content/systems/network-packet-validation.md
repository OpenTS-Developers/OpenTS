---
title: Network packet validation
summary: Rejects malformed or misattributed multiplayer traffic before it changes synchronized state.
category: multiplayer-networking
keys: []
---

Private packets must fit the receiving buffers and pass their checksum, header,
packet-code, and payload-length checks. The complete event stream then decodes
before any event enters the simulation queue. Its leading `FRAMEINFO`, or sole
`FRAMESYNC`, supplies the frame and the identity already assigned to that
connection; compact events inherit that identity.

Frame arithmetic, collection indexes, animation selectors, production and
superweapon selectors, unit mission numbers, game speed, and the legacy latency
selector are checked before use. A production order names an existing vehicle,
infantry, aircraft, or structure type. A superweapon strike names one of the
sender's superweapons. A mission order carries a defined mission or none.
Power, archive-target, repair, primary-factory, mission, idle, deploy, scatter,
and sell commands require the affected object to still belong to their sender.
A missing or destroyed object remains a no-op, while a captured object rejects
its former owner's command. Timing changes require the master every machine
agrees on, the announced host while seated and otherwise the lowest seat, and
bounded, aligned values.

Public discovery remains public. In-game chat, progress, sign-off, ready, kick,
and movie-skip controls require one unique roster endpoint: an exact IP and
port, or one same-IP entry whose stored port is zero. Unknown private endpoints
are discarded rather than changing a roster address. Each survivor reports a
departure through the synchronized queue, where repeated removals are harmless.

The host announcement, the out-of-sync heartbeat, the decision to continue and a
request to load a saved game are session controls as well. The decision and the
load request are accepted only from the master, and a load request carries the
number of a [numbered save](/formats/save-games/#numbered-multiplayer-saves), below
one thousand, so it cannot name a file outside the saved-games folder. An
announcement is adopted only while no master is known
or it names the one already known.

Checksums and endpoint matching detect damage and attribute traffic; they do not
authenticate participants. Player removal remains a trusted-peer control.
