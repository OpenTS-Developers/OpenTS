---
title: Building animations
summary: "Steps a structure's own artwork through five frame sequences and runs up to thirteen separate animations pinned to it, one per slot."
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

A structure moves on screen through two mechanisms that share almost nothing. Its own artwork has frames, and a small state machine steps through them. Separately, a structure runs up to thirteen attached animations. Each is an independent animation object, named in the [`[Animations]` list](/formats/rules-registries/), pinned to a point on the structure's artwork and cycling on its own timing, and the structure creates and destroys it as it changes state. The second mechanism is most of this page.

Each attached animation lives in a slot. A BuildingType declares one set of settings per slot, a structure holds at most one animation in each at a time, and its own events fill and empty each slot. A slot that no event reaches still stores every setting written for it, and creates nothing from them.

## The thirteen slots

The table lists every slot with the assignment that names its animation and the moment something puts one there. Take the last column as a pointer. The mechanism below governs how any slot's animation is built; the timing belongs to the setting that names it.

| Slot | Named by | Filled when |
| --- | --- | --- |
| Upgrade one, two, three | `PowerUp1Anim=`, `PowerUp2Anim=`, `PowerUp3Anim=`, each with a `PowerUp<n>DamagedAnim=`, `PowerUp<n>LocXX=`, `PowerUp<n>LocYY=`, `PowerUp<n>LocZZ=` and `PowerUp<n>YSort=` beside it | The first, second and third [`Upgrades=`](/keys/upgrades/) plug is installed. Removing a plug empties its slot |
| Active one to four | [`ActiveAnim=`](/keys/activeanim/), [`ActiveAnimTwo=`](/keys/activeanimtwo/), [`ActiveAnimThree=`](/keys/activeanimthree/), [`ActiveAnimFour=`](/keys/activeanimfour/) | The structure comes online, either as construction finishes or as the scenario places it |
| Pre-production | [`PreProductionAnim=`](/keys/preproductionanim/) | Work is being set up on the structure |
| Production | [`ProductionAnim=`](/keys/productionanim/) | The structure's work is in progress |
| Turret | [`TurretAnim=`](/keys/turretanim/) | The structure comes online, or its charge-up begins |
| Special one to three | [`SpecialAnim=`](/keys/specialanim/), [`SpecialAnimTwo=`](/keys/specialanimtwo/), [`SpecialAnimThree=`](/keys/specialanimthree/) | An event on a [`UnitRepair=yes`](/keys/unitrepair/), [`SiloDamage=yes`](/keys/silodamage/) or [`FirestormWall=yes`](/keys/firestormwall/) structure |

Two of the active slots have behavior the other two do not. On a [`SensorArray=yes`](/keys/sensorarray/) structure, slot one starts two seconds — 30 frames — after construction finishes. Every other path that starts slot one, and every path that starts the other three, applies no delay. Slot one is also the slot a [`UnitRepair=yes`](/keys/unitrepair/) bay stops while it services a vehicle. Slot two is the one a [`TurretAnimIsExclusive=yes`](/keys/turretanimisexclusive/) turret displaces while it charges.

The turret slot is the one slot whose animation does not run on its own timing. Its frame is set every pass, either from the structure's facing or from a charge counter stepped at [`TurretChargeAnimRate`](/keys/turretchargeanimrate/) on a [`ChargeAnim=yes`](/keys/chargeanim/) structure. [`TeslaCharge`](/keys/teslacharge/) sounds as that wind-up begins; [`TeslaZap`](/keys/teslazap/) sounds nowhere.

A structure's first active slot, written in its Image ID entry:

```ini title="art.ini"
[MYSTRUCT]                ; the Image ID entry this structure is drawn from
ActiveAnim=TURBINE_A      ; both names are AnimTypes from [Animations]
ActiveAnimDamaged=TURBINE_AD
ActiveAnimX=20
ActiveAnimY=-30
ActiveAnimYSort=14
ActiveAnimZAdjust=-5
```

## Building and emptying a slot

Whatever the occasion, one routine fills a slot. It takes the slot's healthy or damaged name and stops there if that name is empty. Otherwise it looks the name up in `[Animations]` and creates an animation at the structure's drawing position, offset by the slot's `…X` and `…Y` and with its two draw-order biases. A name that no `[Animations]` entry registers gets as far as the lookup and no further. Nothing is created and the slot stays as it was, though the flag recording which damage form the structure shows has already moved by then. Filling a slot that already holds an animation replaces it and hands the new one the frame the old one had reached, so the swap does not show as a jump.

One path bypasses the slot's names. The lettered turret variants a wall tower cycles through are built from the structure's Image ID with `_B`, `_C` or `_D` appended, and take the turret slot's offset and biases but not its name. A [`FirestormWall=yes`](/keys/firestormwall/) structure bypasses them too: its first two special slots are filled outside this routine, and [a firestorm wall section](/keys/specialanim/#a-firestorm-wall-section) covers what fills them.

A slot empties in three ways: its animation plays to its end, something stops it deliberately, or the structure is taken off the map, which stops all thirteen at once. The first is why a looping animation holds its slot for the rest of the structure's life while a finite one leaves it free.

A running animation belongs to the structure rather than to the cell. Placing the structure down re-applies every slot's offset from the new position. The structure's redraw pass brings its animations to the brightness and translucency it is drawn at, so one fading into a cloaking field takes them with it. Fog reaches them the same way. A structure passing under fog marks each of its animations fogged, and an animation created on an already fogged structure is marked as it is created. One whose AnimType leaves [`ShouldFogRemove`](/keys/shouldfogremove/) at its default is not drawn while that mark is set. Uncovering the structure clears it.

## The damaged form

Each slot has a second name for its damaged form. Which of the two is used is fixed as the animation is created, from whether the structure has fallen to [`ConditionYellow`](/keys/conditionyellow/) or below. A slot that names no damaged form is given its healthy name as the art file is read, so one name serves both states. A slot that names only a damaged form runs nothing while the structure is healthy.

A structure keeps one flag recording which form it is currently showing, and that flag is per structure rather than per slot. Creating any animation in a form different from the flag flips it and restarts every animation the structure is running in the new form. Crossing `ConditionYellow` in either direction does the same directly, so a structure that takes a hit past the threshold swaps its whole set at once.

The consequence worth planning around runs the other way. Any slot started in the healthy form drags the rest of them back to healthy, whether or not the structure is damaged. They stay there until the next damage step or repair step re-tests the threshold. [A repair bay's animations](/keys/specialanim/#a-service-depot) and [three of the four production moments](/keys/productionanim/) each do this.

## Placement and draw order

`…X` and `…Y` offset the animation from the point the structure is drawn at, in screen pixels: `…X` to the right, `…Y` down the screen. The `…` stands for the slot's own prefix, so the first active slot writes `ActiveAnimX=` where the second writes `ActiveAnimTwoX=`. The offset pins the animation to a point on the artwork rather than to a cell.

`…ZAdjust` and `…YSort` both decide what the animation ends up drawn over, through two different mechanisms.

- `…ZAdjust` biases the depth value the animation's pixels are tested against. A negative figure brings the animation toward the viewer, so it covers the structure and anything else at that depth; a positive figure pushes it away, so the structure covers it.
- `…YSort` is added to the animation's sorting position, in leptons. A [cell](/glossary/#cell) is 256 leptons. That position orders the ground layer, which is sorted once a frame. An AnimType left at the default [`Surface=no`](/keys/surface/) goes into the air layer instead, which is never sorted, so the bias reaches nothing there.

:::caution[The slot overrides the animation's own sort bias]
An AnimType may set its own [`YSortAdjust=`](/keys/ysortadjust/), but a slot writes its figure over that as the animation is created, including the zero it holds when `…YSort` is omitted. An animation used on its own keeps its bias; the same animation in a building slot loses it, unless the slot repeats the value.
:::

:::caution[Both biases are held in a single byte]
`…ZAdjust` and `…YSort` are each stored in one signed byte. A figure from -128 through 127 is kept as written; anything outside that range wraps into it. So `…ZAdjust=200` is stored as -56, and pulls the animation in front of the structure it was meant to hide behind. The same ceiling caps `…YSort` at rather less than half a cell in either direction.
:::

## Power

The two power flags are two responses to the same shortfall. On the first active slot they are `ActiveAnimPowered=` and `ActiveAnimPoweredLight=`. `…Powered=yes` freezes the animation on its current frame while the house cannot meet its drain, and resumes it at full power. The animation stays on screen throughout. `…PoweredLight=yes` instead destroys the animation on the shortfall and creates it again at full power.

Only one of the two is read. The pass over a house's structures tests `…Powered` first, and reaches `…PoweredLight` only when that flag is `no`. So `…PoweredLight=yes` left beside the default `…Powered=yes` never does anything; a structure that wants the light destroyed writes `…Powered=no` alongside it. That pass reaches all thirteen slots on every structure the house owns. It is the one route by which a structure with none of the three special-slot flags runs a special animation. [Fields, fences and lights](/systems/power/#fields-fences-and-lights) covers which structures a house-wide shortfall reaches and which it spares.

The power cursor, the [Turn off building](/mapping/actions/taction-turn-off-attached/) trigger action and an [EMP pulse](/systems/emp-pulse/) reach the freeze by a different route. They stop one structure's `…Powered=yes` animations directly, rather than through the house-wide pass. A `…PoweredLight=yes` animation keeps running through all three.

## Where each setting is read from

Only the first 15 characters of an animation name are kept. Where a slot's settings are read from is not uniform. The table gives the entry each half of a slot is taken from; the split shows only on a structure that borrows another's artwork, where the two entries are different sections.

| Slot | Names | The remaining settings |
| --- | --- | --- |
| Active one to four | The Image ID art entry | The Image ID art entry |
| Special, production and pre-production | The Image ID art entry | The art entry named after the ObjectType ID |
| Turret | The rules entry named after the ObjectType ID | The rules entry named after the ObjectType ID |

Everything but the turret slot reads its remaining settings only once the slot holds a name, healthy or damaged. On a structure that names neither, those settings are not read at all. The turret slot reads its four unconditionally, and both halves of it come from that one rules entry:

```ini title="rules.ini"
[MYTURRET]                ; the structure's own BuildingType entry
TurretAnim=TURRET_A       ; an AnimType registered in [Animations]
TurretAnimDamaged=TURRET_AD
TurretAnimX=0
TurretAnimY=-20
TurretAnimZAdjust=-10
TurretAnimYSort=0
```

## The upgrade slots and the active slots share one array

The thirteen slots are one array with the three upgrade slots at its head, and two paths walk into it by number rather than by name.

The art file is read one declared upgrade at a time: `PowerUp1Anim=` and its companions into the first slot, `PowerUp2Anim=` and its companions into the second. The walk is bounded by [`Upgrades=`](/keys/upgrades/) rather than by the number of upgrade slots. A type declaring four upgrades therefore has its fourth plug's six art assignments read over the first active slot's two names, its offset and both of its biases, each field only where that plug's assignment is present. A fifth, sixth and seventh declared upgrade reach active slots two, three and four in turn.

:::danger[Installing a plug rewrites the host type's animation name]
Placing a plug copies the plug type's own Image ID into the slot of the host type, not of the structure that received it, for the upgrade level being installed. Every structure of the host type shows that name in that slot from then on, including ones already standing and ones that never received a plug. The first three plugs write into upgrade slots, where that is the intended behavior. A fourth plug writes into the first active slot instead, and the host's `ActiveAnim=` is lost for the rest of the session.

The copy is made without a length check. A plug whose Image ID runs to sixteen characters or more is written into the sixteen-byte slot with no terminator, so the stored name runs on into the damaged name stored immediately after it.
:::

## The structure's own frames

The structure's own artwork has no slots in it. A structure is in one of five frame sequences at a time, four of them declared as a first frame, a frame count and a per-frame delay. [`AnimIdle`](/keys/animidle/) runs whenever nothing else has taken the structure over, and [`AnimActive`](/keys/animactive/) while it is working. [`AnimAux1`](/keys/animaux1/) and [`AnimAux2`](/keys/animaux2/) are the two further states a missile silo uses. The fifth is the construction sequence, which is not declared at all. Its frame count is half the frame count of the [`Buildup=`](/keys/buildup/) file, or [`GateStages`](/keys/gatestages/) plus one on a gate. Its rate is that count divided into [`BuildupTime`](/keys/builduptime/). A [`Theater=yes`](/keys/theater/) structure is timed again as each theater is set up, over every frame in the file at a fixed five seconds, so neither the halving nor `BuildupTime` reaches it.

Damaged artwork is a second block of frames after the first, and where that block begins depends on which sequence the structure is in. A damaged structure in the idle sequence draws the very next frame after the one it would otherwise draw. In any of the other three it is offset by the largest end any of the four sequences reaches. [`ExtraDamageStage`](/keys/extradamagestage/) promises a third condition and reaches nothing.

Four further shapes are drawn around the structure rather than as part of it, and none is an attached animation: [`BibShape`](/keys/bibshape/) under it, [`DeployingAnim`](/keys/deployinganim/) in place of it while it unloads, and [`DoorAnim`](/keys/dooranim/) with [`UnderDoorAnim`](/keys/underdooranim/) around its factory door. Each is a shape file named in the Image ID art entry and drawn from a frame the structure works out, so none cycles on its own. Despite the name, [`Bib=yes`](/keys/bib/) has nothing to do with the apron artwork. It opens the eastern edge of the footprint to vehicles and draws nothing.
