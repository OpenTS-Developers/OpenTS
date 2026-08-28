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

/* $Header: /CounterStrike/RADIO.H 1     3/03/97 10:25a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : RADIO.H                                                      *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : April 23, 1994                                               *
 *                                                                                             *
 *                  Last Update : April 23, 1994   [JLB]                                       *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "globals.h"
#include "mission.h"

class ObjectClass;
class TechnoClass;


/****************************************************************************
**	Radio contact is controlled by this class. It handles the mundane chore
**	of keeping the radio contact alive as well as broadcasting messages
**	to the receiving radio. Radio contact is primarily used when one object
**	is in "command" of another.
*/
class RadioClass : public MissionClass
{
		typedef MissionClass BASECLASS;

	private:

		/*
		**	This is a record of the last message received by this receiver.
		*/
		RadioMessageType Old[3];

		/*
		**	This is the object that radio communication has been established
		**	with. Although is is only a one-way reference, it is required that
		**	the receiving radio is also tuned to the object that contains this
		**	radio set.
		*/
		RadioClass * Radio;

#ifdef _DEBUG
		/*
		**	This is a text representation of all the possible radio messages. This
		**	text is used for monochrome debug printing.
		*/
		static char const * Messages[RADIO_COUNT];
#endif

	public:

		/*---------------------------------------------------------------------
		**	Constructors, Destructors, and overloaded operators.
		*/
		RadioClass(void);
		virtual ~RadioClass(void) override {/*Radio=0;*/};


		virtual void Serialize(SaveStreamClass & stream) override;

		/*---------------------------------------------------------------------
		**	Member function prototypes.
		*/
		bool In_Radio_Contact(void) const {return(Radio != 0);};
		void Radio_Off(void) {Radio = 0;};
		TechnoClass * Contact_With_Whom(void) const {return((TechnoClass *)Radio);};

		// Inherited from base class(es).
		virtual void Detach(AbstractClass const * target, bool all = true) override;
		virtual void Compute_CRC(CRCEngine &) const override;
		virtual RadioMessageType Receive_Message(RadioClass * from, RadioMessageType message, intptr_t & param) override;
		virtual RadioMessageType Transmit_Message(RadioMessageType message, intptr_t & param=LParam, RadioClass * to=NULL);
		virtual RadioMessageType Transmit_Message(RadioMessageType message, RadioClass * to);
#ifdef _DEBUG
		virtual void Debug_Dump(MonoClass *mono) const override;
#endif
		virtual bool Limbo(void) override;
};
