---
key: Burst
summary: How many shots the weapon fires in quick succession before it waits out its full reload.
see_also: ["ROF", "BurstDelay0", "PrimaryFireFLH", "VoxelBarrelOffsetToBarrelEnd", "FiringSyncFrame1"]
when_omitted:
  kind: value
  value: "1"
---

The firing object keeps a running count of where it stands in the burst. Every shot but the last is followed by a short gap; the last shot of the burst is the one that pays [`ROF`](/keys/rof/), scaled by the house's rate of fire bias, raised by a random zero to two frames and shortened by the veteran rate of fire ability. The count then returns to the start.

Losing the target partway through a burst keeps that count and starts the same full rearm interval that the last shot would have paid. Remaining without a target until the interval ends returns the count to the first shot. Acquiring another valid target before then keeps the count, so firing resumes with the next shot of the partial burst. Clearing and immediately reissuing a target therefore cannot turn every shot into the first shot of a burst.

```ini title="rules.ini"
[MyTwinCannon] ; example WeaponType
Burst=2
ROF=60      ; paid after the second shot, not after each one
BurstDelay0=4 ; frames between the first shot and the second
```

The gap between shots within a burst comes from a [`BurstDelay0`](/keys/burstdelay0/), [`BurstDelay1`](/keys/burstdelay1/), [`BurstDelay2`](/keys/burstdelay2/) or [`BurstDelay3`](/keys/burstdelay3/) assignment on the weapon, chosen by which shot has just been fired. An entry left at its default of `-1`, and every shot after the fourth however the entries are set, takes a random three to five frames instead.

The lateral half of the firing slot's own fire offset — [`PrimaryFireFLH`](/keys/primaryfireflh/) or its secondary counterpart — is negated on every second shot, so a burst weapon mounted off the centerline alternates between a pair of muzzles; [`VoxelBarrelOffsetToBarrelEnd`](/keys/voxelbarreloffsettobarrelend/) does the same for a voxel barrel, with the third shot and any after it centered. A burst above one also doubles the rating a computer house gives the object when it weighs its base defenses.

A weapon that draws a wave or a particle system — [`IsSonic`](/keys/issonic/), [`IsRailgun`](/keys/israilgun/), [`UseFireParticles`](/keys/usefireparticles/) or [`UseSparkParticles`](/keys/usesparkparticles/) — takes `ROF` between every shot and never the short gap, so a burst on such a weapon changes only the muzzle it alternates to.

:::caution[A building's first upgrade decides whether both slots fire]
Once a structure has anything plugged into its first upgrade slot, it stops choosing between its two weapons. It fires both slots in the same frame when the plugged-in type counts as a two shooter — which means that type's own weapons name the same weapon in both slots, or either of them sets a burst above one — and fires neither slot at all when it does not.
:::

:::caution[Nonpositive values become one]
`0` and every negative value are stored as `1` when the WeaponType is read from the rules.
:::
