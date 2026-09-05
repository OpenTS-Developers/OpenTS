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
 * UTF-8 is the engine's text encoding: resource strings, INI text, typed input, player
 * names and chat are UTF-8 bytes. These helpers decode and encode it, keep cuts on
 * sequence boundaries, and map code points onto the code pages the shipped fonts and
 * legacy text files use. Nothing here throws: a malformed or truncated sequence decodes
 * as REPLACEMENT and consumes one byte.
 */
namespace UTF8
{
	constexpr char32_t REPLACEMENT = 0xFFFD;
	constexpr int MAX_SEQUENCE = 4;

	bool Is_Continuation(unsigned char byte);

	/// <summary>
	/// Returns the byte length a lead byte announces, or 0 for a byte that cannot lead.
	/// </summary>
	int Sequence_Length(unsigned char lead);

	/// <summary>
	/// Decodes the code point at text and advances past it.
	/// </summary>
	char32_t Decode(char const * & text);
	char32_t Decode(char * & text);

	/// <summary>
	/// Decodes the code point at text without advancing; length receives the bytes it spans.
	/// </summary>
	char32_t Peek(char const * text, int & length);
	char32_t Peek(char const * text);

	/// <summary>
	/// Writes the encoding of code into out, which needs MAX_SEQUENCE bytes, and returns the
	/// count. A surrogate or out-of-range value encodes as REPLACEMENT.
	/// </summary>
	int Encode(char32_t code, char * out);

	/// <summary>
	/// Returns the start of the sequence before text, never earlier than begin.
	/// </summary>
	char const * Previous(char const * begin, char const * text);
	char * Previous(char * begin, char * text);

	bool Is_Valid(std::string_view text);

	/// <summary>
	/// Tells whether code draws as a character: not a control, the delete, or the C1 range.
	/// </summary>
	bool Is_Printable(char32_t code);

	std::string From_Windows_1252(std::string_view text);

	/// <summary>
	/// Transcodes to Windows-1252, writing '?' for every code point the code page lacks.
	/// </summary>
	std::string To_Windows_1252(std::string_view text);

	/// <summary>
	/// Returns the Windows-1252 byte for code, or -1.
	/// </summary>
	int Windows_1252_Index(char32_t code);

	/// <summary>
	/// Returns the code page 437 byte for code, or -1. Control positions never map, and a
	/// close visual match is accepted for what the code page lacks.
	/// </summary>
	int OEM_437_Index(char32_t code);

	/// <summary>
	/// Returns the glyph index of code in a Westwood font laid out as code page 437 with
	/// Westwood's own additions, or -1.
	/// </summary>
	int Font_Index_437(char32_t code);

	/// <summary>
	/// Returns the glyph index of code in a font laid out as Windows-1252, or -1. A close
	/// visual match is accepted for what the code page lacks.
	/// </summary>
	int Font_Index_1252(char32_t code);

	/// <summary>
	/// Copies source into dest, at most size - 1 bytes and never ending inside a sequence,
	/// and returns the bytes copied. dest is always terminated when size is not zero.
	/// </summary>
	std::size_t Copy(char * dest, std::size_t size, char const * source);

	/// <summary>
	/// Returns the largest byte count no greater than limit at which text can be cut without
	/// splitting a sequence.
	/// </summary>
	std::size_t Boundary_Before(char const * text, std::size_t limit);
}
