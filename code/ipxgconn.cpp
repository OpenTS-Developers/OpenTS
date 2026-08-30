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

/* $Header: /CounterStrike/IPXGCONN.CPP 3     10/13/97 2:20p Steve_t $ */
/***************************************************************************
 **   C O N F I D E N T I A L --- W E S T W O O D    S T U D I O S        **
 ***************************************************************************
 *                                                                         *
 *                 Project Name : Command & Conquer                        *
 *                                                                         *
 *                    File Name : IPXGCONN.CPP                             *
 *                                                                         *
 *                   Programmer : Bill Randolph                            *
 *                                                                         *
 *                   Start Date : December 20, 1994                        *
 *                                                                         *
 *                  Last Update : July 6, 1995 [BRR]                       *
 *-------------------------------------------------------------------------*
 * Functions:                                                              *
 *   IPXGlobalConnClass::IPXGlobalConnClass -- class constructor           *
 *   IPXGlobalConnClass::~IPXGlobalConnClass -- class destructor           *
 *   IPXGlobalConnClass::Send_Packet -- adds a packet to the send queue    *
 *   IPXGlobalConnClass::Receive_Packet -- adds packet to the receive queue*
 *   IPXGlobalConnClass::Get_Packet -- gets a packet from the receive queue*
 *   IPXGlobalConnClass::Send -- sends a packet                            *
 *   IPXGlobalConnClass::Service_Receive_Queue -- services receive queue   *
 *   IPXGlobalConnClass::Set_Bridge -- Sets up connection to cross a bridge*
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "ipxgconn.h"

#include "dbgprint.h"

#include <cstdio>
#include <cstring>


/***************************************************************************
 * IPXGlobalConnClass::IPXGlobalConnClass -- class constructor             *
 *                                                                         *
 * This routine chains to the parent constructor, but it adjusts the size  *
 * of the packet by the added bytes in the GlobalHeaderType structure.     *
 * This forces the parent classes to allocate the proper sized PacketBuf   *
 * for outgoing packets, and to set MaxPacketLen to the proper value.      *
 *                                                                         *
 * INPUT:                                                                  *
 *      numsend         desired # of entries for the send queue            *
 *      numreceive      desired # of entries for the receive queue         *
 *      maxlen         max length of an application packet                 *
 *      product_id      unique ID for this product                         *
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
IPXGlobalConnClass::IPXGlobalConnClass (int numsend, int numreceive,
	int maxlen, unsigned short product_id) :
	BASECLASS (numsend, numreceive,
		maxlen + sizeof(GlobalHeaderType) - sizeof(CommHeaderType),
		GLOBAL_MAGICNUM,            // magic number for this connection
		NULL,                       // IPX Address (none)
		0,                          // Connection ID
		"",                         // Connection Name
		sizeof (IPXAddressClass))   // extra storage for the sender's address
{
	int i;

	ProductID = product_id;
	MaxRXIndex = numreceive;
	LastPacketID = new unsigned int[MaxRXIndex];
	LastAddress = new IPXAddressClass[MaxRXIndex];

	for (i = 0; i < MaxRXIndex; i++) {
		LastPacketID[i] = 0xffffffff;
	}

	LastRXIndex = 0;

}	/* end of IPXGlobalConnClass */


/// <summary>
/// Destroys the global connection.
/// The record of recently received packets, which the connection keeps in order to
/// recognize resends, is thrown away along with it.
/// </summary>
IPXGlobalConnClass::~IPXGlobalConnClass(void)
{
	if (LastPacketID) {
		delete [] LastPacketID;
		LastPacketID = NULL;
	}
	if (LastAddress) {
		delete [] LastAddress;
		LastAddress = NULL;
	}
}

/***************************************************************************
 * IPXGlobalConnClass::Send_Packet -- adds a packet to the send queue      *
 *                                                                         *
 * This routine prefixes the given buffer with a GlobalHeaderType and      *
 * queues the resulting packet into the Send Queue.  The packet's          *
 * MagicNumber, Code, PacketID, destination Address and ProductID are set  *
 * here.                                                                   *
 *                                                                         *
 * INPUT:                                                                  *
 *      buf         buffer to send                                         *
 *      buflen      length of buffer                                       *
 *      address      address to send the packet to (NULL = Broadcast)      *
 *      ack_req      1 = ACK is required for this packet; 0 = isn't        *
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
int IPXGlobalConnClass::Send_Packet (void * buf, int buflen,
	IPXAddressClass *address, int ack_req)
{
	IPXAddressClass dest_addr;

	if (buf == NULL || buflen <= 0) {
		Record_Packet_Drop(CONNECTION_DROP_EMPTY_DATA);
		return(0);
	}
	if (buflen > MaxPacketLen - (int)sizeof(GlobalHeaderType)) {
		Record_Packet_Drop(CONNECTION_DROP_OVERSIZED_DATA);
		return(0);
	}

	/*------------------------------------------------------------------------
	Store the packet's Magic Number
	------------------------------------------------------------------------*/
	((GlobalHeaderType *)PacketBuf)->Header.MagicNumber = MagicNum;

	/*------------------------------------------------------------------------
	If this is a ACK-required packet, sent to a specific system, mark it as
	ACK-required; otherwise, mark as no-ACK-required.
	------------------------------------------------------------------------*/
	if (ack_req && address != NULL) {
		((GlobalHeaderType *)PacketBuf)->Header.Code = PACKET_DATA_ACK;
	}
	else {
		((GlobalHeaderType *)PacketBuf)->Header.Code = PACKET_DATA_NOACK;
	}

	/*------------------------------------------------------------------------
	Fill in the packet ID.  This will have very limited meaning; it only
	allows us to determine if an ACK packet we receive later goes with this
	packet; it doesn't let us detect re-sends of other systems' packets.
	------------------------------------------------------------------------*/
	((GlobalHeaderType *)PacketBuf)->Header.PacketID = Queue->Send_Total();

	/*------------------------------------------------------------------------
	Set the product ID for this packet.
	------------------------------------------------------------------------*/
	((GlobalHeaderType *)PacketBuf)->ProductID = ProductID;

	/*------------------------------------------------------------------------
	Set this packet's destination address.  If no address is specified, use
	a Broadcast address (which IPXAddressClass's default constructor creates).
	------------------------------------------------------------------------*/
	if (address != NULL) {
		dest_addr = (*address);
	}

	/*------------------------------------------------------------------------
	Copy the application's data
	------------------------------------------------------------------------*/
	memcpy(PacketBuf + sizeof(GlobalHeaderType), buf, buflen);

	/*------------------------------------------------------------------------
	Queue it, along with the destination address
	------------------------------------------------------------------------*/
	return(Queue->Queue_Send(PacketBuf,buflen + sizeof(GlobalHeaderType),
		&dest_addr, sizeof (IPXAddressClass)));

}	/* end of Send_Packet */


/***************************************************************************
 * IPXGlobalConnClass::Receive_Packet -- adds packet to the receive queue  *
 *                                                                         *
 * INPUT:                                                                  *
 *      buf      buffer to process (already includes GlobalHeaderType)     *
 *      buflen   length of buffer to process                               *
 *      address   the address of the sender (the IPX Manager class must    *
 *               extract this from the IPX Header of the received packet.) *
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
int IPXGlobalConnClass::Receive_Packet (void * buf, int buflen,
	IPXAddressClass *address)
{
	SendQueueType *send_entry;      // ptr to send entry header
	GlobalHeaderType ackpacket;     // ACK packet to send
	int i;
	int resend;

	if (address == NULL) {
		Record_Packet_Drop(CONNECTION_DROP_SHORT_HEADER);
		return(1);
	}

	std::span<std::byte const> packet_bytes;
	if (buf != NULL && buflen > 0) {
		packet_bytes = {static_cast<std::byte const *>(buf), static_cast<std::size_t>(buflen)};
	}
	NetConnectionAdmission const packet = Admit_Connection_Packet(packet_bytes, sizeof(GlobalHeaderType), static_cast<std::size_t>(MaxPacketLen));
	if (!packet.Succeeded()) {
		Record_Admission_Drop(packet.Error, packet.Code);
		return(1);
	}

	/*------------------------------------------------------------------------
	Check the magic #
	------------------------------------------------------------------------*/
	if (packet.Magic != MagicNum) {
		return(0);
	}

	/*------------------------------------------------------------------------
	Process the packet based on its Code
	------------------------------------------------------------------------*/
	switch (packet.Code) {
		//.....................................................................
		// DATA_ACK: Check for a resend by comparing the source address &
		// ID of this packet with our last 4 received packets.
		// Send an ACK for the packet, regardless of whether it's a resend
		// or not.
		//.....................................................................
		case PACKET_DATA_ACK:
		{
			//..................................................................
			// Check for a resend
			//..................................................................
			resend = 0;
			for (i = 0; i < MaxRXIndex; i++) {
				if ((unsigned)i >= Queue->Receive_Total()) {
					break;
				}
				if ((*address)==LastAddress[i] &&
					packet.PacketID == LastPacketID[i]) {
					resend = 1;
					break;
				}
			}

			bool send_ack = true;

			//..................................................................
			// If it's not a resend, queue it; then, record the sender's address
			// & the packet ID for future resend detection.
			//..................................................................
			if (!resend) {
				if (Queue->Queue_Receive (buf, buflen, address, sizeof(IPXAddressClass))) {
					LastAddress[LastRXIndex] = (*address);
					LastPacketID[LastRXIndex] = packet.PacketID;
					LastRXIndex++;
					if (LastRXIndex >= MaxRXIndex) {
						LastRXIndex = 0;
					}
				}else{
					//..................................................................
					// Don't send an ack if we didn't have room to store the packet.
					//..................................................................
					send_ack = false;
					DebugString("Failed to queue incoming packet. Ack not sent\n");
				}
			}


			//..................................................................
			// Send an ACK for this packet
			//..................................................................
			if (send_ack) {
				ackpacket.Header.MagicNumber = MagicNum;
				ackpacket.Header.Code = PACKET_ACK;
				ackpacket.Header.PacketID = packet.PacketID;
				ackpacket.ProductID = ProductID;
				if (!Send ((char *)&ackpacket, sizeof(GlobalHeaderType),
					address, sizeof(IPXAddressClass))) {
					DebugString("Failed to ack global packet\n");
				}
			}


			break;
		}
		/*.....................................................................
		DATA_NOACK: Queue this message, along with the sender's address.
		Don't bother checking for a Re-Send, since the other system will only
		send this packet once.
		.....................................................................*/
		case PACKET_DATA_NOACK:
			Queue->Queue_Receive (buf, buflen, address, sizeof(IPXAddressClass));
			break;

		/*.....................................................................
		ACK: If this ACK is for any of my packets, mark that packet as
		acknowledged, then throw this packet away.  Otherwise, ignore the ACK
		(if we re-sent before we received the other system's first ACK, this
		ACK will be a leftover)
		.....................................................................*/
		case PACKET_ACK:
			for (i = 0; i < Queue->Num_Send(); i++) {
				/*...............................................................
				Get queue entry ptr
				...............................................................*/
				send_entry = Queue->Get_Send(i);

				/*...............................................................
				If ptr is valid, get ptr to its data
				...............................................................*/
				if (send_entry == NULL) {
					continue;
				}
				std::span<std::byte const> entry_bytes;
				if (send_entry->Buffer != NULL && send_entry->BufLen > 0) {
					entry_bytes = {reinterpret_cast<std::byte const *>(send_entry->Buffer), static_cast<std::size_t>(send_entry->BufLen)};
				}
				NetConnectionAdmission const entry = Admit_Connection_Packet(entry_bytes, sizeof(GlobalHeaderType), static_cast<std::size_t>(MaxPacketLen));

				/*...............................................................
				If ACK is for this entry, mark it
				...............................................................*/
				if (entry.Succeeded() && packet.PacketID == entry.PacketID && entry.Code == PACKET_DATA_ACK) {
					send_entry->IsACK = 1;
					break;
				}
			}
			break;

		/*.....................................................................
		Default: ignore the packet
		.....................................................................*/
		default:
			break;
	}

	return(1);

}	/* end of Receive_Packet */


/***************************************************************************
 * IPXGlobalConnClass::Get_Packet -- gets a packet from the receive queue  *
 *                                                                         *
 * INPUT:                                                                  *
 *      buf         location to store buffer                               *
 *      capacity    maximum bytes the caller's buffer can store            *
 *      buflen      filled in with length of 'buf'                         *
 *      address      filled in with sender's address                       *
 *      product_id   filled in with sender's ProductID                     *
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
int IPXGlobalConnClass::Get_Packet (void * buf, int capacity, int *buflen,
	IPXAddressClass *address, unsigned short *product_id)
{
	ReceiveQueueType *rec_entry;					// ptr to receive entry header

	if (buflen != NULL) {
		(*buflen) = 0;
	}
	if (buf == NULL || buflen == NULL || address == NULL || product_id == NULL || capacity <= 0) {
		return(0);
	}

	/*------------------------------------------------------------------------
	Return if nothing to do
	------------------------------------------------------------------------*/
	if (Queue->Num_Receive() == 0) {
		return(0);
	}

	/*------------------------------------------------------------------------
	Get ptr to the next available entry
	------------------------------------------------------------------------*/
	rec_entry = Queue->Get_Receive(0);

	/*------------------------------------------------------------------------
	Read it if it's un-read
	------------------------------------------------------------------------*/
	if (rec_entry!=NULL && rec_entry->IsRead==0) {
		std::span<std::byte const> entry_bytes;
		if (rec_entry->Buffer != NULL && rec_entry->BufLen > 0) {
			entry_bytes = {reinterpret_cast<std::byte const *>(rec_entry->Buffer), static_cast<std::size_t>(rec_entry->BufLen)};
		}
		NetConnectionAdmission const admission = Admit_Connection_Packet(entry_bytes, sizeof(GlobalHeaderType), static_cast<std::size_t>(MaxPacketLen));
		if (!admission.Succeeded()) {
			rec_entry->IsRead = 1;
			Record_Admission_Drop(admission.Error, admission.Code);
			return(0);
		}
		if (admission.Code == PACKET_ACK) {
			rec_entry->IsRead = 1;
			Record_Packet_Drop(CONNECTION_DROP_INVALID_CODE);
			return(0);
		}

		/*.....................................................................
		Mark as read
		.....................................................................*/
		rec_entry->IsRead = 1;

		/*.....................................................................
		Copy data packet
		.....................................................................*/
		NetAdmissionError const destination = Validate_Network_Destination(admission.Payload, static_cast<std::size_t>(capacity));
		if (destination != NetAdmissionError::NONE) {
			Record_Admission_Drop(destination, admission.Code);
			return(0);
		}
		memcpy(buf, admission.Payload.data(), admission.Payload.size());
		(*buflen) = static_cast<int>(admission.Payload.size());
		memcpy(product_id, entry_bytes.data() + offsetof(GlobalHeaderType, ProductID), sizeof(*product_id));
		memcpy(address, rec_entry->ExtraBuffer, sizeof(*address));

		return(1);
	}

	return(0);

}	/* end of Get_Packet */


/***************************************************************************
 * IPXGlobalConnClass::Send -- sends a packet                              *
 *                                                                         *
 * This routine gets invoked by NonSequencedConn, when it's processing     *
 * the Send & Receive Queues.  The buffer provided will already have the   *
 * GlobalHeaderType header embedded in it.                                 *
 *                                                                         *
 * INPUT:                                                                  *
 *      buf         buffer to send                                         *
 *      buflen      length of buffer                                       *
 *      extrabuf      extra buffer to send                                 *
 *      extralen      length of extra buffer                               *
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
int IPXGlobalConnClass::Send(char *buf, int buflen, void *extrabuf, int )
{
	IPXAddressClass *addr;
	int rc;

	/*------------------------------------------------------------------------
	Extract the packet's embedded IPX address
	------------------------------------------------------------------------*/
	addr = (IPXAddressClass *)extrabuf;

	/*------------------------------------------------------------------------
	If it's a broadcast address, broadcast it
	------------------------------------------------------------------------*/
	if (addr->Is_Broadcast()) {
		return(Broadcast (buf, buflen));
	}

	/*------------------------------------------------------------------------
	Otherwise, send it
	------------------------------------------------------------------------*/
	else {
		return(Send_To (buf, buflen, addr));
	}

} 	/* end of Send */


/***************************************************************************
 * IPXGlobalConnClass::Service_Receive_Queue -- services the receive queue *
 *                                                                         *
 * This routine is necessary because the regular ConnectionClass checks    *
 * for sequential packet ID's before removing them from the receive queue; *
 * this class cannot do that.                                              *
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
int IPXGlobalConnClass::Service_Receive_Queue (void)
{
	int i;
	ReceiveQueueType *rec_entry;					// ptr to receive entry header

	//------------------------------------------------------------------------
	// Remove all dead packets:  If a packet's been read, throw it away.
	//------------------------------------------------------------------------
	for (i = 0; i < Queue->Num_Receive(); i++) {
		rec_entry = Queue->Get_Receive(i);

		if (rec_entry->IsRead) {
			Queue->UnQueue_Receive(NULL,NULL,i,NULL,NULL);
			i--;
		}
	}

	return(1);

} 	/* end of Service_Receive_Queue */


/// <summary>
/// Throws away the outgoing packets that will never be acknowledged.
/// The IPX manager calls this routine when the global channel reports that it has gone
/// bad, so that a single packet nobody ever answers cannot clog the channel forever.
/// Only packets that have already given up on their acknowledgement are discarded.
/// </summary>
/// <returns>Returns with the number of packets thrown away.</returns>
int IPXGlobalConnClass::Discard_Undeliverable_Packets(void)
{
	int num = 0;

	for (int i = 0; i < Queue->Num_Send(); i++) {
		SendQueueType *send_entry = Queue->Get_Send(i);

		if (send_entry->IsUndeliverable && !send_entry->IsACK) {

			IPXAddressClass address;
			char buf[1024];
			int buflen;

			int address_size = sizeof(address);
			Queue->UnQueue_Send(buf, &buflen, i, &address, &address_size);
			i--;
			num++;
		}
	}

	return(num);
}


/// <summary>
/// Adds a packet to the send queue.
/// This is the plain form inherited from the base connection; the global channel's own
/// version also takes the address to send the packet to.
/// </summary>
/// <param name="ack_req">Should the receiver be required to acknowledge this packet?</param>
/// <returns>Returns with non-zero if the packet was queued for sending.</returns>
int IPXGlobalConnClass::Send_Packet(void * buf, int buflen, int ack_req)
{
	return(ConnectionClass::Send_Packet(buf, buflen, ack_req));
}


/// <summary>
/// Hands a freshly arrived packet over to the connection.
/// This is the plain form inherited from the base connection; the global channel's own
/// version also takes the address the packet came from, which it needs in order to
/// recognize resends.
/// </summary>
/// <returns>Returns with non-zero if the packet was accepted.</returns>
int IPXGlobalConnClass::Receive_Packet(void * buf, int buflen)
{
	return(ConnectionClass::Receive_Packet(buf, buflen));
}


/// <summary>
/// Fetches the next packet from the receive queue.
/// This is the plain form inherited from the base connection; the global channel's own
/// version also reports the address the packet came from and the product that sent it.
/// </summary>
/// <param name="buflen">Pointer to the value to fill in with the length of the packet.</param>
/// <returns>Returns with non-zero if a packet was pulled off the queue.</returns>
int IPXGlobalConnClass::Get_Packet(void * buf, int capacity, int * buflen)
{
	return(ConnectionClass::Get_Packet(buf, capacity, buflen));
}


/************************** end of ipxgconn.cpp ****************************/
