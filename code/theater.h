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

#include "stringid.h"
#include "theater.hh"

class CCINIClass;


class TheaterClass
{
	public:
		TheaterClass(char const * name, bool listed = true);
		~TheaterClass(void);

		char const * Name(void) const {return(IniName);}

		bool Read_INI(CCINIClass const & ini);

		static TheaterType From_Name(char const * name);
		static TheaterClass * Find_Or_Make(char const * name);
		static TheaterClass const & As_Reference(TheaterType theater);

		static void One_Time(void);
		static void Clear(void);

	public:
		TStringID<24> IniName;

		/*
		 * These name the files a theater loads. Root gives <Root>.MIX, <Root>.PAL and the
		 * <Root>.INI tile set control file; IsoRoot gives the <IsoRoot>.MIX the tile artwork
		 * is read from. Suffix is the extension theater artwork carries, and also names
		 * <Suffix>.MIX, ISO<Suffix>.PAL and UNIT<Suffix>.PAL. MMSuffix is the extension tried
		 * where a tile set allows marble madness artwork and the theater's own file is missing.
		 */
		TStringID<16> Root;
		TStringID<16> IsoRoot;
		TStringID<8> Suffix;
		TStringID<8> MMSuffix;

		/*
		 * Artwork marked NewTheater carries this as the second character of its name. A letter
		 * that also opens artwork following no such convention will capture it.
		 */
		char ImageLetter;

		/*
		 * Terrain occupation bits come in a temperate and a snow pair, so a theater picks one
		 * side of it rather than adding a third. This also darkens the waypoint path line.
		 */
		bool IsArctic;

		bool IsIceGrowth;

		/*
		 * How far a cell's two terrain colors are scaled on the radar, at ground level and at
		 * the top of the height range, with the cell interpolated between them by its height.
		 */
		float LowRadarBrightness;
		float HighRadarBrightness;
};
