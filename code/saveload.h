/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2025 Electronic Arts Inc.
 * Copyright 2026 OpenTS contributors
 *
 * Contains material derived from Electronic Arts source code.
 * Modified by OpenTS contributors, 2026.
 * EA's GPLv3 Section 7 additional terms and supplemental warranty
 * disclaimers apply; see LICENSE.md.
 ******************************************************************************/

#pragma once

#include "persist.h"

#include <cstdio>
#include <memory>

class SaveStreamClass;
class SaveVersionInfo;
struct ILocomotion;

/*
**	SAVELOAD.CPP
*/
int Load_Misc_Values(SaveStreamClass & stream);
int Save_Misc_Values(SaveStreamClass & stream);

// A loaded object is handed back owned; one that belongs to a heap is released there by
// the caller that puts it in one. docs/SAVE-FORMAT.md records what a record holds.
bool Save_Object(SaveStreamClass & stream, IPersistent * object);
bool Save_Object(SaveStreamClass & stream, ILocomotion * locomotion);
std::unique_ptr<IPersistent> Load_Object(SaveStreamClass & stream,
	bool (*accepts)(IPersistent const * object) = nullptr);

/// <summary>
/// Loads the record next in the stream and requires it to be of the class asked for.
/// </summary>
/// <returns>The object, owned by the caller, or nothing with the stream failed when the
/// record holds another class. A record of the wrong class is destroyed before it can take
/// its place, so the test happens while the object is still only the reader's.</returns>
template<class T>
std::unique_ptr<T> Load_Object_As(SaveStreamClass & stream)
{
	std::unique_ptr<IPersistent> object = Load_Object(stream, [](IPersistent const * candidate) {
		return(dynamic_cast<T const *>(candidate) != nullptr);
	});

	// The record was accepted only if it holds a T, so this cast answers for what was loaded.
	T * const wanted = dynamic_cast<T *>(object.get());
	if (wanted != nullptr) {
		object.release();
	}
	return(std::unique_ptr<T>(wanted));
}

bool Get_Savefile_Info(char const * name, SaveVersionInfo * info);
bool Save_Game(const char *file_name, char const * descr);
bool Load_Game(const char *file_name);
bool Reconcile_Players(void);
void Print_Heap_CRCs(FILE * fp);

extern unsigned int ExpectedGameVersion;
