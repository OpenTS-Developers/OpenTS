---
title: Superweapons
summary: "How a house gains, charges and fires each declared superweapon, and what each of the seven hard-coded behaviors delivers."
category: superweapons-special
keys:
  - AIIonCannonAPCValue
  - AIIonCannonBaseDefenseValue
  - AIIonCannonConYardValue
  - AIIonCannonEngineerValue
  - AIIonCannonHarvesterValue
  - AIIonCannonHelipadValue
  - AIIonCannonMCVValue
  - AIIonCannonPlugValue
  - AIIonCannonPowerValue
  - AIIonCannonTempleValue
  - AIIonCannonThiefValue
  - AIIonCannonWarFactoryValue
  - Action
  - AuxBuilding
  - ChargeToDrainRatio
  - ChargingVoice
  - DamageToFirestormDamageCoefficient
  - FirestormWall
  - FirestormWarhead
  - GDIFirestormGenerator
  - GDIHunterSeeker
  - HSBuilding
  - ImpatientVoice
  - IonCannonDamage
  - IonCannonWarhead
  - IsPowered
  - ManualControl
  - NodHunterSeeker
  - NukeSilo
  - RechargeTime
  - RechargeVoice
  - SidebarImage
  - SuperWeapon
  - SuperWeapon2
  - SuperWeapons
  - SuspendVoice
  - Type
  - UseChargeDrain
  - WeaponType
  - WeedCapacity
related:
  - type: system
    id: drop-pods
  - type: system
    id: emp-pulse
  - type: system
    id: power
  - type: action
    id: TACTION_1_SPECIAL
  - type: action
    id: TACTION_FULL_SPECIAL
  - type: action
    id: TACTION_ACTIVATE_FIRESTORM
  - type: action
    id: TACTION_DEACTIVATE_FIRESTORM
---

Every superweapon has one rules section and one copy in each house. The section sets what all copies share: the recharge delay, the cameo, the mouse action and the behavior. Each house's copy keeps its own state: whether the house holds the weapon, whether it is suspended, and how far its countdown has run.

A weapon's **behavior** is the effect it delivers when fired. `Type=` selects one of seven behaviors built into the engine, and rules cannot add more. The **declared list** is `[SuperWeaponTypes]`, and a weapon's **position** in that list is the number trigger actions use to name it.

## Declaring a superweapon

`[SuperWeaponTypes]` lists the sections that declare superweapons.

```ini title="rules.ini"
[SuperWeaponTypes]
1=MultiSpecial
2=EMPulseSpecial
3=FirestormSpecial
4=IonCannonSpecial
5=HuntSeekSpecial
6=ChemicalSpecial
7=DropPodSpecial
```

The list is read in order, and each new name takes the next position. Later rules files, such as `FIRESTRM.INI` and a scenario's rules override, can add names. A name that first appears there goes after everything `rules.ini` declared. A name that is already declared keeps its position.

Each house receives one copy of every declared weapon when the house is created, in list order. The two trigger actions that grant a weapon, [Add 1-time special weapon](/mapping/actions/taction-1-special/) and [Add repeating special weapon](/mapping/actions/taction-full-special/), name it by its position. A structure names it by section name, in [`SuperWeapon=`](/keys/superweapon/) or [`SuperWeapon2=`](/keys/superweapon2/).

A listed name with no matching section is still declared, with every default. It charges for five minutes, has no cameo artwork, and does nothing when fired.

[`Type=`](/keys/type/#scope-superweapontype) selects the behavior. Several sections may use the same behavior, and each still has a separate cameo, countdown and grant condition. A section with no recognized `Type=` charges and shows a cameo like any other, but firing it has no effect.

:::danger[Keep `[SuperWeaponTypes]` in the shipped order]
A missile silo does not launch the `WeaponType=` of the weapon that ordered the launch. It uses the section whose position matches that weapon's behavior, in this order: `MultiMissile`, `EMPulse`, `Firestorm`, `IonCannon`, `HunterSeeker`, `ChemMissile`, `DropPod`. The first section in the list therefore supplies every `Type=MultiMissile` launch, and the sixth supplies every `Type=ChemMissile` launch. The shipped list follows this order.

If you insert, remove or reorder entries, a silo launches the [`WeaponType=`](/keys/weapontype/) of whichever section now holds that position. Two sections with the same missile behavior always launch the same missile. A list with fewer than six entries makes a chemical missile launch read past the end of the list.
:::

## Becoming available

A house updates which superweapons it holds after one of its structures is built, placed, sold, destroyed, captured, switched on or off, or fitted with a plug. On the house's next update, two passes run in order:

1. The first pass removes each weapon whose granting structures are gone, and suspends or resumes the rest. [Power output and drain](/systems/power/#superweapons) covers when a held weapon is suspended.
2. The second pass grants every weapon that the house's structures now provide.

A defeated house loses every weapon in the first pass, including one-time and trigger-granted weapons, and skips the second pass. When the house's power crosses the full-power line, only the first pass runs.

### From a structure or a plug

A house gains a weapon it does not already hold when it owns a structure that grants the weapon and is active and out of [limbo](/glossary/#limbo). A structure grants the weapons its type names in [`SuperWeapon=`](/keys/superweapon/) and [`SuperWeapon2=`](/keys/superweapon2/), and the weapons named the same way by a plug fitted to it. For the local player, the cameo appears on the sidebar at the same time.

```ini title="rules.ini"
[NAMISL]        ; Missile Silo
SuperWeapon=MultiSpecial
SuperWeapon2=ChemicalSpecial
NukeSilo=yes

[GAPLUG3]       ; Ion Cannon Uplink, a plug for the GDI Upgrade Center
PowersUpBuilding=GAPLUG
SuperWeapon=IonCannonSpecial
```

[`AuxBuilding=`](/keys/auxbuilding/) adds a condition to the grant that comes from a structure's type. A weapon that names an `AuxBuilding=` is granted by a structure's `SuperWeapon=` or `SuperWeapon2=` only while its house owns at least one standing structure of that BuildingType.

:::caution[A plug ignores `AuxBuilding=`]
A plug grants its weapon without the `AuxBuilding=` check, and the house keeps the weapon on the same terms. In the shipped rules the ion cannon and the drop pods come only from plugs, so an `AuxBuilding=` on either has no effect. The hunter seeker comes both from a plug and from the Nod temple, and the check applies to the temple's grant.
:::

A weapon granted while its house is short of power arrives suspended, even with [`IsPowered=no`](/keys/ispowered/), and its `ChargingVoice=` does not play. [Power output and drain](/systems/power/#superweapons) covers suspension by power and what it costs a charge-draining weapon.

### One shot at a time

The [Add 1-time special weapon](/mapping/actions/taction-1-special/) trigger action grants a single-use copy of the weapon at the position it names. A [missile crate](/systems/crates/#what-each-result-does) also grants one; the warning below explains which weapon.

A one-time weapon is fully charged the moment it is granted and is never suspended. It needs no structure, and it is removed from the house as soon as it fires. No voice announces the grant.

A house that already holds the weapon gains nothing from either path. Its copy keeps its current charge and stays a repeating weapon.

:::danger[Keep `Type=MultiMissile` and `Action=Nuke` on one section]
The missile crate checks that the collecting house has a `Type=MultiMissile` weapon. It then grants the first section in the declared list that sets [`Action=Nuke`](/keys/action/#scope-superweapontype), which need not be the same section. If a `Type=MultiMissile` section exists and no section sets `Action=Nuke`, collecting the crate crashes the game. If no `Type=MultiMissile` section exists, the crate grants nothing. The shipped rules put both on `MultiSpecial`.
:::

### Granted outright

The [Add repeating special weapon](/mapping/actions/taction-full-special/) trigger action grants the weapon and frees it from structures. An ordinary weapon recharges after every shot for as long as the house lives, and a `ManualControl=yes` weapon still waits for its [next start](#manual-control). Losing or selling structures cannot remove the weapon, and low power cannot suspend it. Only defeat removes it.

If the house already holds the weapon, the action only frees it from structures. A one-time copy the house already holds stays one-time and is still removed when it fires.

:::caution[Granting a weapon that is on hold]
If the weapon is suspended when the action runs, it stays suspended for the rest of the match. Only the structure check can resume a suspended weapon, and the action removes the weapon from that check.
:::

## Charging

[`RechargeTime=`](/keys/rechargetime/) sets the charge delay in minutes, which the game converts at 900 frames a minute. The weapon's countdown starts at that delay, and the weapon is ready when the countdown reaches zero.

The table lists the events that change an ordinary weapon's countdown. The weed pool and charge-draining weapons are covered below. A stopped countdown does not run, so the weapon stays uncharged until another event starts it again.

| Event | Effect on the countdown |
| --- | --- |
| Granted, ordinary weapon | started at the full delay |
| Granted, `ManualControl=yes` | set to the full delay and stopped |
| Granted as a one-time weapon | set to zero, so the weapon is ready at once |
| Suspended | stopped where it stands |
| Resumed, ordinary weapon | continues from where it stopped |
| Resumed, `ManualControl=yes` | stays stopped |
| Resumed, `UseChargeDrain=yes` | restarted at the full delay |
| Fired, repeating weapon | restarted at the full delay |
| Fired, `ManualControl=yes` | set to the full delay and stopped |
| Fired, one-time weapon | the weapon is removed from the house |

Voices play only for the local player's weapons. [`ChargingVoice=`](/keys/chargingvoice/) plays when a countdown starts: when a structure grants the weapon, after each shot, and when the weed pool restarts a chemical missile. [`RechargeVoice=`](/keys/rechargevoice/) plays when the countdown reaches zero. No voice plays when a trigger action or a crate grants the weapon, or when a suspended weapon resumes. A charge-draining weapon plays `ChargingVoice=` only when a structure grants it; its later charge and drain cycles are silent.

:::caution[`RechargeTime=0` counts as unset]
A value of exactly `0` is ignored. The weapon keeps the delay an earlier rules file set, or five minutes if none did. A near-instant recharge needs a small positive value.
:::

### Manual control

[`ManualControl=yes`](/keys/manualcontrol/) stops the countdown when the weapon is granted and again after every shot, so the weapon never charges on its own. The only event that starts it is a full weed pool, and only for a `Type=ChemMissile` weapon. A weapon of any other behavior with `ManualControl=yes` never charges, although a one-time grant still arrives fully charged.

When a house's [weed pool](/systems/veins/#the-weed-pool) holds exactly [`WeedCapacity`](/keys/weedcapacity/) units, the first `Type=ChemMissile` weapon the house holds that is not ready restarts its countdown at the full delay, and the pool is emptied. This happens with or without `ManualControl=yes`, and a countdown that is already running starts over.

Two states change what a full pool does:

- A ready weapon leaves the pool full. The countdown restarts as soon as the weapon fires.
- A suspended weapon still empties the pool, but its countdown does not start.

## Charge-draining weapons

[`UseChargeDrain=yes`](/keys/usechargedrain/) gives a weapon three states: charging, ready and discharged. Firing a ready weapon delivers its `Type=` effect and moves it to the discharged state, where the countdown measures how long the effect lasts. Firing it again during the drain returns it to ready. When the drain runs out, the weapon returns to charging with a full delay on the countdown, and the effect ends.

[`ChargeToDrainRatio`](/keys/chargetodrainratio/) converts between charge and drain time:

- On firing, the charge built so far (`RechargeTime` minus the time left on the countdown) is multiplied by the ratio and becomes the drain time.
- On firing again, the unspent drain is divided by the ratio and returned as charge.

An effect switched off the moment it starts therefore gives back the full charge. An effect switched off with half its drain left gives back half. A weapon returned to ready can be fired again at once, but its next drain lasts only as long as the charge it holds allows. Meanwhile its countdown keeps running toward a full charge.

A charge-draining weapon always shows a clock on its cameo and never shows the plain ready face. It can be fired while ready or discharged, but not while charging or suspended.

This cycle runs only for a house under human control. For any other house, firing the weapon only switches the effect on or off; the state and the countdown do not change.

### The firestorm defense

`Type=Firestorm` is the only behavior built for this cycle. Firing the weapon raises every [`FirestormWall=yes`](/keys/firestormwall/) structure its house owns, and firing it again lowers them.

Two events outside the cycle also change the wall. Damage aimed at a raised section [shortens the countdown](/systems/laser-fences/#damage-while-the-wall-is-up) through [`DamageToFirestormDamageCoefficient`](/keys/damagetofirestormdamagecoefficient/). Losing the last working [`GDIFirestormGenerator`](/keys/gdifirestormgenerator/) structure [lowers the wall](/systems/laser-fences/#losing-the-generator), as if the weapon had been fired again.

[The firestorm wall](/systems/laser-fences/#the-firestorm-wall) covers what a raised section does to what it touches, through [`FirestormWarhead`](/keys/firestormwarhead/) and the animations beside it.

## Firing it

### The sidebar cameo

A superweapon's cameo is the shape file named by [`SidebarImage=`](/keys/sidebarimage/), or `XXICON.SHP` when that file cannot be found. The cameo shows no numeric countdown. Its charge appears as a clock and a short caption. [The sidebar](/systems/sidebar/) covers where the cameo sits, how it is announced, how it is captioned, and when it leaves.

The caption depends on the weapon's state. A charge-draining weapon has a caption while it charges and a fourth state that ordinary weapons lack.

| Weapon state | Caption |
| --- | --- |
| Suspended | "On Hold" |
| Charging | none, or "Charging..." for a charge-draining weapon |
| Ready | "Ready", or "Release" for `Type=HunterSeeker` |
| Discharged, charge-draining | "Activated" |

The clock is drawn while the weapon is not fully charged, and on every charge-draining weapon. It uses the charge-up art while the weapon cannot be fired and the discharge art once it can.

Clicking a cameo that cannot be fired plays [`SuspendVoice=`](/keys/suspendvoice/) when its countdown is stopped and [`ImpatientVoice=`](/keys/impatientvoice/) otherwise. A `ManualControl=yes` weapon waiting for its next start has a stopped countdown, so it plays `SuspendVoice=`.

### Aiming and the click

[`Action=`](/keys/action/#scope-superweapontype) decides what clicking a cameo that can be fired does:

- `Action=None` fires the weapon at once at cell 0,0, with no targeting step. The shipped firestorm and hunter seeker are fired this way.
- Any other value arms targeting mode, deselects everything, and plays the select-target announcement.

While targeting mode is armed, the cursor over the map shows the weapon's `Action=` in place of the usual cursor. An EM pulse also shows an out-of-range cursor, which [the EM pulse cannon](/systems/emp-pulse/#em-pulse-cannon-superweapon) covers.

Releasing the left button fires the weapon at the cell under the pointer. A right click on the cameo or on the map cancels targeting.

Band selection is off while targeting mode is armed. The minimap does not accept superweapon actions, so a shot cannot be aimed there.

A left click fires the first section in the declared list whose `Action=` matches the click's action. This happens whether or not targeting mode is armed, and whichever cameo armed it. Two sections that share one `Action=` therefore always fire the earlier section. Clicking the later section's cameo arms targeting, but the shot fires the earlier section, or nothing if that section is not ready.

If a section's `Action=` is one that ordinary orders also use, the player's next such order fires the weapon whenever it is charged. For example, a ready weapon with `Action=Attack` fires at the target of the player's next attack order.

:::caution[A misspelled `Action=` becomes `None`]
An `Action=` value the engine does not recognize reads as `None`. Clicking the charged cameo then fires the weapon at once at cell 0,0, with no way to choose where the effect lands.
:::

### The computer's use

A computer house fires its ready superweapons during its periodic AI pass, which runs every 7 to 7.5 seconds. Outside a campaign, the pass always fires them. In a campaign, it fires them only when the house's [`IQ=`](/keys/iq/) is at least [`SuperWeapons`](/keys/superweapons/) in `[IQ]`. A campaign house takes its `IQ=` from its section in the scenario, and has 0 when the section sets none.

Each ready weapon goes to the handler for its `Type=`. There is no handler for `EMPulse` or `Firestorm`, so the computer never fires either on its own. Its firestorm wall goes up only through the [Activate Firestorm Defense](/mapping/actions/taction-activate-firestorm/) trigger action.

Every handler waits until the house has a [declared enemy](/systems/base-attacked/#picking-a-first-enemy). In a campaign the computer does not pick an enemy on its own, so a campaign house has none until damage or a trigger makes it angry at someone. Until then its superweapons stay charged and unused.

- **Multi missile and chem missile** target the enemy structure whose cell rates highest on the firing house's [threat map](/systems/base-attacked/#the-threat-map). A structure at full translucency, the last step of a cloak's fade, is rated at random from 0 to 100 instead. Every structure the enemy owns is considered, including one in [limbo](/glossary/#limbo).
- **Hunter seeker** is released with no target; the drone chooses one itself.
- **Drop pods** land around the computer's *own* base, not the enemy's. The handler picks a random point in one of four compass quadrants, one to two base radii from the base's center, with the radius held between 3 and 8 cells. It then aims at the nearest cell to that point that infantry can enter.
- **Ion cannon** rates every enemy object and strikes one of the highest rated.

The ion cannon's rating is the only one of the four with settings.

Only enemy objects that are on the ground layer, active and out of [limbo](/glossary/#limbo) are candidates. In difficulty slot 0, an object still being built also counts, if its factory is producing and not on hold.

Each candidate starts at a rating of 1, or 3 for a structure. A candidate whose current strength is at or below [`IonCannonDamage`](/keys/ioncannondamage/) takes its rating from the table below. A candidate above that figure keeps its starting rating. The test compares strength with the damage figure only; it does not predict whether the blast will destroy the object.

Only the highest rating matters. The computer collects every candidate that ties for the highest rating and strikes one of them at random. Apart from cloaked objects, described below, a table value of `4` and one of `40` therefore select the same target when nothing else rates 4 or higher. A table value replaces the starting rating; it is not added to it. A value below another candidate's rating ranks the object below that candidate. For example, an [`AIIonCannonConYardValue`](/keys/aiioncannonconyardvalue/) of `2` ranks a nearly destroyed construction yard below any structure still above `IonCannonDamage`, which keeps its starting 3.

The rows are tested from the top for each kind of object, and the first match wins. A base defense that also produces vehicles is therefore rated as a war factory. Rows marked as fixed cannot be changed by any rules file. Each per-difficulty list is read at the position of the *firing* house's [difficulty slot](/systems/difficulty/#from-the-setting-to-a-slot), not the target's. None of these lists has a built-in value, so each needs one entry for every difficulty.

| Candidate | Rating | Where the figure comes from |
| --- | --- | --- |
| Engineer | [`AIIonCannonEngineerValue`](/keys/aiioncannonengineervalue/) | Per-difficulty list |
| Vehicle thief | [`AIIonCannonThiefValue`](/keys/aiioncannonthiefvalue/) | Per-difficulty list |
| Any other infantry | `2` | Fixed in the engine |
| [`Factory=BuildingType`](/keys/factory/) structure | [`AIIonCannonConYardValue`](/keys/aiioncannonconyardvalue/) | Per-difficulty list |
| `Factory=UnitType` structure | [`AIIonCannonWarFactoryValue`](/keys/aiioncannonwarfactoryvalue/) | Per-difficulty list |
| Structure whose rated output beats its rated drain | [`AIIonCannonPowerValue`](/keys/aiioncannonpowervalue/) | Per-difficulty list |
| [`IsBaseDefense=yes`](/keys/isbasedefense/) structure | [`AIIonCannonBaseDefenseValue`](/keys/aiioncannonbasedefensevalue/) | Per-difficulty list |
| [`IsPlug=yes`](/keys/isplug/) structure | [`AIIonCannonPlugValue`](/keys/aiioncannonplugvalue/) | Per-difficulty list |
| [`IsTemple=yes`](/keys/istemple/) structure | [`AIIonCannonTempleValue`](/keys/aiioncannontemplevalue/) | Per-difficulty list |
| [`HoverPad=yes`](/keys/hoverpad/) structure | [`AIIonCannonHelipadValue`](/keys/aiioncannonhelipadvalue/) | Per-difficulty list |
| Any other structure | `4` | Fixed in the engine |
| [`Harvester=yes`](/keys/harvester/) vehicle | [`AIIonCannonHarvesterValue`](/keys/aiioncannonharvestervalue/) | Per-difficulty list |
| Vehicle whose [`DeploysInto`](/keys/deploysinto/) is a [`BuildConst`](/keys/buildconst/) type | [`AIIonCannonMCVValue`](/keys/aiioncannonmcvvalue/) | Per-difficulty list |
| Vehicle with [`Passengers`](/keys/passengers/) above zero | [`AIIonCannonAPCValue`](/keys/aiioncannonapcvalue/) | Per-difficulty list |
| Any other vehicle | `2` | Fixed in the engine |

No row covers aircraft, so an aircraft on the ground is a candidate rated 1 however badly damaged it is.

A cloaked object, or a structure at full translucency, takes a random rating instead, from 0 up to ten above the best rating found so far in the scan. It can therefore outrate everything scanned before it, and its chance depends on its place in the scan. The higher the best rating so far, the less likely the draw is to beat it, so large table values make cloaked objects rarely chosen. This rule is separate from the 0 to 100 draw the missile handlers use.

## What each behavior delivers

`Type=DropPod` calls the [drop-pod delivery](/systems/drop-pods/#drop-pods-superweapon) on the chosen cell, and `Type=Firestorm` toggles [the firestorm defense](#the-firestorm-defense). This section covers the other behaviors.

### Ion cannon

The blast lands on the target cell, at the height of that cell's terrain. It plays [`IonBlast`](/keys/ionblast/), or the last entry of [`SplashList`](/keys/splashlist/) over water, and always plays [`IonBeam`](/keys/ionbeam/).

The blast deals `IonCannonDamage` through [`IonCannonWarhead`](/keys/ioncannonwarhead/) with no attacker. The warhead's [`Verses`](/keys/verses/) table, its spread falloff and [`Immune=yes`](/keys/immune/) all apply, and no kill is credited. A bright warhead also lights the scene. A cell under a bridge is hit twice: once at bridge height and once at ground level.

A shockwave then rolls outward and stops the infantry and vehicles it passes near the target. Vehicles driving out of a war factory, or standing on its exit cells, keep moving.

### Multi missile and chem missile

A repeating missile launches from a silo. The engine finds the first BuildingType with [`NukeSilo=yes`](/keys/nukesilo/) that names this weapon in `SuperWeapon=` or `SuperWeapon2=`, and uses one of the house's structures of that type. If the house owns none, nothing launches, but the charge is spent. Only that first type is searched, so a silo of a later type that grants the same weapon never launches it.

The house stores a single missile target. A second launch before the first silo has launched its missile therefore redirects that silo. A missile already in flight keeps its target.

The silo opens its door, launches the missile, closes the door and returns to guard. The projectile, warhead, maximum speed and range come from the `WeaponType=` of the section the [declaration warning](#declaring-a-superweapon) describes. The silo ignores that weapon's `Damage=` and gives the projectile a fixed strength of 200. The missile leaves five eighths of a cell (160 leptons) north of the silo's center, pointing straight up. When the launching house is not the local player's, the player hears the launch-detected announcement.

A one-time missile needs no silo. It enters from the map edge nearest the target. It is built from the hard-coded weapon `MultiLauncher` or `ChemLauncher`, according to the behavior, and deals that weapon's `Damage=`. It is fired with a range of `100000` leptons, longer than any map is wide.

### Hunter seeker

The drone comes out of the house's structure whose type is listed in [`HSBuilding`](/keys/hsbuilding/). If the house owns several, the newest is used. If it owns none, the charge is spent and nothing launches.

The drone appears at the nearest cell to that structure that infantry could enter, even though the drone is a vehicle. If that cell lies outside the playable area, the region a scenario declares with `[Map] LocalSize=`, or the drone cannot be placed there, nothing launches and the charge is spent. A placed drone chooses a target and attacks it.

The drone type is the [`HunterSeeker`](/keys/hunterseeker/#scope-side) of the side the firing house [acts as](/keys/actslike/). In each rules file, [`GDIHunterSeeker`](/keys/gdihunterseeker/) and [`NodHunterSeeker`](/keys/nodhunterseeker/) in `[General]` set it for the first two sides in `[Sides]`, and a side's section in the same file overrides them. A house with no side, or whose side names no drone, spends the charge and launches nothing.

### EM pulse

The shot goes to the house's [EM pulse cannon](/systems/emp-pulse/#em-pulse-cannon-superweapon) nearest the target. That page covers everything the pulse then does.

:::caution[A one-time EM pulse launches nothing]
An EM pulse granted by the [Add 1-time special weapon](/mapping/actions/taction-1-special/) trigger action or by a missile crate spends its charge and is removed from the house, but no cannon fires. The same section granted by a structure or by [Add repeating special weapon](/mapping/actions/taction-full-special/) launches normally.
:::

## Scripting

Two trigger actions grant a weapon, and [becoming available](#becoming-available) covers what each one does. Two more toggle the firestorm defense. [Activate Firestorm Defense](/mapping/actions/taction-activate-firestorm/) and [Deactivate Firestorm Defense](/mapping/actions/taction-deactivate-firestorm/) each fire the house's first `Type=Firestorm` weapon at cell 0,0. Each does nothing when the wall is already in the state it asks for.

Three strike actions do not use superweapons at all: [Ion-cannon strike](/mapping/actions/taction-ion-cannon/), [Nuke strike](/mapping/actions/taction-multi-missile/) and [Chem-missile strike](/mapping/actions/taction-chem-missile/). They create the effect directly at the waypoint, so they need no weapon, no charge, no silo and no house that owns one.

The [Preferred target](/mapping/actions/taction-preferred-target/) action does not affect superweapons. A computer house aims each behavior as [the computer's use](#the-computers-use) describes.

## Parsed settings without effect

[`NukeProjectile`](/keys/nukeprojectile/) and [`NukeDown`](/keys/nukedown/) in `[SpecialWeapons]` are read but have no effect; a silo takes its projectile from a section's `WeaponType=` instead. [`EMPulseWarhead`](/keys/empulsewarhead/) and [`EMPulseProjectile`](/keys/empulseprojectile/) in the same section have no effect either. [The EM pulse cannon](/systems/emp-pulse/#em-pulse-cannon-superweapon) covers them.
