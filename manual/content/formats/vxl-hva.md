---
format_id: vxl-hva
title: VXL and HVA models
summary: Draws an object from a voxel model whose layers are placed by a companion motion file.
kind: binary
extensions:
  - .VXL
  - .HVA
role: model
related:
  - { type: format, id: mix }
  - { type: format, id: save-games }
source_files:
  - code/objtype.cpp
  - code/builtype.cpp
  - code/techtype.cpp
  - code/unit.cpp
  - code/vanimtype.cpp
  - code/voxel.hh
  - code/voxlib.cpp
  - code/motlib.cpp
---

A voxel model is a `.VXL` and a `.HVA` under the same base name, loaded as a pair. The `.VXL` holds the model itself, divided into layers. The `.HVA` holds one transform per layer per frame, and it is what puts each layer where it belongs. One pair is one piece of an object, so a turreted vehicle draws from three of them.

## Which pairs a type loads

An ObjectType marked [`Voxel=yes`](/keys/voxel/) fetches its whole set at once, from base names built out of its [Image ID](/keys/image/). The table sets each base name against the condition under which it is looked for at all. The column to read is the second one. What a type asks for is settled by the kind of type it is and whether it has a turret, never by what its artwork happens to contain. Anything that is not a UnitType is therefore asked for a turret and a barrel whether it could use them or not.

| Base name | Looked for when |
| --- | --- |
| `<Image ID>` | Always; this is the body |
| `<Image ID>TUR` | The type is not a UnitType, or is a UnitType with a turret |
| `<Image ID>BARL` | The same condition as the turret |
| `<Image ID>W` | The type is a UnitType without a turret whose ID is exactly `APC` |

Only the `.VXL` decides whether a piece is looked for at all. Once one is present its `.HVA` is opened whether or not it exists. Within this set, a model with no motion file beside it counts as a piece that failed rather than as a piece without motion. The shape-drawn case below is fetched by a separate routine that records no failure.

A UnitType that is drawn from shapes rather than voxels still picks up `<Image ID>TUR` and `<Image ID>BARL` when it has a turret and its shape artwork loaded. A building takes its voxel turret and barrel from a base name of its own instead of from its Image ID. [`TurretAnimIsVoxel`](/keys/turretanimisvoxel/) and [`VoxelBarrelFile`](/keys/voxelbarrelfile/) cover how that name is arrived at. A voxel animation type loads a `.VXL` alone and no motion file with it. [`ShareSource`](/keys/sharesource/) is the exception: the type borrows another type's body, turret or barrel and takes that pair whole. One further pair, `DPOD.VXL` and `DPOD.HVA`, is loaded once at startup under those fixed names.

Both files are found through the ordinary game file layer, so either can sit in a [MIX archive](/formats/mix/) or loose in the game directory.

:::danger[A voxel barrel with no motion file beside it stops the game when the unit is drawn]
The routine that gives a shape-drawn turreted UnitType its turret and barrel neither records a failure nor releases what it built. A `<Image ID>BARL.VXL` with no `<Image ID>BARL.HVA` beside it therefore leaves the type holding a barrel model paired with a motion library that never loaded. That library reports no frames, and the shape drawing code accepts the pair on the strength of both pointers being set. Asking it for a frame takes the modulus against that count, and the game stops the first time the unit is drawn. A missing `<Image ID>TUR.HVA` does not reach this, because the shape path draws the barrel and not the turret.
:::

## When a set fails

If any piece fails, the body and turret pieces are released and the type is left with no voxel artwork, whatever else loaded successfully. A piece fails when the body model is missing, or when a model or motion file that is present refuses to load. A barrel that loaded is not released with them.

Where the whole set loads, the largest X, Y or Z size among the body model's layers becomes the size the type reports for its artwork. A size below 8 is raised to 8.

Voxel artwork is not stored in a save game; [save games](/formats/save-games/) covers what a restored type is given in its place.

## What the model file holds

The loader reads a `.VXL` in this order:

- a header, holding an internal name, the number of palettes, the number of layers, the number of layer info records and the size of the voxel body;
- one palette per the header's count, each a pair of remap bounds followed by a red, a green and a blue byte for each of 256 colors;
- one header per layer, holding a name and the index of the first info record belonging to that layer;
- the voxel body, run length encoded down each column, with a color index and a normal index for every voxel in a run;
- one record per layer info, holding the offsets within the body of its two span tables and of its voxel data, the scale the layer was built at, a placement transform, two opposite corners of its bounding box, its X, Y and Z sizes in voxels, and which normal table its voxels index.

A layer names its first info record and may own several that follow it, so the layer count and the info count need not agree.

Several of those parts never reach the routines that draw the model. The palettes are stepped over rather than loaded: no caller asks for the file's own colors, so a model is always drawn from `VOXELS.VPL`, which startup reads separately. The palette count still matters, because it is what sizes the step. The internal names in the header and in each layer header are never read: files are found by filename, and a layer is paired with its motion by position. The placement transform is converted into the engine's own matrix layout as it is read and then left alone, so a layer's position comes entirely from the motion file.

A normal table selector of zero marks a layer as having no usable normals, and it is drawn without lighting. Values 1 through 4 select one of the four normal tables the engine holds. Nothing rejects a larger one. The selector is an unvalidated byte from the file, and the drawing pass tests only whether it is zero. The routine that builds the lighting lookup indexes both its table of normal sets and its table of set sizes with it. A value above four therefore reads past the end of each.

## What the motion file holds

The loader reads a `.HVA` as a header holding an internal name, the number of frames and the number of layers. A table of layer names follows, sixteen bytes each and one per layer, and it is stepped over. Then, frame by frame, one transform of twelve floating point values is read for each layer. As the pair is loaded, the translation of every one of those transforms is multiplied by the scale recorded in the model's first layer info. That brings the animation into the model's scale.

Frames are asked for modulo the frame count, so an animation loops rather than running off its end. A file that declares no frames at all still loads. The modulus against a count of zero is what stops the game the first time the model is drawn.

:::danger[An empty model or motion file is loaded from uninitialized memory]
The counts of layers, layer infos and frames are taken from the start of each file without testing that any bytes were read. An empty file is the plainest case. Its counts come from whatever that memory last held, and the tables the loader allocates from them are as large as those counts demand. Allocating for an implausible count is not survivable; where the count is small enough to allocate, the first read the file cannot satisfy abandons the load.
:::

:::danger[A model with no first layer is read all the same]
Loading a pair scales the motion by the scale in the model's first layer info, and nothing checks that the model has one. A `.VXL` that is present but refuses to load, beside a `.HVA` that loads, is read for that scale through a layer table the failed load has already released. The game stops as the type is read. A model that loads but declares no layers is read past the end of its own empty table instead, and whatever figure comes back scales the entire animation.
:::
