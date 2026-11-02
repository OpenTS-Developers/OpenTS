/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "darwin.h"
#include "sdl/sdlkeys.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_scancode.h>

#include <cstdio>
#include <cstring>
#include <cwchar>


int Language_Load_String(unsigned, char *, int)
{
	return(0);
}


static bool Fail(char const * message)
{
	std::fputs(message, stderr);
	std::fputc('\n', stderr);
	return(false);
}


static bool Round_Trip(char const * argument)
{
	int count = 0;
	wchar_t ** wide = CommandLineToArgvW(GetCommandLineW(), &count);
	if (wide == nullptr) return(Fail("CommandLineToArgvW returned nothing"));

	bool matched = false;
	bool acute = false;
	for (int index = 0; index < count; index++) {
		bool const has_acute = std::wcschr(wide[index], (wchar_t)0x00E9) != nullptr;
		bool const split = std::wcschr(wide[index], (wchar_t)0x00C3) != nullptr || std::wcschr(wide[index], (wchar_t)0x00A9) != nullptr;
		char bytes[512];
		int const length = WideCharToMultiByte(CP_UTF8, 0, wide[index], -1, bytes, (int)sizeof(bytes), nullptr, nullptr);
		if (length <= 0) continue;
		if (std::strcmp(bytes, argument) != 0) continue;
		matched = true;
		acute = has_acute && !split;
	}
	LocalFree(wide);

	if (!matched) return(Fail("the -DATADIR argument did not round-trip"));
	if (!acute) return(Fail("é was widened as bytes"));
	return(true);
}


static bool Expect_Scan(SDL_Scancode scancode, unsigned windows)
{
	if (Windows_Scan_Code((int)scancode) == windows) return(true);
	std::fprintf(stderr, "scan code of %d is not 0x%02X\n", (int)scancode, windows);
	return(false);
}


static bool Keys(void)
{
	if (!SDL_Init(SDL_INIT_VIDEO)) return(Fail(SDL_GetError()));

	bool ok = Expect_Scan(SDL_SCANCODE_A, 0x1E)
		&& Expect_Scan(SDL_SCANCODE_MINUS, 0x0C)
		&& Expect_Scan(SDL_SCANCODE_LEFTBRACKET, 0x1A)
		&& Expect_Scan(SDL_SCANCODE_SEMICOLON, 0x27)
		&& Expect_Scan(SDL_SCANCODE_SLASH, 0x35)
		&& Expect_Scan(SDL_SCANCODE_Y, 0x15)
		&& Expect_Scan(SDL_SCANCODE_Z, 0x2C)
		&& Windows_Scan_Code((int)SDL_SCANCODE_LEFT) == 0;
	if (!ok && Windows_Scan_Code((int)SDL_SCANCODE_LEFT) != 0) {
		Fail("an arrow key was given a layout scan code");
	}

	SDL_Keycode const printed = SDL_GetKeyFromScancode(SDL_SCANCODE_A, SDL_KMOD_NONE, true);
	UINT const scan = Windows_Scan_Code((int)SDL_SCANCODE_A);
	UINT const virtualkey = MapVirtualKeyExW(scan, MAPVK_VSC_TO_VK_EX, nullptr);
	if (printed >= 'a' && printed <= 'z' && virtualkey != (UINT)(printed - 'a' + 'A')) {
		ok = Fail("the A key's virtual key is not the letter it prints");
	}
	UINT const character = MapVirtualKeyExW(virtualkey, MAPVK_VK_TO_CHAR, nullptr) & 0x7FFFFFFF;
	if (character == 0) ok = Fail("the letter has no character");
	if (Virtual_Key_Name((int)virtualkey).empty()) ok = Fail("the letter has no name");

	SDL_Keycode const comma = SDL_GetKeyFromScancode(SDL_SCANCODE_COMMA, SDL_KMOD_NONE, true);
	if (comma == ',') {
		std::string const name = Virtual_Key_Name(VK_OEM_COMMA);
		if (name != ",") ok = Fail("the comma key is not named \",\"");
	}

	SDL_Keycode const sharp = SDL_GetKeyFromScancode(SDL_SCANCODE_MINUS, SDL_KMOD_NONE, true);
	if (sharp == 0x00DF && MapVirtualKeyExW(0x0C, MAPVK_VSC_TO_VK_EX, nullptr) != VK_OEM_4) {
		ok = Fail("the sharp s key did not keep its code");
	}

	wchar_t letters[3] = { (wchar_t)0x00F6, (wchar_t)0x00DF, L'\0' };
	CharUpperW(letters);
	if (letters[0] != (wchar_t)0x00D6 || letters[1] != (wchar_t)0x00DF) {
		ok = Fail("U+00F6 did not become U+00D6, or U+00DF changed");
	}

	SDL_Quit();
	return(ok);
}


int main(int argc, char ** argv)
{
	char const * argument = nullptr;
	for (int index = 1; index < argc; index++) {
		if (std::strncmp(argv[index], "-DATADIR=", 9) == 0) argument = argv[index];
	}
	if (argument == nullptr) {
		Fail("expected a -DATADIR= argument");
		return(1);
	}
	if (!Round_Trip(argument) || !Keys()) return(1);
	return(0);
}
