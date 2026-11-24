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

#include <cstdint>
#include <cstring>
#include <string>

#include "utf8.h"

#include "doctest/doctest.h"

namespace {

// Code points are compared as integers so that a failure prints their values.
void Check_Decode(char const * text, char32_t code, int length)
{
	INFO("decoding \"", text, "\"");
	char const * cursor = text;
	char32_t got = UTF8::Decode(cursor);
	CHECK((uint32_t)got == (uint32_t)code);
	CHECK(cursor - text == length);
}


void Check_Encode(char32_t code, char const * expected)
{
	INFO("encoding ", (uint32_t)code);
	char buffer[UTF8::MAX_SEQUENCE + 1] = {};
	int count = UTF8::Encode(code, buffer);
	CHECK(count == (int)std::strlen(expected));
	CHECK(std::string(buffer, count) == expected);
}


uint32_t Code(int page, unsigned char byte)
{
	return((uint32_t)UTF8::Windows_Code(page, byte));
}

}


TEST_CASE("a well formed sequence decodes to its code point and spans its bytes")
{
	SUBCASE("an ASCII byte decodes to itself") { Check_Decode("A", 'A', 1); }
	SUBCASE("two bytes, e acute") { Check_Decode("\xC3\xA9", 0xE9, 2); }
	SUBCASE("three bytes, the euro sign") { Check_Decode("\xE2\x82\xAC", 0x20AC, 3); }
	SUBCASE("four bytes, the astral plane") { Check_Decode("\xF0\x9F\x98\x80", 0x1F600, 4); }
	SUBCASE("the terminator decodes as zero") { Check_Decode("", 0, 1); }
}


TEST_CASE("a code point encodes to its sequence")
{
	SUBCASE("ASCII to one byte") { Check_Encode('A', "A"); }
	SUBCASE("e acute to two bytes") { Check_Encode(0xE9, "\xC3\xA9"); }
	SUBCASE("the euro sign to three bytes") { Check_Encode(0x20AC, "\xE2\x82\xAC"); }
	SUBCASE("an astral code point to four bytes") { Check_Encode(0x1F600, "\xF0\x9F\x98\x80"); }
}


TEST_CASE("a value that is no code point encodes as the replacement character")
{
	SUBCASE("a surrogate") { Check_Encode(0xD800, "\xEF\xBF\xBD"); }
	SUBCASE("a value past the last code point") { Check_Encode(0x110000, "\xEF\xBF\xBD"); }
}


TEST_CASE("a malformed sequence is one byte of replacement")
{
	SUBCASE("an overlong sequence") { Check_Decode("\xC0\xAF", UTF8::REPLACEMENT, 1); }
	SUBCASE("a truncated sequence") { Check_Decode("\xE2\x82", UTF8::REPLACEMENT, 1); }
	SUBCASE("an encoded surrogate") { Check_Decode("\xED\xA0\x80", UTF8::REPLACEMENT, 1); }
	SUBCASE("a stray continuation byte") { Check_Decode("\xA9", UTF8::REPLACEMENT, 1); }
	SUBCASE("a Windows-1252 e acute") { Check_Decode("\xE9 ", UTF8::REPLACEMENT, 1); }
	SUBCASE("a lead byte above F4") { Check_Decode("\xF5\x80\x80\x80", UTF8::REPLACEMENT, 1); }
}


TEST_CASE("a lead byte announces its sequence length, and a byte that cannot lead announces zero")
{
	CHECK(UTF8::Sequence_Length('A') == 1);
	CHECK(UTF8::Sequence_Length(0xC3) == 2);
	CHECK(UTF8::Sequence_Length(0xE2) == 3);
	CHECK(UTF8::Sequence_Length(0xF0) == 4);
	CHECK(UTF8::Sequence_Length(0xA9) == 0);
	CHECK(UTF8::Sequence_Length(0xC1) == 0);
	CHECK(UTF8::Sequence_Length(0xFF) == 0);
}


TEST_CASE("continuation bytes are told from leads and ASCII")
{
	CHECK(UTF8::Is_Continuation(0xA9));
	CHECK_FALSE(UTF8::Is_Continuation('A'));
	CHECK_FALSE(UTF8::Is_Continuation(0xC3));
}


TEST_CASE("peeking reports the code point and its length without advancing")
{
	int length = 0;
	char32_t code = UTF8::Peek("\xC3\xA9x", length);
	CHECK((uint32_t)code == 0xE9u);
	CHECK(length == 2);
}


TEST_CASE("letters, space, and accented or foreign characters are printable")
{
	CHECK(UTF8::Is_Printable('a'));
	CHECK(UTF8::Is_Printable(' '));
	CHECK(UTF8::Is_Printable(0xE9));
	CHECK(UTF8::Is_Printable(0x4E2D));
}


TEST_CASE("controls, the delete, the C1 range, and the replacement character are not printable")
{
	CHECK_FALSE(UTF8::Is_Printable('\r'));
	CHECK_FALSE(UTF8::Is_Printable(0x7F));
	CHECK_FALSE(UTF8::Is_Printable(0x85));
	CHECK_FALSE(UTF8::Is_Printable(0xA0));
	CHECK_FALSE(UTF8::Is_Printable(UTF8::REPLACEMENT));
}


TEST_CASE("well formed text is valid")
{
	CHECK_MESSAGE(UTF8::Is_Valid("plain text"), "ASCII");
	CHECK_MESSAGE(UTF8::Is_Valid("caf\xC3\xA9 \xE2\x82\xAC \xF0\x9F\x98\x80"), "multi byte text");
	CHECK_MESSAGE(UTF8::Is_Valid("\xEF\xBF\xBD"), "a literal replacement character");
}


TEST_CASE("malformed text is not valid")
{
	CHECK_MESSAGE(!UTF8::Is_Valid("caf\xE9"), "a Windows-1252 byte");
	CHECK_MESSAGE(!UTF8::Is_Valid(std::string_view("\xC3", 1)), "a sequence cut by the end of the text");
}


TEST_CASE("validity respects the length given and not the bytes after it")
{
	CHECK(UTF8::Is_Valid(std::string_view("\xC3\xA9\xC3", 2)));
}


TEST_CASE("overlong forms, surrogates, values past U+10FFFF, and stray continuation bytes are not valid")
{
	CHECK_MESSAGE(!UTF8::Is_Valid("\xC0\xAF"), "an overlong two byte form");
	CHECK_MESSAGE(!UTF8::Is_Valid("\xE0\x80\xAF"), "an overlong three byte form");
	CHECK_MESSAGE(!UTF8::Is_Valid("\xF0\x80\x80\xAF"), "an overlong four byte form");
	CHECK_MESSAGE(!UTF8::Is_Valid("\xED\xA0\x80"), "the first surrogate");
	CHECK_MESSAGE(!UTF8::Is_Valid("\xED\xBF\xBF"), "the last surrogate");
	CHECK_MESSAGE(!UTF8::Is_Valid("\xF4\x90\x80\x80"), "U+110000");
	CHECK_MESSAGE(!UTF8::Is_Valid("\xF5\x80\x80\x80"), "a lead byte above F4");
	CHECK_MESSAGE(!UTF8::Is_Valid("a\x80" "b"), "a stray continuation byte");
	CHECK_MESSAGE(!UTF8::Is_Valid("\xE2\x82"), "a three byte sequence cut short");
	CHECK_MESSAGE(!UTF8::Is_Valid("\xF0\x9F\x98"), "a four byte sequence cut short");
	CHECK_MESSAGE(!UTF8::Is_Valid("\xE2\x82" "A"), "a sequence broken by an ASCII byte");

	CHECK_MESSAGE(UTF8::Is_Valid("\xED\x9F\xBF"), "U+D7FF, just below the surrogates");
	CHECK_MESSAGE(UTF8::Is_Valid("\xEE\x80\x80"), "U+E000, just above them");
	CHECK_MESSAGE(UTF8::Is_Valid("\xF4\x8F\xBF\xBF"), "U+10FFFF");
}


TEST_CASE("a sequence reports its length, and a malformed one the maximal subpart one replacement stands for")
{
	auto check = [](std::string_view text, int length, int subpart) {
		INFO("text of ", text.size(), " bytes starting ", (unsigned)(unsigned char)(text.empty() ? 0 : text[0]));
		int got = -1;
		CHECK(UTF8::Sequence_At(text, got) == length);
		CHECK(got == subpart);
	};

	check("A", 1, 1);
	check("\xE2\x82\xAC" "A", 3, 3);
	check("\xF0\x9F\x98\x80", 4, 4);
	check("\xE2\x82" "A", 0, 2);
	check("\xE2\x82", 0, 2);
	check("\xF0\x9F\x98" "A", 0, 3);
	check("\xC0\xAF", 0, 1);
	check("\xED\xA0\x80", 0, 1);
	check("\xF4\x90\x80\x80", 0, 1);
	check("\x80\x80", 0, 1);
	check("", 0, 0);
}


TEST_CASE("stepping back lands on the lead of the previous character")
{
	char const * text = "a\xC3\xA9" "b";
	CHECK_MESSAGE(UTF8::Previous(text, text + 3) == text + 1, "from after a two byte sequence");
	CHECK_MESSAGE(UTF8::Previous(text, text + 1) == text, "from after an ASCII byte");
	CHECK_MESSAGE(UTF8::Previous(text, text) == text, "from the start, which stays at the start");
	CHECK_MESSAGE(UTF8::Previous(text, text + 4) == text + 3, "from the terminator, onto the last byte");
}


TEST_CASE("stepping back over stray continuation bytes gives up after a sequence's worth")
{
	char const * cont = "\xA9\xA9\xA9\xA9\xA9";
	CHECK(UTF8::Previous(cont, cont + 5) == cont + 1);
}


TEST_CASE("Windows-1252 text transcodes to UTF-8")
{
	CHECK_MESSAGE(UTF8::From_Windows_1252("caf\xE9") == "caf\xC3\xA9", "e acute becomes two bytes");
	CHECK_MESSAGE(UTF8::From_Windows_1252("\x80\x99") == "\xE2\x82\xAC\xE2\x84\xA2", "the euro and trade mark bytes map through the high table");
	CHECK_MESSAGE(UTF8::From_Windows_1252("\x81") == "\xC2\x81", "a byte the code page leaves undefined keeps its own value");
	CHECK_MESSAGE(UTF8::From_Windows_1252("ascii") == "ascii", "ASCII passes through unchanged");
}


TEST_CASE("UTF-8 text transcodes back to Windows-1252")
{
	CHECK_MESSAGE(UTF8::To_Windows_1252("caf\xC3\xA9 \xE2\x82\xAC") == "caf\xE9 \x80", "e acute and the euro sign go back to their bytes");
	CHECK_MESSAGE(UTF8::To_Windows_1252("\xE2\x84\x83") == "?", "a code point outside Windows-1252 becomes a question mark");
}


TEST_CASE("Windows-1252 text survives a round trip")
{
	CHECK(UTF8::To_Windows_1252(UTF8::From_Windows_1252("na\xEFve \x93quoted\x94")) == "na\xEFve \x93quoted\x94");
}


// The digest of a legacy file is checked by converting the database back, so the conversion
// has to be an exact inverse for every byte the file could hold.
TEST_CASE("every Windows-1252 byte survives a round trip")
{
	std::string every;
	for (int byte = 0; byte < 256; byte++) {
		every.push_back((char)byte);
	}
	CHECK(UTF8::To_Windows_1252(UTF8::From_Windows_1252(every)) == every);
}


TEST_CASE("a code point has its Windows-1252 index, or none")
{
	CHECK_MESSAGE(UTF8::Windows_1252_Index('A') == 'A', "ASCII keeps its byte");
	CHECK_MESSAGE(UTF8::Windows_1252_Index(0xE9) == 0xE9, "Latin-1 keeps its byte");
	CHECK_MESSAGE(UTF8::Windows_1252_Index(0x20AC) == 0x80, "the euro sign");
	CHECK_MESSAGE(UTF8::Windows_1252_Index(0x0153) == 0x9C, "the oe ligature");
	CHECK_MESSAGE(UTF8::Windows_1252_Index(0x0100) == -1, "a code point Windows-1252 lacks");
}


TEST_CASE("a code point has its code page 437 glyph")
{
	CHECK_MESSAGE(UTF8::OEM_437_Glyph('A') == 'A', "ASCII keeps its byte");
	CHECK_MESSAGE(UTF8::OEM_437_Glyph(0xC7) == 0x80, "C cedilla");
	CHECK_MESSAGE(UTF8::OEM_437_Glyph(0xE9) == 0x82, "e acute");
	CHECK_MESSAGE(UTF8::OEM_437_Glyph(0xE4) == 0x84, "a umlaut");
	CHECK_MESSAGE(UTF8::OEM_437_Glyph(0xC9) == 0x90, "E acute");
	CHECK_MESSAGE(UTF8::OEM_437_Glyph(0xBF) == 0xA8, "inverted question mark");
	CHECK_MESSAGE(UTF8::OEM_437_Glyph(0xA1) == 0xAD, "inverted exclamation mark");
}


TEST_CASE("a code point code page 437 cannot show has no glyph")
{
	CHECK_MESSAGE(UTF8::OEM_437_Glyph(0x4E2D) == -1, "a CJK ideograph");
	CHECK_MESSAGE(UTF8::OEM_437_Glyph(0x1F600) == -1, "an astral code point");
	CHECK_MESSAGE(UTF8::OEM_437_Glyph(0x2302) == -1, "a code point mapping to the delete position");
	CHECK_MESSAGE(UTF8::OEM_437_Glyph(0x263A) == -1, "a code point mapping to a control position");
	CHECK_MESSAGE(UTF8::OEM_437_Glyph(0x20AC) == -1, "the euro sign");
}


TEST_CASE("code page 437 falls back to the closest ASCII form")
{
	CHECK_MESSAGE(UTF8::OEM_437_Glyph(0x2019) == '\'', "typographic punctuation");
	CHECK_MESSAGE(UTF8::OEM_437_Glyph(0x2014) == '-', "typographic punctuation");
	CHECK_MESSAGE(UTF8::OEM_437_Glyph(0x2026) == '.', "typographic punctuation");
	CHECK_MESSAGE(UTF8::OEM_437_Glyph(0x0141) == 'L', "a letter code page 437 lacks");
	CHECK_MESSAGE(UTF8::OEM_437_Glyph(0xA9) == 'c', "a letter code page 437 lacks");
}


TEST_CASE("a Windows-1252 font has its glyphs at their bytes and falls back to the closest letter")
{
	CHECK_MESSAGE(UTF8::Windows_1252_Glyph(0xE9) == 0xE9, "e acute");
	CHECK_MESSAGE(UTF8::Windows_1252_Glyph(0xA9) == 0xA9, "the copyright sign");
	CHECK_MESSAGE(UTF8::Windows_1252_Glyph(0x20AC) == 0x80, "the euro sign in the high row");
	CHECK_MESSAGE(UTF8::Windows_1252_Glyph(0x0153) == 0x9C, "the oe ligature in the high row");
	CHECK_MESSAGE(UTF8::Windows_1252_Glyph(0x0141) == 'L', "the closest letter");
}


TEST_CASE("a Windows-1252 font has no glyph for a C1 control or a CJK ideograph")
{
	CHECK(UTF8::Windows_1252_Glyph(0x0081) == -1);
	CHECK(UTF8::Windows_1252_Glyph(0x4E2D) == -1);
}


TEST_CASE("every code page shows the same character below the high row")
{
	for (int page : { 1250, 1251, 1252, 1253, 1254, 1257 }) {
		INFO("code page ", page);
		CHECK(Code(page, 'A') == (uint32_t)U'A');
	}
}


TEST_CASE("Windows-1252 shows e acute and the euro sign")
{
	CHECK(Code(1252, 0xE9) == (uint32_t)U'é');
	CHECK(Code(1252, 0x80) == (uint32_t)U'€');
}


TEST_CASE("one byte shows a different character on each page")
{
	CHECK(Code(1250, 0xE8) == (uint32_t)U'č');
	CHECK(Code(1251, 0xE8) == (uint32_t)U'и');
	CHECK(Code(1253, 0xE8) == (uint32_t)U'θ');
	CHECK(Code(1254, 0xE8) == (uint32_t)U'è');
	CHECK(Code(1257, 0xE8) == (uint32_t)U'č');
}


TEST_CASE("the pages carry Cyrillic, Greek and Baltic where Windows-1252 carries accented Latin")
{
	CHECK(Code(1251, 0xC0) == (uint32_t)U'А');
	CHECK(Code(1253, 0xC1) == (uint32_t)U'Α');
	CHECK(Code(1257, 0xC0) == (uint32_t)U'Ą');
}


TEST_CASE("a byte its page leaves undefined shows nothing")
{
	CHECK(Code(1252, 0x81) == 0u);
	CHECK(Code(1250, 0x81) == 0u);
	CHECK(Code(1251, 0x98) == 0u);
	CHECK(Code(1253, 0xFF) == 0u);
	CHECK(Code(1257, 0xA1) == 0u);
}


TEST_CASE("a page the engine carries no table for shows nothing above the low row")
{
	CHECK(Code(1255, 0xE0) == 0u);
	CHECK(Code(874, 0xA1) == 0u);
	CHECK(Code(0, 0xE9) == 0u);
}


TEST_CASE("a bounded copy stops before a sequence it cannot fit")
{
	char dest[4];
	std::memset(dest, 'x', sizeof(dest));

	CHECK(UTF8::Copy(dest, sizeof(dest), "a\xC3\xA9\xC3\xA9") == 3);
	CHECK(std::string(dest) == "a\xC3\xA9");

	CHECK(UTF8::Copy(dest, 3, "a\xC3\xA9") == 1);
	CHECK(std::string(dest) == "a");
}


TEST_CASE("a bounded copy takes a short string whole")
{
	char dest[4];
	std::memset(dest, 'x', sizeof(dest));
	CHECK(UTF8::Copy(dest, sizeof(dest), "abc") == 3);
	CHECK(std::string(dest) == "abc");
}


TEST_CASE("a one byte destination gets only the terminator, and a zero size one is left alone")
{
	char dest[4];
	std::memset(dest, 'x', sizeof(dest));
	CHECK(UTF8::Copy(dest, 1, "abc") == 0);
	CHECK(dest[0] == '\0');
	CHECK(UTF8::Copy(dest, 0, "abc") == 0);
}


TEST_CASE("a cut lands on a character boundary")
{
	CHECK_MESSAGE(UTF8::Boundary_Before("a\xC3\xA9", 2) == 1, "a cut inside a sequence moves back to before it");
	CHECK_MESSAGE(UTF8::Boundary_Before("a\xC3\xA9", 3) == 3, "a cut on a boundary stays");
	CHECK_MESSAGE(UTF8::Boundary_Before("a\xC3\xA9", 10) == 3, "a limit past the end returns the length");
	CHECK_MESSAGE(UTF8::Boundary_Before("\xF0\x9F\x98\x80", 3) == 0, "a cut inside a four byte sequence at the start returns zero");
}
