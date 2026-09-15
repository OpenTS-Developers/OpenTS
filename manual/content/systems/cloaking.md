---
title: Cloaking and detection
summary: "Hides objects from the houses that do not own them, and lets detectors and sensor coverage bring them back into view."
category: combat-targeting
keys:
  - Cloakable
  - CloakDelay
  - CloakDetectionRadius
  - CloakGenerator
  - CloakingSpeed
  - CloakingStages
  - CloakRadiusInCells
  - CloakSound
  - CloakStop
  - HasRadialIndicator
  - Invisible
  - InvisibleInGame
  - RadarVisible
  - RadialColor
  - SensorArray
  - Sensors
related:
  - type: system
    id: emp-pulse
  - type: system
    id: power
  - type: system
    id: veterancy
---

Three separate pieces of state decide what is hidden and who can see it. Each object holds its own cloak state. Each cell holds the set of houses whose cloaking field covers it, and the set of houses whose sensor coverage reaches it. A field never hides anything directly: it marks cells, and the objects standing on a marked cell hide themselves. Sensor coverage never reveals anything directly either. It leaves the cloak alone and changes what the sensing house may see, target and shoot.

## Composing the settings

Each line belongs in the section of the type that sets it. The three sections below ship with the game: the Nod stealth generator, the deployed sensor array, and the Nod stealth tank. A type of your own takes the same lines in its own section, registered the way [rules registration](/formats/rules-registries/) describes.

```ini title="rules.ini"
[NASTLH]
CloakGenerator=yes     ; the structure raises a cloaking field of its own
CloakRadiusInCells=12  ; how far that field reaches

[GADPSA]
SensorArray=yes        ; the deployed structure senses cells for its house

[STNK]
Cloakable=yes          ; the vehicle can hide on its own
CloakingSpeed=5        ; how many frames it spends on each fade stage
```

## Hiding an object

### The four states

An object is fully visible, fading out, hidden, or fading back in. Only the hidden state changes what other houses may do with it, and every fade runs through the same stages in between.

[`CloakingSpeed`](/keys/cloakingspeed/) is how many frames the object spends on each stage. [`CloakingStages`](/keys/cloakingstages/) is the figure the current stage is divided by, and fixed fractions of it set the appearance. The table lists the five bands in the order the stage figure rises through them, and the rest of this page uses these names.

| Stage as a fraction of `CloakingStages` | How the object is drawn |
| --- | --- |
| Below a quarter | Indistinct |
| A quarter up to but not including half | Darkened |
| Half up to but not including three quarters | Shadowy |
| Three quarters up to but not including the whole | A ripple |
| The whole | Not drawn at all |

The two fades are not the same length. Fading out stops early. The object counts as hidden the moment the fade reaches the shadowy band, at the first stage at or above half of `CloakingStages`. The fade ends there, so no ripple is drawn on the way out and the last band in the table is the hidden state itself. Fading back in is the only fade that shows the ripple: it starts one stage below the top and runs the whole way down to zero. [`CloakSound`](/keys/cloaksound/) plays at the object's position as each fade begins.

A structure ignores that machinery entirely: neither `CloakingSpeed` nor `CloakingStages` touches it. It steps through fifteen fixed levels of translucency, one level per frame in each direction, and its cloak is complete at the fifteenth.

### Starting a cloak

A fully visible object tries to start a cloak on each frame while all of these hold:

- **Any of:**
  - **All of:** [`Cloakable=yes`](/keys/cloakable/) applies to it, it is not immobilized, and, for a vehicle, infantryman or aircraft of a [`CloakStop=yes`](/keys/cloakstop/) type, it is not moving;
  - it holds the `CLOAK` [veteran ability](/systems/veterancy/#abilities);
- it is not in radio contact with a [`WeaponsFactory=yes`](/keys/weaponsfactory/) structure.

The ability stands in for the whole of the first group rather than for one term of it, so an object holding it attempts a cloak while immobilized, while moving under `CloakStop=yes`, and with no `Cloakable=yes` at all. The radio test has no such exemption, so a vehicle cannot hide until it has broken contact with the factory that built it.

The attempt is then refused by any of the following:

- being hidden already, or standing part way through either fade (a structure is exempt from the fade test);
- a weapon rearm still running;
- holding a target inside the range of its primary weapon;
- the [`CloakDelay`](/keys/cloakdelay/) countdown still running;
- for a structure, an object whose owner does not consider the structure allied and whose type is [`Sensors=yes`](/keys/sensors/) standing within one cell of its footprint.

An object that clears all of that begins hiding immediately while its health is above [`ConditionRed`](/keys/conditionred/). At or below that fraction each frame's attempt instead succeeds with a 4% chance, so hiding starts at an unpredictable moment.

Cells covered by a cloaking field ask their occupants to hide through a second route. That route applies the refusal list alone and asks for none of the entry conditions above: not the flag, not the ability, not the movement or immobilization tests, not the radio contact test, and not the health roll. It is how an object with no cloak of its own disappears inside a field, and why a `CloakStop=yes` object hides inside one without stopping.

When a computer-owned vehicle finishes hiding it scatters, so it does not stay where it was last seen. A human-owned one holds its ground.

### Losing a cloak

Every event below drops the cloak outright and starts the fade back into view. None of them produces a brief flicker.

- **Firing:** a shot is refused while the object is in any state but fully visible, and the object answers that refusal by uncloaking rather than by holding fire. An aircraft is the exception: it may fire through both fades and is refused only once fully hidden.
- **Taking damage:** any damage that does not destroy the object.
- **Crushing:** a vehicle that crushes anything.
- **Planting a demolition charge:** an infantryman that plants one on a structure.
- **Carrying a captured flag:** a vehicle holding one is uncloaked on every frame, so it can never stay hidden.
- **Walking past a detector:** a hidden vehicle, infantryman or aircraft reaches the center of a cell while one of the eight neighboring cells inside the playable area holds an object of a non-allied house. That object must be of a `Sensors=yes` type or hold the `SENSORS` ability. The test runs on arrival alone, so an object that has stopped moving is never caught by it however long a detector stands beside it.
- **Blocking the way:** a vehicle or infantryman treats a cell holding a non-allied hidden object as passable rather than as impassable, at a [path cost](/glossary/#path-cost) of 1000 against the 1 an ordinary step costs. As it tries to step in it uncloaks everything standing in that cell, its own house's objects included, because that step tests no ownership.
- **A jumpjet overhead:** every frame a jumpjet moves, it uncloaks every object in the square reaching [`CloakDetectionRadius`](/keys/cloakdetectionradius/) cells out from the position it has just arrived at. That square covers ground and bridge deck alike, and the sweep tests no ownership. The setting is the square's half-width rather than a true radius, so `2` sweeps a five-by-five block of cells.
- **Losing cover:** an object hidden only because a field covered its cell, the moment that cover is lifted.
- **Being immobilized:** an [EM pulse](/systems/emp-pulse/) and anything else that stuns. The cloak ability and standing in cloaking cover both override this.
- **Critical damage part way out:** while the fade out sits in its darkened band and health is at or below `ConditionRed`, a 10% chance on each frame gives the cloak up. The fade holds that band for several frames, so a critically damaged object more often abandons a cloak than finishes one. This is the one uncloak that starts without `CloakSound`.
- **A detector beside a structure:** a hidden structure with a `Sensors=yes` object standing within one cell of its footprint, when that object's owner does not consider the structure allied.

Sensor coverage is not on the list. Marking a cell as sensed never touches the cloak state of anything standing on it.

When an object starts to hide, and again when it finishes, everything shooting at it loses the target, except an attacker that owns it and an attacker whose house senses the cell it stands on.

## Cloaking fields

### Growing and collapsing

A [`CloakGenerator=yes`](/keys/cloakgenerator/) structure marks the cells around it as covered for its own house, out to [`CloakRadiusInCells`](/keys/cloakradiusincells/). The field is not raised in one go. The radius grows by one cell per frame, and the whole disc is stamped again on each of those frames. Every cell the growth has just reached is marked, and its occupants are asked to hide. A ring that would newly cover the center cell of a structure of the generator's own house is rolled back, so a friendly building holds the field short of its radius for as long as it stands. A collapse runs the same way in reverse, giving up one ring per frame and asking the occupants of each abandoned cell to reconsider. It is held back the same way by a friendly structure its ring would uncover.

Growth starts when the structure becomes [operational](/systems/power/#defenses) and a collapse starts when it stops being; [what low power costs](/systems/power/#fields-fences-and-lights) covers the exemption a [`Powered=no`](/keys/powered/) generator has.

Being destroyed, being sold and changing hands each collapse the whole field in a single pass rather than a ring at a time. The same single pass runs again as the structure is finally taken off the map, which is what covers a generator removed some other way. A captured generator raises a fresh field for its new owner as soon as it is operational.

Two refreshes hang off the ends of that process:

- The frame a field finishes growing, every operational [`SensorArray=yes`](/keys/sensorarray/) structure in the game (of any house) stamps its coverage again.
- The frame a field finishes collapsing, every other operational generator of any house within twice the collapsing structure's own radius plus four cells restarts its own growth from the center. That re-marks the cells the two fields shared. Nothing is unmarked while it happens, so a neighboring field does not blink out as it regrows.

### What a field covers

The mark is made per house, and a generator sets only its owner's. A vehicle, infantryman or aircraft standing on a covered cell hides only when the cell is covered for its own house. A structure of the same house standing inside the field hides too: its own check passes because the cell is covered for its house, so it needs no `Cloakable=yes` of its own. It gives the cloak up again when the field retreats past it.

An object that has no cloak of its own drops it the moment its cell stops being covered. One that could have hidden anyway keeps it, which is why a `Cloakable=yes` vehicle that picks up its cloak inside a field takes it out again.

:::caution[A field does not cover an ally]
Only the owning house's mark is set, and each object tests the mark for its own house alone. Allied vehicles and infantry parked inside a friendly generator's field stand in plain sight, and so do allied structures.
:::

:::caution[A large field is squared off at the edge of its working area]
The disc is stamped into a working area whose side is the largest `CloakRadiusInCells` in the rules plus sixteen cells, and cells outside that area are never examined. A field whose radius reaches half that width is therefore cut off flat instead of reaching the radius it names. At the engine default of 20 the working area is 36 cells across and the field reaches eighteen cells on two sides and seventeen on the other two. The two sides differ by one because the disc's center falls on a cell rather than on the corner where four meet.
:::

## Detection

### Detector objects

[`Sensors=yes`](/keys/sensors/) makes an object reveal a nearby hidden object whose house its owner does not consider allied, through the two proximity tests listed under [losing a cloak](#losing-a-cloak). The detector owner's alliance list decides this even when the hidden object's owner considers the detector allied. Neither test marks a cell, so a detector grants its house nothing at all beyond the eight cells around it. Every InfantryType sets the flag unless its section switches it off.

### Sensor arrays

A [`SensorArray=yes`](/keys/sensorarray/) structure marks every cell lying strictly nearer to it than [`CloakRadiusInCells`](/keys/cloakradiusincells/) cells, in a single pass and only while the structure is operational. That key is the same one that sizes a cloak generator's field, but the sweep has no working area to be clipped by.

Coverage is stamped when the structure first opens for business, and lifted only when it is taken off the map. A power shortfall never lifts it; what a shortfall costs an array is the refresh, since an array that is not operational is skipped by the two events that re-stamp coverage. Lifting an array's coverage makes every other operational array, of any house, stamp its own cells again, so overlapping coverage is not lost with it.

:::caution[Capturing an array leaves the old owner's coverage behind]
Coverage is given up only when the structure is taken off the map, and the sweep that gives it up marks off whichever house owns the structure at that moment. A captured array therefore never releases the cells it marked for its previous owner, and that house keeps seeing hidden objects inside the old radius for the rest of the match. The new owner gets nothing until something re-stamps the array: a cloak field finishing its growth, or another array being removed.
:::

### What sensing changes

A house that senses the cell an object stands on treats that object as though it were not hidden:

- its threat scans and its weapons accept the object as a target;
- the object is drawn shadowy rather than not at all;
- an attacker keeps it as a target when it disappears, and a vehicle, infantryman or aircraft keeps it as a destination;
- the local player can click on it, read its tooltip, and get an action cursor over it;
- it is plotted on the radar.

An object traveling below ground in a sensed cell draws its condition indicator even while it is not selected, so a tunneling vehicle shows a health bar on the map.

None of that reaches the object's own state. It is still hidden, still hidden from every other house, and nothing about being sensed makes it reappear.

### Jumpjet overflight

A flying jumpjet changes the cloak state just as a ground detector does, but it is the one detector that tests no ownership at all. It reveals a friendly and an allied object exactly as it reveals an enemy, so an air patrol over a base strips the cover from everything hidden underneath it. The engine default of `0` for [`CloakDetectionRadius`](/keys/cloakdetectionradius/) reduces the sweep to the single cell the jumpjet is over, so the setting has to be raised before a jumpjet detects anything it does not fly directly across.

## Targeting a hidden object

Against a house that neither owns the object nor senses its cell:

- threat scans reject it outright, so nothing acquires it on its own;
- a weapon is refused unless the shot does no damage and the target is an ally;
- an aircraft drops it as a target the moment its house stops sensing it;
- the cursor offers no action against it, and clicking selects nothing.

That weapon test turns on two questions, and only one of the four answers lets a shot through. The table crosses them.

| | Target is an ally | Target is not an ally |
| --- | --- | --- |
| **The shot would do damage** | Refused | Refused |
| **The shot would do no damage** | Goes through | Refused |

Two things get through regardless. Area damage never asks whether anything inside its radius is hidden, and damage uncloaks what it lands on. A hidden blocker is also not impassable to a vehicle or infantryman. The step through its cell is priced rather than refused, at the figure given under [losing a cloak](#losing-a-cloak), so a route that has to take it reveals what is standing there.

A human house's own cloaking objects do not acquire targets by themselves either. A vehicle, infantryman or aircraft that may cloak, whether from `Cloakable=yes` or from the `CLOAK` ability, returns no target from its own threat scan while it is on the Guard mission. It holds fire on everything that walks past rather than giving itself away. A computer house is under no such restriction.

## What the player sees

### On the tactical map

An object part way through a fade is drawn through the five bands set out under [the four states](#the-four-states). Its owner never sees it drawn fainter than shadowy: for the player who owns it, both the ripple and the disappearance are drawn shadowy instead. A structure runs its own fifteen-level version. It is indistinct for the first five levels, darkened to the tenth, and out of sight from the eleventh for any house that does not own or sense it. That is four levels before its own cloak is complete. Over those four levels the structure is drawn as hidden but is not in the hidden state yet. An enemy can therefore still acquire it as a target, though that enemy's weapons refuse the shot.

A hidden object is still drawn shadowy, rather than not at all, for the player that owns it, for a house that senses its cell, and for a player who has been given the whole map. Outside a campaign game it is also drawn shadowy between players who are each other's allies. Selection follows the same rule: an object of another house is dropped from the player's selection as it starts to hide unless the player senses its cell.

[`Invisible=yes`](/keys/invisible/) sits outside all of this. It hides the object from every player but its owner, whatever its cloak state, and [`InvisibleInGame=yes`](/keys/invisibleingame/) hides a structure from its owner as well.

### On the radar

The local player's radar asks these questions in order and takes the first answer:

1. `Invisible=yes` — never plotted, for any house.
2. [`RadarVisible=yes`](/keys/radarvisible/) — always plotted.
3. An object of a house under the local player's control — plotted once the player has discovered it.
4. Anything else — plotted while it is not fogged, not hidden, not more than 20 leptons below ground level, not holding the `RADAR_INVISIBLE` [veteran ability](/systems/veterancy/#abilities), and not under shroud.
5. Otherwise — plotted only while the player senses its cell.

An object of a house the player is not allied to that reaches the last step counts as a detection rather than a sighting, unless any of these is already true of it:

- it holds the `RADAR_INVISIBLE` ability;
- it is fogged or under shroud;
- it is sinking.

A player who has been given the whole map senses every cell and never counts a sighting as a detection; the [observers and coach mode](/systems/observers/) page owns who that is. A radar event is raised at its cell, and EVA speaks the hard-coded `00-I172` line for a hidden object or `00-I174` for one more than 20 leptons below ground. An object traveling through a tunnel raises no announcement, and a detection close to an enemy-sensed event already on the radar is dropped rather than repeated.

A structure is asked the same questions, with its own fade level added to the hidden test and without the height and ability tests, and always reports the hidden line rather than the underground one.

### Radius rings

A selected structure of a [`HasRadialIndicator=yes`](/keys/hasradialindicator/) type draws an ellipse showing how far its field reaches, while it is switched on and its house is player-controlled. Only a cloak generator and a sensor array draw one; every other type with the key draws nothing. The ellipse is sized from `CloakRadiusInCells`, drawn in [`RadialColor`](/keys/radialcolor/), and has four spokes that sweep round it. Its proportions and sweep rates are fixed in the engine.

## Parsed settings without effect

[`RadarInvisible`](/keys/radarinvisible/) is read into every object type and nothing reads it anywhere. Keeping an object off the radar while leaving it on the map is done by the `RADAR_INVISIBLE` veteran ability instead, which only [`VeteranAbilities`](/keys/veteranabilities/) or [`EliteAbilities`](/keys/eliteabilities/) can grant.

[`IsMobileStealth=yes`](/keys/ismobilestealth/) names no cloaking behavior despite its spelling. It marks a structure as the deployed form of a vehicle, alongside the sensor array and the other mobile deployers, and nothing reads it for anything else.
