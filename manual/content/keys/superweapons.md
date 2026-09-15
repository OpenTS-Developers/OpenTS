---
key: SuperWeapons
summary: The house intelligence level at which a computer house begins firing its superweapons in a campaign.
see_also: [IQ, "system:superweapons"]
when_omitted:
  kind: value
  value: "4"
---

The threshold is read only in a campaign game, where a computer house takes its [`IQ`](/keys/iq/) from its own scenario section and starts at 0. Outside a campaign the check is bypassed entirely, so every computer house fires its superweapons whatever its intelligence level says.

The gate covers the whole [superweapon step of the computer's decision pass](/systems/superweapons/#the-computers-use), so raising it above the intelligence a campaign grants leaves the house charging weapons it never fires.
