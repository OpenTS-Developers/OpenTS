/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// Holds LZOPipe and LZOStraw to their buffers on both sides of a block stream, and holds
// LZOStraw::Is_Damaged to the difference between a stream that ended and one that was cut.
// These classes carry the IsoMapPack terrain, the map preview and the save game blocks, so a
// header claiming more than a block must stop at the buffer rather than run past it. Needs no
// game data.

#include <cstdio>
#include <cstring>

#include <lzo/lzo1x.h>

#include "lzopipe.h"
#include "lzostraw.h"
#include "xpipe.h"
#include "xstraw.h"

namespace {

int const BLOCK = 1024 * 8;
int const SRCMAX = BLOCK * 4;
int const DSTMAX = SRCMAX * 2;

// A block of incompressible data comes out larger than it went in. Anything that only reserves
// a block for the compressed side is too small by this much.
int const WORST_CASE = BLOCK + BLOCK / 16 + 64 + 3;

unsigned char Source[SRCMAX];
unsigned char Packed[DSTMAX];
unsigned char Unpacked[SRCMAX + 64];

unsigned int Seed = 1;

int Failures = 0;


unsigned int Next_Random(void)
{
	Seed = Seed * 1103515245u + 12345u;
	return(Seed >> 8);
}


void Report(char const * what, bool ok)
{
	std::printf("%-64s %s\n", what, ok ? "ok" : "FAILED");
	if (!ok) Failures++;
}


void Fill_Random(unsigned char * data, int size)
{
	for (int i = 0; i < size; i++) {
		data[i] = (unsigned char)(Next_Random() & 0xFF);
	}
}


void Fill_Runs(unsigned char * data, int size)
{
	for (int i = 0; i < size; i++) {
		data[i] = (unsigned char)(((i / 64) & 1) ? 0x7E : (Next_Random() & 0xFF));
	}
}


// Random first, so the compressor has to expand at least one block, then runs it can pack down.
void Fill_Mixed(unsigned char * data, int size)
{
	int const half = size / 2;

	Fill_Random(data, half);
	Fill_Runs(&data[half], size - half);
}


void Arm_Guard(int offset)
{
	std::memset(&Unpacked[offset], 0xA5, sizeof(Unpacked) - offset);
}


bool Guard_Intact(int offset)
{
	for (int i = offset; i < (int)sizeof(Unpacked); i++) {
		if (Unpacked[i] != 0xA5) return(false);
	}
	return(true);
}


// Walks a block stream, counting its blocks and returning the offset just past the last whole
// one. Each block is a compressed count, an uncompressed count, and the compressed bytes.
int Walk_Blocks(unsigned char const * stream, int length, int & blocks, int & largest)
{
	int pos = 0;

	blocks = 0;
	largest = 0;
	while (pos + 4 <= length) {
		int const compcount = stream[pos] | (stream[pos + 1] << 8);
		if (pos + 4 + compcount > length) break;
		if (compcount > largest) largest = compcount;
		pos += 4 + compcount;
		blocks++;
	}

	return(pos);
}


int Compress_Stream(unsigned char const * source, int size)
{
	BufferPipe sink(Packed, DSTMAX);
	LZOPipe comp(LZOPipe::COMPRESS);
	comp.Put_To(&sink);

	int packed = comp.Put(source, size);
	packed += comp.Flush();
	return(packed);
}


// A whole stream must come back byte for byte, and the straw must not call that damaged.
void Test_Round_Trip(void)
{
	int const size = BLOCK * 3 + 100;

	Fill_Mixed(Source, size);

	int const packed = Compress_Stream(Source, size);

	int blocks = 0;
	int largest = 0;
	Walk_Blocks(Packed, packed, blocks, largest);

	Arm_Guard(0);

	BufferStraw feed(Packed, packed);
	LZOStraw decomp(LZOStraw::DECOMPRESS);
	decomp.Get_From(&feed);

	int const got = decomp.Get(Unpacked, size);
	unsigned char extra[16];
	int const beyond = decomp.Get(extra, sizeof(extra));

	Report("pipe to straw round trip",
		got == size && beyond == 0 && std::memcmp(Source, Unpacked, size) == 0
		&& Guard_Intact(size) && !decomp.Is_Damaged());
	std::printf("  %d bytes in %d blocks, largest block %d bytes, a block's worst case is %d\n",
		size, blocks, largest, WORST_CASE);
}


// The reverse pairing, fed to the pipe in odd sized pieces so headers straddle calls.
void Test_Straw_To_Pipe(void)
{
	int const size = BLOCK * 2 + 777;

	Fill_Mixed(Source, size);

	BufferStraw raw(Source, size);
	LZOStraw comp(LZOStraw::COMPRESS);
	comp.Get_From(&raw);

	int const packed = comp.Get(Packed, DSTMAX);

	Arm_Guard(0);

	BufferPipe sink(Unpacked, SRCMAX);
	LZOPipe decomp(LZOPipe::DECOMPRESS);
	decomp.Put_To(&sink);

	int got = 0;
	for (int pos = 0; pos < packed; pos += 1003) {
		int const piece = (packed - pos < 1003) ? (packed - pos) : 1003;
		got += decomp.Put(&Packed[pos], piece);
	}
	got += decomp.Flush();

	Report("straw to pipe round trip in odd sized pieces",
		got == size && std::memcmp(Source, Unpacked, size) == 0 && Guard_Intact(size));
}


// A whole block of random bytes is the input the compressor cannot shrink. Both classes have to
// carry the expansion rather than assume a block compresses into a block.
void Test_Incompressible_Block(void)
{
	int const size = BLOCK;

	Fill_Random(Source, size);

	int const packed = Compress_Stream(Source, size);

	int blocks = 0;
	int largest = 0;
	Walk_Blocks(Packed, packed, blocks, largest);

	Arm_Guard(0);

	BufferStraw feed(Packed, packed);
	LZOStraw decomp(LZOStraw::DECOMPRESS);
	decomp.Get_From(&feed);

	int const got = decomp.Get(Unpacked, size);

	Report("a block that will not compress round trips and stays in its buffer",
		got == size && largest > BLOCK && std::memcmp(Source, Unpacked, size) == 0
		&& Guard_Intact(size) && !decomp.Is_Damaged());
	std::printf("  %d bytes of random data compressed to %d\n", size, largest);
}


// Every way of cutting a stream short that leaves a partial block behind has to read as damage.
// Cutting it on a block boundary does not: the blocks that remain are whole, which is why the
// terrain readers also require the pack's own terminator.
void Test_Cut_Stream(void)
{
	int const size = BLOCK * 2 + 500;

	Fill_Mixed(Source, size);

	int const packed = Compress_Stream(Source, size);

	int blocks = 0;
	int largest = 0;
	int const whole = Walk_Blocks(Packed, packed, blocks, largest);

	struct Case {
		char const * what;
		int length;
		bool damaged;
	};

	int const boundary = Walk_Blocks(Packed, whole - 1, blocks, largest);

	Case const cases[] = {
		{"a stream that ends after its last block", packed, false},
		{"a stream cut on a block boundary", boundary, false},
		{"a stream cut inside a block header", boundary + 2, true},
		{"a stream cut inside a block's data", boundary + 6, true},
		{"a stream missing one byte of its last block", packed - 1, true},
	};

	bool ok = true;

	for (Case const & entry : cases) {
		Arm_Guard(0);

		BufferStraw feed(Packed, entry.length);
		LZOStraw decomp(LZOStraw::DECOMPRESS);
		decomp.Get_From(&feed);

		decomp.Get(Unpacked, size);

		if (decomp.Is_Damaged() != entry.damaged || !Guard_Intact(size)) {
			std::printf("  %s: damaged %d, wanted %d, guard %d\n",
				entry.what, (int)decomp.Is_Damaged(), (int)entry.damaged, (int)Guard_Intact(size));
			ok = false;
		}
	}

	Report("the straw separates the end of a stream from a cut one", ok);
}


// Headers the compressor cannot have written. The straw has to stop at each without letting the
// decompressor write past the block it decompresses into.
void Test_Bad_Headers(void)
{
	int const payload = 600;

	Fill_Mixed(Source, payload);

	unsigned char work[LZO1X_1_MEM_COMPRESS];
	lzo_uint packed = 0;
	lzo1x_1_compress(Source, payload, &Packed[4], &packed, work);

	struct Case {
		char const * what;
		unsigned short compcount;
		unsigned short uncompcount;
	};

	Case const cases[] = {
		{"a block that expands to less than its header claims", (unsigned short)packed, payload + 1},
		{"a block that expands to more than its header claims", (unsigned short)packed, payload - 1},
		{"a header claiming more output than a block holds", (unsigned short)packed, 0xFFFF},
		{"a header claiming more compressed bytes than are there", (unsigned short)(packed + 32), payload},
	};

	bool ok = true;

	for (Case const & entry : cases) {
		Packed[0] = (unsigned char)(entry.compcount & 0xFF);
		Packed[1] = (unsigned char)(entry.compcount >> 8);
		Packed[2] = (unsigned char)(entry.uncompcount & 0xFF);
		Packed[3] = (unsigned char)(entry.uncompcount >> 8);

		Arm_Guard(0);

		BufferStraw feed(Packed, 4 + (int)packed);
		LZOStraw decomp(LZOStraw::DECOMPRESS);
		decomp.Get_From(&feed);

		int const got = decomp.Get(Unpacked, SRCMAX);

		if (got != 0 || !decomp.Is_Damaged() || !Guard_Intact(0)) {
			std::printf("  %s: %d bytes, damaged %d, guard %d\n",
				entry.what, got, (int)decomp.Is_Damaged(), (int)Guard_Intact(0));
			ok = false;
		}
	}

	Report("the straw yields nothing behind a header the compressor cannot write", ok);
}


// LZO1X carries no checksum, so a corrupted block that still decodes to its claimed length
// cannot be told from an intact one. What has to hold instead is the bound: every corruption
// either reads as damaged or yields what the header claimed, and neither exceeds a block.
void Test_Corrupted_Block(void)
{
	int const payload = 4000;
	int const ceiling = BLOCK * 2;

	Fill_Mixed(Source, payload);

	unsigned char work[LZO1X_1_MEM_COMPRESS];
	lzo_uint packed = 0;
	lzo1x_1_compress(Source, payload, &Packed[4], &packed, work);

	Packed[0] = (unsigned char)((int)packed & 0xFF);
	Packed[1] = (unsigned char)((int)packed >> 8);
	Packed[2] = (unsigned char)(payload & 0xFF);
	Packed[3] = (unsigned char)(payload >> 8);

	bool ok = true;
	int damaged = 0;
	int decoded = 0;

	for (int index = 0; index < (int)packed; index += 7) {
		unsigned char const saved = Packed[4 + index];
		Packed[4 + index] ^= 0xFF;

		Arm_Guard(ceiling);

		BufferStraw feed(Packed, 4 + (int)packed);
		LZOStraw decomp(LZOStraw::DECOMPRESS);
		decomp.Get_From(&feed);

		int const got = decomp.Get(Unpacked, ceiling);

		if (decomp.Is_Damaged()) {
			damaged++;
			if (got != 0) {
				std::printf("  byte %d: damaged but yielded %d bytes\n", index, got);
				ok = false;
			}
		} else {
			decoded++;
			if (got != payload) {
				std::printf("  byte %d: yielded %d bytes, header claimed %d\n", index, got, payload);
				ok = false;
			}
		}

		if (!Guard_Intact(ceiling)) {
			std::printf("  byte %d: wrote past a block's worth of output\n", index);
			ok = false;
		}

		Packed[4 + index] = saved;
	}

	Report("a corrupted block stays inside a block whether or not it is caught", ok);
	std::printf("  %d corruptions rejected, %d still decoded to their claimed length\n", damaged, decoded);
}


// The pipe accumulates a block's compressed bytes into its own buffer before decompressing, so
// an oversized header has to be refused there rather than at the decompressor. The block size
// is small enough that an unguarded run lands far past the allocation.
void Test_Oversized_Block_Header(void)
{
	int const block = 1024;
	int const claimed = 40000;

	std::memset(Packed, 0x5A, sizeof(Packed));
	Packed[0] = (unsigned char)(claimed & 0xFF);
	Packed[1] = (unsigned char)(claimed >> 8);
	Packed[2] = 0x00;
	Packed[3] = 0x20;

	Arm_Guard(0);

	BufferPipe sink(Unpacked, SRCMAX);
	LZOPipe pipe(LZOPipe::DECOMPRESS, block);
	pipe.Put_To(&sink);

	int put = 0;
	for (int pos = 0; pos < 4 + claimed; pos += 997) {
		int const piece = (4 + claimed - pos < 997) ? (4 + claimed - pos) : 997;
		put += pipe.Put(&Packed[pos], piece);
	}
	put += pipe.Flush();

	Report("the pipe refuses a block larger than it can accumulate", put == 0 && Guard_Intact(0));
}

}	// namespace


int main(void)
{
	if (lzo_init() != LZO_E_OK) {
		std::printf("lzo_init failed\n");
		return(1);
	}

	Test_Round_Trip();
	Test_Straw_To_Pipe();
	Test_Incompressible_Block();
	Test_Cut_Stream();
	Test_Bad_Headers();
	Test_Corrupted_Block();
	Test_Oversized_Block_Header();

	std::printf("%d failures\n", Failures);

	return(Failures == 0 ? 0 : 1);
}
