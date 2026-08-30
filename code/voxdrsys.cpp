/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "always.h"

#include "voxdrsys.h"

#include "_voxel.h"
#include "bsurface.h"
#include "stbuffer.h"
#include "voxlib.h"
#include "wwfile.h"

BOOL VoxelDrawSystem::EnableLighting;
BOOL VoxelDrawSystem::EnableZBuffer;

unsigned char VoxelDrawBuffer[VOXEL_BITMAP_WIDTH * VOXEL_BITMAP_HEIGHT * VOXEL_BITMAP_BPP];
BSurface VoxelSurface(VOXEL_BITMAP_WIDTH, VOXEL_BITMAP_HEIGHT, VOXEL_BITMAP_BPP, VoxelDrawBuffer);

unsigned char VoxelDrawZBuffer[VOXEL_BITMAP_WIDTH * VOXEL_BITMAP_HEIGHT * VOXEL_BITMAP_BPP];
BSurface VoxelZSurface(VOXEL_BITMAP_WIDTH, VOXEL_BITMAP_HEIGHT, VOXEL_BITMAP_BPP, VoxelDrawZBuffer);

Vector3 MinVoxelBounds;
Vector3 MaxVoxelBounds;

VoxelRenderStruct VoxelRenderData[64];
VoxelShadowRenderStruct VoxelShadowRenderData[64];

int VoxelRenderDataCount;
int VoxelShadowRenderDataCount;

RGBStruct VoxelRGBColors[VOXEL_PALETTE_SIZE];
unsigned short Voxel16BitColors[VOXEL_PALETTE_SIZE];
unsigned char VoxelPaletteTranslateTable[MAX_PALETTE_LOOKUP_ENTRIES][VOXEL_PALETTE_SIZE];

int VPLRemapStart;
int VPLRemapEnd;
int VPLLUTCount;
int VPLUnused;

unsigned char Voxel256Array[VOXEL_PALETTE_SIZE];


/// <summary>
/// Loads the voxel palette library from a file.
/// This routine is called at startup to read VOXELS.VPL, which supplies both the palette
/// every voxel model is colored from and the lookup tables the drawing routines shade
/// with.
/// </summary>
/// <param name="file">The palette library file to read.</param>
/// <returns>Returns with zero if the library was read, or one if the file let us down.</returns>
int VoxelDrawSystem::Load_VPL_File(FileClass & file)
{
	VoxelPaletteLibrary vpl(VoxelRGBColors, VoxelPaletteTranslateTable);
	int result = vpl.Read_File(file);
	VPLRemapStart = vpl.Header.RemapStart;
	VPLRemapEnd = vpl.Header.RemapEnd;
	VPLLUTCount = vpl.Header.LUTCount;
	VPLUnused = vpl.Header.Unused;
	return(result);
}


/// <summary>
/// Builds the identity voxel remap ramp.
/// This routine fills the 256 entry table that can be handed to the voxel drawing routines
/// when no remapping at all is wanted, since every index maps to itself.
/// </summary>
void VoxelDrawSystem::Init_256_Array(void)
{
	for (int i = 0; i < (int)sizeof(Voxel256Array); i++) {
		Voxel256Array[i] = i;
	}
}


/// <summary>
/// Converts the voxel palette into hicolor pixels.
/// This routine is called once the palette library has been loaded, to build the table of
/// screen ready colors that the voxel drawing routines write through. The shift pairs
/// describe where each component sits in a destination pixel.
/// </summary>
/// <param name="red_left">The count of low bits to discard from the red component.</param>
/// <param name="red_right">The bit position of red within the destination pixel.</param>
/// <param name="green_left">The count of low bits to discard from the green component.</param>
/// <param name="green_right">The bit position of green within the destination pixel.</param>
/// <param name="blue_left">The count of low bits to discard from the blue component.</param>
/// <param name="blue_right">The bit position of blue within the destination pixel.</param>
void VoxelDrawSystem::Convert_Voxel_Colors(int red_left, int red_right, int green_left, int green_right, int blue_left, int blue_right)
{
	for (unsigned i = 0; i < VOXEL_PALETTE_SIZE; i++) {
		unsigned char red = VoxelRGBColors[i].Red;
		unsigned char green = VoxelRGBColors[i].Green;
		unsigned char blue = VoxelRGBColors[i].Blue;
		Voxel16BitColors[i] = (((red >> red_left) << red_right) | ((green >> green_left) << green_right) | ((blue >> blue_left) << blue_right));
	}
}


/// <summary>
/// Fetches the raw pixel memory of the voxel drawing surface.
/// This is the very memory the drawing surface is built over, handed out for code that
/// would rather touch the pixels itself than go through the surface.
/// </summary>
unsigned char *VoxelDrawSystem::Get_Surface_Buffer(void)
{
	return(VoxelDrawBuffer);
}


/// <summary>
/// Fetches the surface that voxels are drawn into.
/// Callers blit out of this surface once Render has composed the object onto it.
/// </summary>
Surface * VoxelDrawSystem::Get_Surface(void)
{
	return(&VoxelSurface);
}


/// <summary>
/// Precalculates the voxel shading table.
/// This routine builds the lookup that the drawing routines shade each voxel through,
/// indexed by the model's normal set. The light is rotated out of the transform it was
/// given in, so the table holds for the object rather than for the world.
/// </summary>
/// <param name="info">The layer info variant whose normal set the table is built for.</param>
/// <param name="light_transform">The transform the light direction is expressed in.</param>
/// <remarks>Call this routine before preparing the objects; every object drawn in the frame
/// shades through the one table it builds.</remarks>
void VoxelDrawSystem::Precalculate_Light(VoxelLibrary * voxlib, int layer, int info, Matrix3D const & light_transform, Vector3 const & light)
{
	Matrix3D light_transform_inv = Matrix3D::Orthogonal_Inverse(light_transform);
	Vector3 light_tr = light_transform_inv.Rotate_Vector(light);
	int normal_type = voxlib->Get_Layer_Info(layer, info).NormalType;
	Precalculate_Normal_Lookup(light_tr, normal_type);
}


/// <summary>
/// Precalculates the voxel shading table, with a specular highlight.
/// This routine builds the lookup that the drawing routines shade each voxel through,
/// indexed by the model's normal set. Use this overload for a model that should catch a
/// highlight from the viewer's direction as well as the plain diffuse light.
/// </summary>
/// <param name="info">The layer info variant whose normal set the table is built for.</param>
/// <param name="light_transform">The transform the light direction is expressed in.</param>
/// <param name="view_transform">The transform the viewer direction is expressed in.</param>
/// <param name="specular_strength">How hot the specular highlight should burn.</param>
/// <remarks>Call this routine before preparing the objects; every object drawn in the frame
/// shades through the one table it builds.</remarks>
void VoxelDrawSystem::Precalculate_Light(VoxelLibrary * voxlib, int layer, int info, Matrix3D const & light_transform, Matrix3D const & view_transform, Vector3 const & light, float specular_strength)
{
	Matrix3D light_transform_inv = Matrix3D::Orthogonal_Inverse(light_transform);
	Matrix3D view_transform_inv = Matrix3D::Orthogonal_Inverse(view_transform);
	Vector3 light_tr = light_transform_inv.Rotate_Vector(light);
	Vector3 viewer_tr = view_transform_inv.Rotate_Vector(Vector3(0, 0, 1));
	int normal_type = voxlib->Get_Layer_Info(layer, info).NormalType;
	Precalculate_Normal_Lookup(light_tr, viewer_tr, specular_strength, normal_type);
}


/// <summary>
/// Clears the voxel buffers ready for a fresh draw.
/// This routine throws away whatever was prepared for the previous draw and erases the
/// drawing and depth buffers behind it. It opens every voxel drawing sequence.
/// </summary>
/// <remarks>Forget this routine and the previous object is still sitting in the buffer.</remarks>
void VoxelDrawSystem::Reset(void)
{
	VoxelRenderDataCount = 0;
	VoxelShadowRenderDataCount = 0;

	/* Cached images must never retain a pixel outside a previous render's bounds. */
	Clear_Buffer();
	if (VoxelDrawSystem::EnableZBuffer) {
		Clear_Z_Buffer();
	}

	MinVoxelBounds = Vector3(10000, 10000, 10000);
	MaxVoxelBounds = Vector3(-10000, -10000, -10000);
}


/// <summary>
/// Prepares one voxel object's shadow for the next render.
/// This routine flattens the model's bounding box onto the ground and slides it along the
/// light vector, which gives the render list the quadrilateral to smear the shadow across.
/// </summary>
/// <param name="info">The layer info variant of the model to cast the shadow from.</param>
/// <param name="camera">The camera transform to view the flattened shadow through.</param>
/// <param name="motion">The object's own transform.</param>
/// <param name="light">The direction the shadow is thrown in.</param>
/// <remarks>Reset must be called before the first shadow of a frame is prepared.</remarks>
void VoxelDrawSystem::Prep_For_Shadow(VoxelLibrary * voxlib, int layer, int info, Matrix3D const & camera, Matrix3D const & motion, Vector3 const & light)
{
	VoxelLibrary::LayerInfoStruct const & layer_info = voxlib->Get_Layer_Info(layer, info);

	VoxelShadowRenderStruct & data = VoxelShadowRenderData[VoxelShadowRenderDataCount];

	data.VoxLib = voxlib;
	data.Layer = layer;
	data.Info = info;

	for (int i = 0; i < 4; i++) {
		data.ShadowCorner[i] = motion * layer_info.BoxCorner[i];
		data.ShadowCorner[i].Z = 0;
		data.ShadowCorner[i] = camera * data.ShadowCorner[i];
		data.ShadowCorner[i] = data.ShadowCorner[i] + light;
		data.ShadowCorner[i].Y = -data.ShadowCorner[i].Y;

		if (data.ShadowCorner[i].X < MinVoxelBounds.X) {
			MinVoxelBounds.X = data.ShadowCorner[i].X;
		}
		if (data.ShadowCorner[i].Y < MinVoxelBounds.Y) {
			MinVoxelBounds.Y = data.ShadowCorner[i].Y;
		}
		if (data.ShadowCorner[i].X > MaxVoxelBounds.X) {
			MaxVoxelBounds.X = data.ShadowCorner[i].X;
		}
		if (data.ShadowCorner[i].Y > MaxVoxelBounds.Y) {
			MaxVoxelBounds.Y = data.ShadowCorner[i].Y;
		}
	}

	VoxelShadowRenderDataCount++;
}


/// <summary>
/// Prepares one voxel object for the next render.
/// This routine transforms the model's bounding box into view space and hands it to the
/// render list, growing the bounds that the whole scene will be composed within. Call it
/// once for every layer of the object that should appear.
/// </summary>
/// <param name="info">The layer info variant of the model to draw.</param>
/// <param name="transform">The camera transform combined with the object's own.</param>
/// <remarks>Reset must be called before the first object of a frame is prepared.</remarks>
void VoxelDrawSystem::Prep_For_Object(VoxelLibrary * voxlib, int layer, int info, Matrix3D const & transform)
{
	VoxelLibrary::LayerInfoStruct const & layer_info = voxlib->Get_Layer_Info(layer, info);

	VoxelRenderStruct & data = VoxelRenderData[VoxelRenderDataCount];

	data.MinBounds = Vector3(10000, 10000, 10000);
	data.MaxBounds = Vector3(-10000, -10000, -10000);
	data.Info = info;
	data.Layer = layer;
	data.VoxLib = voxlib;

	for (int i = 0; i < FACING_COUNT; i++) {
		data.BoxCorner[i] = transform * layer_info.BoxCorner[i];
		data.BoxCorner[i].Y = -data.BoxCorner[i].Y;
		if (data.BoxCorner[i].X < data.MinBounds.X) {
			data.MinBounds.X = data.BoxCorner[i].X;
		}
		if (data.BoxCorner[i].X > data.MaxBounds.X) {
			data.MaxBounds.X = data.BoxCorner[i].X;
		}
		if (data.BoxCorner[i].Y < data.MinBounds.Y) {
			data.MinBounds.Y = data.BoxCorner[i].Y;
		}
		if (data.BoxCorner[i].Y > data.MaxBounds.Y) {
			data.MaxBounds.Y = data.BoxCorner[i].Y;
		}
		if (data.BoxCorner[i].Z < data.MinBounds.Z) {
			data.MinBounds.Z = data.BoxCorner[i].Z;
			data.AnchorCornerIndex = i;
		}
		if (data.BoxCorner[i].Z > data.MaxBounds.Z) {
			data.MaxBounds.Z = data.BoxCorner[i].Z;
		}
	}

	if (data.MinBounds.X < MinVoxelBounds.X) {
		MinVoxelBounds.X = data.MinBounds.X;
	}
	if (data.MinBounds.Y < MinVoxelBounds.Y) {
		MinVoxelBounds.Y = data.MinBounds.Y;
	}
	if (data.MinBounds.Z < MinVoxelBounds.Z) {
		MinVoxelBounds.Z = data.MinBounds.Z;
	}
	if (data.MaxBounds.X > MaxVoxelBounds.X) {
		MaxVoxelBounds.X = data.MaxBounds.X;
	}
	if (data.MaxBounds.Y > MaxVoxelBounds.Y) {
		MaxVoxelBounds.Y = data.MaxBounds.Y;
	}
	if (data.MaxBounds.Z > MaxVoxelBounds.Z) {
		MaxVoxelBounds.Z = data.MaxBounds.Z;
	}

	VoxelRenderDataCount++;
}


/// <summary>
/// Draws every prepared voxel object into the drawing buffer.
/// This routine finishes the voxel drawing sequence. The shadows go down first, then the
/// objects in depth order, and the caller blits the result out of the voxel surface. This
/// overload serves the voxel animation drawer, which wants the drawn rectangle and the
/// screen offset as loose values rather than as a surface region.
/// </summary>
/// <param name="rect">Filled in with the rectangle of the voxel buffer that was drawn.</param>
/// <param name="x">Filled in with the horizontal screen offset to blit that rectangle at.</param>
/// <param name="y">Filled in with the vertical screen offset to blit that rectangle at.</param>
/// <remarks>Reset must be called, and the objects prepared, before this routine.</remarks>
void VoxelDrawSystem::Render(Rect & rect, int & x, int & y)
{
	static VoxelRenderStruct * render_data[64];

	int index;

	Vector3 center((MaxVoxelBounds.X + MinVoxelBounds.X) * 0.5f, (MaxVoxelBounds.Y + MinVoxelBounds.Y) * 0.5f, (MaxVoxelBounds.Z + MinVoxelBounds.Z) * 0.5f);

	for (index = 0; index < VoxelShadowRenderDataCount; index++) {
		VoxelShadowRenderData[index].VoxLib->Render_Shadow(VoxelShadowRenderData[index], center);
	}

	for (index = 0; index < VoxelRenderDataCount; index++) {
		render_data[index] = &VoxelRenderData[index];
	}

	for (index = 1; index - 1 < VoxelRenderDataCount -1; index++) {
		for (int index2 = index; index2 < VoxelRenderDataCount; index2++) {
			VoxelRenderStruct * first = render_data[index - 1];
			VoxelRenderStruct * second = render_data[index2];

			if (first->MaxBounds.Z > second->MaxBounds.Z) {
				render_data[index - 1] = second;
				render_data[index2] = first;
			}
		}
	}

	if (VoxelDrawSystem::EnableZBuffer) {
		for (index = VoxelRenderDataCount - 1; index >= 0; index--) {
			render_data[index]->VoxLib->Render_Object(*render_data[index], center);
		}
	} else {
		for (index = 0; index < VoxelRenderDataCount; index++) {
			render_data[index]->VoxLib->Render_Object(*render_data[index], center);
		}
	}

	rect.Width = MaxVoxelBounds.X - MinVoxelBounds.X;

	rect.Height = MaxVoxelBounds.Y - MinVoxelBounds.Y;

	rect.Set(VOXEL_BITMAP_WIDTH / 2 - rect.Width / 2, VOXEL_BITMAP_HEIGHT / 2 - rect.Height / 2, rect.Width + 8, rect.Height + 8);

	rect.X -= 4;
	rect.Y -= 4;

	x = (int)center.X - rect.Width / 2;
	y = (int)center.Y - rect.Height / 2;
}


/// <summary>
/// Draws every prepared voxel object into the drawing buffer.
/// This routine finishes the voxel drawing sequence. The shadows go down first, then the
/// objects in depth order, and the caller blits the result out of the voxel surface.
/// </summary>
/// <returns>Returns with the region drawn -- the rectangle within the voxel buffer, and the
/// point on the screen that it belongs at.</returns>
/// <remarks>Reset must be called, and the objects prepared, before this routine.</remarks>
SurfaceRegion VoxelDrawSystem::Render(void)
{
	static VoxelRenderStruct * render_data[64];

	int index;

	Vector3 center((MaxVoxelBounds.X + MinVoxelBounds.X) * 0.5f, (MaxVoxelBounds.Y + MinVoxelBounds.Y) * 0.5f, (MaxVoxelBounds.Z + MinVoxelBounds.Z) * 0.5f);

	for (index = 0; index < VoxelShadowRenderDataCount; index++) {
		VoxelShadowRenderData[index].VoxLib->Render_Shadow(VoxelShadowRenderData[index], center);
	}

	for (index = 0; index < VoxelRenderDataCount; index++) {
		render_data[index] = &VoxelRenderData[index];
	}

	for (index = 1; index - 1 < VoxelRenderDataCount - 1; index++) {
		for (int index2 = index; index2 < VoxelRenderDataCount; index2++) {
			VoxelRenderStruct * first = render_data[index - 1];
			VoxelRenderStruct * second = render_data[index2];

			if (first->MaxBounds.Z > second->MaxBounds.Z) {
				render_data[index - 1] = second;
				render_data[index2] = first;
			}
		}
	}

	if (VoxelDrawSystem::EnableZBuffer) {
		for (index = VoxelRenderDataCount - 1; index >= 0; index--) {
			render_data[index]->VoxLib->Render_Object(*render_data[index], center);
		}
	} else {
		for (index = 0; index < VoxelRenderDataCount; index++) {
			render_data[index]->VoxLib->Render_Object(*render_data[index], center);
		}
	}

	int width = MaxVoxelBounds.X - MinVoxelBounds.X;
	int height = MaxVoxelBounds.Y - MinVoxelBounds.Y;

	SurfaceRegion region;
	region.Point.X = (int)center.X - (width + 8) / 2;
	region.Point.Y = (int)center.Y - (height + 8) / 2;
	region.Bounds.Width = width + 8;
	region.Bounds.Height = height + 8;
	region.Bounds.X = VOXEL_BITMAP_WIDTH / 2 - width / 2 - 4;
	region.Bounds.Y = VOXEL_BITMAP_HEIGHT / 2 - height / 2 - 4;

	return(region);
}


/// <summary>
/// Clears the entire voxel drawing buffer.
/// </summary>
void VoxelDrawSystem::Clear_Buffer(void)
{
	memset(VoxelDrawBuffer, 0, sizeof(VoxelDrawBuffer));
}


/// <summary>
/// Clears the entire voxel depth buffer.
/// </summary>
void VoxelDrawSystem::Clear_Z_Buffer(void)
{
	memset(VoxelDrawZBuffer, 0, sizeof(VoxelDrawZBuffer));
}
