---
key: FirestormWarhead
summary: The WarheadType a raised firestorm wall destroys objects with.
see_also: [FirestormWall, C4Warhead, "system:laser-fences"]
when_omitted:
  kind: value
  value: none
---

An object caught in a raised wall section's own cell is dealt damage equal to its whole remaining strength through this warhead. The damage is forced and has no source, so armor and immunity do not save it and nothing is credited with the kill. The [approach sweep](/systems/laser-fences/#what-a-raised-section-destroys) beside it uses [`C4Warhead`](/keys/c4warhead/) instead, so the two paths do not share their impact effects.
