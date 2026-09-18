/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "ui/uiunicode.h"

#include <windows.h>

#include <climits>
#include <new>


bool UI_UTF8_To_UTF16(std::string_view text, std::wstring & wide)
{
	wide.clear();

	if (text.size() > UI_CLIPBOARD_MAX_BYTES || text.size() > INT_MAX) {
		return(false);
	}
	if (text.empty()) {
		return(true);
	}

	int length = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, text.data(), (int)text.size(), nullptr, 0);
	if (length == 0) {
		return(false);
	}

	try {
		wide.resize((std::size_t)length);
	} catch (std::bad_alloc const &) {
		return(false);
	}

	if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, text.data(), (int)text.size(), wide.data(), length) != length) {
		wide.clear();
		return(false);
	}
	return(true);
}


bool UI_UTF16_To_UTF8(std::wstring_view wide, std::string & text)
{
	text.clear();

	if (wide.size() > UI_CLIPBOARD_MAX_BYTES / sizeof(wchar_t) || wide.size() > INT_MAX) {
		return(false);
	}
	if (wide.empty()) {
		return(true);
	}

	int length = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, wide.data(), (int)wide.size(), nullptr, 0, nullptr, nullptr);
	if (length == 0) {
		return(false);
	}

	try {
		text.resize((std::size_t)length);
	} catch (std::bad_alloc const &) {
		return(false);
	}

	if (WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, wide.data(), (int)wide.size(), text.data(), length, nullptr, nullptr) != length) {
		text.clear();
		return(false);
	}
	return(true);
}
