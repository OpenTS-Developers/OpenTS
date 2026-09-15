---
key: ImageLetter
scope: theater
label: Theater image letter
see_also: [NewTheater, Theater]
when_omitted:
  kind: context-dependent
  note: "`T` for TEMPERATE and `A` for SNOW, which keep their original settings; none for any other theater, whose artwork is then never renamed."
---

Artwork marked [`NewTheater=yes`](/keys/newtheater/) spells the theater into the second letter of its own file name rather than into an extension. This setting supplies that letter. `GACNST` becomes `GTCNST` in a theater lettered `T` and `GACNST` in one lettered `A`. Only the first character of the value is read, and it is used in upper case.

```ini title="rules.ini"
[DESERT]
ImageLetter=D   ; GACNST becomes GDCNST
```

The letter decides which artwork is renamed as well as what it is renamed to: a name is rewritten only when its second letter is already some declared theater's image letter. Choosing a letter that unrelated artwork uses as its second character draws that artwork into the convention. Once a theater lettered `I` is declared, a theater lettered `T` loads `CITY01` as `CTTY01`. Pick a letter that no unrelated artwork uses as its second character.
