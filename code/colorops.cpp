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

/****************************************************************************
*
*  File              : winasm.asm
*  Description       : Palette tinting and spot light brightening for each of
*                      the supported hicolor pixel layouts.
*
****************************************************************************/

#include "always.h"

namespace {

/*
 * How one hicolor layout packs a colour. The mask keeps the bits a channel is allowed to
 * carry, and the shift moves them into place; red and green shift up, blue shifts down.
 */
struct PackFormat {
	unsigned int RedMask;
	unsigned int RedShift;
	unsigned int GreenMask;
	unsigned int GreenShift;
	unsigned int BlueMask;
	unsigned int BlueShift;
};

PackFormat const _Format565 = {0xF8, 8, 0xFC, 3, 0xF8, 3};
PackFormat const _Format555 = {0xF8, 7, 0xF8, 2, 0xF8, 3};
PackFormat const _Format556 = {0xF8, 8, 0xF8, 3, 0xFC, 2};
PackFormat const _Format655 = {0xFC, 8, 0xF8, 2, 0xF8, 3};


/// <summary>
/// Scales one channel by a fixed point factor and holds the result at 255. The assembly built
/// the ceiling out of the carry flag; the multiply wraps at 32 bits either way.
/// </summary>
/// <param name="channel">The channel value, 0 to 255.</param>
/// <param name="scale">The 16.16 fixed point factor to scale it by.</param>
/// <returns>unsigned int; The scaled channel, at most 255.</returns>
inline unsigned int Scale_Channel(unsigned int channel, unsigned int scale)
{
	unsigned int const scaled = (unsigned int)(channel * scale) >> 16;
	return((scaled > 255) ? 255 : scaled);
}


void Adjust_Color(unsigned char const * palette, unsigned short * translator, int red, int green, int blue,
	int intensity, unsigned char const * mask, PackFormat const & format)
{
	/*
	 * Index zero is the transparent one and is never scaled.
	 */
	translator[0] = 0;

	for (int i = 1; i < 256; i++) {
		unsigned int const r = palette[i * 3 + 0];
		unsigned int const g = palette[i * 3 + 1];
		unsigned int const b = palette[i * 3 + 2];

		unsigned int redscale = (unsigned int)intensity;
		unsigned int greenscale = (unsigned int)intensity;
		unsigned int bluescale = (unsigned int)intensity;

		if (mask[i] != 0) {
			redscale = (unsigned int)red;
			greenscale = (unsigned int)green;
			bluescale = (unsigned int)blue;
		}

		unsigned int const outr = Scale_Channel(r, redscale);
		unsigned int const outg = Scale_Channel(g, greenscale);
		unsigned int const outb = Scale_Channel(b, bluescale);

		translator[i] = (unsigned short)(((outr & format.RedMask) << format.RedShift)
			| ((outg & format.GreenMask) << format.GreenShift)
			| ((outb & format.BlueMask) >> format.BlueShift));
	}
}


/// <summary>
/// Adds two channel values, holding the result at 255 rather than letting it wrap.
/// </summary>
/// <param name="left">One value.</param>
/// <param name="right">The other.</param>
/// <returns>unsigned int; The sum, at most 255.</returns>
inline unsigned int Add_Saturated(unsigned int left, unsigned int right)
{
	unsigned int const sum = (left & 0xFF) + (right & 0xFF);
	return((sum > 255) ? 255 : sum);
}


/*
 * How the table-driven brightening puts a pixel back together. The channels arrive already
 * separated, so only the reassembly differs between layouts.
 */
struct MmxBrightenFormat {
	unsigned int Down;
	unsigned int BlueDown;
	unsigned int GreenUp;
	unsigned int RedUp;
	unsigned int Mask;
};

/*
 * The 655 entry carries 0x423A0A60 where the others carry a channel mask. The assembly treated
 * that value as a marker selecting a different reassembly, and then masked green with it as
 * well. It is preserved because the recorded output depends on it, not because it reads like a
 * mask anyone intended.
 */
unsigned int const MMX_ALTERNATE_MARKER = 0x423A0A60;

MmxBrightenFormat const _MmxBrighten565 = {2, 1, 5, 10, 0xF8};
MmxBrightenFormat const _MmxBrighten555 = {3, 0, 5, 10, 0x7C};
MmxBrightenFormat const _MmxBrighten556 = {2, 0, 6, 10, 0xF8};
MmxBrightenFormat const _MmxBrighten655 = {2, 1, 4, 10, MMX_ALTERNATE_MARKER};


void MMX_Brighten_Color(unsigned char const * mulbuffer, unsigned short * colorbuffer, int mulbuffwidth,
	int colorbuffwidth, int width, int height, int const * mmxbuffer, MmxBrightenFormat const & format)
{
	unsigned char const * mulrow = mulbuffer;
	unsigned char * colorrow = (unsigned char *)colorbuffer;

	for (int y = 0; y < height; y++) {
		unsigned char const * mul = mulrow;
		unsigned short * color = (unsigned short *)colorrow;

		for (int x = 0; x < width; x++) {
			unsigned int const multiplier = *mul;

			if (multiplier != 0) {
				unsigned int const pixel = *color;
				unsigned int const entry = (unsigned int)mmxbuffer[pixel];

				/*
				 * The table holds the three channels one per byte, which the assembly
				 * widened to a word each before scaling them together.
				 */
				unsigned int const blue = entry & 0xFF;
				unsigned int const green = (entry >> 8) & 0xFF;
				unsigned int const red = (entry >> 16) & 0xFF;

				unsigned int const outblue = Add_Saturated((blue * multiplier) >> 8, blue) >> format.Down;
				unsigned int const outgreen = Add_Saturated((green * multiplier) >> 8, green) >> format.Down;
				unsigned int const outred = Add_Saturated((red * multiplier) >> 8, red) >> format.Down;

				unsigned int result = outblue >> format.BlueDown;
				unsigned int const greenpart = outgreen << format.GreenUp;
				unsigned int const redpart = outred << format.RedUp;

				if (format.Mask == MMX_ALTERNATE_MARKER) {
					result |= (greenpart & MMX_ALTERNATE_MARKER);
				} else {
					result |= greenpart;
					result |= (redpart & (format.Mask * 256));
				}

				if (format.Mask == MMX_ALTERNATE_MARKER) {
					result |= redpart;
				}

				*color = (unsigned short)result;
			}

			mul++;
			color++;
		}

		mulrow += mulbuffwidth;
		colorrow += colorbuffwidth;
	}
}

}	// namespace


extern "C" {

void __cdecl Adjust_Color_565(void * palette, void * translator, int red, int green, int blue, int intensity, void * mask)
{
	Adjust_Color((unsigned char const *)palette, (unsigned short *)translator, red, green, blue, intensity, (unsigned char const *)mask, _Format565);
}


void __cdecl Adjust_Color_555(void * palette, void * translator, int red, int green, int blue, int intensity, void * mask)
{
	Adjust_Color((unsigned char const *)palette, (unsigned short *)translator, red, green, blue, intensity, (unsigned char const *)mask, _Format555);
}


void __cdecl Adjust_Color_556(void * palette, void * translator, int red, int green, int blue, int intensity, void * mask)
{
	Adjust_Color((unsigned char const *)palette, (unsigned short *)translator, red, green, blue, intensity, (unsigned char const *)mask, _Format556);
}


void __cdecl Adjust_Color_655(void * palette, void * translator, int red, int green, int blue, int intensity, void * mask)
{
	Adjust_Color((unsigned char const *)palette, (unsigned short *)translator, red, green, blue, intensity, (unsigned char const *)mask, _Format655);
}


void __cdecl MMX_Brighten_Color_565(unsigned char * mulbuffer, unsigned short * colorbuffer, int mulbuffwidth, int colorbuffwidth, int width, int height, int * mmxbuffer)
{
	MMX_Brighten_Color(mulbuffer, colorbuffer, mulbuffwidth, colorbuffwidth, width, height, mmxbuffer, _MmxBrighten565);
}


void __cdecl MMX_Brighten_Color_555(unsigned char * mulbuffer, unsigned short * colorbuffer, int mulbuffwidth, int colorbuffwidth, int width, int height, int * mmxbuffer)
{
	MMX_Brighten_Color(mulbuffer, colorbuffer, mulbuffwidth, colorbuffwidth, width, height, mmxbuffer, _MmxBrighten555);
}


void __cdecl MMX_Brighten_Color_556(unsigned char * mulbuffer, unsigned short * colorbuffer, int mulbuffwidth, int colorbuffwidth, int width, int height, int * mmxbuffer)
{
	MMX_Brighten_Color(mulbuffer, colorbuffer, mulbuffwidth, colorbuffwidth, width, height, mmxbuffer, _MmxBrighten556);
}


void __cdecl MMX_Brighten_Color_655(unsigned char * mulbuffer, unsigned short * colorbuffer, int mulbuffwidth, int colorbuffwidth, int width, int height, int * mmxbuffer)
{
	MMX_Brighten_Color(mulbuffer, colorbuffer, mulbuffwidth, colorbuffwidth, width, height, mmxbuffer, _MmxBrighten655);
}

}	// extern "C"
