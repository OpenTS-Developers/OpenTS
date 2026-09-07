---
key: DropPodWeapon
summary: Selects the WeaponType used by airborne drop-pod effects.
see_also: ["system:drop-pods"]
when_omitted:
  kind: value
  value: none
---

The configured weapon drives both airborne effects of a descending pod: the smoke trail behind it and the covering fire on the landing cell. Without a weapon, pods fall silently and deliver no covering fire; every other part of the descent and landing is unaffected. [Descent and airborne effects](/systems/drop-pods/#descent-and-airborne-effects) covers what a configured weapon does.

:::danger[This assignment can change weapon numbering]
A weapon the rules [`[Weapons]` list](/formats/rules-registries/) leaves out is numbered where it is first named, and `[General]` is read before any object type names its own weapons. Adding, removing or renaming an unlisted weapon here therefore shifts the weapon numbers stored in [Do Explosion At](/mapping/actions/taction-do-explosion/) trigger actions. Listing the weapon settles its number instead.
:::
