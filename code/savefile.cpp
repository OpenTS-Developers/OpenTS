/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "savefile.h"

#include <lzo/lzo1x.h>

#include <array>
#include <cstdint>
#include <cstring>
#include <new>
#include <string>

namespace {

unsigned char const Signature[4] = { 'O', 'T', 'S', 'V' };

constexpr std::uint32_t FLAG_LZO = 0x0001;
constexpr std::uint32_t FIELD_HEADER_SIZE = 8;
constexpr std::uint32_t MAX_FIELD_LENGTH = 0x10000;
// No game state comes near this, and a header asking for more is asking for memory.
constexpr std::uint32_t MAX_CONTENT_LENGTH = 0x10000000;
// A listing is a dozen short fields; a table beyond this is not one.
constexpr std::uint32_t MAX_TABLE_LENGTH = 0x100000;


std::uint32_t Get_U16(unsigned char const * from)
{
	return((std::uint32_t)from[0] | ((std::uint32_t)from[1] << 8));
}


std::uint32_t Get_U32(unsigned char const * from)
{
	return((std::uint32_t)from[0] | ((std::uint32_t)from[1] << 8)
		| ((std::uint32_t)from[2] << 16) | ((std::uint32_t)from[3] << 24));
}


void Put_U16(unsigned char * into, std::uint32_t value)
{
	into[0] = (unsigned char)(value & 0xFF);
	into[1] = (unsigned char)((value >> 8) & 0xFF);
}


void Put_U32(unsigned char * into, std::uint32_t value)
{
	into[0] = (unsigned char)(value & 0xFF);
	into[1] = (unsigned char)((value >> 8) & 0xFF);
	into[2] = (unsigned char)((value >> 16) & 0xFF);
	into[3] = (unsigned char)((value >> 24) & 0xFF);
}


void Append(std::vector<unsigned char> & into, void const * data, std::size_t length)
{
	unsigned char const * bytes = (unsigned char const *)data;
	into.insert(into.end(), bytes, bytes + length);
}


// Sizes a buffer the header asked for, and says so rather than throw when the process
// cannot hold it.
bool Reserve(std::vector<unsigned char> & buffer, std::size_t length)
{
	try {
		buffer.resize(length);
	} catch (std::bad_alloc const &) {
		buffer.clear();
		return(false);
	}
	return(true);
}


// The CRC-32 table of IEEE 802.3, folded at build time rather than on the first save.
constexpr std::array<std::uint32_t, 256> Make_CRC_Table(void)
{
	std::array<std::uint32_t, 256> table{};

	for (std::uint32_t index = 0; index < 256; index++) {
		std::uint32_t value = index;
		for (int bit = 0; bit < 8; bit++) {
			value = (value & 1) ? (0xEDB88320u ^ (value >> 1)) : (value >> 1);
		}
		table[index] = value;
	}

	return(table);
}

constexpr std::array<std::uint32_t, 256> CRCTable = Make_CRC_Table();


bool Read_Range(HANDLE file, void * into, std::uint32_t length)
{
	unsigned char * cursor = (unsigned char *)into;

	while (length > 0) {
		DWORD got = 0;
		if (!ReadFile(file, cursor, length, &got, nullptr) || got == 0) return(false);
		cursor += got;
		length -= got;
	}

	return(true);
}


bool Write_Range(HANDLE file, void const * data, std::uint32_t length)
{
	unsigned char const * cursor = (unsigned char const *)data;

	while (length > 0) {
		DWORD const block = (length > 0x100000) ? 0x100000 : length;
		DWORD written = 0;
		if (!WriteFile(file, cursor, block, &written, nullptr) || written != block) return(false);
		cursor += written;
		length -= written;
	}

	return(true);
}


struct HeaderType {
	std::uint32_t Version;
	std::uint32_t Flags;
	std::uint32_t TableLength;
	std::uint32_t ContentOffset;
	std::uint32_t StoredLength;
	std::uint32_t ContentLength;
	std::uint32_t ContentCRC;
	std::uint32_t HeaderCRC;
};


// The header checksum continues over the field table, so a listing can verify what it
// reads without touching the content.
std::uint32_t Header_CRC(unsigned char const * header, unsigned char const * table, std::uint32_t length)
{
	return(SaveFileClass::Checksum(table, length, SaveFileClass::Checksum(header, SaveFileClass::HEADER_SIZE - 4)));
}


// Decides everything the first 32 bytes can decide, in the order a caller wants to
// hear about it: not ours, a version we do not read, or damage.
SaveFileClass::ResultType Parse_Header(unsigned char const * bytes, std::uint32_t available, HeaderType & header)
{
	if (available < sizeof(Signature) || memcmp(bytes, Signature, sizeof(Signature)) != 0) {
		return(SaveFileClass::RESULT_NOT_A_SAVE);
	}
	if (available < SaveFileClass::HEADER_SIZE) {
		return(SaveFileClass::RESULT_CORRUPT);
	}

	header.Version = Get_U16(bytes + 4);
	header.Flags = Get_U16(bytes + 6);
	header.TableLength = Get_U32(bytes + 8);
	header.ContentOffset = Get_U32(bytes + 12);
	header.StoredLength = Get_U32(bytes + 16);
	header.ContentLength = Get_U32(bytes + 20);
	header.ContentCRC = Get_U32(bytes + 24);
	header.HeaderCRC = Get_U32(bytes + 28);

	if (header.Version == 0 || header.Version > SaveFileClass::FORMAT_VERSION) {
		return(SaveFileClass::RESULT_UNSUPPORTED_VERSION);
	}
	if ((header.Flags & ~FLAG_LZO) != 0) {
		return(SaveFileClass::RESULT_UNSUPPORTED_VERSION);
	}
	if (header.TableLength > MAX_TABLE_LENGTH) {
		return(SaveFileClass::RESULT_CORRUPT);
	}
	if (header.ContentOffset != SaveFileClass::HEADER_SIZE + header.TableLength) {
		return(SaveFileClass::RESULT_CORRUPT);
	}
	if (header.StoredLength > MAX_CONTENT_LENGTH || header.ContentLength > MAX_CONTENT_LENGTH) {
		return(SaveFileClass::RESULT_CORRUPT);
	}

	return(SaveFileClass::RESULT_OK);
}

}	// namespace


SaveFileClass::SaveFileClass(void)
{
}


std::uint32_t SaveFileClass::Checksum(unsigned char const * data, std::uint32_t length, std::uint32_t seed)
{
	std::uint32_t crc = ~seed;

	for (std::uint32_t index = 0; index < length; index++) {
		crc = CRCTable[(crc ^ data[index]) & 0xFF] ^ (crc >> 8);
	}

	return(~crc);
}


char const * SaveFileClass::Result_Text(ResultType result)
{
	switch (result) {
		case RESULT_OK: return("ok");
		case RESULT_MISSING: return("the file is missing");
		case RESULT_NOT_A_SAVE: return("the file is not a saved game");
		case RESULT_UNSUPPORTED_VERSION: return("the file uses a format version this build does not read");
		case RESULT_CORRUPT: return("the file is damaged");
		case RESULT_WRITE_FAILED: return("the file could not be written");
		case RESULT_NO_MEMORY: return("there is not enough memory to read the file");
		case RESULT_TOO_LARGE: return("the game state is larger than a saved game can hold");
	}
	return("unknown");
}


SaveFileClass::FieldType const * SaveFileClass::Find(int id, int kind) const
{
	for (FieldType const & field : Fields) {
		if (field.ID == id && field.Kind == kind) return(&field);
	}
	return(nullptr);
}


void SaveFileClass::Set(int id, int kind, void const * data, std::size_t length)
{
	for (FieldType & field : Fields) {
		if (field.ID == id && field.Kind == kind) {
			field.Bytes.assign((unsigned char const *)data, (unsigned char const *)data + length);
			return;
		}
	}

	FieldType field;
	field.ID = id;
	field.Kind = kind;
	field.Bytes.assign((unsigned char const *)data, (unsigned char const *)data + length);
	Fields.push_back(field);
}


void SaveFileClass::Set_String(int id, char const * text)
{
	if (text == nullptr) text = "";
	Set(id, FIELD_STRING, text, strlen(text));
}


void SaveFileClass::Set_Int(int id, int value)
{
	unsigned char bytes[4];
	Put_U32(bytes, (std::uint32_t)value);
	Set(id, FIELD_INT, bytes, sizeof(bytes));
}


void SaveFileClass::Set_Time(int id, FILETIME const & time)
{
	unsigned char bytes[8];
	Put_U32(bytes, time.dwLowDateTime);
	Put_U32(bytes + 4, time.dwHighDateTime);
	Set(id, FIELD_TIME, bytes, sizeof(bytes));
}


// A string that does not fit is truncated to what does; the result is always terminated.
bool SaveFileClass::Get_String(int id, char * text, int size) const
{
	if (text == nullptr || size <= 0) return(false);

	FieldType const * const field = Find(id, FIELD_STRING);
	if (field == nullptr) {
		text[0] = '\0';
		return(false);
	}

	std::size_t length = field->Bytes.size();
	if (length > (std::size_t)(size - 1)) {
		// A cut never splits a UTF-8 sequence, so a shortened description stays text.
		length = (std::size_t)(size - 1);
		while (length > 0 && (field->Bytes[length] & 0xC0) == 0x80) length--;
	}
	memcpy(text, field->Bytes.data(), length);
	text[length] = '\0';

	return(true);
}


bool SaveFileClass::Get_Int(int id, int * value) const
{
	FieldType const * const field = Find(id, FIELD_INT);
	if (field == nullptr || field->Bytes.size() != 4) return(false);

	if (value != nullptr) *value = (int)Get_U32(field->Bytes.data());
	return(true);
}


bool SaveFileClass::Get_Time(int id, FILETIME * time) const
{
	FieldType const * const field = Find(id, FIELD_TIME);
	if (field == nullptr || field->Bytes.size() != 8) return(false);

	if (time != nullptr) {
		time->dwLowDateTime = Get_U32(field->Bytes.data());
		time->dwHighDateTime = Get_U32(field->Bytes.data() + 4);
	}
	return(true);
}


void SaveFileClass::Clear_Fields(void)
{
	Fields.clear();
}


void SaveFileClass::Serialize_Fields(std::vector<unsigned char> & table) const
{
	table.clear();

	for (FieldType const & field : Fields) {
		unsigned char head[FIELD_HEADER_SIZE];
		Put_U16(head, (std::uint32_t)field.ID);
		Put_U16(head + 2, (std::uint32_t)field.Kind);
		Put_U32(head + 4, (std::uint32_t)field.Bytes.size());
		Append(table, head, sizeof(head));
		Append(table, field.Bytes.data(), field.Bytes.size());
	}
}


SaveFileClass::ResultType SaveFileClass::Parse_Fields(unsigned char const * table, std::uint32_t length)
{
	Fields.clear();

	std::uint32_t offset = 0;
	while (offset < length) {
		if (length - offset < FIELD_HEADER_SIZE) return(RESULT_CORRUPT);

		FieldType field;
		field.ID = (int)Get_U16(table + offset);
		field.Kind = (int)Get_U16(table + offset + 2);
		std::uint32_t const bytes = Get_U32(table + offset + 4);
		offset += FIELD_HEADER_SIZE;

		if (bytes > MAX_FIELD_LENGTH || bytes > length - offset) return(RESULT_CORRUPT);
		field.Bytes.assign(table + offset, table + offset + bytes);
		offset += bytes;

		Fields.push_back(field);
	}

	return(RESULT_OK);
}


// The file lands under its final name only once every byte is on disk, so a save
// interrupted at any point leaves the previous file untouched.
SaveFileClass::ResultType SaveFileClass::Write(char const * path) const
{
	if (path == nullptr) return(RESULT_WRITE_FAILED);

	// The reader's limits bind the writer too, so a save this build writes is one it reads,
	// and one it cannot write leaves the file on disk alone.
	if (Content.size() > MAX_CONTENT_LENGTH) return(RESULT_TOO_LARGE);
	for (FieldType const & field : Fields) {
		if (field.Bytes.size() > MAX_FIELD_LENGTH) return(RESULT_TOO_LARGE);
	}

	std::vector<unsigned char> table;
	Serialize_Fields(table);
	if (table.size() > MAX_TABLE_LENGTH) return(RESULT_TOO_LARGE);

	std::vector<unsigned char> stored;
	std::uint32_t flags = 0;

	if (!Content.empty()) {
		std::vector<unsigned char> work(LZO1X_MEM_COMPRESS);
		stored.resize(Content.size() + Content.size() / 16 + 64 + 3);

		lzo_uint packed = 0;
		int const status = lzo1x_1_compress(Content.data(), (lzo_uint)Content.size(),
			stored.data(), &packed, work.data());

		if (status == LZO_E_OK && packed < Content.size()) {
			stored.resize((std::size_t)packed);
			flags |= FLAG_LZO;
		} else {
			stored = Content;
		}
	}

	std::vector<unsigned char> image(HEADER_SIZE);
	unsigned char * const header = image.data();
	memcpy(header, Signature, sizeof(Signature));
	Put_U16(header + 4, FORMAT_VERSION);
	Put_U16(header + 6, flags);
	Put_U32(header + 8, (std::uint32_t)table.size());
	Put_U32(header + 12, HEADER_SIZE + (std::uint32_t)table.size());
	Put_U32(header + 16, (std::uint32_t)stored.size());
	Put_U32(header + 20, (std::uint32_t)Content.size());
	Put_U32(header + 24, Checksum(stored.data(), (std::uint32_t)stored.size()));
	Put_U32(header + 28, Header_CRC(header, table.data(), (std::uint32_t)table.size()));

	image.insert(image.end(), table.begin(), table.end());
	image.insert(image.end(), stored.begin(), stored.end());

	std::string const temporary = std::string(path) + ".tmp";

	HANDLE const file = CreateFileA(temporary.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL, nullptr);
	if (file == INVALID_HANDLE_VALUE) return(RESULT_WRITE_FAILED);

	bool ok = Write_Range(file, image.data(), (std::uint32_t)image.size());
	if (ok) ok = (FlushFileBuffers(file) != FALSE);
	if (!CloseHandle(file)) ok = false;

	if (ok) ok = (MoveFileExA(temporary.c_str(), path, MOVEFILE_REPLACE_EXISTING) != FALSE);

	if (!ok) {
		DeleteFileA(temporary.c_str());
		return(RESULT_WRITE_FAILED);
	}

	return(RESULT_OK);
}


SaveFileClass::ResultType SaveFileClass::Read(char const * path)
{
	Fields.clear();
	Content.clear();

	if (path == nullptr) return(RESULT_MISSING);

	HANDLE const file = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL, nullptr);
	if (file == INVALID_HANDLE_VALUE) return(RESULT_MISSING);

	// The header is judged before anything the file's size could ask for is allocated.
	unsigned char head[HEADER_SIZE];
	DWORD got = 0;
	bool const ok = (ReadFile(file, head, HEADER_SIZE, &got, nullptr) != FALSE);

	HeaderType header;
	ResultType result = ok ? Parse_Header(head, got, header) : RESULT_CORRUPT;

	std::vector<unsigned char> image;
	if (result == RESULT_OK) {
		DWORD const size = GetFileSize(file, nullptr);
		if (size == INVALID_FILE_SIZE || size != header.ContentOffset + header.StoredLength) {
			result = RESULT_CORRUPT;
		} else if (!Reserve(image, size)) {
			result = RESULT_NO_MEMORY;
		} else {
			memcpy(image.data(), head, HEADER_SIZE);
			if (!Read_Range(file, image.data() + HEADER_SIZE, size - HEADER_SIZE)) result = RESULT_CORRUPT;
		}
	}
	CloseHandle(file);
	if (result != RESULT_OK) return(result);

	if (Header_CRC(image.data(), image.data() + HEADER_SIZE, header.TableLength) != header.HeaderCRC) {
		return(RESULT_CORRUPT);
	}

	result = Parse_Fields(image.data() + HEADER_SIZE, header.TableLength);
	if (result != RESULT_OK) return(result);

	unsigned char const * const stored = image.data() + header.ContentOffset;
	if (Checksum(stored, header.StoredLength) != header.ContentCRC) {
		Fields.clear();
		return(RESULT_CORRUPT);
	}

	if ((header.Flags & FLAG_LZO) != 0) {
		if (!Reserve(Content, header.ContentLength)) {
			Fields.clear();
			return(RESULT_NO_MEMORY);
		}

		lzo_uint unpacked = (lzo_uint)Content.size();
		int const status = lzo1x_decompress_safe(stored, (lzo_uint)header.StoredLength,
			Content.data(), &unpacked, nullptr);

		if (status != LZO_E_OK || unpacked != header.ContentLength) {
			Fields.clear();
			Content.clear();
			return(RESULT_CORRUPT);
		}
	} else {
		if (header.StoredLength != header.ContentLength) {
			Fields.clear();
			return(RESULT_CORRUPT);
		}
		if (!Reserve(Content, header.StoredLength)) {
			Fields.clear();
			return(RESULT_NO_MEMORY);
		}
		memcpy(Content.data(), stored, header.StoredLength);
	}

	return(RESULT_OK);
}


// Reads the header and the field table only, so listing a folder of saves touches a
// few hundred bytes of each file.
SaveFileClass::ResultType SaveFileClass::Read_Fields(char const * path)
{
	Fields.clear();
	Content.clear();

	if (path == nullptr) return(RESULT_MISSING);

	HANDLE const file = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL, nullptr);
	if (file == INVALID_HANDLE_VALUE) return(RESULT_MISSING);

	unsigned char head[HEADER_SIZE];
	DWORD got = 0;
	bool ok = (ReadFile(file, head, HEADER_SIZE, &got, nullptr) != FALSE);

	HeaderType header;
	ResultType result = ok ? Parse_Header(head, got, header) : RESULT_CORRUPT;

	std::vector<unsigned char> table;
	if (result == RESULT_OK && header.TableLength > 0) {
		DWORD const size = GetFileSize(file, nullptr);
		if (size == INVALID_FILE_SIZE || header.TableLength > size - HEADER_SIZE) {
			result = RESULT_CORRUPT;
		} else if (!Reserve(table, header.TableLength)) {
			result = RESULT_NO_MEMORY;
		} else {
			if (!Read_Range(file, table.data(), header.TableLength)) result = RESULT_CORRUPT;
		}
	}
	CloseHandle(file);

	if (result != RESULT_OK) return(result);
	if (Header_CRC(head, table.data(), (std::uint32_t)table.size()) != header.HeaderCRC) return(RESULT_CORRUPT);

	return(Parse_Fields(table.data(), (std::uint32_t)table.size()));
}
