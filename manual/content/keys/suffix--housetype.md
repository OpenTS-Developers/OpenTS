---
key: Suffix
scope: housetype
label: Country filename extension
summary: Parsed country filename extension that the engine never uses.
no_effect: true
see_also: [Prefix, Name]
when_omitted:
  kind: value
  value: ""
  note: The read falls back to an empty string and discards an empty result instead of storing it, so the suffix is empty unless a rules file writes one.
---

The name promises the three-letter extension of a country-specific file. The value is cut to three characters as it is stored. Nothing in the game reads it back. A Debug-only helper composes a team-type description from it, but no code path calls that helper.

```ini title="rules.ini"
[GDI]
Suffix=GDI
```

Unlike [`Prefix=`](/keys/prefix/), this key cannot be cleared: the read discards an empty result.
