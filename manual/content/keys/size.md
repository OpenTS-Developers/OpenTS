---
key: Size
summary: How much room an object takes up inside a transport, or on a scenario the width and height of the playfield.
---

The playfield is the diamond of cells that the scenario assignment `[Map] Size=x,y,w,h` declares. It is every cell the map has, and nothing can stand, path, or be revealed outside it. The playfield is a diamond on the cell grid rather than a rectangle, and the upright map rectangle that encloses it measures `width + height - 1` cells on a side.

Inside it sits the playable area, the smaller diamond that [`LocalSize=`](/keys/localsize/) declares. A player sees, scrolls across, and plays in that region alone. The engine clips the playable area into the playfield and pulls it in from the playfield's edges, and the ring of playfield left around it is the map border.
