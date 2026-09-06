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

#include <cstdio>

struct IStream;
class SaveVersionInfo;

/*
**	SAVELOAD.CPP
*/
int Load_Misc_Values(IStream * stream);
int Save_Misc_Values(IStream * stream);
bool Get_Savefile_Info(char const * name, SaveVersionInfo * info);
bool Save_Game(const char *file_name, char const * descr);
bool Load_Game(const char *file_name);
bool Reconcile_Players(void);
void Print_Heap_CRCs(FILE * fp);

extern unsigned int ExpectedGameVersion;
