---
title: EMP pulse
summary: "Immobilizes objects inside a warhead-sized radius for a weapon-defined number of frames."
category: weapons-projectiles
keys:
  - Damage
  - EMEffect
  - EMPulseCannon
  - EMPulseSparkles
  - ImmuneToEMP
  - IsMobileEMP
  - MaxCharge
  - Spread
  - StartCharge
related:
  - type: action
    id: TACTION_DO_EXPLOSION
---

## Firing a pulse

A projectile whose warhead sets [`EMEffect=yes`](/keys/emeffect/) creates a pulse where it detonates, instead of dealing blast damage. The warhead's [`Spread`](/keys/spread/#scope-warheadtype) sets the pulse's radius in cells. The firing weapon's [`Damage`](/keys/damage/#scope-weapontype) sets how long its stun lasts, in game frames.

In ordinary combat, the firer's firepower modifiers scale `Damage` before it becomes the duration. The EM pulse cannon and the mobile EMP vehicle below use `Damage` unscaled.

:::caution[Damage sets a duration]
A pulse deals no damage. Set `Damage` to at least `1`: with `Damage=0` the pulse stuns nothing and springs no trigger events.
:::

The explosion animation is picked at random from the warhead's [`AnimList`](/keys/animlist/), not by the size of `Damage`.

```ini title="rules.ini"
[EMPuls]           ; the warhead that makes the pulse
Spread=11          ; the pulse's radius, in cells
EMEffect=yes       ; detonation creates a pulse instead of blast damage

[EMPulseWeapon]    ; the weapon the pulse comes from
Damage=1200        ; the pulse's duration, in frames
Range=40           ; how far from the cannon the cursor allows a launch
Warhead=EMPuls

[NAPULS]           ; a structure that can serve as a launch site
EMPulseCannon=yes
SuperWeapon=EMPulseSpecial
Primary=EMPulseWeapon

[EMPulseSpecial]   ; the superweapon that launch site serves
Type=EMPulse
Action=EMPulse     ; the targeting cursor
```

### EM Pulse Cannon superweapon

A superweapon declared [`Type=EMPulse`](/keys/type/#scope-superweapontype) fires from a launch site. On launch it picks the structure nearest the target that meets **all of**:

- it belongs to the firing house;
- it is on the map, not in [limbo](/glossary/#limbo);
- its type sets [`EMPulseCannon=yes`](/keys/empulsecannon/);
- it is [operational](/systems/power/#defenses): not stunned, not switched off, and not idled by low power.

If no structure qualifies, nothing is fired, but the superweapon's charge is still spent. [Superweapons](/systems/superweapons/#em-pulse) covers why a one-time EM pulse never launches.

Each house holds one pulse target. A second launch before a cannon has fired therefore sends that cannon's shot to the new target.

Give the superweapon [`Action=EMPulse`](/keys/action/#scope-superweapontype) so the player can aim it. A click on the in-range cursor fires the first superweapon that declares `Action=EMPulse`, so two EM pulse superweapons cannot be aimed separately.

While the player aims an EM pulse superweapon, the cursor shows the in-range form only if the player owns an operational cannon and the target lies within the [`Range`](/keys/range/#scope-weapontype) of the nearest such cannon's primary weapon. Elsewhere it shows the out-of-range form. The superweapon's `Action` does not change which form appears.

A click on the out-of-range cursor fires the first superweapon that declares `Action=EMPulseRange`, and launches nothing if none does. The launch has no range limit of its own, so an EM pulse superweapon declared `Action=EMPulseRange` fires at any distance.

The chosen cannon turns toward the target and creates the hard-coded animation `PULSBALL` at its muzzle. Thirty-two frames later it fires its primary weapon at the target, and the pulse comes from that weapon's warhead. [`EMPulseWarhead`](/keys/empulsewarhead/) and [`EMPulseProjectile`](/keys/empulseprojectile/) in `[SpecialWeapons]` have no effect.

### Mobile EMP vehicle

A UnitType with [`IsMobileEMP=yes`](/keys/ismobileemp/) starts with [`StartCharge`](/keys/startcharge/) and gains one point of charge per frame up to [`MaxCharge`](/keys/maxcharge/). It gains nothing while stunned. Deploying does nothing until the charge is full, and the deploy cursor is refused until then.

With a full charge, deploying fires the weapon `MobileEMPulseWeapon` at the vehicle's position, resets the charge to `0`, and returns the vehicle to guard. The weapon name is hard-coded. The pulse does not stun the vehicle that fired it.

:::caution[Define `MobileEMPulseWeapon`]
Give that weapon a [`Projectile`](/keys/projectile/) and a [`Warhead`](/keys/warhead/#scope-weapontype) with `EMEffect=yes`. If either is missing, deploying still empties the charge but fires nothing. Without `EMEffect=yes`, the warhead deals ordinary blast damage around the vehicle instead of a pulse.
:::

### Scripted explosion

The [Do Explosion At](/mapping/actions/taction-do-explosion/) trigger action deals its weapon's ordinary blast damage at the waypoint. If the weapon's ID is `empulseweapon`, in any letter case, it then also creates a pulse there from that weapon's `Damage` and its warhead's `Spread`.

Because the same warhead deals the blast, `Spread` sets both the pulse's radius and the blast's falloff. The two cannot be tuned separately on this path.

A scripted pulse has no firer. Kills it causes are credited to no one.

## What a pulse reaches

A pulse takes effect once, on the frame it is created. It never grows, and an object that enters the radius afterward is unaffected. The pulse checks aircraft first, then burrowing objects, then every cell in its radius. That cell sweep covers every cell within `Spread` cells of the pulse's cell, measured in whole cells, boundary included.

An **immune** type is one that sets [`ImmuneToEMP=yes`](/keys/immunetoemp/). A structure or vehicle type that omits the key is immune when it sets [`IsCoreDefender=yes`](/keys/iscoredefender/).

### Aircraft

The pulse catches an aircraft when **all of**:

- it is on the map, not in limbo;
- its strength is above zero;
- it is less than one height level above the ground;
- its center is less than `Spread` cells from the center of the pulse's cell.

The distance also counts the aircraft's height above the map's lowest ground level. Each terrain height level adds about four tenths of a cell, so over raised terrain the reach shrinks. On high ground a small pulse can miss an aircraft in its own cell.

A caught aircraft springs the [Paralyzed](/mapping/events/tevent-paralyzed/) event on its trigger. If it is off the ground, it crashes unless its type is immune. Its strength drops to zero, the firer is credited with the kill, and its passengers die. An aircraft taking off or landing is caught this way.

An aircraft standing on the ground does not crash. The cell sweep below then treats it like a vehicle, so it springs Paralyzed a second time and, unless immune, is stunned. The sweep skips it if it stands in a structure's cell.

An aircraft one height level or more above the ground is out of the pulse's reach entirely, so a pulse under a flight of aircraft leaves them flying.

### Burrowing objects

An object traveling underground with a tunnel locomotor is stunned when its cell is less than `Spread` cells from the pulse's cell. An immune type is not stunned, but it still springs Paralyzed.

At either end of its trip the object stands on the surface, and the cell sweep handles it instead. The sweep also takes cells exactly `Spread` cells away, so at that distance an underground object escapes where a surface object would not.

### Structures

A structure is tested only on the cell that holds its center:

- An [`InvisibleInGame=yes`](/keys/invisibleingame/) structure is skipped entirely.
- An immune structure springs Paralyzed and nothing else.
- An [`IsLimpetMine=yes`](/keys/islimpetmine/) structure is destroyed, with the firer credited.
- Any other structure is powered off and stunned, and springs Paralyzed. A structure that is one of the [deployed-vehicle kinds](/keys/deploysinto/) also gets an [`EMPulseSparkles`](/keys/empulsesparkles/) animation.

:::caution[A structure is tested on one cell only]
A structure whose footprint overlaps the circle is untouched when its center cell lies outside it.
:::

### Vehicles and infantry

The sweep reaches only objects on the ground. A vehicle in the air, such as a jumpjet in flight, is out of reach.

The sweep skips every object in a cell that holds any part of a structure, even when that structure is outside the radius. Objects on a bridge are not reached either.

In other cells, the result depends on the object:

| Object | Result |
| --- | --- |
| Vehicle or aircraft | Stunned and springs Paralyzed |
| Infantry whose type sets [`Cyborg=yes`](/keys/cyborg/) | Stunned and springs Paralyzed |
| Either of the above with an immune type | Springs Paralyzed only |
| The vehicle or aircraft that fired the pulse | Nothing, or Paralyzed only if its type is immune |
| A [large](/keys/largevisceroid/#scope-unittype) or [small](/keys/smallvisceroid/#scope-unittype) visceroid | Nothing, unless its type is immune, which springs Paralyzed only |
| Other infantry | Nothing |

When several objects share a cell, as cyborgs can, the sweep may miss some of them.

:::caution[Ordinary infantry are never stunned]
Only cyborgs among infantry are stunned. Other soldiers walk through a pulse untouched and spring no event.
:::

## While stunned

A stun lasts the pulse's full duration. A second pulse sets the stun to its own duration, which can shorten a stun already running.

A stunned vehicle, cyborg or aircraft stops where it is and cannot move. It is given an [`EMPulseSparkles`](/keys/empulsesparkles/) animation. The player cannot deploy it, and a mobile EMP vehicle neither charges nor discharges.

A stunned object cannot fire. Unless it has the `CLOAK` ability or stands in its house's cloaking field, it [uncloaks](/systems/cloaking/) and cannot recloak until the stun ends.

A stunned structure is powered off: its lights, laser fence and cloaking field stop. Its powered animations stop too, but restart the next time its house's power balance changes while the house has enough power. The player cannot switch it back on or undeploy it. It cannot serve as a launch site for the EM pulse cannon. A stunned [`Radar=yes`](/keys/radar/) structure can take away the player's radar map, as [radar](/systems/power/#radar) describes.

A stun does not change a structure's power output or drain. A stunned power plant keeps feeding the grid, as [what each structure contributes](/systems/power/#what-each-structure-contributes) describes.

## Recovery

Each object recovers when its own stun runs out:

- A structure powers back on, and a `Radar=yes` structure can supply the radar map again. One the player had switched off stays off.
- A vehicle, cyborg or aircraft can move again.
- A harvester listed in [`HarvesterUnit`](/keys/harvesterunit/) is sent back to harvesting, unless it was unloading.

A sparkle animation still playing stops at the end of its current loop.
