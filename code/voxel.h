/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include "face.h"
#include "sun.h"

class Matrix3D;
class Vector3;
class Coord;

struct VPLHeaderStruct
{
	/*
	 * These bound the range of palette indices, inclusive, that are set aside for the house
	 * color remap. A color inside the range is only ever shaded to another remap color, so
	 * a voxel keeps its owner's color however dark it is drawn.
	 */
	int RemapStart;
	int RemapEnd;

	/*
	 * This is the number of brightness steps the shading tables hold, each step being a
	 * full palette worth of entries stored after the colors.
	 */
	int LUTCount;

	/// Unused
	int Unused;
};
static_assert(sizeof(VPLHeaderStruct) == 16, "the VPL header is 16 bytes on disk");

Matrix3D Get_Isometric_View_Matrix(void);
void Init_Voxel_Matrices(void);

void Get_Screen_Slope_Matrix(int ramp, Matrix3D & matrix);
Matrix3D Get_Slope_Matrix(int ramp);

void Get_Screen_Slope_Transition_Matrix(int oldramp, int newramp, Matrix3D & matrix, double time);
Matrix3D Get_Slope_Transition_Matrix(int oldramp, int newramp, double time);

void Apply_Light_Transform(Matrix3D & matrix, int axis, int angle);
Vector3 Get_Light_Vector(Dir256 dir, int);

void Clear_Voxel_Indexes(void);

Matrix3D Get_Bounce_Matrix(FacingType facing);
