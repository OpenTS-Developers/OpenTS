---
key: EngineerDamage
summary: A figure the rules parser stores for engineer damage that no gameplay path reads.
see_also: ["system:capture", "EngineerCaptureLevel"]
when_omitted:
  kind: value
  value: "0"
---

The value survives a save, but writing it changes nothing a player can see. [`EngineerCaptureLevel`](/keys/engineercapturelevel/) does decide one thing: the cursor an engineer shows over a non-allied structure. What an engineer does on arrival is settled from the flags at the structure rather than from either figure.
