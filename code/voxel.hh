/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include "matrix3d.h"
#include "vector3.h"


/*
 * Voxel file identifier.
 */
#define VOXEL_ANIM_ID "Voxel Animation"

/*
 * Max internal name length.
 */
#define MAX_VOXEL_NAME_LENGTH 16 /// Fixed by the file format; do not change.

/*
 * Size of the voxel palette, 256 colors.
 */
#define VOXEL_PALETTE_SIZE 256

enum VoxelBoundsEnum {
	VOXEL_BOUNDS_BFR = 0, // Bottom Front Right (+X +Y -Z)
	VOXEL_BOUNDS_BBR = 1, // Bottom Back  Right (+X -Y -Z)
	VOXEL_BOUNDS_BBL = 2, // Bottom Back  Left  (-X -Y -Z)
	VOXEL_BOUNDS_BFL = 3, // Bottom Front Left  (-X +Y -Z)
	VOXEL_BOUNDS_TFR = 4, // Top    Front Right (+X +Y +Z)
	VOXEL_BOUNDS_TBR = 5, // Top    Back  Right (+X -Y +Z)
	VOXEL_BOUNDS_TBL = 6, // Top    Back  Left  (-X -Y +Z)
	VOXEL_BOUNDS_TFL = 7, // Top    Front Left  (-X +Y +Z)
	VOXEL_BOUNDS_COUNT
};

/*
 * Number of palette adjustment tables (game engine supports max 32 facings).
 */
#define MAX_PALETTE_LOOKUP_ENTRIES 32

#define VOXEL_PALETTE_LOOKUP_NEUTRAL (MAX_PALETTE_LOOKUP_ENTRIES / 2)

/*
 * Voxel file header (VXL).
 */
struct VoxelHeaderStruct
{
	/*
	 * This is the internal name of the voxel object, recorded when the file was built. The
	 * game locates voxels by file name, so this is not consulted.
	 */
	char Name[MAX_VOXEL_NAME_LENGTH];

	/*
	 * This is the number of palettes stored in the file. Only the first is used, and any
	 * others are skipped over.
	 */
	unsigned int PaletteCount;

	/*
	 * This is the number of layer headers that follow the palettes. A layer is one
	 * independently placed piece of the object, such as a tank's turret or its barrel.
	 */
	int LayerCount;

	/*
	 * This is the number of layer info records at the tail of the file. A layer names the
	 * first record that belongs to it and may own several, so the two counts need not match.
	 */
	int LayerInfoCount;

	/*
	 * This is the size, in bytes, of the voxel body data sitting between the layer headers
	 * and the layer info records.
	 */
	int DataSize;
};
static_assert(sizeof(VoxelHeaderStruct) == 32, "the VXL header before the remap bytes and palette is 32 bytes on disk");


/*
 * Voxel layer header.
 */
struct VoxelLayerHeaderStruct
{
	/*
	 * This is the name of the layer. Layers are paired with the object's animation by
	 * position rather than by name, so this is not consulted.
	 */
	char Name[MAX_VOXEL_NAME_LENGTH];

	/*
	 * This is the index of the first layer info record belonging to this layer. Any
	 * further records for the layer follow it in sequence.
	 */
	int InfoIndex;

	/// Unused
	int Unused1;
	unsigned char Unused2;
};
static_assert(sizeof(VoxelLayerHeaderStruct) == 28, "the VXL section header is 28 bytes on disk; the last byte is padded out to a word");


/*
 * Voxel layer info header.
 */
struct VoxelLayerInfoStruct
{
	/*
	 * This is the offset, from the start of the voxel body, of this layer's table of span
	 * offsets. It holds one entry per column of the layer's X by Y footprint, and a column
	 * that contains no voxels is marked with -1.
	 */
	int StartOffset;

	/*
	 * This is the offset of the matching span table used when the viewing orientation
	 * walks the layer in reverse. The shadow drawer reads it alone, only to learn which
	 * columns are occupied.
	 */
	int EndOffset;

	/*
	 * This is the offset, from the start of the voxel body, of the run length encoded
	 * voxel data itself. The span tables give each column's position within it.
	 */
	int DataOffset;

	/*
	 * This is the scale the layer was built at. It is applied to the translation of every
	 * matrix in the object's animation, bringing the animation into voxel scale.
	 */
	float Scale;

	/*
	 * This is the layer's placement transform, a 3 by 4 matrix stored row by row. The
	 * animation supplies a matrix per layer per frame, so the drawer works from that.
	 */
	float Transform[12];

	/*
	 * These are the two opposite corners of the layer's bounding box. Loading expands them
	 * into the eight corners that the drawer projects onto the screen.
	 */
	Vector3 MinBounds;
	Vector3 MaxBounds;

	/*
	 * These are the dimensions of the layer, measured in voxels.
	 */
	unsigned char XSize;
	unsigned char YSize;
	unsigned char ZSize;

	/*
	 * This specifies which of the normal tables the layer's voxels index (1 - 4). If it is
	 * zero, then the layer carries no usable normals and is drawn without lighting.
	 */
	unsigned char NormalType;
};
static_assert(sizeof(VoxelLayerInfoStruct) == 92, "the VXL section tailer is 92 bytes on disk");


/*
 * Voxel anim header (HVA: Hierarchical Voxel Animation).
 */
struct VoxelAnimFileHeaderStruct
{
	/*
	 * This is the internal name of the animation, recorded when the file was built. The
	 * game locates animations by file name, so this is not consulted.
	 */
	char Name[MAX_VOXEL_NAME_LENGTH];

	/*
	 * This is the number of frames in the animation. The transformation matrices are
	 * stored frame by frame, one per layer within each frame.
	 */
	int FrameCount;

	/*
	 * This is the number of layers the animation drives. It sizes both the table of layer
	 * names that follows the header and each frame's run of matrices.
	 */
	int LayerCount;
};
static_assert(sizeof(VoxelAnimFileHeaderStruct) == 24, "the HVA header is 24 bytes on disk");
