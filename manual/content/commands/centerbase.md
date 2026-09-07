---
command_id: CenterBase
---

Centres the view on the player's construction yard: the first structure the player controls whose type is listed in [`BuildConst`](/keys/buildconst/), preferring the primary one. When the player has structures but none of those types, the first other structure is used instead. When the player has no structures at all, the view goes to an undeployed unit of a [`BaseUnit`](/keys/baseunit/) type. Follow mode ends, and a structure waiting for placement keeps its cursor.
