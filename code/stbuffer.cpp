/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "always.h"

#include "stbuffer.h"

#include "rle.h"
#include "surface.h"
#include "voxdrsys.h"

#include <cstring>


/// <summary>
/// Creates a static buffer of the capacity requested.
/// This routine will allocate the whole block up front. Space is only ever handed out from
/// the front of it, since individual entries are never released -- Reset is what reclaims
/// the buffer for reuse.
/// </summary>
/// <param name="size">The capacity of the buffer, in bytes.</param>
StaticBufferClass::StaticBufferClass(int size)
{
	Size = size;
	Buffer = new unsigned char[size];
	Cursor = Buffer;
}


/// <summary>
/// Frees the memory held by the static buffer.
/// Every entry handed out by Add points into this one block, so none of them outlive the
/// buffer they were cached in.
/// </summary>
StaticBufferClass::~StaticBufferClass(void)
{
	if (Buffer != NULL) {
		delete [] Buffer;
	}
}

/*
 * A 256-pixel voxel row can expand when RLE encodes isolated transparent pixels:
 * each such pixel takes two bytes, plus the row's length prefix. Reserve twice
 * the voxel-buffer width so every cached voxel row fits.
 */
static char CompressionBuffer[2 * VOXEL_BITMAP_WIDTH];

/// <summary>
/// Adds a compressed copy of a surface region to the buffer.
/// This routine is used by the voxel drawing cache to stash a freshly rendered voxel so
/// that later frames can blit the image back instead of rendering it again. The image is
/// run length compressed as it is copied out of the surface.
/// </summary>
/// <param name="surface">The surface to capture the image from.</param>
/// <param name="region">The area of the surface to capture, along with the draw offset to
/// remember with it.</param>
/// <returns>Returns with a pointer to the entry describing the cached image. Otherwise,
/// NULL is returned if the buffer has no room left.</returns>
StaticBufferClass::Entry * StaticBufferClass::Add(Surface & surface, SurfaceRegion const & region)
{
	Entry * header = (Entry *)Reserve(sizeof(Entry));
	if (header == NULL) {
		return(NULL);
	}

	header->X = region.Point.X;
	header->Y = region.Point.Y;
	header->Width = region.Bounds.Width;
	header->Height = region.Bounds.Height;
	header->Data = Cursor;

	unsigned char * data = (unsigned char *)surface.Lock(region.Bounds.TopLeft);

	RLEEngine rle;
	int line = 0;
	while (line < region.Bounds.Height) {
		int comp_size = rle.Line_Compress(data, CompressionBuffer, region.Bounds.Width);
		unsigned char * buffer = Reserve(comp_size);
		if (buffer == NULL) {
			surface.Unlock();
			return(NULL);
		}

		memcpy(buffer, CompressionBuffer, comp_size);
		line++;
		data += surface.Stride();
	}

	surface.Unlock();
	return(header);
}


/// <summary>
/// Adds a compressed copy of a clipped surface rectangle to the buffer.
/// This routine serves the same purpose as the region flavor above, except that the caller
/// states the draw offset to remember with the image rather than taking it from a surface
/// region.
/// </summary>
/// <param name="surface">The surface to capture the image from.</param>
/// <param name="cliprect">The area of the surface to capture.</param>
/// <param name="x">The horizontal draw offset to remember with the cached image.</param>
/// <param name="y">The vertical draw offset to remember with the cached image.</param>
/// <returns>Returns with a pointer to the entry describing the cached image. Otherwise,
/// NULL is returned if the buffer has no room left.</returns>
StaticBufferClass::Entry * StaticBufferClass::Add(Surface & surface, Rect const & cliprect, short x, short y)
{
	Entry * header = (Entry *)Reserve(sizeof(Entry));
	if (header == NULL) {
		return(NULL);
	}

	header->X = x;
	header->Y = y;
	header->Width = cliprect.Width;
	header->Height = cliprect.Height;
	header->Data = Cursor;

	unsigned char * data = (unsigned char *)surface.Lock(cliprect.TopLeft);

	RLEEngine rle;
	int line = 0;
	while (line < cliprect.Height) {
		int comp_size = rle.Line_Compress(data, CompressionBuffer, cliprect.Width);
		unsigned char * buffer = Reserve(comp_size);
		if (buffer == NULL) {
			surface.Unlock();
			return(NULL);
		}

		memcpy(buffer, CompressionBuffer, comp_size);
		line++;
		data += surface.Stride();
	}

	surface.Unlock();
	return(header);
}
