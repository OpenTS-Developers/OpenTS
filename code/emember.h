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

class TechnoTypeClass;
class CRCEngine;

/*
**	This class specifies the quantity and type of members desired for the
**	team.
*/
class EnlistedMemberClass
{
	public:
		EnlistedMemberClass(void) : Quantity(0), Class(NULL) {}
		EnlistedMemberClass(int quantity, TechnoTypeClass const *type) : Quantity(quantity), Class(type) {}
		EnlistedMemberClass(char const * entry);


		char const * Build_INI_Entry(void) const;

		void Compute_CRC(CRCEngine & crc) const;

		// Carries the member entry to or from a save game.
		template<typename S>
		void Serialize(S & stream)
		{
			stream.Serialize(Quantity);
			stream.Serialize(Class);
		}

		int Quantity;					// Number of objects desired for this type.
		TechnoTypeClass const * Class;	// The type of object desired.
};
