---
key: ComputerBaseDefenseResponse
summary: Multiplies an attacker's ThreatPosed to size the defenders a computer house calls up.
see_also: ["system:target-selection"]
when_omitted:
  kind: value
  value: "3"
---

When one of a computer house's buildings, or one of its [`ToProtect=yes`](/keys/toprotect/) objects, is damaged by enemy infantry or an enemy vehicle, the house sizes its response at the attacker's [`ThreatPosed`](/keys/threatposed/) multiplied by this value. It then picks defenders from its own infantry and vehicles until that figure is covered. Anything already fighting that attacker is counted off the figure rather than picked again. An attacker whose type leaves `ThreatPosed` at zero calls up nobody however large this value is.

The call-up is skipped for a human house, for an attacker that is an ally, and for a building whose type is [`Insignificant=yes`](/keys/insignificant/) or that deploys back into a vehicle. In a campaign game only, it is also skipped for a building that has a weapon of its own.
