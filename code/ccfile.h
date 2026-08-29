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

/* $Header: /CounterStrike/CCFILE.H 1     3/03/97 10:24a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : CCFILE.H                                                     *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : October 17, 1994                                             *
 *                                                                                             *
 *                  Last Update : October 17, 1994   [JLB]                                     *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "buff.h"
#include "cdfile.h"

#include <climits>


/*
**	This derived class for file access knows about mixfiles (packed files). It can handle opening
**	a file that is embedded within a mixfile. This is true if the mixfile is cached or resides on
**	disk. It is functionally similar to pakfiles, except much faster and less RAM intensive.
*/
class CCFileClass : public CDFileClass
{
		typedef CDFileClass BASECLASS;

	public:
		CCFileClass(char const * filename);
		CCFileClass(void);
		virtual ~CCFileClass(void) override {Position = 0;};

		// Delete should be overloaded here as well. Don't allow deletes of mixfiles.

		bool Is_Resident(void) const {return(Data.Get_Buffer() != NULL);}
		virtual bool Is_Available(int forced=false) override;
		virtual bool Is_Open(void) const override;
		virtual int Open(char const * filename, int rights=READ) override;
		virtual int Open(int rights=READ) override;
		virtual int Read(void * buffer, int size) override;
		virtual int Seek(int pos, int dir=SEEK_CUR) override;
		virtual int Size(void) override;
		virtual int Write(void const * buffer, int size) override;
		virtual void Close(void) override;
		virtual unsigned int Get_Date_Time(void) override;
		virtual bool Set_Date_Time(unsigned int datetime) override;
		virtual void Error(int error, int canretry = false, char const * filename=NULL) override;

	private:

		/*
		**	This indicates the file is actually part of a resident image of the mixfile
		**	itself. In this case, the embedded file handle is invalid. All file access actually
		**	gets routed through the cached version of the file. This is a pointer to the start
		**	of the RAM image of the file.
		*/
		::Buffer Data;
//		void * Pointer;

		/*
		**	This is the size of the file if it was embedded in a mixfile. The size must be manually
		**	kept track of because the DOS file size is invalid.
		*/
//		int Length;

		/*
		**	This is the current seek position of the file. It is duplicated here if the file is
		**	part of a mixfile since the DOS seek position is not accurate. This value will
		**	range from zero to the size of the file in bytes.
		*/
		int Position;

		// Force these to never be invoked.
		CCFileClass const & operator = (CCFileClass const & c);
		CCFileClass (CCFileClass const & );
};
