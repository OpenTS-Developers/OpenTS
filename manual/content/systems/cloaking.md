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

Three separate pieces of state decide what is hidden and who can see it. Each object carries its own cloak state. Each cell carries the set of houses whose cloaking field covers it, and the set of houses whose sensor coverage reaches it. A field never hides anything directly — it marks cells, and the objects standing on a marked cell hide themselves. Sensor coverage never reveals anything directly either — it leaves the cloak alone and changes what the sensing house may see, target and shoot.

## Hiding an object

### The four states

An object is fully visible, fading out, hidden, or fading back in. Only the hidden state changes what other houses may do with it, and every fade runs through the same stages in between.

[`CloakingSpeed`](/keys/cloakingspeed/) is how many frames the object spends on each stage, and [`CloakingStages`](/keys/cloakingstages/) is the figure the current stage is divided by. Fixed fractions of that figure set the appearance. The table gives the five bands in the order a fade out passes through them; a fade back in runs the same five in reverse, and the rest of this page uses these names.

| Stage as a fraction of `CloakingStages` | How the object is drawn |
| --- | --- |
| Below a quarter | Indistinct |
| A quarter up to but not including half | Darkened |
| Half up to but not including three quarters | Shadowy |
| Three quarters up to but not including the whole | A ripple |
| The whole | Not drawn at all |

The two fades are not the same length. Fading out stops early — the object counts as hidden the moment the fade reaches the shadowy band, at half of `CloakingStages`. Fading back in starts one stage below the top and runs the whole way down to zero. [`CloakSound`](/keys/cloaksound/) plays at the object's position as each fade begins.

A structure ignores that machinery entirely. It steps through fifteen fixed levels of translucency, one level per frame in each direction, and its cloak is complete at the fifteenth.

### Starting a cloak

A fully visible object tries to start a cloak on each frame while all of these hold:

- **Any of:**
  - **All of:** [`Cloakable=yes`](/keys/cloakable/) applies to it, it is not immobilized, and — for a vehicle, infantryman or aircraft of a [`CloakStop=yes`](/keys/cloakstop/) type — it is not moving;
  - it carries the `CLOAK` [veteran ability](/systems/veterancy/#abilities);
- it is not in radio contact with a [`WeaponsFactory=yes`](/keys/weaponsfactory/) structure.

The ability stands in for the whole of the first group rather than for one term of it, so an object carrying it attempts a cloak while immobilized, while moving under `CloakStop=yes`, and with no `Cloakable=yes` at all. The radio test has no such exemption, so a vehicle cannot hide until it has broken contact with the factory that built it.

The attempt is then refused by any of the following:

- being hidden already, or standing part way through either fade — a structure is exempt from the fade test;
- a rearming delay still running;
- holding a target inside the range of its primary weapon;
- the [`CloakDelay`](/keys/cloakdelay/) countdown still running;
- for a structure, an object whose owner does not consider the structure allied and whose type is [`Sensors=yes`](/keys/sensors/) standing within one cell of its footprint.

An object that clears all of that begins hiding immediately while its health is above [`ConditionRed`](/keys/conditionred/). At or below that fraction each frame's attempt instead succeeds with a 4% chance, so hiding starts at an unpredictable moment.

Cells covered by a cloaking field ask their occupants to hide through a second route, which asks for none of the entry conditions above — not the flag, not the ability, not the movement or immobilization tests, not the radio contact test, and not the health roll — and applies the refusal list alone. That is how an object with no cloak of its own disappears inside a field, and why a `CloakStop=yes` object hides inside one without stopping.

When a computer-owned vehicle finishes hiding it scatters, so it does not stay where it was last seen. A human-owned one holds its ground.

### Losing a cloak

Every event below drops the cloak outright and starts the fade back into view. None of them produces a brief flicker.

- **Firing.** A shot is refused while the object is in any state but fully visible, and the object answers that refusal by uncloaking rather than by holding fire. An aircraft is the exception: it may fire through both fades and is refused only once fully hidden.
- **Taking damage.** Any damage that does not destroy the object.
- **Crushing.** A vehicle that crushes anything.
- **Planting a demolition charge.** An infantryman that plants one on a structure.
- **Carrying a captured flag.** A vehicle holding one is uncloaked on every frame, so it can never stay hidden.
- **Walking past a detector.** A hidden vehicle, infantryman or aircraft reaches the center of a cell while one of the eight neighboring cells inside the playable area holds an object of a non-allied house whose type is `Sensors=yes` or which carries the `SENSORS` ability. The test runs on arrival alone, so an object that has stopped moving is never caught by it however long a detector stands beside it.
- **Blocking the way.** A vehicle or infantryman treats a cell holding a non-allied hidden object as passable rather than as impassable, at a [path cost](/glossary/#path-cost) of 1000 against the 1 an ordinary step costs, and uncloaks everything standing in that cell as it tries to step in — its own house's objects included, because that step tests no ownership.
- **A jumpjet overhead.** Every frame a jumpjet moves it uncloaks every object in the square reaching [`CloakDetectionRadius`](/keys/cloakdetectionradius/) cells out from the position it has just arrived at, on the ground and on a bridge deck alike, with no test of ownership. The setting is the square's half-width rather than a true radius, so `2` sweeps a five-by-five block of cells.
- **Losing cover.** An object hidden only because a field covered its cell, the moment that cover is lifted.
- **Being immobilized.** An [EM pulse](/systems/emp-pulse/) and anything else that stuns. The cloak ability and standing in cloaking cover both override this.
- **Critical damage part way out.** While the fade out sits in its darkened band and health is at or below `ConditionRed`, a 10% chance on each frame gives the cloak up. The fade holds that band for several frames, so a critically damaged object more often abandons a cloak than finishes one. This is the one uncloak that starts without `CloakSound`.
- **A detector beside a structure.** A hidden structure with a `Sensors=yes` object standing within one cell of its footprint, when that object's owner does not consider the structure allied.

Sensor coverage is not on the list. Marking a cell as sensed never touches the cloak state of anything standing on it.

When an object starts to hide, and again when it finishes, everything shooting at it loses the target — except an attacker that owns it, and an attacker whose house senses the cell it stands on.

## Cloaking fields

### Growing and collapsing

A [`CloakGenerator=yes`](/keys/cloakgenerator/) structure marks the cells around it as covered for its own house, out to [`CloakRadiusInCells`](/keys/cloakradiusincells/). The field is not raised in one go: the radius grows by one cell per frame, the whole disc is stamped again on each of those frames, and every cell the growth has just reached is marked and its occupants asked to hide. A collapse runs the same way in reverse, giving up one ring per frame and asking the occupants of each abandoned cell to reconsider.

Growth starts when the structure becomes [operational](/systems/power/#defenses) and a collapse starts when it stops being; [what low power costs](/systems/power/#fields-fences-and-lights) covers the exemption a [`Powered=no`](/keys/powered/) generator has.

Being destroyed, being sold and changing hands each collapse the whole field in a single pass rather than a ring at a time. The same single pass runs again as the structure is finally taken off the map, which is what covers a generator removed some other way. A captured generator raises a fresh field for its new owner as soon as it is operational.

Two refreshes hang off the ends of that process:

- The frame a field finishes growing, every operational [`SensorArray=yes`](/keys/sensorarray/) structure in the game — of any house — stamps its coverage again.
- The frame a field finishes collapsing, every other operational generator of any house within twice the collapsing structure's own radius plus four cells restarts its own growth from the center, so that cells the two fields shared are marked again. Nothing is unmarked while that happens, so a neighboring field does not blink out as it regrows.

### What a field covers

The mark is made per house, and a generator sets only its owner's. A vehicle, infantryman or aircraft standing on a covered cell hides only when the cell is covered for its own house. A structure of the same house standing inside the field hides through its own check rather than through the field's sweep, and gives the cloak up again when the field retreats past it.

An object that has no cloak of its own drops it the moment its cell stops being covered. One that could have hidden anyway keeps it, which is why a `Cloakable=yes` vehicle that picks up its cloak inside a field carries it out again.

:::caution[A field does not cover an ally]
Only the owning house's mark is set, and each object tests the mark for its own house alone. Allied vehicles and infantry parked inside a friendly generator's field stand in plain sight, and so do allied structures.
:::

:::caution[A large field is squared off at the edge of its working area]
The disc is stamped into a working area whose side is the largest `CloakRadiusInCells` in the rules plus sixteen cells, and cells outside that area are never examined. A field whose radius reaches half that width is therefore cut off flat instead of reaching the radius it names. At the engine default of 20 the working area is 36 cells across and the field reaches eighteen cells on two sides and seventeen on the other two.
:::

## Detection

### Detector objects

[`Sensors=yes`](/keys/sensors/) makes an object reveal a nearby hidden object whose house its owner does not consider allied, through the two proximity tests listed under [losing a cloak](#losing-a-cloak). The detector owner's alliance list decides this even when the hidden object's owner considers the detector allied. Neither test marks a cell, so a detector grants its house nothing at all beyond the eight cells around it. Every InfantryType carries the flag unless its section switches it off.

### Sensor arrays

A [`SensorArray=yes`](/keys/sensorarray/) structure marks every cell lying strictly nearer to it than [`CloakRadiusInCells`](/keys/cloakradiusincells/) cells — the same key that sizes a cloak generator's field — in a single pass, and only while the structure is operational.

Coverage is stamped when the structure first opens for business, and lifted only when it is taken off the map. A power shortfall never lifts it; what a shortfall costs an array is the refresh, since an array that is not operational is skipped by the two events that re-stamp coverage. Lifting an array's coverage makes every other operational array, of any house, stamp its own cells again, so overlapping coverage is not lost with it.

:::caution[Capturing an array leaves the old owner's coverage behind]
Coverage is given up only when the structure is taken off the map, and the sweep that gives it up marks off whichever house owns the structure at that moment. A captured array therefore never releases the cells it marked for its previous owner, and that house keeps seeing hidden objects inside the old radius for the rest of the match. The new owner gets nothing until something re-stamps the array — a cloak field finishing its growth, or another array being removed.
:::

### What sensing changes

A house that senses the cell an object stands on treats that object as though it were not hidden:

- its threat scans and its weapons accept the object as a target;
- the object is drawn shadowy rather than not at all;
- an attacker keeps it as a target when it disappears, and a vehicle, infantryman or aircraft keeps it as a destination;
- the local player can click on it, read its tooltip, and get an action cursor over it;
- it is plotted on the radar.

An object traveling below ground in a sensed cell also draws its selection box and condition indicator while it is not selected, so a tunneling vehicle is marked on the map by an outline even though the vehicle itself shows at most a ripple.

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

Two things get through regardless. Area damage never asks whether anything inside its radius is hidden, and damage uncloaks what it lands on. And a hidden blocker is not impassable to a vehicle or infantryman: the step through its cell is priced rather than refused, at the figure given under [losing a cloak](#losing-a-cloak), so a route that has to take it reveals what is standing there.

A human house's own cloaking objects do not acquire targets by themselves either. A vehicle, infantryman or aircraft that may cloak — because `Cloakable=yes` applies to it, or because it carries the `CLOAK` ability — returns no target from its own threat scan while it is on the Guard mission, so it holds fire on everything that walks past it rather than giving itself away. A computer house is under no such restriction.

## What the player sees

### On the tactical map

An object part way through a fade is drawn through the five bands set out under [the four states](#the-four-states). Its owner never sees it drawn fainter than shadowy: for the player who owns it, both the ripple and the disappearance are drawn shadowy instead. A structure runs its own fifteen-level version: indistinct for the first five levels, darkened to the tenth, and out of sight from the eleventh — four levels before its cloak is complete.

A hidden object is still drawn shadowy, rather than not at all, for the player that owns it, for a house that senses its cell, and — outside a campaign game — between players who are each other's allies. Selection follows the same rule: an object of another house is dropped from the player's selection as it starts to hide unless the player senses its cell.

[`Invisible=yes`](/keys/invisible/) sits outside all of this. It hides the object from every machine but its owner's whatever its cloak state, and [`InvisibleInGame=yes`](/keys/invisibleingame/) hides a structure from its owner as well.

### On the radar

The local player's radar asks these questions in order and takes the first answer:

1. `Invisible=yes` — never plotted, for any house.
2. [`RadarVisible=yes`](/keys/radarvisible/) — always plotted.
3. An object of a house under the local player's control — plotted once the player has discovered it.
4. Anything else — plotted while it is not fogged, not hidden, not more than 20 leptons below ground level, not carrying the `RADAR_INVISIBLE` [veteran ability](/systems/veterancy/#abilities), and not under shroud.
5. Otherwise — plotted only while the player senses its cell.

An object of a house the player is not allied to that reaches the last step counts as a detection rather than a sighting. A radar event is raised at its cell, and EVA speaks the hard-coded `00-I172` line for a hidden object or `00-I174` for one more than 20 leptons below ground. An object traveling through a tunnel raises no announcement, and a detection close to an enemy-sensed event already on the radar is dropped rather than repeated.

A structure is asked the same questions, with its own fade level added to the hidden test and without the height and ability tests, and always reports the hidden line rather than the underground one.

### Radius rings

A selected structure of a [`HasRadialIndicator=yes`](/keys/hasradialindicator/) type draws an ellipse showing how far its field reaches, while it is switched on and its house is player-controlled. Only a cloak generator and a sensor array draw one; every other type carrying the key draws nothing. The ellipse is sized from `CloakRadiusInCells`, drawn in [`RadialColor`](/keys/radialcolor/), and carries four spokes that sweep round it. Its proportions and sweep rates are fixed in the engine.

## Parsed settings without effect

[`RadarInvisible`](/keys/radarinvisible/) is read into every object type and never consulted anywhere. Keeping an object off the radar while leaving it on the map is done by the `RADAR_INVISIBLE` veteran ability instead, and no type-level key grants it.

[`IsMobileStealth=yes`](/keys/ismobilestealth/) names no cloaking behavior despite its spelling. It marks a structure as the deployed form of a vehicle, alongside the sensor array and the other mobile deployers, and nothing reads it for anything else.
