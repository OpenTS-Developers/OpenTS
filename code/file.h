/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2020 Electronic Arts Inc.
 * Copyright 2020-2024 Vanilla Conquer contributors
 * Copyright 2026 OpenTS contributors
 *
 * Contains material derived from Electronic Arts source code and from Vanilla
 * Conquer (https://github.com/TheAssemblyArmada/Vanilla-Conquer).
 * Modified by OpenTS contributors, 2026.
 * EA's GPLv3 Section 7 additional terms and supplemental warranty
 * disclaimers apply; see LICENSE.md.
 ******************************************************************************/

#pragma once

void Resolve_File(char * fname);

class Find_File_Data
{
	public:
		static Find_File_Data * CreateFindData();

		virtual ~Find_File_Data() {}
		virtual const char * GetName() const = 0;
		virtual const char * GetFullName() const { return nullptr; };
		virtual unsigned int GetTime() const = 0;

		virtual bool FindFirst(const char * fname) = 0;
		virtual bool FindNext() = 0;
		virtual void Close() = 0;
};

extern bool Find_First(const char * fname, unsigned int mode, Find_File_Data ** ffblk);
extern bool Find_Next(Find_File_Data * ffblk);
extern void Find_Close(Find_File_Data * ffblk);
