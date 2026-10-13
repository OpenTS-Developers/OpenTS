---
title: Battlefield zoom
summary: "The mouse wheel over the tactical view magnifies the battlefield by a whole factor from 1x to 4x while the sidebar and the tab strip keep their size."
category: interface-controls
keys: []
related:
  - type: system
    id: sidebar
---

Each wheel notch over the tactical view zooms in one step, away from the player, or out one step, toward the player. A zoomed view shows a smaller part of the map, with every map pixel drawn as a square of 2, 3 or 4 screen pixels. Over the sidebar or the tab strip the wheel scrolls the sidebar.

## Available factors

Factors above 1x are offered only while the part of the map they show is at least 472 pixels wide and 524 pixels high. The 1x view remains available at every valid size. With the sidebar and the tab strip at the automatic [`UIScale`](/keys/uiscale/) size, that gives:

| Game resolution | Largest factor |
| --- | --- |
| 1280 by 720 | 1x |
| 1920 by 1080 | 2x |
| 2560 by 1440 | 2x |
| 3840 by 2160 | 4x |

A wheel step past the largest factor is ignored. Rows and columns that a whole factor cannot fill are black and split evenly around the picture.

## When the zoom changes

The new factor takes effect on the next frame with no key press waiting and no mouse button held. The zoom change keeps the map point at the last wheel notch's pointer position in place, unless that would move the view past the edge of the map.

The wheel does not zoom while the window is in the background, while a trigger or the end of a campaign mission locks the player's input, or while a trigger zoom is in effect. Any waiting wheel change is canceled in these cases. A [Zoom in](/mapping/actions/taction-zoom-in/) trigger action first returns the battlefield to 1x.

## What stays at screen size

The sidebar, the tab strip with its mission timer, and chat and game messages are drawn at their normal size over a zoomed battlefield. Tooltips over the battlefield are magnified with it. The radar's view box shrinks to the part of the map on screen.

A drag that starts on the battlefield stays on it: a selection box or a coast scroll stops at the edge of the view when the pointer moves over the sidebar. Edge scrolling works on all four screen edges at every factor.

## Persistence

The factor belongs to the local display. It is not stored in saved games or sent to other players. A mission or a loaded game starts at the [`UIScale`](/keys/uiscale/) size, lowered to the largest factor available, so at 3840 by 2160 with the automatic size it starts at 3x. This happens even when the mission opens with the player's input locked. A game saved while zoomed loads with the view centred where the zoomed view was. The game resolution can only be changed from the main menu, where the factor is always 1x.
