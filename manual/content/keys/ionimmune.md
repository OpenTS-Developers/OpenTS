---
key: IonImmune
summary: Exempts the team's members from ion storm lightning damage and from being aimed at.
see_also: [IonStormWarhead, LightningRod, "system:ion-storms"]
when_omitted:
  kind: value
  value: "no"
---

```ini title="ai.ini or map file"
[MyStormTeam] ; example TeamType
Name=Storm walkers
IonImmune=yes
```

The exemption covers two separate points. A member of such a team is skipped when an aimed bolt [builds its candidate list](/systems/ion-storms/#where-it-strikes), unless the member's own type sets [`LightningRod=yes`](/keys/lightningrod/), which puts it back in. Separately, explosion damage passes over a member whenever the detonating warhead is the one [`IonStormWarhead`](/keys/ionstormwarhead/) names, so a random bolt that lands on the team still leaves it untouched.

The immunity belongs to the team, not to the object type: a member that leaves the team loses it, and an object that never joined a team never had it. The storm's other effects are unaffected: an immune team's aircraft still crash and its hover units still lose power.
