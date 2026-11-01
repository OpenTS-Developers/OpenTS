/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "contentsha256.h"

#include <cstring>


namespace
{

std::uint32_t Rotr(std::uint32_t value, std::uint32_t bits)
{
	return((value >> bits) | (value << (32 - bits)));
}

std::uint32_t const K[64] = {
	0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
	0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
	0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
	0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
	0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
	0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
	0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
	0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2,
};

}


ContentSha256::ContentSha256(void)
{
	Reset();
}


void ContentSha256::Reset(void)
{
	Hash[0] = 0x6a09e667;
	Hash[1] = 0xbb67ae85;
	Hash[2] = 0x3c6ef372;
	Hash[3] = 0xa54ff53a;
	Hash[4] = 0x510e527f;
	Hash[5] = 0x9b05688c;
	Hash[6] = 0x1f83d9ab;
	Hash[7] = 0x5be0cd19;
	Bits = 0;
	Used = 0;
}


void ContentSha256::Transform(unsigned char const block[64])
{
	std::uint32_t word[64];
	for (int index = 0; index < 16; index++) {
		word[index] = ((std::uint32_t)block[index * 4] << 24)
			| ((std::uint32_t)block[index * 4 + 1] << 16)
			| ((std::uint32_t)block[index * 4 + 2] << 8)
			| (std::uint32_t)block[index * 4 + 3];
	}
	for (int index = 16; index < 64; index++) {
		std::uint32_t s0 = Rotr(word[index - 15], 7) ^ Rotr(word[index - 15], 18) ^ (word[index - 15] >> 3);
		std::uint32_t s1 = Rotr(word[index - 2], 17) ^ Rotr(word[index - 2], 19) ^ (word[index - 2] >> 10);
		word[index] = word[index - 16] + s0 + word[index - 7] + s1;
	}

	std::uint32_t a = Hash[0];
	std::uint32_t b = Hash[1];
	std::uint32_t c = Hash[2];
	std::uint32_t d = Hash[3];
	std::uint32_t e = Hash[4];
	std::uint32_t f = Hash[5];
	std::uint32_t g = Hash[6];
	std::uint32_t h = Hash[7];

	for (int index = 0; index < 64; index++) {
		std::uint32_t s1 = Rotr(e, 6) ^ Rotr(e, 11) ^ Rotr(e, 25);
		std::uint32_t choose = (e & f) ^ (~e & g);
		std::uint32_t temp1 = h + s1 + choose + K[index] + word[index];
		std::uint32_t s0 = Rotr(a, 2) ^ Rotr(a, 13) ^ Rotr(a, 22);
		std::uint32_t majority = (a & b) ^ (a & c) ^ (b & c);
		std::uint32_t temp2 = s0 + majority;
		h = g;
		g = f;
		f = e;
		e = d + temp1;
		d = c;
		c = b;
		b = a;
		a = temp1 + temp2;
	}

	Hash[0] += a;
	Hash[1] += b;
	Hash[2] += c;
	Hash[3] += d;
	Hash[4] += e;
	Hash[5] += f;
	Hash[6] += g;
	Hash[7] += h;
}


void ContentSha256::Update(void const * data, std::size_t size)
{
	unsigned char const * bytes = static_cast<unsigned char const *>(data);
	Bits += (std::uint64_t)size * 8;
	while (size > 0) {
		std::size_t room = 64 - Used;
		std::size_t take = size < room ? size : room;
		std::memcpy(Block + Used, bytes, take);
		Used += take;
		bytes += take;
		size -= take;
		if (Used == 64) {
			Transform(Block);
			Used = 0;
		}
	}
}


void ContentSha256::Final(unsigned char out[32])
{
	Block[Used++] = 0x80;
	if (Used > 56) {
		while (Used < 64) {
			Block[Used++] = 0;
		}
		Transform(Block);
		Used = 0;
	}
	while (Used < 56) {
		Block[Used++] = 0;
	}
	for (int shift = 56; shift >= 0; shift -= 8) {
		Block[Used++] = (unsigned char)((Bits >> shift) & 0xff);
	}
	Transform(Block);
	for (int index = 0; index < 8; index++) {
		out[index * 4] = (unsigned char)((Hash[index] >> 24) & 0xff);
		out[index * 4 + 1] = (unsigned char)((Hash[index] >> 16) & 0xff);
		out[index * 4 + 2] = (unsigned char)((Hash[index] >> 8) & 0xff);
		out[index * 4 + 3] = (unsigned char)(Hash[index] & 0xff);
	}
}


std::string Content_Sha256_Hex(unsigned char const digest[32])
{
	static char const digits[] = "0123456789abcdef";
	std::string hex(64, '0');
	for (int index = 0; index < 32; index++) {
		hex[index * 2] = digits[digest[index] >> 4];
		hex[index * 2 + 1] = digits[digest[index] & 0x0f];
	}
	return(hex);
}


bool Content_Sha256_Matches(char const * hex, unsigned char const digest[32])
{
	if (hex == nullptr) {
		return(false);
	}
	std::string actual = Content_Sha256_Hex(digest);
	if (std::strlen(hex) != actual.size()) {
		return(false);
	}
	for (std::size_t index = 0; index < actual.size(); index++) {
		char left = hex[index];
		char right = actual[index];
		if (left >= 'A' && left <= 'F') {
			left = (char)(left - 'A' + 'a');
		}
		if (left != right) {
			return(false);
		}
	}
	return(true);
}
