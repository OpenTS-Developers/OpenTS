---
key: DetectDisguise
summary: Lets an object see a disguised soldier for what it is when it scans for a target.
see_also: [Disguised, Disguise, AIDetectDisguise, "system:target-selection"]
when_omitted:
  kind: value
  value: "no"
---

```ini title="rules.ini"
[DOG] ; an InfantryType registered in [InfantryTypes]
DetectDisguise=yes
```

A [`Disguised=yes`](/keys/disguised/) soldier is normally [rejected while a candidate is being scored](/systems/target-selection/#why-a-candidate-is-rejected), so nothing picks one up by scanning. An object of a type set this way is not stopped by that rejection and scores a disguised soldier like any other candidate. The scanner needs no weapon: an object without one has candidates range-checked against its [`GuardRange=`](/keys/guardrange/) rather than a weapon's range.

The setting reads the scanning object's own type, not the house that owns it, so a player's own dog and a computer's alike see through a disguise. It changes nothing else about the disguise. The soldier still shows the substituted name, artwork and color, is still drawn in the local player's scheme, and is still avoided by anything deciding on its own whether to run something down.

Every other rule about a candidate applies unchanged. An ally is still rejected before the disguise is ever considered, so this cannot turn an object on a friendly spy.
