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

A destroyed object of this type damages everything around it. The blast uses the warhead of the weapon the object would fire from its first slot: [`Primary`](/keys/primary/) ordinarily, [`Elite`](/keys/elite/) once the object is elite, and an upgrade's weapon on a structure with one. An empty first slot contributes no warhead at all. The blast's strength is the object's [collateral damage figure](/keys/collateraldamagecoefficient/). A combat explosion animation and a lighting flash appear at its center, sized for that figure, that warhead and the land type under the object. The damage is then applied as area damage sourced to whatever killed it. The same branch is reached without the key by a veteran or elite object whose type grants it the `EXPLODES` ability through [`VeteranAbilities`](/keys/veteranabilities/) or [`EliteAbilities`](/keys/eliteabilities/).

The radius is that figure divided by 100, divided again by [`[CombatDamage] ExpSpread`](/keys/expspread/), and read as cells. It is clamped to between one lepton and three cells, and the damage applied over it is the figure multiplied by the whole number of cells the radius covers. The first division discards its remainder, so any collateral figure below 100 collapses the blast to a one-lepton radius and a single helping of damage.

Two object kinds do more with the flag on top of that blast:

- A vehicle swaps its death animation for the **last** entry of its [`Explosion`](/keys/explosion/) list, instead of a random one, whenever it still has ammunition, meaning an unlimited [`Ammo`](/keys/ammo/) pool or a count above zero.
- A structure sets its four neighboring cells alight where they hold an [explosive overlay](/keys/explodes/#scope-overlaytype). It is also not taken off the map on the frame it dies the way an ordinary structure is, which [turns out its survivors and marks its footprint a second time](/systems/destruction-and-debris/#when-the-structure-leaves-the-map).

A vehicle or infantry that falls, through a bridge collapsing under it or a hover vehicle settling low into water, splashes if it dies within ten leptons of that water. The splash leaves the death path before the wreckage and before this blast. A shot-down aircraft never takes that path, so its blast runs even over water.
