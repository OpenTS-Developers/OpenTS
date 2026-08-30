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

/* $Header: /CounterStrike/CONNMGR.H 1     3/03/97 10:24a Joe_bostic $ */
/***************************************************************************
 **   C O N F I D E N T I A L --- W E S T W O O D    S T U D I O S        **
 ***************************************************************************
 *                                                                         *
 *                 Project Name : Command & Conquer                        *
 *                                                                         *
 *                    File Name : CONNMGR.H                                *
 *                                                                         *
 *                   Programmer : Bill Randolph                            *
 *                                                                         *
 *                   Start Date : December 19, 1994                        *
 *                                                                         *
 *                  Last Update : April 3, 1995   [BR]                     *
 *                                                                         *
 *-------------------------------------------------------------------------*
 *                                                                         *
 * This is the Connection Manager base class.  This is an abstract base    *
 * class that's just a shell for more functional derived classes.          *
 * The main job of the Connection Manager classes is to parse a "pool" of  *
 * incoming packets, which may be from different computers, and distribute *
 * those packets to Connection Classes via their Receive_Packet function.  *
 *                                                                         *
 * This class should be the only access to the network/modem for the       *
 * application, so if the app needs any functions to access the            *
 * connections or the queue's, the derived versions of this class should   *
 * provide them.                                                           *
 *                                                                         *
 * It's up to the derived class to define:                                 *
 * - Service:     polling routine; should Service each connection          *
 * - Init:        initialization; should perform hardware-dependent        *
 *                initialization, then Init each connection; this function *
 *                isn't defined in this class, since the parameters will   *
 *                be highly protocol-dependent)                            *
 * - Send_Message:sends a packet across the connection (this function      *
 *                isn't defined in this class, since the parameters will   *
 *                be highly protocol-dependent)                            *
 * - Get_Message: gets a message from the connection (this function        *
 *                isn't defined in this class, since the parameters will   *
 *                be highly protocol-dependent)                            *
 *                                                                         *
 * If the derived class supports multiple connections, it should provide   *
 * functions for creating the connections, associating them with a name    *
 * or ID or both, destroying them, and sending data through all or any     *
 * connection.                                                             *
 *                                                                         *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#pragma once

#include "nettime.h"

#include <optional>


/*
***************************** Class Declaration *****************************
*/
class ConnManClass
{
	/*
	---------------------------- Public Interface ----------------------------
	*/
	public:
		/*.....................................................................
		Various useful enums:
		.....................................................................*/
		enum IPXConnTag {
			CONNECTION_NONE = -1			// value of an invalid connection ID
		};

		/*.....................................................................
		Constructor/Destructor.  These currently do nothing.
		.....................................................................*/
		ConnManClass (void) {};
		virtual ~ConnManClass (void) {};

		/*.....................................................................
		The Service routine:
		- Parses incoming packets, and adds them to the Receive Queue for the
		  Connection Class(s) for this protocol
		- Invokes each connection's Service routine; returns an error if the
		  connection's Service routine indicates an error.
		.....................................................................*/
		virtual int Service (void) = 0;

		/*.....................................................................
		Sending & receiving data
		.....................................................................*/
		virtual int Send_Private_Message (void *buf, int buflen, int ack_req = 1, int conn_id = CONNECTION_NONE) = 0;
		virtual int Get_Private_Message (void *buf, int capacity, int *buflen, int *conn_id) = 0;

		/*.....................................................................
		Connection management
		.....................................................................*/
		virtual int Num_Connections(void) = 0;
		virtual int Connection_ID(int index) = 0;
		virtual int Connection_Index(int id) = 0;

		/*.....................................................................
		Queue utility routines
		.....................................................................*/
		virtual int Global_Num_Send(void) = 0;
		virtual int Global_Num_Receive(void) = 0;
		virtual int Private_Num_Send(int id = CONNECTION_NONE) = 0;
		virtual int Private_Num_Receive(int id = CONNECTION_NONE) = 0;

		/*.....................................................................
		Timing management
		.....................................................................*/
		virtual void Reset_Response_Time(bool zero) = 0;
		virtual unsigned int Response_Time(void) = 0;
		virtual std::optional<NetTiming::Milliseconds> Worst_Local_Round_Trip_MS(void) const = 0;
		virtual void Set_Timing (unsigned int retrydelta, unsigned int maxretries, unsigned int timeout, bool set_external = true) = 0;
		virtual void Set_External_Timing (unsigned int retrydelta, unsigned int maxretries, unsigned int timeout) = 0;

		/*.....................................................................
		Debugging
		.....................................................................*/
		virtual void Configure_Debug(int index, int type_offset, int type_size,
			char **names, int namestart, int namecount) = 0;
#ifdef _DEBUG
		virtual void Mono_Debug_Print(int index, int refresh) = 0;
#endif

	/*
	--------------------------- Private Interface ----------------------------
	*/
	private:
		/*.....................................................................
		This abstract class contains no data members; but a derived class
		will contain:
		- An instance of one or more derived Connection Classes
		- A buffer to store incoming packets
		.....................................................................*/
};
