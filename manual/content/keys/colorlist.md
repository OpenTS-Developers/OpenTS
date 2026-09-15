---
key: ColorList
summary: The colors a spark or railgun particle blends through as it ages.
see_also: ["ColorSpeed", "StartColor1", "StartColor2", "BehavesLike"]
when_omitted:
  kind: value
  value: ""
---

```ini title="rules.ini"
[MYSPARK] ; a ParticleType registered in [Particles]
BehavesLike=Spark
MaxEC=500
XVelocity=10
YVelocity=10
ZVelocityRange=15
ColorList=(255,255,255),(200,200,80),(200,10,10),(0,0,0)
ColorSpeed=.13
```

Each entry is a red, green and blue triplet. The parser strips an opening parenthesis from the front of the red component and a closing one from the end of the blue component before converting the numbers, so `(255,255,255)` reads as white. The triplets are counted off in threes from the start of the line regardless of where the brackets fall, so a triplet with a component missing shifts every color after it. Components are stored in one byte each, so 256 reads as 0 and 300 as 44.

Only the two behaviors that are plotted as a single lit pixel, [`Spark` and `Railgun`](/keys/behaveslike/#scope-particletype), are drawn from the list. The particle holds a position between two neighboring entries and slides along it at [`ColorSpeed`](/keys/colorspeed/); the pixel put on screen is the mix of the two at that position. The first pair runs from the color picked out of [`StartColor1`](/keys/startcolor1/) and [`StartColor2`](/keys/startcolor2/) to the second entry. It then steps from one entry to the next until it reaches the last pair, which is where the particle stays for the rest of its life. The pair is fetched only in the drawing code, and only for a particle that is actually on screen. A list too short to hold the pair therefore costs nothing while no particle of the type is in view, and the first one drawn is the one that reads past the list.

:::danger[An empty list stops the game and a single entry draws an arbitrary color]
The pair is fetched by position with no check that the list is long enough to hold it. With a single entry the second color is read from whatever memory follows the list, and the particle is drawn an arbitrary color. With an empty list it is read from a list that was never allocated at all, and the game stops the first time such a particle is drawn.
:::

:::danger[The list is emptied by any later rules layer that reopens the section]
Unlike every neighboring setting, the list is not read against its current contents: a section that does not set this key clears it. A map's rules overrides or an expansion rules file that opens the particle's section to change one unrelated value therefore discards the colors. The next spark or railgun particle of that type drawn on screen then crashes the game. Repeat the whole list in every layer that names the section.
:::
