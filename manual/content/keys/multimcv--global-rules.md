---
key: MultiMCV
scope: global-rules
label: Any yard builds for its owner
summary: Lets a construction yard produce for every country in a structure's Owner list, not only for the country it was built by.
see_also: [BuildConst, BaseUnit, ActsLike, "system:production"]
when_omitted:
  kind: value
  value: "no"
---

```ini title="rules.ini"
[General]
MultiMCV=yes
```

A construction yard normally produces only the structures whose [`Owner`](/keys/owner/) list includes the country the yard [acts as](/keys/actslike/), which is what lets a captured yard keep building for the country it was built by. With `yes` that test is skipped in the sidebar and in the [factory search](/systems/production/#what-counts-as-a-factory) alike, and the yard's own type's `Owner` overlap with the product is the only ownership gate left. A rules set that gives each faction its own MCV needs it once an MCV can come out of a captured foreign factory: that MCV deploys into a yard acting for its captor, which could otherwise build nothing of its own type's tree.

The key is shared with Vinifera, where it carries the same meaning and default.
