---
format_id: ai_triggers
title: AI triggers
summary: Defines weighted conditions that create one or two TeamTypes for an AI house.
kind: record
route: /mapping/ai-triggers/
files:
  - AI.INI
  - AIFS.INI
  - map file
section: AITriggerTypes
syntax: "<AITrigger ID>=<up to 18 comma-separated fields>"
enable_section: AITriggerTypesEnable
fields:
  - position: 1
    label: Name
    value: Display name
    required: true
  - position: 2
    label: Primary team
    value: TeamType ID or <none>
    required: true
  - position: 3
    label: Owner
    value: House ID, <all>, or <none>
    required: true
  - position: 4
    label: Ignored
    value: Present but discarded
    required: true
  - position: 5
    label: Condition type
    value: Integer from 0 through 4
    required: true
  - position: 6
    label: Condition object
    value: TechnoType ID
    required: true
  - position: 7
    label: Comparator
    value: Hexadecimal comparison block
    required: true
  - { position: 8, label: Starting weight, value: Number truncated to an integer, required: false }
  - { position: 9, label: Minimum weight, value: Number truncated to an integer, required: false }
  - { position: 10, label: Maximum weight, value: Number truncated to an integer, required: false }
  - { position: 11, label: Skirmish, value: 0 or 1, required: false }
  - { position: 12, label: Ignored, value: Present but discarded, required: false }
  - { position: 13, label: Side, value: "The side's position in the rules' [Sides] list, counted from one; 0 leaves the trigger unrestricted", required: false }
  - { position: 14, label: Base defense, value: 0 or 1, required: false, note: "Stored and written back, never consulted; whether a trigger counts as defensive comes from its teams' IsBaseDefense." }
  - { position: 15, label: Secondary team, value: TeamType ID or <none>, required: false }
  - { position: 16, label: Easy, value: 0 or 1, required: false }
  - { position: 17, label: Medium, value: 0 or 1, required: false }
  - { position: 18, label: Hard, value: 0 or 1, required: false }
key_scopes:
  - applies_to: AITriggerType
source_files:
  - code/aitrig.cpp
  - code/init.cpp
  - code/scenario.cpp
---

Entries in `[AITriggerTypes]` are definitions: the key is the AITrigger ID and the value is its serialized record. Positions 1 through 7 must be present for parsing to finish. An incomplete entry remains registered with partially parsed state.

OpenTS loads `AI.INI`, then `AIFS.INI` when Firestorm is enabled, then the map-local definitions.

For global definitions, OpenTS enables every loaded AI trigger. For map definitions, `[AITriggerTypesEnable]` maps AITrigger IDs to booleans. In non-campaign sessions, a listed local trigger is enabled regardless of that boolean.

Five condition types are defined. The table sets each one against what it measures and which of the record's two supporting fields that measurement reaches. What the third column settles is which fields a given condition leaves inert — a record may write anything at all in those and none of it is consulted. Only one of the five asks about the owning house; the other four are questions about that house's enemy, and a house with no enemy passes none of them.

| Value | What it measures | Supporting fields it reads |
| --- | --- | --- |
| `0` | How many of the condition object the enemy currently holds | Condition object and comparison block |
| `1` | How many of the condition object the owning house currently holds | Condition object and comparison block |
| `2` | The enemy's power output less its drain, against a fixed `100` | Neither |
| `3` | The enemy's power output less its drain, against a fixed `0` | Neither |
| `4` | The enemy's spendable credits | Comparison block |

The comparison block carries two values rather than one: the number being compared against, and which of six comparisons is applied to it — less than, less than or equal to, equal to, greater than or equal to, greater than, or not equal to.

The condition object resolves as infantry, vehicle, aircraft, then building, and the first matching ID is used. An ID that matches none of them does not reject the trigger: the count it stands for is taken as zero and the comparison is made against that.
