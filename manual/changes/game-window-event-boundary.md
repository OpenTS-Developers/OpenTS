---
title: Separate game window responses from native events
category: internal
release: 0.2.0
targets: []
credit: [Krisztiaan]
---

The game's responses to its window live apart from the code that reads the window's events: repainting, stopping scroll coasting when the right button is released, and stepping the sidebar with the mouse wheel. The video presenter takes refresh-rate updates directly instead of handling a native display-change event. Controls and rendering configuration are unchanged.
