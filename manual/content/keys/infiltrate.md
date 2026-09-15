---
key: Infiltrate
summary: Allows a soldier to be given a structure as a target and to be sent into it.
see_also: ["system:capture"]
when_omitted:
  kind: value
  value: "no"
---

On its own the setting does nothing at the structure. It is what makes the structure reachable. The attack cursor over a non-allied object normally needs a weapon; this setting offers it to an unarmed soldier all the same, so armed and unarmed soldiers of the type take the same cursor. Over a [`Capturable=yes`](/keys/capturable/) structure that cursor becomes the enter cursor. An attack order aimed at a structure is converted into a capture order. The soldier is also exempt from the check that would otherwise discard a destination lying in another movement zone. What the arrival then does comes from [`Engineer=yes`](/keys/engineer/#scope-infantrytype) or [`Agent=yes`](/keys/agent/); a soldier with neither is simply consumed.

[`C4=yes`](/keys/c4/) and `Engineer=yes` each force this on immediately after the section's own line has been read. An explicit `no` in such a section is overwritten in the same pass. `Agent=yes` forces nothing, which is why a spy has to be given the setting explicitly.
