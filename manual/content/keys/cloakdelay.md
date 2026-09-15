---
key: CloakDelay
summary: The time in game minutes an object must wait after it is uncloaked before it may hide again.
see_also: ["system:cloaking"]
when_omitted:
  kind: value
  value: "0"
---

```ini title="rules.ini"
[General]
CloakDelay=.02
```

The countdown is loaded onto the object every time an uncloak is applied to it, and loaded once more when [the fade back in](/systems/cloaking/#the-four-states) completes. After an actual reveal, the wait runs from the moment the object is fully visible. While it is running the object is refused the start of a new cloak. At the engine default it expires on the frame it is loaded, so an object begins hiding again as soon as whatever revealed it stops applying.

:::caution[Damage restarts the wait on objects that were never hidden]
Damage that does not destroy an object runs the uncloak path whatever state the object was in, and that path loads the countdown before it looks at the state. Anything under fire therefore has a running lockout, so a non-zero delay is measured from the last hit rather than from the moment the object was revealed.
:::
