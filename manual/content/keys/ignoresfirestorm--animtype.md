---
key: IgnoresFirestorm
scope: animtype
label: Animation types
no_effect: true
when_omitted:
  kind: value
  value: "no"
---

An AnimType's art entry is read into the same flag as an object type's, but no firestorm test ever examines an animation. The four tests that read the flag look at units, aircraft, infantry and projectiles only, so the value stored here is never read.
