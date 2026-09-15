---
format_id: shp
title: SHP images
summary: Stores indexed two-dimensional image frames used by sprites and interface graphics.
kind: binary
extensions:
  - .SHP
role: image
source_files:
  - code/shapeset.h
  - code/objtype.cpp
  - code/builtype.cpp
---

An SHP file opens with a short header with the frame count and the logical width and height every frame is placed within. One record per frame follows, then the frame data itself. Each record holds that frame's offset inside the logical box, its own width and height, and the position of its pixels in the file. It also holds one color standing in for the whole frame, so the radar can draw a cell without examining it. Two flags record whether the frame has transparent pixels and whether its pixels are run length encoded.

Non-voxel object art, animations, cursors and interface graphics all use SHP data. Cursors and interface graphics are asked for by fixed names written into the engine. Object art is named from the type's [Image ID](/keys/image/) plus `.SHP`, and two art settings change that name before the file is looked up. [`Theater=yes`](/keys/theater/) replaces the extension with the theater's own, `.TEM` in temperate and `.SNO` in snow, and leaves the rest of the name alone. Failing that, [`NewTheater=yes`](/keys/newtheater/) keeps the `.SHP` extension and rewrites the second letter of the name instead, to the theater's [`ImageLetter`](/keys/imageletter/): `T` in temperate and `A` in snow. A name is rewritten only where that second letter is already the image letter of some declared theater, compared without regard to case. Every other name is left as written even with the setting on, and a type with both settings takes the first.

Whichever name is arrived at is fetched from a cached archive rather than opened as a file, so object art has to be a member of an archive that startup cached. [MIX archives](/formats/mix/) covers which those are. A type whose artwork is not found is left with no image rather than with a placeholder.

For a BuildingType, `Image=` in `art.ini [<Image ID>]` selects the basename of the main SHP. It does not change the building's Image ID or the section used by other building art keys.
