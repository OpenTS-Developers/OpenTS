---
key: Translucent25State
summary: The animation state at which a flame particle thins to a quarter faded.
see_also: ["Translucent50State", "Translucency", "EndStateAI", "StartStateAI"]
when_omitted:
  kind: value
  value: "-1"
---

Only a [`Fire`](/keys/behaveslike/#scope-particletype) particle has the two fade states, and it tests them on the frames its state advances, never in between. A state at or below [`StartStateAI`](/keys/startstateai/) is therefore never matched, because the first test happens after the first advance, and neither is one above [`EndStateAI`](/keys/endstateai/), which the sequence stops at.

The state is held in one unsigned byte while the particle's own state is signed, so anything from 128 to 255 is beyond what a particle can reach. A `-1` is stored as 255, which is what leaves a flame that never thins. Setting this and [`Translucent50State`](/keys/translucent50state/) to the same state leaves the particle at the half fade, since that one is applied second.

The fade only shows at the High detail setting; [`Translucency`](/keys/translucency/#scope-particletype) covers what the three levels look like and what happens at the lower settings.
