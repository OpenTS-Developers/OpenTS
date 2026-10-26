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
#include "win.h"

class FileClass;


class MotionLibrary
{
	public:
		MotionLibrary();
		MotionLibrary(FileClass & file);
		~MotionLibrary();

		void Clear();
		BOOL Read_File(FileClass & file);
		void Scale(float scale);

		bool Load_Failed() const { return(LoadFailed); }
		int Get_Layer_Count() const { return(LayerCount); }
		int Get_Frame_Count() const { return(FrameCount); }
		Matrix3D const & Get_Layer_Matrix(int layer, unsigned frame) const {return(LayerMatrices[layer + Get_Layer_Count() * (frame % Get_Frame_Count())]);}

	private:
		/*
		 * If the animation this library was constructed from could not be read, then this
		 * flag will be true. A constructor has no way to report a failure, so the owner is
		 * expected to ask afterward rather than draw through a library that holds nothing.
		 */
		bool LoadFailed;

		/*
		 * This is the number of voxel sections the animation drives. Every frame carries
		 * one transform for each of them.
		 */
		unsigned int LayerCount;

		/*
		 * This is the number of frames the animation runs for. Frames are asked for modulo
		 * this count, so an animation loops rather than running off its end.
		 */
		unsigned int FrameCount;

		/*
		 * Pointer to the transform matrices read from the animation file, one per layer per
		 * frame, with the layers of a frame laid out together. It remains NULL until an
		 * animation has been read in.
		 */
		Matrix3D * LayerMatrices;
};
