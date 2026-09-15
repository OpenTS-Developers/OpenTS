---
key: Droppod
summary: Makes an infantry-only TeamType arrive by drop pod.
see_also: [DropPod, "system:drop-pods"]
when_omitted:
  kind: value
  value: "no"
---

```ini title="AI.INI, AIFS.INI, or map file"
[MyDropTeam]
Droppod=yes
TaskForce=MyInfantryTaskForce
```

When the team is created as reinforcements, each member receives drop-pod locomotion instead of entering from the map edge. The flag is honored only when the linked TaskForce contains infantry and no other object types.

:::caution[Mixed task forces do not use pods]
If even one TaskForce member is not infantry, the engine rejects drop-pod delivery for the whole team. It does not drop only the infantry subset.
:::

This is not the `[AudioVisual]` animation list. That separate key is spelled [`DropPod`](/keys/droppod-global-rules/) with an uppercase second `P`.
