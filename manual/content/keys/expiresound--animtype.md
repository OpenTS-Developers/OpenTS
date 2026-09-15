---
key: ExpireSound
scope: animtype
label: Animation impact sound
see_also: ["ExpireAnim", "BounceSound", "Report"]
when_omitted:
  kind: value
  value: none
---

The sound plays where a thrown animation ends its life, unless that end comes low over water, where only the splash is heard. It is tested on its own rather than inside the [`ExpireAnim`](/keys/expireanim/#scope-animtype) branch that deals the blast damage. An animation with no impact animation still plays it.

A value naming no registered sound leaves whatever was set before in place.
