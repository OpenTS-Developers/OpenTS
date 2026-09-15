---
key: CrateRegen
summary: The range, in minutes, of the random lifetime given to each engine-placed crate.
see_also: ["system:crates"]
when_omitted:
  kind: value
  value: "10"
---

Each crate the engine places draws its own lifetime when it is created, uniformly between half this figure and twice it. The default of `10` produces crates that live between 5 and 20 minutes; `CrateRegen=3` produces crates that live between 1.5 and 6 minutes. The setting names a range, not a lifetime.

An expired crate is removed and replaced by a fresh random one, so the setting governs how often crates move around the map rather than how many there are. Crates drawn into a map's overlay layer and crates dropped by destroyed vehicles are not tracked and never expire, so this setting does not reach them.
