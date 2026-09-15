---
key: SidebarImage
summary: The name of the shape file drawn as the superweapon's sidebar cameo.
see_also: ["system:superweapons"]
when_omitted:
  kind: computed
  note: The name of the section that declares the superweapon.
---

The value is completed with a `.SHP` extension and looked up in the mix files as the rules are read. A name that resolves to nothing falls back to the generic `XXICON.SHP` icon, so a section that exists never shows a blank cameo. A weapon declared with no section at all is a different case: its read is skipped whole, so no artwork is fetched when the rules are read. Loading a saved game refetches artwork for every superweapon, and such a weapon then falls back on the generic icon like any other. Only the first 24 characters are kept.

```ini title="rules.ini"
[MyIonStrike]      ; example superweapon section
Type=IonCannon
SidebarImage=IONCICON
```
