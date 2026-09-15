---
key: Prefix
summary: Parsed country filename letter that the engine never uses.
no_effect: true
see_also: [Suffix, Name]
when_omitted:
  kind: value
  value: "A"
---

The key's name suggests the letter that begins a country-specific filename. Only the first character of the value is kept, because the destination has room for one character and a terminator. No gameplay path reads the stored letter.
