/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include <cstring>

// The identity a persistent class is saved and named by. The sixteen bytes are those of
// the COM class identifier the class once registered, kept as they are because saved
// games and the Locomotor= key carry them.
struct ClassID
{
	unsigned int Data1;
	unsigned short Data2;
	unsigned short Data3;
	unsigned char Data4[8];
};

static_assert(sizeof(ClassID) == 16, "a class identifier is sixteen bytes on disk");

inline bool operator==(ClassID const & left, ClassID const & right)
{
	return(std::memcmp(&left, &right, sizeof(ClassID)) == 0);
}

inline bool operator!=(ClassID const & left, ClassID const & right)
{
	return(!(left == right));
}
