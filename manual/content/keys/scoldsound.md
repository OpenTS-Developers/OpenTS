---
key: ScoldSound
summary: The sound played when the interface refuses a request.
see_also: ["system:production"]
when_omitted:
  kind: value
  value: "none"
---

Three established paths play it:

- a production order dropped because the queue is full or the type is at its [build limit](/systems/production/#the-queue);
- clicking a sidebar scroll arrow on a column already at its end, or giving a command that scrolls both columns when neither can move (the commands that scroll one column alone never play it);
- ordering repair on a structure already at maximum strength.

The production and repair cases check whose house it is, playing only for a player-controlled house; the sidebar case is already player-side by nature.

A fourth path is written and cannot be reached. Infantry, a walker, a hovercraft and a driven vehicle would each scold on abandoning a movement order they could not carry out. Every one of those places first tests a flag that nothing in the game ever raises: it starts clear, is cleared again in two dozen places, and is never set anywhere. Giving an order that cannot be carried out is silent.
