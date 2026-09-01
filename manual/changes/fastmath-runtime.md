---
title: Replace the fastmath lookup tables
category: internal
release: 0.1.0
breaking: true
migration:
- Finish or abandon replays and mid-mission saves recorded with an earlier build. Continuing one under this build may diverge.
- Check that every peer in a network game runs the same OpenTS version.
targets: []
credit: [ZivDero]
---

OpenTS now uses the C runtime trigonometric and square-root functions in place
of the inherited fastmath lookup tables. Save and packet formats are unchanged,
but numerical results may differ.
