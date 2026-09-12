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

/* $Header: /CounterStrike/IPXMGR.H 1     3/03/97 10:24a Joe_bostic $ */
/***************************************************************************
 **   C O N F I D E N T I A L --- W E S T W O O D    S T U D I O S        **
 ***************************************************************************
 *                                                                         *
 *                 Project Name : Command & Conquer                        *
 *                                                                         *
 *                    File Name : IPXMGR.H                                 *
 *                                                                         *
 *                   Programmer : Bill Randolph                            *
 *                                                                         *
 *                   Start Date : December 19, 1994                        *
 *                                                                         *
 *                  Last Update : April 3, 1995   [BR]                     *
 *                                                                         *
 *-------------------------------------------------------------------------*
 *                                                                         *
 * This is the Connection Manager for IPX network communications.  It      *
 * creates, manages, & orchestrates multiple IPX connections, as well as   *
 * the "global" connection ("Global Channel"), which can talk to any       *
 * system on the net.                                                      *
 *                                                                         *
 * Use the Global Channel to query systems for their names, ID's, &        *
 * IPX addresses.  Then, create a Private Connection with each system      *
 * that joins your game, and use the Private Channel to send game packets  *
 * (the private channel will perform somewhat faster, & gives you better   *
 * control than the Global Channel; it can detect retries, and the Global  *
 * Channel can't).                                                         *
 *                                                                         *
 * HOW THIS CLASS WORKS:                                                   *
 * This class has to set up an IPX Event Service Routine in low (DOS)      *
 * memory.  So, it uses DPMI to allocate & lock a chunk of DOS memory;     *
 * this memory is used for all incoming packet buffers, the outgoing       *
 * packet buffer, and the actual code for the event handler.  The real-    *
 * mode handler code & this class share a portion of memory that's mapped  *
 * into a "RealModeDataType" structure.  As packets come in, the handler   *
 * points IPX to the next available packet buffer & restarts listening;    *
 * it sets a flag to tell this class that a packet is present at that      *
 * buffer slot.  This class must read all the packets & determine which    *
 * connection they go with (the Global Channel, or one of the Private      *
 * Channels).  This parsing is done in the Service routine for this class. *
 *                                                                         *
 * Constructor:   Just inits some variables, checks to see if IPX is there *
 * Destructor:    Complete shutdown; stops IPX listening, frees all memory *
 * Init:          Should only be called once (but can be called more);     *
 *                allocates all memory, creates the Global Channel         *
 *                connection, starts IPX listening.  By not placing this   *
 *                step in the constructor, the app can control when        *
 *                listening actually starts; also, you don't get a bunch   *
 *                of allocations just by declaring an IPXManagerClass      *
 *                instance.  You have to call Init() for the allocations   *
 *                to occur.                                                *
 * Connection utilities: Create & manage Private Connections.  Each        *
 *                connection has its own IPX address, numerical ID, and    *
 *                character name (presumably the name of the other         *
 *                player).                                                 *
 * Send/Get_Global_Message: adds a packet to the Global Connection queue,  *
 *                or reads from the queue.  The caller should check the    *
 *                ProductID value from returned packets to be sure it's    *
 *                talking to the right product.                            *
 * Send/Get_Private_Message: adds a packet to a Private Connection queue,  *
 *                or reads from the queue                                  *
 * Service:       Checks the Real-Mode-Memory packet array to see if any   *
 *                new packets have come in; if they have, it parses them   *
 *                & distributes them to the right connection queue.  The   *
 *                queue's Service routine handles ACK'ing or Resending     *
 *                packets.                                                 *
 *                                                                         *
 * Here's a memory map of the Real-Mode memory block.  'N' is the number   *
 * of packet buffers allocated in low memory:                              *
 *                                                                         *
 *                ----------------------------------                       *
 *                |       Shared-memory data       |                       *
 *                |--------------------------------|                       *
 *                |  Real-mode event handler code  |                       *
 *                |--------------------------------|                       *
 *                |  IPX Header & Packet Buffer 0  |                       *
 *                |--------------------------------|                       *
 *                |  IPX Header & Packet Buffer 1  |                       *
 *                |--------------------------------|                       *
 *                |  IPX Header & Packet Buffer 2  |                       *
 *                |--------------------------------|                       *
 *                |             . . .              |                       *
 *                |--------------------------------|                       *
 *                |  IPX Header & Packet Buffer N  |                       *
 *                |--------------------------------|                       *
 *                |    Send Event Control Block    |                       *
 *                |--------------------------------|                       *
 *                |         Send IPX Header        |                       *
 *                |--------------------------------|                       *
 *                |       Send Packet Buffer       |                       *
 *                |--------------------------------|                       *
 *                |        Flags Array [N]         |                       *
 *                ----------------------------------                       *
 *                                                                         *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#pragma once


/*
********************************* Includes **********************************
*/
#include "connmgr.h"
#include "ipxaddr.h"
#include "ipxconn.h"
#include "ipxgconn.h"

/*
********************************** Defines **********************************
*/
/*---------------------------------------------------------------------------
This is the maximum number of connections supported.  Just change this
value to support more.
---------------------------------------------------------------------------*/
#define	CONNECT_MAX			7

/*
***************************** Class Declaration *****************************
*/
class IPXManagerClass : public ConnManClass
{
		typedef ConnManClass BASECLASS;

	/*
	---------------------------- Public Interface ----------------------------
	*/
	public:
		/*
		 * How the packets reach the other players. The mode decides how the transport is
		 * set up and how an address is read, not how the game plays; a LAN game and a
		 * tunnelled game differ only here.
		 */
		enum TransportModeType {
			TRANSPORT_NONE = 0,     // not configured; Init() will fail
			TRANSPORT_LAN,          // broadcast onto the local network to find games
			TRANSPORT_DIRECT,       // send to the addresses a lobby outside the game supplied
			TRANSPORT_TUNNEL,       // send through a CnCNet tunnel server
			TRANSPORT_DIRECT_PEERS  // send straight to a known set of peers
		};

		/*.....................................................................
		Constructor/destructor.
		.....................................................................*/
		IPXManagerClass (int glb_maxlen, int pvt_maxlen, int glb_num_packets,
			int pvt_num_packets, unsigned short product_id);
		virtual ~IPXManagerClass () override;	// stop listening

		/*
		 * One of these must be called before Init(). Each disposes of the transport
		 * already in place and creates the one its mode needs; Shutdown() takes it down.
		 */
		void Configure_LAN(unsigned short port = 0);
		void Configure_Direct();
		void Configure_Tunnel(unsigned short local_id, unsigned long tunnel_ip, unsigned short tunnel_port);
		void Configure_Direct_Peers(unsigned short listen_port);
		void Add_Peer(const IPXAddressClass &address);
		void Shutdown();

		TransportModeType Transport_Mode() const {return(TransportMode);}

		/*.....................................................................
		Initialization routines.
		.....................................................................*/
		int Init (void);
		virtual void Set_Timing (unsigned int retrydelta, unsigned int maxretries,
			unsigned int timeout, bool set_external = true) override;
		virtual void Set_External_Timing (unsigned int retrydelta, unsigned int maxretries,
			unsigned int timeout) override;

		/*.....................................................................
		These routines control creation of the "Connections" (data queues) for
		each remote system.
		.....................................................................*/
		int Create_Connection(int id, char *name, IPXAddressClass *address);
		int Delete_Connection(int id);
		virtual int Num_Connections(void) override;
		virtual int Connection_ID(int index) override;
		char *Connection_Name(int id);
		IPXAddressClass * Connection_Address(int id);
		virtual int Connection_Index(int id) override;
		void Set_Connection_Parms(int index, int id, char *name);

		/*.....................................................................
		This is how the application sends & receives messages.
		.....................................................................*/
		int Send_Global_Message (void *buf, int buflen, int ack_req = 0,
			IPXAddressClass *address = NULL);
		int Get_Global_Message (void *buf, int capacity, int *buflen, IPXAddressClass *address,
			unsigned short *product_id);

		virtual int Send_Private_Message (void *buf, int buflen,
			int ack_req = 1, int conn_id = CONNECTION_NONE) override;
		virtual int Get_Private_Message (void *buf, int capacity, int *buflen, int *conn_id) override;

		/*.....................................................................
		The main polling routine; should be called as often as possible.
		.....................................................................*/
		virtual int Service (void) override;

		/*.....................................................................
		This routine reports which connection has an error on it.
		.....................................................................*/
		int Get_Bad_Connection(void);
		int Receive_Discards(void) const { return(ReceiveDiscards); }

		/*.....................................................................
		Queue utility routines.  The application can determine how many
		messages are in the send/receive queues.
		.....................................................................*/
		virtual int Global_Num_Send(void) override;
		virtual int Global_Num_Receive(void) override;
		virtual int Private_Num_Send(int id = CONNECTION_NONE) override;
		virtual int Private_Num_Receive(int id = CONNECTION_NONE) override;

		/*.....................................................................
		Routines to return the largest average queue response time, and to
		reset the response time for all queues.
		.....................................................................*/
		virtual unsigned int Response_Time(void) override;
		virtual std::optional<NetTiming::Milliseconds> Worst_Local_Round_Trip_MS(void) const override;
		unsigned int Global_Response_Time(void);
		virtual void Reset_Response_Time(bool zero) override;

		virtual unsigned int Avg_Response_Time(int index);

		/*.....................................................................
		This routine returns a pointer to the oldest non-ACK'd buffer I've sent.
		.....................................................................*/
		void * Oldest_Send(void);

		/*.....................................................................
		Debug routines
		.....................................................................*/
		virtual void Store_Stats(void);
		virtual void Configure_Debug(int index, int type_offset, int type_size,
			char **names, int namestart, int namecount) override;

		virtual void Mono_Debug_Print(int index, int refresh = 0);

		void Multiplayer_Debug_Print(int top);

	/*
	--------------------------- Private Interface ----------------------------
	*/
	private:

		/*.....................................................................
		Misc variables
		.....................................................................*/
		TransportModeType TransportMode;	// how the packets reach the other players
		bool Listening;		// 1 = Listening is on

		/*.....................................................................
		Packet Sizes, used when allocating the channels
		.....................................................................*/
		int Glb_MaxPacketLen;				// Global Channel maximum packet size
		int Glb_NumPackets;					// # Global send/receive packets
		int Pvt_MaxPacketLen;				// Private Channel maximum packet size
		int Pvt_NumPackets;					// # Private send/receive packets

		/*.....................................................................
		The ProductID is used in the Global Channel's packet header, and it's
		used for the Private Channels' Magic Number.
		.....................................................................*/
		unsigned short ProductID;			// product ID

		/*.....................................................................
		Local connection number
		.....................................................................*/
		int ConnectionNum;					// local connection #, 0=not logged in

		/*.....................................................................
		Array of connection queues
		.....................................................................*/
		IPXConnClass * Connection[CONNECT_MAX]; // array of connection object ptrs
		int NumConnections;                     // # connection objects in use
		IPXGlobalConnClass *GlobalChannel;      // the Global Channel

		/*.....................................................................
		Current queue for polling for received packets
		.....................................................................*/
		int CurConnection;

		/*.....................................................................
		Timing parameters for all connections
		.....................................................................*/
		unsigned int RetryDelta;
		unsigned int MaxRetries;
		unsigned int Timeout;

		/*.....................................................................
		Various Statistics
		.....................................................................*/
		int SendOverflows;
		int ReceiveOverflows;
		int ReceiveDiscards;
		int BadConnection;
};


/*************************** end of ipxmgr.h *******************************/
