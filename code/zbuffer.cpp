/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "always.h"

#include "zbuffer.h"

#include "bsurface.h"

#include <cstdlib>

#define ZBUFFER_COLOR  0xFFFF
#define ZBUFFER_BPP    2		/// Bytes per depth entry (16 bit surface).


/// <summary>
/// Creates a depth buffer covering the area specified.
/// The buffer is a 16 bit surface that the depth aware blitters test and write against. It
/// is treated as a ring rather than a plain bitmap, so that the tactical map can scroll it
/// by sliding its origin. It starts out cleared to maximum depth, so that the first objects
/// drawn always pass the depth test.
/// </summary>
/// <param name="rect">The area the buffer is to cover.</param>
ZBuffer::ZBuffer(Rect rect) :
	BufferWidth(rect.Width),
	BufferHeight(rect.Height)
{
	Bounds = rect;
	BufferSize = BufferWidth * BufferHeight * ZBUFFER_BPP;
	SurfacePtr = new BSurface(BufferWidth, BufferHeight, ZBUFFER_BPP);

	Fill(ZBUFFER_COLOR);

	BufferStart = (uintptr_t)(SurfacePtr->Lock());
	SurfaceOffset = 0;
	BufferEnd = BufferStart + BufferWidth * BufferHeight * ZBUFFER_BPP;
	ScrollOffset = ZBUFFER_MAX;

	SurfacePtr->Unlock();
}


/// <summary>
/// Copies the depth buffer out to a surface.
/// The buffer is unwrapped as it is copied, so the destination receives the region laid out
/// the way it is displayed rather than the way it happens to be stored.
/// </summary>
/// <param name="surface">The surface to copy into.</param>
/// <param name="rect">The region of the destination surface to copy into.</param>
void ZBuffer::Copy_To(Surface *surface, Rect rect)
{
	//if (!surface) return;

	unsigned short *surfbuffptr = (unsigned short *)(surface->Lock(Point2D(rect.X, rect.Y)));

	unsigned short *pixptr = (unsigned short *)(BufferStart + SurfaceOffset);

	int steps = (surface->Stride() / ZBUFFER_BPP) - rect.Width;

	if (surfbuffptr != NULL) {
		for (int i = 0; i < rect.Height; ++i) {
			for (int j = 0; j < rect.Width; ++j) {
				*surfbuffptr = *pixptr;
				++surfbuffptr;
				pixptr = (unsigned short *)((unsigned char *)pixptr + ZBUFFER_BPP);
				pixptr = (unsigned short *)Wrap_Overflow((uintptr_t)pixptr);
			}
			surfbuffptr += steps;
		}
	}
	surface->Unlock();
}


/// <summary>
/// Frees the surface that backs the depth buffer.
/// </summary>
void ZBuffer::Release_Surface(void)
{
	delete SurfacePtr;
//	SurfacePtr = NULL;
}


/// <summary>
/// Fills a run of depth entries with the value specified.
/// This is the low level fill routine that the pan and update code use once they have
/// worked out where in the buffer to write. It works in raw buffer addresses rather than
/// in coordinates.
/// </summary>
/// <param name="dst">The buffer address to begin filling at.</param>
/// <param name="size">The number of depth entries to fill.</param>
/// <param name="value">The depth value to fill with.</param>
/// <remarks>The run is not wrapped. The caller must split any fill that would otherwise
/// run off the end of the buffer.</remarks>
void ZBuffer::Set(uintptr_t dst, int size, unsigned short value)
{
	/// Write a single entry to bring the address up to an int boundary.
	if (dst & 2) {
		if (size != 0) {
			*(unsigned short *)dst = value;
			dst = (uintptr_t)((unsigned short *)dst + 1);
			size--;
		}
	}

	int size_half = size >> 1;
	unsigned int *dst_ptr = NULL;

	/// Duplicate the value into both halves of an int and write the bulk of the run two
	/// entries at a time. This loop becomes a memset32.
	if (size_half) {
		int val = (value * (2 * ZBUFFER_MAX)) + value;
		dst_ptr = (unsigned int *)dst;
		while (size_half) {
			*dst_ptr = val;
			dst_ptr++;
			size_half--;
		}
	}

	/// Write the odd entry left over at the end. A run of a single entry that began
	/// aligned never sets dst_ptr, so it writes nothing.
	if (size & 1) {
		if (dst_ptr != NULL) {
			*(unsigned short *)dst_ptr = value;
		}
	}
}


/// <summary>
/// Scrolls the depth buffer by the amount specified.
/// This routine is what makes the buffer cheap for the tactical map to scroll. The depth
/// entries are never moved; the buffer's origin slides instead, and only the rows and
/// columns that have just come into view are reset. A scroll far enough to leave nothing
/// worth keeping resets the whole buffer instead.
/// </summary>
/// <param name="x">The distance to scroll horizontally, in pixels.</param>
/// <param name="y">The distance to scroll vertically, in pixels.</param>
/// <param name="value">The depth value to reset the newly exposed area to.</param>
void ZBuffer::Pan(int x, int y, unsigned short value)
{
	int target_col;
	int target_row;
	int x_delta = x;
	int y_delta = y;

	/// The column the buffer origin currently sits on.
	int current_col = (SurfaceOffset / ZBUFFER_BPP) % BufferWidth;

	if (abs(x_delta) > BufferWidth || abs(y_delta) > BufferHeight) {
		Fill(value);
		ScrollOffset = ZBUFFER_MAX;

	} else {
		target_col = current_col + x_delta;

		if (x_delta != 0) {

			/// Slide the origin along the row and fold it back into the buffer.
			SurfaceOffset += x_delta * ZBUFFER_BPP;

			uintptr_t new_offset = Wrap_Underflow(SurfaceOffset + BufferStart);
			new_offset = Wrap_Overflow(new_offset);
			SurfaceOffset = new_offset - BufferStart;

			/// Reset the columns that have just come into view, in two pieces when the
			/// exposed strip straddles the wrap point.
			if (x_delta < 0) {
				if (target_col < 0) {
					Fill(value, Rect(0, 0, current_col, BufferHeight));
					target_col += BufferWidth;
					Fill(value, Rect(target_col, 0, BufferWidth - target_col, BufferHeight));

				} else {
					Fill(value, Rect(target_col, 0, -x_delta, BufferHeight));
				}

			} else if (target_col >= BufferWidth) {
				Fill(value, Rect(current_col, 0, BufferWidth - current_col, BufferHeight));
				target_col -= BufferWidth;
				Fill(value, Rect(0, 0, target_col, BufferHeight));

			} else {
				Fill(value, Rect(current_col, 0, x_delta, BufferHeight));
			}
		}

		/// The row the buffer origin currently sits on.
		int current_row = (SurfaceOffset / ZBUFFER_BPP) / BufferWidth;

		if (y_delta != 0) {
			target_row = current_row + y_delta;

			/// The scroll bias is what turns a screen row into a depth value, so carrying
			/// the pan in it is what spares the entries already stored from a rewrite.
			ScrollOffset -= y_delta;

			int prev_offset = SurfaceOffset;

			/// Slide the origin by whole rows and fold it back into the buffer.
			SurfaceOffset += y_delta * BufferWidth * ZBUFFER_BPP;

			uintptr_t new_offset = Wrap_Underflow(SurfaceOffset + BufferStart);
			new_offset = Wrap_Overflow(new_offset);
			SurfaceOffset = new_offset - BufferStart;

			/// Reset the rows that have just come into view, in two pieces when the
			/// exposed strip straddles the wrap point.
			if (y_delta < 0) {
				if (target_row < 0) {
					Set(BufferStart, prev_offset / ZBUFFER_BPP, value);
					Set(BufferStart + SurfaceOffset, ((BufferWidth * BufferHeight * ZBUFFER_BPP) - SurfaceOffset) / ZBUFFER_BPP, value);

				} else {
					Set(BufferStart + SurfaceOffset, (prev_offset - SurfaceOffset) / ZBUFFER_BPP, value);
				}

			} else if (target_row >= BufferHeight) {
				Set(BufferStart + prev_offset, ((BufferWidth * BufferHeight * ZBUFFER_BPP) - prev_offset) / ZBUFFER_BPP, value);
				Set(BufferStart, SurfaceOffset / ZBUFFER_BPP, value);

			} else {
				Set(BufferStart + prev_offset, (SurfaceOffset - prev_offset) / ZBUFFER_BPP, value);
			}
		}
	}
}


/// <summary>
/// Fills the whole depth buffer with the value specified.
/// </summary>
/// <param name="value">The depth value to fill the buffer with.</param>
/// <returns>bool; Was the buffer filled?</returns>
bool ZBuffer::Fill(unsigned short value)
{
	return(SurfacePtr->Fill_Rect(Rect(0, 0, BufferWidth, BufferHeight), value));
}


/// <summary>
/// Fills a region of the depth buffer with the value specified.
/// </summary>
/// <param name="value">The depth value to fill the region with.</param>
/// <param name="rect">The region of the buffer to fill.</param>
/// <returns>bool; Was the region filled?</returns>
bool ZBuffer::Fill(unsigned short value, Rect rect)
{
	return(SurfacePtr->Fill_Rect(rect, value));
}


/// <summary>
/// Resets a region of the depth buffer back to maximum depth.
/// The tactical map calls this routine for every region it is about to redraw, so that the
/// depth compares there start from a clean slate. Unlike Fill, the region is located
/// through the current pan offset and is allowed to straddle the wrap point.
/// </summary>
/// <param name="rect">The region of the buffer to reset.</param>
void ZBuffer::Update(Rect rect)
{
	uintptr_t buffptr = Get_Buffer_Offset(Point2D(rect.X, rect.Y));

	for (int i = 0; i < rect.Height; ++i) {

		unsigned int size;
		if ((buffptr + rect.Width * ZBUFFER_BPP) >= BufferEnd) {
			size = (BufferEnd - buffptr) / ZBUFFER_BPP;
			Set(buffptr, size, ZBUFFER_COLOR);
			Set(BufferStart, rect.Width - size, ZBUFFER_COLOR);
		} else {
			Set(buffptr, rect.Width, ZBUFFER_COLOR);
		}

		buffptr += BufferWidth * ZBUFFER_BPP;
		buffptr = Wrap_Overflow(buffptr);
	}
}


/// <summary>
/// Fetches the address of a point within the depth buffer.
/// The point is given in buffer coordinates. The current pan offset is applied to it and
/// the result is wrapped, so the address returned is where that entry actually lives.
/// </summary>
/// <param name="pos">The point within the buffer to locate.</param>
/// <returns>Returns with the address of the entry within the depth buffer.</returns>
uintptr_t ZBuffer::Get_Buffer_Offset(Point2D pos)
{
	uintptr_t buffptr = (uintptr_t)SurfacePtr->Lock(pos);

	SurfacePtr->Unlock();

	buffptr += SurfaceOffset;
	return(Wrap_Overflow(buffptr));
}
