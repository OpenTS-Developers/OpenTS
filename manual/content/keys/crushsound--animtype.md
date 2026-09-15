---
key: CrushSound
scope: animtype
label: Crushed sound
no_effect: true
see_also: [Crushable, Report]
when_omitted:
  kind: value
  value: none
---

The sound is played by a crusher as it flattens its victim, and only after that victim has passed a [`Crushable`](/keys/crushable/#scope-animtype) test that an animation can never reach. Every crush path looks for its victim among a cell's occupiers or at the cell's overlay, and an animation is neither. No gameplay path reads the sound.

An animation's own sound is [`Report`](/keys/report/#scope-animtype), read from the same `art.ini` section. A `CrushSound=` value names a sound ID registered in [SOUND.INI](/formats/sound-ini/).
