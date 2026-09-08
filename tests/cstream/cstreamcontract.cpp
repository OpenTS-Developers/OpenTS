/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "cstream.h"

#include <lzo/lzo1x.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdio>
#include <vector>

ULONG COMRefCount = 0;

namespace {

int Failures = 0;


void Report(char const * name, bool ok)
{
	std::printf("%-64s %s\n", name, ok ? "ok" : "FAILED");
	if (!ok) Failures++;
}


bool Create_Storage(IStreamPtr & storage)
{
	IStream * stream = nullptr;
	if (FAILED(CreateStreamOnHGlobal(nullptr, TRUE, &stream))) {
		return(false);
	}
	storage.Attach(stream, false);
	return(true);
}


bool Rewind(IStream * storage)
{
	LARGE_INTEGER const start = {};
	return(SUCCEEDED(storage->Seek(start, STREAM_SEEK_SET, nullptr)));
}


std::vector<unsigned char> Make_Source(ULONG size)
{
	std::vector<unsigned char> source(size);
	std::uint32_t seed = 123456789;
	for (unsigned char & value : source) {
		seed ^= seed << 13;
		seed ^= seed >> 17;
		seed ^= seed << 5;
		value = static_cast<unsigned char>(seed & 15);
	}
	return(source);
}


void Test_Roundtrip(bool fragmented, ULONG tail)
{
	std::vector<unsigned char> const source = Make_Source(CStreamClass::BUFFER_SIZE + tail);
	IStreamPtr storage;
	bool ok = Create_Storage(storage);
	if (ok) {
		CStreamClass writer;
		ok = SUCCEEDED(writer.Link_Stream(storage));
		for (ULONG offset = 0; ok && offset < source.size();) {
			ULONG const count = std::min(static_cast<ULONG>(source.size()) - offset, fragmented ? 997UL : static_cast<ULONG>(source.size()));
			ULONG written = 0;
			ok = SUCCEEDED(writer.Write(source.data() + offset, count, &written)) && written == count;
			offset += count;
		}
		ok = SUCCEEDED(writer.Unlink_Stream(nullptr)) && ok;
	}

	std::array<ULONG, 2> header = {};
	if (ok) {
		ULONG read = 0;
		ok = Rewind(storage) && SUCCEEDED(storage->Read(header.data(), sizeof(header), &read)) && read == sizeof(header);
		ok = ok && header[0] > CStreamClass::BUFFER_SIZE && header[0] <= CStreamClass::STREAM_BUFFER_SIZE;
		std::printf("First compressed block: %lu bytes\n", header[0]);
	}

	if (ok) {
		ok = Rewind(storage);
		CStreamClass reader;
		ok = SUCCEEDED(reader.Link_Stream(storage)) && ok;
		std::vector<unsigned char> restored(source.size());
		for (ULONG offset = 0; ok && offset < restored.size();) {
			ULONG const count = std::min(static_cast<ULONG>(restored.size()) - offset, fragmented ? 613UL : static_cast<ULONG>(restored.size()));
			ULONG read = 0;
			ok = SUCCEEDED(reader.Read(restored.data() + offset, count, &read)) && read == count;
			offset += count;
		}
		ok = ok && restored == source;

		unsigned char extra = 0xA5;
		ULONG read = 123;
		ok = FAILED(reader.Read(&extra, sizeof(extra), &read)) && read == 0 && extra == 0xA5 && ok;
	}

	Report(fragmented ? "Fragmented writes and reads with a partial final block" : "Full expanded block and exact end of stream", ok);
}


void Test_Read_Bound(void)
{
	IStreamPtr storage;
	bool ok = Create_Storage(storage);
	if (ok) {
		std::array<ULONG, 2> const header = {CStreamClass::STREAM_BUFFER_SIZE + 1, CStreamClass::BUFFER_SIZE};
		unsigned char const payload = 0;
		ULONG written = 0;
		ok = SUCCEEDED(storage->Write(header.data(), sizeof(header), &written)) && written == sizeof(header);
		ok = SUCCEEDED(storage->Write(&payload, sizeof(payload), &written)) && written == sizeof(payload) && ok;
		ok = Rewind(storage) && ok;

		CStreamClass reader;
		ok = SUCCEEDED(reader.Link_Stream(storage)) && ok;
		unsigned char result = 0xA5;
		ULONG read = 123;
		ok = FAILED(reader.Read(&result, sizeof(result), &read)) && read == 0 && result == 0xA5 && ok;

		LARGE_INTEGER const offset = {};
		ULARGE_INTEGER position = {};
		ok = SUCCEEDED(storage->Seek(offset, STREAM_SEEK_CUR, &position)) && position.QuadPart == sizeof(header) && ok;
	}
	Report("Oversized compressed header rejected before reading payload", ok);
}

}


int main(void)
{
	Report("LZO initialization", lzo_init() == LZO_E_OK);
	Test_Roundtrip(false, 0);
	Test_Roundtrip(true, 137);
	Test_Read_Bound();
	return(Failures == 0 ? 0 : 1);
}
