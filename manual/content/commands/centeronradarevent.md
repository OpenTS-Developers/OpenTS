---
command_id: CenterOnRadarEvent
---

Centers the tactical view on the last place the game flagged for the player's attention. Until something has been flagged in the current game, the command does nothing. The place is kept in saved games.

These move the place:

- A radar event starts on the radar. The game starts one when EVA announces that the player's base or one of the player's harvesters is under attack, when the radar detects a hidden enemy such as a cloaked or tunneling unit, and when a map's [Create Radar Event](/mapping/actions/taction-radar-event/) trigger action runs. An event left out because one of the same kind is already showing nearby does not move the place.
- One of the player's vehicles, soldiers or aircraft is destroyed and EVA [announces the loss](/systems/destruction-and-debris/#the-loss-announcement). The place becomes the cell the object was heading to.
- [Reinforcements](/mapping/actions/taction-reinforcements/) arrive for the player's house or an ally. The place becomes the cell they arrived at.

A unit or aircraft leaving a factory does not move the place.
