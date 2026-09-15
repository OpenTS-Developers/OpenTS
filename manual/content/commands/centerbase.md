---
command_id: CenterBase
---

Centers the view on the player's construction yard: a structure the player controls whose type is listed in [`BuildConst`](/keys/buildconst/), preferring the primary one. When the player has structures but none of those types, the first other structure is used instead. When the player has no structures at all, the view goes to an undeployed unit of a [`BaseUnit`](/keys/baseunit/) type. When the player has neither, the view stays where it is. Follow mode ends in every case. A structure waiting to be placed does not stop the jump, but keeps its placement cursor under the mouse pointer.
