/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2025 Electronic Arts Inc.
 * Copyright 2026 OpenTS contributors
 *
 * Contains material derived from Electronic Arts source code.
 * Modified by OpenTS contributors, 2026.
 * EA's GPLv3 Section 7 additional terms and supplemental warranty
 * disclaimers apply; see LICENSE.md.
 ******************************************************************************/

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwlib/blitter.h                              $*
 *                                                                                             *
 *                      $Author:: Jani_p                                                      $*
 *                                                                                             *
 *                     $Modtime:: 5/04/01 7:48p                                               $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "win.h" // for NULL


/*
**	This is the interface class to the blitter object. The blitter object handles moving
**	pixels. That's all it does. For every type of pixel translation, there should be a
**	blitter object created that supports this interface. The blit blitting routines will
**	call the appropriate method as the pixel are being processed.
*/
class Blitter {
	public:
		virtual ~Blitter(void) {}

		/*
		**	Blits from source to dest (starts at first pixel). This is the preferred
		**	method of pixel blitting and this routine will be called 99% of the time under
		**	normal circumstances.
		*/
		virtual void BlitForward(void * dest, void const * source, int length, int z_min = 0, unsigned short * z_buff = NULL, unsigned short * a_buff = NULL, int alpha_level = 1000, int warp_offset = 0) const = 0;

		/*
		**	Copies the pixel in reverse order. This only required if the source and dest
		**	pixel regions overlap in a certain way. This routine will rarely be called.
		*/
		virtual void BlitBackward(void * dest, void const * source, int length, int z_min = 0, unsigned short * z_buff = NULL, unsigned short * a_buff = NULL, int alpha_level = 1000) const = 0;

		/*
		**	This routine calls the appropriate blit routine. A proper overlap check cannot
		**	be performed by this routine because the pixel size information is not present.
		**	as such, you should call the appropriate blit routine rather than letting this
		**	routine perform the check and call.
		*/
		void Blit(void * dest, void const * source, int length) const {if (dest < source) BlitBackward(dest, source, length); else BlitForward(dest, source, length);}
};


/*
**	This is the blitter object interface for dealing with RLE compressed pixel data. For
**	every type of RLE compressed blitter operation desired, there would be an object created
**	that supports this interface.
*/
class RLEBlitter {
	public:
		virtual ~RLEBlitter(void) {}

		/*
		**	Blits from the RLE compressed source to the destination buffer. An optional
		**	leading pixel skip value can be supplied when a sub-section of an RLE
		**	compressed pixel sequence is desired. This is necessary because RLE decompression
		**	must begin at the start of the compressed data sequence.
		*/
		virtual void Blit(void * dest, void const * source, int length, int leadskip = 0, int z_min = 0, unsigned short * z_buff = NULL, unsigned short * a_buff = NULL, int alpha_level = 0, int warp_offset = 0, void * zshape = NULL) const = 0;
};
