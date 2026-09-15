---
key: Name
scope: aitriggertype
label: Type display name
see_also: [Multiplay, SidebarCameoText]
when_omitted:
  kind: value
  value: "the type's own section name"
---

Every registered type has two names: the section it is declared under, and the display name players see. The value is held in a 49-byte field, so it is cut off after 48 characters. It is displayed literally rather than being looked up as a translatable label.

```ini title="rules.ini"
[GAPOWR]
Name=GDI Power Plant
```

The display name appears wherever an object or a choice has to be identified. It appears in the sidebar's build label and price line, and it names the object under the cursor. The skirmish and multiplayer setup screens use it as the label in the country box. A country's display name is also matched, alongside its section name, whenever a house is looked up by name. A house list that names `Civilian` finds the country declared as `[Neutral]`.

There is no way to clear a display name back to nothing. An assignment with no value at all is discarded while the file is read. `Name=` on its own is the same as leaving the line out: the display name is the section name.
