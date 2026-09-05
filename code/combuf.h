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

/* $Header: /CounterStrike/COMBUF.H 1     3/03/97 10:24a Joe_bostic $ */
/***************************************************************************
 **   C O N F I D E N T I A L --- W E S T W O O D    S T U D I O S        **
 ***************************************************************************
 *                                                                         *
 *                 Project Name : Command & Conquer                        *
 *                                                                         *
 *                    File Name : COMBUF.H                                 *
 *                                                                         *
 *                   Programmer : Bill Randolph                            *
 *                                                                         *
 *                   Start Date : December 19, 1994                        *
 *                                                                         *
 *                  Last Update : April 1, 1995   [BR]                     *
 *                                                                         *
 *-------------------------------------------------------------------------*
 *                                                                         *
 * This class's job is to store outgoing messages & incoming messages,     *
 * and serves as a storage area for various flags for ACK & Retry logic.   *
 *                                                                         *
 * This class stores buffers in a non-sequenced order; it allows freeing   *
 * any entry, so the buffers can be kept clear, even if packets come in    *
 * out of order.                                                           *
 *                                                                         *
 * The class also contains routines to maintain a cumulative response time *
 * for this queue.  It's up to the caller to call Add_Delay() whenever     *
 * it detects that an outgoing message has been ACK'd; this class adds     *
 * that delay into a computed average delay over the last few message      *
 * delays.                                                                 *
 *                                                                         *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "win.h"

/*
********************************** Defines **********************************
*/
/*---------------------------------------------------------------------------
This is one output queue entry
---------------------------------------------------------------------------*/
struct SendQueueType {
	unsigned int IsActive	: 1;    // 1 = this entry is ready to be processed
	unsigned int IsACK		: 1;    // 1 = ACK received for this packet
	unsigned int IsUndeliverable : 1;  /// 1 = gave up on it (retries or timeout)
	unsigned int FirstTime;        // time this packet was first sent
	unsigned int LastTime;         // time this packet was last sent
	unsigned int FirstTimeMilliseconds = 0; // monotonic time of the first transmission
	unsigned int LastTimeMilliseconds = 0;  // monotonic time of the latest transmission
	unsigned int RetransmitTimeoutMilliseconds = 0; // base RTO captured for this packet
	unsigned int SendCount;        // # of times this packet has been sent
	int BufLen;                     // size of the packet stored in this entry
	char *Buffer;                   // the data packet
	int ExtraLen;                   // size of extra data
	char *ExtraBuffer;              // extra data buffer
};

/*---------------------------------------------------------------------------
This is one input queue entry
---------------------------------------------------------------------------*/
struct ReceiveQueueType {
	unsigned int IsActive	: 1;    // 1 = this entry is ready to be processed
	unsigned int IsRead		: 1;    // 1 = caller has read this entry
	unsigned int IsACK		: 1;    // 1 = ACK sent for this packet
	int BufLen;                     // size of the packet stored in this entry
	char *Buffer;                   // the data packet
	int ExtraLen;                   // size of extra data
	char *ExtraBuffer;              // extra data buffer
};

/*
***************************** Class Declaration *****************************
*/
class CommBufferClass
{
	/*
	---------------------------- Public Interface ----------------------------
	*/
	public:
		/*
		....................... Constructor/Destructor ........................
		*/
		CommBufferClass(int numsend, int numrecieve, int maxlen,
			int extralen = 0);
		virtual ~CommBufferClass(void);
		void Init(void);
		void Init_Send_Queue(void);

		/*
		......................... Send Queue routines .........................
		*/
		int Queue_Send(void *buf, int buflen, void *extrabuf = NULL,
			int extralen = 0);
		int UnQueue_Send(void *buf, int *buflen, int index,
			void *extrabuf = NULL, int *extralen = NULL);
		int Num_Send(void) {return(SendCount);}     // # entries in queue
		int Max_Send(void) { return(MaxSend);}      // max # send queue entries
		SendQueueType * Get_Send(int index);        // random access to queue
		unsigned int Send_Total(void) {return(SendTotal);}

		/*
		....................... Receive Queue routines ........................
		*/
		int Queue_Receive(void *buf, int buflen, void *extrabuf = NULL,
			int extralen = 0);
		int UnQueue_Receive(void *buf, int *buflen, int index,
			void *extrabuf = NULL, int *extralen = NULL);
		int Num_Receive(void) {return(ReceiveCount);}   // # entries in queue
		int Max_Receive(void) { return(MaxReceive); }   // max # recv queue entries
		ReceiveQueueType * Get_Receive(int index);      // random access to queue
		unsigned int Receive_Total(void) {return(ReceiveTotal);}

		/*
		....................... Response time routines ........................
		*/
		void Add_Delay(unsigned int delay);	// accumulates response time
		unsigned int Avg_Response_Time(void);	// gets mean response time
		unsigned int Max_Response_Time(void);	// gets max response time
		void Reset_Response_Time(bool zero);	// resets computations

		/*
		........................ Debug output routines ........................
		*/
		void Configure_Debug(int type_offset, int type_size, char **names,
			int namestart, int namecount);
		void Mono_Debug_Print(int refresh = 0);
		void Mono_Debug_Print2(int refresh = 0);

	/*
	--------------------------- Private Interface ----------------------------
	*/
	private:
		/*
		.......................... Limiting variables .........................
		*/
		int MaxSend;        // max # send queue entries
		int MaxReceive;     // max # receive queue entries
		int MaxPacketSize;  // max size of a packet, in bytes
		int MaxExtraSize;   // max size of extra bytes

		/*
		....................... Response time variables .......................
		*/
		unsigned int DelaySum;				// sum of last 4 delay times
		unsigned int NumDelay;				// current # delay times summed
		unsigned int MeanDelay;			// current average delay time
		unsigned int MaxDelay;				// max delay ever for this queue

		/*
		........................ Send Queue variables .........................
		*/
		SendQueueType * SendQueue;			// incoming packets
		int SendCount; 						// # packets in the queue
		unsigned int SendTotal;			// total # added to send queue
		int *SendIndex;						// array of Send entry indices

		/*
		....................... Receive Queue variables .......................
		*/
		ReceiveQueueType * ReceiveQueue;    // outgoing packets
		int ReceiveCount;                   // # packets in the queue
		unsigned int ReceiveTotal;         // total # added to receive queue
		int *ReceiveIndex;                  // array of Receive entry indices

		/*
		......................... Debugging Variables .........................
		*/
		int DebugOffset;    // offset into app's packet for ID
		int DebugSize;      // size of app's ID
		char **DebugNames;  // ptr to array of app-specific names
		int DebugNameStart; // number of 1st ID
		int DebugNameCount; // # of names in array
};


/**************************** end of combuf.h ******************************/
