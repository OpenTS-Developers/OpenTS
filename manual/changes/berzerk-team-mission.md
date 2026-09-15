---
title: Run the Berzerk team mission
category: fix
release: 0.1.0
targets:
- type: mission
  id: TMISSION_BERZERK
  effect: changed
credit: [ZivDero, tomsons26]
---

A team script that reaches the Berzerk mission now sends the team's cyborgs berserk and moves on to the next line. The mission had no handler, so a team that reached it did nothing and never advanced. The rest of the script was unreachable for the remainder of the scenario.
