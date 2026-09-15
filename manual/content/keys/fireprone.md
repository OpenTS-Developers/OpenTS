---
key: FireProne
summary: The stage of the prone firing animation at which an infantry type releases its round.
see_also: ["FireUp", "Sequence", "Crawls"]
when_omitted:
  kind: value
  value: "0"
---

This is the same stage gate as [`FireUp`](/keys/fireup/), read instead of it while the soldier is prone. The figure counts stages of the run named `FireProne` in the type's [`Sequence`](/keys/sequence/) section, from `0`. The prone firing run ends at its frame count. The stage test keeps running while the soldier has a target, though, so a later animation played lying down can still release the round.

A type that declares [`FireUp`](/keys/fireup/) but not this key releases prone shots on the first stage of its prone firing run, since the two are separate figures with separate defaults.
