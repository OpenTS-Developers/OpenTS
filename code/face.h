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

/* $Header: /CounterStrike/FACE.H 1     3/03/97 10:24a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : FACE.H                                                       *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : 03/08/96                                                     *
 *                                                                                             *
 *                  Last Update : March 8, 1996 [JLB]                                          *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "coord.h"
#include "visualc.h"

#include <cmath>
#include <cstdlib>

class DirType;

inline DirType Direction(Coord const & coord1, Coord const & coord2);

/*
 * Free rather than member so that either side may be an unconverted Dir256,
 * FacingType or int. The ordering operators weigh magnitude, so they mean
 * something only when comparing a turn threshold against a difference.
 */
bool operator == (DirType const & lvalue, DirType const & rvalue);
bool operator != (DirType const & lvalue, DirType const & rvalue);
bool operator < (DirType const & lvalue, DirType const & rvalue);
bool operator > (DirType const & lvalue, DirType const & rvalue);
bool operator <= (DirType const & lvalue, DirType const & rvalue);
bool operator >= (DirType const & lvalue, DirType const & rvalue);

/*
 * The 32 facing grid of the body and turret artwork. Typed only so that a
 * DirType built from one picks the shifting constructor over the raw one.
 */
enum Dir32 {};

// Enumerations of the facing values returned from Desired_Facing().
enum Dir256 {
	DIR_MIN=0,
	DIR_N=0,
	DIR_NE=1<<5,
	DIR_E=2<<5,
	DIR_SE=3<<5,
	DIR_S=4<<5,
	DIR_SW=5<<5,
	DIR_W=6<<5,
	DIR_NW=7<<5,
	DIR_MAX=255,

	/*
	 * One step of each facing grid, for turn tolerances and for composing
	 * the directions that fall between compass points.
	 */
	DIR_STEP_2=1<<7,		/// 180 degrees, one of 2 facings
	DIR_STEP_4=1<<6,		/// 90 degrees
	DIR_STEP_8=1<<5,		/// 45 degrees
	DIR_STEP_16=1<<4,		/// 22.5 degrees
	DIR_STEP_32=1<<3,		/// 11.25 degrees
	DIR_STEP_64=1<<2,		/// 5.625 degrees
	DIR_STEP_128=1<<1,		/// 2.8125 degrees
	DIR_STEP_256=1			/// 1.40625 degrees
};

#define DIR_SW_X1	Dir256((5<<5)-8)
#define DIR_SW_X2	Dir256((5<<5)-16)


class DirType
{
	public:
		DirType(void) = default;
		DirType(int dir) { Facing = dir; }
		DirType(FacingType dir) { From_Facing(dir); }
		DirType(Dir32 dir) { From_Dir32(dir); }
		DirType(Dir256 dir) { From_Dir256(dir); }
		DirType(double rad) { From_Radian(rad); }

		DirType & Direction(Coord const & coord1, Coord const & coord2)
		{
			if (coord1.TPoint2D<int>::operator==(coord2)) {
				Facing = 0;
			}

			*this = DirType(std::atan2((double)coord1.Y - (double)coord2.Y, (double)coord2.X - (double)coord1.X));

			return *this;
		}

		DirType & Direction(Cell const & cell1, Cell const & cell2)
		{
			if (cell1 == cell2) {
				Facing = 0;
			}

			*this = DirType(std::atan2((double)cell1.Y - (double)cell2.Y, (double)cell2.X - (double)cell1.X));

			return *this;
		}
		/*
		 * Builds the direction from a facing grid index. The shift is 16 minus
		 * the log2 of the grid size; the store width supplies the wrap.
		 */
		void From_Dir4(int dir)
		{
			Facing = 0;
			Facing |= (dir << 14);
		}

		void From_Facing(FacingType dir)
		{
			Facing = 0;
			Facing |= (dir << 13);
		}

		void From_Dir32(Dir32 dir)
		{
			Facing = 0;
			Facing |= (dir << 11);
		}

		void From_Dir256(Dir256 dir)
		{
			Facing = 0;
			Facing |= (dir << 8);
		}

		void From_Radian(double rad)
		{
			/// The circle spans 65534 binary-angle units rather than 65536 -- an
			/// off-by-one that the reverse conversion shares.
			Facing = short((rad - M_PI / 2) / -DEG_TO_RAD(360.0 / 65534.0));
		}

		/*
		 * Rounds to the nearest facing of the grid without wrapping, so a
		 * direction past the last facing comes back as the count itself and the
		 * caller must wrap -- by mask, or by feeding it back through From_.
		 *
		 * These read the whole word, padding included, which is why the wrap can
		 * never be dropped downstream.
		 */
		unsigned int	Round_To_4(void) const		{ return	((((unsigned int)Raw >> 13) + 1) >> 1); }
		unsigned int	Round_To_8(void) const		{ return	((((unsigned int)Raw >> 12) + 1) >> 1); }
		unsigned int	Round_To_16(void) const		{ return	((((unsigned int)Raw >> 11) + 1) >> 1); }
		unsigned int	Round_To_32(void) const		{ return	((((unsigned int)Raw >> 10) + 1) >> 1); }
		unsigned int	Round_To_64(void) const		{ return	((((unsigned int)Raw >> 9) + 1) >> 1); }
		unsigned int	Round_To_256(void) const	{ return	((((unsigned int)Raw >> 7) + 1) >> 1); }

		/*
		 * The same rounding, wrapped to a legal index of the grid.
		 */
		int			As_Dir4(void) const		{ return				(Round_To_4() % (4)); }
		FacingType	As_Dir8(void) const		{ return (FacingType)	(Round_To_8() % (8)); }
		Dir32		As_Dir32(void) const	{ return (Dir32)		(Round_To_32() % (32)); }
		Dir256		As_Dir256(void) const	{ return (Dir256)		(Round_To_256() % (256)); }

		int			As_Axis(void) const		{ return (int)			(Round_To_8() % (4)); }

		/*
		 * As_Facing reads Facing where the As_Dir conversions read Raw -- the
		 * whole difference from As_Dir8. They always agree, since padding only
		 * reaches bits above the wrap, so it decides folding rather than value:
		 * only this form folds for a constant direction whatever the padding
		 * holds, and the constant DIR_ conversions are built on it.
		 */
		FacingType	As_Facing(void) const	{ return (FacingType)	((((((unsigned int)Facing >> 12) + 1) >> 1)) % (8)); }

		/*
		 * Truncates onto the grid in place -- down, not to nearest -- handing
		 * back a reference so the result passes on without binding a temporary.
		 * Snapping to 256 cannot change a direction built from a 256 grid
		 * index, which is the only way it is used.
		 */
		DirType const & Snap_To_4(void)		{ Facing &= 0xC000; return(*this); }
		DirType const & Snap_To_8(void)		{ Facing &= 0xE000; return(*this); }
		DirType const & Snap_To_32(void)	{ Facing &= 0xF800; return(*this); }
		DirType const & Snap_To_256(void)	{ Facing &= 0xFF00; return(*this); }

		double		As_Radian(void) const { return (double)		((Facing - 16383) * -DEG_TO_RAD(360.0 / 65534.0)); }
		double		As_Radian32(void) const { return (double)		((As_Dir32() - 8) * -DEG_TO_RAD(360.0 / 32.0)); }

		int			As_Int(void) const { return(Facing); }

		DirType Left_90(void) const { return(DirType(Facing - 0x3FFF)); }
		DirType Left_180(void) const { return(DirType(Facing - 0x7FFF)); }
		DirType Left_270(void) const { return(DirType(Facing - 0xBFFF)); }
		DirType Right_90(void) const { return(DirType(Facing + 0x3FFF)); }
		DirType Right_180(void) const { return(DirType(Facing + 0x7FFF)); }
		DirType Right_270(void) const { return(DirType(Facing + 0xBFFF)); }

		DirType operator + (DirType const & rvalue) const { return DirType(Facing + rvalue.Facing); }
		DirType operator - (DirType const & rvalue) const { return DirType(Facing - rvalue.Facing); }
		DirType operator * (int rvalue) const { return DirType(Facing * rvalue); }
		DirType operator / (int rvalue) const { return DirType(Facing / rvalue); }

		const DirType & operator += (DirType const & rvalue) { Facing += rvalue.Facing; return *this; }
		const DirType & operator -= (DirType const & rvalue) { Facing -= rvalue.Facing; return *this; }
		const DirType & operator *= (int rvalue) { Facing *= rvalue; return *this; }
		const DirType & operator /= (int rvalue) { Facing /= rvalue; return *this; }

		bool Is_Complete_Turn(DirType const & towards, DirType const & by)
		{
			DirType diff(*this - towards);
			if (by >= diff) {
				return(true);
			}
			return(false);
		}

		bool Turn(DirType const & towards, DirType const & by)
		{
			if (Is_Complete_Turn(towards, by)) {
				Facing = towards.Facing;
				return(true);
			}

			if ((towards - *this).Facing < 0) {
				*this -= by;
			} else {
				*this += by;
			}
			return(false);
		}

		/*
		 * Carries the direction to or from a save game. The whole storage travels rather than
		 * the 16 bit angle alone, since the rounding conversions read the padding with it.
		 */
		template<typename S>
		void Serialize(S & stream)
		{
			stream.Serialize(Raw);
		}

	/// probably should be private.
	public:
		union {
			/*
			 * This is the direction, expressed as a 16 bit binary angle -- zero is north and
			 * the value rises clockwise, so that 0x4000 is east. Every write goes through
			 * this member, so the upper half of the storage keeps whatever it was built with.
			 */
			short Facing;

			/*
			 * This is the whole of the storage read as one dword, which is what the rounding
			 * conversions read. Construction zeroes it, but a direction restored from a save
			 * game can carry any value in the upper half, which is why those conversions must
			 * always wrap.
			 */
			int Raw = 0;
		};
};

inline bool operator == (DirType const & lvalue, DirType const & rvalue) { return lvalue.Facing == rvalue.Facing; }
inline bool operator != (DirType const & lvalue, DirType const & rvalue) { return lvalue.Facing != rvalue.Facing; }

inline bool operator < (DirType const & lvalue, DirType const & rvalue) {return abs(lvalue.Facing) < abs(rvalue.Facing); }
inline bool operator > (DirType const & lvalue, DirType const & rvalue) {return abs(lvalue.Facing) > abs(rvalue.Facing); }
inline bool operator <= (DirType const & lvalue, DirType const & rvalue) {return abs(lvalue.Facing) <= abs(rvalue.Facing); }
inline bool operator >= (DirType const & lvalue, DirType const & rvalue) {return abs(lvalue.Facing) >= abs(rvalue.Facing); }

// The eighth of a turn puts index zero on northwest, where the eight facing layout has it.
inline int Shape_Facing_Index(DirType dir, int count)
{
	switch (count) {
		case 8:		return((int)((dir.Round_To_8()  + 1) % 8));
		case 16:	return((int)((dir.Round_To_16() + 2) % 16));
		case 32:	return((int)((dir.Round_To_32() + 4) % 32));
		case 64:	return((int)((dir.Round_To_64() + 8) % 64));
		default:	return(0);
	}
}

