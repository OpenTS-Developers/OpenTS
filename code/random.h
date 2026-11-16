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

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                     $Archive:: /VSS_Sync/wwlib/random.h                                    $*
 *                                                                                             *
 *                      $Author:: Vss_sync                                                    $*
 *                                                                                             *
 *                     $Modtime:: 8/29/01 10:24p                                              $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   Pick_Random_Number -- Picks a random number between two values (inclusive).               *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

/*
**	This class functions like a 'magic' int value that returns a random number
**	every time it is read. To set the "random seed" for this, just assign a number
**	to the object (just as you would if it were a true 'int' value). Take note that although
**	this class will return an 'int', the actual significance of the random number is
**	limited to 15 bits (0..32767).
*/
class RandomClass {
	public:
		RandomClass(unsigned seed=0);

		operator int(void) {return(operator()());};
		int operator() (void);
		int operator() (int minval, int maxval);

		enum {
			SIGNIFICANT_BITS=15				// Random number bit significance.
		};

	protected:
		unsigned int Seed;

		/*
		**	Internal working constants that are used to generate the next
		**	random number.
		*/
		enum {
			MULT_CONSTANT=0x41C64E6D,		// K multiplier value.
			ADD_CONSTANT=0x00003039,		// K additive value.
			THROW_AWAY_BITS=10				// Low bits to throw away.
		};
};


/*
**	This class functions like a 'magic' number where it returns a different value every
**	time it is read. It is nearly identical in function to the RandomClass, but has the
**	following improvements.
**
**	1> It generates random numbers very quickly. No multiplies are
**		used in the algorithm.
**
**	2> The return value is a full 32 bits rather than 15 bits of
**		the RandomClass.
**
**	3>	The bit pattern won't ever repeat. (actually it will repeat
**		in about 10 to the 50th power times).
*/
// WARNING!!!!
// This random number generator starts breaking down in 64 dimensions
// behaving very badly in that domain
// HY 6/14/01
class Random2Class {
	public:
		Random2Class(unsigned seed=0);

		operator int(void) {return(operator()());};
		int operator() (void);
		int operator() (int minval, int maxval);

		// The two lagged-Fibonacci table cursors, reported by the out-of-sync diagnostics.
		int Index_1(void) const {return(Index1);}
		int Index_2(void) const {return(Index2);}

		// The value the next draw returns, read without advancing the generator.
		int Peek(void) const {return(Table[Index1] ^ Table[Index2]);}

		enum {
			TABLE_SIZE = 250,
			SIGNIFICANT_BITS=32				// Random number bit significance.
		};

		/*
		 * Carries the generator to or from a save game. The whole lagged Fibonacci table
		 * travels, since a game resumed with a differently seeded generator would drift out
		 * of step with the other machines in a network game.
		 */
		template<typename S>
		void Serialize(S & stream)
		{
			stream.Serialize(Index1);
			stream.Serialize(Index2);
			stream.Serialize(Table);
		}

	protected:
		int Index1;
		int Index2;
		int Table[TABLE_SIZE];
};


/// This code is adapted from the book "Inner Loops" by Rick Booth.
/*
**	This class functions like a 'magic' number where it returns a different value every
**	time it is read. It is nearly identical in function to the RandomClass and Random2Class,
**	but has the following improvements.
**
**	1> The random number returned is very strongly random. Approaching cryptographic
**		quality.
**
**	2> The return value is a full 32 bits rather than 15 bits of
**		the RandomClass.
**
**	3> The bit pattern won't repeat until 2^32 times.
*/
// WARNING!!!!
// This random number generator starts breaking down in 3 dimensions
// exhibiting a strange bias
// HY 6/14/01
class Random3Class {
	public:
		Random3Class(unsigned seed1=0, unsigned seed2=0);

		operator int(void) {return(operator()());};
		int operator() (void);
		int operator() (int minval, int maxval);

		enum {
			TABLE_SIZE = 20,
			SIGNIFICANT_BITS=32				// Random number bit significance.
		};

	protected:
		static int Mix1[TABLE_SIZE];
		static int Mix2[TABLE_SIZE];
		int Seed;
		int Index;
};


/***********************************************************************************************
 * Pick_Random_Number -- Picks a random number between two values (inclusive).                 *
 *                                                                                             *
 *    This is a utility template that works with one of the random number classes. Given a     *
 *    random number generator, this routine will return with a value that lies between the     *
 *    minimum and maximum values specified (subject to the bit precision limitations of the    *
 *    random number generator itself).                                                         *
 *                                                                                             *
 * INPUT:   generator   -- Reference to the random number generator to use for the pick        *
 *                         process.                                                            *
 *                                                                                             *
 *          minval      -- The minimum legal return value (inclusive).                         *
 *                                                                                             *
 *          maxval      -- The maximum legal return value (inclusive).                         *
 *                                                                                             *
 * OUTPUT:  Returns with a random number between the minval and maxval (inclusive).            *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/23/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
template<class T>
int Pick_Random_Number(T & generator, int minval, int maxval)
{
	/*
	**	Test for shortcut case where the range is null and thus
	**	the number to return is actually implicit from the
	**	parameters.
	*/
	if (minval == maxval) return(minval);

	/*
	**	Ensure that the min and max range values are in proper order.
	*/
	if (minval > maxval) {
		int temp = minval;
		minval = maxval;
		maxval = temp;
	}

	/*
	**	Find the highest bit that fits within the magnitude of the
	**	range of random numbers desired. Notice that the scan is
	**	limited to the range of significant bits returned by the
	**	random number algorithm.
	*/
	int magnitude = maxval - minval;
	int highbit = T::SIGNIFICANT_BITS-1;
	while ((magnitude & (1 << highbit)) == 0 && highbit > 0) {
		highbit--;
	}

	/*
	**	Create a full bit mask pattern that has all bits set that just
	**	barely covers the magnitude of the number range desired.
	*/
	int mask = ~( (~0L) << (highbit+1));

	/*
	**	Keep picking random numbers until it fits within the magnitude desired. With a
	**	good random number generator, it will have to perform this loop an average
	**	of one and a half times.
	*/
	int pick = magnitude+1;
	while (pick > magnitude) {
		pick = generator() & mask;
	}

	/*
	**	Finally, bias the random number pick to the start of the range
	**	requested.
	*/
	return(pick + minval);
}
