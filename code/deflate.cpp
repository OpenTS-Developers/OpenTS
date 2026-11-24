/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "deflate.h"

#include <miniz/miniz.h>

#include <algorithm>
#include <cstring>
#include <new>


namespace
{

constexpr std::size_t BUFFER_SIZE = 64 * 1024;

// mz_stream counts input in 32 bits, so larger input is handed over in pieces.
constexpr std::size_t MAX_PIECE = 1u << 30;

}


struct DeflaterClass::StreamType {
	mz_stream Stream;
};


struct InflaterClass::StreamType {
	mz_stream Stream;
};


DeflaterClass::DeflaterClass(void) :
	Failed(false),
	Finished(false)
{
	Stream.reset(new (std::nothrow) StreamType);
	if (Stream == nullptr) {
		Failed = true;
		return;
	}
	std::memset(&Stream->Stream, 0, sizeof(Stream->Stream));
	if (mz_deflateInit2(&Stream->Stream, MZ_DEFAULT_COMPRESSION, MZ_DEFLATED, -MZ_DEFAULT_WINDOW_BITS, 9, MZ_DEFAULT_STRATEGY) != MZ_OK) {
		Stream.reset();
		Failed = true;
		return;
	}
	Buffer.resize(BUFFER_SIZE);
}


DeflaterClass::~DeflaterClass(void)
{
	if (Stream != nullptr) {
		mz_deflateEnd(&Stream->Stream);
	}
}


// Runs the compressor until it has taken all of data and leaves room in the buffer, because a
// sync flush can leave bytes inside miniz when the output space runs out.
bool DeflaterClass::Run(std::span<std::uint8_t const> data, int flush, std::vector<std::uint8_t> & out)
{
	if (Failed || Finished) {
		return(false);
	}

	mz_stream & stream = Stream->Stream;
	std::size_t offset = 0;
	for (;;) {
		std::size_t piece = std::min(data.size() - offset, MAX_PIECE);
		stream.next_in = data.data() + offset;
		stream.avail_in = mz_uint32(piece);
		bool last_piece = (offset + piece == data.size());

		for (;;) {
			stream.next_out = Buffer.data();
			stream.avail_out = mz_uint32(Buffer.size());
			int status = mz_deflate(&stream, last_piece ? flush : MZ_NO_FLUSH);
			std::size_t produced = Buffer.size() - stream.avail_out;
			out.insert(out.end(), Buffer.begin(), Buffer.begin() + produced);

			if (status == MZ_STREAM_END) {
				Finished = true;
				return(true);
			}
			if (status != MZ_OK && status != MZ_BUF_ERROR) {
				Failed = true;
				return(false);
			}
			if (stream.avail_in == 0 && stream.avail_out != 0) {
				break;
			}
		}

		offset += piece;
		if (offset >= data.size()) {
			return(true);
		}
	}
}


bool DeflaterClass::Write(std::span<std::uint8_t const> data, std::vector<std::uint8_t> & out)
{
	if (data.empty()) {
		return(!Failed && !Finished);
	}
	return(Run(data, MZ_NO_FLUSH, out));
}


bool DeflaterClass::Sync_Flush(std::vector<std::uint8_t> & out)
{
	return(Run(std::span<std::uint8_t const>(), MZ_SYNC_FLUSH, out));
}


bool DeflaterClass::Finish(std::vector<std::uint8_t> & out)
{
	return(Run(std::span<std::uint8_t const>(), MZ_FINISH, out) && Finished);
}


InflaterClass::InflaterClass(std::uint64_t limit) :
	Limit(limit),
	Total(0),
	Used(0),
	Status(InflateStatus::MORE)
{
	Stream.reset(new (std::nothrow) StreamType);
	if (Stream == nullptr) {
		Status = InflateStatus::MEMORY;
		return;
	}
	std::memset(&Stream->Stream, 0, sizeof(Stream->Stream));
	if (mz_inflateInit2(&Stream->Stream, -MZ_DEFAULT_WINDOW_BITS) != MZ_OK) {
		Stream.reset();
		Status = InflateStatus::MEMORY;
		return;
	}
	try {
		Buffer.resize(BUFFER_SIZE);
	} catch (std::exception const &) {
		Status = InflateStatus::MEMORY;
	}
}


InflaterClass::~InflaterClass(void)
{
	if (Stream != nullptr) {
		mz_inflateEnd(&Stream->Stream);
	}
}


InflateStatus InflaterClass::Write(std::span<std::uint8_t const> data, std::vector<std::uint8_t> & out)
{
	if (Status != InflateStatus::MORE) {
		return(Status);
	}

	mz_stream & stream = Stream->Stream;
	std::size_t offset = 0;
	do {
		std::size_t piece = std::min(data.size() - offset, MAX_PIECE);
		stream.next_in = data.data() + offset;
		stream.avail_in = mz_uint32(piece);

		for (;;) {
			stream.next_out = Buffer.data();
			stream.avail_out = mz_uint32(Buffer.size());
			mz_uint32 const available = stream.avail_in;
			int status = mz_inflate(&stream, MZ_SYNC_FLUSH);
			std::size_t produced = Buffer.size() - stream.avail_out;
			Used += available - stream.avail_in;

			if (produced > Limit - Total) {
				out.insert(out.end(), Buffer.begin(), Buffer.begin() + std::size_t(Limit - Total));
				Total = Limit;
				Status = InflateStatus::LIMIT;
				return(Status);
			}
			out.insert(out.end(), Buffer.begin(), Buffer.begin() + produced);
			Total += produced;

			if (status == MZ_STREAM_END) {
				Status = InflateStatus::DONE;
				return(Status);
			}
			if (status != MZ_OK && status != MZ_BUF_ERROR) {
				Status = InflateStatus::BROKEN;
				return(Status);
			}
			if (stream.avail_in == 0 && stream.avail_out != 0) {
				break;
			}
			if (status == MZ_BUF_ERROR && produced == 0) {
				break;
			}
		}
		offset += piece;
	} while (offset < data.size());

	return(Status);
}


bool Deflate_All(std::span<std::uint8_t const> data, std::vector<std::uint8_t> & out)
{
	DeflaterClass deflater;
	return(deflater.Write(data, out) && deflater.Finish(out));
}


InflateStatus Inflate_All(std::span<std::uint8_t const> data, std::vector<std::uint8_t> & out, std::uint64_t limit)
{
	out.clear();
	InflaterClass inflater(limit);
	return(inflater.Write(data, out));
}
