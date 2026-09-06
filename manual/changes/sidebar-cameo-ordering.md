---
title: Order the sidebar cameos by category and rules index
category: feature
release: 0.2.0
targets:
- type: system
  id: sidebar
  effect: changed
- type: key
  id: SidebarSorting
  effect: added
- type: key
  id: CameoSortOrder
  effect: added
- type: key
  id: SortCameoAsBaseDefense
  effect: added
- type: format
  id: save-games
  effect: changed
credit:
- ZivDero
- Rampastring
---

The sidebar arranged its cameos in the order the offers arrived, so the same rules set gave a
different strip in every game. Each strip is now sorted by kind and then by declaration order,
with walls, gates and base defenses last among the structures. `SidebarSorting=no` in `SUN.INI`
restores the old arrangement. `CameoSortOrder=` places a type's cameo within its category
without moving any rules list, and `SortCameoAsBaseDefense=` decides whether a building sorts
with the defenses.

Saves made by earlier development snapshots of this cycle no longer load, because the stored
type definitions grew by the new type keys.
