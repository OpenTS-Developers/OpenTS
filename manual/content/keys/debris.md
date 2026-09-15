---
key: Debris
summary: Animations a Tiberium type can leave behind when a chain reaction wipes one of its cells.
see_also: ["system:tiberium", "Color", "ChainReaction"]
when_omitted:
  kind: value
  value: "none"
  note: An empty list, which leaves nothing behind after a chain reaction.
---

An animation with [`TiberiumChainReaction=yes`](/keys/tiberiumchainreaction/) clears the Tiberium in the cell it occupies. One time in three it also creates one entry of this list, picked at random, ten leptons above the cell and recolored through the type's [`Color`](/keys/color/#scope-tiberium). That is the only path that reads the list; an empty list simply leaves nothing behind.
