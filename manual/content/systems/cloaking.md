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

A hidden object is out of sight for other houses, and they cannot target it. An object hides by itself when it is cloakable, or because a cloaking field of its house covers the cell it stands on.

Other houses get it back in two different ways. Detectors, and events such as firing or taking damage, end the cloak, so every house sees the object again. Sensor coverage leaves the cloak in place: the sensing house may see and shoot the object, and it stays hidden from everyone else.

## Composing the settings

Each line belongs in the section of the type that sets it. The three sections below ship with the game: the Nod stealth generator, the deployed sensor array, and the Nod stealth tank. A new type takes the same lines in its section, registered the way [rules registration](/formats/rules-registries/) describes.

```ini title="rules.ini"
[NASTLH]
CloakGenerator=yes     ; the structure raises a cloaking field
CloakRadiusInCells=12  ; how far that field reaches

[GADPSA]
SensorArray=yes        ; the deployed structure senses cells for its house

[STNK]
Cloakable=yes          ; the vehicle can hide by itself
CloakingSpeed=5        ; how many frames it spends on each fade stage
```

## Hiding an object

### The four states

An object is fully visible, fading out, hidden, or fading back in. Only the hidden state changes what other houses may do with it.

A vehicle, infantryman or aircraft fades in steps. [`CloakingSpeed`](/keys/cloakingspeed/) sets how many frames it spends on each step. [`CloakingStages`](/keys/cloakingstages/) sets how many steps make a complete fade, and the object's appearance depends on the fraction of them it has passed. The rest of this page uses the names in the table.

| Steps passed, as a fraction of `CloakingStages` | How the object is drawn |
| --- | --- |
| Below a quarter | Indistinct |
| A quarter up to but not including half | Darkened |
| Half up to but not including three quarters | Shadowy |
| Three quarters up to but not including the whole | A ripple |
| The whole | Not drawn at all |

The two fades differ in length:

- **Fading out** ends early. The object becomes hidden as soon as it reaches the shadowy band, at the first step at or above half of `CloakingStages`. It never shows the ripple on the way out.
- **Fading back in** starts one step below the whole, so it opens with the ripple, and runs all the way down to fully visible.

With the stock `CloakingStages=9` and the stealth tank's `CloakingSpeed=5` from the example above, the tank disappears after about 25 frames and takes about 40 to reappear. [`CloakSound`](/keys/cloaksound/) plays at the object's position as each fade begins.

A structure ignores both settings. It steps through fifteen fixed levels of translucency, one level per frame in either direction, and is hidden at the fifteenth.

### Starting a cloak

A fully visible vehicle, infantryman or aircraft tries to start a cloak on each frame while all of these hold:

- **Any of:**
  - **All of:** the object is cloakable, it is not immobilized, and it is not moving if its type is [`CloakStop=yes`](/keys/cloakstop/);
  - it holds the `CLOAK` [veteran ability](/systems/veterancy/#abilities);
- it is not in radio contact with a [`WeaponsFactory=yes`](/keys/weaponsfactory/) structure.

An object is cloakable through its type's [`Cloakable=yes`](/keys/cloakable/), which an aircraft never receives, or through a cloak crate.

The ability replaces the whole first group. An object that holds it tries to cloak while immobilized, while moving under `CloakStop=yes`, and without `Cloakable=yes`. The radio test has no such exemption, so a vehicle cannot hide until it has broken contact with the factory that built it.

Any of the following refuses the attempt:

- the object is already hidden, or is part way through either fade;
- its weapon is still reloading;
- its target is within range of its primary weapon;
- the [`CloakDelay`](/keys/cloakdelay/) countdown is still running;
- for a structure, a [`Sensors=yes`](/keys/sensors/) object stands within one cell of its footprint, and that object's owner does not consider the structure allied.

If nothing refuses, an object whose health is above [`ConditionRed`](/keys/conditionred/) starts hiding at once. At or below `ConditionRed`, each frame's attempt succeeds with a 4% chance, so the cloak starts at an unpredictable moment.

A structure has no entry conditions and no health roll. It starts hiding on any frame the refusal list allows, provided it is cloakable, holds the `CLOAK` ability, or has its center cell covered by its house's cloaking field.

A cloaking field also hides vehicles, infantry and aircraft by a second route. Each frame, an object standing in a cell its house's field covers is asked to hide, and only the refusal list applies. The entry conditions above and the health roll do not. That is how an object that cannot cloak by itself disappears inside a field, and why a `CloakStop=yes` object inside one hides without stopping.

When a computer-owned vehicle finishes hiding, it scatters, so it does not stay where it was last seen. A human-owned vehicle holds its ground.

Attackers lose the object as a target when it starts to hide, and any that picked it up again lose it when it finishes. Two kinds of attacker keep it: one that belongs to the same house, and one whose house senses the cell the object stands on.

### Losing a cloak

Each event below ends the cloak and starts the full fade back into view:

- **Firing:** an object that is not fully visible cannot fire. When it tries to, it uncloaks and does not fire. An aircraft is the exception: it may fire during either fade, and uncloaks only when it tries to fire while fully hidden.
- **Being hit:** any hit that does not destroy the object, including a heal and a hit its armor reduces to nothing.
- **Crushing:** a vehicle that crushes anything.
- **Planting a demolition charge:** an infantryman that plants one on a structure.
- **Carrying a captured flag:** a vehicle holding one is uncloaked on every frame, so it can never stay hidden.
- **Walking past a detector:** a hidden vehicle, infantryman or aircraft reaches the center of a cell, and one of the eight neighboring cells inside the playable area holds a detector. A detector here is an object of a `Sensors=yes` type or one holding the `SENSORS` ability, whose owner does not consider the hidden object allied. The test runs only when the hidden object arrives in a cell. A stationary hidden object is never caught by it, however long a detector stands beside it.
- **Blocking the way:** a vehicle or infantryman can path through a cell holding a non-allied hidden object. The step costs 1000 where an ordinary step costs 1 (see [path cost](/glossary/#path-cost)). As it tries to enter the cell, it uncloaks everything standing there, including objects of the same house.
- **A jumpjet nearby:** a moving jumpjet uncloaks everything near it, whoever owns it. [Jumpjet overflight](#jumpjet-overflight) gives the area.
- **Losing cover:** an object hidden only because a field covered its cell, as soon as that cover is lifted.
- **Being immobilized:** an [EM pulse](/systems/emp-pulse/) and anything else that stuns. An object that holds the `CLOAK` ability, or stands in its house's cloaking field, keeps its cloak.
- **Critical damage while fading out:** while the fade out is in its darkened band and health is at or below `ConditionRed`, a 10% chance on each frame abandons the cloak. The chance is taken on every frame of that band, so a slow fade is abandoned more often. With the stock nine stages the band lasts about twice `CloakingSpeed` frames, so at the stealth tank's `CloakingSpeed=5` about two critically damaged cloaks in three are abandoned. This is the only uncloak that plays no `CloakSound`.
- **A detector beside a structure:** a hidden structure has a `Sensors=yes` object within one cell of its footprint, and that object's owner does not consider the structure allied.

Sensor coverage is not on the list. Marking a cell as sensed never uncloaks anything standing on it.

## Cloaking fields

### Growing and collapsing

A [`CloakGenerator=yes`](/keys/cloakgenerator/) structure covers the cells around it for its house, out to [`CloakRadiusInCells`](/keys/cloakradiusincells/). The field grows by one ring of cells per frame. When a cell becomes covered for the house, the vehicles, infantry and aircraft on it try to cloak.

A collapse runs in reverse, giving up one ring per frame. When a cell stops being covered for the house, the vehicles, infantry and aircraft on it check whether they may stay hidden.

When the growth covers the center cell of a structure of the generator's house, the generator included, it pauses for one frame. A collapse pauses the same way when it uncovers such a cell. Each such structure therefore adds one frame to a growth or a collapse.

Growth starts when the structure becomes [operational](/systems/power/#defenses), and a collapse starts when it stops being operational. [What low power costs](/systems/power/#fields-fences-and-lights) covers the exemption for a [`Powered=no`](/keys/powered/) generator.

Destroying, selling or capturing the generator removes the whole field in a single frame. The same happens whenever the structure is deleted from the game. A captured generator raises a new field for its new owner as soon as it is operational.

Fields of one house may overlap. A cell stays covered while any of the house's generators covers it, so a field that collapses or is removed leaves the cells its neighbors cover in place.

On the frame a field finishes growing, every operational [`SensorArray=yes`](/keys/sensorarray/) structure, whatever its house, marks its cells if it has not marked them yet.

### What a field covers

Cover is marked per house, and a generator marks only its owner's. A vehicle, infantryman or aircraft on a covered cell hides only when the cell is covered for its house. A structure of the generator's house hides when its center cell is covered, even without `Cloakable=yes`. It drops the cloak again when the field retreats past that cell.

When cover is lifted, an object that cannot cloak by itself loses its cloak. A cloakable object keeps it, so a `Cloakable=yes` vehicle that hides inside a field stays hidden after it drives out.

:::caution[A field does not cover an ally]
Only the owning house's mark is set, and each object tests the mark for its house alone. Allied vehicles, infantry and structures inside a friendly generator's field stay in plain sight.
:::

:::caution[A large field is cut off at a square edge]
Every field is drawn on a square grid centered on its generator, and cells outside that grid are never covered. The grid's side is the largest `CloakRadiusInCells` of any structure type plus sixteen cells, and a structure type that omits the key counts as 20. A field reaches at most half the grid's width from its generator, rounded down, and one cell less on two sides when the width is even. A radius beyond that is cut off flat. For example, if no type sets a radius above 20, the grid is 36 cells across, and a 20-cell field stops 18 cells out on two sides and 17 on the other two.
:::

## Detection

### Detector objects

[`Sensors=yes`](/keys/sensors/) makes an object force nearby hidden objects out of their cloak. It uses the two tests listed under [losing a cloak](#losing-a-cloak): the one for vehicles, infantry and aircraft and the one for structures. Both apply only when the detector's owner does not consider the hidden object's house allied. The hidden object's owner may consider the detector allied, and detection still happens.

Neither test marks a cell as sensed. A detector lets its house see or target nothing that is still hidden; it only ends cloaks in the cells around it. Every InfantryType sets the flag unless its section sets `Sensors=no`.

### Sensor arrays

A [`SensorArray=yes`](/keys/sensorarray/) structure marks as sensed every cell whose distance from it is less than [`CloakRadiusInCells`](/keys/cloakradiusincells/). This is the same key that sizes a cloak generator's field, but an array's coverage is not cut off by the square grid a field is drawn on.

The array marks its cells when it first opens, in a single frame, and only if it is operational at that moment. An array that is not operational when it opens marks nothing until a cloaking field next finishes growing while the array is operational.

Coverage is lifted when the array is taken off the map. Capturing an array moves its coverage: the old owner stops sensing the array's cells, and the new owner senses them at once if the array is operational. A power shortfall never lifts coverage.

Arrays of one house may overlap. A cell stays sensed while any of the house's arrays covers it, so taking one array away leaves the cells the others cover in place.

### What sensing changes

A house that senses the cell an object stands on treats that object as though it were not hidden:

- its threat scans and its weapons accept the object as a target;
- the object is drawn shadowy instead of not at all;
- an attacker keeps it as a target when it hides, and a vehicle, infantryman or aircraft keeps it as a destination;
- the local player can click on it, read its tooltip, and get an action cursor over it;
- it is plotted on the radar.

When the local player senses the cell of an object traveling below ground, the object's condition indicator is drawn even while it is not selected. A tunneling vehicle therefore shows a health bar on the map.

None of this changes the object's cloak. It is still hidden from every other house, and being sensed never makes it reappear.

### Jumpjet overflight

On every frame a jumpjet is moving, it uncloaks every object in a square centered on the cell it has just moved to. The square reaches [`CloakDetectionRadius`](/keys/cloakdetectionradius/) cells out in each direction and covers both ground and bridge decks. The setting is the square's half-width, not a true radius: `2` sweeps a five-by-five block of cells. At the engine default of `0`, the sweep covers only the cell the jumpjet is over, so raise the setting before relying on jumpjets as detectors.

The sweep tests no ownership. A jumpjet reveals its house's objects and its allies' exactly as it reveals an enemy's, so an air patrol over a base uncloaks everything hidden underneath it.

## Targeting a hidden object

Against a house that neither owns the object nor senses its cell:

- threat scans reject it, so nothing acquires it automatically;
- weapons refuse to fire at it, with one exception described below;
- an aircraft drops it as a target on its next frame, unless the aircraft's house considers the object allied;
- the cursor offers no action against it, and clicking selects nothing.

The weapon test asks two questions: whether the shot would do damage, and whether the target's owner considers the attacker's house allied. Only a harmless shot at a target whose owner counts the attacker as an ally goes through.

| | Target's owner considers the attacker allied | Target's owner does not |
| --- | --- | --- |
| **The shot would do damage** | Refused | Refused |
| **The shot would do no damage** | Goes through | Refused |

Area damage still reaches a hidden object, because it does not check whether anything inside its radius is hidden. The damage also uncloaks the object it lands on.

A hidden object also does not block a vehicle's or infantryman's path. The step through its cell costs more (see [losing a cloak](#losing-a-cloak)), and a route that takes that step reveals what is standing there.

A human player's cloaking vehicles, infantry and aircraft do not pick targets for themselves while on the Guard mission. This applies to an object that may cloak by itself, through `Cloakable=yes` or the `CLOAK` ability. It holds fire at everything that walks past, so it does not give itself away. Computer-owned objects have no such restriction.

## What the player sees

### On the tactical map

A vehicle, infantryman or aircraft part way through a fade is drawn in the bands listed under [the four states](#the-four-states). The player who owns it never sees it fainter than shadowy: where other players see the ripple or nothing, the owner sees the shadowy band.

A structure uses its fifteen levels of translucency:

| Level | How the structure is drawn |
| --- | --- |
| 1 to 5 | Indistinct |
| 6 to 10 | Darkened |
| 11 to 15 | Out of sight, except for the players listed below, who see it shadowy |

A structure is out of sight from level 11 but hidden only at level 15. During those four levels, an enemy's threat scans can still pick it as a target, but that enemy's weapons refuse to fire at it.

A hidden object is drawn shadowy, not left out, for these players:

- the player who owns it;
- a player whose house senses its cell;
- a player who has been given the whole map;
- outside a campaign game, a player who is mutually allied with its owner.

Selection follows the sensing rule. When an object of a house the player does not control starts to hide, it leaves the player's selection unless the player senses its cell.

[`Invisible=yes`](/keys/invisible/) is separate from cloaking. It hides the object from every player except its owner, whatever the cloak state. [`InvisibleInGame=yes`](/keys/invisibleingame/) also hides a structure from its owner.

### On the radar

The local player's radar asks these questions in order and uses the first answer that applies:

1. Is it `Invisible=yes`? Then it is never plotted, for any house.
2. Is it [`RadarVisible=yes`](/keys/radarvisible/)? Then it is always plotted.
3. Does it belong to a house under the local player's control? Then it is plotted once the player has discovered it.
4. Is it in plain view? It is plotted when all of these hold: it is not fogged, not hidden, not under shroud, not more than 20 leptons below ground level, and it does not hold the `RADAR_INVISIBLE` [veteran ability](/systems/veterancy/#abilities).
5. Otherwise, it is plotted only while the player senses its cell.

When an object reaches the last step and the player does not consider its house allied, it counts as a detection, unless any of these is true of it:

- it holds the `RADAR_INVISIBLE` ability;
- it is fogged or under shroud;
- it is sinking.

A player who has been given the whole map senses every cell, and never counts an object as a detection. The [observers and coach mode](/systems/observers/) page covers who that is.

A detection raises a radar event at the object's cell. EVA speaks the hard-coded `00-I172` line for a hidden object, or `00-I174` for one more than 20 leptons below ground. An object traveling through a tunnel raises no announcement. A detection near an enemy-sensed event already on the radar is dropped, not repeated.

A structure goes through the same questions, with two differences. The hidden test in step 4 also fails at its fifteenth translucency level, and the height and ability tests do not apply. A detected structure always gets the hidden line, never the underground one.

### Radius rings

A selected structure of a [`HasRadialIndicator=yes`](/keys/hasradialindicator/) type draws an ellipse showing how far its field reaches. The ellipse appears while the structure is switched on and its house is player-controlled. Only a cloak generator and a sensor array draw one; any other type with the key draws nothing. The ellipse is sized from `CloakRadiusInCells`, drawn in [`RadialColor`](/keys/radialcolor/), and has four spokes that sweep around it. Its proportions and sweep speed are fixed in the engine.

## Parsed settings without effect

[`RadarInvisible`](/keys/radarinvisible/) has no effect. To keep an object off the radar while leaving it on the map, grant the `RADAR_INVISIBLE` veteran ability through [`VeteranAbilities`](/keys/veteranabilities/) or [`EliteAbilities`](/keys/eliteabilities/).

[`IsMobileStealth=yes`](/keys/ismobilestealth/) does not make anything cloak. It marks a structure as the deployed form of a vehicle, like the sensor array and the other deployers, and has no other effect.
