---
title: Fix structure sales with unusable survivor types
category: fix
release: 0.3.0
targets:
- type: system
  id: capture
  effect: changed
credit: [ZivDero]
---

Selling a structure could hang while picking a second engineer or crash when a survivor type was unset. The sale now skips that survivor when repeated picks still return engineers or when the type is unset.
