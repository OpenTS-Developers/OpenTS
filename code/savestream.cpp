/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "always.h"

#include "savestream.h"

#include "saveload.h"

#include <cstring>


unsigned int LoadedSaveVersion = 0;


/// <summary>
/// Builds a save stream over the buffer given, appending to it when saving and reading
/// it from the start when loading.
/// </summary>
/// <param name="buffer">The bytes of the saved game, which must outlive this stream.</param>
/// <param name="mode">Is this stream saving or loading?</param>
SaveStreamClass::SaveStreamClass(std::vector<unsigned char> & buffer, ModeType mode) :
	Buffer(&buffer),
	Cursor(mode == MODE_SAVE ? (unsigned int)buffer.size() : 0),
	Limit((unsigned int)buffer.size()),
	Mode(mode),
	Failed(false),
	FormatVersion(mode == MODE_LOAD ? LoadedSaveVersion : ExpectedGameVersion),
	OwnerType(NULL),
	OwnerID(0)
{
}


void SaveStreamClass::Fail(void)
{
	Failed = true;
}


/// <summary>
/// Moves the bytes of one value between the caller and the stream.
/// A load that runs out of stream in the middle of a value fails the stream rather than
/// hand back a partly read value, and every later call is ignored. A negative length is
/// a count that wrapped, and fails the same way.
/// </summary>
void SaveStreamClass::Serialize_Bytes(void * data, int length)
{
	if (Failed) {
		return;
	}
	if (length < 0) {
		Failed = true;
		return;
	}
	if (length == 0) {
		return;
	}

	unsigned char * const bytes = (unsigned char *)data;

	if (Mode == MODE_SAVE) {
		Buffer->insert(Buffer->end(), bytes, bytes + length);
		Cursor = (unsigned int)Buffer->size();
	} else {
		// The bound is the record being read rather than the whole stream, so a member that
		// reads more than its own is refused instead of quietly spending the bytes of the
		// record after it.
		if ((unsigned int)length > Limit - Cursor) {
			Failed = true;
			return;
		}
		memcpy(bytes, Buffer->data() + Cursor, (std::size_t)length);
		Cursor += (unsigned int)length;
	}
}


// A saver patches a length it could not know until the record was written.
void SaveStreamClass::Overwrite_Bytes(unsigned int offset, void const * data, int length)
{
	if (Failed || Mode != MODE_SAVE || length <= 0) {
		return;
	}
	if (offset > Buffer->size() || (unsigned int)length > Buffer->size() - offset) {
		Failed = true;
		return;
	}
	memcpy(Buffer->data() + offset, data, (std::size_t)length);
}
