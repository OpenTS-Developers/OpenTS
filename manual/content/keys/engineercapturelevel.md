---
key: EngineerCaptureLevel
summary: The fraction of maximum strength at or below which an engineer's cursor over a non-allied structure is the enter cursor.
see_also: ["system:capture"]
when_omitted:
  kind: value
  value: "1"
---

Over a capturable non-allied structure, the value decides nothing but the cursor. Above the fraction the engineer takes a damage action that has no cursor art of its own and falls back to the ordinary pointer; at or below it the enter cursor appears. Clicking issues a capture order whichever cursor the engineer was showing. [What happens at the structure](/systems/capture/#walking-in) is settled again from the flags found there, so lowering the value cannot make an engineer damage anything.

At the engine default of `1` the damage action is unreachable, because a structure's strength as a fraction of its maximum can never exceed `1`. Any value below `1` opens a band in which a healthy non-allied structure shows no capture cursor at all. At `0.5`, for example, a structure above half strength falls into that band.
