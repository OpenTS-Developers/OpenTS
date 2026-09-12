/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <cstdint>
#include <vector>

// The file a saved game is kept in: a fixed header, a table of listing fields, and one
// compressed block of game state. docs/SAVE-FORMAT.md records the layout.
class SaveFileClass
{
	public:
		enum ResultType {
			RESULT_OK,
			RESULT_MISSING,				// No file under that name.
			RESULT_NOT_A_SAVE,			// The file does not begin with the signature.
			RESULT_UNSUPPORTED_VERSION,	// A format version, or a header flag, this build does not read.
			RESULT_CORRUPT,				// A length, checksum or block that does not add up.
			RESULT_WRITE_FAILED,		// The file could not be written or moved into place.
			RESULT_NO_MEMORY,			// The file is within its limits but the process cannot hold it.
			RESULT_TOO_LARGE,			// The content or a listing field is more than a save can hold.
		};

		enum {
			FORMAT_VERSION = 1,
			HEADER_SIZE = 32,
		};

		SaveFileClass(void);

		void Set_String(int id, char const * text);
		void Set_Int(int id, int value);
		void Set_Time(int id, FILETIME const & time);
		bool Get_String(int id, char * text, int size) const;
		bool Get_Int(int id, int * value) const;
		bool Get_Time(int id, FILETIME * time) const;
		void Clear_Fields(void);

		ResultType Write(char const * path) const;
		ResultType Read(char const * path);
		ResultType Read_Fields(char const * path);

		static char const * Result_Text(ResultType result);
		static std::uint32_t Checksum(unsigned char const * data, std::uint32_t length, std::uint32_t seed = 0);

		std::vector<unsigned char> Content;

	private:
		enum FieldKind {
			FIELD_STRING = 1,
			FIELD_INT = 2,
			FIELD_TIME = 3,
		};

		struct FieldType {
			int ID;
			int Kind;
			std::vector<unsigned char> Bytes;
		};

		FieldType const * Find(int id, int kind) const;
		void Set(int id, int kind, void const * data, std::size_t length);
		void Serialize_Fields(std::vector<unsigned char> & table) const;
		ResultType Parse_Fields(unsigned char const * table, std::uint32_t length);

		std::vector<FieldType> Fields;
};
