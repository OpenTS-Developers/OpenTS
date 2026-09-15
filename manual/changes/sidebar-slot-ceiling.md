---
title: Stop a tall picture from laying out more sidebar rows than a strip holds
category: fix
release: 0.1.0
targets:
- type: system
  id: sidebar
  effect: changed
credit: [ZivDero]
---

Each sidebar strip now has 60 cameo slots rather than 20, and the number of rows a strip lays out is held to that figure. How many rows fit follows from the height of the sidebar picture the game renders. Nothing kept that count within the slots a strip has. A rendering resolution tall enough for more than 20 rows in a strip laid rows out past the end of the array holding them and wrote over whatever followed.

The sidebar backdrop is built from the same count. A picture tall enough for more than 60 rows therefore ends its sidebar art below the sixtieth row rather than at the foot of the screen.
