---
key: ActsLike
summary: The country a scenario's house plays as, whose things its construction yard builds.
see_also: [Owner, HunterSeeker, MultiplayPassive, "system:production", "system:ai-base-building"]
when_omitted:
  kind: computed
  note: The house's own country, or no country at all for a country that takes no part in the multiplayer contest.
---

The value names a country, by its identifier or by its position in the rules country list, and `<none>` names no country at all. A value naming no country is logged and leaves the default standing. Only a campaign mission reads its house records, so this is a campaign setting; outside a campaign every house acts for its own country.

```ini title="scenario map file"
[Special] ; a house record in the scenario's own house list
ActsLike=Nod ; play as Nod; ActsLike=1 names the same country by position in the stock rules
```

Four things consult it.

- Every role list the computer plans from, prices and is handed — [construction yards](/keys/buildconst/), [power plants](/keys/buildpower/), [barracks](/keys/buildbarracks/), [factories](/keys/buildweapons/), [radars](/keys/buildradar/), [tech centers](/keys/buildtech/), [refineries](/keys/buildrefinery/), [harvesters](/keys/harvesterunit/), [walls](/keys/concretewalls/) and [gates](/keys/ewgates/) — resolves to the first entry the country named here may own, and the candidates a [base plan](/systems/ai-base-building/) is built from are filtered on that country's [`Owner`](/keys/owner/) bit. A house acting for no country plans nothing.
- Every object the house creates copies the value into a record of its own, which is what survives a capture. A construction yard can only put up a BuildingType whose [`Owner`](/keys/owner/) list includes the country its own copy names, so a captured yard keeps building for the country it was built by.
- A hunter-seeker discharge creates the [`HunterSeeker`](/keys/hunterseeker/#scope-side) of the country's side.
- An AI trigger restricted to one side runs only for a house whose country here belongs to that side.

The default is the house's own country whatever it is called: a country named `GDI-Reserve` no longer inherits a country from its name. The stock `Neutral` and `Special`, and any other [`MultiplayPassive=yes`](/keys/multiplaypassive/) country, act for no country at all unless the record says otherwise.
