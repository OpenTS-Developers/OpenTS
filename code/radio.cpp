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

/* $Header: /CounterStrike/RADIO.CPP 1     3/03/97 10:25a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : RADIO.CPP                                                    *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : September 10, 1993                                           *
 *                                                                                             *
 *                  Last Update : June 5, 1996 [JLB]                                           *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   RadioClass::Debug_Dump -- Displays the current status of the radio to the mono monitor.   *
 *   RadioClass::Limbo -- When limboing a unit will always break radio contact.                *
 *   RadioClass::Receive_Message -- Handles receipt of a radio message.                        *
 *   RadioClass::Transmit_Message -- Transmit message from one object to another.              *
 *   RadioClass::Transmit_Message -- Transmits a message to the object specified.              *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "radio.h"

#include "_rtti.h"
#include "house.h"
#include "mono.h"
#include "savestream.h"
#include "swizzle.h"
#include "techno.h"

#ifdef _DEBUG
/*
**	These are the text representations of the radio messages that can be transmitted.
*/
char const * RadioClass::Messages[RADIO_COUNT] = {
	"static (no message)",
	"Roger.",
	"Come in.",
	"Over and out.",
	"Requesting transport.",
	"Attach to transport.",
	"I've got a delivery for you.",
	"I'm performing load/unload maneuver. Be careful.",
	"I'm clear.",
	"You are clear to unload. Driving away now.",
	"Am unable to comply.",
	"I'm starting construction now... act busy.",
	"I've finished construction. You are free.",
	"We bumped, redraw yourself please.",
	"I'm trying to load up now.",
	"May I become a passenger?",
	"Are you ready to receive shipment?",
	"Are you trying to become a passenger?",
	"Move to location X.",
	"Do you need to move?",
	"All right already. Now what?",
	"I'm a passenger now.",
	"Backup into refinery now.",
	"Run away!",
	"Tether established.",
	"Tether broken.",
	"Repair one step.",
	"Are you prepared to fight?",
	"Attack this target please.",
	"Reload one step.",
	"Circumstances prevent success.",
	"All done with the request.",
	"Do you need service depot work?",
	"Are you sitting on service depot?"
};
#endif


/// <summary>
/// Creates a radio set that is not in contact with anyone.
/// The message history is cleared as well, so the first message this object receives will
/// always be treated as a new one rather than a repeat.
/// </summary>
RadioClass::RadioClass(void) :
	BASECLASS(),
	Radio(NULL)
{
	for (int i = 0; i < 3; i++) {
		Old[i] = RADIO_STATIC;
	}
};


#ifdef _DEBUG
/***********************************************************************************************
 * RadioClass::Debug_Dump -- Displays the current status of the radio to the mono monitor.     *
 *                                                                                             *
 *    This displays the radio connection value to the monochrome monitor.                      *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/02/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void RadioClass::Debug_Dump(MonoClass * mono) const
{
	mono->Set_Cursor(29, 7);mono->Printf("0-%-47s", Messages[Old[0]]);
	mono->Set_Cursor(29, 8);mono->Printf("1-%-47s", Messages[Old[1]]);
	mono->Set_Cursor(29, 9);mono->Printf("2-%-47s", Messages[Old[2]]);
	if (Radio != NULL) {
		mono->Set_Cursor(20, 7);mono->Printf("%08X", Radio);
	}
	BASECLASS::Debug_Dump(mono);
}
#endif


/***********************************************************************************************
 * RadioClass::Receive_Message -- Handles receipt of a radio message.                          *
 *                                                                                             *
 *    This is the base version of what should happen when a radio message is received. It      *
 *    turns the radio off when the "OVER_OUT" message is received. All other messages are      *
 *    merely acknowledged with a "ROGER".                                                      *
 *                                                                                             *
 * INPUT:   from     -- The object that is initiating this radio message (always valid).       *
 *                                                                                             *
 *          message  -- The radio message received.                                            *
 *                                                                                             *
 *          param    -- Reference to optional value that might be used to return more          *
 *                      information than can be conveyed in the simple radio response          *
 *                      messages.                                                              *
 *                                                                                             *
 * OUTPUT:  Returns with the response radio message.                                           *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/28/1994 JLB : Created.                                                                 *
 *   09/24/1994 JLB : Streamlined to be only a communications carrier.                         *
 *   05/22/1995 JLB : Recognized who is sending the message                                    *
 *   06/05/1996 JLB : Radio message history tracking.                                          *
 *=============================================================================================*/
RadioMessageType RadioClass::Receive_Message(RadioClass * from, RadioMessageType message, intptr_t & param)
{
	/*
	**	Keep a record of the last message received by this radio.
	*/
	if (message != Old[0]) {
		Old[2] = Old[1];
		Old[1] = Old[0];
		Old[0] = message;
	}

	/*
	**	When this message is received, it means that the other object
	**	has already turned its radio off. Turn this radio off as well.
	**	This only applies if the message is coming from the object that
	**	has an established conversation with this object.
	*/
	if (from == Radio && message == RADIO_OVER_OUT) {
		BASECLASS::Receive_Message(from, message, param);
		Radio_Off();
		return(RADIO_ROGER);
	}

	/*
	**	The "hello" message is an attempt to establish contact. If this radio
	**	is already in an established conversation with another object, then
	**	return with "negative". If all is well, return with "roger".
	*/
	if (message == RADIO_HELLO && Strength) {
		if (Radio == NULL || Radio == from) {
			if (from != NULL && ((TechnoClass *)from)->House->Is_Ally(this) && Is_Techno() && ((TechnoClass *)this)->House->Is_Ally(from)) {
				Radio = from;
				return(RADIO_ROGER);
			}
		}
		return(RADIO_NEGATIVE);
	}

	return(BASECLASS::Receive_Message(from, message, param));
}


/***********************************************************************************************
 * RadioClass::Transmit_Message -- Transmit message from one object to another.                *
 *                                                                                             *
 *    This routine is used to transmit a radio message from this object to another. Most       *
 *    inter object coordination is handled through this mechanism.                             *
 *                                                                                             *
 * INPUT:   to       -- Pointer to the object that will receive the radio message.             *
 *                                                                                             *
 *          message  -- The message itself (see RadioType).                                    *
 *                                                                                             *
 *          param    -- Optional reference to parameter that might be used to pass or          *
 *                      receive additional information.                                        *
 *                                                                                             *
 * OUTPUT:  Returns with the response radio message from the receiving object.                 *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/22/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
RadioMessageType RadioClass::Transmit_Message(RadioMessageType message, intptr_t & param, RadioClass * to)
{
	if (to == NULL) {
		to = (RadioClass *)Contact_With_Whom();
	}

	/*
	**	If there is no target for the radio message, then always return static.
	*/
	if (to == NULL) return(RADIO_STATIC);

	/*
	**	Handle some special case processing that occurs when certain messages
	**	are transmitted.
	*/
	if (to == Radio && message == RADIO_OVER_OUT) {
		Radio = NULL;
	}

	/*
	**	If this object is not in radio contact but the message
	**	indicates that radio contact should be established, then
	**	try to do so. If the other party agrees then contact
	**	is established.
	*/
	if (message == RADIO_HELLO) {
		Transmit_Message(RADIO_OVER_OUT);
		if (to->Receive_Message(Dynamic_Cast<TechnoClass *>(this), message, param) == RADIO_ROGER) {
			Radio = to;
			return(RADIO_ROGER);
		}
		return(RADIO_NEGATIVE);
	}

	return(to->Receive_Message(Dynamic_Cast<TechnoClass *>(this), message, param));
}


/***********************************************************************************************
 * RadioClass::Limbo -- When limboing a unit will always break radio contact.                  *
 *                                                                                             *
 *    This routine will break radio contact as the object is entering limbo state.             *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Was the object successfully limboed?                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/25/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
bool RadioClass::Limbo(void)
{
	if (!IsInLimbo) {
		Transmit_Message(RADIO_OVER_OUT);
	}
	return(BASECLASS::Limbo());
}


/***********************************************************************************************
 * RadioClass::Transmit_Message -- Transmits a message to the object specified.                *
 *                                                                                             *
 *    This routine will transmit the specified message to the object. This routine differs     *
 *    from the normal Transmit_Message in that the LParam value is "faked" into the            *
 *    parameter list. It is presumed that the message sent with this function does not         *
 *    require the LParam.                                                                      *
 *                                                                                             *
 * INPUT:   message  -- The message to transmit.                                               *
 *                                                                                             *
 *          to       -- The requested receiver of this message.                                *
 *                                                                                             *
 * OUTPUT:  Returns with the radio response from the receiver.                                 *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/21/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
RadioMessageType RadioClass::Transmit_Message(RadioMessageType message, RadioClass * to)
{
	return(Transmit_Message(message, LParam, to));
}


/// <summary>
/// Removes any reference to the specified object.
/// This routine is called when an object is about to be destroyed or removed from the
/// game. Any radio contact with the doomed object must be broken here, since a stale
/// contact would leave this object talking to freed memory.
/// </summary>
/// <param name="target">Pointer to the object that is going away.</param>
/// <param name="all">Should even a casual reference to the target be severed?</param>
void RadioClass::Detach(AbstractClass const * target, bool all)
{
	BASECLASS::Detach(target, all);
	if (Radio == target && all) {
		Radio = NULL;
	}
}


/// <summary>
/// Adds this radio set to the game state checksum.
/// This routine is used by the multiplayer synchronization checker. The identity of the
/// object this radio is tuned to contributes to the checksum, so a desynchronized radio
/// contact will be caught.
/// </summary>
void RadioClass::Compute_CRC(CRCEngine & crc) const
{
	BASECLASS::Compute_CRC(crc);
	if (Radio != NULL) {
		crc(Radio->Fetch_ID());
	}
	if (Radio != NULL) {
		crc((RTTIType)Radio->RTTI);
	}
}


/// <summary>
/// Lists the members this radio set carries.
/// </summary>
/// <param name="stream">The stream carrying the members.</param>
void RadioClass::Serialize(SaveStreamClass & stream)
{
	BASECLASS::Serialize(stream);

	stream.Serialize(Old);
	stream.Serialize(Radio);
}
