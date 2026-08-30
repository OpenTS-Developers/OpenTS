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
#include "rgb.h"
#include "sun.h"
#include "vector3.h"
#include "vector3i.h"
#include "win.h"

#include "voxel.hh"

class Surface;
class BSurface;
class FileClass;
class VoxelLibrary;
struct SurfaceRegion;
class Vector3i16;
template<class T> class TRect;
typedef TRect<int> Rect;


#define VOXEL_BITMAP_WIDTH 256
#define VOXEL_BITMAP_HEIGHT 256
#define VOXEL_BITMAP_BPP 1


namespace VoxelDrawSystem
{
	extern BOOL EnableLighting;
	extern BOOL EnableZBuffer;

	int Load_VPL_File(FileClass & file);

	inline void Enable_Lighting(void) { EnableLighting = true; }
	inline void Disable_Lighting(void) { EnableLighting = false; }
	inline void Enable_ZBuffer(void) { EnableZBuffer = true; }
	inline void Disable_ZBuffer(void) { EnableZBuffer = false; }

	void Init_256_Array(void);
	void Convert_Voxel_Colors(int red_left, int red_right, int green_left, int green_right, int blue_left, int blue_right);

	unsigned char * Get_Surface_Buffer(void);
	Surface * Get_Surface(void);

	void Precalculate_Light(VoxelLibrary * voxlib, int layer, int info, const Matrix3D & light_transform, Vector3 const & light);
	void Precalculate_Light(VoxelLibrary * voxlib, int layer, int info, const Matrix3D & light_transform, Matrix3D const & view_transform, Vector3 const & light, float specular_strength);

	void Reset(void);

	void Prep_For_Shadow(VoxelLibrary * voxlib, int layer, int info, Matrix3D const & camera, Matrix3D const & motion, Vector3 const & light);
	void Prep_For_Object(VoxelLibrary * voxlib, int layer, int info, Matrix3D const & transform);

	void Render(Rect & rect, int & x, int & y);
	SurfaceRegion Render(void);

	void Clear_Buffer(void);

	void Clear_Z_Buffer(void);
};


/*
 * This structure contains pre-processed data for a drawing a voxel object.
 */
struct VoxelRenderStruct {
	VoxelLibrary * VoxLib;					/// The voxel library to use.
	int Layer;								/// The layer to use.
	int Info;								/// The layer info to use.
	int AnchorCornerIndex;					/// Which corner is the anchor corner, determines the orientation.
	Vector3 MinBounds;						/// The minimum bounds of the voxel model.
	Vector3 MaxBounds;						/// The maximum bounds of the voxel model.
	Vector3 BoxCorner[VOXEL_BOUNDS_COUNT];	/// The 8 corners of the voxel model.
};


/*
 * This structure contains pre-processed data for a drawing a voxel object's shadow.
 */
struct VoxelShadowRenderStruct {
	/*
	 * These identify the voxel library, and the layer and layer info within it, whose
	 * silhouette is to be cast.
	 */
	VoxelLibrary * VoxLib;
	int Layer;
	int Info;

	/*
	 * These are the corners of the layer's bounding box after it has been flattened onto
	 * the ground and slid along the light vector. They span the parallelogram that the
	 * layer's silhouette is smeared across to make the shadow.
	 */
	Vector3 ShadowCorner[4];
};


/*
 * You can see at most 3 faces of a cube at a time.
 * This structure defines the parameters of an orientation of the cube.
 */
struct VoxelRenderOrientation {
	int Reversed;		/// Whether to reverse the voxel data traversal (0 = normal, 1 = reversed).
	int Corner0;		/// Anchor corner (starting point of the bounding box)
	int CornerX;		/// Corner defining the X axis direction
	int CornerY;		/// Corner defining the Y axis direction
	int CornerZ;		/// Corner defining the Z/depth axis
	int ZIndexFactor;	/// Multiplier used in computing RLE start index for Z offset
	int YIndexFactor;	/// Multiplier used in computing RLE start index for Y offset
	int XIndexStride;	/// How much to add to the RLE index per step in X
	int YIndexStride;	/// How much to add to the RLE index per step in Y (pre-scaled by XSize)
};

extern VoxelRenderOrientation VoxelRenderOrientations[VOXEL_BOUNDS_COUNT];


/*
 * Struct used to pass data to the low-level voxel drawing functions.
 * WARNING: If you modify this struct you must update the struct in WINASM.ASM!
 */
struct VoxelFuncArgumentStruct {
	/*
	 * These are copies of the layer's two span tables and of its run length encoded voxel
	 * data, taken so that the low level drawers need no access to the library itself. A
	 * drawer reads whichever span table matches the direction it walks the layer in.
	 */
	unsigned char * StartOffset;
	unsigned char * EndOffset;
	unsigned char * DataOffset;

	/*
	 * This is the index into the span table of the column the walk begins at, chosen from
	 * the anchor corner so that the layer is drawn back to front. The drawers advance it as
	 * they go, so it names the current column once drawing is under way.
	 */
	int StartIndex;

	/*
	 * These are the amounts added to StartIndex to reach the next column and the next row,
	 * signed to suit the direction the anchor corner sends the walk in.
	 */
	int StrideX;
	int StrideY;

	/*
	 * This is the projection the drawers work in. The first entry is the screen position of
	 * the anchor corner and the other three are the steps taken per voxel along X, Y and Z,
	 * all expressed as 8.8 fixed point so that the walk can be stepped in integers.
	 */
	Vector3i16 TransformMatrix[4];

	/*
	 * These are the dimensions of the layer being drawn, measured in voxels.
	 */
	unsigned char XSize;
	unsigned char YSize;
	unsigned char ZSize;
};
extern "C" {
extern unsigned char VoxelPaletteTranslateTable[MAX_PALETTE_LOOKUP_ENTRIES][VOXEL_PALETTE_SIZE];
extern unsigned char VoxelDrawBuffer[];
extern unsigned char VoxelDrawZBuffer[];
}

extern RGBStruct VoxelRGBColors[VOXEL_PALETTE_SIZE];

extern BSurface VoxelSurface;

extern BSurface VoxelZSurface;
