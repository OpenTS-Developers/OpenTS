/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "always.h"

#include <cctype>


/// <summary>
/// Converts an ASCII hexadecimal string into an integer.
/// This routine is used by the type classes as they read their INI sections, where the
/// entry name is a hexadecimal object ID that must be handed to the swizzler. Conversion
/// stops at the first character that is not a hexadecimal digit.
/// </summary>
/// <returns>Returns with the integer value of the string.</returns>
int ahtoi(const char * str)
{
	int integer = 0;
	while (str) {
		if (!isxdigit((unsigned char)*str)) {
			break;
		}
		char ch = *str;

		integer *= 16; /// desired base is 16
		str++;

		/// is digit?
		if (ch >= '0' && ch <= '9') {
			integer += ch - '0';
		} else {
			integer += toupper((unsigned char)ch) - ('A' - 10);
		}
	}
	return(integer);
}
