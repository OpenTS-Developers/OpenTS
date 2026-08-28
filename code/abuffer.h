/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include "rect.h"

#include <cstdint>

class Surface;

class ABuffer
{
		friend class IsometricTileTypeClass;
	public:
		//ABuffer(void);
		ABuffer(Rect rect);
		~ABuffer(void) { Release_Surface(); }

		unsigned Get_Scroll(void) const { return(ScrollOffset); }
		void Set_Scroll(int position) { ScrollOffset = position; }
		int Get_Scroll_Delta(int position) const { return(ScrollOffset - position); }

		void Copy_To(Surface * surface, Rect rect);

		void Set(uintptr_t dst, int size, unsigned short value);

		void Pan(int x_delta, int y_delta, unsigned short value);

		bool Fill(unsigned short value);
		bool Fill(unsigned short value, Rect rect);

		void Update(Rect rect);

		uintptr_t Get_Buffer_Offset(Point2D position);

		uintptr_t Wrap_Overflow(uintptr_t position) const;
		uintptr_t Wrap_Underflow(uintptr_t position) const;

		Surface * Get_Surface(void) const { return(SurfacePtr); }

		Rect const & Get_Bounds(void) const { return(Bounds); }
		unsigned int Get_Buffer_Width(void) const { return(BufferWidth); }
		uintptr_t Get_Buffer_End(void) const { return(BufferEnd); }

	private:
		void Release_Surface(void);

	public:
		/*
		 * This is the area of the screen that the alpha buffer covers, normally the tactical
		 * view. Buffer coordinates are relative to its upper left corner.
		 */
		Rect Bounds;

	private:
		/*
		 * This is how far, expressed in bytes, the upper left of the covered area now sits
		 * from the start of the surface. Panning advances this rather than moving the alpha
		 * values themselves, which is what makes the buffer a ring.
		 */
		int SurfaceOffset;

		/*
		 * This points to the 16 bit surface that the alpha values are accumulated into. The
		 * alpha aware blitters work through raw addresses into it rather than through the
		 * surface.
		 */
		Surface *SurfacePtr;

		/*
		 * These are the address the surface's pixels begin at, the address one past their
		 * end, and the number of bytes between the two. The buffer is treated as a ring, so
		 * an address that walks off either end is folded back around by that size.
		 */
		uintptr_t BufferStart;
		uintptr_t BufferEnd;
		unsigned int BufferSize;

		/*
		 * This is the bias carried along by every vertical pan, starting at the middle of
		 * its range and returning there whenever the buffer is cleared outright. The alpha
		 * buffer maintains it in step with the depth buffer but never consults it.
		 */
		unsigned int ScrollOffset;

		/*
		 * These are the width and height of the buffer, expressed in pixels, and they match
		 * the size of the area covered. A row is BufferWidth pixels long, which is what
		 * walking down a column steps an address by.
		 */
		int BufferWidth;
		int BufferHeight;
};

inline uintptr_t ABuffer::Wrap_Overflow(uintptr_t position) const
{
	if (position >= BufferEnd) {
		position -= BufferSize;
	}
	return(position);
}


inline uintptr_t ABuffer::Wrap_Underflow(uintptr_t position) const
{
	if (position < BufferStart) {
		position += BufferSize;
	}
	return(position);
}


extern ABuffer * AlphaBuffer;


inline unsigned short *Blit_Wrap_A_Buffer(unsigned short *buf)
{
	return((unsigned short *)AlphaBuffer->Wrap_Overflow((uintptr_t)buf));
}
