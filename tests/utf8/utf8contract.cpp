/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// Pins the UTF-8 helper's contract with no engine and no game data: how sequences decode
// and encode, what a malformed byte turns into, where a cut may land, and which glyph the
// shipped fonts and legacy code pages give a code point.

#include <cstdio>
#include <cstring>
#include <string>

#include "utf8.h"

namespace {

int Failures = 0;


void Check(bool condition, char const * what)
{
	std::printf("%-76s %s\n", what, condition ? "ok" : "FAILED");

	if (!condition) {
		Failures++;
	}
}


bool Decodes_To(char const * text, char32_t code, int length)
{
	char const * cursor = text;
	char32_t got = UTF8::Decode(cursor);
	return(got == code && cursor - text == length);
}


bool Encodes_To(char32_t code, char const * expected)
{
	char buffer[UTF8::MAX_SEQUENCE + 1] = {};
	int count = UTF8::Encode(code, buffer);
	return(count == (int)std::strlen(expected) && std::memcmp(buffer, expected, count) == 0);
}

}


int main(void)
{
	Check(Decodes_To("A", 'A', 1), "an ASCII byte decodes to itself and spans one byte");
	Check(Decodes_To("\xC3\xA9", 0xE9, 2), "a two byte sequence decodes (e acute)");
	Check(Decodes_To("\xE2\x82\xAC", 0x20AC, 3), "a three byte sequence decodes (euro sign)");
	Check(Decodes_To("\xF0\x9F\x98\x80", 0x1F600, 4), "a four byte sequence decodes (astral plane)");
	Check(Decodes_To("", 0, 1), "the terminator decodes as zero");

	Check(Encodes_To('A', "A"), "an ASCII code point encodes to one byte");
	Check(Encodes_To(0xE9, "\xC3\xA9"), "e acute encodes to two bytes");
	Check(Encodes_To(0x20AC, "\xE2\x82\xAC"), "the euro sign encodes to three bytes");
	Check(Encodes_To(0x1F600, "\xF0\x9F\x98\x80"), "an astral code point encodes to four bytes");
	Check(Encodes_To(0xD800, "\xEF\xBF\xBD"), "a surrogate encodes as the replacement character");
	Check(Encodes_To(0x110000, "\xEF\xBF\xBD"), "a value past the last code point encodes as the replacement character");

	Check(Decodes_To("\xC0\xAF", UTF8::REPLACEMENT, 1), "an overlong sequence is one byte of replacement");
	Check(Decodes_To("\xE2\x82", UTF8::REPLACEMENT, 1), "a truncated sequence is one byte of replacement");
	Check(Decodes_To("\xED\xA0\x80", UTF8::REPLACEMENT, 1), "an encoded surrogate is one byte of replacement");
	Check(Decodes_To("\xA9", UTF8::REPLACEMENT, 1), "a stray continuation byte is one byte of replacement");
	Check(Decodes_To("\xE9 ", UTF8::REPLACEMENT, 1), "a Windows-1252 e acute is one byte of replacement");
	Check(Decodes_To("\xF5\x80\x80\x80", UTF8::REPLACEMENT, 1), "a lead byte above F4 is one byte of replacement");

	Check(UTF8::Sequence_Length('A') == 1 && UTF8::Sequence_Length(0xC3) == 2 && UTF8::Sequence_Length(0xE2) == 3 && UTF8::Sequence_Length(0xF0) == 4, "a lead byte announces its sequence length");
	Check(UTF8::Sequence_Length(0xA9) == 0 && UTF8::Sequence_Length(0xC1) == 0 && UTF8::Sequence_Length(0xFF) == 0, "a byte that cannot lead announces length zero");
	Check(UTF8::Is_Continuation(0xA9) && !UTF8::Is_Continuation('A') && !UTF8::Is_Continuation(0xC3), "continuation bytes are told from leads and ASCII");

	{
		int length = 0;
		char32_t code = UTF8::Peek("\xC3\xA9x", length);
		Check(code == 0xE9 && length == 2, "peeking reports the code point and its length without advancing");
	}

	Check(UTF8::Is_Printable('a') && UTF8::Is_Printable(' ') && UTF8::Is_Printable(0xE9) && UTF8::Is_Printable(0x4E2D), "letters, space, and accented or foreign characters are printable");
	Check(!UTF8::Is_Printable('\r') && !UTF8::Is_Printable(0x7F) && !UTF8::Is_Printable(0x85) && !UTF8::Is_Printable(0xA0) && !UTF8::Is_Printable(UTF8::REPLACEMENT), "controls, the delete, the C1 range, and the replacement character are not printable");

	Check(UTF8::Is_Valid("plain text"), "ASCII is valid");
	Check(UTF8::Is_Valid("caf\xC3\xA9 \xE2\x82\xAC \xF0\x9F\x98\x80"), "well formed multi byte text is valid");
	Check(UTF8::Is_Valid("\xEF\xBF\xBD"), "a literal replacement character is valid");
	Check(!UTF8::Is_Valid("caf\xE9"), "a Windows-1252 byte is not valid");
	Check(!UTF8::Is_Valid(std::string_view("\xC3", 1)), "a sequence cut by the end of the text is not valid");
	Check(UTF8::Is_Valid(std::string_view("\xC3\xA9\xC3", 2)), "validity respects the length given and not the bytes after it");

	{
		char const * text = "a\xC3\xA9" "b";
		Check(UTF8::Previous(text, text + 3) == text + 1, "stepping back from after a two byte sequence lands on its lead");
		Check(UTF8::Previous(text, text + 1) == text, "stepping back from after an ASCII byte lands on it");
		Check(UTF8::Previous(text, text) == text, "stepping back from the start stays at the start");
		Check(UTF8::Previous(text, text + 4) == text + 3, "stepping back from the terminator lands on the last byte");
		char const * cont = "\xA9\xA9\xA9\xA9\xA9";
		Check(UTF8::Previous(cont, cont + 5) == cont + 1, "stepping back over stray continuation bytes gives up after a sequence's worth");
	}

	Check(UTF8::From_Windows_1252("caf\xE9") == "caf\xC3\xA9", "a Windows-1252 e acute becomes two bytes");
	Check(UTF8::From_Windows_1252("\x80\x99") == "\xE2\x82\xAC\xE2\x84\xA2", "the Windows-1252 euro and trade mark bytes map through the high table");
	Check(UTF8::From_Windows_1252("\x81") == "\xC2\x81", "a byte the code page leaves undefined keeps its own value");
	Check(UTF8::From_Windows_1252("ascii") == "ascii", "ASCII passes through the Windows-1252 transcode unchanged");

	Check(UTF8::To_Windows_1252("caf\xC3\xA9 \xE2\x82\xAC") == "caf\xE9 \x80", "e acute and the euro sign go back to their Windows-1252 bytes");
	Check(UTF8::To_Windows_1252("\xE2\x84\x83") == "?", "a code point outside Windows-1252 becomes a question mark");
	Check(UTF8::To_Windows_1252(UTF8::From_Windows_1252("na\xEFve \x93quoted\x94")) == "na\xEFve \x93quoted\x94", "Windows-1252 text survives a round trip");

	{
		// The digest of a legacy file is checked by converting the database back, so the
		// conversion has to be an exact inverse for every byte the file could hold.
		std::string every;
		for (int byte = 0; byte < 256; byte++) {
			every.push_back((char)byte);
		}
		Check(UTF8::To_Windows_1252(UTF8::From_Windows_1252(every)) == every,
			"every Windows-1252 byte survives a round trip");
	}

	Check(UTF8::Windows_1252_Index('A') == 'A', "ASCII keeps its byte in Windows-1252");
	Check(UTF8::Windows_1252_Index(0xE9) == 0xE9, "Latin-1 keeps its byte in Windows-1252");
	Check(UTF8::Windows_1252_Index(0x20AC) == 0x80, "the euro sign is 0x80 in Windows-1252");
	Check(UTF8::Windows_1252_Index(0x0153) == 0x9C, "oe ligature is 0x9C in Windows-1252");
	Check(UTF8::Windows_1252_Index(0x0100) == -1, "a code point Windows-1252 lacks has no index");

	Check(UTF8::OEM_437_Glyph('A') == 'A', "ASCII keeps its byte in code page 437");
	Check(UTF8::OEM_437_Glyph(0xC7) == 0x80, "C cedilla is 0x80 in code page 437");
	Check(UTF8::OEM_437_Glyph(0xE9) == 0x82, "e acute is 0x82 in code page 437");
	Check(UTF8::OEM_437_Glyph(0xE4) == 0x84, "a umlaut is 0x84 in code page 437");
	Check(UTF8::OEM_437_Glyph(0xC9) == 0x90, "E acute is 0x90 in code page 437");
	Check(UTF8::OEM_437_Glyph(0xBF) == 0xA8, "inverted question mark is 0xA8 in code page 437");
	Check(UTF8::OEM_437_Glyph(0xA1) == 0xAD, "inverted exclamation mark is 0xAD in code page 437");
	Check(UTF8::OEM_437_Glyph(0x4E2D) == -1, "a CJK ideograph has no code page 437 index");
	Check(UTF8::OEM_437_Glyph(0x1F600) == -1, "an astral code point has no code page 437 index");
	Check(UTF8::OEM_437_Glyph(0x2302) == -1, "a code point mapping to the delete position has no index");
	Check(UTF8::OEM_437_Glyph(0x263A) == -1, "a code point mapping to a control position has no index");
	Check(UTF8::OEM_437_Glyph(0x20AC) == -1, "the euro sign has no code page 437 index");
	Check(UTF8::OEM_437_Glyph(0x2019) == '\'' && UTF8::OEM_437_Glyph(0x2014) == '-' && UTF8::OEM_437_Glyph(0x2026) == '.', "typographic punctuation falls back to its ASCII form");
	Check(UTF8::OEM_437_Glyph(0x0141) == 'L' && UTF8::OEM_437_Glyph(0xA9) == 'c', "a letter code page 437 lacks falls back to its closest ASCII letter");

	Check(UTF8::Windows_1252_Glyph(0xE9) == 0xE9 && UTF8::Windows_1252_Glyph(0xA9) == 0xA9, "a Windows-1252 font has e acute and the copyright sign at their bytes");
	Check(UTF8::Windows_1252_Glyph(0x20AC) == 0x80 && UTF8::Windows_1252_Glyph(0x0153) == 0x9C, "a Windows-1252 font has the euro sign and oe ligature in the high row");
	Check(UTF8::Windows_1252_Glyph(0x0141) == 'L', "a Windows-1252 font falls back to the closest letter");
	Check(UTF8::Windows_1252_Glyph(0x0081) == -1 && UTF8::Windows_1252_Glyph(0x4E2D) == -1, "a Windows-1252 font has no index for a C1 control or a CJK ideograph");

	{
		char dest[4];
		std::memset(dest, 'x', sizeof(dest));
		Check(UTF8::Copy(dest, sizeof(dest), "a\xC3\xA9\xC3\xA9") == 3 && std::strcmp(dest, "a\xC3\xA9") == 0, "a bounded copy stops before a sequence it cannot fit");
		Check(UTF8::Copy(dest, 3, "a\xC3\xA9") == 1 && std::strcmp(dest, "a") == 0, "a bounded copy that cannot fit the next sequence stops before it");
		Check(UTF8::Copy(dest, sizeof(dest), "abc") == 3 && std::strcmp(dest, "abc") == 0, "a bounded copy takes a short string whole");
		Check(UTF8::Copy(dest, 1, "abc") == 0 && dest[0] == '\0', "a one byte destination gets only the terminator");
		Check(UTF8::Copy(dest, 0, "abc") == 0, "a zero size destination is left alone");
	}

	Check(UTF8::Boundary_Before("a\xC3\xA9", 2) == 1, "a cut inside a sequence moves back to before it");
	Check(UTF8::Boundary_Before("a\xC3\xA9", 3) == 3, "a cut on a boundary stays");
	Check(UTF8::Boundary_Before("a\xC3\xA9", 10) == 3, "a limit past the end returns the length");
	Check(UTF8::Boundary_Before("\xF0\x9F\x98\x80", 3) == 0, "a cut inside a four byte sequence at the start returns zero");

	std::printf("\n%s\n", Failures == 0 ? "PASSED" : "FAILED");
	return(Failures == 0 ? 0 : 1);
}
