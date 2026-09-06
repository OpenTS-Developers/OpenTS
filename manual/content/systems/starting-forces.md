---
title: Starting forces
summary: "Places each house's base unit and a budget of randomly chosen vehicles and infantry around its start position as a skirmish or multiplayer match begins."
category: maps-scenarios
keys:
  - AllowedToStartInMultiplayer
  - BaseUnit
  - Bases
  - Cost
  - InitialVeteran
  - MultiplayPassive
  - Owner
  - TechLevel
  - UnitCount
related:
  - type: key
    id: Official
    scope: scenarios-2
  - type: format
    id: spawn-ini
  - type: system
    id: crates
  - type: system
    id: observers
  - type: system
    id: veterancy
---

The pass runs once, as a skirmish or multiplayer scenario finishes loading, and never in a campaign. It visits every house in turn and skips a house whose country is [`MultiplayPassive`](/keys/multiplaypassive/) and an [observer's](/systems/observers/) house. Each remaining house is placed at its start position, then given a base unit when bases are on, then random vehicles and infantry until a budget is spent.

## The budget

Every house draws against one budget worked out once for the match. Every InfantryType, and every UnitType other than the [`BaseUnit`](/keys/baseunit/), that is [`AllowedToStartInMultiplayer=yes`](/keys/allowedtostartinmultiplayer/) contributes its [`Cost`](/keys/cost/) to an average price, whoever may own it. The budget is that average multiplied by the lobby's [unit count](/keys/unitcount/), or by one less than it when [bases](/keys/bases/) are on, since the base unit is paid for out of the same figure. When no type is allowed at all the average is zero, and so is the budget: every house is placed with its base unit alone.

## Each house's shortlist

A house then makes its own shortlist from the allowed types: those whose [`TechLevel`](/keys/techlevel/) its tech level reaches and whose [`Owner`](/keys/owner/) admits its country. Vehicles and infantry are listed separately, and the base unit is held out of the vehicle list.

## The start position

Positions are settled as the scenario loads, before its spawn house sections, teams, triggers, and objects are read, so that a [spawn house](/formats/scenario-objects/#spawn-houses) can name the house that starts there. The pool is the map's placed waypoints `0` through `7`, narrowed by the map's [`Official`](/keys/official/#scope-scenarios-2) flag. A position named for a seat in [the launch file](/formats/spawn-ini/#who-is-playing) is held for that house before anybody draws; the first house to draw takes an open position at random, and every house after it takes whichever open position lies furthest from those already held. A house left over when the eligible waypoints run out starts on a random cell of open ground, found once the map is loaded, and holds no numbered position. The cell chosen becomes the house's center.

## The base unit

With bases on, each house is given one [`BaseUnit`](/keys/baseunit/), on the start cell when the cell takes it and otherwise on the nearest cell the [placement search](#where-an-object-lands) finds between one and thirty-one cells out. A base unit no cell takes is discarded. In capture the flag the house's flag is attached to it. With bases off nothing is placed ahead of the random objects.

A match may deploy the base unit the moment it is placed, for every house rather than only the people playing. It deploys where it stands, into the cell its building's foundation reaches from there, and the placement search keeps the two cells nearest the start clear for that. Ground that will not take the building leaves a unit: a person's stands where it was placed, and a computer's goes looking for ground it can use. A [launch file](/formats/spawn-ini/) carries the option.

## Spending the budget

The house draws objects one at a time until the budget is spent or nothing is left to draw, charging each placed object its own `Cost`. While less than two thirds of the budget is spent and the vehicle list holds anything, the draw is a vehicle chosen at random from that list; otherwise it is an infantry type chosen at random. An object the search cannot place is discarded without being charged, and the draw is repeated. A placed object is put on guard, or on area guard for a computer house, and [`InitialVeteran`](/keys/initialveteran/) makes it [elite](/systems/veterancy/) as it is created; the base unit is not covered. Objects stay where they are put.

## Where an object lands

The search tries the start cell itself first, whatever distance range it was given, so with bases off the first random object stands on the start position; with bases on the base unit already holds it. It then works outward one distance at a time: from three to thirty-two cells for the random objects, which keeps the two cells nearest the start clear for the base unit to deploy into, and from one to thirty-one cells for the base unit itself.

At each distance the search picks a random compass direction to begin from and tries the eight cells that far out along the compass directions in turn. When none takes the object it tries the same eight again, each shifted at random by up to one cell in each axis. Candidates are clipped to the map rectangle, the upright square of cells enclosing the playfield, so near an edge the spokes bunch along it. A candidate is skipped when the shift has brought it back to the start cell, or when it lies outside the playfield, the diamond of cells the map has, as a clipped corner candidate can. A cell already holding a vehicle, infantryman, aircraft or structure is refused unless both it and the object are infantry; beyond that the object's own placement test decides, so ground the type cannot enter is refused too.

## When placement fails

:::danger[A start that cannot hold the budget never finishes loading]
The search gives up on an object past its farthest distance and the object is discarded without being charged, but the draw is repeated with no limit on attempts. A start position whose surroundings cannot take every object the budget buys keeps drawing and discarding, and the scenario never finishes loading.
:::

:::caution[A house with nothing left to draw stops short of its budget]
Once two thirds of the budget is spent only infantry may be drawn, so a house whose infantry list is empty keeps the vehicles it has and spends no more. A house whose vehicle list and infantry list are both empty, because every allowed type is above its tech level or not ownable by its country, is placed with its base unit alone. Denying every type at once makes every budget zero, with the same result.
:::
