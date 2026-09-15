---
key: DeployingAnim
summary: The shape file that replaces a structure's own artwork while it unloads.
see_also: ["DoorAnim", "UnderDoorAnim", "NormalZAdjust", "WeaponsFactory"]
when_omitted:
  kind: value
  value: ""
  note: No deploying shape is loaded and the structure keeps its own artwork throughout.
---

The value is a filename without its extension, and only a non-empty value is taken. The named `<value>.SHP` is loaded with the rules, and refetched on the same occasions and with the same theater rewrite [`DoorAnim`](/keys/dooranim/) describes.

While the structure is unloading, this file is drawn in place of the structure's main shape, and the type's [`NormalZAdjust`](/keys/normalzadjust/) is dropped to zero for that draw. The frame is still the structure's own current frame number. Its highest reachable value is the frame at half the count of whichever file is in use, one past the last of the healthy frames. A 20-frame file therefore caps at frame `10`. A deploying shape has to follow the same healthy-then-damaged layout as the main artwork. That limit sits on the main draw itself and applies whether or not a deploying shape is in use.
