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

#include "always.h"

#include "srfcache.h"

#include "ccfile.h"
#include "dsurface.h"
#include "ownrdraw.h"
#include "pcx.h"

#include <algorithm>
#include <new>


/*
 * Layout of a Windows .BMP file image in memory: the file header immediately
 * followed by the info header and palette. Packed to 2 to match the on-disk
 * layout (BITMAPFILEHEADER is 14 bytes).
 */
#pragma pack(push, 2)
struct MSBitmap
{
	BITMAPFILEHEADER filehead;
	BITMAPINFO info;
};
#pragma pack(pop)


/// <summary>
/// Builds a 16-bit pixel from an RGB triple with the channels remapped for the
/// green tint used by the owner-draw online UI: green is boosted by 1.5
/// (clamped to 255) and takes the red channel's place, red takes green's
/// place, and blue is quartered. Single-pixel variant of
/// SurfaceCacheConvertPalette.
/// </summary>
/// <param name="red">Red component (0-255).</param>
/// <param name="green">Green component (0-255).</param>
/// <param name="blue">Blue component (0-255).</param>
/// <returns>The remapped 16-bit pixel.</returns>
int SurfaceCacheConvertPixel(int red, int green, int blue)
{
	int boosted = (green * 15) / 10;
	green = std::min(255, boosted);
	return(DSurface::Build_Hicolor_Pixel(green, red, blue / 4));
}


/// <summary>
/// Applies the green-tint channel remap of SurfaceCacheConvertPixel to a
/// 256-entry RGB palette in place (see ODDrawBitsStrand, which tints the
/// "bits" strand imagery with this before drawing).
/// </summary>
/// <param name="pal">Palette to remap; 256 entries of 3 bytes each.</param>
void SurfaceCacheConvertPalette(unsigned char *pal)
{
	unsigned char *p = pal;
	for (int i = 0; i < 256; i++) {
		int red = p[0];
		int green = p[1];
		int blue = p[2];

		p[0] = std::min(255, ((green * 15) / 10));
		p[1] = red;
		p[2] = blue / 4;
		p += 3;
	}
}


/// <summary>
/// Hash function for the surface cache dictionary: a rotating shift-xor hash
/// over the key's characters, seeded with the string length.
/// </summary>
/// <param name="string">Key string to hash.</param>
/// <returns>The hash value.</returns>
static unsigned int SurfaceCache_Wstring_Hash(Wstring & string)
{
	unsigned int hash = 0;

	hash = string.length();

	for (unsigned int i = 0; i < string.length(); i++) {
		hash += *(string.get() + i);
		hash += i;
		hash = (hash << 8) ^ (hash >> 24);
	}
	return(hash);
}


/// <summary>
/// Constructs the cache as a Wstring-keyed dictionary using the surface
/// cache hash function.
/// </summary>
SurfaceCacheClass::SurfaceCacheClass(void) :
	Dictionary<Wstring, SurfaceCacheEntry>(SurfaceCache_Wstring_Hash)
{

}


/// <summary>
/// Empties the cache, deleting every cached surface.
/// </summary>
SurfaceCacheClass::~SurfaceCacheClass(void)
{
	Wstring key;
	SurfaceCacheEntry value;
	while (removeAny(key, value)) {
		delete value.surf;
	}
}


/// <summary>
/// Caches an in-memory Windows .BMP image under the given (case-insensitive)
/// name, replacing any surface already cached under it. The 8-bit source is
/// either converted to a 16-bit surface through its palette (bpp == 2) or
/// stored as-is with the palette kept in the cache entry (bpp == 1).
/// </summary>
/// <param name="name">Name to cache the image under; lowercased for the key.</param>
/// <param name="bitmap">Complete .BMP file image in memory.</param>
/// <param name="bytes">Size of the image in bytes; unused.</param>
/// <param name="bpp">Bytes per pixel of the cached surface: 2 or 1.</param>
/// <returns>Always true.</returns>
bool SurfaceCacheClass::CacheBMP(char const * name, void * bitmap, int bytes, int bpp)
{
	unsigned char *bitmap_palette_data;
	unsigned char *bitmap_data;
	BITMAPFILEHEADER filehead;
	BITMAPINFO *bitmap_info_header;
	int palette_bytes;
	int bitmap_width;
	int surface_width;
	BSurface *surface;
	unsigned short *surface_data;
	BITMAPINFOHEADER header;

	/*
	 * The 16-bit palette scratch buffer doubles as the storage for the old
	 * cache entry fetched below.
	 */
	unsigned short pal16[sizeof(SurfaceCacheEntry) / 2];
	SurfaceCacheEntry &old_entry = *(SurfaceCacheEntry *)&pal16[0];
	SurfaceCacheEntry new_entry;

	((void)bytes);

	/*
	 * Pull the file and info headers out of the raw file image; the palette
	 * follows the info header.
	 */
	filehead = ((MSBitmap *)bitmap)->filehead;
	BITMAPINFO *info = &((MSBitmap *)bitmap)->info;
	header = info->bmiHeader;
	bitmap_palette_data = (unsigned char *)info->bmiColors;

	/*
	 * Build a working copy of the info header with room for the palette
	 * behind it, then copy the palette in.
	 */
	bitmap_info_header = (BITMAPINFO *)operator new(4 * (1 << header.biBitCount) + sizeof(BITMAPINFOHEADER));
	bitmap_info_header->bmiHeader.biHeight = header.biHeight;
	bitmap_info_header->bmiHeader.biSize = header.biSize;
	bitmap_info_header->bmiHeader.biCompression = header.biCompression;
	bitmap_info_header->bmiHeader.biPlanes = header.biPlanes;
	bitmap_info_header->bmiHeader.biYPelsPerMeter = header.biYPelsPerMeter;
	bitmap_info_header->bmiHeader.biWidth = header.biWidth;
	bitmap_info_header->bmiHeader.biSizeImage = header.biSizeImage;
	bitmap_info_header->bmiHeader.biClrUsed = header.biClrUsed;
	bitmap_info_header->bmiHeader.biBitCount = header.biBitCount;
	palette_bytes = header.biClrUsed * sizeof(RGBQUAD);
	bitmap_info_header->bmiHeader.biClrImportant = header.biClrImportant;
	bitmap_info_header->bmiHeader.biXPelsPerMeter = header.biXPelsPerMeter;
	memcpy(bitmap_info_header->bmiColors, bitmap_palette_data, palette_bytes);
	bitmap_palette_data += palette_bytes;

	/*
	**	Convert the palette (RGBQUAD entries, stored blue-green-red-reserved)
	**	into a table of ready-made 16-bit pixels.
	*/
	unsigned char *p = (unsigned char *)bitmap_info_header->bmiColors;
	for (int i = 0; i < 256; i++) {
		int blue = *p++;
		int green = *p++;
		int pixel = (blue >> DSurface::BlueLeft) << DSurface::BlueRight;
		pixel |= (green >> DSurface::GreenLeft) << DSurface::GreenRight;
		pixel |= (unsigned char)(*p >> DSurface::RedLeft) << DSurface::RedRight;
		pal16[i] = pixel;
		p += 2;
	}

	/*
	 * Copy the pixel data out of the file image.
	 */
	bitmap_data = (unsigned char *)operator new(filehead.bfSize - filehead.bfOffBits);
	memcpy(bitmap_data, bitmap_palette_data, filehead.bfSize - filehead.bfOffBits);

	/*
	 * Blit the bitmap into a new surface. BMP pixel data is stored bottom-up,
	 * so source rows are walked from the last row backward. The source pitch
	 * is the width rounded down to a dword boundary plus four, which over-pads
	 * widths that are already dword-aligned.
	 */
	surface = new BSurface(header.biWidth, header.biHeight, bpp);
	surface_data = (unsigned short *)surface->Lock();
	surface_width = surface->Stride() / 2;
	bitmap_width = header.biWidth - (header.biWidth & 3) + 4;

	if (header.biHeight > 0) {
		int offset = 0;
		int row_step = -bitmap_width;
		int row_offset = bitmap_width * (header.biHeight - 1);
		int row_count = header.biHeight;
		do {
			int x = 0;
			if (header.biWidth > 0) {
				do {
					int source_index = row_offset + x;

					if (bpp == 2) {
						surface_data[offset + x] = pal16[bitmap_data[source_index]];
					} else {
						surface_data[offset + x] = bitmap_data[source_index];
					}
					x++;
				} while (x < header.biWidth);
			}
			row_offset += row_step;
			offset += surface_width;
			row_count--;
		} while (row_count);
	}

	/*
	 * An 8-bit surface keeps its palette in the cache entry.
	 */
	if (bpp == 1) {
		memcpy(new_entry.palette, bitmap_info_header->bmiColors, sizeof(new_entry.palette));
	}
	new_entry.surf = surface;

	surface->Unlock();
	operator delete(bitmap_info_header);
	operator delete(bitmap_data);

	Wstring key(name);
	key.toLower();

	/*
	 * If an entry is already cached under this name, remove it and delete
	 * its surface before adding the new one.
	 */
	old_entry.surf = NULL;
	if (getValue(key, old_entry)) {
		remove(key);
		if (old_entry.surf != NULL) {
			delete old_entry.surf;
		}
	}

	add(key, new_entry);

	return(true);
}


/// <summary>
/// Loads a .PCX file and caches it under its (case-insensitive) file name,
/// replacing any surface already cached under it. The 8-bit source is either
/// converted to a 16-bit surface through its palette (bpp == 2) or kept 8-bit
/// with the palette stored in the cache entry (bpp == 1).
/// </summary>
/// <param name="name">File name of the .PCX; also the cache key.</param>
/// <param name="bpp">Bytes per pixel of the cached surface: 2 or 1.</param>
/// <param name="red_channel">If true (with bpp == 1), replace each pixel with its
/// palette entry's red component, turning the image into raw intensity
/// values.</param>
/// <returns>True on success, false if the file could not be read.</returns>
bool SurfaceCacheClass::CachePCX(char const *name, int bpp, int red_channel)
{
	/*
	 * The 16-bit palette scratch buffer doubles as the storage for the old
	 * cache entry fetched below.
	 */
	unsigned short pal16[sizeof(SurfaceCacheEntry) / 2];
	SurfaceCacheEntry &old_entry = *(SurfaceCacheEntry *)&pal16[0];
	RGBClass pal[256];
	SurfaceCacheEntry new_entry;
	CCFileClass file(name);

	Surface *pcx = Read_PCX_File(file, (PaletteClass *)&pal);
	Surface *surf = pcx;
	if (pcx == NULL) {
		return(false);
	}
	if (bpp == 2) {

		/*
		 * Convert the palette into a table of ready-made 16-bit pixels, then
		 * blit the paletted image into a new 16-bit surface through it.
		 */
		int i;
		for (i = 0; i < 256; i++) {
			RGBClass & rgb = pal[i];
			pal16[i] = ((rgb.Get_Red() >> DSurface::RedLeft) << DSurface::RedRight)
				| ((rgb.Get_Green() >> DSurface::GreenLeft) << DSurface::GreenRight)
				| ((rgb.Get_Blue() >> DSurface::BlueLeft) << DSurface::BlueRight);
		}

		Rect rect = pcx->Get_Rect();
		surf = new BSurface(rect.Width, rect.Height, 2);
		unsigned short *s = (unsigned short *)surf->Lock();
		unsigned char *p = (unsigned char *)pcx->Lock();
		for (i = 0; i < rect.Width * rect.Height; i++) {
			s[i] = pal16[p[i]];
		}
		pcx->Unlock();
		surf->Unlock();
		delete pcx;
	} else if (bpp == 1) {

		/*
		 * An 8-bit surface keeps its palette in the cache entry.
		 */
		memcpy(new_entry.palette, pal, sizeof(pal));
	}

	/*
	 * Optionally reduce a paletted image to its red channel.
	 */
	if (red_channel && bpp == 1) {
		Rect rect = surf->Get_Rect();
		unsigned char *s = (unsigned char *)surf->Lock();
		for (int i = 0; i < rect.Width * rect.Height; i++) {
			s[i] = pal[s[i]].Get_Red();
		}
		surf->Unlock();
	}
	new_entry.surf = surf;
	Wstring key(name);
	key.toLower();

	/*
	 * If an entry is already cached under this name, remove it and delete
	 * its surface before adding the new one.
	 */
	old_entry.surf = NULL;
	if (getValue(key, old_entry)) {
		Surface *oldsurf = old_entry.surf;
		old_entry.surf = NULL;
		remove(key, old_entry);
		if (oldsurf != NULL) {
			delete oldsurf;
		}
	}

	add(key, new_entry);

	return(true);
}


/// <summary>
/// Caches a .PCX file as an 8-bit surface with its pixels reduced to their
/// palette red components (see CachePCX).
/// </summary>
/// <param name="name">File name of the .PCX; also the cache key.</param>
/// <returns>True on success, false if the file could not be read.</returns>
bool SurfaceCacheClass::CachePalettedPCX(char const *name)
{
	return(CachePCX(name, 1, true));
}


/// <summary>
/// Looks up a cached surface by name. The surface remains owned by the cache.
/// </summary>
/// <param name="name">Name the surface was cached under.</param>
/// <param name="palette">If not NULL, receives a copy of the entry's 768-byte
/// palette (meaningful for surfaces cached with bpp == 1).</param>
/// <returns>The cached surface, or NULL if the name is not cached.</returns>
Surface * SurfaceCacheClass::GetSurface(char const * name, void * palette)
{
	/*
	 * The looked-up entry is copied into a function-local static rather than
	 * a 772-byte stack local.
	 */
	Wstring n(name);
	static SurfaceCacheEntry _x;

	_x.surf = NULL;

	SurfaceCacheEntry *p = &_x;
	if (getPointer(n, &p)) {
		if (p != NULL) {
			_x = *p;
		}
		if (palette != NULL) {
			memcpy(palette, _x.palette, sizeof(_x.palette));
		}
		return(_x.surf);
	}
	return(NULL);
}


/// <summary>
/// Fills the destination rectangle from a 16-bit source surface, wrapping the
/// source with modulo arithmetic so it tiles endlessly. A source larger than
/// the rectangle is centered over it.
/// </summary>
/// <param name="rect">Destination rectangle on the target surface.</param>
/// <param name="tosurface">Destination surface to draw onto.</param>
/// <param name="fromsurface">16-bit source surface to tile.</param>
/// <param name="x">Horizontal scroll offset into the source.</param>
/// <param name="y">Vertical scroll offset into the source.</param>
/// <returns>True on success, false if a surface lock failed.</returns>
bool SurfaceCacheClass::Draw(Rect const & rect, Surface & tosurface, Surface & fromsurface, int x, int y)
{
	unsigned short *tbuf = (unsigned short *)tosurface.Lock();
	if (tbuf == NULL) {
		return(false);
	}

	unsigned short *fbuf = (unsigned short *)fromsurface.Lock();
	if (fbuf == NULL) {
		tosurface.Unlock();
		return(false);
	}

	/*
	 * When the source is larger than the rectangle, start sampling at its
	 * centering margin so the middle of the image shows through.
	 */
	int twidth = tosurface.Stride() / 2;
	int fwidth = fromsurface.Stride() / 2;
	Rect frect = fromsurface.Get_Rect();
	Rect trect = tosurface.Get_Rect();
	int ox = (frect.Width - rect.Width) / 2 <= 0 ? 0 : (frect.Width - rect.Width) / 2;
	int oy = (frect.Height - rect.Height) / 2 <= 0 ? 0 : (frect.Height - rect.Height) / 2;
	for (int ny = 0; ny < rect.Height; ny++) {
		unsigned short *t = tbuf + rect.X + twidth * (rect.Y + ny);
		for (int nx = 0; nx < rect.Width; nx++) {
			*t = fbuf[fwidth * ((oy + y + ny) % frect.Height) + (ox + x + nx) % frect.Width];
			t++;
		}
	}
	fromsurface.Unlock();
	tosurface.Unlock();
	return(true);
}


/// <summary>
/// Copies a 16-bit source surface to the rectangle's position on the
/// destination, skipping pixels that match the transparent color key. The
/// whole source is copied; the rectangle supplies only the position.
/// </summary>
/// <param name="rect">Position on the target surface (X and Y are used).</param>
/// <param name="tosurface">Destination surface to draw onto.</param>
/// <param name="fromsurface">16-bit source surface.</param>
/// <param name="trans">Pixel value treated as transparent.</param>
/// <returns>Always true.</returns>
bool SurfaceCacheClass::DrawTrans(Rect const & rect, Surface & tosurface, Surface & fromsurface, short trans)
{
	unsigned short *tbuf = (unsigned short *)tosurface.Lock();
	unsigned short *fbuf = (unsigned short *)fromsurface.Lock();
	Rect r = rect;
	int fwidth = fromsurface.Get_Width();
	int fheight = fromsurface.Get_Height();
	int twidth = tosurface.Stride() / 2;

	if (fheight > 0) {
		unsigned short *sptr = (unsigned short *)fbuf;
		int itval = 2 * twidth;
		int h_left = fheight;
		unsigned short *dptr = &tbuf[r.X + twidth * r.Y];

		/*
		 * Copy row by row through scratch cursors, storing only the pixels
		 * that don't match the color key.
		 */
		while (h_left) {
			if (fwidth > 0) {
				int w_left = fwidth;
				unsigned short *scolor = sptr;
				sptr = (unsigned short *)((char *)sptr + 2 * fwidth);
				unsigned short *dcolor = dptr;
				while (w_left) {
					if (*scolor != (unsigned short)trans) {
						*dcolor = *scolor;
					}
					scolor++;
					dcolor++;
					w_left--;
				}
			}
			dptr = (unsigned short *)((char *)dptr + itval);
			h_left--;
		}
	}
	tosurface.Unlock();
	fromsurface.Unlock();
	return(true);
}


/// <summary>
/// Do-nothing draw variant; never draws.
/// </summary>
/// <param name="rect">Unused.</param>
/// <param name="tosurface">Unused.</param>
/// <param name="fromsurface">Unused.</param>
/// <returns>Always false.</returns>
bool SurfaceCacheClass::DrawNullsub(Rect const & rect, Surface & tosurface, Surface & fromsurface)
{
	return(false);
}


/// <summary>
/// Alpha-blends an image onto a destination surface using a mask surface as a
/// per-pixel alpha. Supports optional centering and X/Y offset clipping. When a
/// palette is supplied the source is treated as paletted (8-bit) and converted
/// to 16-bit via a built color table; otherwise the source is read as 16-bit.
/// </summary>
/// <param name="rect">Destination rectangle on the target surface.</param>
/// <param name="tosurface">Destination surface to draw onto.</param>
/// <param name="fromsurface">Source image surface.</param>
/// <param name="masksurface">Mask surface providing the per-pixel alpha.</param>
/// <param name="palette">Optional palette (3 bytes per entry); NULL for 16-bit source.</param>
/// <param name="center">If true, center the image within the destination rectangle.</param>
/// <param name="x_offset">Horizontal scroll/clip offset.</param>
/// <param name="y_offset">Vertical scroll/clip offset.</param>
/// <returns>True on success, false if a surface lock failed.</returns>
bool SurfaceCacheClass::DrawMasked(Rect const & rect, Surface & tosurface, Surface & fromsurface, Surface & masksurface, void * palette, bool center, int x_offset, int y_offset)
{
	unsigned short pal16[256];

	Rect image_rect = fromsurface.Get_Rect();
	int src_y = 0;
	int src_x = 0;
	int image_w = image_rect.Width;
	int top_clip = 0;
	int image_width = image_w;
	int right_clip = 0;
	int image_h = image_rect.Height;
	int image_height = image_h;
	int src_y2 = 0;
	int rect_width = rect.Width;
	int dst_x_center = 0;
	int dst_y_center = 0;

	/*
	 * Work out the horizontal fit: an image wider than the rectangle is
	 * clipped (split between both sides when centering), a narrower one is
	 * optionally centered in the destination.
	 */
	if (image_w > rect_width) {
		if (center) {
			int diff = image_w - rect_width;
			src_x = (diff + 1) / 2;
			right_clip = diff / 2;
		} else {
			right_clip = image_w - rect_width;
		}
	} else if (center) {
		dst_x_center = (rect_width - image_w) / 2;
	}

	/*
	 * The same for the vertical fit.
	 */
	int rect_height = rect.Height;
	if (image_h > rect_height) {
		if (center) {
			top_clip = (image_h - rect_height + 1) / 2;
			src_y = (image_h - rect_height) / 2;
			src_y2 = src_y;
		} else {
			src_y = image_h - rect_height;
			src_y2 = image_h - rect_height;
		}
	} else if (center) {
		dst_y_center = (rect_height - image_h) / 2;
	}

	/*
	 * Scroll offsets shift the visible window further into the source.
	 */
	if (x_offset > src_x) {
		int newclip = src_x - x_offset + right_clip;
		src_x = x_offset;
		right_clip = newclip;
	}
	if (y_offset > top_clip) {
		int delta = top_clip - y_offset;
		top_clip = y_offset;
		src_y += delta;
		src_y2 = src_y;
	}

	unsigned short * dest = (unsigned short *)tosurface.Lock();
	if (dest == NULL) {
		return(false);
	}

	unsigned char * source = NULL;
	unsigned char * palsource = NULL;
	if (palette != NULL) {
		palsource = (unsigned char *)fromsurface.Lock();
		if (palsource == NULL) {
			tosurface.Unlock();
			return(false);
		}
	} else {
		source = (unsigned char *)fromsurface.Lock();
		if (source == NULL) {
			tosurface.Unlock();
			return(false);
		}
	}

	unsigned char * mask = (unsigned char *)masksurface.Lock();

	/*
	 * For a paletted source, convert the supplied palette into a table of
	 * ready-made 16-bit pixels and restart from the unscrolled clip values.
	 */
	if (palette != NULL) {
		unsigned char * pal = (unsigned char *)palette;
		for (int i = 0; i < 256; i++) {
			pal16[i] = DSurface::Build_Hicolor_Pixel(pal[0], pal[1], pal[2]);
			pal += 3;
		}
		src_y = src_y2;
		image_h = image_height;
	}

	int dst_stride = tosurface.Stride() / 2;
	unsigned int src_stride = fromsurface.Stride();
	if (palette == NULL) {
		src_stride >>= 1;
	}

	/*
	 * Blend the visible rows. Each source pixel is alpha-blended onto the
	 * destination with the mask byte as its alpha; zero mask bytes are
	 * skipped entirely. The paletted arm reads the source pixel through its
	 * fixed distance from the mask pointer and converts it via the table.
	 */
	int cur_y = top_clip;
	int row_count = image_h - src_y;
	if (top_clip < row_count) {
		unsigned char * mask_row = mask + src_x + top_clip * src_stride;
		unsigned short * source_row = (unsigned short *)(source + 2 * (src_x + top_clip * src_stride));
		do {
			int dst_index = dst_stride * (rect.Y - top_clip + cur_y + dst_y_center) + rect.X + dst_x_center;
			if (palette == NULL) {
				if (src_x < image_width - right_clip) {
					unsigned char * mptr = mask_row;
					unsigned short * sptr = source_row;
					unsigned short * dptr = dest + dst_index;
					int count = image_width - right_clip - src_x;
					do {
						if (*mptr != 0) {
							*dptr = OD_Blend_Color(*dptr, *sptr, *mptr);
						}
						dptr++;
						mptr++;
						sptr++;
						count--;
					} while (count != 0);
				}
			} else if (src_x < image_width - right_clip) {
				unsigned char * mptr = mask_row;
				int source_delta = (int)(palsource - mask);
				unsigned short * dptr = dest + dst_index;
				int count = image_width - right_clip - src_x;
				do {
					if (*mptr != 0) {
						*dptr = OD_Blend_Color(*dptr, pal16[(unsigned char)mptr[source_delta]], *mptr);
					}
					dptr++;
					mptr++;
					count--;
				} while (count != 0);
			}
			mask_row += src_stride;
			source_row += src_stride;
			cur_y++;
		} while (cur_y < row_count);
	}

	tosurface.Unlock();
	fromsurface.Unlock();
	masksurface.Unlock();
	return(true);
}
