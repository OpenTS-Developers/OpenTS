---
key: TiberiumHeal
scope: global-rules
label: Healing interval
see_also: ["system:repair"]
when_omitted:
  kind: value
  value: ".0167"
---

The value is a fraction of a minute, multiplied by 900 frames and truncated. That gives one healing step every 15 frames when the setting is omitted, and every 9 at the stock `.010`. The test is against the global frame counter, so every infantry, vehicle and aircraft standing in Tiberium with [`TiberiumHeal=yes`](/keys/tiberiumheal/#scope-aircrafttype) is healed on the same frames. Any value under `1/900`, `0` included, truncates to a zero interval and crashes the game the moment a damaged object with the flag occupies Tiberium.
