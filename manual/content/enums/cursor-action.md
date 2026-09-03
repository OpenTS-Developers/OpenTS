---
enum_id: ActionType
slug: cursor-action
title: Cursor action
summary: Named cursor and click actions accepted by action-valued INI settings.
representation: token
bindings:
  key_value_types: [actiontype]
  scripting_parameter_types: []
source_files: [code/action.hh, code/ccini.cpp]
values:
  - { constant: ACTION_NONE, value: 0, input: "None", meaning: "No available action." }
  - { constant: ACTION_MOVE, value: 1, input: "Move", meaning: "Move to the selected position." }
  - { constant: ACTION_NOMOVE, value: 2, input: "NoMove", meaning: "Movement is not allowed at this position." }
  - { constant: ACTION_ENTER, value: 3, input: "Enter", meaning: "Enter the selected transport, facility, or building." }
  - { constant: ACTION_SELF, value: 4, input: "Self", meaning: "Act on the selected object itself." }
  - { constant: ACTION_ATTACK, value: 5, input: "Attack", meaning: "Attack the selected target." }
  - { constant: ACTION_HARVEST, value: 6, input: "Harvest", meaning: "Harvest the selected resource cell." }
  - { constant: ACTION_SELECT, value: 7, input: "Select", meaning: "Replace the current selection." }
  - { constant: ACTION_TOGGLE_SELECT, value: 8, input: "ToggleSelect", meaning: "Toggle the target's selection state." }
  - { constant: ACTION_CAPTURE, value: 9, input: "Capture", meaning: "Capture the selected building." }
  - { constant: ACTION_REPAIR, value: 10, input: "Repair", meaning: "Repair the selected object." }
  - { constant: ACTION_SELL, value: 11, input: "Sell", meaning: "Sell the selected building." }
  - { constant: ACTION_SELL_UNIT, value: 12, input: "SellUnit", meaning: "Sell the selected unit." }
  - { constant: ACTION_NO_SELL, value: 13, input: "NoSell", meaning: "Indicate that selling is unavailable." }
  - { constant: ACTION_NO_REPAIR, value: 14, input: "NoRepair", meaning: "Indicate that repair is unavailable." }
  - { constant: ACTION_SABOTAGE, value: 15, input: "Sabotage", meaning: "Enter and sabotage the selected object." }
  - { constant: ACTION_TOTE, value: 16, input: "Tote", meaning: "Send an empty carryall to pick up the allied vehicle under the cursor." }
  - { constant: ACTION_PARA_INFANTRY, value: 17, input: "DontUse2", meaning: "Legacy parachute-infantry action.", note: "The parser token intentionally marks this slot as not for normal use." }
  - { constant: ACTION_PARA_SABOTEUR, value: 18, input: "DontUse3", meaning: "Legacy parachute-saboteur action.", note: "The parser token intentionally marks this slot as not for normal use." }
  - { constant: ACTION_NUKE_BOMB, value: 19, input: "Nuke", meaning: "Target a nuclear or multi-missile strike." }
  - { constant: ACTION_AIR_STRIKE, value: 20, input: "DontUse4", meaning: "Legacy air-strike action.", note: "The parser token intentionally marks this slot as not for normal use." }
  - { constant: ACTION_CHRONOSPHERE, value: 21, input: "DontUse5", meaning: "Legacy chronosphere source action.", note: "The parser token intentionally marks this slot as not for normal use." }
  - { constant: ACTION_CHRONO2, value: 22, input: "DontUse6", meaning: "Legacy chronosphere destination action.", note: "The parser token intentionally marks this slot as not for normal use." }
  - { constant: ACTION_IRON_CURTAIN, value: 23, input: "DontUse7", meaning: "Legacy invulnerability action.", note: "The parser token intentionally marks this slot as not for normal use." }
  - { constant: ACTION_SPY_MISSION, value: 24, input: "DontUse8", meaning: "Legacy reconnaissance action.", note: "The parser token intentionally marks this slot as not for normal use." }
  - { constant: ACTION_GUARD_AREA, value: 25, input: "GuardArea", meaning: "Guard the selected location or object." }
  - { constant: ACTION_HEAL, value: 26, input: "Heal", meaning: "Heal the selected infantry." }
  - { constant: ACTION_DAMAGE, value: 27, input: "Damage", meaning: "Enter and damage the selected building." }
  - { constant: ACTION_GREPAIR, value: 28, input: "GRepair", meaning: "Enter and complete building repair." }
  - { constant: ACTION_NO_DEPLOY, value: 29, input: "NoDeploy", meaning: "Indicate that deployment is unavailable." }
  - { constant: ACTION_NO_ENTER, value: 30, input: "NoEnter", meaning: "Indicate that entry is unavailable." }
  - { constant: ACTION_NO_GREPAIR, value: 31, input: "NoGRepair", meaning: "Indicate that building entry repair is unavailable." }
  - { constant: ACTION_TOGGLE_POWER, value: 32, input: "TogglePower", meaning: "Toggle a building's power state." }
  - { constant: ACTION_NO_TOGGLE_POWER, value: 33, input: "NoTogglePower", meaning: "Indicate that power toggling is unavailable." }
  - { constant: ACTION_ENTER_TUNNEL, value: 34, input: "EnterTunnel", meaning: "Enter a tunnel." }
  - { constant: ACTION_NO_ENTER_TUNNEL, value: 35, input: "NoEnterTunnel", meaning: "Indicate that tunnel entry is unavailable." }
  - { constant: ACTION_EMPULSE, value: 36, input: "EMPulse", meaning: "Target an EMPulse superweapon." }
  - { constant: ACTION_ION_CANNON, value: 37, input: "IonCannon", meaning: "Target an ion-cannon strike." }
  - { constant: ACTION_EMPULSE_RANGE, value: 38, input: "EMPulseRange", meaning: "Refuse the target: the player has no powered EM pulse cannon whose weapon reaches it." }
  - { constant: ACTION_CHEM_BOMB, value: 39, input: "ChemBomb", meaning: "Target a chemical missile strike." }
  - { constant: ACTION_PLACE_WAYPOINT, value: 40, input: "PlaceWaypoint", meaning: "Place a waypoint." }
  - { constant: ACTION_NO_PLACE_WAYPOINT, value: 41, input: "NoPlaceWaypoint", meaning: "Indicate that a waypoint cannot be placed." }
  - { constant: ACTION_ENTER_WAYPOINT_MODE, value: 42, input: "EnterWaypointMode", meaning: "Enter waypoint planning mode." }
  - { constant: ACTION_FOLLOW_WAYPOINT, value: 43, input: "FollowWaypoint", meaning: "Follow the selected waypoint path." }
  - { constant: ACTION_SELECT_WAYPOINT, value: 44, input: "SelectWaypoint", meaning: "Select a waypoint." }
  - { constant: ACTION_LOOP_WAYPOINT_PATH, value: 45, input: "LoopWaypointPath", meaning: "Toggle looping for a waypoint path." }
  - { constant: ACTION_DRAG_WAYPOINT, value: 46, input: "DragWaypoint", meaning: "Drag an existing waypoint." }
  - { constant: ACTION_ATTACK_WAYPOINT, value: 47, input: "AttackWaypoint", meaning: "Attack along a waypoint path." }
  - { constant: ACTION_ENTER_WAYPOINT, value: 48, input: "EnterWaypoint", meaning: "Append or enter a waypoint." }
  - { constant: ACTION_PATROL_WAYPOINT, value: 49, input: "PatrolWaypoint", meaning: "Patrol a waypoint path." }
  - { constant: ACTION_DROP_POD, value: 50, input: "DropPod", meaning: "Target a drop-pod delivery." }
  - { constant: ACTION_RALLY_TO_POINT, value: 51, input: "Rally To Point", meaning: "Set a rally point." }
  - { constant: ACTION_ATTACK_SUPPORT, value: 52, input: "Attack Support", meaning: "Put a healer on Guard Area and any other unarmed object on Guard." }
---

These are the actions the engine picks between as the cursor crosses the map — what the mouse shape offers and what a left click will then do. They are not the trigger actions a map fires from its tags, which are a separate list.

One rules setting takes a name from here: a SuperWeaponType's [`Action`](/keys/action/). While the map is in that weapon's targeting mode the name becomes the action under the cursor, which is what picks the mouse shape. A weapon left at `None` neither takes the cursor nor can be fired by a click at all — which is where an unrecognized name lands it, since the parser reads anything it does not know as `None`.

Seven entries carry historical `DontUse` tokens even though their engine constants still name older actions. Nothing assigns or tests those seven, so what survives of them is the spelling: they are distinct names a superweapon can claim, and nothing more.
