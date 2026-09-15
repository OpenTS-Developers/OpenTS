---
key: CompEasyBonus
summary: Whether a computer house drops one difficulty slot when the session holds more than one human player.
see_also: ["system:ai-team-production"]
when_omitted:
  kind: value
  value: "yes"
---

The drop is applied as computer players are assigned in a session with more than one human player. A house already in slot 0 keeps its slot. The computer takes the chosen setting's slot counted from the other end: the Easy setting gives it slot 2, the Hard setting slot 0. Dropping a slot therefore moves the computer toward the harder end, and on the Medium setting it lands in the slot the Hard setting would have given it.

The shift is not confined to the combat and production biases of the difficulty sections. Every per-difficulty rules list the house reads moves with it, and so does the [difficulty flag its AI triggers are tested against](/systems/ai-team-production/#difficulty).
