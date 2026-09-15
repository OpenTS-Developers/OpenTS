---
key: BridgeExplosions
summary: The explosions that accompany a bridge span coming down.
see_also: [MetallicDebris, BridgeStrength, C4Warhead]
when_omitted:
  kind: value
  value: ""
---

```ini title="rules.ini"
[AudioVisual]
BridgeExplosions=MYBLAST1,MYBLAST2 ; AnimTypes registered in [Animations]
```

A bridge cell that is destroyed kills everything on the deck, drops everything hanging beneath it, and then on a ninety-five percent roll shows one entry chosen at random from this list. It is placed at deck height over the cell, thrown up to twenty-five leptons off center in each direction, and held back one to five frames before it starts. A long span coming down therefore produces a ragged run of blasts rather than a single flash.

[`MetallicDebris`](/keys/metallicdebris/) covers the wreckage thrown alongside it on an even chance.

:::caution[An empty list also suppresses the wreckage]
This is one of the few animation lists the engine checks the length of, so leaving it empty is silent rather than fatal. The check gates the whole branch, though (the metallic debris is thrown inside it), so a bridge that has no explosion to show throws no wreckage either.
:::
