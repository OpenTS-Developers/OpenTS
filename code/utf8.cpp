/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/


#include "utf8.h"

#include <cstring>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>


namespace {

// The code points at Windows-1252 bytes 0x80 to 0x9F. The five the code page leaves
// undefined keep their own values, so every byte survives a round trip.
constexpr char32_t Windows_1252_High[32] = {
	0x20AC, 0x0081, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021,
	0x02C6, 0x2030, 0x0160, 0x2039, 0x0152, 0x008D, 0x017D, 0x008F,
	0x0090, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014,
	0x02DC, 0x2122, 0x0161, 0x203A, 0x0153, 0x009D, 0x017E, 0x0178,
};


/// <summary>
/// Decodes the sequence at text and reports the bytes it spans. A malformed or truncated
/// sequence reports one byte, REPLACEMENT, and valid false.
/// </summary>
/// <param name="available">Readable bytes at text; the decoder never looks past them.</param>
char32_t Decode_Sequence(char const * text, std::size_t available, int & length, bool & valid)
{
	unsigned char const * bytes = (unsigned char const *)text;
	unsigned char lead = bytes[0];

	valid = true;
	length = 1;
	if (lead < 0x80) {
		return(lead);
	}

	int trailing;
	char32_t code;
	unsigned char low = 0x80;
	unsigned char high = 0xBF;
	if (lead >= 0xC2 && lead <= 0xDF) {
		trailing = 1;
		code = lead & 0x1F;
	} else if (lead >= 0xE0 && lead <= 0xEF) {
		trailing = 2;
		code = lead & 0x0F;
		if (lead == 0xE0) low = 0xA0;
		if (lead == 0xED) high = 0x9F;
	} else if (lead >= 0xF0 && lead <= 0xF4) {
		trailing = 3;
		code = lead & 0x07;
		if (lead == 0xF0) low = 0x90;
		if (lead == 0xF4) high = 0x8F;
	} else {
		valid = false;
		return(UTF8::REPLACEMENT);
	}

	if ((std::size_t)trailing >= available) {
		valid = false;
		return(UTF8::REPLACEMENT);
	}

	for (int index = 1; index <= trailing; index++) {
		unsigned char byte = bytes[index];
		unsigned char min = (index == 1) ? low : 0x80;
		unsigned char max = (index == 1) ? high : 0xBF;
		if (byte < min || byte > max) {
			valid = false;
			return(UTF8::REPLACEMENT);
		}
		code = (code << 6) | (byte & 0x3F);
	}

	length = trailing + 1;
	return(code);
}


// A NUL byte fails every continuation test, so a terminated string needs no explicit bound.
constexpr std::size_t UNBOUNDED = ~(std::size_t)0;


/// <summary>
/// Returns the byte of the given code page that shows code, or -1. Drawing text asks for
/// every glyph on every frame, so the answers are kept in the caller's cache, where 0 marks
/// an unasked slot.
/// </summary>
int Best_Fit_Index(unsigned page, short * cache, char32_t code)
{
	if (code < 0x80) {
		return((int)code);
	}
	if (code > 0xFFFF) {
		return(-1);
	}

	short & slot = cache[code];
	if (slot == 0) {
		wchar_t wide = (wchar_t)code;
		char narrow = 0;
		BOOL defaulted = FALSE;
		int written = WideCharToMultiByte(page, 0, &wide, 1, &narrow, 1, NULL, &defaulted);
		unsigned char byte = (unsigned char)narrow;
		slot = (written == 1 && !defaulted && byte >= 0x20 && byte != 0x7F) ? (short)byte : (short)-1;
	}
	return(slot);
}

}


bool UTF8::Is_Continuation(unsigned char byte)
{
	return((byte & 0xC0) == 0x80);
}


/// <summary>
/// Returns the byte length a lead byte announces, or 0 for a byte that cannot lead.
/// </summary>
int UTF8::Sequence_Length(unsigned char lead)
{
	if (lead < 0x80) return(1);
	if (lead >= 0xC2 && lead <= 0xDF) return(2);
	if (lead >= 0xE0 && lead <= 0xEF) return(3);
	if (lead >= 0xF0 && lead <= 0xF4) return(4);
	return(0);
}


/// <summary>
/// Decodes the code point at text and advances past it.
/// </summary>
char32_t UTF8::Decode(char const * & text)
{
	int length;
	bool valid;
	char32_t code = Decode_Sequence(text, UNBOUNDED, length, valid);
	text += length;
	return(code);
}


char32_t UTF8::Decode(char * & text)
{
	char const * cursor = text;
	char32_t code = Decode(cursor);
	text = const_cast<char *>(cursor);
	return(code);
}


/// <summary>
/// Decodes the code point at text without advancing; length receives the bytes it spans.
/// </summary>
char32_t UTF8::Peek(char const * text, int & length)
{
	bool valid;
	return(Decode_Sequence(text, UNBOUNDED, length, valid));
}


char32_t UTF8::Peek(char const * text)
{
	int length;
	return(Peek(text, length));
}


/// <summary>
/// Writes the encoding of code into out, which needs MAX_SEQUENCE bytes, and returns the
/// count. A surrogate or out-of-range value encodes as REPLACEMENT.
/// </summary>
int UTF8::Encode(char32_t code, char * out)
{
	if (code > 0x10FFFF || (code >= 0xD800 && code <= 0xDFFF)) {
		code = REPLACEMENT;
	}

	if (code < 0x80) {
		out[0] = (char)code;
		return(1);
	}
	if (code < 0x800) {
		out[0] = (char)(0xC0 | (code >> 6));
		out[1] = (char)(0x80 | (code & 0x3F));
		return(2);
	}
	if (code < 0x10000) {
		out[0] = (char)(0xE0 | (code >> 12));
		out[1] = (char)(0x80 | ((code >> 6) & 0x3F));
		out[2] = (char)(0x80 | (code & 0x3F));
		return(3);
	}
	out[0] = (char)(0xF0 | (code >> 18));
	out[1] = (char)(0x80 | ((code >> 12) & 0x3F));
	out[2] = (char)(0x80 | ((code >> 6) & 0x3F));
	out[3] = (char)(0x80 | (code & 0x3F));
	return(4);
}


/// <summary>
/// Returns the start of the sequence before text, never earlier than begin.
/// </summary>
char const * UTF8::Previous(char const * begin, char const * text)
{
	if (text <= begin) {
		return(begin);
	}

	char const * cursor = text - 1;
	for (int steps = 1; steps < MAX_SEQUENCE && cursor > begin && Is_Continuation(*cursor); steps++) {
		cursor--;
	}
	return(cursor);
}


char * UTF8::Previous(char * begin, char * text)
{
	return(const_cast<char *>(Previous((char const *)begin, (char const *)text)));
}


bool UTF8::Is_Valid(std::string_view text)
{
	std::size_t offset = 0;
	while (offset < text.size()) {
		int length;
		bool valid;
		Decode_Sequence(text.data() + offset, text.size() - offset, length, valid);
		if (!valid) {
			return(false);
		}
		offset += length;
	}
	return(true);
}


/// <summary>
/// Tells whether code draws as a character: not a control, the delete, or the C1 range.
/// </summary>
bool UTF8::Is_Printable(char32_t code)
{
	return(code >= ' ' && (code < 0x7F || code > 0xA0) && code != REPLACEMENT);
}


/// <summary>
/// Returns the length of the byte order mark that opens text, or 0.
/// </summary>
std::size_t UTF8::BOM_Length(std::string_view text)
{
	return(text.starts_with(BOM) ? BOM.size() : 0);
}


/// <summary>
/// Copies source into dest, at most size - 1 bytes and never ending inside a sequence, and
/// returns the bytes copied. dest is always terminated when size is not zero.
/// </summary>
std::size_t UTF8::Copy(char * dest, std::size_t size, char const * source)
{
	if (size == 0) {
		return(0);
	}

	std::size_t count = Boundary_Before(source, size - 1);
	std::memcpy(dest, source, count);
	dest[count] = '\0';
	return(count);
}


/// <summary>
/// Returns the largest byte count no greater than limit at which text can be cut without
/// splitting a sequence.
/// </summary>
std::size_t UTF8::Boundary_Before(char const * text, std::size_t limit)
{
	std::size_t length = std::strlen(text);
	if (length <= limit) {
		return(length);
	}

	std::size_t cut = limit;
	while (cut > 0 && Is_Continuation((unsigned char)text[cut])) {
		cut--;
	}
	return(cut);
}


std::string UTF8::From_Windows_1252(std::string_view text)
{
	std::string result;
	result.reserve(text.size() + text.size() / 4);

	for (unsigned char byte : text) {
		if (byte < 0x80) {
			result.push_back((char)byte);
			continue;
		}

		char32_t code = (byte < 0xA0) ? Windows_1252_High[byte - 0x80] : byte;

		char encoded[MAX_SEQUENCE];
		result.append(encoded, Encode(code, encoded));
	}
	return(result);
}


/// <summary>
/// Transcodes to Windows-1252, writing '?' for every code point the code page lacks.
/// </summary>
std::string UTF8::To_Windows_1252(std::string_view text)
{
	std::string result;
	result.reserve(text.size());

	std::size_t offset = 0;
	while (offset < text.size()) {
		int length;
		bool valid;
		char32_t code = Decode_Sequence(text.data() + offset, text.size() - offset, length, valid);
		offset += length;

		int index = Windows_1252_Index(code);
		result.push_back(index < 0 ? '?' : (char)index);
	}
	return(result);
}


/// <summary>
/// Returns the Windows-1252 byte for code, or -1.
/// </summary>
int UTF8::Windows_1252_Index(char32_t code)
{
	if (code < 0x80 || (code >= 0xA0 && code <= 0xFF)) {
		return((int)code);
	}

	for (int index = 0; index < 32; index++) {
		if (Windows_1252_High[index] == code) {
			return(0x80 + index);
		}
	}
	return(-1);
}


/// <summary>
/// Returns the code page 437 byte that shows code, or -1. Control positions never map, and
/// a close visual match is accepted for what the code page lacks.
/// </summary>
int UTF8::OEM_437_Glyph(char32_t code)
{
	static short cache[0x10000];
	return(Best_Fit_Index(437, cache, code));
}


/// <summary>
/// Returns the Windows-1252 byte that shows code, or -1. The C1 range never maps, and a close
/// visual match is accepted for what the code page lacks.
/// </summary>
int UTF8::Windows_1252_Glyph(char32_t code)
{
	if (code >= 0x80 && code < 0xA0) {
		return(-1);
	}
	static short cache[0x10000];
	return(Best_Fit_Index(1252, cache, code));
}
