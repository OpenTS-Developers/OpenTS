/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include <miniz/miniz.h>

#include "png.h"

static int Failures = 0;


static void Check(bool condition, char const * what)
{
	if (!condition) {
		Failures++;
		std::printf("FAIL: %s\n", what);
	}
}


// An in-memory file, so the encoder is tested without the file layer or a temporary folder.
class MemoryFileClass : public FileClass
{
	public:
		std::vector<unsigned char> Bytes;

		virtual char const * File_Name(void) const override {return("memory");}
		virtual char const * Set_Name(char const *) override {return(File_Name());}
		virtual int Create(void) override {Bytes.clear(); return(1);}
		virtual int Delete(void) override {Bytes.clear(); return(1);}
		virtual bool Is_Available(int = false) override {return(true);}
		virtual bool Is_Open(void) const override {return(IsOpen);}
		virtual int Open(char const *, int rights = READ) override {return(Open(rights));}
		virtual int Open(int = READ) override {IsOpen = true; return(1);}
		virtual int Read(void *, int) override {return(0);}
		virtual int Seek(int, int = SEEK_CUR) override {return(0);}
		virtual int Size(void) override {return((int)Bytes.size());}
		virtual void Close(void) override {IsOpen = false;}
		virtual void Error(int, int = false, char const * = NULL) override {}

		virtual int Write(void const * buffer, int size) override
		{
			unsigned char const * from = (unsigned char const *)buffer;
			Bytes.insert(Bytes.end(), from, from + size);
			return(size);
		}

	private:
		bool IsOpen = false;
};


static unsigned int Big_Endian(unsigned char const * where)
{
	return(((unsigned int)where[0] << 24) | ((unsigned int)where[1] << 16)
		| ((unsigned int)where[2] << 8) | (unsigned int)where[3]);
}


/*
 * A chunk as the file holds it: its four letter name, its payload, and whether the checksum
 * the file carries agrees with the payload.
 */
struct ChunkStruct
{
	std::string Name;
	std::vector<unsigned char> Data;
	bool CrcAgrees;
};


static bool Split_Chunks(std::vector<unsigned char> const & file, std::vector<ChunkStruct> & chunks)
{
	static unsigned char const signature[8] = {0x89, 'P', 'N', 'G', 0x0D, 0x0A, 0x1A, 0x0A};

	if (file.size() < sizeof(signature) || std::memcmp(file.data(), signature, sizeof(signature)) != 0) {
		return(false);
	}

	size_t at = sizeof(signature);
	while (at + 12 <= file.size()) {
		unsigned int const size = Big_Endian(&file[at]);
		if (at + 12 + size > file.size()) {
			return(false);
		}

		ChunkStruct chunk;
		chunk.Name.assign((char const *)&file[at + 4], 4);
		chunk.Data.assign(file.begin() + at + 8, file.begin() + at + 8 + size);

		mz_ulong crc = mz_crc32(MZ_CRC32_INIT, &file[at + 4], 4 + size);
		chunk.CrcAgrees = ((unsigned int)crc == Big_Endian(&file[at + 8 + size]));

		chunks.push_back(chunk);
		at += 12 + size;
	}

	return(at == file.size());
}


static void Test_A_Picture_Round_Trips(void)
{
	int const width = 37;
	int const height = 11;

	// A stride wider than the picture, which is what a Windows bitmap row rounds up to.
	int const stride = (width * 2) + 6;

	std::vector<unsigned char> source((size_t)stride * height, 0xCD);
	for (int y = 0; y < height; y++) {
		unsigned short * row = (unsigned short *)&source[(size_t)y * stride];
		for (int x = 0; x < width; x++) {
			row[x] = (unsigned short)(((x & 0x1F) << 11) | ((y & 0x3F) << 5) | ((x + y) & 0x1F));
		}
	}

	MemoryFileClass file;
	Check(Write_PNG_File(file, width, height, stride, (unsigned short const *)source.data()),
		"a picture is written");
	Check(!file.Is_Open(), "and the file this routine opened is closed again");

	std::vector<ChunkStruct> chunks;
	Check(Split_Chunks(file.Bytes, chunks), "the signature is there and the chunks span the file exactly");
	Check(chunks.size() >= 3, "there is a header, a picture and an end");

	for (ChunkStruct const & chunk : chunks) {
		Check(chunk.CrcAgrees, "every chunk carries a checksum that agrees with it");
	}

	Check(chunks.front().Name == "IHDR", "the header comes first");
	Check(chunks.back().Name == "IEND", "the end comes last");
	Check(chunks.back().Data.empty(), "and carries nothing");

	std::vector<unsigned char> const & header = chunks.front().Data;
	Check(header.size() == 13, "the header is the size the format fixes");
	Check((int)Big_Endian(&header[0]) == width, "the header states the width");
	Check((int)Big_Endian(&header[4]) == height, "the header states the height");
	Check(header[8] == 8, "eight bits a component");
	Check(header[9] == 2, "three components and no palette");
	Check(header[10] == 0 && header[11] == 0 && header[12] == 0,
		"deflated, filtered the one way the format allows, and not interlaced");

	std::vector<unsigned char> deflated;
	for (ChunkStruct const & chunk : chunks) {
		if (chunk.Name == "IDAT") {
			deflated.insert(deflated.end(), chunk.Data.begin(), chunk.Data.end());
		}
	}
	Check(!deflated.empty(), "the picture reaches the file");
	Check(deflated.size() < (size_t)width * height * 3,
		"and is smaller deflated than the pixels it came from");

	size_t const rowsize = (size_t)width * 3;
	std::vector<unsigned char> raw((rowsize + 1) * height);
	mz_ulong produced = (mz_ulong)raw.size();
	Check(mz_uncompress(raw.data(), &produced, deflated.data(), (mz_ulong)deflated.size()) == MZ_OK,
		"the picture inflates");
	Check(produced == raw.size(), "back to one filter byte and one row of pixels per line");

	std::vector<unsigned char> prior(rowsize, 0);
	bool matches = true;
	for (int y = 0; y < height && matches; y++) {
		unsigned char const * line = &raw[(size_t)y * (rowsize + 1)];
		if (line[0] != 2) {
			matches = false;
			break;
		}

		for (size_t index = 0; index < rowsize; index++) {
			prior[index] = (unsigned char)(line[1 + index] + prior[index]);
		}

		unsigned short const * from = (unsigned short const *)&source[(size_t)y * stride];
		for (int x = 0; x < width && matches; x++) {
			unsigned short const value = from[x];
			unsigned char const red = (unsigned char)(((value >> 11) & 0x1F) * 255 / 31);
			unsigned char const green = (unsigned char)(((value >> 5) & 0x3F) * 255 / 63);
			unsigned char const blue = (unsigned char)((value & 0x1F) * 255 / 31);

			matches = prior[x * 3] == red && prior[x * 3 + 1] == green && prior[x * 3 + 2] == blue;
		}
	}
	Check(matches, "and every pixel comes back the colour it went in, the padding ignored");
}


static void Test_A_Picture_With_Nothing_In_It_Is_Refused(void)
{
	unsigned short pixel = 0;
	MemoryFileClass file;

	Check(!Write_PNG_File(file, 0, 4, 2, &pixel), "no width is refused");
	Check(!Write_PNG_File(file, 4, 0, 8, &pixel), "no height is refused");
	Check(!Write_PNG_File(file, 4, 4, 8, NULL), "no pixels are refused");
	Check(file.Bytes.empty(), "and none of them reaches the file");
}


int main(void)
{
	Test_A_Picture_Round_Trips();
	Test_A_Picture_With_Nothing_In_It_Is_Refused();

	if (Failures != 0) {
		std::printf("%d check(s) failed.\n", Failures);
		return(1);
	}

	std::printf("PNG contract verified.\n");
	return(0);
}
