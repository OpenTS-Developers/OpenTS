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
 *                     $Archive:: /VSS_Sync/wwlib/random.cpp                                  $*
 *                                                                                             *
 *                      $Author:: Vss_sync                                                    $*
 *                                                                                             *
 *                     $Modtime:: 8/29/01 10:24p                                              $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   RandomClass::RandomClass -- Constructor for the random number class.                      *
 *   RandomClass::operator() -- Fetches the next random number in the sequence.                *
 *   RandomClass::operator() -- Ranged random number generator.                                *
 *   Random2Class::Random2Class -- Constructor for the random class.                           *
 *   Random2Class::operator -- Generates a random number between two values.                   *
 *   Random2Class::operator -- Randomizer function that returns value.                         *
 *   Random3Class::Random3Class -- Initializer for the random number generator.                *
 *   Random3Class::operator -- Generates a random number between two values.                   *
 *   Random3Class::operator -- Random number generator function.                               *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "random.h"

// Timing tests for random these random number generators in seconds for
// 10000000 iterations. Testing done by Hector Yee, 6/20/01
// using DIEHARD and multidimensional Monte-Carlo Integration
/*
/*
**	Time for Random=0.156000
** Time for Random2=0.250000
** Time for Random3=1.281000
** Time for Built in Rand()=0.813000
** Results from DIEHARD battery of tests
** A p-value of 0.0 or 1.0 means it fails. Anything inbetween is ok.
** R0 - FAILED almost all tests, didn't complete squeeze test
** R2 - Passed all tests, p-value 0.6
** R3 - Failed 11 of 253 tests, p-value 1.0
** Rand() - FAILED almost all tests, didn't complete squeeze test
** Results from Montecarlo Integration 2-96 dimensions
** R0 - starts breaking in 24 dimensions
** R2 - Ok, but will repeat with short period. Huge spike at 300000 samples in 64 dimensions
** R3 - Strong bias from 2 dimensions up
** Rand() - starts breaking in 24 dimensions
*/


/***********************************************************************************************
 * RandomClass::RandomClass -- Constructor for the random number class.                        *
 *                                                                                             *
 *    This constructor can take an integer as a parameter. This allows the class to be         *
 *    constructed by assigning an integer to an existing object. The compiler creates a        *
 *    temporary with the constructor and then performs a copy constructor operation.           *
 *                                                                                             *
 * INPUT:   seed  -- The optional starting seed value to use.                                  *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/27/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
RandomClass::RandomClass(unsigned seed) :
	Seed(seed)
{
}


/***********************************************************************************************
 * RandomClass::operator() -- Fetches the next random number in the sequence.                  *
 *                                                                                             *
 *    This routine will fetch the next random number in the sequence.                          *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the next random number.                                               *
 *                                                                                             *
 * WARNINGS:   This routine modifies the seed value so that subsequent calls will not return   *
 *             the same value. Take note that this routine only returns 15 bits of             *
 *             random number.                                                                  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/27/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
int RandomClass::operator ()(void)
{
	/*
	**	Transform the seed value into the next number in the sequence.
	*/
	Seed = (Seed * MULT_CONSTANT) + ADD_CONSTANT;

	/*
	**	Extract the 'random' bits from the seed and return that value as the
	**	random number result.
	*/
	return((Seed >> THROW_AWAY_BITS) & (~((~0) << SIGNIFICANT_BITS)));
}


/***********************************************************************************************
 * RandomClass::operator() -- Ranged random number generator.                                  *
 *                                                                                             *
 *    This function will return with a random number within the range specified. This replaces *
 *    the functionality of IRandom() in the old library.                                       *
 *                                                                                             *
 * INPUT:   minval   -- The minimum value to return from the function.                         *
 *                                                                                             *
 *          maxval   -- The maximum value to return from the function.                         *
 *                                                                                             *
 * OUTPUT:  Returns with a random number that falls between the minval and maxval (inclusive). *
 *                                                                                             *
 * WARNINGS:   The range specified must fall within the maximum bit significance of the        *
 *             random number algorithm (15 bits), otherwise the value returned will be         *
 *             decidedly non-random.                                                           *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/27/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
int RandomClass::operator() (int minval, int maxval)
{
	return(Pick_Random_Number(*this, minval, maxval));
}


/***********************************************************************************************
 * Random2Class::Random2Class -- Constructor for the random class.                             *
 *                                                                                             *
 *    This will initialize the random class object with the seed value specified.              *
 *                                                                                             *
 * INPUT:   seed  -- The seed value used to scramble the random number generator.              *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/14/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
Random2Class::Random2Class(unsigned seed) :
	Index1(0),
	Index2(103)
{
	Random3Class random(seed);

	for (int index = 0; index < TABLE_SIZE; index++) {
		Table[index] = random;
	}
}


/***********************************************************************************************
 * Random2Class::operator -- Randomizer function that returns value.                           *
 *                                                                                             *
 *    This is the random number generator function. It will generate a random number and       *
 *    return the value.                                                                        *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with a random number.                                                      *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/20/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
int Random2Class::operator() (void)
{
	Table[Index1] ^= Table[Index2];
	int val = Table[Index1];

	Index1++;
	Index2++;

	if (Index1 >= TABLE_SIZE) Index1 = 0;
	if (Index2 >= TABLE_SIZE) Index2 = 0;

	return(val);
}


/***********************************************************************************************
 * Random2Class::operator -- Generates a random number between two values.                     *
 *                                                                                             *
 *    This routine will generate a random number between the two values specified. It uses     *
 *    a method that will not bias the values in any way.                                       *
 *                                                                                             *
 * INPUT:   minval   -- The minium return value (inclusive).                                   *
 *                                                                                             *
 *          maxval   -- The maximum return value (inclusive).                                  *
 *                                                                                             *
 * OUTPUT:  Returns with a random number that falls between the two values (inclusive).        *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/20/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
int Random2Class::operator() (int minval, int maxval)
{
	return(Pick_Random_Number(*this, minval, maxval));
}


/*
**	This is the seed table for the Random3Class generator. These ensure
**	that the algorithm is not vulnerable to being primed with a weak seed
**	and thus prevents the algorithm from breaking down as a result.
*/
int Random3Class::Mix1[TABLE_SIZE] = {
	-0x45569779, +0x1e17d32c, +0x03bcdc3c, +0x0f33d1b2,
	+0x76a6491d, -0x3a8f27a3, -0x1c7d4e1d, +0x78db4362,
	+0x7439a9d4, -0x6315753b, -0x76ac83a4, +0x2588f55d,
	+0x415b5e1d,  0x216e3d95, -0x7a399d19, +0x5e8ab368,
	+0x3ea5cc8c, -0x2d95f08c, -0x0c56ddd5, +0x48aad7e4
};

int Random3Class::Mix2[TABLE_SIZE] = {
	+0x4b0f3b58, -0x178b0f3d, +0x6955c5a6, +0x55a7ca46,
	+0x4d9a9d86, -0x01d75e6b, -0x4e35879b, +0x6b235751,
	-0x6566859f, -0x55916a38, -0x5556711f, +0x5af9154c,
	-0x0371dd9d, +0x390f5e8c, +0x58ffd802, -0x53f5a146,
	-0x53b78b0a, -0x5620f6ed, -0x7941b38c, -0x12d3edc5
};


/***********************************************************************************************
 * Random3Class::Random3Class -- Initializer for the random number generator.                  *
 *                                                                                             *
 *    This initializes the random number generator with the seed values specified. Due to a    *
 *    peculiarity of the random number design, the second seed value can be used to find the   *
 *    Nth random number generated by this algorithm. The second seed is used as the Nth index  *
 *    value.                                                                                   *
 *                                                                                             *
 * INPUT:   seed1 -- The seed value to inialize the generator with. It is suggest that some    *
 *                   random value based on user input seed this value.                         *
 *                                                                                             *
 *          seed2 -- The auxiliary seed value. This adds randomness and thus allows this       *
 *                   algorithm to use a 64 bit seed. This second seed also serves as an index  *
 *                   into the algorithm such that the value passed as 'seed2' is will prime    *
 *                   the generator to return that Nth random number when it is called.         *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   As with all random number generators. Randomness is only as strong as the       *
 *             initial seed value.                                                             *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/20/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
Random3Class::Random3Class(unsigned seed1, unsigned seed2) :
	Seed(seed1),
	Index(seed2)
{
}


/***********************************************************************************************
 * Random3Class::operator -- Random number generator function.                                 *
 *                                                                                             *
 *    This routine generates a random number. The number returned is strongly random and is    *
 *    nearly good enough for cryptography.                                                     *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with a 32bit random number.                                                *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/20/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
int Random3Class::operator() (void)
{
	int loword = Seed;
	int hiword = Index++;
	for (int i = 0; i < 4; i++) {
		int hihold  = hiword;                                   /// save hiword for later
		int temp    = hihold ^  Mix1[i];                        /// mix up bits of hiword
		int itmpl   = temp   &  0xffff;                         /// decompose to hi & lo
		int itmph   = temp   >> 16;                             /// 16-bit words
		temp    = itmpl * itmpl + ~(itmph * itmph);             /// do a multiplicative mix
		temp    = (temp >> 16) | (temp << 16);                  /// swap hi and lo halves
		hiword  = loword ^ ((temp ^ Mix2[i]) + itmpl * itmph);  /// loword mix
		loword  = hihold;                                       /// old hiword is loword
	}
	return(hiword);
}


/***********************************************************************************************
 * Random3Class::operator -- Generates a random number between two values.                     *
 *                                                                                             *
 *    This routine will generate a random number between the two values specified. It uses     *
 *    a method that will not bias the values in any way.                                       *
 *                                                                                             *
 * INPUT:   minval   -- The minium return value (inclusive).                                   *
 *                                                                                             *
 *          maxval   -- The maximum return value (inclusive).                                  *
 *                                                                                             *
 * OUTPUT:  Returns with a random number that falls between the two values (inclusive).        *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/20/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
int Random3Class::operator() (int minval, int maxval)
{
	return(Pick_Random_Number(*this, minval, maxval));
}
