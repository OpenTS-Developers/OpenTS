/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include "abstract.h"
#include "coord.h"
#include "globals.h"

#include "facing.hh"

class CCINIClass;


class TubeClass : public AbstractClass
{
		typedef AbstractClass BASECLASS;

		virtual ClassID Class_ID(void) const override;

		virtual void Serialize(SaveStreamClass & stream) override;

	public:
		TubeClass(Cell const &cell = CELL_NONE, FacingType dir = FACING_N);

		virtual ~TubeClass(void) override;

		virtual RTTIType Fetch_RTTI(void) const override;
		virtual void Compute_CRC(CRCEngine & crc) const override;

		static void Assign_Tubes(void);
		static void Write_INI(CCINIClass & ini);
		static void Read_INI(CCINIClass const & ini);

		bool Not_In_Tube(Cell const & cell) const;
		void Add_Direction(FacingType dir);

	public:
		/*
		 * This is the cell the tunnel is entered from. That cell keeps this tunnel's index in
		 * its Tube member, which is how something standing there finds its way underground.
		 */
		Cell Enter;

		/*
		 * This is the cell the tunnel comes out of. It travels along with the tunnel as
		 * directions are appended to it.
		 */
		Cell Exit;

		/*
		 * This is the facing that the tunnel mouth is entered by. A unit must be heading
		 * roughly this way to be allowed underground, and it turns to face this way as it
		 * disappears.
		 */
		FacingType EnterDir;

		/*
		 * This is the chain of facings that traces the tunnel's path underground, one step per
		 * cell from the entrance to the exit. The path is terminated by FACING_NONE.
		 */
		FacingType Dirs[100];

		/*
		 * This is the number of steps recorded in the Dirs path. A tunnel may be no longer
		 * than 99 steps, after which it refuses to be extended any further.
		 */
		int Count;

		/*
		 * This is the name of the map file section that the tunnels are written to and read
		 * back from.
		 */
		static char const * INI_NAME;
};
