/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include <cstdint>
#include <memory>
#include <span>
#include <vector>

// Raw deflate streams (RFC 1951, no zlib wrapper).

// Compresses one stream, appending output to out as it becomes ready. Every call returns false
// once the compressor has failed.
class DeflaterClass
{
	public:
		DeflaterClass(void);
		~DeflaterClass(void);
		DeflaterClass(DeflaterClass const &) = delete;
		DeflaterClass & operator = (DeflaterClass const &) = delete;

		bool Write(std::span<std::uint8_t const> data, std::vector<std::uint8_t> & out);

		// Appends output that inflates to everything written so far.
		bool Sync_Flush(std::vector<std::uint8_t> & out);

		// Ends the stream with a final block; nothing may be written after it.
		bool Finish(std::vector<std::uint8_t> & out);

	private:
		bool Run(std::span<std::uint8_t const> data, int flush, std::vector<std::uint8_t> & out);

		struct StreamType;
		std::unique_ptr<StreamType> Stream;
		std::vector<std::uint8_t> Buffer;
		bool Failed;
		bool Finished;
};

enum class InflateStatus : std::uint8_t {
	MORE,     // every byte given was used and the stream has not ended
	DONE,     // the final block ended
	BROKEN,   // the input is not valid deflate data
	LIMIT,    // the output passed the limit
	MEMORY,   // memory ran out
};

// Decompresses one stream from input that arrives in pieces. Input cut short is not an error:
// Write returns MORE after appending everything the input so far decodes to.
class InflaterClass
{
	public:
		explicit InflaterClass(std::uint64_t limit);
		~InflaterClass(void);
		InflaterClass(InflaterClass const &) = delete;
		InflaterClass & operator = (InflaterClass const &) = delete;

		InflateStatus Write(std::span<std::uint8_t const> data, std::vector<std::uint8_t> & out);
		std::uint64_t Total_Out(void) const { return(Total); }

		// The input bytes used; after DONE, the stream ends there and the rest is unused.
		std::uint64_t Total_In(void) const { return(Used); }

		// Replaces the output limit, which must not be below Total_Out.
		void Set_Limit(std::uint64_t limit) { Limit = limit; }

	private:
		struct StreamType;
		std::unique_ptr<StreamType> Stream;
		std::vector<std::uint8_t> Buffer;
		std::uint64_t Limit;
		std::uint64_t Total;
		std::uint64_t Used;
		InflateStatus Status;
};

// Appends data, compressed as one complete stream, to out.
bool Deflate_All(std::span<std::uint8_t const> data, std::vector<std::uint8_t> & out);

// Replaces out with one complete stream decompressed; MORE means the input ended first.
InflateStatus Inflate_All(std::span<std::uint8_t const> data, std::vector<std::uint8_t> & out, std::uint64_t limit);
