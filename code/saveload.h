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

class SaveStreamClass;
class SaveVersionInfo;
struct ILocomotion;

/*
**	SAVELOAD.CPP
*/
int Load_Misc_Values(SaveStreamClass & stream);
int Save_Misc_Values(SaveStreamClass & stream);

// A locomotor loaded this way is handed back unowned; the caller takes it.
// docs/SAVE-FORMAT.md records what a record holds.
HRESULT Save_Object(SaveStreamClass & stream, IPersistent * object);
HRESULT Save_Object(SaveStreamClass & stream, ILocomotion * locomotion);
IPersistent * Load_Object(SaveStreamClass & stream, bool (*accepts)(IPersistent const * object) = nullptr);

/// <summary>
/// Loads the record next in the stream and requires it to be of the class asked for.
/// </summary>
/// <returns>The object, or NULL with the stream failed when the record holds another
/// class. A record of the wrong class is destroyed before it can take its place, so the
/// test happens while the object is still only the reader's.</returns>
template<class T>
T * Load_Object_As(SaveStreamClass & stream)
{
	IPersistent * const object = Load_Object(stream, [](IPersistent const * candidate) {
		return(dynamic_cast<T const *>(candidate) != nullptr);
	});
	return(dynamic_cast<T *>(object));
}

bool Get_Savefile_Info(char const * name, SaveVersionInfo * info);
bool Save_Game(const char *file_name, char const * descr);
bool Load_Game(const char *file_name);
bool Reconcile_Players(void);
void Print_Heap_CRCs(FILE * fp);

extern unsigned int ExpectedGameVersion;
