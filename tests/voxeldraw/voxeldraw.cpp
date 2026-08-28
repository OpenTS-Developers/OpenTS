/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// Holds the six voxel drawers in voxlib.cpp to the output the assembly they replaced produced.
// The vectors in voxelgolden.h were recorded from that assembly before it was removed.
//
// These drawers were reached only through entries 16 to 31 of VoxelDrawFunctions, which the
// dispatch never indexed, so until that table was repointed none of this code had ever run and
// nothing would have noticed it drawing the wrong thing. Needs no game data.

#include <cstdio>
#include <cstring>
#include <climits>

#include "voxdrsys.h"
#include "voxlib.h"

#include "voxelgolden.h"

void __cdecl Draw_Voxel_Regular_Normals(VoxelFuncArgumentStruct * state);
void __cdecl Draw_Voxel_Reverse_Normals(VoxelFuncArgumentStruct * state);
void __cdecl Draw_Voxel_Regular_Normals_Lighting(VoxelFuncArgumentStruct * state);
void __cdecl Draw_Voxel_Reverse_Normals_Lighting(VoxelFuncArgumentStruct * state);
void __cdecl Draw_Voxel_Regular(VoxelFuncArgumentStruct * state);
void __cdecl Draw_Voxel_Reverse(VoxelFuncArgumentStruct * state);

/*
 * Standing in for voxdrsys.cpp, which the harness does not build. The draw buffer is larger
 * than the engine's so that a drawer overrunning it is caught here rather than corrupting
 * whatever the engine happens to place next to it.
 */
extern "C" {
unsigned char VoxelDrawBuffer[262144];
unsigned char VoxelDrawZBuffer[262144];
unsigned char VoxelPaletteTranslateTable[MAX_PALETTE_LOOKUP_ENTRIES][VOXEL_PALETTE_SIZE];
}

RGBStruct VoxelRGBColors[VOXEL_PALETTE_SIZE];

namespace VoxelDrawSystem {
	BOOL EnableLighting = 0;
	BOOL EnableZBuffer = 0;
}

Matrix3D::Matrix3D(float * const) {}
VoxelPaletteLibrary::VoxelPaletteLibrary(RGBStruct *, void *) {}
VoxelPaletteLibrary::~VoxelPaletteLibrary(void) {}
void VoxelPaletteLibrary::Calculate_Lookup_Table(float *, int) {}

namespace {

int const SPANMAX = 4096;
int const DATAMAX = 65536;

unsigned char StartOffsets[SPANMAX];
unsigned char EndOffsets[SPANMAX];
unsigned char VoxelData[DATAMAX];

unsigned int Seed = 0;

int Failures = 0;
int Checked = 0;


unsigned int Next_Random(void)
{
	Seed = Seed * 1103515245u + 12345u;
	return(Seed >> 8);
}


unsigned long long Hash(unsigned char const * data, int size)
{
	unsigned long long hash = 1469598103934665603ULL;
	for (int i = 0; i < size; i++) {
		hash ^= (unsigned long long)data[i];
		hash *= 1099511628211ULL;
	}
	return(hash);
}

typedef void (* Drawer)(VoxelFuncArgumentStruct *);

Drawer const Drawers[6] = {
	Draw_Voxel_Regular_Normals,
	Draw_Voxel_Reverse_Normals,
	Draw_Voxel_Regular_Normals_Lighting,
	Draw_Voxel_Reverse_Normals_Lighting,
	Draw_Voxel_Regular,
	Draw_Voxel_Reverse
};

char const * const Names[6] = {
	"Draw_Voxel_Regular_Normals",
	"Draw_Voxel_Reverse_Normals",
	"Draw_Voxel_Regular_Normals_Lighting",
	"Draw_Voxel_Reverse_Normals_Lighting",
	"Draw_Voxel_Regular",
	"Draw_Voxel_Reverse"
};


/*
 * Must build byte for byte what the generator built.
 *
 * A column's spans have to account for exactly ZSize voxels between them, because the drawers
 * count down from ZSize and stop on zero; a column adding up to anything else takes the counter
 * past it and the walk runs away through the data. The column table holds 32 bit offsets and a
 * negative entry means an empty column. Each span ends with a repeat of its length, which the
 * forward drawers step over and the reverse drawers read first.
 */
void Build_Layer(unsigned int seed, int columns, int zsize, bool withnormals)
{
	Seed = seed;

	unsigned int * starts = (unsigned int *)StartOffsets;
	unsigned int * ends = (unsigned int *)EndOffsets;

	int at = 0;

	for (int i = 0; i < columns; i++) {

		if ((Next_Random() % 8) == 0) {
			starts[i] = UINT_MAX;
			ends[i] = UINT_MAX;
			continue;
		}

		starts[i] = (unsigned int)at;

		int remaining = zsize;

		while (remaining > 0) {
			int const skip = (int)(Next_Random() % (unsigned int)remaining);
			remaining -= skip;

			int const run = 1 + (int)(Next_Random() % (unsigned int)remaining);
			remaining -= run;

			VoxelData[at++] = (unsigned char)skip;
			VoxelData[at++] = (unsigned char)run;

			/*
			 * A voxel is a colour and a normal for the drawers that shade, and a colour
			 * on its own for the two that do not.
			 */
			for (int v = 0; v < run; v++) {
				VoxelData[at++] = (unsigned char)(1 + (Next_Random() % 254));

				if (withnormals) {
					VoxelData[at++] = (unsigned char)(Next_Random() % 244);
				}
			}

			VoxelData[at++] = (unsigned char)run;
		}

		ends[i] = (unsigned int)(at - 1);
	}

	for (int i = 0; i < 256 * 3; i++) {
		((unsigned char *)VoxelRGBColors)[i] = (unsigned char)(Next_Random() & 0xFF);
	}

	for (int i = 0; i < MAX_PALETTE_LOOKUP_ENTRIES; i++) {
		for (int j = 0; j < VOXEL_PALETTE_SIZE; j++) {
			VoxelPaletteTranslateTable[i][j] = (unsigned char)(Next_Random() & 0xFF);
		}
	}
}


void Setup(VoxelFuncArgumentStruct & arg)
{
	std::memset(&arg, 0, sizeof(arg));

	arg.StartOffset = StartOffsets;
	arg.EndOffset = EndOffsets;
	arg.DataOffset = VoxelData;
	arg.StartIndex = 24;
	arg.StrideX = 1;
	arg.StrideY = 4;

	for (int i = 0; i < 4; i++) {
		arg.TransformMatrix[i].I = (short)(64 + i * 16);
		arg.TransformMatrix[i].J = (short)(48 + i * 8);
		arg.TransformMatrix[i].K = (short)(32 + i * 4);
	}

	arg.XSize = 4;
	arg.YSize = 4;
	arg.ZSize = 8;
}

}	// namespace


int main(void)
{
	for (int i = 0; i < VoxelGoldenCaseCount; i++) {
		VoxelGoldenCase const & test = VoxelGoldenCases[i];

		Build_Layer(test.Seed, 64, 8, test.Which < 4);

		VoxelFuncArgumentStruct arg;
		std::memset(VoxelDrawBuffer, 0, sizeof(VoxelDrawBuffer));
		Setup(arg);

		Drawers[test.Which](&arg);

		unsigned long long const hash = Hash(VoxelDrawBuffer, sizeof(VoxelDrawBuffer));

		if (hash != test.Hash) {
			std::printf("FAILED %-38s seed %u: expected %llu, got %llu\n",
				Names[test.Which], test.Seed, test.Hash, hash);
			Failures++;
		}

		Checked++;
	}

	std::printf("%-52s %s\n", "Voxel drawing matches the recorded assembly", Failures == 0 ? "ok" : "FAILED");
	std::printf("checked %d cases, %d mismatches\n", Checked, Failures);

	return(Failures == 0 ? 0 : 1);
}
