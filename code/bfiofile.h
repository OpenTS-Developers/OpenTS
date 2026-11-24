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

/* $Header: /CounterStrike/BFIOFILE.H 1     3/03/97 10:24a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Westwood Library                                             *
 *                                                                                             *
 *                    File Name : BFIOFILE.H                                                   *
 *                                                                                             *
 *                   Programmer : David R. Dettmer                                             *
 *                                                                                             *
 *                   Start Date : November 10, 1995                                            *
 *                                                                                             *
 *                  Last Update : November 10, 1995  [DRD]                                     *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "rawfile.h"

/*
**	This derivation of the raw file class handles buffering the input/output in order to
**	achieve greater speed. The buffering is not active by default. It must be activated
**	by setting the appropriate buffer through the Cache() function.
*/
class BufferIOFileClass : public RawFileClass
{
		typedef RawFileClass BASECLASS;

	public:

		BufferIOFileClass(char const *filename);
		BufferIOFileClass(void);
		virtual ~BufferIOFileClass(void) override;

		bool Cache( int size=0, void *ptr=NULL );
		void Free( void );
		bool Commit( void );
		virtual char const * Set_Name(char const *filename) override;
		virtual bool Is_Available(int forced=false) override;
		virtual bool Is_Open(void) const override;
		virtual int Open(char const *filename, int rights=READ) override;
		virtual int Open(int rights=READ) override;
		virtual int Read(void *buffer, int size) override;
		virtual int Seek(int pos, int dir=SEEK_CUR) override;
		virtual int Size(void) override;
		virtual int Write(void const *buffer, int size) override;
		virtual void Close(void) override;
		virtual bool Flush(void) override;

		enum {MINIMUM_BUFFER_SIZE=1024};

	private:

		bool IsAllocated;
		bool IsOpen;
		bool IsDiskOpen;
		bool IsCached;
		bool IsChanged;
		bool UseBuffer;

		int BufferRights;

		void *Buffer;

		int BufferSize;
		int BufferPos;
		int BufferFilePos;
		int BufferChangeBeg;
		int BufferChangeEnd;
		int FileSize;
		int FilePos;
		int TrueFileStart;
};
