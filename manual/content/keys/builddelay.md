---
key: BuildDelay
summary: Parsed value that the engine never uses.
no_effect: true
see_also: ["system:production"]
when_omitted:
  kind: value
  value: ".03"
  note: The difficulty block is re-read from fixed defaults whenever its section is present, so a later file that contains the section without this key restores .03 rather than keeping the earlier value.
---
