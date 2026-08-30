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

/* $Header: /CounterStrike/CONNECT.H 1     3/03/97 10:24a Joe_bostic $ */
/***************************************************************************
 **   C O N F I D E N T I A L --- W E S T W O O D    S T U D I O S        **
 ***************************************************************************
 *                                                                         *
 *                 Project Name : Command & Conquer                        *
 *                                                                         *
 *                    File Name : CONNECT.H                                *
 *                                                                         *
 *                   Programmer : Bill Randolph                            *
 *                                                                         *
 *                   Start Date : December 19, 1994                        *
 *                                                                         *
 *                  Last Update : April 1, 1995   [BR]                     *
 *                                                                         *
 *-------------------------------------------------------------------------*
 *                                                                         *
 * DESCRIPTION:                                                            *
 * This class represents a single "connection" with another system.  It's  *
 * a pure virtual base class that acts as a framework for other classes.   *
 *                                                                         *
 * This class contains a CommBufferClass member, which stores received     *
 * & transmitted packets.  The ConnectionClass has virtual functions to    *
 * handle adding packets to the queue, reading them from the queue,        *
 * a Send routine for actually sending data, and a Receive_Packet function *
 * which is used to tell the connection that a new packet has come in.     *
 *                                                                         *
 * The virtual Service routines handle all ACK & Retry logic for           *
 * communicating between this system & another.  Thus, any class derived   *
 * from this class may overload the basic ACK/Retry logic.                 *
 *                                                                         *
 * THE PACKET HEADER:                                                      *
 * The Connection Classes prefix every packet sent with a header that's    *
 * local to this class.  The header contains a "Magic Number" which should *
 * be unique for each product, and Packet "Code", which will tell the      *
 * receiving end if this is DATA, or an ACK packet, and a packet ID, which *
 * is a unique numerical ID for this packet (useful for detecting resends).*
 * The header is stored with each packet in the send & receive Queues;     *
 * it's removed before it's passed back to the application, via            *
 * Get_Packet()                                                            *
 *                                                                         *
 * THE CONNECTION MANAGER:                                                 *
 * It is assumed that there will be a "Connection Manager" class which     *
 * will handle parsing incoming packets; it will then tell the connection  *
 * that new packets have come in, and the connection will process them in  *
 * whatever way it needs to for its protocol (check for resends, handle    *
 * ACK packets, etc).  The job of the connection manager is to parse       *
 * incoming packets & distribute them to the connections that need to      *
 * store them (for multi-connection protocols).                            *
 *                                                                         *
 * NOTES ON ACK/RETRY:                                                     *
 * This class provides a "non-sequenced" ACK/Retry approach to packet      *
 * transmission.  It sends out as many packets as are in the queue, whose  *
 * resend delta times have expired; and it ACK's any packets its received  *
 * who haven't been ACK'd yet.  Thus, order of delivery is NOT guaranteed; *
 * but, the performance is better than a "sequenced" approach.  Also, the  *
 * Packet ID scheme (see below) ensures that the application will read     *
 * the packets in the proper order.  Thus, this class guarantees delivery  *
 * and order of deliver.                                                   *
 *                                                                         *
 * Each packet has a unique numerical ID; the ID is set to a count of the  *
 * number of packets sent.  Different count values are provided, for both  *
 * DATA_ACK & DATA_NOACK packets.  This ensures that the counter can be    *
 * used to detect resends of DATA_ACK packets; the counters for DATA_NOACK *
 * packets aren't currently used.  Other counters keep track of the        *
 * last-sequentially-received packet ID (for DATA_ACK packets), so we      *
 * can check for resends & missed packets, and the last-sequentially-read  *
 * packet ID, so we can ensure the app reads the packets in order.         *
 *                                                                         *
 * If the protocol being used already guarantees delivery of packets,      *
 * no ACK is required for the packets.  In this case, the connection       *
 * class for this protocol can overload the Service routine to avoid       *
 * sending ACK packets, or the Connection Manager can just mark the        *
 * packet as ACK'd when it adds it to the Receive Queue for the connection.*
 *                                                                         *
 * Derived classes must provide:                                           *
 * - Init: Initialization of any hardware-specific values.                 *
 * - Send: a hardware-dependent send routine.                              *
 *                                                                         *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#pragma once

/*
********************************* Includes **********************************
*/
#include "combuf.h"
#include "netadmit.h"

/*
********************************** Defines **********************************
*/
#define CONN_DEBUG	0
#pragma pack(push,1)
/*---------------------------------------------------------------------------
This structure is the header prefixed to any packet sent by the application.
MagicNumber:	This is a number unique to the application; it's up to the
					Receive_Packet routine to check this value, to be sure we're
					not getting data from some other product.  This value should
					be unique for each application.
Code:				This will be one of the below-defined codes.
PacketID:		This is a unique numerical ID for this packet.  The Connection
					sets this ID on all packets sent out.
---------------------------------------------------------------------------*/
struct CommHeaderType {
	std::uint16_t MagicNumber;
	std::uint8_t Code;
	std::uint32_t PacketID;
};
#pragma pack(pop)

/*
***************************** Class Declaration *****************************
*/
class ConnectionClass
{
	/*
	---------------------------- Public Interface ----------------------------
	*/
	public:
		/*.....................................................................
		These are the possible values for the Code field of the CommHeaderType:
		.....................................................................*/
		enum ConnectionEnum {
			PACKET_DATA_ACK = static_cast<int>(NetPacketCode::DATA_ACK),       // this is a data packet requiring an ACK
			PACKET_DATA_NOACK = static_cast<int>(NetPacketCode::DATA_NOACK),   // this is a data packet not requiring an ACK
			PACKET_ACK = static_cast<int>(NetPacketCode::ACK),                 // this is an ACK for a packet
			PACKET_COUNT = static_cast<int>(NetPacketCode::COUNT)              // for computational purposes
		};

		/*.....................................................................
		Constructor/destructor.
		.....................................................................*/
		ConnectionClass (int numsend, int numrecieve, int maxlen,
			unsigned short magicnum, unsigned int retry_delta,
			unsigned int max_retries, unsigned int timeout, int extralen = 0);
		virtual ~ConnectionClass (void);

		/*.....................................................................
		Initialization.
		.....................................................................*/
		virtual void Init (void);

		/*.....................................................................
		Send/Receive routines.
		.....................................................................*/
		virtual int Send_Packet (void * buf, int buflen, int ack_req);
		virtual int Receive_Packet (void * buf, int buflen);
		virtual int Get_Packet (void * buf, int capacity, int * buflen);

		/*.....................................................................
		The main polling routine for the connection.  Should be called as often
		as possible.
		.....................................................................*/
		virtual int Service (void);

		virtual int Discard_Undeliverable_Packets(void);

		/*.....................................................................
		This routine is used by the retry logic; returns the current time in
		60ths of a second.
		.....................................................................*/
		static unsigned int Time (void);

		/*.....................................................................
		Utility routines.
		.....................................................................*/
		unsigned short Magic_Num (void) { return(MagicNum); }
		unsigned int Retry_Delta (void) { return(RetryDelta); }
		void Set_Retry_Delta (unsigned int delta) { RetryDelta = delta;}
		unsigned int Max_Retries (void) { return(MaxRetries); }
		void Set_Max_Retries (unsigned int retries) { MaxRetries = retries;}
		unsigned int Time_Out (void) { return(Timeout); }
		void Set_TimeOut (unsigned int t) { Timeout = t;}
		unsigned int Max_Packet_Len (void) { return(MaxPacketLen); }
		static const char * Command_Name(int command);

		int Num_Resends(void) const { return(NumResends); }
		int Num_Lost(void) const { return(NumLost); }
		int Percent_Lost(void) const { return(PercentLost); }
		int Missed_Overall(void) const { return(MissedOverall); }
		int Missed_Magic(void) const { return(MissedMagic); }

		enum PacketDropReasonType {
			CONNECTION_DROP_SHORT_HEADER,
			CONNECTION_DROP_INVALID_CODE,
			CONNECTION_DROP_INVALID_LENGTH,
			CONNECTION_DROP_EMPTY_DATA,
			CONNECTION_DROP_OVERSIZED_DATA,
			CONNECTION_DROP_OUTPUT_TOO_SMALL,
			CONNECTION_DROP_COUNT
		};

		unsigned int Dropped_Packets(PacketDropReasonType reason) const;

		/*.....................................................................
		The packet "queue"; this non-sequenced version isn't really much of
		a queue, but more of a repository.
		.....................................................................*/
		CommBufferClass *Queue;

	/*
	-------------------------- Protected Interface ---------------------------
	*/
	protected:
		/*.....................................................................
		Routines to service the Send & Receive queues.
		.....................................................................*/
		virtual int Service_Send_Queue(void);
		virtual int Service_Receive_Queue(void);

		/*.....................................................................
		This routine actually performs a hardware-dependent data send.  It's
		pure virtual, so it must be defined by a derived class.  The routine
		is protected; it's only called by the ACK/Retry logic, not the
		application.
		.....................................................................*/
		virtual int Send(char *buf, int buflen, void *extrabuf,
			int extralen) = 0;
		/// <summary>Returns whether this channel represents one peer with adaptive link timing.</summary>
		virtual bool Adaptive_Timing_Enabled(void) const {return(true);}
		void Record_Packet_Drop(PacketDropReasonType reason);
		/// <summary>Maps a shared admission failure to this connection's stable counter.</summary>
		void Record_Admission_Drop(NetAdmissionError error, unsigned char code);

		/*
		 * This is the number of times a packet had to be transmitted again because no ACK
		 * came back for it within the retry delay.
		 */
		int NumResends;

		/*
		 * These record how many of the packets sent without an ACK request never reached
		 * us, judged by comparing the newest packet ID against the number actually
		 * received -- as a raw count and as a percentage of everything sent so far.
		 */
		int NumLost;
		int PercentLost;

		/*
		 * This is the number of incoming packets thrown away because the receive queue was
		 * full. Only packets that require no ACK are counted; the sender never resends those.
		 */
		int MissedOverall;

		/*
		 * This is the number of incoming packets rejected because their magic number did not
		 * match this connection's, meaning they came from some other product.
		 */
		int MissedMagic;
		unsigned int DroppedPackets[CONNECTION_DROP_COUNT];

		/*.....................................................................
		This is the maximum packet length, including our own internal header.
		.....................................................................*/
		int MaxPacketLen;

		/*.....................................................................
		Packet staging area; this is where the CommHeaderType gets tacked onto
		the application's packet before it's sent.
		.....................................................................*/
		char *PacketBuf;

		/*.....................................................................
		This is the magic number assigned to this connection.  It is the first
		few bytes of any transmission.
		.....................................................................*/
		unsigned short MagicNum;

		/*.....................................................................
		This value determines the time delay before a packet is re-sent.
		.....................................................................*/
		unsigned int RetryDelta;

		/*.....................................................................
		This is the maximum number of retries allowed for a packet; if this
		value is exceeded, the connection is probably broken.
		.....................................................................*/
		unsigned int MaxRetries;

		/*.....................................................................
		This is the total timeout for this connection; if this time is exceeded
		on a packet, the connection is probably broken.
		.....................................................................*/
		unsigned int Timeout;

		/*.....................................................................
		Running totals of # of packets we send & receive which require an ACK,
		and those that don't.
		.....................................................................*/
		unsigned int NumRecNoAck;
		unsigned int NumRecAck;
		unsigned int NumSendNoAck;
		unsigned int NumSendAck;

		/*.....................................................................
		This is the ID of the last consecutively-received packet; anything older
		than this, we know is a resend.  Anything newer than this MUST be lying
		around in the Queue for us to detect it as a resend.
		.....................................................................*/
		unsigned int LastSeqID;

		/*.....................................................................
		This is the ID of the PACKET_DATA_ACK packet we read last; it ensures
		that the application reads that type of packet in order.
		.....................................................................*/
		unsigned int LastReadID;

		/*.....................................................................
		Names of all packet commands
		.....................................................................*/
		static char const * Commands[PACKET_COUNT];
};


/**************************** end of connect.h *****************************/
