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

/* $Header: /CounterStrike/CONNECT.CPP 1     3/03/97 10:24a Joe_bostic $ */
/***************************************************************************
 **   C O N F I D E N T I A L --- W E S T W O O D    S T U D I O S        **
 ***************************************************************************
 *                                                                         *
 *                 Project Name : Command & Conquer                        *
 *                                                                         *
 *                    File Name : CONNECT.CPP                              *
 *                                                                         *
 *                   Programmer : Bill Randolph                            *
 *                                                                         *
 *                   Start Date : December 20, 1994                        *
 *                                                                         *
 *                  Last Update : May 31, 1995 [BRR]                       *
 *-------------------------------------------------------------------------*
 * Functions:                                                              *
 *   ConnectionClass::ConnectionClass -- class constructor                 *
 *   ConnectionClass::~ConnectionClass -- class destructor                 *
 *   ConnectionClass::Init -- Initializes connection queue to empty        *
 *   ConnectionClass::Send_Packet -- adds a packet to the send queue       *
 *   ConnectionClass::Receive_Packet -- adds packet to receive queue       *
 *   ConnectionClass::Get_Packet -- gets a packet from receive queue       *
 *   ConnectionClass::Service -- main polling routine; services packets    *
 *   ConnectionClass::Service_Send_Queue -- services the send queue        *
 *   ConnectionClass::Service_Receive_Queue -- services receive queue      *
 *   ConnectionClass::Time -- gets current time                            *
 *   ConnectionClass::Command_Name -- returns name for a packet command    *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "connect.h"

#include "_timer.h"
#include "dbgprint.h"

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <limits>
#include <sys\timeb.h>


/*
********************************* Globals ***********************************
*/
char const * ConnectionClass::Commands[PACKET_COUNT] = {
	"ADATA",
	"NDATA",
	"ACK"
};

namespace {

/// <summary>Converts engine ticks to milliseconds.</summary>
NetTiming::Milliseconds Ticks_To_Milliseconds(unsigned int ticks)
{
	std::uint64_t const milliseconds = (static_cast<std::uint64_t>(ticks) * 1000 + TIMER_SECOND - 1) / TIMER_SECOND;
	if (milliseconds > std::numeric_limits<NetTiming::Milliseconds>::max()) {
		return(std::numeric_limits<NetTiming::Milliseconds>::max());
	}
	return(static_cast<NetTiming::Milliseconds>(milliseconds));
}


/// <summary>Converts and bounds a legacy connection timeout.</summary>
NetTiming::Milliseconds Legacy_Connection_Timeout(unsigned int ticks)
{
	return(std::clamp(Ticks_To_Milliseconds(ticks), NetTiming::MINIMUM_CONNECTION_TIMEOUT, NetTiming::MAXIMUM_CONNECTION_TIMEOUT));
}

}


/***************************************************************************
 * ConnectionClass::ConnectionClass -- class constructor                   *
 *                                                                         *
 * INPUT:                                                                  *
 *      numsend         desired # of entries for the send queue            *
 *      numreceive      desired # of entries for the receive queue         *
 *      maxlen         max length of an application packet                 *
 *      magicnum         the packet "magic number" for this connection     *
 *      retry_delta      the time to wait between sends                    *
 *      max_retries      the max # of retries allowed for a packet         *
 *                     (-1 means retry forever, based on this parameter)   *
 *      timeout         the max amount of time before we give up on a packet*
 *                     (-1 means retry forever, based on this parameter)   *
 *      extralen         max size of app-specific extra bytes (optional)   *
 *      clock            monotonic millisecond clock (default if NULL)     *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      none.                                                              *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   12/20/1994 BR : Created.                                              *
 *=========================================================================*/
ConnectionClass::ConnectionClass (int numsend, int numreceive,
	int maxlen, unsigned short magicnum, unsigned int retry_delta,
	unsigned int max_retries, unsigned int timeout, int extralen, NetTiming::MillisecondClock const * clock)
{
	/*------------------------------------------------------------------------
	Compute our maximum packet length
	------------------------------------------------------------------------*/
	MaxPacketLen = maxlen + sizeof(CommHeaderType);

	/*------------------------------------------------------------------------
	Assign the magic number
	------------------------------------------------------------------------*/
	MagicNum = magicnum;

	/*------------------------------------------------------------------------
	Initialize the retry time.  This is the time that t2 - t1 must be greater
	than before a retry will occur.
	------------------------------------------------------------------------*/
	RetryDelta = retry_delta;

	/*------------------------------------------------------------------------
	Set the maximum allowable retries.
	------------------------------------------------------------------------*/
	MaxRetries = max_retries;

	/*------------------------------------------------------------------------
	Set the timeout for this connection.
	------------------------------------------------------------------------*/
	Timeout = timeout;
	MillisecondTime = clock != nullptr ? clock : &NetTiming::Default_Clock();

	/*------------------------------------------------------------------------
	Allocate the packet staging buffer.  This will be used to
	------------------------------------------------------------------------*/
	PacketBuf = new char[ MaxPacketLen ];

	/*------------------------------------------------------------------------
	Allocate the packet Queue.  This will store incoming packets (placed there
	by Receive_Packet), and outgoing packets (placed there by Send_Packet).
	It can optionally store "extra" bytes, which are stored along with each
	packet, but aren't transmitted as part of the packet.  If 'extralen'
	is 0, the CommBufferClass ignores this parameter.
	------------------------------------------------------------------------*/
	Queue = new CommBufferClass (numsend, numreceive, MaxPacketLen, extralen);
	memset(DroppedPackets, 0, sizeof(DroppedPackets));

}	/* end of ConnectionClass */


/***************************************************************************
 * ConnectionClass::~ConnectionClass -- class destructor                   *
 *                                                                         *
 * INPUT:                                                                  *
 *      none.                                                              *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      none.                                                              *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   12/20/1994 BR : Created.                                              *
 *=========================================================================*/
ConnectionClass::~ConnectionClass (void)
{
	/*------------------------------------------------------------------------
	Free memory.
	------------------------------------------------------------------------*/
	delete [] PacketBuf;
	delete Queue;

}	/* end of ~ConnectionClass */


/***************************************************************************
 * ConnectionClass::Init -- Initializes connection queue to empty          *
 *                                                                         *
 * INPUT:                                                                  *
 *      none.                                                              *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      none.                                                              *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   12/20/1994 BR : Created.                                              *
 *=========================================================================*/
void ConnectionClass::Init (void)
{
	NumRecNoAck = 0;
	NumRecAck = 0;
	NumSendNoAck = 0;
	NumSendAck = 0;

	NumResends = 0;
	NumLost = 0;
	PercentLost = 0;
	MissedOverall = 0;
	MissedMagic = 0;
	memset(DroppedPackets, 0, sizeof(DroppedPackets));

	LastSeqID = 0xffffffff;
	LastReadID = 0xffffffff;
	RoundTripEstimator.Reset();
	IsBad = false;

	Queue->Init();

}	/* end of Init */


/***************************************************************************
 * ConnectionClass::Send_Packet -- adds a packet to the send queue         *
 *                                                                         *
 * This routine prefixes the given buffer with a CommHeaderType and        *
 * queues the resulting packet into the Send Queue.  (It's actually the    *
 * Service() routine that handles the hardware-dependent Send of the data).*
 * The packet's MagicNumber, Code, and PacketID are set here.              *
 *                                                                         *
 * INPUT:                                                                  *
 *      buf         buffer to send                                         *
 *      buflen      length of buffer                                       *
 *      ack_req      1 = ACK is required for this packet; 0 = isn't        *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      1 = packet was queue'd OK, 0 = wasn't                              *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   12/20/1994 BR : Created.                                              *
 *=========================================================================*/
int ConnectionClass::Send_Packet (void * buf, int buflen, int ack_req)
{
	if (buf == NULL || buflen <= 0) {
		Record_Packet_Drop(CONNECTION_DROP_EMPTY_DATA);
		return(0);
	}
	if (buflen > MaxPacketLen - (int)sizeof(CommHeaderType)) {
		Record_Packet_Drop(CONNECTION_DROP_OVERSIZED_DATA);
		return(0);
	}

	/*------------------------------------------------------------------------
	Set the magic # for the packet
	------------------------------------------------------------------------*/
	((CommHeaderType *)PacketBuf)->MagicNumber = MagicNum;

	/*------------------------------------------------------------------------
	Set the packet Code: DATA_ACK if it requires an ACK, NOACK if it doesn't
	Set the packet ID to the appropriate counter value.
	------------------------------------------------------------------------*/
	if (ack_req) {
		((CommHeaderType *)PacketBuf)->Code = PACKET_DATA_ACK;
		((CommHeaderType *)PacketBuf)->PacketID = NumSendAck;
	} else {
		((CommHeaderType *)PacketBuf)->Code = PACKET_DATA_NOACK;
		((CommHeaderType *)PacketBuf)->PacketID = NumSendNoAck;
	}

	/*------------------------------------------------------------------------
	Now build the packet
	------------------------------------------------------------------------*/
	memcpy(PacketBuf + sizeof(CommHeaderType), buf, buflen);

	/*------------------------------------------------------------------------
	Add it to the queue; don't add any extra data with it.
	------------------------------------------------------------------------*/
	if (Queue->Queue_Send(PacketBuf,buflen + sizeof(CommHeaderType), NULL, 0)) {
		if (ack_req) {
			NumSendAck++;
		} else {
			NumSendNoAck++;
		}
		return(1);
	} else {
		return(0);
	}

}	/* end of Send_Packet */


/***************************************************************************
 * ConnectionClass::Receive_Packet -- adds packet to receive queue         *
 *                                                                         *
 * INPUT:                                                                  *
 *      buf      buffer to process (already includes CommHeaderType)       *
 *      buflen   length of buffer to process                               *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      1 = packet was processed OK, 0 = error                             *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   12/20/1994 BR : Created.                                              *
 *=========================================================================*/
int ConnectionClass::Receive_Packet (void * buf, int buflen)
{
	CommHeaderType packet_header;   // packet header
	CommHeaderType *packet;         // ptr to packet header
	SendQueueType *send_entry;      // ptr to send entry header
	ReceiveQueueType *rec_entry;    // ptr to recv entry header
	CommHeaderType ackpacket;       // ACK packet to send
	int i;
	int save_packet = 1;									// 0 = this is a resend
	int found;

	std::span<std::byte const> packet_bytes;
	if (buf != NULL && buflen > 0) {
		packet_bytes = {static_cast<std::byte const *>(buf), static_cast<std::size_t>(buflen)};
	}
	NetAdmission::ConnectionResult const admission = NetAdmission::Admit_Connection_Packet(
		packet_bytes, sizeof(CommHeaderType), static_cast<std::size_t>(MaxPacketLen));
	if (!admission.Succeeded()) {
		Record_Admission_Drop(admission.ErrorCode, admission.Code);
		return(1);
	}

	packet_header.MagicNumber = admission.Magic;
	packet_header.Code = admission.Code;
	packet_header.PacketID = admission.PacketID;
	packet = &packet_header;

	/*------------------------------------------------------------------------
	Check the magic #
	------------------------------------------------------------------------*/
	if (packet->MagicNumber != MagicNum) {
		MissedMagic++;
		return(0);
	}

	/*------------------------------------------------------------------------
	Handle an incoming ACK
	------------------------------------------------------------------------*/
	if (packet->Code == PACKET_ACK) {

		for (i = 0; i < Queue->Num_Send(); i++) {
			/*..................................................................
			Get queue entry ptr
			..................................................................*/
			send_entry = Queue->Get_Send(i);

			/*..................................................................
			If ptr is valid, get ptr to its data
			..................................................................*/
			if (send_entry != NULL) {
				std::span<std::byte const> entry_bytes;
				if (send_entry->Buffer != NULL && send_entry->BufLen > 0) {
					entry_bytes = {reinterpret_cast<std::byte const *>(send_entry->Buffer),
						static_cast<std::size_t>(send_entry->BufLen)};
				}
				NetAdmission::ConnectionResult const entry = NetAdmission::Admit_Connection_Packet(
					entry_bytes, sizeof(CommHeaderType), static_cast<std::size_t>(MaxPacketLen));

				/*...............................................................
				If ACK is for this entry, mark it
				...............................................................*/
				if (entry.Succeeded() && packet->PacketID == entry.PacketID &&
					entry.Code == PACKET_DATA_ACK) {
					send_entry->IsACK = 1;
					break;
				}
			}
		}

		return(1);
	}

	/*------------------------------------------------------------------------
	Handle an incoming PACKET_DATA_NOACK packet
	------------------------------------------------------------------------*/
	else if (packet->Code == PACKET_DATA_NOACK) {
		/*.....................................................................
		If there's only one slot left, don't tie up the queue with this packet
		.....................................................................*/
		if (Queue->Max_Receive() - Queue->Num_Receive() <= 1) {
			MissedOverall++;
			return(0);
		}

		/*.....................................................................
		Error if we can't queue the packet
		.....................................................................*/
		if (!Queue->Queue_Receive (buf, buflen, NULL, 0)) {
			DebugString("Error - unable to queue incoming packet. %s %d\n", __FILE__, __LINE__);
			MissedOverall++;
			return(0);
		}

		NumRecNoAck++;

		NumLost = packet->PacketID - NumRecNoAck + 1;
		if (packet->PacketID) {
			PercentLost = 100 - (100 * NumRecNoAck) / (unsigned int)(packet->PacketID + 1);
		}

		return(1);
	}

	/*------------------------------------------------------------------------
	Handle an incoming PACKET_DATA_ACK packet
	------------------------------------------------------------------------*/
	else if (packet->Code == PACKET_DATA_ACK) {
		/*.....................................................................
		If this is a packet requires an ACK, and it's ID is older than our
		"oldest" ID, we know it's a resend; send an ACK, but don't queue it
		.....................................................................*/
		if (packet->PacketID <= LastSeqID && LastSeqID != 0xffffffff) {
			save_packet = 0;
		}

		/*.....................................................................
		Otherwise, scan the queue for this entry; if it's found, it's a
		resend, so don't save it.
		.....................................................................*/
		else {
			save_packet = 1;
			for (i = 0; i < Queue->Num_Receive(); i++) {
				rec_entry = Queue->Get_Receive(i);

				if (rec_entry) {
					std::span<std::byte const> entry_bytes;
					if (rec_entry->Buffer != NULL && rec_entry->BufLen > 0) {
						entry_bytes = {reinterpret_cast<std::byte const *>(rec_entry->Buffer),
							static_cast<std::size_t>(rec_entry->BufLen)};
					}
					NetAdmission::ConnectionResult const entry = NetAdmission::Admit_Connection_Packet(
						entry_bytes, sizeof(CommHeaderType), static_cast<std::size_t>(MaxPacketLen));

					/*...........................................................
					Packet is found; it's a resend
					...........................................................*/
					if (entry.Succeeded() && entry.Code == PACKET_DATA_ACK &&
						entry.PacketID == packet->PacketID) {
						save_packet = 0;
						break;
					}
				}
			}
		}	/* end of scan for resend */

		/*.....................................................................
		Queue the packet & update our LastSeqID value.
		.....................................................................*/
		if (save_packet) {
			/*..................................................................
			If there's only one slot left, make sure we only put a packet in it
			if this packet will let us increment our LastSeqID; otherwise, we'll
			get stuck, forever unable to increment LastSeqID.
			..................................................................*/
			if (Queue->Max_Receive() - Queue->Num_Receive() <= 1) {
				if (packet->PacketID != (LastSeqID + 1) ) {
					return(0);
				}
			}

			/*..................................................................
			If we can't queue the packet, return; don't send an ACK.
			..................................................................*/
			if (!Queue->Queue_Receive (buf, buflen, NULL, 0)) {
				return(0);
			}

			NumRecAck++;

			/*..................................................................
			Update our LastSeqID value if we can.  Anything less than LastSeqID
			we'll know is a resend.
			..................................................................*/
			if (packet->PacketID == (LastSeqID + 1)) {
				LastSeqID = packet->PacketID;
				/*...............................................................
				Now that we have a new 'LastSeqID', search our Queue to see if
				the next ID is there; if so, keep checking for the next one;
				break only when the next one isn't found.  This forces
				LastSeqID to be the largest possible value.
				...............................................................*/
				do {
					found = 0;
					for (i = 0; i < Queue->Num_Receive(); i++) {

						rec_entry = Queue->Get_Receive(i);

						if (rec_entry) {
							std::span<std::byte const> entry_bytes;
							if (rec_entry->Buffer != NULL && rec_entry->BufLen > 0) {
								entry_bytes = {reinterpret_cast<std::byte const *>(rec_entry->Buffer),
									static_cast<std::size_t>(rec_entry->BufLen)};
							}
							NetAdmission::ConnectionResult const entry = NetAdmission::Admit_Connection_Packet(
								entry_bytes, sizeof(CommHeaderType),
								static_cast<std::size_t>(MaxPacketLen));

							/*......................................................
							Entry is found
							......................................................*/
							if (entry.Succeeded() && entry.Code == PACKET_DATA_ACK &&
								entry.PacketID == (LastSeqID + 1)) {

								LastSeqID = entry.PacketID;
								found = 1;
								break;
							}
						}
					}
				} while (found);
			}
		}	/* end of save packet */

		/*.....................................................................
		Send an ACK, regardless of whether this was a resend or not.
		.....................................................................*/
		ackpacket.MagicNumber = Magic_Num();
		ackpacket.Code = PACKET_ACK;
		ackpacket.PacketID = packet->PacketID;
		Send ((char *)&ackpacket, sizeof(CommHeaderType), NULL, 0);

		return(1);
	}

	return(0);

}	/* end of Receive_Packet */


/***************************************************************************
 * ConnectionClass::Get_Packet -- gets a packet from receive queue         *
 *                                                                         *
 * INPUT:                                                                  *
 *      buf      location to store buffer                                  *
 *      capacity maximum bytes the caller's buffer can store               *
 *      buflen   filled in with length of 'buf'                            *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      1 = packet was read, 0 = wasn't                                    *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   12/20/1994 BR : Created.                                              *
 *=========================================================================*/
int ConnectionClass::Get_Packet (void * buf, int capacity, int *buflen)
{
	ReceiveQueueType *rec_entry;    // ptr to receive entry header
	int i;

	if (buflen != NULL) {
		(*buflen) = 0;
	}
	if (buf == NULL || buflen == NULL || capacity <= 0) {
		return(0);
	}

	/*------------------------------------------------------------------------
	Ensure that we read the packets in order.  LastReadID is the ID of the
	last PACKET_DATA_ACK packet we read.
	------------------------------------------------------------------------*/
	for (i = 0; i < Queue->Num_Receive(); i++) {

		rec_entry = Queue->Get_Receive(i);

		/*.....................................................................
		Only read this entry if it hasn't been yet
		.....................................................................*/
		if (rec_entry && rec_entry->IsRead==0) {
			std::span<std::byte const> entry_bytes;
			if (rec_entry->Buffer != NULL && rec_entry->BufLen > 0) {
				entry_bytes = {reinterpret_cast<std::byte const *>(rec_entry->Buffer),
					static_cast<std::size_t>(rec_entry->BufLen)};
			}
			NetAdmission::ConnectionResult const admission = NetAdmission::Admit_Connection_Packet(
				entry_bytes, sizeof(CommHeaderType), static_cast<std::size_t>(MaxPacketLen));
			if (!admission.Succeeded()) {
				rec_entry->IsRead = 1;
				Record_Admission_Drop(admission.ErrorCode, admission.Code);
				continue;
			}
			if (admission.Code == PACKET_ACK) {
				rec_entry->IsRead = 1;
				Record_Packet_Drop(CONNECTION_DROP_INVALID_CODE);
				continue;
			}

			/*..................................................................
			If this is a DATA_ACK packet, its ID must be one greater than
			the last one we read.
			..................................................................*/
			if ( (admission.Code == PACKET_DATA_ACK) &&
				(admission.PacketID == (LastReadID + 1))) {

				LastReadID = admission.PacketID;
				rec_entry->IsRead = 1;

				NetAdmission::Error const destination = NetAdmission::Validate_Destination(
					admission.Payload, static_cast<std::size_t>(capacity));
				if (destination != NetAdmission::Error::NONE) {
					Record_Admission_Drop(destination, admission.Code);
					continue;
				}
				memcpy(buf, admission.Payload.data(), admission.Payload.size());
				(*buflen) = static_cast<int>(admission.Payload.size());
				return(1);
			}
			/*..................................................................
			If this is a DATA_NOACK packet, who cares what the ID is?
			..................................................................*/
			else if (admission.Code == PACKET_DATA_NOACK) {
				rec_entry->IsRead = 1;

				NetAdmission::Error const destination = NetAdmission::Validate_Destination(
					admission.Payload, static_cast<std::size_t>(capacity));
				if (destination != NetAdmission::Error::NONE) {
					Record_Admission_Drop(destination, admission.Code);
					continue;
				}
				memcpy(buf, admission.Payload.data(), admission.Payload.size());
				(*buflen) = static_cast<int>(admission.Payload.size());
				return(1);
			}
		}
	}

	return(0);

}	/* end of Get_Packet */


namespace {

char const * Packet_Drop_Name(ConnectionClass::PacketDropReasonType reason)
{
	switch (reason) {
		case ConnectionClass::CONNECTION_DROP_SHORT_HEADER: return("connection-short-header");
		case ConnectionClass::CONNECTION_DROP_INVALID_CODE: return("connection-invalid-code");
		case ConnectionClass::CONNECTION_DROP_INVALID_LENGTH: return("connection-invalid-length");
		case ConnectionClass::CONNECTION_DROP_EMPTY_DATA: return("connection-empty-data");
		case ConnectionClass::CONNECTION_DROP_OVERSIZED_DATA: return("connection-oversized-data");
		case ConnectionClass::CONNECTION_DROP_OUTPUT_TOO_SMALL: return("connection-output-too-small");
		default: return("connection-unknown");
	}
}

}	// namespace


/// <summary>Returns the number of packets rejected for one stable admission reason.</summary>
unsigned int ConnectionClass::Dropped_Packets(PacketDropReasonType reason) const
{
	if (reason < 0 || reason >= CONNECTION_DROP_COUNT) {
		return(0);
	}

	return(DroppedPackets[reason]);
}


/// <summary>Records and rate-limits diagnostics for one rejected packet.</summary>
void ConnectionClass::Record_Packet_Drop(PacketDropReasonType reason)
{
	if (reason < 0 || reason >= CONNECTION_DROP_COUNT) {
		return;
	}

	unsigned int count = ++DroppedPackets[reason];
	if (count == 1 || (count & (count - 1)) == 0) {
		DebugString("Network packet drop [%s]: %u\n", Packet_Drop_Name(reason), count);
	}
}


/// <summary>Maps one shared admission failure to the connection's stable drop counters.</summary>
void ConnectionClass::Record_Admission_Drop(NetAdmission::Error error, unsigned char code)
{
	switch (error) {
		case NetAdmission::Error::HEADER_TOO_SHORT:
		case NetAdmission::Error::DATAGRAM_TOO_SHORT:
			Record_Packet_Drop(CONNECTION_DROP_SHORT_HEADER);
			break;
		case NetAdmission::Error::PACKET_TOO_LARGE:
		case NetAdmission::Error::DATAGRAM_TOO_LARGE:
			Record_Packet_Drop(CONNECTION_DROP_OVERSIZED_DATA);
			break;
		case NetAdmission::Error::INVALID_PACKET_CODE:
			Record_Packet_Drop(CONNECTION_DROP_INVALID_CODE);
			break;
		case NetAdmission::Error::INVALID_PACKET_LENGTH:
			Record_Packet_Drop(code == PACKET_ACK
				? CONNECTION_DROP_INVALID_LENGTH : CONNECTION_DROP_EMPTY_DATA);
			break;
		case NetAdmission::Error::DESTINATION_TOO_SMALL:
			Record_Packet_Drop(CONNECTION_DROP_OUTPUT_TOO_SMALL);
			break;
		case NetAdmission::Error::BAD_CRC:
			Record_Packet_Drop(CONNECTION_DROP_INVALID_LENGTH);
			break;
		case NetAdmission::Error::NONE:
		case NetAdmission::Error::COUNT:
			break;
	}
}


/***************************************************************************
 * ConnectionClass::Service -- main polling routine; services packets      *
 *                                                                         *
 * INPUT:                                                                  *
 *      none.                                                              *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      1 = OK, 0 = error (connection is broken!)                          *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   12/20/1994 BR : Created.                                              *
 *=========================================================================*/
int ConnectionClass::Service (void)
{
	/*------------------------------------------------------------------------
	Service the Send Queue:  This [re]sends packets in the Send Queue which
	haven't been ACK'd yet, and if their retry timeout has expired, and
	updates the FirstTime, LastTime & SendCount values in the Queue entry.
	Entries that have been ACK'd should be removed.

	Service the Receive Queue:  This sends ACKs for packets that haven't
	been ACK'd yet.  Entries that the app has read, and have been ACK'd,
	should be removed.
	------------------------------------------------------------------------*/
	IsBad = !(Service_Send_Queue() && Service_Receive_Queue());
	return(IsBad ? 0 : 1);

}	/* end of Service */


/***************************************************************************
 * ConnectionClass::Service_Send_Queue -- services the send queue          *
 *                                                                         *
 * INPUT:                                                                  *
 *      none.                                                              *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      1 = OK, 0 = error                                                  *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   12/20/1994 BR : Created.                                              *
 *=========================================================================*/
int ConnectionClass::Service_Send_Queue (void)
{
	int i;
	int num_entries;
	SendQueueType *send_entry;  // ptr to send queue entry
	CommHeaderType packet_header; // packet header
	unsigned int curtime;      // current time
	int bad_conn = 0;

	/*------------------------------------------------------------------------
	Remove any ACK'd packets from the queue
	------------------------------------------------------------------------*/
	for (i = 0; i < Queue->Num_Send(); i++) {
		/*.....................................................................
		Get this queue entry
		.....................................................................*/
		send_entry = Queue->Get_Send(i);

		/*.....................................................................
		If ACK has been received, unqueue it
		.....................................................................*/
		if (send_entry->IsACK) {

			/*..................................................................
			Update this queue's response time
			..................................................................*/
			if (send_entry->BufLen >= (int)sizeof(CommHeaderType)) {
				CommHeaderType header;
				memcpy(&header, send_entry->Buffer, sizeof(header));
				if (header.Code == PACKET_DATA_ACK) {
					Queue->Add_Delay(Time() - send_entry->FirstTime);
					if (Adaptive_Timing_Enabled()) {
						RoundTripEstimator.Acknowledge(send_entry->FirstTimeMilliseconds, send_entry->SendCount, *MillisecondTime);
					}
				}
			}

			/*..................................................................
			Unqueue the packet
			..................................................................*/
			Queue->UnQueue_Send(NULL,NULL,i,NULL,NULL);
			i--;
		}
	}

	/*------------------------------------------------------------------------
	Loop through all entries in the Send queue.  [Re]Send any entries that
	need it.
	------------------------------------------------------------------------*/
	num_entries = Queue->Num_Send();
	curtime = Time();
	NetTiming::Milliseconds const current_milliseconds = MillisecondTime->Now();
	bool const adaptive_channel = Adaptive_Timing_Enabled();
	bool const adaptive_timing = adaptive_channel && RoundTripEstimator.Has_Sample();
	bool const timeout_enabled = Timeout != (unsigned int)-1;
	NetTiming::Milliseconds const connection_timeout = !timeout_enabled
		? NetTiming::MAXIMUM_CONNECTION_TIMEOUT
		: (adaptive_timing ? NetTiming::Connection_Timeout(RoundTripEstimator.Smoothed_Rtt())
			: (adaptive_channel ? Legacy_Connection_Timeout(Timeout) : Ticks_To_Milliseconds(Timeout)));
	NetTiming::Milliseconds const base_retry_timeout = adaptive_timing
		? NetTiming::Initial_Retry_Timeout(RoundTripEstimator.Retransmit_Timeout(), connection_timeout)
		: Ticks_To_Milliseconds(RetryDelta);

	for (i = 0; i < num_entries; i++) {
		send_entry = Queue->Get_Send(i);

		if (send_entry->IsACK) {
			continue;
		}

		NetTiming::RetransmitState const retransmit_state{
			send_entry->FirstTimeMilliseconds,
			send_entry->LastTimeMilliseconds,
			send_entry->RetransmitTimeoutMilliseconds,
			send_entry->SendCount
		};
		NetTiming::RetryDecision const retry_decision = NetTiming::Evaluate_Retry(
			retransmit_state, current_milliseconds, base_retry_timeout, connection_timeout, timeout_enabled, adaptive_channel);
		if (retry_decision.TimedOut) {
			bad_conn = 1;
			send_entry->IsUndeliverable = true;
		}
		if (retry_decision.Send) {

			/*..................................................................
			Send the message
			..................................................................*/
			Send (send_entry->Buffer, send_entry->BufLen, send_entry->ExtraBuffer,
				send_entry->ExtraLen);

			/*..................................................................
			Fill in Time fields
			..................................................................*/
			send_entry->LastTime = curtime;
			send_entry->LastTimeMilliseconds = current_milliseconds;
			if (send_entry->SendCount==0) {
				send_entry->FirstTime = curtime;
				send_entry->FirstTimeMilliseconds = current_milliseconds;
				send_entry->RetransmitTimeoutMilliseconds = base_retry_timeout;

				/*...............................................................
				If this is the 1st time we're sending this packet, and it doesn't
				require an ACK, mark it as ACK'd; then, the next time through,
				it will just be removed from the queue.
				...............................................................*/
				memcpy(&packet_header, send_entry->Buffer, sizeof(packet_header));
				if (packet_header.Code == PACKET_DATA_NOACK) {
					send_entry->IsACK = 1;
				}
			} else {
				NumResends++;
				if (adaptive_channel) {
					RoundTripEstimator.Note_Retransmit(send_entry->RetransmitTimeoutMilliseconds);
				}
			}

			/*..................................................................
			Update SendCount
			..................................................................*/
			send_entry->SendCount++;

			/*..................................................................
			Perform error detection, based on either MaxRetries or Timeout
			..................................................................*/
			if (MaxRetries != -1 && send_entry->SendCount > MaxRetries) {
				bad_conn = 1;
				send_entry->IsUndeliverable = true;
			}
		}
	}

	/*------------------------------------------------------------------------
	If the connection is going bad, return an error
	------------------------------------------------------------------------*/
	if (bad_conn) {
		return(0);
	} else {
		return(1);
	}

}	/* end of Service_Send_Queue */


/// <summary>
/// Handles a connection that has failed its service pass.
/// This routine is the do-nothing default for connections that cannot clog. The
/// connection manager calls it when Service reports failure, giving a derived
/// connection the chance to throw away whatever is blocking its queues.
/// </summary>
/// <returns>Returns with the number of entries discarded, none at this level.</returns>
int ConnectionClass::Discard_Undeliverable_Packets(void)
{
	return(0);
}

/***************************************************************************
 * ConnectionClass::Service_Receive_Queue -- services receive queue        *
 *                                                                         *
 * INPUT:                                                                  *
 *      none.                                                              *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      1 = OK, 0 = error                                                  *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   12/20/1994 BR : Created.                                              *
 *=========================================================================*/
int ConnectionClass::Service_Receive_Queue (void)
{
	ReceiveQueueType *rec_entry;    // ptr to receive entry header
	CommHeaderType packet_header;   // packet header
	int i;

	/*------------------------------------------------------------------------
	Remove all dead packets.
	PACKET_DATA_NOACK: if it's been read, throw it away.
	PACKET_DATA_ACK: if it's been read, and its ID is older than LastSeqID,
	throw it away.
	------------------------------------------------------------------------*/
	for (i = 0; i < Queue->Num_Receive(); i++) {
		rec_entry = Queue->Get_Receive(i);

		if (rec_entry->IsRead) {
			if (rec_entry->BufLen < (int)sizeof(packet_header)) {
				Queue->UnQueue_Receive(NULL, NULL, i, NULL, NULL);
				i--;
				continue;
			}
			memcpy(&packet_header, rec_entry->Buffer, sizeof(packet_header));

			if (packet_header.Code == PACKET_DATA_NOACK) {
				Queue->UnQueue_Receive(NULL,NULL,i,NULL,NULL);
				i--;

			} else if (packet_header.PacketID < LastSeqID) {
				Queue->UnQueue_Receive(NULL,NULL,i,NULL,NULL);
				i--;
			}
		}
	}

	return(1);

}	/* end of Service_Receive_Queue */


/***************************************************************************
 * ConnectionClass::Time -- gets current time                              *
 *                                                                         *
 * INPUT:                                                                  *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      none.                                                              *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   12/20/1994 BR : Created.                                              *
 *=========================================================================*/
unsigned int ConnectionClass::Time (void)
{
	return(TickCount);
}	/* end of Time */


/***************************************************************************
 * ConnectionClass::Command_Name -- returns name for given packet command  *
 *                                                                         *
 * INPUT:                                                                  *
 *      command      packet Command value to get name for                  *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      ptr to command name, NULL if invalid                               *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   05/31/1995 BRR : Created.                                             *
 *=========================================================================*/
const char *ConnectionClass::Command_Name(int command)
{
	if (command >= 0 && command < PACKET_COUNT) {
		return(Commands[command]);
	} else {
		return(NULL);
	}

}	/* end of Command_Name */

/************************** end of connect.cpp *****************************/
