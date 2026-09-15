---
key: TiberiumTransmogrify
summary: Parsed number that the engine never uses.
no_effect: true
see_also: ["system:tiberium"]
when_omitted:
  kind: context-dependent
  note: The read falls back to the stored field itself, and no code assigns that field before the read, so the not-set outcome is whatever memory the rules object was allocated with.
---
