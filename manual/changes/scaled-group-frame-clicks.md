---
title: Deliver clicks through group frames when the picture is scaled
category: fix
release: 0.2.0
targets: []
credit:
- ZivDero
---

With the picture scaled into the window, a control standing inside a dialog's group frame — the thin box drawn around a set of related options — ignored clicks: the game delivers every mouse message itself while scaling and handed each one to the topmost window over the position, which is the frame. In the skirmish lobby this left the Game Speed slider and the Re-Deployable MCV, Short Game and Multi Engineer checkboxes dead to the mouse whenever the window was scaled, while the controls around them kept working. Clicks now reach the control the player sees, at any window size.
