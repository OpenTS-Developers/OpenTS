/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "always.h"

#include "png.h"

#include <miniz/miniz.h>

#include <cstring>
#include <memory>


// Deflated output is written as it is produced, so memory use does not grow with the picture.
#define PNG_CHUNK_SIZE 65536

#define PNG_FILTER_UP 2


static unsigned char const _Signature[8] = {0x89, 'P', 'N', 'G', 0x0D, 0x0A, 0x1A, 0x0A};


static void Put_Big_Endian(unsigned char * where, unsigned int value)
{
	where[0] = (unsigned char)(value >> 24);
	where[1] = (unsigned char)(value >> 16);
	where[2] = (unsigned char)(value >> 8);
	where[3] = (unsigned char)value;
}


static bool Write_Chunk(FileClass & file, char const * type, unsigned char const * data, int size)
{
	unsigned char header[8];
	Put_Big_Endian(header, (unsigned int)size);
	std::memcpy(header + 4, type, 4);

	if (file.Write(header, (int)sizeof(header)) != (int)sizeof(header)) {
		return(false);
	}

	if (size > 0 && file.Write(data, size) != size) {
		return(false);
	}

	mz_ulong crc = mz_crc32(MZ_CRC32_INIT, header + 4, 4);
	if (size > 0) {
		crc = mz_crc32(crc, data, (size_t)size);
	}

	unsigned char trailer[4];
	Put_Big_Endian(trailer, (unsigned int)crc);

	return(file.Write(trailer, (int)sizeof(trailer)) == (int)sizeof(trailer));
}


static bool Write_Header(FileClass & file, int width, int height)
{
	if (file.Write(_Signature, (int)sizeof(_Signature)) != (int)sizeof(_Signature)) {
		return(false);
	}

	unsigned char header[13];
	Put_Big_Endian(header, (unsigned int)width);
	Put_Big_Endian(header + 4, (unsigned int)height);
	header[8] = 8;
	header[9] = 2;
	header[10] = 0;
	header[11] = 0;
	header[12] = 0;

	return(Write_Chunk(file, "IHDR", header, (int)sizeof(header)));
}


static bool Write_Rows(FileClass & file, int width, int height, int stride, unsigned short const * pixels)
{
	// The levels are spread across the whole byte, so a component at its maximum reaches 255.
	unsigned char five[32];
	unsigned char six[64];
	for (int level = 0; level < 64; level++) {
		six[level] = (unsigned char)(level * 255 / 63);
		if (level < 32) {
			five[level] = (unsigned char)(level * 255 / 31);
		}
	}

	int const rowsize = width * 3;

	std::unique_ptr<unsigned char[]> row(new (std::nothrow) unsigned char[rowsize + 1]);
	std::unique_ptr<unsigned char[]> prior(new (std::nothrow) unsigned char[rowsize]);
	std::unique_ptr<unsigned char[]> out(new (std::nothrow) unsigned char[PNG_CHUNK_SIZE]);

	if (row == nullptr || prior == nullptr || out == nullptr) {
		return(false);
	}

	std::memset(prior.get(), 0, rowsize);

	mz_stream stream;
	std::memset(&stream, 0, sizeof(stream));

	if (mz_deflateInit(&stream, MZ_DEFAULT_COMPRESSION) != MZ_OK) {
		return(false);
	}

	stream.next_out = out.get();
	stream.avail_out = PNG_CHUNK_SIZE;

	bool ok = true;

	for (int y = 0; ok && y < height; y++) {
		unsigned short const * source = (unsigned short const *)((unsigned char const *)pixels + (size_t)y * (size_t)stride);
		unsigned char * target = row.get() + 1;

		row[0] = PNG_FILTER_UP;

		for (int x = 0; x < width; x++) {
			unsigned short const value = source[x];
			target[0] = five[(value >> 11) & 0x1F];
			target[1] = six[(value >> 5) & 0x3F];
			target[2] = five[value & 0x1F];
			target += 3;
		}

		// The row above the first is taken as zero, which makes this the same as no filter there.
		for (int index = 0; index < rowsize; index++) {
			unsigned char const raw = row[index + 1];
			row[index + 1] = (unsigned char)(raw - prior[index]);
			prior[index] = raw;
		}

		stream.next_in = row.get();
		stream.avail_in = (unsigned int)(rowsize + 1);

		while (ok && stream.avail_in > 0) {
			if (mz_deflate(&stream, MZ_NO_FLUSH) != MZ_OK) {
				ok = false;
				break;
			}

			if (stream.avail_out == 0) {
				ok = Write_Chunk(file, "IDAT", out.get(), PNG_CHUNK_SIZE);
				stream.next_out = out.get();
				stream.avail_out = PNG_CHUNK_SIZE;
			}
		}
	}

	int result = MZ_OK;
	while (ok && result != MZ_STREAM_END) {
		result = mz_deflate(&stream, MZ_FINISH);
		if (result != MZ_OK && result != MZ_STREAM_END) {
			ok = false;
			break;
		}

		int const produced = PNG_CHUNK_SIZE - (int)stream.avail_out;
		if (produced > 0) {
			ok = Write_Chunk(file, "IDAT", out.get(), produced);
			stream.next_out = out.get();
			stream.avail_out = PNG_CHUNK_SIZE;
		} else if (result != MZ_STREAM_END) {
			ok = false;
		}
	}

	mz_deflateEnd(&stream);

	return(ok && Write_Chunk(file, "IEND", NULL, 0));
}


/// <summary>
/// Writes 565 pixels to the file as a PNG, opening and closing it if needed. On failure the
/// caller removes whatever was written.
/// </summary>
/// <param name="stride">The bytes from one row of 565 pixels to the next.</param>
/// <returns>bool; Was the whole picture written?</returns>
bool Write_PNG_File(FileClass & file, int width, int height, int stride, unsigned short const * pixels)
{
	if (width <= 0 || height <= 0 || pixels == NULL) {
		return(false);
	}

	bool opened = false;
	if (!file.Is_Open()) {
		if (file.Open(FileClass::WRITE) == 0) {
			return(false);
		}
		opened = true;
	}

	bool const ok = Write_Header(file, width, height) && Write_Rows(file, width, height, stride, pixels);

	if (opened) {
		file.Close();
	}

	return(ok);
}
