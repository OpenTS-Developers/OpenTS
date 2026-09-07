---
title: Restate the briefing the scenario carries
category: fix
release: 0.1.0
targets: []
credit: [ZivDero]
---

Asking for the briefing again during a mission read the mission database and nothing else, falling back to the scenario only when the whole `MISSION.INI` file was missing from the installation. Since that file always ships, a scenario the database does not list — every mission written outside the shipped campaign — restated to an empty page however much briefing text its own map held. The dialog now shows the briefing the scenario carries and consults the database only when the scenario carries none, so it matches what the briefing screen showed at the start of the mission and survives a save and reload.
