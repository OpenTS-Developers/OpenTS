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
 *                     $Archive:: /G/wwlib/crc.h                                              $*
 *                                                                                             *
 *                      $Author:: Neal_k                                                      $*
 *                                                                                             *
 *                     $Modtime:: 10/04/99 10:25a                                             $*
 *                                                                                             *
 *                    $Revision:: 4                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include <cstdlib>


// the CRC class defines a few static functions for dealing with CRCs a little differently than
// the CRCEngine. This class uses a algorithm that produces CRC values that have a high probability
// of being unique under most circumstances.
// Note: this code was provided by Byon Garrabrant
//
// 12/09/97 EHC - converted from c to c++ static class and added to crc.h and crc.cpp
//
#define CRC32(c,crc) (CRC::_Table[((unsigned int)(crc) ^ (c)) & 0xFFL] ^ (((crc) >> 8) & 0x00FFFFFFL))
class CRC {

	// CRC for poly 0x04C11DB7
	static unsigned int _Table[256];

public:

	// get the CRC of a block of memory
	static unsigned int	Memory( unsigned char const *data, unsigned int length, unsigned int crc = 0 );

	// get the CRC of a null-terminated string
	static unsigned int	String( const char *string, unsigned int crc = 0 );
};


/*
**	This is a CRC engine class. It will process submitted data and generate a CRC from it.
**	Well, actually, the value returned is not a true CRC. However, it shares the same strength
**	characteristic and is faster to generate than the traditional CRC. This object is treated like
**	a method class. If it is called as a function (using the function operator), it will return
**	the CRC value. There are other function operators to submit data for processing.
*/
class CRCEngine {
	public:

		// Constructor for CRC engine (it can have an override initial CRC value).
		CRCEngine(int initial=0) : CRC(initial), Index(0) {
			StagingBuffer.Composite = 0;
		};

		// Fetches CRC value.
		int operator() (void) const {return(Value());};

		// Submits one byte sized datum to the CRC accumulator.
		void operator() (char datum);
		void operator() (bool datum);

		/// Submits one word sized datum to the CRC accumulator.
		void operator() (short datum);

		/// Submits one dword sized datum to the CRC accumulator.
		void operator() (int datum);
		void operator() (float datum);

		/// Submits one qword sized datum to the CRC accumulator.
		void operator() (double datum);

		/// Submits zero terminated buffer to the CRC accumulator.
		void operator() (const char * buffer);

		// Submits an arbitrary buffer to the CRC accumulator.
		int operator() (void const * buffer, int length);

		// Implicit conversion operator so this object appears like a 'long integer'.
		operator int(void) const {return(Value());};

	protected:

		bool Buffer_Needs_Data(void) const {
			return(Index != 0);
		};

		int Value(void) const {
			if (Buffer_Needs_Data()) {
				((CRCEngine *)this)->Add_Padding();
				int composite = StagingBuffer.Composite;
				return(CRC::Memory((unsigned char *)&composite, sizeof(StagingBuffer.Composite), CRC));
			}
			return(CRC);
		};

		void Submit(char datum) {
			if (!Buffer_Needs_Data()) {
				StagingBuffer.Composite = 0;
			}
			StagingBuffer.Buffer[Index++] = datum;
			if (Index == sizeof(int)) {
				int composite = StagingBuffer.Composite;
				CRC = CRC::Memory((unsigned char *)&composite, sizeof(StagingBuffer.Composite), CRC);
				Index = 0;
			}
		}

		void Add_Padding(void) {
			StagingBuffer.Buffer[Index] = Index;
			for (unsigned int i = Index + 1; i < sizeof(int); i++) {
				StagingBuffer.Buffer[i] = StagingBuffer.Buffer[0];
			}
		}

		/*
		**	Current accumulator of the CRC value. This value doesn't take into
		**	consideration any pending data in the staging buffer.
		*/
		int CRC;

		/*
		**	This is the sub index into the staging buffer used to keep track of
		**	partial data blocks as they are submitted to the CRC engine.
		*/
		int Index;

		/*
		**	This is the buffer that holds the incoming partial data. When the buffer
		**	is filled, the value is transformed into the CRC and the buffer is flushed
		**	in preparation for additional data.
		*/
		union {
			int Composite;
			char Buffer[sizeof(int)];
		} StagingBuffer;
};
