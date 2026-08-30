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

/* $Header: /CounterStrike/IPXMGR.CPP 3     10/13/97 2:20p Steve_t $ */
/***************************************************************************
 **   C O N F I D E N T I A L --- W E S T W O O D    S T U D I O S        **
 ***************************************************************************
 *                                                                         *
 *                 Project Name : Command & Conquer                        *
 *                                                                         *
 *                    File Name : IPXMGR.CPP                               *
 *                                                                         *
 *                   Programmer : Bill Randolph                            *
 *                                                                         *
 *                   Start Date : December 20, 1994                        *
 *                                                                         *
 *                  Last Update : May 4, 1995 [BRR]                        *
 *                                                                         *
 *-------------------------------------------------------------------------*
 * Functions:                                                              *
 *   IPXManagerClass::IPXManagerClass -- class constructor                 *
 *   IPXManagerClass::~IPXManagerClass -- class destructor                 *
 *   IPXManagerClass::Init -- initialization routine                       *
 *   IPXManagerClass::Is_IPX -- tells if IPX is installed or not           *
 *   IPXManagerClass::Set_Timing -- sets timing for all connections        *
 *   IPXManagerClass::Create_Connection -- creates a new connection        *
 *   IPXManagerClass::Delete_Connection -- deletes a connection            *
 *   IPXManagerClass::Num_Connections -- gets the # of connections         *
 *   IPXManagerClass::Connection_ID -- gets the given connection's ID      *
 *   IPXManagerClass::Connection_Name -- gets name for given connection    *
 *   IPXManagerClass::Connection_Address -- retrieves connection's address *
 *   IPXManagerClass::Connection_Index -- gets given connection's index    *
 *   IPXManagerClass::Set_Connection_Parms -- sets connection's name & id  *
 *   IPXManagerClass::Send_Global_Message -- sends a Global Message        *
 *   IPXManagerClass::Get_Global_Message -- polls the Global Message queue *
 *   IPXManagerClass::Send_Private_Message -- Sends a Private Message      *
 *   IPXManagerClass::Get_Private_Message -- Polls Private Message queue   *
 *   IPXManagerClass::Service -- main polling routine for IPX Connections  *
 *   IPXManagerClass::Get_Bad_Connection -- returns bad connection ID      *
 *   IPXManagerClass::Global_Num_Send  -- gets # entries in send queue     *
 *   IPXManagerClass::Global_Num_Receive -- gets # entries in recv queue   *
 *   IPXManagerClass::Private_Num_Send -- gets # entries in send queue     *
 *   IPXManagerClass::Private_Num_Receive -- gets # entries in recv queue  *
 *   IPXManagerClass::Set_Bridge -- prepares to cross a bridge             *
 *   IPXManagerClass::Set_Socket -- sets socket ID for all connections     *
 *   IPXManagerClass::Response_Time -- Returns largest Avg Response Time   *
 *   IPXManagerClass::Global_Response_Time -- Returns Avg Response Time    *
 *   IPXManagerClass::Reset_Response_Time -- Reset response time           *
 *   IPXManagerClass::Oldest_Send -- gets ptr to oldest send buf           *
 *   IPXManagerClass::Mono_Debug_Print -- debug output routine             *
 *   IPXManagerClass::Alloc_RealMode_Mem -- allocates real-mode memory     *
 *   IPXManagerClass::Free_RealMode_Mem -- frees real-mode memory          *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "ipxmgr.h"

#include "_surface.h"
#include "dbgprint.h"
#include "dialog.h"
#include "globals.h"
#include "house.h"
#include "queue.h"
#include "scheme.h"
#include "session.h"
#include "stats.h"
#include "stimer.h"
#include "surface.h"
#include "vector.h"
#include "wsproto.h"
#include "wspudp.h"

#include <algorithm>
#include <array>


/***************************************************************************
 * IPXManagerClass::IPXManagerClass -- class constructor                   *
 *                                                                         *
 * INPUT:                                                                  *
 *      glb_maxlen      Global Channel maximum packet length               *
 *      pvt_maxlen      Private Channel maximum packet length              *
 *      product_id      a unique numerical ID for this product             *
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
IPXManagerClass::IPXManagerClass (int glb_maxlen, int pvt_maxlen,
	int glb_num_packets, int pvt_num_packets, unsigned short product_id)
{
	int i;

	TransportMode = TRANSPORT_NONE;

	//........................................................................
	// Set listening state flag to off
	//........................................................................
	Listening = 0;

	//........................................................................
	// Set max packet sizes, for allocating the channels
	//........................................................................
	Glb_MaxPacketLen = glb_maxlen;
	Glb_NumPackets = glb_num_packets;
	Pvt_MaxPacketLen = pvt_maxlen;
	Pvt_NumPackets = pvt_num_packets;

	//........................................................................
	// Save the app's product ID
	//........................................................................
	ProductID = product_id;

	//------------------------------------------------------------------------
	// Get the user's IPX local connection number
	//------------------------------------------------------------------------
	ConnectionNum = 0;

	//------------------------------------------------------------------------
	// Init connection states
	//------------------------------------------------------------------------
	NumConnections = 0;
	CurConnection = 0;
	for (i = 0; i < CONNECT_MAX; i++) {
		Connection[i] = 0;
	}
	GlobalChannel = 0;

	SendOverflows = 0;
	ReceiveOverflows = 0;
	ReceiveDiscards = 0;
	BadConnection = CONNECTION_NONE;

	//------------------------------------------------------------------------
	// Init timing parameters
	//------------------------------------------------------------------------
	RetryDelta = 2;     // 2 ticks between retries
	MaxRetries = -1;    // disregard # retries
	Timeout = 60;       // report bad connection after 1 second

}	/* end of IPXManagerClass */


/***************************************************************************
 * IPXManagerClass::~IPXManagerClass -- class destructor                   *
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
IPXManagerClass::~IPXManagerClass(void)
{
	int i;

	//------------------------------------------------------------------------
	// Stop all IPX events
	//------------------------------------------------------------------------
	if (Listening) {
		IPXConnClass::Stop_Listening();
		Listening = 0;
	}

	//------------------------------------------------------------------------
	// Free all protected-mode memory
	//------------------------------------------------------------------------
	if (GlobalChannel) {
		delete GlobalChannel;
		GlobalChannel = 0;
	}
	for (i = 0; i < NumConnections; i++) {
		delete Connection[i];
		Connection[i] = 0;
	}
	NumConnections = 0;
}	/* end of ~IPXManagerClass */


/// <summary>
/// Sets the manager up to play over the local network.
/// Games are found by broadcasting onto every network the machine is attached to, and
/// the players answering are recorded at the address their packets came from.
/// </summary>
/// <param name="port">UDP port to play over, or zero to take the default one.</param>
void IPXManagerClass::Configure_LAN(unsigned short port)
{
	Shutdown();

	if (port == 0) {
		port = static_cast<unsigned short>(WestwoodOnline_PortNumber);
	}

	UDPInterfaceClass *udp = new UDPInterfaceClass;
	udp->Set_Local_Port(port);
	udp->Set_Destination_Port(port);
	udp->Enable_Broadcast(true);

	PacketTransport = udp;
	TransportMode = TRANSPORT_LAN;
}


/// <summary>
/// Sets the manager up to play against addresses a lobby outside the game supplied.
/// Every other player is already known, so nothing has to be discovered here; the
/// transport is left with its default port and no broadcasting.
/// </summary>
void IPXManagerClass::Configure_Direct()
{
	Shutdown();

	PacketTransport = new UDPInterfaceClass;
	TransportMode = TRANSPORT_DIRECT;
}


/// <summary>
/// Sets the manager up to play through a CnCNet tunnel server, for players who have no
/// route to each other. Every player, including us, is known by a tunnel ID instead of an
/// address, so there is nothing to discover and nothing to relearn.
/// </summary>
/// <param name="local_id">The ID the tunnel server knows us by.</param>
/// <param name="tunnel_ip">Address of the tunnel server, in network order.</param>
/// <param name="tunnel_port">Port of the tunnel server, in network order.</param>
void IPXManagerClass::Configure_Tunnel(unsigned short local_id, unsigned long tunnel_ip, unsigned short tunnel_port)
{
	Shutdown();

	UDPInterfaceClass *udp = new UDPInterfaceClass;

	// The tunnel server replies to whatever port we send from, so any will do.
	udp->Set_Local_Port(0);
	udp->Configure_Tunnel(local_id, tunnel_ip, tunnel_port);

	PacketTransport = udp;
	TransportMode = TRANSPORT_TUNNEL;
}


/// <summary>
/// Sets the manager up to play straight to a known set of players, each at an address it
/// was given rather than one discovered by asking the network who is out there.
/// </summary>
/// <param name="listen_port">Port to play over, in host order.</param>
void IPXManagerClass::Configure_Direct_Peers(unsigned short listen_port)
{
	Shutdown();

	UDPInterfaceClass *udp = new UDPInterfaceClass;
	udp->Set_Local_Port(listen_port);
	udp->Set_Destination_Port(listen_port);

	PacketTransport = udp;
	TransportMode = TRANSPORT_DIRECT_PEERS;
}


/// <summary>
/// Names a player the game is to reach without discovering them first. The address is
/// what a broadcast fans out to, in place of the network-wide one a LAN game uses.
/// </summary>
/// <param name="address">The player's address, or their tunnel ID when tunnelling.</param>
void IPXManagerClass::Add_Peer(const IPXAddressClass &address)
{
	if (PacketTransport) {
		PacketTransport->Set_Broadcast_Address(address);
	}
}


/// <summary>
/// Takes the network back down.
/// This stops the listening, throws away the channels and disposes of the transport, so
/// that a later Configure call starts from nothing. It is safe to call at any time.
/// </summary>
void IPXManagerClass::Shutdown()
{
	if (Listening) {
		IPXConnClass::Stop_Listening();
		Listening = false;
	}

	delete GlobalChannel;
	GlobalChannel = nullptr;

	for (int i = 0; i < NumConnections; i++) {
		delete Connection[i];
		Connection[i] = nullptr;
	}
	NumConnections = 0;

	delete PacketTransport;
	PacketTransport = nullptr;

	TransportMode = TRANSPORT_NONE;
}


/***************************************************************************
 * IPXManagerClass::Init -- initialization routine                         *
 *                                                                         *
 * This routine allocates memory, & initializes variables                  *
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
int IPXManagerClass::Init(void)
{
	int i;

	//------------------------------------------------------------------------
	// Stop Listening
	//------------------------------------------------------------------------
	if (Listening) {
		IPXConnClass::Stop_Listening();
		Listening = 0;
	}

	//------------------------------------------------------------------------
	// Free the channels left over from any previous session
	//------------------------------------------------------------------------
	if (GlobalChannel) {
		delete GlobalChannel;
		GlobalChannel = 0;
	}
	for (i = 0; i < NumConnections; i++) {
		delete Connection[i];
		Connection[i] = 0;
	}
	NumConnections = 0;

	//------------------------------------------------------------------------
	// Allocate the Global Channel
	//------------------------------------------------------------------------
	GlobalChannel = new IPXGlobalConnClass (Glb_NumPackets, Glb_NumPackets * 2,
		Glb_MaxPacketLen, ProductID);
	if (!GlobalChannel) {
		return(0);
	}
	GlobalChannel->Init();
	GlobalChannel->Set_Retry_Delta (RetryDelta);
	GlobalChannel->Set_Max_Retries (MaxRetries);
	GlobalChannel->Set_TimeOut (Timeout);

	//------------------------------------------------------------------------
	// Configure the connections
	//------------------------------------------------------------------------
	IPXConnClass::Configure(ConnectionNum);

	if (PacketTransport == NULL) {
		return(0);
	}

	//------------------------------------------------------------------------
	// Start Listening
	//------------------------------------------------------------------------
	if (!IPXConnClass::Start_Listening()) return(0);
	Listening = 1;

	return(1);

}	/* end of Init */


/***************************************************************************
 * IPXManagerClass::Set_Timing -- sets timing for all connections          *
 *                                                                         *
 * This will set the timing parameters for all existing connections, and   *
 * all connections created from now on.  This allows an application to     *
 * measure the Response_Time while running, and adjust timing accordingly. *
 *                                                                         *
 * INPUT:                                                                  *
 *      retrydelta   value to set for retry delta                          *
 *      maxretries   value to set for max # retries                        *
 *      timeout      value to set for connection timeout                   *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      none.                                                              *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   07/02/1995 BR : Created.                                              *
 *=========================================================================*/
void IPXManagerClass::Set_Timing (unsigned int retrydelta,
	unsigned int maxretries, unsigned int timeout, bool set_external)
{
	int i;

	RetryDelta = retrydelta;
	MaxRetries = maxretries;
	Timeout = timeout;

	DebugString("IPX Manager: RetryDelta = %d\n", retrydelta);
	DebugString("MaxAhead is %d\n", Session.MaxAhead);

	if (set_external) {
		Set_External_Timing(RetryDelta, MaxRetries, Timeout);
	}

	for (i = 0; i < NumConnections; i++) {
		Connection[i]->Set_Retry_Delta (RetryDelta);
		Connection[i]->Set_Max_Retries (MaxRetries);
		Connection[i]->Set_TimeOut (Timeout);
	}

}	/* end of Set_Timing */


/// <summary>
/// Sets the timing for the global channel.
/// This routine adjusts the retry and timeout behavior of the global (broadcast) channel
/// alone, leaving the private connections as they were. The lobby uses this routine to
/// pace its own chatter independently of the game connections.
/// </summary>
/// <param name="retrydelta">Value to set for the retry delta.</param>
/// <param name="maxretries">Value to set for the maximum number of retries.</param>
/// <param name="timeout">Value to set for the channel timeout.</param>
void IPXManagerClass::Set_External_Timing (unsigned int retrydelta,
	unsigned int maxretries, unsigned int timeout)
{
	if (GlobalChannel) {
		GlobalChannel->Set_Retry_Delta (retrydelta);
		GlobalChannel->Set_Max_Retries (maxretries);
		GlobalChannel->Set_TimeOut (timeout);
	}
}


/***************************************************************************
 * IPXManagerClass::Create_Connection -- creates a new connection          *
 *                                                                         *
 * INPUT:                                                                  *
 *      id            application-specific numerical ID for this connection*
 *      node         ptr to IPXNodeIDType (name & address)                 *
 *      address      IPX address for this connection                       *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      1 = OK, 0 = error                                                  *
 *                                                                         *
 * WARNINGS:                                                               *
 *      Never create a connection with an 'id' of -1.                      *
 *                                                                         *
 * HISTORY:                                                                *
 *   12/20/1994 BR : Created.                                              *
 *=========================================================================*/
int IPXManagerClass::Create_Connection(int id, char *name,
	IPXAddressClass *address)
{
	//------------------------------------------------------------------------
	// Error if no more room
	//------------------------------------------------------------------------
	if (NumConnections==CONNECT_MAX) {
		return(0);
	}

	//------------------------------------------------------------------------
	// Create new connection
	//------------------------------------------------------------------------
	Connection[NumConnections] = new IPXConnClass(Pvt_NumPackets,
		Pvt_NumPackets, Pvt_MaxPacketLen, ProductID, address, id, name);
	if (!Connection[NumConnections]) {
		return(0);
	}

	Connection[NumConnections]->Init ();
	Connection[NumConnections]->Set_Retry_Delta (RetryDelta);
	Connection[NumConnections]->Set_Max_Retries (MaxRetries);
	Connection[NumConnections]->Set_TimeOut (Timeout);

	NumConnections++;

	return(1);

}	/* end of Create_Connection */


/***************************************************************************
 * IPXManagerClass::Delete_Connection -- deletes a connection              *
 *                                                                         *
 * INPUT:                                                                  *
 *      id      ID of connection to delete                                 *
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
int IPXManagerClass::Delete_Connection(int id)
{
	int i,j;

	//------------------------------------------------------------------------
	// Error if no connections to delete
	//------------------------------------------------------------------------
	if (NumConnections==0) {
		return(0);
	}

	//------------------------------------------------------------------------
	// Loop through all connections
	//------------------------------------------------------------------------
	for (i = 0; i < NumConnections; i++) {
		//.....................................................................
		// If a match, delete it
		//.....................................................................
		if (Connection[i]->ID==id) {
			delete Connection[i];

			//..................................................................
			// Move array elements back one index
			//..................................................................
			for (j = i; j < NumConnections - 1; j++) {
				Connection[j] = Connection[j+1];
			}

			//..................................................................
			// Adjust counters
			//..................................................................
			NumConnections--;
			if (CurConnection >= NumConnections)
				CurConnection = 0;
			return(1);
		}
	}

	//------------------------------------------------------------------------
	//	No match; error
	//------------------------------------------------------------------------
	return(0);

}	/* end of Delete_Connection */


/***************************************************************************
 * IPXManagerClass::Num_Connections -- gets the # of connections           *
 *                                                                         *
 * INPUT:                                                                  *
 *      none.                                                              *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      # of connections                                                   *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   01/25/1995 BR : Created.                                              *
 *=========================================================================*/
int IPXManagerClass::Num_Connections(void)
{
	return(NumConnections);

}	/* end of Num_Connections */


/***************************************************************************
 * IPXManagerClass::Connection_ID -- gets the given connection's ID        *
 *                                                                         *
 * INPUT:                                                                  *
 *      index         index of connection to retrieve                      *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      ID for that connection, CONNECTION_NONE if invalid index           *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   01/25/1995 BR : Created.                                              *
 *=========================================================================*/
int IPXManagerClass::Connection_ID(int index)
{
	if (index >= 0 && index < NumConnections) {
		return(Connection[index]->ID);
	}
	else {
		return(CONNECTION_NONE);
	}
}	/* end of Connection_ID */


/***************************************************************************
 * IPXManagerClass::Connection_Name -- retrieves name for given connection *
 *                                                                         *
 * INPUT:                                                                  *
 *      id      ID of connection to get name of                            *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      ptr to connection's name, NULL if not found                        *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   01/19/1995 BR : Created.                                              *
 *=========================================================================*/
char *IPXManagerClass::Connection_Name(int id)
{
	int i;

	for (i = 0; i < NumConnections; i++) {
		if (Connection[i]->ID==id) {
			return(Connection[i]->Name);
		}
	}

	return(NULL);

}	/* end of Connection_Name */


/***************************************************************************
 * IPXManagerClass::Connection_Address -- retrieves connection's address   *
 *                                                                         *
 * INPUT:                                                                  *
 *      id      ID of connection to get address for                        *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      pointer to IXPAddressClass, NULL if not found                      *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   01/19/1995 BR : Created.                                              *
 *=========================================================================*/
IPXAddressClass * IPXManagerClass::Connection_Address(int id)
{
	int i;

	for (i = 0; i < NumConnections; i++) {
		if (Connection[i]->ID==id) {
			return(&Connection[i]->Address);
		}
	}

	return(NULL);

}	/* end of Connection_Address */


/***************************************************************************
 * IPXManagerClass::Connection_Index -- gets given connection's index      *
 *                                                                         *
 * INPUT:                                                                  *
 *      ID to retrieve index for                                           *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      index for this connection, CONNECTION_NONE if not found            *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   01/25/1995 BR : Created.                                              *
 *=========================================================================*/
int IPXManagerClass::Connection_Index(int id)
{
	int i;

	for (i = 0; i < NumConnections; i++) {
		if (Connection[i]->ID==id) {
			return(i);
		}
	}

	return(CONNECTION_NONE);

}	/* end of Connection_Index */


/***************************************************************************
 * IPXManagerClass::Set_Connection_Parms -- sets connection's name & id    *
 *                                                                         *
 * INPUT:                                                                  *
 *      index      connection index                                        *
 *      id         new connection ID                                       *
 *      name      new connection name                                      *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      none.                                                              *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   01/25/1995 BR : Created.                                              *
 *=========================================================================*/
void IPXManagerClass::Set_Connection_Parms(int index, int id, char *name)
{
	if (index >= NumConnections)
		return;

	Connection[index]->ID = id;
	strcpy(Connection[index]->Name,name);

}	/* end of Set_Connection_Parms */


/***************************************************************************
 * IPXManagerClass::Send_Global_Message -- sends a Global Message          *
 *                                                                         *
 * INPUT:                                                                  *
 *      buf         buffer to send                                         *
 *      buflen      length of buf                                          *
 *      ack_req      1 = ACK required; 0 = no ACK required                 *
 *      address      address to send to; NULL = broadcast                  *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      1 = OK, 0 = error                                                  *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   01/25/1995 BR : Created.                                              *
 *=========================================================================*/
int IPXManagerClass::Send_Global_Message(void *buf, int buflen,
	int ack_req, IPXAddressClass *address)
{
	int rc;

	//------------------------------------------------------------------------
	// Error if IPX not installed or not Listening
	//------------------------------------------------------------------------
	if (!Listening) return(0);

	if (ack_req != 0 && (address == NULL || address->Is_Broadcast())) {
		ack_req = 0;
	}

	rc = GlobalChannel->Send_Packet (buf, buflen, address, ack_req);
	if (!rc) {
		SendOverflows++;
		DebugString("Send overflow %d\n", SendOverflows);
	}

	return(rc);

}	/* end of Send_Global_Message */


/***************************************************************************
 * IPXManagerClass::Get_Global_Message -- polls the Global Message queue   *
 *                                                                         *
 * INPUT:                                                                  *
 *      buf            buffer to store received packet                     *
 *      buflen         length of data placed in 'buf'                      *
 *      address         IPX address of sender                              *
 *      product_id      product ID of sender                               *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      1 = OK, 0 = error                                                  *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   01/25/1995 BR : Created.                                              *
 *=========================================================================*/
int IPXManagerClass::Get_Global_Message(void *buf, int capacity, int *buflen,
	IPXAddressClass *address, unsigned short *product_id)
{
	//------------------------------------------------------------------------
	// Error if IPX not installed or not Listening
	//------------------------------------------------------------------------
	if (!Listening) return(0);

	return(GlobalChannel->Get_Packet (buf, capacity, buflen, address, product_id));

}	/* end of Get_Global_Message */


/***************************************************************************
 * IPXManagerClass::Send_Private_Message -- Sends a Private Message        *
 *                                                                         *
 * INPUT:                                                                  *
 *      buf         buffer to send                                         *
 *      buflen      length of 'buf'                                        *
 *      conn_id      connection ID to send to (CONNECTION_NONE = all)      *
 *      ack_req      1 = ACK required; 0 = no ACK required                 *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      1 = OK, 0 = error                                                  *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   01/25/1995 BR : Created.                                              *
 *=========================================================================*/
int IPXManagerClass::Send_Private_Message(void *buf, int buflen, int ack_req,
	int conn_id)
{
	int i;						// loop counter
	int connect_idx;			// index of channel to send to, if specified

	//------------------------------------------------------------------------
	// Error if IPX not installed or not Listening
	//------------------------------------------------------------------------

	if (!Listening || (NumConnections==0)) {
		return(0);
	}

	//------------------------------------------------------------------------
	// Send the message to all connections
	//------------------------------------------------------------------------
	if (conn_id==CONNECTION_NONE) {
		//.....................................................................
		// Check for room in all connections
		//.....................................................................
		for (i = 0; i < NumConnections; i++) {
			if (Connection[i]->Queue->Num_Send() ==
				Connection[i]->Queue->Max_Send()) {
				SendOverflows++;
				return(0);
			}
		}

		//.....................................................................
		// Send packet to all connections
		//.....................................................................
		for (i = 0; i < NumConnections; i++) {
			Connection[i]->Send_Packet (buf, buflen, ack_req);
		}
		return(1);
	}

	//------------------------------------------------------------------------
	// Send the message to the specified connection
	//------------------------------------------------------------------------
	else {
		connect_idx = Connection_Index (conn_id);
		if (connect_idx == CONNECTION_NONE) {
			SendOverflows++;
			return(0);
		}

		//.....................................................................
		// Check for room in the connection
		//.....................................................................
		if (Connection[connect_idx]->Queue->Num_Send() ==
			Connection[connect_idx]->Queue->Max_Send()) {
			SendOverflows++;
			return(0);
		}

		//.....................................................................
		// Send the packet to that connection
		//.....................................................................
		Connection[connect_idx]->Send_Packet (buf, buflen, ack_req);
		return(1);
	}

}	/* end of Send_Private_Message */


/***************************************************************************
 * IPXManagerClass::Get_Private_Message -- Polls the Private Message queue *
 *                                                                         *
 * INPUT:                                                                  *
 *      buf         buffer to store incoming packet                        *
 *      buflen      length of data placed in 'buf'                         *
 *      conn_id      filled in with connection ID of sender                *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      1 = OK, 0 = error                                                  *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   01/25/1995 BR : Created.                                              *
 *=========================================================================*/
int IPXManagerClass::Get_Private_Message(void *buf, int capacity, int *buflen, int *conn_id)
{
	int i;
	int rc;
	int c_id;

	//------------------------------------------------------------------------
	// Error if IPX not installed or not Listening
	//------------------------------------------------------------------------
	if (!Listening || (NumConnections==0)) {
		return(0);
	}

	//------------------------------------------------------------------------
	// Safety check: ensure CurConnection is in range.
	//------------------------------------------------------------------------
	if (CurConnection >= NumConnections) {
		CurConnection = 0;
	}

	//------------------------------------------------------------------------
	// Scan all connections for a received packet, starting with 'CurConnection'
	//------------------------------------------------------------------------
	for (i = 0; i < NumConnections; i++) {

		//.....................................................................
		// Check this connection for a packet
		//.....................................................................
		rc = Connection[CurConnection]->Get_Packet (buf, capacity, buflen);
		c_id = Connection[CurConnection]->ID;

		//.....................................................................
		// Increment CurConnection to the next connection index
		//.....................................................................
		CurConnection++;
		if (CurConnection >= NumConnections) {
			CurConnection = 0;
		}

		//.....................................................................
		// If we got a packet, return the connection ID
		//.....................................................................
		if (rc) {
			(*conn_id) = c_id;
			return(1);
		}
	}

	return(0);

}	/* end of Get_Private_Message */


/***************************************************************************
 * IPXManagerClass::Service -- main polling routine for IPX Connections    *
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
 *   01/25/1995 BR : Created.                                              *
 *=========================================================================*/
int IPXManagerClass::Service(void)
{
	int rc = 1;
	int i;
	CommHeaderType packet_header;
	CommHeaderType *packet;
	int packetlen;
	IPXAddressClass address;

	unsigned char temp_receive_buffer[1024];
	int	temp_receive_buffer_len;
	int	temp_address_len;


	char temp_address [128];

	if ( PacketTransport ) {

		do {
			temp_receive_buffer_len = sizeof (temp_receive_buffer);
			temp_address_len = sizeof (temp_address);
			packetlen = PacketTransport->Read ( temp_receive_buffer, temp_receive_buffer_len, temp_address, temp_address_len );
			if ( packetlen ) {
				if (packetlen < (int)sizeof(unsigned short)) {
					ReceiveDiscards++;
					if (ReceiveDiscards == 1 || (ReceiveDiscards & (ReceiveDiscards - 1)) == 0) {
						DebugString("Network packet drop [manager-short-magic]: %d\n", ReceiveDiscards);
					}
					continue;
				}
				memcpy(&address, temp_address, sizeof(address));

				memset(&packet_header, 0, sizeof(packet_header));
				memcpy(&packet_header, temp_receive_buffer, std::min(packetlen, (int)sizeof(packet_header)));
				packet = &packet_header;
				if (packet->MagicNumber == GlobalChannel->Magic_Num()) {

					/*
					**	Put the packet in the Global Queue
					*/
					if (!GlobalChannel->Receive_Packet (temp_receive_buffer, packetlen, &address)) {
						ReceiveOverflows++;
						DebugString("GlobalChannel recive buffer overflow %d\n", ReceiveOverflows);
						break;
					}
				} else {
					if (packet->MagicNumber == ProductID) {

						/*
						**	Find the Private Queue that this packet is for
						*/
						bool found_address = false;
						for (i = 0; i < NumConnections; i++) {
							if (Connection[i]->Address == address) {
								found_address = true;
								break;
							}
						}
						if (found_address) {
							if (!Connection[i]->Receive_Packet (temp_receive_buffer, packetlen)) {
								ReceiveOverflows++;
								DebugString("Recive buffer overflow %d\n", ReceiveOverflows);
								packetlen = 0;
								break;
							}
						}

						if (!found_address) {
							// A tunnel ID names a player rather than a place, so it cannot
							// go stale the way an address a player moved away from can.
							if ( TransportMode != TRANSPORT_TUNNEL && !ScenarioInit)
							{
								/*
								**	This packet came from an unknown source. If it looks like one of our players
								**	packets then it might be from a player whos IP has changed.
								*/
								int frame_info_size = sizeof(CommHeaderType) + offsetof(EventClass, Data) + size_of(EventClass, Data.FrameInfo);
								if (Frame > 8 && packetlen >= frame_info_size) {
									if (packet->Code == ConnectionClass::PACKET_DATA_NOACK){
										/*
										**	Magic number and packet code are valid. It's probably a C&C packet.
										*/
										unsigned char event_type;
										int id;
										memcpy(&event_type, temp_receive_buffer + sizeof(CommHeaderType), sizeof(event_type));
										memcpy(&id, temp_receive_buffer + sizeof(CommHeaderType) + offsetof(EventClass, ID), sizeof(id));

										/*
										**	If this is a framesync packet then grab the address and match it to an existing player.
										*/
										if (event_type == EventClass::FRAMESYNC) {
											assert (id != PlayerPtr->ID);
											for ( int i=1 ; i<Session.Players.Count() ; i++) {
												if (Session.Players[i]->Player.ID == id) {

													Session.Players[i]->Address = address;

													if ( Connection_Index(id) != CONNECTION_NONE ) //	(else Create_Connections() has not yet been called)
													{
														/*
														**	Found a likely candidate. Update his address. It should be OK to drop this
														**	packet since it's a framesync packet and will will pick up the next one.
														*/
														Connection[Connection_Index(id)]->Address = address;
													}
													break;
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}

		} while (packetlen);

	}

	//------------------------------------------------------------------------
	// Service all connections.  If a connection reports that it's gone "bad",
	// report an error to the caller.  If it's the Global Channel, un-queue the
	// send entry that's holding things up.  This will keep the Global Channel
	// from being clogged by one un-ACK'd outgoing packet.
	//------------------------------------------------------------------------
	if (GlobalChannel) {
		if (!GlobalChannel->Service()) {
			GlobalChannel->Discard_Undeliverable_Packets();
			rc = 0;
		}
	}
	for (i = 0; i < NumConnections; i++) {
		if (!Connection[i]->Service()) {
			rc = 0;
			BadConnection = Connection[i]->ID;
			DebugString("Error - Connection %d has gone bad\n", BadConnection);
		}
	}

	if (rc) {
		BadConnection = CONNECTION_NONE;
	}

	return(rc);

}	/* end of Service */

/***************************************************************************
 * IPXManagerClass::Get_Bad_Connection -- returns bad connection ID        *
 *                                                                         *
 * INPUT:                                                                  *
 *      none.                                                              *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      ID of bad connection; CONNECTION_NONE if none.                     *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   05/04/1995 BRR : Created.                                             *
 *=========================================================================*/
int IPXManagerClass::Get_Bad_Connection(void)
{
	return(BadConnection);

}	/* end of Get_Bad_Connection */


/***************************************************************************
 * IPXManagerClass::Global_Num_Send   -- reports # entries in send queue   *
 *                                                                         *
 * INPUT:                                                                  *
 *      none.                                                              *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      # entries in the Global Send Queue                                 *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   01/25/1995 BR : Created.                                              *
 *=========================================================================*/
int IPXManagerClass::Global_Num_Send(void)
{
	//------------------------------------------------------------------------
	// Error if IPX not installed or not Listening
	//------------------------------------------------------------------------
	if (!Listening) {
		return(0);
	}

	return(GlobalChannel->Queue->Num_Send());

}	/* end of Global_Num_Send */


/***************************************************************************
 * IPXManagerClass::Global_Num_Receive -- reports # entries in recv queue  *
 *                                                                         *
 * INPUT:                                                                  *
 *      none.                                                              *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      # entries in the Global Receive Queue                              *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   01/25/1995 BR : Created.                                              *
 *=========================================================================*/
int IPXManagerClass::Global_Num_Receive(void)
{
	//------------------------------------------------------------------------
	// Error if IPX not installed or not Listening
	//------------------------------------------------------------------------
	if (!Listening) {
		return(0);
	}

	return(GlobalChannel->Queue->Num_Receive());

}	/* end of Global_Num_Receive */


/***************************************************************************
 * IPXManagerClass::Private_Num_Send -- reports # entries in send queue    *
 *                                                                         *
 * INPUT:                                                                  *
 *      # entries in the Private Send Queue                                *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      1 = OK, 0 = error                                                  *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   01/25/1995 BR : Created.                                              *
 *=========================================================================*/
int IPXManagerClass::Private_Num_Send(int id)
{
	int i;
	int maxnum;

	//------------------------------------------------------------------------
	// Error if IPX not installed or not Listening
	//------------------------------------------------------------------------
	if (!Listening || (NumConnections==0)) {
		return(0);
	}

	//------------------------------------------------------------------------
	//	If connection ID specified, return that connection's # of packets
	//------------------------------------------------------------------------
	if (id != CONNECTION_NONE) {
		i = Connection_Index(id);
		if (i != CONNECTION_NONE) {
			return(Connection[i]->Queue->Num_Send());
		}
		else {
			return(0);
		}

	}

	//------------------------------------------------------------------------
	//	Otherwise, return the max # of all connections
	//------------------------------------------------------------------------
	else {
		maxnum = 0;
		for (i = 0; i < NumConnections; i++) {
			if (Connection[i]->Queue->Num_Send() > maxnum) {
				maxnum = Connection[i]->Queue->Num_Send();
			}
		}
		return(maxnum);
	}

}	/* end of Private_Num_Send */


/***************************************************************************
 * IPXManagerClass::Private_Num_Receive -- reports # entries in recv queue *
 *                                                                         *
 * INPUT:                                                                  *
 *      id      ID of connection to query                                  *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      # entries in the Private Receive Queue                             *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   01/25/1995 BR : Created.                                              *
 *=========================================================================*/
int IPXManagerClass::Private_Num_Receive(int id)
{
	int i;
	int maxnum;

	//------------------------------------------------------------------------
	// Error if IPX not installed or not Listening
	//------------------------------------------------------------------------
	if (!Listening || (NumConnections==0))
		return(0);

	//------------------------------------------------------------------------
	//	If connection ID specified, return that connection's # of packets
	//------------------------------------------------------------------------
	if (id != CONNECTION_NONE) {
		i = Connection_Index(id);
		if (i != CONNECTION_NONE) {
			return(Connection[i]->Queue->Num_Receive());
		}
		else {
			return(0);
		}

	}

	//------------------------------------------------------------------------
	//	Otherwise, return the max # of all connections
	//------------------------------------------------------------------------
	else {
		maxnum = 0;
		for (i = 0; i < NumConnections; i++) {
			if (Connection[i]->Queue->Num_Receive() > maxnum) {
				maxnum = Connection[i]->Queue->Num_Receive();
			}
		}
		return(maxnum);
	}

}	/* end of Private_Num_Receive */


/***************************************************************************
 * IPXManagerClass::Response_Time -- Returns largest Avg Response Time     *
 *                                                                         *
 * INPUT:                                                                  *
 *      none.                                                              *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      largest avg response time                                          *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   05/04/1995 BRR : Created.                                             *
 *=========================================================================*/
unsigned int IPXManagerClass::Response_Time(void)
{
	unsigned int resp;
	unsigned int maxresp = 0;
	int i;

	for (i = 0; i < NumConnections; i++) {
		resp = Connection[i]->Queue->Avg_Response_Time();
		if (resp > maxresp) {
			maxresp = resp;
		}
	}

	return(maxresp);

}	/* end of Response_Time */


/// <summary>Returns the worst measured round trip among active private links.</summary>
std::optional<NetTiming::Milliseconds> IPXManagerClass::Worst_Local_Round_Trip_MS(void) const
{
	NetTiming::Milliseconds worst = 0;
	std::array<bool, NetTiming::MAX_TIMING_PLAYERS> connected = {};
	for (int i = 0; i < NumConnections; i++) {
		int const id = Connection[i]->ID;
		if (!Session.Is_Network_Timing_Player_Active(id)) {
			continue;
		}
		connected[id] = true;

		std::optional<NetTiming::Milliseconds> const round_trip = Connection[i]->Smoothed_Round_Trip_MS();
		if (!round_trip) {
			return(std::nullopt);
		}
		worst = std::max(worst, *round_trip);
	}

	int const local_id = PlayerPtr != NULL ? PlayerPtr->HeapID : -1;
	for (unsigned int id = 0; id < connected.size(); id++) {
		if (static_cast<int>(id) != local_id && Session.Is_Network_Timing_Player_Active(id) && !connected[id]) {
			return(std::nullopt);
		}
	}

	return(worst);
}


/// <summary>
/// Fetches the average response time of a single connection.
/// This routine is used by the network queue logic to pace itself against the slowest
/// player, and by the debug display.
/// </summary>
/// <param name="index">Index of the connection to examine.</param>
/// <returns>Returns with the average round trip time, expressed in game timer ticks.</returns>
unsigned int IPXManagerClass::Avg_Response_Time(int index)
{
	return(Connection[index]->Queue->Avg_Response_Time());
}


/***************************************************************************
 * IPXManagerClass::Global_Response_Time -- Returns Avg Response Time      *
 *                                                                         *
 * INPUT:                                                                  *
 *      none.                                                              *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      avg global channel response time                                   *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   05/04/1995 BRR : Created.                                             *
 *=========================================================================*/
unsigned int IPXManagerClass::Global_Response_Time(void)
{
	if (GlobalChannel) {
		return(GlobalChannel->Queue->Avg_Response_Time());
	}
	else {
		return(0);
	}

}	/* end of Global_Response_Time */


/// <summary>
/// Records the connection statistics for the remote players.
/// This routine folds each connection's round trip, resend and packet loss figures into
/// the session statistics, so that the end of game screen can report on how well the
/// network behaved. The local player's own connection is passed over.
/// </summary>
void IPXManagerClass::Store_Stats(void)
{
	for (int i = 0; i < NumConnections; i++) {
		HouseClass * house = Houses[Connection[i]->ID];

		if (house != NULL && house != PlayerPtr) {
			MPStatsType & stats = Session.ConnectionStats[i];
			if (stats.Name[0] == '\0') {
				strcpy(stats.Name, Connection[i]->Name);
				stats.Address = Connection[i]->Address;
			}

			int maxtime = Connection[i]->Queue->Max_Response_Time();
			stats.MaxRoundTrip = std::max(Session.ConnectionStats[i].MaxRoundTrip, 1000 * maxtime / TIMER_SECOND);

			unsigned int avgtime = Connection[i]->Queue->Avg_Response_Time();
			stats.MaxAvgRoundTrip = std::max(int(1000 * avgtime / TIMER_SECOND), Session.ConnectionStats[i].MaxAvgRoundTrip);

			stats.Resends = Connection[i]->Num_Resends();
			stats.Lost = Connection[i]->Num_Lost();
			stats.PercentLost = Connection[i]->Percent_Lost();
		}
	}
}


/// <summary>
/// Draws the network diagnostics display.
/// This routine prints the retry and timeout settings currently in force, followed by a
/// column of round trip, resend and packet loss figures for every remote player in the
/// game. Use this routine when the multiplayer debug display has been switched on.
/// </summary>
void IPXManagerClass::Multiplayer_Debug_Print(void)
{
	char buffer[256];

	sprintf(buffer, "Rtr delta : %d", 1000 * RetryDelta / TIMER_SECOND);
	Fancy_Text_Print(buffer, *VisibleSurface, VisibleSurface->Get_Rect(), Point2D(0, 450), Fetch_Scheme_By_Name("Grey"), TBLACK, TextPrintType(TPF_NOSHADOW|TPF_EFNT));

	sprintf(buffer, "Rtr timeout : %d", 1000 * Timeout / TIMER_SECOND);
	Fancy_Text_Print(buffer, *VisibleSurface, VisibleSurface->Get_Rect(), Point2D(0, 458), Fetch_Scheme_By_Name("Grey"), 0, TextPrintType(TPF_NOSHADOW|TPF_EFNT));

	sprintf(buffer, "Lat Fudge : %d", Session.LatencyFudge);
	Fancy_Text_Print(buffer, *VisibleSurface, VisibleSurface->Get_Rect(), Point2D(0, 466), Fetch_Scheme_By_Name("Grey"), TBLACK, TextPrintType(TPF_NOSHADOW|TPF_EFNT));

	if (SentFrameSyncTimer / TIMER_SECOND) {
		sprintf(buffer, "FSPS : %d", SentFrameSyncCount / (SentFrameSyncTimer / TIMER_SECOND));
		Fancy_Text_Print(buffer, *VisibleSurface, VisibleSurface->Get_Rect(), Point2D(0, 474), Fetch_Scheme_By_Name("Grey"), TBLACK, TextPrintType(TPF_NOSHADOW|TPF_EFNT));
		if ((Frame & 0x7F) == 0x7F) {
			SentFrameSyncTimer = 0;
			SentFrameSyncCount = 0;
		}
	}

	for (int i = 0; i < NumConnections; i++) {
		HouseClass * house = Houses[Connection[i]->ID];
		if (house != NULL && house != PlayerPtr) {
			int scheme = house->Scheme;

			Fancy_Text_Print(Connection[i]->Name, *VisibleSurface, VisibleSurface->Get_Rect(), Point2D((i + 1) * 100, 402), ColorSchemes[scheme], TBLACK, TextPrintType(TPF_EFNT|TPF_NOSHADOW));

			int avg = Connection[i]->Queue->Avg_Response_Time();
			sprintf(buffer, "Average  : %d", 1000 * avg / TIMER_SECOND);
			Fancy_Text_Print(buffer, *VisibleSurface, VisibleSurface->Get_Rect(), Point2D((i + 1) * 100, 411), ColorSchemes[scheme], TBLACK, TextPrintType(TPF_EFNT|TPF_NOSHADOW));

			int max = Connection[i]->Queue->Max_Response_Time();
			sprintf(buffer, "Max      : %d", 1000 * max / TIMER_SECOND);
			Fancy_Text_Print(buffer, *VisibleSurface, VisibleSurface->Get_Rect(), Point2D((i + 1) * 100, 418), ColorSchemes[scheme], TBLACK, TextPrintType(TPF_EFNT|TPF_NOSHADOW));

			int resends = Connection[i]->Num_Resends();
			sprintf(buffer, "Resends  : %d", resends);
			Fancy_Text_Print(buffer, *VisibleSurface, VisibleSurface->Get_Rect(), Point2D((i + 1) * 100, 425), ColorSchemes[scheme], TBLACK, TextPrintType(TPF_EFNT|TPF_NOSHADOW));

			int numlost = std::max(0, Connection[i]->Num_Lost());
			sprintf(buffer, "Num lost : %d", numlost);
			Fancy_Text_Print(buffer, *VisibleSurface, VisibleSurface->Get_Rect(), Point2D((i + 1) * 100, 432), ColorSchemes[scheme], TBLACK, TextPrintType(TPF_EFNT|TPF_NOSHADOW));

			int pcnt_lost = Connection[i]->Percent_Lost();
			sprintf(buffer, "Pcnt lost: %d", pcnt_lost);
			Fancy_Text_Print(buffer, *VisibleSurface, VisibleSurface->Get_Rect(), Point2D((i + 1) * 100, 439), ColorSchemes[scheme], TBLACK, TextPrintType(TPF_EFNT|TPF_NOSHADOW));

			int process_time = 0;
			for (int j = 0; j < Session.Players.Count(); ++j) {
				if (Connection[i]->ID == Session.Players[j]->Player.ID) {
					process_time = Session.Players[j]->Player.ProcessTime;
					break;
				}
			}
			sprintf(buffer, "Process : %d", process_time);
			Fancy_Text_Print(buffer, *VisibleSurface, VisibleSurface->Get_Rect(), Point2D((i + 1) * 100, 446), ColorSchemes[scheme], TBLACK, TextPrintType(TPF_EFNT|TPF_NOSHADOW));

			sprintf(buffer, "Frame   : %d", -Session.PlayerLatency[i]);
			Fancy_Text_Print(buffer, *VisibleSurface, VisibleSurface->Get_Rect(), Point2D((i + 1) * 100, 453), ColorSchemes[scheme], TBLACK, TextPrintType(TPF_EFNT|TPF_NOSHADOW));

			sprintf(buffer, "Queue s/r: %d/%d", Connection[i]->Queue->Num_Send(), Connection[i]->Queue->Num_Receive());
			Fancy_Text_Print(buffer, *VisibleSurface, VisibleSurface->Get_Rect(), Point2D((i + 1) * 100, 460), ColorSchemes[scheme], TBLACK, TextPrintType(TPF_EFNT|TPF_NOSHADOW));

			sprintf(buffer, "Missed o/m: %d/%d", Connection[i]->Missed_Overall(), Connection[i]->Missed_Magic());
			Fancy_Text_Print(buffer, *VisibleSurface, VisibleSurface->Get_Rect(), Point2D((i + 1) * 100, 467), ColorSchemes[scheme], TBLACK, TextPrintType(TPF_EFNT|TPF_NOSHADOW));
		}
	}
}


/***************************************************************************
 * IPXManagerClass::Reset_Response_Time -- Reset response time             *
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
 *   05/04/1995 BRR : Created.                                             *
 *=========================================================================*/
void IPXManagerClass::Reset_Response_Time(bool zero)
{
	int i;

	for (i = 0; i < NumConnections; i++) {
		Connection[i]->Queue->Reset_Response_Time(zero);
	}

	if (GlobalChannel)
		GlobalChannel->Queue->Reset_Response_Time(true);

}	/* end of Reset_Response_Time */


/***************************************************************************
 * IPXManagerClass::Oldest_Send -- gets ptr to oldest send buf             *
 *                                                                         *
 * INPUT:                                                                  *
 *      none.                                                              *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      buf ptr                                                            *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   05/04/1995 BRR : Created.                                             *
 *=========================================================================*/
void * IPXManagerClass::Oldest_Send(void)
{
	int i,j;
	unsigned int time;
	unsigned int mintime = 0xffffffff;
	SendQueueType *send_entry;				// ptr to send entry header
	CommHeaderType *packet;
	void *buf = NULL;

	for (i = 0; i < NumConnections; i++) {

		send_entry = NULL;

		for (j = 0; j < Connection[i]->Queue->Num_Send(); j++) {
			send_entry = Connection[i]->Queue->Get_Send(j);
			if (send_entry) {
				packet = (CommHeaderType *)send_entry->Buffer;
				if (packet->Code == ConnectionClass::PACKET_DATA_ACK &&
					send_entry->IsACK == 0) {
					break;
				}
				else {
					send_entry = NULL;
				}
			}
		}

		if (send_entry!=NULL) {

			time = send_entry->FirstTime;

			if (time < mintime) {
				mintime = time;
				buf = send_entry->Buffer;
			}
		}
	}

	return(buf);

}	/* end of Oldest_Send */


/***************************************************************************
 * IPXManagerClass::Configure_Debug -- sets up special debug values        *
 *                                                                         *
 * Mono_Debug_Print2() can look into a packet to pull out a particular     *
 * ID, and can print both that ID and a string corresponding to            *
 * that ID.  This routine configures these values so it can find           *
 * and decode the ID.  This ID is used in addition to the normal           *
 * CommHeaderType values.                                                  *
 *                                                                         *
 * INPUT:                                                                  *
 *      index            connection index to configure (-1 = Global Channel)*
 *      type_offset      ID's byte offset into packet                      *
 *      type_size      size of ID, in bytes; 0 if none                     *
 *      names            ptr to array of names; use ID as an index into this*
 *      namestart      numerical value of 1st name in the array            *
 *      namecount      # in the names array; 0 if none.                    *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      none.                                                              *
 *                                                                         *
 * WARNINGS:                                                               *
 *      Names shouldn't be longer than 12 characters.                      *
 *                                                                         *
 * HISTORY:                                                                *
 *   05/31/1995 BRR : Created.                                             *
 *=========================================================================*/
void IPXManagerClass::Configure_Debug(int index, int type_offset,
	int type_size, char **names, int namestart, int namecount)
{
	if (index == -1) {
		GlobalChannel->Queue->Configure_Debug (type_offset, type_size, names,
			namestart, namecount);
	}
	else if (Connection[index]) {
		Connection[index]->Queue->Configure_Debug (type_offset, type_size, names,
			namestart, namecount);
	}

}	/* end of Configure_Debug */


/***************************************************************************
 * IPXManagerClass::Mono_Debug_Print -- debug output routine               *
 *                                                                         *
 * INPUT:                                                                  *
 *      index         index of connection to display (-1 = Global Channel) *
 *      refresh      1 = complete screen refresh                           *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      1 = OK, 0 = error                                                  *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   01/25/1995 BR : Created.                                              *
 *=========================================================================*/
void IPXManagerClass::Mono_Debug_Print(int index, int refresh)
{
#if 0
	char txt[80];
	int i;

	if (index == -1)
		GlobalChannel->Queue->Mono_Debug_Print (refresh);

	else if (Connection[index])
		Connection[index]->Queue->Mono_Debug_Print (refresh);

	if (refresh) {
		Mono_Set_Cursor (20,1);
		Mono_Printf ("IPX Queue:");

		Mono_Set_Cursor (9,2);
		Mono_Printf ("Average Response Time:");

		Mono_Set_Cursor (43,1);
		Mono_Printf ("Send Overflows:");

		Mono_Set_Cursor (40,2);
		Mono_Printf ("Receive Overflows:");

	}

	Mono_Set_Cursor (32,1);
	Mono_Printf ("%d",index);

	Mono_Set_Cursor (32,2);
	if (index == -1) {
		Mono_Printf ("%d  ", GlobalChannel->Queue->Avg_Response_Time());
	}
	else {
		Mono_Printf ("%d  ", Connection[index]->Queue->Avg_Response_Time());
	}

	Mono_Set_Cursor (59,1);
	Mono_Printf ("%d  ", SendOverflows);

	Mono_Set_Cursor (59,2);
	Mono_Printf ("%d  ", ReceiveOverflows);

#else
	index = index;
	refresh = refresh;
#endif

}	/* end of Mono_Debug_Print */

/*************************** end of ipxmgr.cpp *****************************/
