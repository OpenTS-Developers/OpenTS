---
title: Restate the briefing the scenario carries
category: fix
release: 0.1.0
targets: []
credit: [ZivDero]
---

The briefing the player can ask for again during a mission now comes from the scenario rather than the mission database. Previously the dialog read the database alone and fell back to the scenario only when `MISSION.INI` was missing from the installation. That file always ships, so a mission the database does not list showed an empty page. Every mission written outside the shipped campaign did so, however much briefing text its own map held. The dialog matches the briefing screen the mission opened with, and the text survives a save and reload.
