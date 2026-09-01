/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// Round-trips data through LZOPipe and LZOStraw, the compression path save games
// travel. The compressor's dictionary holds pointers, so its byte size differs
// between 32-bit and LP64 targets; an undersized allocation corrupts the stream
// this test would then fail to expand. Needs no game data.

#include <cstdio>
#include <cstring>
#include <vector>

#include "lzopipe.h"
#include "lzostraw.h"

namespace {

int Failures = 0;

unsigned int Seed = 0;

unsigned int Next_Random(void)
{
	Seed = Seed * 1103515245u + 12345u;
	return(Seed >> 8);
}


// Collects everything a pipe chain emits.
class CapturePipe : public Pipe
{
	public:
		std::vector<unsigned char> Data;

		virtual int Put(void const * source, int slen) override
		{
			unsigned char const * bytes = (unsigned char const *)source;
			Data.insert(Data.end(), bytes, bytes + slen);
			return(slen);
		}
};


// Serves a fixed buffer to a straw chain.
class MemoryStraw : public Straw
{
	public:
		MemoryStraw(unsigned char const * data, int length) : Data(data), Length(length), Offset(0) {}

		virtual int Get(void * buffer, int slen) override
		{
			int count = (slen < Length - Offset) ? slen : (Length - Offset);
			if (count > 0) {
				memcpy(buffer, Data + Offset, count);
				Offset += count;
			}
			return(count);
		}

	private:
		unsigned char const * Data;
		int Length;
		int Offset;
};


void Fill_Source(std::vector<unsigned char> & source, int shape, int size, unsigned int seed)
{
	Seed = seed;
	source.resize((size_t)size);

	for (int i = 0; i < size; i++) {
		switch (shape) {
			case 0:
				source[(size_t)i] = (unsigned char)Next_Random();
				break;

			case 1:
				source[(size_t)i] = (unsigned char)(i & 0x0F);
				break;

			case 2:
				source[(size_t)i] = (unsigned char)((Next_Random() % 8 == 0) ? Next_Random() : 0x55);
				break;

			default:
				source[(size_t)i] = 0;
				break;
		}
	}
}


void Check_Roundtrip(int shape, int size, unsigned int seed)
{
	std::vector<unsigned char> source;
	Fill_Source(source, shape, size, seed);

	CapturePipe captured;
	{
		LZOPipe compressor(LZOPipe::COMPRESS);
		compressor.Put_To(captured);
		int offset = 0;
		while (offset < size) {
			int chunk = (size - offset < 977) ? (size - offset) : 977;
			compressor.Put(&source[(size_t)offset], chunk);
			offset += chunk;
		}
		compressor.End();
	}

	MemoryStraw stored(captured.Data.data(), (int)captured.Data.size());
	LZOStraw expander(LZOStraw::DECOMPRESS);
	expander.Get_From(stored);

	std::vector<unsigned char> expanded((size_t)size, 0);
	int got = 0;
	while (got < size) {
		int step = expander.Get(&expanded[(size_t)got], size - got);
		if (step <= 0) {
			break;
		}
		got += step;
	}

	if (got != size || memcmp(source.data(), expanded.data(), (size_t)size) != 0) {
		printf("FAIL shape %d size %d: %d of %d bytes back, %s\n", shape, size, got, size,
			(got == size) ? "content differs" : "stream ended short");
		Failures++;
	}
}

}	// namespace


int main(void)
{
	int const sizes[] = { 1, 100, 8191, 8192, 8193, 65536, 250000 };

	for (int shape = 0; shape < 3; shape++) {
		for (int size : sizes) {
			Check_Roundtrip(shape, size, 0x1234u + (unsigned int)shape);
		}
	}

	if (Failures > 0) {
		printf("lzocomp: %d failures\n", Failures);
		return(1);
	}

	printf("lzocomp: all round trips match\n");
	return(0);
}
