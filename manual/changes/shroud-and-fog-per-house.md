---
title: Keep a shroud and fog for every house
category: feature
release: 0.2.0
targets:
- type: system
  id: map-visibility
  effect: changed
- type: key
  id: AllyReveal
  effect: changed
credit: [ZivDero]
---

Every house now has its own shroud and fog; before, only the local player's existed.

Outside a campaign, shroud and fog regrowth now leaves uncovered what an ally's vehicles and infantry see while `rules.ini` sets `AllyReveal=yes` under `[AudioVisual]`, what the objects of a house whose radar the player has spied on see, and what an object carrying the player's limpet drone sees. Before, of an ally's objects only its structures were spared, and the fog pass could still cover part of what they saw. The same objects now also uncover ground when the playable area grows to include them. A campaign keeps the original rules for both.

An ally's airborne infantry and tile-laying structures, a structure an ally captures, and an object carrying an ally's limpet drone now uncover ground for the player like the ally's other objects. So do the aircraft and airborne infantry of a house whose radar the player has spied on.
