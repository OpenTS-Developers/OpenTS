---
key: IsVeins
scope: overlaytype
label: Vein ground
see_also: ["system:veins", "IsVeinholeMonster"]
when_omitted:
  kind: value
  value: "no"
---

The flag makes the overlay count as vein ground wherever [vein growth](/systems/veins/#what-stops-veins) looks at an overlay. A cell with one still accepts veins instead of being blocked by its own overlay. A cell beside one is not blocked by that neighbor either. A thin vein draws its connecting piece as though the neighbor were mature vein. A monster's disposal strips overlays with the flag from the block around it. The in-map render gives the local player's color scheme to the vein carpet itself, picking it by the overlay's fixed slot in `[OverlayTypes]` rather than by the flag.

The flag does not make an overlay behave as veins. Growing, harvesting, withering, and the attack all read the vein overlay by its fixed slot in `[OverlayTypes]`. An overlay given the flag becomes ground the field will cross without becoming a field itself.
