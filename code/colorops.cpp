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

#include <cstdint>

namespace {

/*
 * How one hicolor layout packs a colour. The mask keeps the bits a channel is allowed to
 * carry, and the shift moves them into place; red and green shift up, blue shifts down.
 */
struct PackFormat {
	std::uint32_t RedMask;
	std::uint32_t RedShift;
	std::uint32_t GreenMask;
	std::uint32_t GreenShift;
	std::uint32_t BlueMask;
	std::uint32_t BlueShift;
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
/// <returns>std::uint32_t; The scaled channel, at most 255.</returns>
inline std::uint32_t Scale_Channel(std::uint32_t channel, std::uint32_t scale)
{
	std::uint32_t const scaled = (std::uint32_t)(channel * scale) >> 16;
	return((scaled > 255) ? 255 : scaled);
}


void Adjust_Color(std::uint8_t const * palette, std::uint16_t * translator, std::int32_t red, std::int32_t green,
	std::int32_t blue, std::int32_t intensity, std::uint8_t const * mask, PackFormat const & format)
{
	/*
	 * Index zero is the transparent one and is never scaled.
	 */
	translator[0] = 0;

	for (std::int32_t i = 1; i < 256; i++) {
		std::uint32_t const r = palette[i * 3 + 0];
		std::uint32_t const g = palette[i * 3 + 1];
		std::uint32_t const b = palette[i * 3 + 2];

		std::uint32_t redscale = (std::uint32_t)intensity;
		std::uint32_t greenscale = (std::uint32_t)intensity;
		std::uint32_t bluescale = (std::uint32_t)intensity;

		if (mask[i] != 0) {
			redscale = (std::uint32_t)red;
			greenscale = (std::uint32_t)green;
			bluescale = (std::uint32_t)blue;
		}

		std::uint32_t const outr = Scale_Channel(r, redscale);
		std::uint32_t const outg = Scale_Channel(g, greenscale);
		std::uint32_t const outb = Scale_Channel(b, bluescale);

		translator[i] = (std::uint16_t)(((outr & format.RedMask) << format.RedShift)
			| ((outg & format.GreenMask) << format.GreenShift)
			| ((outb & format.BlueMask) >> format.BlueShift));
	}
}


/// <summary>
/// Adds two channel values, holding the result at 255 rather than letting it wrap.
/// </summary>
/// <param name="left">One value.</param>
/// <param name="right">The other.</param>
/// <returns>std::uint32_t; The sum, at most 255.</returns>
inline std::uint32_t Add_Saturated(std::uint32_t left, std::uint32_t right)
{
	std::uint32_t const sum = (left & 0xFF) + (right & 0xFF);
	return((sum > 255) ? 255 : sum);
}


/*
 * How the table-driven brightening puts a pixel back together. The channels arrive already
 * separated, so only the reassembly differs between layouts.
 */
struct BrightenFormat {
	std::uint32_t Down;
	std::uint32_t BlueDown;
	std::uint32_t GreenUp;
	std::uint32_t RedUp;
	std::uint32_t Mask;
};

/*
 * The 655 entry carries 0x423A0A60 where the others carry a channel mask. The assembly treated
 * that value as a marker selecting a different reassembly, and then masked green with it as
 * well. It is preserved because the recorded output depends on it, not because it reads like a
 * mask anyone intended.
 */
std::uint32_t const ALTERNATE_MARKER = 0x423A0A60;

BrightenFormat const _Brighten565 = {2, 1, 5, 10, 0xF8};
BrightenFormat const _Brighten555 = {3, 0, 5, 10, 0x7C};
BrightenFormat const _Brighten556 = {2, 0, 6, 10, 0xF8};
BrightenFormat const _Brighten655 = {2, 1, 4, 10, ALTERNATE_MARKER};


void Brighten_Color(std::uint8_t const * mulbuffer, std::uint16_t * colorbuffer, std::int32_t mulbuffwidth,
	std::int32_t colorbuffwidth, std::int32_t width, std::int32_t height, std::int32_t const * colortable,
	BrightenFormat const & format)
{
	std::uint8_t const * mulrow = mulbuffer;
	std::uint8_t * colorrow = (std::uint8_t *)colorbuffer;

	for (std::int32_t y = 0; y < height; y++) {
		std::uint8_t const * mul = mulrow;
		std::uint16_t * color = (std::uint16_t *)colorrow;

		for (std::int32_t x = 0; x < width; x++) {
			std::uint32_t const multiplier = *mul;

			if (multiplier != 0) {
				std::uint32_t const pixel = *color;
				std::uint32_t const entry = (std::uint32_t)colortable[pixel];

				/*
				 * The table holds the three channels one per byte, which the assembly
				 * widened to a word each before scaling them together.
				 */
				std::uint32_t const blue = entry & 0xFF;
				std::uint32_t const green = (entry >> 8) & 0xFF;
				std::uint32_t const red = (entry >> 16) & 0xFF;

				std::uint32_t const outblue = Add_Saturated((blue * multiplier) >> 8, blue) >> format.Down;
				std::uint32_t const outgreen = Add_Saturated((green * multiplier) >> 8, green) >> format.Down;
				std::uint32_t const outred = Add_Saturated((red * multiplier) >> 8, red) >> format.Down;

				std::uint32_t result = outblue >> format.BlueDown;
				std::uint32_t const greenpart = outgreen << format.GreenUp;
				std::uint32_t const redpart = outred << format.RedUp;

				if (format.Mask == ALTERNATE_MARKER) {
					result |= (greenpart & ALTERNATE_MARKER);
					result |= redpart;
				} else {
					result |= greenpart;
					result |= (redpart & (format.Mask * 256));
				}

				*color = (std::uint16_t)result;
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

void __cdecl Adjust_Color_565(void * palette, void * translator, std::int32_t red, std::int32_t green, std::int32_t blue, std::int32_t intensity, void * mask)
{
	Adjust_Color((std::uint8_t const *)palette, (std::uint16_t *)translator, red, green, blue, intensity, (std::uint8_t const *)mask, _Format565);
}


void __cdecl Adjust_Color_555(void * palette, void * translator, std::int32_t red, std::int32_t green, std::int32_t blue, std::int32_t intensity, void * mask)
{
	Adjust_Color((std::uint8_t const *)palette, (std::uint16_t *)translator, red, green, blue, intensity, (std::uint8_t const *)mask, _Format555);
}


void __cdecl Adjust_Color_556(void * palette, void * translator, std::int32_t red, std::int32_t green, std::int32_t blue, std::int32_t intensity, void * mask)
{
	Adjust_Color((std::uint8_t const *)palette, (std::uint16_t *)translator, red, green, blue, intensity, (std::uint8_t const *)mask, _Format556);
}


void __cdecl Adjust_Color_655(void * palette, void * translator, std::int32_t red, std::int32_t green, std::int32_t blue, std::int32_t intensity, void * mask)
{
	Adjust_Color((std::uint8_t const *)palette, (std::uint16_t *)translator, red, green, blue, intensity, (std::uint8_t const *)mask, _Format655);
}


void __cdecl Brighten_Color_565(std::uint8_t * mulbuffer, std::uint16_t * colorbuffer, std::int32_t mulbuffwidth, std::int32_t colorbuffwidth, std::int32_t width, std::int32_t height, std::int32_t * colortable)
{
	Brighten_Color(mulbuffer, colorbuffer, mulbuffwidth, colorbuffwidth, width, height, colortable, _Brighten565);
}


void __cdecl Brighten_Color_555(std::uint8_t * mulbuffer, std::uint16_t * colorbuffer, std::int32_t mulbuffwidth, std::int32_t colorbuffwidth, std::int32_t width, std::int32_t height, std::int32_t * colortable)
{
	Brighten_Color(mulbuffer, colorbuffer, mulbuffwidth, colorbuffwidth, width, height, colortable, _Brighten555);
}


void __cdecl Brighten_Color_556(std::uint8_t * mulbuffer, std::uint16_t * colorbuffer, std::int32_t mulbuffwidth, std::int32_t colorbuffwidth, std::int32_t width, std::int32_t height, std::int32_t * colortable)
{
	Brighten_Color(mulbuffer, colorbuffer, mulbuffwidth, colorbuffwidth, width, height, colortable, _Brighten556);
}


void __cdecl Brighten_Color_655(std::uint8_t * mulbuffer, std::uint16_t * colorbuffer, std::int32_t mulbuffwidth, std::int32_t colorbuffwidth, std::int32_t width, std::int32_t height, std::int32_t * colortable)
{
	Brighten_Color(mulbuffer, colorbuffer, mulbuffwidth, colorbuffwidth, width, height, colortable, _Brighten655);
}

}	// extern "C"
