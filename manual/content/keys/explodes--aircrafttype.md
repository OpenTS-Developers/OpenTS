---
key: Explodes
scope: aircrafttype
label: Violent death
see_also: [CollateralDamageCoefficient, Explosion, ExpSpread, Primary, MaxDebris]
when_omitted:
  kind: value
  value: "no"
---

```ini title="rules.ini"
[MYAMMOTRUCK] ; a UnitType registered in [VehicleTypes]
Explodes=yes
```

A destroyed object of this type damages everything around it. The blast carries the warhead of the weapon the object would fire from its first slot — [`Primary`](/keys/primary/) ordinarily, [`Elite`](/keys/elite/) once the object is elite, and an upgrade's weapon on a structure carrying one — with no warhead at all when that slot is empty — and its strength is the object's [collateral damage figure](/keys/collateraldamagecoefficient/). A combat explosion animation and a lighting flash sized for that figure, that warhead and the land type under the object appear at its center, and the damage is then applied as area damage sourced to whatever killed it. The same branch is reached without the key by a veteran or elite object carrying the explodes ability.

The radius is that figure divided by 100, divided again by [`[CombatDamage] ExpSpread`](/keys/expspread/), and read as cells; it is clamped to between one lepton and three cells, and the damage applied over it is the figure multiplied by the whole number of cells the radius covers. The first division discards its remainder, so any collateral figure below 100 collapses the blast to a one-lepton radius and a single helping of damage.

Two object kinds do more with the flag on top of that blast:

- A vehicle swaps its death animation for the **last** entry of its [`Explosion`](/keys/explosion/) list, instead of a random one, whenever it still has ammunition — either an unlimited [`Ammo`](/keys/ammo/) pool or a count above zero.
- A structure sets its four neighboring cells alight where they hold an [explosive overlay](/keys/explodes/#scope-overlaytype), and it is not taken off the map on the frame it dies the way an ordinary structure is — which [turns out its survivors and marks its footprint a second time](/systems/destruction-and-debris/#when-the-structure-leaves-the-map).

An object that lost its footing and fell, and then died within ten leptons of water, splashes instead: it leaves the death path before the wreckage and before this blast.
