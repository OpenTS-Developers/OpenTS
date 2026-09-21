---
key: RecheckPrerequisites
scope: global-rules
label: Re-check what the sidebar offers
summary: Removes a type's sidebar cameo and cancels its production once the house no longer meets the type's prerequisites, tech level or ownership rules.
see_also: [Prerequisite, BuildLimit, "system:sidebar", "system:production"]
when_omitted:
  kind: value
  value: "no"
---

```ini title="rules.ini"
[General]
RecheckPrerequisites=yes
```

With `RecheckPrerequisites=yes`, [the sidebar check](/systems/sidebar/#what-removes-a-cameo) also removes a cameo whose type fails the tech-level, prerequisite or ownership gate of [what a house may build](/systems/production/#what-a-house-may-build). Selling or losing a structure named in the type's [`Prerequisite=`](/keys/prerequisite/) therefore removes the cameo, and the cameo returns once the house can build the type again. With `no`, the check ignores those gates, so the cameo stays and can still start production. Computer houses have no sidebar and are not affected.

Removing the cameo also cancels that type's production. The item under construction and every queued copy are canceled, the credits spent so far are refunded, and a finished structure waiting to be placed is taken off the cursor.

The ownership gate needs a switched-on construction yard unless [`MultiMCV=yes`](/keys/multimcv/). A construction yard whose type drains power or sets [`Powered=yes`](/keys/powered/) can be [switched off](/keys/togglepower/), and switching off every one removes all structure cameos and cancels structure production. Low power does not switch a structure off.

Vinifera reads the same key with the same meaning and default.
