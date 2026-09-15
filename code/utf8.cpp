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


// The code points at bytes 0x80 to 0xFF of the other code pages the shipped bitmap fonts are
// cut in. A zero is a byte its page leaves undefined, which carries no character.
constexpr char32_t Windows_1250_High[128] = {
	0x20AC, 0x0000, 0x201A, 0x0000, 0x201E, 0x2026, 0x2020, 0x2021,
	0x0000, 0x2030, 0x0160, 0x2039, 0x015A, 0x0164, 0x017D, 0x0179,
	0x0000, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014,
	0x0000, 0x2122, 0x0161, 0x203A, 0x015B, 0x0165, 0x017E, 0x017A,
	0x00A0, 0x02C7, 0x02D8, 0x0141, 0x00A4, 0x0104, 0x00A6, 0x00A7,
	0x00A8, 0x00A9, 0x015E, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x017B,
	0x00B0, 0x00B1, 0x02DB, 0x0142, 0x00B4, 0x00B5, 0x00B6, 0x00B7,
	0x00B8, 0x0105, 0x015F, 0x00BB, 0x013D, 0x02DD, 0x013E, 0x017C,
	0x0154, 0x00C1, 0x00C2, 0x0102, 0x00C4, 0x0139, 0x0106, 0x00C7,
	0x010C, 0x00C9, 0x0118, 0x00CB, 0x011A, 0x00CD, 0x00CE, 0x010E,
	0x0110, 0x0143, 0x0147, 0x00D3, 0x00D4, 0x0150, 0x00D6, 0x00D7,
	0x0158, 0x016E, 0x00DA, 0x0170, 0x00DC, 0x00DD, 0x0162, 0x00DF,
	0x0155, 0x00E1, 0x00E2, 0x0103, 0x00E4, 0x013A, 0x0107, 0x00E7,
	0x010D, 0x00E9, 0x0119, 0x00EB, 0x011B, 0x00ED, 0x00EE, 0x010F,
	0x0111, 0x0144, 0x0148, 0x00F3, 0x00F4, 0x0151, 0x00F6, 0x00F7,
	0x0159, 0x016F, 0x00FA, 0x0171, 0x00FC, 0x00FD, 0x0163, 0x02D9,
};

constexpr char32_t Windows_1251_High[128] = {
	0x0402, 0x0403, 0x201A, 0x0453, 0x201E, 0x2026, 0x2020, 0x2021,
	0x20AC, 0x2030, 0x0409, 0x2039, 0x040A, 0x040C, 0x040B, 0x040F,
	0x0452, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014,
	0x0000, 0x2122, 0x0459, 0x203A, 0x045A, 0x045C, 0x045B, 0x045F,
	0x00A0, 0x040E, 0x045E, 0x0408, 0x00A4, 0x0490, 0x00A6, 0x00A7,
	0x0401, 0x00A9, 0x0404, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x0407,
	0x00B0, 0x00B1, 0x0406, 0x0456, 0x0491, 0x00B5, 0x00B6, 0x00B7,
	0x0451, 0x2116, 0x0454, 0x00BB, 0x0458, 0x0405, 0x0455, 0x0457,
	0x0410, 0x0411, 0x0412, 0x0413, 0x0414, 0x0415, 0x0416, 0x0417,
	0x0418, 0x0419, 0x041A, 0x041B, 0x041C, 0x041D, 0x041E, 0x041F,
	0x0420, 0x0421, 0x0422, 0x0423, 0x0424, 0x0425, 0x0426, 0x0427,
	0x0428, 0x0429, 0x042A, 0x042B, 0x042C, 0x042D, 0x042E, 0x042F,
	0x0430, 0x0431, 0x0432, 0x0433, 0x0434, 0x0435, 0x0436, 0x0437,
	0x0438, 0x0439, 0x043A, 0x043B, 0x043C, 0x043D, 0x043E, 0x043F,
	0x0440, 0x0441, 0x0442, 0x0443, 0x0444, 0x0445, 0x0446, 0x0447,
	0x0448, 0x0449, 0x044A, 0x044B, 0x044C, 0x044D, 0x044E, 0x044F,
};

constexpr char32_t Windows_1253_High[128] = {
	0x20AC, 0x0000, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021,
	0x0000, 0x2030, 0x0000, 0x2039, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014,
	0x0000, 0x2122, 0x0000, 0x203A, 0x0000, 0x0000, 0x0000, 0x0000,
	0x00A0, 0x0385, 0x0386, 0x00A3, 0x00A4, 0x00A5, 0x00A6, 0x00A7,
	0x00A8, 0x00A9, 0x0000, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x2015,
	0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x0384, 0x00B5, 0x00B6, 0x00B7,
	0x0388, 0x0389, 0x038A, 0x00BB, 0x038C, 0x00BD, 0x038E, 0x038F,
	0x0390, 0x0391, 0x0392, 0x0393, 0x0394, 0x0395, 0x0396, 0x0397,
	0x0398, 0x0399, 0x039A, 0x039B, 0x039C, 0x039D, 0x039E, 0x039F,
	0x03A0, 0x03A1, 0x0000, 0x03A3, 0x03A4, 0x03A5, 0x03A6, 0x03A7,
	0x03A8, 0x03A9, 0x03AA, 0x03AB, 0x03AC, 0x03AD, 0x03AE, 0x03AF,
	0x03B0, 0x03B1, 0x03B2, 0x03B3, 0x03B4, 0x03B5, 0x03B6, 0x03B7,
	0x03B8, 0x03B9, 0x03BA, 0x03BB, 0x03BC, 0x03BD, 0x03BE, 0x03BF,
	0x03C0, 0x03C1, 0x03C2, 0x03C3, 0x03C4, 0x03C5, 0x03C6, 0x03C7,
	0x03C8, 0x03C9, 0x03CA, 0x03CB, 0x03CC, 0x03CD, 0x03CE, 0x0000,
};

constexpr char32_t Windows_1254_High[128] = {
	0x20AC, 0x0000, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021,
	0x02C6, 0x2030, 0x0160, 0x2039, 0x0152, 0x0000, 0x0000, 0x0000,
	0x0000, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014,
	0x02DC, 0x2122, 0x0161, 0x203A, 0x0153, 0x0000, 0x0000, 0x0178,
	0x00A0, 0x00A1, 0x00A2, 0x00A3, 0x00A4, 0x00A5, 0x00A6, 0x00A7,
	0x00A8, 0x00A9, 0x00AA, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00AF,
	0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x00B4, 0x00B5, 0x00B6, 0x00B7,
	0x00B8, 0x00B9, 0x00BA, 0x00BB, 0x00BC, 0x00BD, 0x00BE, 0x00BF,
	0x00C0, 0x00C1, 0x00C2, 0x00C3, 0x00C4, 0x00C5, 0x00C6, 0x00C7,
	0x00C8, 0x00C9, 0x00CA, 0x00CB, 0x00CC, 0x00CD, 0x00CE, 0x00CF,
	0x011E, 0x00D1, 0x00D2, 0x00D3, 0x00D4, 0x00D5, 0x00D6, 0x00D7,
	0x00D8, 0x00D9, 0x00DA, 0x00DB, 0x00DC, 0x0130, 0x015E, 0x00DF,
	0x00E0, 0x00E1, 0x00E2, 0x00E3, 0x00E4, 0x00E5, 0x00E6, 0x00E7,
	0x00E8, 0x00E9, 0x00EA, 0x00EB, 0x00EC, 0x00ED, 0x00EE, 0x00EF,
	0x011F, 0x00F1, 0x00F2, 0x00F3, 0x00F4, 0x00F5, 0x00F6, 0x00F7,
	0x00F8, 0x00F9, 0x00FA, 0x00FB, 0x00FC, 0x0131, 0x015F, 0x00FF,
};

constexpr char32_t Windows_1257_High[128] = {
	0x20AC, 0x0000, 0x201A, 0x0000, 0x201E, 0x2026, 0x2020, 0x2021,
	0x0000, 0x2030, 0x0000, 0x2039, 0x0000, 0x00A8, 0x02C7, 0x00B8,
	0x0000, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014,
	0x0000, 0x2122, 0x0000, 0x203A, 0x0000, 0x00AF, 0x02DB, 0x0000,
	0x00A0, 0x0000, 0x00A2, 0x00A3, 0x00A4, 0x0000, 0x00A6, 0x00A7,
	0x00D8, 0x00A9, 0x0156, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00C6,
	0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x00B4, 0x00B5, 0x00B6, 0x00B7,
	0x00F8, 0x00B9, 0x0157, 0x00BB, 0x00BC, 0x00BD, 0x00BE, 0x00E6,
	0x0104, 0x012E, 0x0100, 0x0106, 0x00C4, 0x00C5, 0x0118, 0x0112,
	0x010C, 0x00C9, 0x0179, 0x0116, 0x0122, 0x0136, 0x012A, 0x013B,
	0x0160, 0x0143, 0x0145, 0x00D3, 0x014C, 0x00D5, 0x00D6, 0x00D7,
	0x0172, 0x0141, 0x015A, 0x016A, 0x00DC, 0x017B, 0x017D, 0x00DF,
	0x0105, 0x012F, 0x0101, 0x0107, 0x00E4, 0x00E5, 0x0119, 0x0113,
	0x010D, 0x00E9, 0x017A, 0x0117, 0x0123, 0x0137, 0x012B, 0x013C,
	0x0161, 0x0144, 0x0146, 0x00F3, 0x014D, 0x00F5, 0x00F6, 0x00F7,
	0x0173, 0x0142, 0x015B, 0x016B, 0x00FC, 0x017C, 0x017E, 0x02D9,
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


/// <summary>
/// Returns the code point a byte of the named code page shows, or zero where the page leaves
/// the byte undefined and where the page itself is not one of the shipped fonts'. The tables
/// are the engine's own rather than the system's, so a machine without a page installed still
/// reads the fonts cut in it.
/// </summary>
char32_t UTF8::Windows_Code(unsigned int page, unsigned char byte)
{
	if (byte < 0x80) {
		return(byte);
	}

	switch (page) {
		case 1250:
			return(Windows_1250_High[byte - 0x80]);

		case 1251:
			return(Windows_1251_High[byte - 0x80]);

		case 1252: {
			if (byte >= 0xA0) {
				return(byte);
			}
			// That table keeps an undefined byte at its own value so text survives a round
			// trip; a font has nothing to draw for one.
			char32_t code = Windows_1252_High[byte - 0x80];
			return((code == byte) ? 0 : code);
		}

		case 1253:
			return(Windows_1253_High[byte - 0x80]);

		case 1254:
			return(Windows_1254_High[byte - 0x80]);

		case 1257:
			return(Windows_1257_High[byte - 0x80]);

		default:
			break;
	}
	return(0);
}
