---
title: Keep AI teams from retaliating against allies
category: fix
release: 0.1.0
targets:
- type: system
  id: ai-team-execution
  effect: changed
credit: [ZivDero, Rampastring]
---

An AI team that is under way now keeps its target and its members' orders when an allied object damages a member. Damage from a non-allied object can still turn the team onto the attacker. A team that has not started still enters its reforming state after any nonzero damage, whoever dealt it.
