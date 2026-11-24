/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "sha256.h"

#include <cstring>


static constexpr std::uint32_t ROUND_CONSTANTS[64] = {
	0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
	0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
	0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
	0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
	0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
	0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
	0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
	0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2,
};


static std::uint32_t Rotate_Right(std::uint32_t value, int count)
{
	return((value >> count) | (value << (32 - count)));
}


SHA256Class::SHA256Class(void)
{
	Reset();
}


void SHA256Class::Reset(void)
{
	State[0] = 0x6a09e667;
	State[1] = 0xbb67ae85;
	State[2] = 0x3c6ef372;
	State[3] = 0xa54ff53a;
	State[4] = 0x510e527f;
	State[5] = 0x9b05688c;
	State[6] = 0x1f83d9ab;
	State[7] = 0x5be0cd19;
	Length = 0;
	Buffered = 0;
	std::memset(Buffer, 0, sizeof(Buffer));
}


void SHA256Class::Process_Block(std::uint8_t const * block)
{
	std::uint32_t schedule[64];
	for (int i = 0; i < 16; i++) {
		schedule[i] = (std::uint32_t(block[i * 4]) << 24) | (std::uint32_t(block[i * 4 + 1]) << 16)
			| (std::uint32_t(block[i * 4 + 2]) << 8) | std::uint32_t(block[i * 4 + 3]);
	}
	for (int i = 16; i < 64; i++) {
		std::uint32_t s0 = Rotate_Right(schedule[i - 15], 7) ^ Rotate_Right(schedule[i - 15], 18) ^ (schedule[i - 15] >> 3);
		std::uint32_t s1 = Rotate_Right(schedule[i - 2], 17) ^ Rotate_Right(schedule[i - 2], 19) ^ (schedule[i - 2] >> 10);
		schedule[i] = schedule[i - 16] + s0 + schedule[i - 7] + s1;
	}

	std::uint32_t a = State[0];
	std::uint32_t b = State[1];
	std::uint32_t c = State[2];
	std::uint32_t d = State[3];
	std::uint32_t e = State[4];
	std::uint32_t f = State[5];
	std::uint32_t g = State[6];
	std::uint32_t h = State[7];

	for (int i = 0; i < 64; i++) {
		std::uint32_t sum1 = Rotate_Right(e, 6) ^ Rotate_Right(e, 11) ^ Rotate_Right(e, 25);
		std::uint32_t choose = (e & f) ^ (~e & g);
		std::uint32_t temp1 = h + sum1 + choose + ROUND_CONSTANTS[i] + schedule[i];
		std::uint32_t sum0 = Rotate_Right(a, 2) ^ Rotate_Right(a, 13) ^ Rotate_Right(a, 22);
		std::uint32_t majority = (a & b) ^ (a & c) ^ (b & c);
		std::uint32_t temp2 = sum0 + majority;

		h = g;
		g = f;
		f = e;
		e = d + temp1;
		d = c;
		c = b;
		b = a;
		a = temp1 + temp2;
	}

	State[0] += a;
	State[1] += b;
	State[2] += c;
	State[3] += d;
	State[4] += e;
	State[5] += f;
	State[6] += g;
	State[7] += h;
}


void SHA256Class::Hash(void const * data, std::size_t size)
{
	auto bytes = static_cast<std::uint8_t const *>(data);
	Length += size;

	if (Buffered > 0) {
		std::size_t take = sizeof(Buffer) - Buffered;
		if (take > size) {
			take = size;
		}
		std::memcpy(Buffer + Buffered, bytes, take);
		Buffered += take;
		bytes += take;
		size -= take;
		if (Buffered < sizeof(Buffer)) {
			return;
		}
		Process_Block(Buffer);
		Buffered = 0;
	}

	while (size >= sizeof(Buffer)) {
		Process_Block(bytes);
		bytes += sizeof(Buffer);
		size -= sizeof(Buffer);
	}

	if (size > 0) {
		std::memcpy(Buffer, bytes, size);
		Buffered = size;
	}
}


void SHA256Class::Hash(std::span<std::uint8_t const> data)
{
	Hash(data.data(), data.size());
}


/// <summary>
/// Returns the digest of the data hashed since construction or the last Reset. The object is
/// unchanged, so more data can follow.
/// </summary>
SHA256Class::DigestType SHA256Class::Result(void) const
{
	SHA256Class copy = *this;
	std::uint64_t bits = Length * 8;

	std::uint8_t padding[72] = { 0x80 };
	std::size_t pad = (Buffered < 56) ? (56 - Buffered) : (120 - Buffered);
	for (int i = 0; i < 8; i++) {
		padding[pad + i] = std::uint8_t(bits >> (56 - i * 8));
	}
	copy.Hash(padding, pad + 8);

	DigestType digest;
	for (int i = 0; i < 8; i++) {
		digest[i * 4] = std::uint8_t(copy.State[i] >> 24);
		digest[i * 4 + 1] = std::uint8_t(copy.State[i] >> 16);
		digest[i * 4 + 2] = std::uint8_t(copy.State[i] >> 8);
		digest[i * 4 + 3] = std::uint8_t(copy.State[i]);
	}
	return(digest);
}


SHA256Class::DigestType SHA256Class::Digest(void const * data, std::size_t size)
{
	SHA256Class engine;
	engine.Hash(data, size);
	return(engine.Result());
}


SHA256Class::DigestType SHA256Class::Digest(std::span<std::uint8_t const> data)
{
	return(Digest(data.data(), data.size()));
}
