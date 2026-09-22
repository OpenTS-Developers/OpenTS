---
title: Building animations
summary: "Steps a structure's artwork through five frame sequences and runs up to thirteen attached animations pinned to it, one per slot."
category: buildings-economy
keys:
  - ActiveAnim
  - ActiveAnimDamaged
  - ActiveAnimX
  - ActiveAnimY
  - ActiveAnimZAdjust
  - ActiveAnimYSort
  - ActiveAnimPowered
  - ActiveAnimPoweredLight
  - ActiveAnimTwo
  - ActiveAnimTwoDamaged
  - ActiveAnimTwoX
  - ActiveAnimTwoY
  - ActiveAnimTwoZAdjust
  - ActiveAnimTwoYSort
  - ActiveAnimTwoPowered
  - ActiveAnimTwoPoweredLight
  - ActiveAnimThree
  - ActiveAnimThreeDamaged
  - ActiveAnimThreeX
  - ActiveAnimThreeY
  - ActiveAnimThreeZAdjust
  - ActiveAnimThreeYSort
  - ActiveAnimThreePowered
  - ActiveAnimThreePoweredLight
  - ActiveAnimFour
  - ActiveAnimFourDamaged
  - ActiveAnimFourX
  - ActiveAnimFourY
  - ActiveAnimFourZAdjust
  - ActiveAnimFourYSort
  - ActiveAnimFourPowered
  - ActiveAnimFourPoweredLight
  - SpecialAnim
  - SpecialAnimDamaged
  - SpecialAnimX
  - SpecialAnimY
  - SpecialAnimZAdjust
  - SpecialAnimYSort
  - SpecialAnimPowered
  - SpecialAnimPoweredLight
  - SpecialAnimTwo
  - SpecialAnimTwoDamaged
  - SpecialAnimTwoX
  - SpecialAnimTwoY
  - SpecialAnimTwoZAdjust
  - SpecialAnimTwoYSort
  - SpecialAnimTwoPowered
  - SpecialAnimTwoPoweredLight
  - SpecialAnimThree
  - SpecialAnimThreeDamaged
  - SpecialAnimThreeX
  - SpecialAnimThreeY
  - SpecialAnimThreeZAdjust
  - SpecialAnimThreeYSort
  - SpecialAnimThreePowered
  - SpecialAnimThreePoweredLight
  - ProductionAnim
  - ProductionAnimDamaged
  - ProductionAnimX
  - ProductionAnimY
  - ProductionAnimZAdjust
  - ProductionAnimYSort
  - PreProductionAnim
  - PreProductionAnimDamaged
  - PreProductionAnimX
  - PreProductionAnimY
  - PreProductionAnimZAdjust
  - PreProductionAnimYSort
  - TurretAnim
  - TurretAnimDamaged
  - TurretAnimX
  - TurretAnimY
  - TurretAnimZAdjust
  - TurretAnimYSort
  - TurretAnimIsVoxel
  - TurretAnimIsExclusive
  - TurretChargeAnimRate
  - ChargeAnim
  - TeslaCharge
  - TeslaZap
  - Upgrades
  - AnimIdle
  - AnimActive
  - AnimAux1
  - AnimAux2
  - ExtraDamageStage
  - DeployingAnim
  - DoorAnim
  - DoorStages
  - DamagedDoor
  - UnderDoorAnim
  - Bib
  - BibShape
  - SensorArray
  - UnitRepair
  - SiloDamage
  - FirestormWall
  - Surface
  - YSortAdjust
related:
  - type: system
    id: power
  - type: system
    id: repair
  - type: system
    id: production
  - type: system
    id: laser-fences
  - type: system
    id: emp-pulse
  - type: system
    id: cloaking
  - type: format
    id: shp
---

A structure animates in two independent ways. Its artwork steps through a few frame sequences, covered in [the last section](#the-structures-own-frames). It can also run up to thirteen attached animations. Each is a separate animation from the [`[Animations]` list](/formats/rules-registries/), pinned to a point on the structure's artwork and playing at its own rate. The structure creates and removes these animations as its state changes. Most of this page covers attached animations.

Each attached animation occupies an animation slot. A BuildingType has one set of settings per slot, and a structure runs at most one animation in each slot at a time. Events on the structure fill and empty the slots. A slot that no event fills never runs anything, whatever settings are written for it.

## The thirteen slots

The table lists each slot, the key that names its animation, and when the slot is filled. The key pages give the full timing for each kind of structure.

| Slot | Named by | Filled when |
| --- | --- | --- |
| Upgrade one, two, three | `PowerUp1Anim=`, `PowerUp2Anim=`, `PowerUp3Anim=`, each with a `PowerUp<n>DamagedAnim=`, `PowerUp<n>LocXX=`, `PowerUp<n>LocYY=`, `PowerUp<n>LocZZ=` and `PowerUp<n>YSort=` beside it | The first, second or third [`Upgrades=`](/keys/upgrades/) plug is installed. Removing that plug empties the slot |
| Active one to four | [`ActiveAnim=`](/keys/activeanim/), [`ActiveAnimTwo=`](/keys/activeanimtwo/), [`ActiveAnimThree=`](/keys/activeanimthree/), [`ActiveAnimFour=`](/keys/activeanimfour/) | The structure comes online, either when construction finishes or when the scenario places it. A repair step, or a plug installed on a damaged structure, refills any active slot that is empty. A repair step skips active slot two while a `TurretAnimIsExclusive=yes` turret is charging or charged |
| Pre-production | [`PreProductionAnim=`](/keys/preproductionanim/) | A construction yard begins a structure, or a harvester begins unloading into the structure |
| Production | [`ProductionAnim=`](/keys/productionanim/) | A construction yard finishes a structure, a factory releases what it built, a refinery finishes unloading a harvester, or a service depot begins a repair |
| Turret | [`TurretAnim=`](/keys/turretanim/) | A turret-equipped or [`ChargeAnim=yes`](/keys/chargeanim/) structure comes online, or the structure begins charging its weapon. With [`TurretAnimIsExclusive=yes`](/keys/turretanimisexclusive/), only charging fills it. A [`TurretAnimIsVoxel=yes`](/keys/turretanimisvoxel/) structure skips the fill when it comes online |
| Special one to three | [`SpecialAnim=`](/keys/specialanim/), [`SpecialAnimTwo=`](/keys/specialanimtwo/), [`SpecialAnimThree=`](/keys/specialanimthree/) | An event on a [`UnitRepair=yes`](/keys/unitrepair/), [`SiloDamage=yes`](/keys/silodamage/) or [`FirestormWall=yes`](/keys/firestormwall/) structure |

Three active-slot rules apply only to particular structures:

- On a [`SensorArray=yes`](/keys/sensorarray/) structure, active slot one starts 30 game frames (two seconds) after construction finishes. No other start of an active slot is delayed.
- A [`UnitRepair=yes`](/keys/unitrepair/) service depot stops active slot one while it repairs a vehicle. [A service depot](/keys/specialanim/#a-service-depot) covers when it starts again.
- A `TurretAnimIsExclusive=yes` structure stops active slot two while its turret charges, and starts it again once the charge is spent.

The turret slot's animation does not play at its own rate. The structure sets its frame on every update, from the direction the turret faces. On a `ChargeAnim=yes` structure the frame comes from a charge counter instead, which advances at [`TurretChargeAnimRate`](/keys/turretchargeanimrate/). [`TeslaCharge`](/keys/teslacharge/) plays when charging begins. [`TeslaZap`](/keys/teslazap/) is never played.

This example gives a structure one active animation, written in its Image ID art entry:

```ini title="art.ini"
[MYSTRUCT]                ; the Image ID entry this structure is drawn from
ActiveAnim=TURBINE_A      ; both names are AnimTypes from [Animations]
ActiveAnimDamaged=TURBINE_AD
ActiveAnimX=20
ActiveAnimY=-30
ActiveAnimYSort=14
ActiveAnimZAdjust=-5
```

`TURBINE_A` runs 20 pixels right of and 30 pixels above the structure's drawing point, and `TURBINE_AD` replaces it once the structure is damaged. The negative `ActiveAnimZAdjust` brings it slightly toward the viewer.

## Building and emptying a slot

Every slot is filled the same way. The structure picks the slot's healthy or damaged name, as described under [The damaged form](#the-damaged-form). If that name is empty, nothing happens. Otherwise the structure creates that animation at its drawing point, moved by the slot's `…X` and `…Y` offset and given the slot's two draw-order biases. Only the first 15 characters of an animation name are kept.

A name that no `[Animations]` entry registers creates nothing, and the slot keeps whatever it held. The form still changes: if the name belonged to the form the structure is not showing, every running animation switches to that form.

Filling a slot that already holds an animation replaces it. The new animation continues from the frame the old one had reached, so the swap does not show as a jump.

Two cases do not use the slot's names:

- The structure that [`WallTower`](/keys/walltower/) names cycles through lettered turret variants, named after its Image ID with `_B`, `_C` or `_D` appended. They use the turret slot's offset and biases.
- A [`FirestormWall=yes`](/keys/firestormwall/) section fills its first two special slots with fixed animations. [A firestorm wall section](/keys/specialanim/#a-firestorm-wall-section) covers them.

A slot empties when its animation plays to its end or when an event stops it. A looping animation therefore holds its slot until something stops it, while a finite one frees the slot when it ends. All thirteen slots empty when the structure begins to be sold or undeployed, and when it is removed from the game.

Attached animations follow the structure. When the structure is placed on a new cell, each animation moves to its slot's offset from the new position.

Cloaking fades the attached animations along with the structure. Animations that keep the default [`ShouldUseCellDrawer=yes`](/keys/shouldusecelldrawer/) are also drawn in the owner's colors at the structure's brightness.

Under fog of war, the player sees the structure's remembered image, with each attached animation frozen at the frame it had reached when fog covered it. The live animations are marked as fogged, and so is any animation created while the structure stays fogged. Uncovering the structure clears the mark. A marked animation is not drawn unless its AnimType sets [`ShouldFogRemove=no`](/keys/shouldfogremove/).

## The damaged form

Each slot has a healthy name and a damaged name. Most fills use the damaged name when the structure's health is at or below [`ConditionYellow`](/keys/conditionyellow/) at that moment. Some fills always use the healthy name; they are listed at the end of this section.

A slot that names only a healthy form uses that name in both states. The upgrade slots are the exception: an upgrade slot without `PowerUp<n>DamagedAnim=` keeps running its healthy animation while the structure is damaged.

A slot that names only a damaged form starts only when it is filled in the damaged form. It does not start while the structure is healthy, and it never starts from the fills listed below that always use the healthy name. If it starts while the structure is damaged, repairing the structure leaves that animation running.

Apart from those cases, a structure shows all its attached animations in one form at a time. Whenever a slot is filled in the other form, every animation the structure is running restarts in that form. Each damage or repair step checks the structure's health again, and if the result differs from the form on show, the whole set switches at once.

Some fills always use the healthy form, whatever the structure's health. On a damaged structure, each of them switches every running animation back to healthy. The set stays healthy until a damage or repair step, or until another slot is filled in the damaged form. The fills that do this are:

- the production slot's start on every structure except a construction yard ([`ProductionAnim`](/keys/productionanim/) lists them);
- the pre-production slot's start when a harvester begins unloading;
- every animation of [a service depot's repair sequence](/keys/specialanim/#a-service-depot);
- the fill indicator of a `SiloDamage=yes` structure.

## Placement and draw order

`…X` and `…Y` move the animation from the structure's drawing point, in screen pixels. A positive `…X` moves it right and a positive `…Y` moves it down. The `…` stands for the slot's key prefix, so active slot one uses `ActiveAnimX=` and active slot two uses `ActiveAnimTwoX=`. The offset pins the animation to a point on the artwork, not to a cell.

`…ZAdjust` and `…YSort` both decide what the animation is drawn over, in different ways:

- `…ZAdjust` shifts the depth the animation's pixels are tested at. A negative value brings the animation toward the viewer, so it covers the structure and anything else at that depth. A positive value pushes it back, so the structure covers it.
- `…YSort` is added to the animation's sorting position, in leptons; a [cell](/glossary/#cell) is 256 leptons. Sorting orders the ground layer once a frame. An AnimType left at the default [`Surface=no`](/keys/surface/) is drawn in the air layer, which is never sorted, so `…YSort` has no effect on it.

:::caution[The slot replaces the animation's sort bias]
An AnimType can set a sort bias with [`YSortAdjust=`](/keys/ysortadjust/). A building slot replaces that bias with the slot's `…YSort`, which is 0 when omitted. To keep the AnimType's bias in a building slot, repeat the value in the slot's `…YSort`.
:::

:::caution[Keep both biases between -128 and 127]
`…ZAdjust` and `…YSort` are each stored in one signed byte. A value outside -128 to 127 wraps around: `…ZAdjust=200` is stored as -56, which draws the animation in front of the structure it was meant to hide behind. The same limit keeps `…YSort` within about half a cell in either direction.
:::

## Power

Two flags decide what a slot's animation does when its house is short of power. Only the active and special slots have them; on active slot one they are `ActiveAnimPowered=` and `ActiveAnimPoweredLight=`.

- `…Powered=yes` freezes the animation on its current frame. It stays on screen and resumes at full power.
- `…PoweredLight=yes` removes the animation, and creates it again at full power.

Only one of the two flags is used. The structure tests `…Powered` first and reads `…PoweredLight` only when `…Powered` is `no`. Because `…Powered` defaults to `yes`, a slot that should use `…PoweredLight=yes` must also set `…Powered=no`.

The upgrade, pre-production, production and turret slots have no power flags and always behave as `…Powered=yes`. A frozen turret animation still follows the turret's facing, because the structure sets its frame directly.

The house applies these flags to every structure it owns each time it rechecks its power. At full power it resumes frozen animations and creates any missing `…PoweredLight=yes` animation on every structure. Below full power, only some structures freeze or remove their animations; [Fields, fences and lights](/systems/power/#fields-fences-and-lights) says which.

The full-power pass is the only way a structure that is not `UnitRepair=yes`, `SiloDamage=yes` or `FirestormWall=yes` runs a special animation.

Switching one structure off with the power cursor or the [Turn off building](/mapping/actions/taction-turn-off-attached/) trigger action freezes its `…Powered=yes` animations. Switching off also makes the house recheck its power. If the house has full power after the switch, that recheck resumes the animations at once. If the house is short of power, they stay frozen until the house next rechecks its power at full power.

An [EMP pulse](/systems/emp-pulse/) freezes the same animations. They resume when the structure recovers from the pulse, or earlier if the house rechecks its power at full power in the meantime.

Neither switching off nor an EMP pulse removes a `…PoweredLight=yes` animation unless the house is also short of power.

## Where each setting is read from

A slot's settings do not all come from the same section. The difference matters only for a structure that borrows another structure's artwork with [`Image=`](/keys/image/), so that its Image ID and ObjectType ID name different sections. The table gives the section each half of a slot is read from.

| Slot | Names | The remaining settings |
| --- | --- | --- |
| Upgrade one to three | The Image ID art entry | The Image ID art entry |
| Active one to four | The Image ID art entry | The Image ID art entry |
| Special, production and pre-production | The Image ID art entry | The art entry named after the ObjectType ID |
| Turret | The rules entry named after the ObjectType ID | The rules entry named after the ObjectType ID |

The active, special, production and pre-production slots read their remaining settings only when the slot has a name, healthy or damaged. A slot with neither name ignores them. The upgrade slots read theirs for each declared upgrade. The turret slot reads its four offset and bias settings in every case, from the same rules entry as its names:

```ini title="rules.ini"
[MYTURRET]                ; the structure's BuildingType entry
TurretAnim=TURRET_A       ; an AnimType registered in [Animations]
TurretAnimDamaged=TURRET_AD
TurretAnimX=0
TurretAnimY=-20
TurretAnimZAdjust=-10
TurretAnimYSort=0
```

## The upgrade slots and the active slots share one array

The three upgrade slots come first among the thirteen, directly before the four active slots. Two operations find an upgrade slot by its number, and both run past upgrade slot three into the active slots.

The first is reading the art file. It reads one set of `PowerUp<n>` settings per declared upgrade, up to the [`Upgrades=`](/keys/upgrades/) count: `PowerUp1Anim=` and its companions into upgrade slot one, `PowerUp2Anim=` into slot two, and so on. A type that declares four upgrades therefore reads `PowerUp4Anim=` and its five companions into active slot one. Each `PowerUp4` assignment that is present replaces the matching active-slot setting: its two names, its offset or one of its biases. A fifth, sixth and seventh upgrade reach active slots two, three and four in turn.

The second is installing a plug. The plug's Image ID becomes the host type's animation name for the upgrade slot being filled, replacing what `PowerUp<n>Anim=` gave. The animation that appears is therefore the `[Animations]` entry named after the plug's Image ID. The name is stored on the host's type, not on the one structure, so it applies to every structure of that type from then on.

:::danger[Keep Upgrades= at 3 or below]
A fourth plug fills active slot one. Its Image ID replaces the host type's `ActiveAnim=` name, so every structure of the host type runs the plug's animation in active slot one from then on. [`Upgrades`](/keys/upgrades/) covers the crash a fourth plug can also cause.
:::

:::caution[Keep a plug's Image ID to 15 characters or fewer]
A slot's name holds 16 characters. A plug's Image ID of 16 characters or more is copied without an end marker, so the stored name runs on into the damaged name stored after it.
:::

## The structure's own frames

A structure's artwork plays one of five frame sequences at a time. Four are declared in the Image ID art entry, each as a first frame, a frame count and a delay per frame:

- [`AnimIdle`](/keys/animidle/) runs whenever no other sequence has taken over.
- [`AnimActive`](/keys/animactive/) runs while the structure is working.
- [`AnimAux1`](/keys/animaux1/) and [`AnimAux2`](/keys/animaux2/) are used only by a missile silo, while its doors are held open and while they close.

The fifth is the construction sequence, which is not declared. It plays the [`Buildup=`](/keys/buildup/) file. Its frame count is half the frames in that file, or [`GateStages`](/keys/gatestages/) plus one on a gate, and the delay per frame spreads [`BuildupTime`](/keys/builduptime/) across that count. A [`Theater=yes`](/keys/theater/) structure instead plays every frame of its buildup file over five seconds, so neither the halving nor `BuildupTime` applies to it.

Damaged artwork is a second block of frames after the healthy ones, shown at or below `ConditionYellow`. Where the damaged frame lies depends on the sequence:

- In the idle sequence, it is the frame directly after the healthy one.
- In the active and auxiliary sequences, it is offset by the end of whichever of the four declared sequences ends last (its first frame plus its frame count).
- The construction sequence has no damaged frames. Gates, laser fence sections and firestorm wall sections choose their frames differently.

[`ExtraDamageStage`](/keys/extradamagestage/) is read but has no effect.

Four more shapes are drawn with the structure, but none of them is an attached animation. Each is a shape file named in the Image ID art entry, and the structure chooses its frame, so none plays on its own:

- [`BibShape`](/keys/bibshape/) is drawn under the structure.
- [`DeployingAnim`](/keys/deployinganim/) replaces the structure's artwork while it releases a unit.
- [`DoorAnim`](/keys/dooranim/) and [`UnderDoorAnim`](/keys/underdooranim/) are drawn around its factory door while it releases a unit.

Despite its name, [`Bib=yes`](/keys/bib/) does not draw the apron. It draws nothing, and lets vehicles drive onto the eastern edge of the structure's footprint.
