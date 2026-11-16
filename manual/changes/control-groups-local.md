---
title: Keep the player's control groups apart from scenario groups
category: fix
release: 0.3.0
targets:
- type: key
  id: Group
  scope: teamtype
  effect: changed
- type: action
  id: TACTION_WAKEUP_GROUP
  effect: changed
- type: action
  id: TACTION_SET_GROUP_ID
  effect: changed
credit:
- ZivDero
---

A control group the player assigned set the same group number that teams recruit by and that Wakeup group matches, on the player's machine only. A unit in a control group was skipped by a group `-1` team of the player's house, Wakeup group woke the player's control group of its number, and in a multiplayer game a team or trigger that depended on it could put the match out of sync.

Control groups are now kept apart. Assigning a control group with the hotkeys no longer changes the group, so a team can recruit a unit in a control group, and Wakeup group no longer wakes control groups. A placed object's group, Set Group ID and joining a team set the group only: they no longer put the object in the control group of the same number, and joining a team no longer takes a unit out of its control group.