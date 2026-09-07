/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/


#pragma once

#include <cstddef>
#include <string>
#include <string_view>


/*
 * UTF-8 is the engine's text encoding. Nothing here throws: a malformed or truncated
 * sequence decodes as REPLACEMENT and consumes one byte.
 */
namespace UTF8
{
	constexpr char32_t REPLACEMENT = 0xFFFD;
	constexpr int MAX_SEQUENCE = 4;
	constexpr std::string_view BOM = "\xEF\xBB\xBF";

	// Sequences.
	bool Is_Continuation(unsigned char byte);
	int Sequence_Length(unsigned char lead);
	char32_t Decode(char const * & text);
	char32_t Decode(char * & text);
	char32_t Peek(char const * text, int & length);
	char32_t Peek(char const * text);
	int Encode(char32_t code, char * out);
	char const * Previous(char const * begin, char const * text);
	char * Previous(char * begin, char * text);
	bool Is_Valid(std::string_view text);
	bool Is_Printable(char32_t code);
	std::size_t BOM_Length(std::string_view text);

	// Cuts that never split a sequence.
	std::size_t Copy(char * dest, std::size_t size, char const * source);
	std::size_t Boundary_Before(char const * text, std::size_t limit);

	// The code pages of legacy text files and of the shipped fonts.
	std::string From_Windows_1252(std::string_view text);
	std::string To_Windows_1252(std::string_view text);
	int Windows_1252_Index(char32_t code);
	int OEM_437_Glyph(char32_t code);
	int Windows_1252_Glyph(char32_t code);
}
