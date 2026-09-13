/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "ui/rml/rmlimage.h"

#include <cstring>


// A PCX header is 128 bytes, and an 8-bit one carries its palette in the last 769, behind
// a marker byte.
static const std::size_t UI_PCX_HEADER_BYTES = 128;
static const std::size_t UI_PCX_PALETTE_BYTES = 768;
static const std::uint8_t UI_PCX_PALETTE_MARKER = 0x0C;

// The largest picture the loader will build, which is well past anything the interface
// holds and keeps a malformed header from asking for a gigabyte.
static const int UI_PCX_MAX_DIMENSION = 4096;


static int Read_16(std::span<std::uint8_t const> bytes, std::size_t offset)
{
	return((int)bytes[offset] | ((int)bytes[offset + 1] << 8));
}


bool UI_Decode_PCX(std::span<std::uint8_t const> encoded, UIImageIndexed & image)
{
	image = UIImageIndexed();

	if (encoded.size() < UI_PCX_HEADER_BYTES + UI_PCX_PALETTE_BYTES + 1) {
		return(false);
	}

	// Identifier, run-length encoding, eight bits a pixel, one plane. Anything else is a
	// PCX this loader does not read.
	if (encoded[0] != 10 || encoded[2] != 1 || encoded[3] != 8 || encoded[65] != 1) {
		return(false);
	}

	int width = Read_16(encoded, 8) - Read_16(encoded, 4) + 1;
	int height = Read_16(encoded, 10) - Read_16(encoded, 6) + 1;
	int bytesperline = Read_16(encoded, 66);

	if (width <= 0 || height <= 0 || width > UI_PCX_MAX_DIMENSION || height > UI_PCX_MAX_DIMENSION || bytesperline < width) {
		return(false);
	}

	std::size_t palette = encoded.size() - UI_PCX_PALETTE_BYTES - 1;
	if (encoded[palette] != UI_PCX_PALETTE_MARKER) {
		return(false);
	}

	std::vector<std::uint8_t> pixels((std::size_t)width * (std::size_t)height);
	std::size_t source = UI_PCX_HEADER_BYTES;

	for (int row = 0; row < height; row++) {
		std::uint8_t * line = pixels.data() + (std::size_t)row * width;

		for (int produced = 0; produced < bytesperline; ) {
			if (source >= palette) {
				return(false);
			}

			std::uint8_t value = encoded[source++];
			int count = 1;

			if ((value & 0xC0) == 0xC0) {
				count = value & 0x3F;
				if (count == 0 || source >= palette) {
					return(false);
				}
				value = encoded[source++];
			}

			// A row is padded to an even length and a run may reach into that padding, so
			// what falls past the picture's own width is counted and dropped. The row holds
			// exactly that many bytes, so this bound is also what keeps a padded run inside
			// the picture.
			for (int step = 0; step < count && produced + step < width; step++) {
				line[produced + step] = value;
			}
			produced += count;
		}
	}

	image.Width = width;
	image.Height = height;
	image.Pixels = std::move(pixels);
	std::memcpy(image.Palette, encoded.data() + palette + 1, UI_PCX_PALETTE_BYTES);
	return(true);
}


bool UI_Indexed_To_RGBA(UIImageIndexed const & image, std::vector<std::uint8_t> & rgba)
{
	rgba.clear();

	if (image.Width <= 0 || image.Height <= 0
		|| image.Pixels.size() != (std::size_t)image.Width * (std::size_t)image.Height) {
		return(false);
	}

	rgba.resize(image.Pixels.size() * 4);

	for (std::size_t pixel = 0; pixel < image.Pixels.size(); pixel++) {
		std::uint8_t const * color = image.Palette + (std::size_t)image.Pixels[pixel] * 3;
		std::uint8_t * out = rgba.data() + pixel * 4;

		if (color[0] == 255 && color[1] == 0 && color[2] == 255) {
			out[0] = 0;
			out[1] = 0;
			out[2] = 0;
			out[3] = 0;
			continue;
		}

		out[0] = color[0];
		out[1] = color[1];
		out[2] = color[2];
		out[3] = 255;
	}

	return(true);
}
