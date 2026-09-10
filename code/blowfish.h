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
 *                     $Archive:: /Commando/Library/BLOWFISH.H                                $*
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 7/22/97 11:37a                                              $*
 *                                                                                             *
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "win.h"

#include <climits>

/*
**	This engine will process data blocks by encryption and decryption.
**	The "Blowfish" algorithm is in the public domain. It uses
**	a Feistal network (similar to IDEA). It has no known
**	weaknesses, but is still relatively new. Blowfish is particularly strong
**	against brute force attacks. It is also quite strong against linear and
**	differential cryptanalysis. Unlike public key encription, it is very
**	fast at encryption, as far as cryptography goes. Its weakness is that
**	it takes a relatively long time to set up with a new key (1/100th of
**	a second on a P6-200). The time to set up a key is equivalent to
**	encrypting 4240 bytes.
*/
class BlowfishEngine {
	public:
		BlowfishEngine(void);
		~BlowfishEngine(void);

		void Submit_Key(void const * key, int length);

		int Encrypt(void const * plaintext, int length, void * cyphertext);
		int Decrypt(void const * cyphertext, int length, void * plaintext);

		/*
		**	This is the maximum key length supported.
		*/
		enum {MAX_KEY_LENGTH=56};

	private:
		bool IsKeyed;

		void Sub_Key_Encrypt(unsigned int & left, unsigned int & right);

		void Process_Block(void const * plaintext, void * cyphertext, unsigned int const * ptable);

		enum {
			ROUNDS = 16,		// Feistal round count (16 is standard).
			BYTES_PER_BLOCK=8	// The number of bytes in each cypher block (don't change).
		};

		/*
		**	Initialization data for sub keys. The initial values are constant and
		**	filled with a number generated from pi. Thus they are not random but
		**	they don't hold a weak pattern either.
		*/
		static unsigned int const P_Init[(int)ROUNDS+2];
		static unsigned int const S_Init[4][UCHAR_MAX+1];

		/*
		**	Permutation tables for encryption and decryption.
		*/
		unsigned int P_Encrypt[(int)ROUNDS+2];
		unsigned int P_Decrypt[(int)ROUNDS+2];

		/*
		**	S-Box tables (four).
		*/
		unsigned int bf_S[4][UCHAR_MAX+1];
};
