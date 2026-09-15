---
key: WebbedInfantry
summary: The artwork an infantryman is drawn with while a web holds him.
see_also: [Webby, WebDuration, WebDurationVariation, IsWebImmune, Disguise]
when_omitted:
  kind: value
  value: none
---

```ini title="rules.ini"
[AudioVisual]
WebbedInfantry=MYWEBGUY ; an AnimType registered in [Animations]
```

A [`Webby=yes`](/keys/webby/) warhead striking an infantryman who is not [`IsWebImmune=yes`](/keys/iswebimmune/) deals no damage at all. It instead pins him for [`WebDuration`](/keys/webduration/) frames, give or take [`WebDurationVariation`](/keys/webdurationvariation/), puts him into the struggling sequence, and springs the [Paralyzed](/mapping/events/tevent-paralyzed/) trigger event on his tag. For as long as he is in that sequence, this animation's artwork is used in his place.

Only the artwork is substituted. The frame shown is still chosen from the soldier's own struggling sequence, and his facing, remap and position are unchanged. The substitute therefore has to be built to match the sequence it stands in for.

The substitution is guarded, so leaving the key unset draws the soldier in his own artwork while he struggles rather than failing. It also takes precedence over the [`Disguise`](/keys/disguise/) substitution, so a webbed spy shows the web rather than his cover.
