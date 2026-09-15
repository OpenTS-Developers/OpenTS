---
key: Wake
summary: The ripple laid on the water by something moving across it, falling into it or sinking through it.
see_also: [SplashList, IceBreakingWeight, Locomotor]
when_omitted:
  kind: value
  value: none
---

```ini title="rules.ini"
[AudioVisual]
Wake=MYWAKE ; an AnimType registered in [Animations]
```

Three families of path create it, and only one of them checks that an animation was named first.

**Movement.** An object whose [Locomotor](/keys/locomotor/) is Drive, Hover or Levitate lays one at its own position every tenth frame while it is moving over open water and is not riding a bridge. All three of these read the setting through a guard, so a mod that leaves the key unset simply produces no trail.

**Impacts.** [`SplashList`](/keys/splashlist/) covers which of the paths that make a splash lay a ripple down beside it and which do not.

**Ice giving way.** When a heavy vehicle breaks through ice, everything standing on the break cell and the cells beside it gets one. A vehicle that is not amphibious is stunned and starts to sink under it. An aircraft or infantryman is deleted and leaves it behind.

:::danger[An unset animation crashes the game when a splash lays a ripple or ice gives way]
Only the three movement paths check the setting before using it. The impact and ice paths create the animation outright. With the key unset, the game crashes the first time one of the splashes that lays a ripple happens, or the first time ice gives way under a heavy vehicle.
:::
