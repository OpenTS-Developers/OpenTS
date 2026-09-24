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


constexpr std::size_t UI_CLIPBOARD_MAX_BYTES = 16 * 1024 * 1024;

bool UI_UTF8_To_UTF16(std::string_view text, std::wstring & wide);
bool UI_UTF16_To_UTF8(std::wstring_view wide, std::string & text);
