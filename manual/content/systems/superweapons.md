---
title: Superweapons
summary: "Charges one timer per house for every superweapon a rules section declares, and delivers that section's hard-coded effect where the shot is aimed."
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

A superweapon exists in two layers. The rules section declares one weapon for the whole game and fixes its recharge delay, its cameo, its mouse action and its behavior. Every house then carries its own copy of that weapon, with its own countdown and its own availability, so two houses holding the same weapon share nothing but the settings.

Three words carry that split through the rest of the page. A weapon's **behavior** is one of seven effects wired into the engine and chosen by `Type=`; a section picks one of the seven and cannot add to them. The **declared list** is the order `[SuperWeaponTypes]` puts the sections in, and a weapon's **position** in that list is the only name every later reference uses for it. Everything a house's own copy holds — whether the house has the weapon, whether it is suspended, and where its countdown stands — is per house and is written in no section.

## Declaring a superweapon

`[SuperWeaponTypes]` is a numbered list whose values name the sections that describe each weapon.

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

The list is read in order and each name the game has not already heard of takes the next position at the end of the declared set. `FIRESTRM.INI` and a scenario's own rules override are read afterward through the same path, so a name that appears only there is appended after everything `rules.ini` declared. Every house is given one copy of each declared weapon, in the same order, as it is created. That position is the number every later reference uses: [`SuperWeapon=`](/keys/superweapon/) on a structure, the parameter of the two granting trigger actions, and the place a house's copy occupies among its own set.

A name listed with no matching section is still declared. It keeps every default, which means it charges for five minutes, shows no artwork at all, and does nothing when fired.

[`Type=`](/keys/type/#scope-superweapontype) chooses which of seven hard-coded behaviors the section delivers. Several sections may name the same behavior; each still gets its own cameo, its own timer and its own grant condition, and a section left with no recognized behavior charges and shows a cameo like any other but delivers no effect.

:::danger[Reordering `[SuperWeaponTypes]` misdirects a missile silo]
A missile silo remembers the behavior number of the weapon that ordered the launch, then reaches into the declared list at that number to fetch the projectile, warhead, speed and range it fires. The two agree only while the list is declared in exactly the behavior order `MultiMissile`, `EMPulse`, `Firestorm`, `IonCannon`, `HunterSeeker`, `ChemMissile`, `DropPod` — the order the shipped list above happens to use. Inserting, removing or reordering an entry makes the silo launch the [`WeaponType=`](/keys/weapontype/) of whichever section now stands in that position, and a position past the end of the list is not checked before it is read.
:::

## Becoming available

A house re-examines its superweapons on its own turn, whenever something has flagged its build options for a rebuild — a structure placed, sold, destroyed, captured, switched on or off, or fitted with a plug. Two passes then run in sequence: one takes away or suspends what the house can no longer support, the other switches on everything its structures now grant. A house that has been defeated is stripped by the first pass and skipped entirely by the second. A power balance that crosses the full-power line runs the first pass on its own, without the second.

### From a structure or a plug

For every weapon the house does not already hold, the engine walks the structure list from its end backward and keeps the entries this house owns that are active and out of [limbo](/glossary/#limbo). The first such structure that either names the weapon in [`SuperWeapon=`](/keys/superweapon/) or [`SuperWeapon2=`](/keys/superweapon2/), or carries a plug whose type does, grants it. When the house is the local player's, the same step puts the cameo on the sidebar.

```ini title="rules.ini"
[NAMISL]        ; Missile Silo
SuperWeapon=MultiSpecial
SuperWeapon2=ChemicalSpecial
NukeSilo=yes

[GAPLUG3]       ; Ion Cannon Uplink, a plug for the GDI Upgrade Center
PowersUpBuilding=GAPLUG
SuperWeapon=IonCannonSpecial
```

[`AuxBuilding=`](/keys/auxbuilding/) is tested on the structure's own grant: a weapon that names one is withheld unless its house has at least one standing structure of that BuildingType.

:::caution[A plug is never checked against `AuxBuilding=`]
The upgrade slots are matched straight against `SuperWeapon=` and `SuperWeapon2=` without the test the host structure's own settings go through. A weapon supplied by a plug is therefore granted whether or not its `AuxBuilding=` stands, and the later pass keeps it present for the same reason. In the stock set the ion cannon and the drop pods arrive only through plugs, so an `AuxBuilding=` on either has no effect; the hunter seeker arrives both from a plug and from the Nod temple's own grant, and on that grant the test does apply.
:::

A weapon that becomes available while its house is already below full power is granted suspended, whatever [`IsPowered=`](/keys/ispowered/) says. Suspension by power, and what it costs a charge-draining weapon, are covered by [power output and drain](/systems/power/#superweapons).

### One shot at a time

Two paths hand out a single-use weapon. The [Add 1-time special weapon](/mapping/actions/taction-1-special/) trigger action names the position of a weapon in the declared list; a [missile crate](/systems/crates/#what-each-result-does) picks one of its own. A one-time weapon is forced to full charge the instant it is granted, is never suspended, and is taken away from the house the moment it discharges.

:::danger[The ICBM crate tests one section and hands out another]
The crate looks through the opening house for a weapon whose `Type=MultiMissile`, discards what it found, and then hands out the first section in the declared list whose [`Action=Nuke`](/keys/action/#scope-superweapontype). When a `Type=MultiMissile` section exists and no section declares `Action=Nuke`, that second lookup returns nothing and the crate uses the result anyway, which faults. Keep the two on one section, as the shipped rules do.
:::

### Granted outright

The [Add repeating special weapon](/mapping/actions/taction-full-special/) trigger action grants a weapon and cancels its dependence on a building at the same time. Such a weapon recharges for as long as the house lives: the removal pass no longer examines it, so no structure can be sold to take it away, and low power can never suspend it. Only defeat removes it.

## Charging

[`RechargeTime=`](/keys/rechargetime/) is authored in minutes and converted to game frames at 900 frames to the minute. The timer counts down toward zero and the weapon becomes ready when it arrives.

Every event that touches that timer is listed below, with what it leaves on the clock. Two distinctions are what the table is for: which events put the full delay back rather than resuming from where the clock stopped, and which leave the timer stopped, because a stopped timer never reaches zero on its own and the weapon waits for something else to start it.

| Event | Effect on the timer |
| --- | --- |
| Granted, ordinary weapon | started at the full delay |
| Granted, `ManualControl=yes` | set to the full delay and stopped |
| Granted as a one-time weapon | forced to zero, ready at once |
| Suspended | stopped where it stands |
| Resumed, ordinary weapon | continues from where it stopped |
| Resumed, `UseChargeDrain=yes` | set back to the full delay |
| Discharged, repeating weapon | restarted at the full delay |
| Discharged, `ManualControl=yes` | set to the full delay and stopped |
| Discharged, one-time weapon | the weapon is removed from the house |

Only the local player's own weapon is announced: [`ChargingVoice=`](/keys/chargingvoice/) speaks as an ordinary weapon starts its charge and [`RechargeVoice=`](/keys/rechargevoice/) as it reaches zero. Every other house charges silently. A charge-draining weapon still speaks `ChargingVoice=` on the grant that starts it, but announces neither end of its running cycle after that.

:::caution[`RechargeTime=0` reads as if the key were absent]
The recharge delay is the one setting in a superweapon's section that does not pass its current value through. Exactly `0` is indistinguishable from writing nothing, and both leave the delay standing at whatever the last file to set it left there — five minutes if no file ever has. An instantly recharging superweapon cannot be authored through this key.
:::

### Manual control

[`ManualControl=yes`](/keys/manualcontrol/) leaves the timer stopped when the weapon is granted and stops it again after every discharge, so the weapon does not count down on its own and something else has to start it. The engine has exactly one such starter, and it is tied to the chem missile: on each of the house's turns, a `Type=ChemMissile` weapon that is present and not already charged restarts its charge when its house's [weed pool](/systems/veins/#the-weed-pool) is not empty and holds exactly [`WeedCapacity`](/keys/weedcapacity/) units, and that pool is then spent.

## Charge-draining weapons

[`UseChargeDrain=yes`](/keys/usechargedrain/) replaces the ordinary ready-or-charging pair with three states: charging, ready, and discharged. Firing a ready weapon moves it to the discharged state and rescales the timer by [`ChargeToDrainRatio`](/keys/chargetodrainratio/), so the clock now measures how long the effect lasts. Firing it again returns it to the ready state with the ratio applied in the other direction. Letting the drain run to zero returns the weapon to charging with a full delay on the clock and takes the effect down.

The two conversions are inverses of each other, and the arithmetic is short enough to follow. The charge a weapon has built up is `RechargeTime` less whatever is still on the clock, so firing puts that difference times the ratio on the clock as drain; firing again puts `RechargeTime` less the remaining drain divided by the ratio back on it as charge. An effect switched off the instant it came on therefore runs the second conversion on the whole drain and reads `RechargeTime - (RechargeTime * ratio) / ratio`, which is zero: the weapon is ready again with nothing left to charge. An effect switched off with half its drain still standing leaves half of `RechargeTime` on the clock, which is a half-charged weapon, and any other share comes back in the same proportion.

The flag, rather than [`Type=`](/keys/type/#scope-superweapontype), is also what shapes the cameo: a charge-draining weapon always reports itself as charging, so it always carries a clock and never shows the plain ready face, and it can be fired from any state but charging or suspended.

The whole three-state cycle runs only for a house under human control. Firing the weapon in any other house toggles its effect on or off and touches neither the state nor the clock.

### The firestorm defense

`Type=Firestorm` is the one behavior wired to this machinery. Firing the weapon raises every [`FirestormWall=yes`](/keys/firestormwall/) structure the house owns and firing it again lowers them; damage aimed at a raised section is converted through [`DamageToFirestormDamageCoefficient`](/keys/damagetofirestormdamagecoefficient/) into charge taken off this weapon's clock instead of health taken off the section; and losing the last working [`GDIFirestormGenerator`](/keys/gdifirestormgenerator/) structure while the wall is up discharges the weapon outright. [The firestorm wall](/systems/laser-fences/#the-firestorm-wall) covers the wall itself — what a raised section does to what it touches, through [`FirestormWarhead`](/keys/firestormwarhead/) and the animations beside it, and how the two halves of the cycle are measured.

## Firing it

### The sidebar cameo

A superweapon's artwork is the shape file named by [`SidebarImage=`](/keys/sidebarimage/), falling back to `XXICON.SHP` when that file cannot be found. There is no numeric countdown anywhere on the cameo; the charge readout is a clock and a short caption. [The sidebar](/systems/sidebar/) covers where the cameo sits, how it is announced, how it is captioned, and when it leaves.

The table gives the caption each state carries. The rows to read together are the two charge-draining ones: such a weapon is captioned while it charges where an ordinary weapon is captioned with nothing, and it reaches a fourth state that no ordinary weapon has.

| Weapon state | Caption |
| --- | --- |
| Suspended | "On Hold" |
| Charging | none, or "Charging..." for a charge-draining weapon |
| Ready | "Ready", or "Release" for `Type=HunterSeeker` |
| Discharged, charge-draining | "Activated" |

The clock is drawn while the weapon is not fully charged, and because a charge-draining weapon is always treated as charging, its cameo always carries one. Two clock shapes are used: the charge-up art while the weapon cannot be fired and the discharge art once it can.

Clicking a cameo that cannot be fired speaks [`SuspendVoice=`](/keys/suspendvoice/) when the timer is stopped and [`ImpatientVoice=`](/keys/impatientvoice/) otherwise. A `ManualControl=yes` weapon sits with its timer stopped between firings, so it answers with the suspend voice rather than the impatient one.

### Aiming and the click

Clicking a cameo that can be fired takes one of two paths, decided by [`Action=`](/keys/action/#scope-superweapontype). `Action=None` fires the weapon immediately at cell 0,0 with no targeting step at all; that is how the firestorm and the hunter seeker are fired, neither of which has a mouse action of its own. Any other value arms targeting mode, deselects everything, and speaks the select-target announcement.

While targeting mode is armed, the cursor over the map reports the weapon's `Action=` in place of the ordinary one, and releasing the left button fires the weapon at the cell under the pointer. A right click on the cameo or on the map cancels targeting, and band selection is suppressed while it is armed. The minimap does not accept superweapon actions, so a shot cannot be aimed there.

:::caution[A misspelled `Action=` becomes `None`]
An `Action=` value the engine does not recognize is not rejected; it reads as `None`. The weapon then takes the immediate-fire path: clicking the charged cameo discharges it on the spot at cell 0,0, with no targeting step and no opportunity to choose where the effect lands.
:::

### The computer's use

A computer house fires its superweapons from its periodic decision pass, which re-arms itself with 7 to 7.5 seconds each time it runs. Outside a campaign that pass always reaches the superweapon step. In a campaign it is reached only when the house's [`IQ=`](/keys/iq/) is at least [`SuperWeapons`](/keys/superweapons/) in `[IQ]`, and a campaign house's intelligence is whatever its own scenario section sets, starting at 0.

Every weapon that has finished charging is then handed to the handler for its `Type=`. There is no handler for `EMPulse` and none for `Firestorm`, so a computer house never fires either from this pass; its wall is raised only by the [Activate Firestorm Defense](/mapping/actions/taction-activate-firestorm/) trigger action.

Every handler refuses to fire while the house has no [declared enemy](/systems/base-attacked/#picking-a-first-enemy). The computer's own enemy pick is skipped in a campaign, so a campaign house has no declared enemy — and its superweapons stay charged and unused — until damage or a trigger raises its anger against someone.

- **Multi missile and chem missile** take the enemy structure whose cell carries the highest figure on the firing house's own [threat map](/systems/base-attacked/#the-threat-map). A building at full translucency — one whose cloak fade has run all the way to the last step, where the building counts as cloaked — is rated at a random 0 to 100 instead of by the threat map. Nothing on this loop tests limbo, strength or alliance.
- **Hunter seeker** is fired with no target at all.
- **Drop pods** pick a random point in one of the four compass quadrants of the firing house's *own* base, between one and two base radii out from its center, and then the closest cell to that point that infantry could walk into. The computer's pods land around its own base, not the enemy's.
- **Ion cannon** rates every object the enemy house owns and strikes one of the best rated.

The ion cannon's rating is the only one of the four with settings behind it, and the rest of this section is about that rating.

Only an enemy object on the ground layer, active and out of [limbo](/glossary/#limbo), is a candidate at all — except in difficulty slot 0, where an object still inside a factory that is running and not suspended also qualifies. Every candidate opens at a rating of 1 and a structure at 3, and those two opening figures are what the table below replaces. The table is consulted only for a candidate whose current strength is at or below [`IonCannonDamage`](/keys/ioncannondamage/); anything above that keeps its opening figure whatever it is. The comparison is against the raw figure alone, not a prediction that the blast will kill the object.

What a rating decides is rank, not worth. The routine remembers the highest rating it has seen, collects every candidate that ties with it, and fires at one of those at random. A premium of `4` and a premium of `40` therefore pick out the same object as long as nothing outrates it, and because a premium replaces the opening figure rather than adding to it, a premium below that figure demotes its object: an [`AIIonCannonHarvesterValue`](/keys/aiioncannonharvestervalue/) entry of `1` leaves a nearly destroyed harvester rated below an untouched structure standing beside it.

The rows are tested top to bottom and the first match wins, so a base defense that also produces vehicles is rated as a war factory and never reaches the base defense row. The right-hand column is what the rating column alone hides: three rows are numbers fixed in the engine, and no rules file can move them; every other row names a per-difficulty list read at the position of the *firing* house's own [difficulty slot](/systems/difficulty/#from-the-setting-to-a-slot), never the target's. None of those lists carries a built-in value, so each needs one entry per difficulty written for it.

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
| Vehicle whose [`DeploysInto`](/keys/deploysinto/) is the first [`BuildConst`](/keys/buildconst/) type | [`AIIonCannonMCVValue`](/keys/aiioncannonmcvvalue/) | Per-difficulty list |
| Vehicle with [`Passengers`](/keys/passengers/) above zero | [`AIIonCannonAPCValue`](/keys/aiioncannonapcvalue/) | Per-difficulty list |
| Any other vehicle | `2` | Fixed in the engine |

Nothing else the enemy owns matches a row, so an aircraft standing on the ground is a candidate rated 1 however badly damaged it is.

One override then runs on top of whatever the table produced. A cloaked object, and a building at full translucency, each take a rating drawn at random from zero up to ten above the best rating collected so far, so such an object's rating depends on where it falls in the scan and can outrate everything found before it. This is not the same rule as the flat 0 to 100 the missile handlers draw for a fully translucent building. The two are separate scales measuring different things — threat-map figures there, these premiums here — and each random range is sized for its own scale, so neither figure can be read across from one handler to the other.

## What each behavior delivers

`Type=DropPod` calls the [drop-pod delivery](/systems/drop-pods/#drop-pods-superweapon) on the chosen cell, and `Type=Firestorm` toggles [the firestorm defense](#the-firestorm-defense). The rest are set out here.

### Ion cannon

The blast lands at the target cell, at the height of that cell's terrain. Over water it draws the last entry of [`SplashList`](/keys/splashlist/); over anything else it draws [`IonBlast`](/keys/ionblast/), and the [`IonBeam`](/keys/ionbeam/) animation is created in both cases. It then applies `IonCannonDamage` through [`IonCannonWarhead`](/keys/ioncannonwarhead/) with no source, so the warhead's [`Verses`](/keys/verses/) table, its spread falloff and [`Immune=yes`](/keys/immune/) all apply and no kill is credited, and lights the scene when that warhead is declared bright. A cell that lies under a bridge is detonated twice: once at bridge height and once at ground level. A shockwave then rolls outward over the following frames.

### Multi missile and chem missile

An ordinary weapon of either behavior searches the declared BuildingTypes for the first [`NukeSilo=yes`](/keys/nukesilo/) type that names this weapon in `SuperWeapon=` or `SuperWeapon2=`, then looks for one of the house's structures of that type. That silo takes the missile mission, and the target is stored on the house as a single value, so a second launch overwrites a destination the silo has not yet read. A missile already in flight keeps the cell it was launched against.

The silo opens its door, holds it, and launches a projectile drawn from the recorded weapon's `WeaponType=`: its projectile, warhead, maximum speed and projectile range, carrying a hard-coded strength of 200, released 160 leptons — five eighths of a cell — north of the structure's center and pointed straight up. The launch-detected announcement plays whenever the launching house is not player-controlled. The door then closes and the structure returns to guard.

A one-time missile has no silo at all. It is created from the map edge closest to the target, out of the hard-coded weapon `MultiLauncher` or `ChemLauncher` according to the behavior, at a hard-coded range of `100000`, which is longer than any map is wide.

### Hunter seeker

The house's structures are scanned against [`HSBuilding`](/keys/hsbuilding/) and the last match is kept, not the first. The drone appears at the closest cell to that structure that infantry could walk into — the drone is a vehicle, but the search that places it asks for ground crossable on foot — and only if that cell lies inside the playable area, the region a scenario declares with `[Map] LocalSize=`. Otherwise nothing is created and the charge is spent. [`ActsLike=`](/keys/actslike/) decides which type is used: a house acting as GDI gets [`GDIHunterSeeker`](/keys/gdihunterseeker/) and every other house [`NodHunterSeeker`](/keys/nodhunterseeker/). A drone that cannot be placed is deleted; one that can acquires its own target and attacks.

### EM pulse

The shot is handed to the [EM pulse cannon](/systems/emp-pulse/#em-pulse-cannon-superweapon) the house owns nearest the target, and [the EM pulse cannon](/systems/emp-pulse/#em-pulse-cannon-superweapon) covers everything the pulse then does.

:::caution[A one-time EM pulse can never discharge]
This is the one behavior with a second guard on it: the branch runs only for a charged weapon that is either not a one-time weapon or no longer present. A weapon granted by the [Add 1-time special weapon](/mapping/actions/taction-1-special/) trigger action or by a missile crate is still both one-time and present when the branch is reached, so nothing is launched, and the discharge around it still spends the charge and takes the weapon away. The same section granted by [Add repeating special weapon](/mapping/actions/taction-full-special/) is not a one-time weapon and reaches the launch normally.
:::

## Scripting

Two trigger actions grant a weapon, and [becoming available](#becoming-available) covers what each one does to it. Two more toggle the firestorm defense: [Activate Firestorm Defense](/mapping/actions/taction-activate-firestorm/) and [Deactivate Firestorm Defense](/mapping/actions/taction-deactivate-firestorm/) each fire the house's first `Type=Firestorm` weapon at cell 0,0, and each does nothing when the wall is already in the state it asks for.

The three strike actions — [Ion-cannon strike](/mapping/actions/taction-ion-cannon/), [Nuke strike](/mapping/actions/taction-multi-missile/) and [Chem-missile strike](/mapping/actions/taction-chem-missile/) — do not touch this system at all. They create the effect directly at the waypoint, so they need no weapon, no charge, no silo and no house that owns one.

The [Preferred target](/mapping/actions/taction-preferred-target/) action sets a value no superweapon routine consults; a computer house's aim is entirely the [per-behavior choice](#the-computers-use) above.

## Parsed settings without effect

[`NukeProjectile`](/keys/nukeprojectile/) and [`NukeDown`](/keys/nukedown/) in `[SpecialWeapons]` are read into the rules and never consulted; a silo takes its projectile from the firing weapon's `WeaponType=` instead. [`EMPulseWarhead`](/keys/empulsewarhead/) and [`EMPulseProjectile`](/keys/empulseprojectile/) in the same section are inert too, and are covered by [the EM pulse cannon](/systems/emp-pulse/#em-pulse-cannon-superweapon) they appear to describe.
