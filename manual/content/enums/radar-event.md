---
enum_id: RadarEventType
slug: radar-event
title: Radar event
summary: Radar-notification kinds used by trigger actions that create map events.
representation: integer
bindings:
  key_value_types: []
  scripting_parameter_types: [radar-event]
source_files: [code/revent.hh]
values:
  - { constant: RADAREVENT_COMBAT, value: 0, input: "0", meaning: "Combat event." }
  - { constant: RADAREVENT_NONCOMBAT, value: 1, input: "1", meaning: "Non-combat event." }
  - { constant: RADAREVENT_DROPZONE, value: 2, input: "2", meaning: "Drop-zone event." }
  - { constant: RADAREVENT_BASE_ATTACKED, value: 3, input: "3", meaning: "Base-under-attack event." }
  - { constant: RADAREVENT_HARVESTER_ATTACKED, value: 4, input: "4", meaning: "Harvester-under-attack event." }
  - { constant: RADAREVENT_ENEMY_SENSED, value: 5, input: "5", meaning: "Enemy-detected event." }
---

A radar event is a rotating box that closes in on one cell of the radar map to draw the player's eye there. The engine raises one when a base is attacked, when a harvester is attacked, and when an enemy is sensed. A map author raises any of the six through the [Create Radar Event](/mapping/actions/taction-radar-event/) trigger action. The kinds differ in the color they pulse. Combat and the two attack events cycle orange to dark red. Non-combat and drop-zone events cycle green, and an enemy-sensed event cycles yellow.

The `[General]` section of `rules.ini` holds three comma-separated lists: [`RadarEventDurations`](/keys/radareventdurations/), [`RadarEventVisibilityDurations`](/keys/radareventvisibilitydurations/), and [`RadarEventSuppressionDistances`](/keys/radareventsuppressiondistances/). Each is indexed by these values positionally, one entry per row below, in the order shown. The lookup applies no bounds check, so a list shorter than six entries reads past its end for the later kinds. Only combat, harvester-attacked, and enemy-sensed events read a suppression distance; the other three parse one and never read it.
