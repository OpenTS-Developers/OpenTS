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

/* $Header: /CounterStrike/REGION.H 1     3/03/97 10:25a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : REGION.H                                                     *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : 03/09/95                                                     *
 *                                                                                             *
 *                  Last Update : March 9, 1995 [JLB]                                          *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include <cstring>


class RegionClass {
	public:
		RegionClass(void) {/*Threat = 0;*/};
		~RegionClass(void) {};

		int operator != (RegionClass const & region) {return(memcmp(this, &region, sizeof(RegionClass)));};
		int operator == (RegionClass const & region) {return(!memcmp(this, &region, sizeof(RegionClass)));};
		int operator > (RegionClass const & region) {return(memcmp(this, &region, sizeof(RegionClass)) > 0);};
		int operator < (RegionClass const & region) {return(memcmp(this, &region, sizeof(RegionClass)) < 0);};

		void Reset_Threat(void) {Threat = 0;};
		void Adjust_Threat(int threat, int neg) {if (neg) Threat -= threat; else Threat+= threat; if (Threat < 0) Threat = 0;};
		int Threat_Value(void) const {return(Threat);};

		// Carries this region's threat rating to or from a save game.
		template<typename S>
		void Serialize(S & stream)
		{
			stream.Serialize(Threat);
		}

	protected:
		int  Threat;
};
