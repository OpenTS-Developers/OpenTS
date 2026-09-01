/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "always.h"

#include "voxlib.h"
#include "wwfile.h"


/// <summary>
/// Creates a voxel palette library.
/// The library holds the palette every voxel is colored from together with the tables the
/// drawing routines shade with. The caller may hand over its own buffers -- the voxel draw
/// system does exactly that, so the loaded data lands straight in the globals the drawing
/// routines read -- or leave them NULL and let the library find its own storage.
/// </summary>
/// <param name="rgb">Pointer to the palette buffer to use, or NULL to allocate one.</param>
/// <param name="lut">Pointer to the shading table buffer to use, or NULL to allocate
/// one.</param>
VoxelPaletteLibrary::VoxelPaletteLibrary(RGBStruct * rgb, void * lut)
{
	if (rgb == NULL) {
		Colors = new RGBStruct[VPL_NUM_COLORS];
		ColorsAllocated = TRUE;
	} else {
		Colors = rgb;
		ColorsAllocated = FALSE;
	}

	if (lut == NULL) {
		LUT = new unsigned char[VPL_NUM_COLORS*VPL_NUM_ENTRIES];
		LUTAllocated = TRUE;
	} else {
		LUT = (unsigned char *)lut;
		LUTAllocated = FALSE;
	}
}


/// <summary>
/// Destroys the palette library.
/// Only the buffers this library allocated for itself are freed; any that the creator
/// supplied are left alone.
/// </summary>
VoxelPaletteLibrary::~VoxelPaletteLibrary(void)
{
	if (ColorsAllocated) {
		delete [] Colors;
	}
	if (LUTAllocated) {
		delete [] LUT;
	}
}


/// <summary>
/// Reads just the palette from a file.
/// Use this routine when the colors alone are wanted and the header and shading tables are
/// of no interest.
/// </summary>
/// <returns>Returns with zero if the palette was read, or one if it could not be.</returns>
int VoxelPaletteLibrary::Read_Colors(FileClass & file)
{
	int result = 0;

	file.Open(FileClass::READ);
	if (file.Read(Colors, VPL_NUM_COLORS*sizeof(RGBStruct)) != (VPL_NUM_COLORS*sizeof(RGBStruct))) {
		result = 1;
	}

	file.Close();

	return(result);
}


/// <summary>
/// Reads a complete palette library from a file.
/// This routine loads the header, the palette, and the prebuilt shading tables, which is
/// everything the voxel drawing routines need to color and light a model.
/// </summary>
/// <returns>Returns with zero if the library was read, or one if it could not be.</returns>
int VoxelPaletteLibrary::Read_File(FileClass & file)
{
	VPLHeaderStruct hdr;
	int result = 0;

	if (result == 0) {
		if (!file.Open(FileClass::READ)) {
			result = 1;
		}
	}

	if (result == 0) {
		if (file.Read(&hdr, sizeof(hdr)) != sizeof(hdr)) {
			result = 1;
		}
	}

	if (result == 0) {
		Header.RemapStart = hdr.RemapStart;
		Header.RemapEnd = hdr.RemapEnd;
		Header.LUTCount = hdr.LUTCount;
		Header.Unused = hdr.Unused;

		if (file.Read(Colors, VPL_NUM_COLORS*sizeof(*Colors)) != (VPL_NUM_COLORS*sizeof(*Colors))) {
			result = 1;
		}
	}

	if (result == 0) {
		if (file.Read(LUT, Header.LUTCount*VPL_NUM_COLORS) != (Header.LUTCount*VPL_NUM_COLORS)) {
			result = 1;
		}
	}

	file.Close();

	return(result);
}


/// <summary>
/// Writes this palette library out to a file.
/// This is the counterpart of Read_File and produces a VPL the game can load again --
/// header, palette, and shading tables.
/// </summary>
/// <returns>Returns with zero if the library was written, or one if it could not be.</returns>
int VoxelPaletteLibrary::Write_File(FileClass & file)
{
	VPLHeaderStruct hdr;
	int result = 0;

	if (result == 0) {
		if (!file.Open(FileClass::WRITE)) {
			result = 1;
		}
	}

	if (result == 0) {
		hdr.RemapStart = Header.RemapStart;
		hdr.RemapEnd = Header.RemapEnd;
		hdr.LUTCount = Header.LUTCount;
		hdr.Unused = Header.Unused;

		if (file.Write(&hdr, sizeof(hdr)) != sizeof(hdr)) {
			result = 1;
		}
	}

	if (result == 0) {
		if (file.Write(Colors, VPL_NUM_COLORS*sizeof(*Colors)) != (VPL_NUM_COLORS*sizeof(*Colors))) {
			result = 1;
		}
	}

	if (result == 0) {
		if (file.Write(LUT, Header.LUTCount*VPL_NUM_COLORS) != (Header.LUTCount*VPL_NUM_COLORS)) {
			result = 1;
		}
	}

	file.Close();

	return(result);
}


/// <summary>
/// Builds the shading lookup tables for this palette.
/// This routine is used when a voxel object carries its own palette rather than the one out
/// of VOXELS.VPL. Every palette color is dimmed and brightened through the scale ramp and
/// matched back onto the nearest palette entry, so the drawing routines can shade a voxel by
/// table lookup instead of by color arithmetic.
/// </summary>
/// <param name="scale">The ramp of brightness scales to build with, or NULL to use the
/// built in ramp.</param>
/// <param name="lut_count">The number of brightness steps in the ramp supplied.</param>
void VoxelPaletteLibrary::Calculate_Lookup_Table(float *scale, int lut_count)
{
	int index;
	int entry;
	int pos;

	#define STEPS (1.0f / (VPL_NUM_ENTRIES/2))

	static float _f[VPL_NUM_ENTRIES] = {
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	};

	static const float _addend = 0.6f;

	float *shades;

	if (scale && lut_count) {
		Header.LUTCount = lut_count;
		shades = scale;

	} else {

		shades = _f;

		Header.LUTCount = VPL_NUM_ENTRIES;

		for (index = 0; index < VPL_NUM_ENTRIES/2; index++) {
			_f[index] = (index * STEPS) * 0.8f + _addend;
		}

		for (index = VPL_NUM_ENTRIES/2; index < VPL_NUM_ENTRIES; index++) {
			_f[index] = (index - 16) * STEPS + index * STEPS + 0.4f;
		}
	}

	bool remap;

	remap = false;
	for (entry = 0; entry < VPL_NUM_ENTRIES/2; entry++) {
		for (index = 1; index < VPL_NUM_COLORS; index++) {

			remap = index >= Header.RemapStart && index <= Header.RemapEnd;

			float red = Colors[index].Red;
			red *= shades[entry];
			float green = Colors[index].Green;
			green *= shades[entry];
			float blue = Colors[index].Blue;
			blue *= shades[entry];

			pos = (entry << 8) | index;
			LUT[pos] = Closest_Color(red, green, blue, remap);
		}
	}

	remap = false;
	for (entry = VPL_NUM_ENTRIES/2; entry < VPL_NUM_ENTRIES; entry++) {
		for (index = 1; index < VPL_NUM_COLORS; index++) {

			remap = index >= Header.RemapStart && index <= Header.RemapEnd;

			float red = Colors[index].Red;
			red *= shades[entry];
			float green = Colors[index].Green;
			green *= shades[entry];
			float blue = Colors[index].Blue;
			blue *= shades[entry];

			red = red <= 255 ? red : 255;
			green = green <= 255 ? green : 255;
			blue = blue <= 255 ? blue : 255;

			pos = (entry << 8) | index;
			LUT[pos] = Closest_Color(red, green, blue, remap);
		}
	}

	#undef STEPS
}


/// <summary>
/// Finds the palette entry closest to the color specified.
/// This routine searches the whole voxel palette, house colors included, for the entry
/// nearest the color given.
/// </summary>
/// <returns>Returns with the index of the closest color in the palette.</returns>
unsigned char VoxelPaletteLibrary::Closest_Color(float red, float green, float blue) const
{
	int closest = 0;
	float value = 100000.0f;

	int r, g, b = 0;
	for (int index = 0; index < VPL_NUM_COLORS; index++) {

		r = Colors[index].Red;
		float fr = red	 - r;

		g = Colors[index].Green;
		float fg = green - g;

		b = Colors[index].Blue;
		float fb = blue  - b;

		// Ties are broken by iteration order, so a last-bit difference picks a
		// different colour outright.
		float difference = (float)((double)fr * fr + (double)fg * fg + (double)fb * fb);
		if (difference < value) {
			value = difference;
			closest = index;
		}
	}

	return(closest);
}


/// <summary>
/// Finds the palette entry closest to the color specified, respecting the remap band.
/// This routine is used while the shading tables are built. The house color remap range is
/// kept apart from the rest of the palette, so that a remappable color never shades into a
/// fixed one and a fixed color never strays into the house colors.
/// </summary>
/// <param name="is_remap">Should the search be confined to the house color remap range?</param>
/// <returns>Returns with the index of the closest color on the side of the palette asked
/// for.</returns>
unsigned char VoxelPaletteLibrary::Closest_Color(float red, float green, float blue, bool is_remap) const
{
	int closest = is_remap ? Header.RemapStart : 1;
	float value = 100000.0f;

	for (int index = 1; index < VPL_NUM_COLORS; index++) {

		if (is_remap) {
			if (index < Header.RemapStart || index > Header.RemapEnd) {
				continue;
			}

		} else if (index >= Header.RemapStart && index <= Header.RemapEnd) {
			continue;
		}

		float dred = red	- Colors[index].Red;
		float dgreen = green - Colors[index].Green;
		float dblue = blue	- Colors[index].Blue;

		float difference = dred * dred + dgreen * dgreen + dblue * dblue;
		if (difference < value) {
			value = difference;
			closest = index;
		}
	}

	return(closest);
}
