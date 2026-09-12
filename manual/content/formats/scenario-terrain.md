---
format_id: scenario-terrain
title: Scenario terrain data
summary: Stores a scenario's terrain cells in the Base64-encoded IsoMapPack sections.
kind: file
filenames:
  - "*.MAP"
  - "*.MPR"
source_files:
  - code/display.cpp
  - code/map.cpp
  - code/ini.cpp
  - code/lcwpipe.cpp
  - code/lcwstraw.cpp
  - code/lzopipe.cpp
  - code/lzostraw.cpp
  - code/overlay.cpp
  - code/session.cpp
related:
  - type: format
    id: ini-syntax
---

Scenario map files are INI files, but their terrain cells are stored as binary data. The binary stream is Base64-encoded and split across the numbered assignments of an `[IsoMapPack]` section. The later revisions use `[IsoMapPack2]` through `[IsoMapPack5]` instead.

The loader checks all five sections in revision order. A missing section has no effect. If more than one revision is present, each one that decodes is applied, so a later section may replace terrain loaded by an earlier one. The current map writer clears all five sections and writes only `[IsoMapPack5]`.

A rejected `[IsoMapPack4]` or `[IsoMapPack5]` section stops the load. The loader names it in the debug log and reports a load error to the player. Loading another map after this failure uses that map's theater resources.

## Revisions

| Section | Compression | Cell data |
| --- | --- | --- |
| `[IsoMapPack]` | LCW | Tile type, sub-tile and height for the original 128 by 128 cell grid |
| `[IsoMapPack2]` | LCW | Sparse cell records terminated by `CELL_NONE` |
| `[IsoMapPack3]` | None | Sparse cell records terminated by `CELL_NONE` |
| `[IsoMapPack4]` | LZO | Sparse cell records terminated by `CELL_NONE` |
| `[IsoMapPack5]` | LZO | Version 4 fields plus the cell's ice-growth flag |

An IsoMapPack5 cell record carries the cell coordinate, isometric tile type, sub-tile, height and ice-growth flag. LZO divides that record stream into blocks of at most 8 KiB; each block begins with the compressed and uncompressed byte counts. The final `CELL_NONE` coordinate is not followed by the rest of a cell record.

IsoMapPack5 accepts at most the LZO block storage required for one record per map cell and the terminating `CELL_NONE`. A section that reaches beyond that bound is damaged. The readers for revisions 1 through 4 each accept up to 512,000 Base64-decoded bytes.

An LZO pack is damaged when a block does not decompress, when it expands to a length other than its header claims, when the compressed bytes run out mid-block, or when the records end before the terminating `CELL_NONE`. The last of those catches a pack cut on a block boundary, where every surviving block still decompresses cleanly.

An LCW pack is a sequence of blocks that each begin with their compressed and uncompressed byte counts and expand to at most 8 KiB. A block whose header claims more output than that, or more compressed bytes than the compressor can produce from one block, ends the read there, and the cells after it take nothing from the pack.

## Overlay packs

`[OverlayPack]` and `[OverlayDataPack]` store the overlay type and overlay frame of every cell of the original 128 by 128 grid, one byte per cell in row order, in the same Base64 and LCW block form as `[IsoMapPack]`. The loader opens them only when [`NewINIFormat`](/keys/newiniformat/) is above 1, and the map writer always emits both.
