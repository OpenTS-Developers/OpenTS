---
title: Keep the pixel-art scaled frame upright on OpenGL
category: fix
release: 0.2.0
targets: []
credit: [EJ]
---

The picture is no longer vertically flipped during fullscreen or scaled windowed play on
OpenGL. The pixel-art filter magnifies the frame through an intermediate render target
whenever the window is larger than the frame by a non-integer amount, and on backends
that store render targets bottom-up that target is now sampled flipped on the way to the
screen. Other backends, filters, and window sizes already presented upright and are
unchanged.
