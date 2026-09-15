---
key: Translucent50State
summary: The animation state at which a flame particle thins to half faded.
see_also: ["Translucent25State", "Translucency", "EndStateAI", "StartStateAI"]
when_omitted:
  kind: value
  value: "-1"
---

The second of the two fade states a [`Fire`](/keys/behaveslike/#scope-particletype) particle has. [`Translucent25State`](/keys/translucent25state/) covers when the pair is tested, which states can never be matched, and what a `-1` means. The only difference here is the level reached and the order: this one is applied after the quarter fade and wins if both name the same state.

Nothing forces the two into sequence. A flame whose half fade sits at an earlier state than its quarter fade thins to a half first and then back to a quarter, because each state simply sets the level it names.
