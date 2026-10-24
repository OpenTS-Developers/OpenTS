---
key: Armory
summary: Admits one infantry at a time and promotes it when the servicing delay elapses.
see_also: ["system:veterancy"]
when_omitted:
  kind: value
  value: "no"
---

An `Armory=yes` building promotes infantry that the player sends into it. It takes one infantry at a time, holds it for the servicing delay that [`IRepairRate`](/keys/irepairrate/) sets, then promotes and releases it.

The player gets the enter cursor for their own infantry that is not yet elite, when the building is theirs or an ally's. The building must meet **all of** these conditions:

- it is not being built or sold;
- it is not already servicing another infantry;
- it is switched on;
- its [`Ammo`](/keys/ammo/) count is not zero.

The promotion depends on the infantry's rank when it enters. Infantry below rookie leaves as a veteran, and any other infantry leaves as elite, so a rookie skips the veteran rank. [`VeteranCap`](/keys/veterancap/) limits the result: with the default limit of `1`, every admission leaves the infantry veteran. [Promotion without kills](/systems/veterancy/#promotion-without-kills) compares the armory with the other sources of rank.

[Hospitals and armories](/systems/repair/#hospitals-and-armories) covers the full admission order and the servicing delay. At the default settings, the same `IRepairRate` keeps infantry in an armory about fourteen times longer than in a hospital.

:::caution[Set Ammo to the number of promotions]
Each admission uses one point of `Ammo`. Like a hospital, and unlike other buildings, an armory never restocks. A type that sets no `Ammo` admits one infantry and then refuses every other for the rest of the match. Set `Ammo` to the number of promotions the building should give.
:::
