---
key: HasStupidGuardMode
summary: Ends an unarmed building's Guard mission at once instead of running its idle handling.
see_also: ["system:target-selection"]
when_omitted:
  kind: value
  value: "yes"
---

The setting is read only on the branch of the Guard mission taken by a building with no first-slot weapon. An armed building scans for targets on the other branch and never reaches it, so the setting cannot change what a defensive structure does.

Left at `yes`, Guard returns after 100 frames having done nothing. Set to `no`, the building runs its idle handling instead. A repair bay or reload bay whose customer is waiting at the door, within a quarter of a cell and free to move, switches to the repair mission. A weapons factory pushes any unit parked on its door cell or the cells beside it out of the way, so the exit stays free.
