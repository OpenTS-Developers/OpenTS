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

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                     $Archive:: /Sun/WSPUDP.cpp                                             $*
 *                                                                                             *
 *                      $Author:: Joe_b                                                       $*
 *                                                                                             *
 *                     $Modtime:: 8/05/97 6:45p                                               $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 *                                                                                             *
 *  WSProto.CPP WinsockInterfaceClass to provide an interface to Winsock protocols             *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 *                                                                                             *
 * Functions:                                                                                  *
 * UDPInterfaceClass::UDPInterfaceClass -- Class constructor.                                  *
 * UDPInterfaceClass::Set_Broadcast_Address -- Sets the address to send broadcast packets to   *
 * UDPInterfaceClass::Open_Socket -- Opens a socket for communications via the UDP protocol    *
 * TMC::Message_Handler -- Message handler function for Winsock related messages               *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "wspudp.h"

#include "dbgprint.h"
#include "misc.h"
#include "msgloop.h"
#include "netadmit.h"
#include "vector.h"

#include <cstdio>
#include <cstring>
#include <iphlpapi.h>


extern int WestwoodOnline_PortNumber;

// A tunnelled datagram leads with the sender's and the recipient's tunnel IDs, which is
// all the tunnel server reads in order to forward it.
#define TUNNEL_HEADER_SIZE 4

/***********************************************************************************************
 * UDPInterfaceClass::UDPInterfaceClass -- Class constructor.                                  *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *    8/5/97 12:11PM ST : Created                                                              *
 *=============================================================================================*/
UDPInterfaceClass::UDPInterfaceClass (void) :
	BASECLASS(),
	LocalPort(0),
	DestinationPort(0),
	LocalPortSet(false),
	DestinationPortSet(false),
	UseBroadcast(false),
	TunnelID(0),
	TunnelIP(0),
	TunnelPort(0)
{}


/// <summary>
/// Sets the port to listen on.
/// </summary>
/// <param name="port">Port in host order. Zero binds a port of Winsock's choosing.</param>
void UDPInterfaceClass::Set_Local_Port(unsigned short port)
{
	LocalPort = port;
	LocalPortSet = true;
}


/// <summary>
/// Sets the port that outgoing packets are addressed to.
/// </summary>
/// <param name="port">Port in host order.</param>
void UDPInterfaceClass::Set_Destination_Port(unsigned short port)
{
	DestinationPort = port;
	DestinationPortSet = true;
}


/// <summary>
/// Allows this socket to broadcast, so that a game can be found without knowing who is
/// out there. The broadcast addresses themselves are worked out when the socket opens.
/// </summary>
void UDPInterfaceClass::Enable_Broadcast(bool enable)
{
	UseBroadcast = enable;
}


/// <summary>
/// Sends everything by way of a CnCNet tunnel server, for players who have no route to
/// each other. Each player is known only by a tunnel ID from here on.
/// </summary>
/// <param name="local_id">The ID the tunnel server knows us by.</param>
/// <param name="tunnel_ip">Address of the tunnel server.</param>
/// <param name="tunnel_port">Port of the tunnel server. Zero turns tunnelling off.</param>
void UDPInterfaceClass::Configure_Tunnel(unsigned short local_id, unsigned long tunnel_ip, unsigned short tunnel_port)
{
	TunnelID = local_id;
	TunnelIP = tunnel_ip;
	TunnelPort = tunnel_port;
}


/// <summary>
/// Hands a datagram to Winsock, routing it through the tunnel server when one is in use.
/// </summary>
/// <param name="destination">Where the packet is bound. In tunnel mode the port carries
/// the recipient's tunnel ID rather than a real port.</param>
/// <returns>Whatever sendto returned, counting only the payload.</returns>
int UDPInterfaceClass::Send_To(const char *buffer, int buffer_len, sockaddr_in *destination)
{
	if (TunnelPort == 0) {
		return(sendto(Socket, buffer, buffer_len, 0, reinterpret_cast<const sockaddr *>(destination), sizeof(*destination)));
	}

	// The tunnel server routes on the header alone, so it has to lead the datagram,
	// outside the packet's own framing.
	char tunnelled[TUNNEL_HEADER_SIZE + WS_RECEIVE_BUFFER_LEN];
	if (buffer_len > (int)sizeof(tunnelled) - TUNNEL_HEADER_SIZE) {
		return(SOCKET_ERROR);
	}

	unsigned short header[] = { TunnelID, destination->sin_port };
	std::memcpy(tunnelled, header, sizeof(header));
	std::memcpy(tunnelled + TUNNEL_HEADER_SIZE, buffer, buffer_len);

	sockaddr_in server = {};
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = TunnelIP;
	server.sin_port = TunnelPort;

	int rc = sendto(Socket, tunnelled, buffer_len + TUNNEL_HEADER_SIZE, 0, reinterpret_cast<const sockaddr *>(&server), sizeof(server));

	return(rc > 0 ? rc - TUNNEL_HEADER_SIZE : rc);
}


/// <summary>
/// Takes a datagram from Winsock, unwrapping it when a tunnel is in use. A tunnelled
/// packet reports its sender by tunnel ID, since the server is the only endpoint the
/// socket ever sees.
/// </summary>
/// <returns>The number of payload bytes received, or SOCKET_ERROR.</returns>
int UDPInterfaceClass::Receive_From(char *buffer, int buffer_len, sockaddr_in *source)
{
	int address_len = sizeof(*source);

	if (TunnelPort == 0) {
		return(recvfrom(Socket, buffer, buffer_len, 0, reinterpret_cast<sockaddr *>(source), &address_len));
	}

	char tunnelled[TUNNEL_HEADER_SIZE + WS_RECEIVE_BUFFER_LEN];
	int rc = recvfrom(Socket, tunnelled, sizeof(tunnelled), 0, reinterpret_cast<sockaddr *>(source), &address_len);

	if (rc == SOCKET_ERROR) return(SOCKET_ERROR);

	unsigned short header[2];
	if (rc < (int)sizeof(header)) return(SOCKET_ERROR);
	std::memcpy(header, tunnelled, sizeof(header));

	// Anything too short to carry a header, or addressed to somebody else, is not ours.
	if (rc <= TUNNEL_HEADER_SIZE || header[1] != TunnelID) {
		return(SOCKET_ERROR);
	}

	rc -= TUNNEL_HEADER_SIZE;
	if (rc > buffer_len) return(SOCKET_ERROR);

	std::memcpy(buffer, tunnelled + TUNNEL_HEADER_SIZE, rc);

	source->sin_addr.s_addr = 0;
	source->sin_port = header[0];

	return(rc);
}



/***********************************************************************************************
 * UDPIC::~UDPInterfaceClass -- UDPInterface class destructor                                  *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *    10/9/97 12:17PM ST : Created                                                             *
 *=============================================================================================*/
UDPInterfaceClass::~UDPInterfaceClass (void)
{
	Clear_Broadcast_Addresses();

	while ( LocalAddresses.Count() ) {
		delete LocalAddresses[0];
		LocalAddresses.Delete_Index(0);
	}

	Close();
}


/// <summary>
/// Discards every broadcast address the interface knows about.
/// This routine is used to forget the addresses handed over by Set_Broadcast_Address,
/// so that a later broadcast will not reach a stale set of destinations.
/// </summary>
void UDPInterfaceClass::Clear_Broadcast_Addresses(void)
{
	while ( BroadcastAddresses.Count() ) {
		delete BroadcastAddresses[0];
		BroadcastAddresses.Delete_Index(0);
	}
}


/***********************************************************************************************
 * UDPInterfaceClass::Set_Broadcast_Address -- Sets the address to send broadcast packets to   *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Address to add to the broadcast list                                              *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *    8/5/97 12:12PM ST : Created                                                              *
 *=============================================================================================*/
void UDPInterfaceClass::Set_Broadcast_Address (const IPXAddressClass &address)
{
	BroadcastAddresses.Add (new IPXAddressClass(address));
}


/***********************************************************************************************
 * UDPInterfaceClass::Open_Socket -- Opens a socket for communications via the UDP protocol    *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Socket number to use. Not required for this protocol.                             *
 *                                                                                             *
 * OUTPUT:   True if socket was opened OK                                                      *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *    8/5/97 12:13PM ST : Created                                                              *
 *=============================================================================================*/
bool UDPInterfaceClass::Open_Socket ( SOCKET )
{
	LINGER ling;
	struct 	sockaddr_in addr;

	/*
	**	If Winsock is not initialised then do it now.
	*/
	if ( !WinsockInitialised ) {
		if ( !Init()) return( false );;
	}

	DebugString("About to open a UDP socket\n");

	/*
	**	Create our UDP socket
	*/
	Socket = socket(AF_INET, SOCK_DGRAM, 0);
	if (Socket == INVALID_SOCKET) {
		return(false);
	}

	/*
	**	Bind our UDP socket to our UDP port number
	*/
	addr.sin_family = AF_INET;
	addr.sin_port = (unsigned short) htons ( LocalPortSet ? LocalPort : (unsigned short) WestwoodOnline_PortNumber );
	addr.sin_addr.s_addr = htonl (INADDR_ANY);

	DebugString("About to bind the UDP socket to port %d\n", ntohs(addr.sin_port));

	if ( bind (Socket, (LPSOCKADDR)&addr, sizeof(addr) ) == SOCKET_ERROR) {
		DebugString("Failed to bind the UDP socket - error code %d\n", LAST_ERROR);
		Close_Socket ();
		return(false);
	}

	// Winsock refuses a broadcast on a socket that never asked for one.
	if ( UseBroadcast ) {
		int optval = 1;
		if ( setsockopt ( Socket, SOL_SOCKET, SO_BROADCAST, (char*)&optval, sizeof(optval) ) == SOCKET_ERROR ) {
			DebugString ("Failed to set UDP socket option SO_BROADCAST - error code %d.\n", LAST_ERROR );
		}
	}

	/*
	**	Clear out any old local addresses from the local address list.
	*/
	while ( LocalAddresses.Count() ) {
		delete LocalAddresses[0];
		LocalAddresses.Delete_Index(0);
	}

	Register_Local_Addresses();

	/*
	**	Set options for the UDP socket
	*/
	ling.l_onoff = 0;   // linger off
	ling.l_linger = 0;  // timeout in seconds (ie close now)
	setsockopt (Socket, SOL_SOCKET, SO_LINGER, (LPSTR)&ling, sizeof(ling));

	BASECLASS::Set_Socket_Options();

	DebugString("UDP Socket init complete\n");

	return(true);


}


/// <summary>
/// Collects the addresses this machine can be reached at.
/// Every adapter's address goes into the local address list, which is what lets an incoming
/// packet be recognized as one of our own and thrown away. When broadcasting is enabled,
/// each adapter's network also contributes a directed broadcast address, so that a broadcast
/// reaches every network this machine sits on rather than only the first.
/// </summary>
/// <remarks>
/// The adapter table carries the netmasks that a directed broadcast address is computed from;
/// gethostbyname, which this routine falls back on, reports addresses without them.
/// </remarks>
void UDPInterfaceClass::Register_Local_Addresses()
{
	unsigned long size = 0;

	if (GetAdaptersInfo(nullptr, &size) == ERROR_BUFFER_OVERFLOW) {

		IP_ADAPTER_INFO *adapters = reinterpret_cast<IP_ADAPTER_INFO *>(new char[size]);
		bool enumerated = false;

		if (GetAdaptersInfo(adapters, &size) == NO_ERROR) {

			for (IP_ADAPTER_INFO *adapter = adapters; adapter != nullptr; adapter = adapter->Next) {
				for (IP_ADDR_STRING *entry = &adapter->IpAddressList; entry != nullptr; entry = entry->Next) {

					unsigned long address = inet_addr(entry->IpAddress.String);
					if (address == INADDR_NONE || address == 0) continue;

					DebugString("Found local address: %s\n", entry->IpAddress.String);

					unsigned char *local = new unsigned char[4];
					*reinterpret_cast<unsigned long *>(local) = address;
					LocalAddresses.Add(local);
					enumerated = true;

					if (!UseBroadcast) continue;

					unsigned long mask = inet_addr(entry->IpMask.String);
					if (mask == INADDR_NONE) continue;

					// Every host bit set reaches the whole of that network. A port of zero
					// leaves the send path to use the port this socket was given.
					IPXAddressClass *broadcast = new IPXAddressClass(address | ~mask, 0);
					BroadcastAddresses.Add(broadcast);

					DebugString("Added broadcast address: %s\n", broadcast->As_String());
				}
			}
		}

		delete[] reinterpret_cast<char *>(adapters);

		if (enumerated) return;
	}

	// No adapter table, so settle for the host lookup, which reports addresses without
	// their netmasks, and a broadcast that goes no further than the local wire.
	DebugString("GetAdaptersInfo failed - falling back on gethostbyname\n");

	char hostname[128];

	if (gethostname(hostname, sizeof(hostname)) == 0) {

		struct hostent *host_info = gethostbyname(hostname);

		if (host_info == nullptr) {
			DebugString("gethostbyname failed! Error code %d\n", LAST_ERROR);
		} else {
			unsigned int **addresses = reinterpret_cast<unsigned int **>(host_info->h_addr_list);

			while (*addresses != nullptr) {
				unsigned int address = **addresses++;

				DebugString("Found local address: %d.%d.%d.%d\n", address & 0xff, (address & 0xff00) >> 8, (address & 0xff0000) >> 16, (address & 0xff000000) >> 24);

				unsigned char *a = new unsigned char[4];
				*reinterpret_cast<unsigned int *>(a) = address;
				LocalAddresses.Add(a);
			}
		}
	}

	if (UseBroadcast) {
		BroadcastAddresses.Add(new IPXAddressClass(INADDR_BROADCAST, 0));
	}
}


/***********************************************************************************************
 * UDPIC::Broadcast -- Send data via the Winsock socket                                        *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    ptr to buffer containing data to send                                             *
 *           length of data to send                                                            *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *    3/20/96 3:00PM ST : Created                                                              *
 *=============================================================================================*/
void UDPInterfaceClass::Broadcast (void *buffer, int buffer_len)
{
	if (buffer == NULL || buffer_len <= 0 || buffer_len > WS_INTERNET_BUFFER_LEN) {
		Record_Packet_Drop(WS_DROP_SEND_LENGTH);
		return;
	}

	for ( int i=0 ; i<BroadcastAddresses.Count() ; i++ ) {

		/*
		**	Create a temporary holding area for the packet.
		*/
		WinsockBufferType *packet = (WinsockBufferType *)Get_New_Out_Buffer();
		if (packet == NULL) {
			return;
		}

		/*
		**	Copy the packet into the holding buffer.
		*/
		memcpy ( packet->Buffer, buffer, buffer_len );
		packet->BufferLen = buffer_len;

		/*
		**	Indicate that this packet should be broadcast.
		*/
		packet->IsBroadcast = true;

		/*
		**	Set up the send address for this packet.
		*/
		memset (packet->Address, 0, sizeof (packet->Address));
		memcpy (packet->Address, BroadcastAddresses[i], sizeof (IPXAddressClass));

		Build_Packet_CRC(packet);

		/*
		**	Add it to our out list.
		*/
		OutBuffers.Add ( packet );

		/*
		**	Send a message to ourselves so that we can initiate a write if Winsock is idle.
		*/
		SendMessage ( MainWindow, Protocol_Event_Message(), 0, (LONG)FD_WRITE );

		/*
		**	Make sure the message loop gets called.
		*/
		Windows_Message_Handler();
	}
}


/***********************************************************************************************
 * TMC::Message_Handler -- Message handler function for Winsock related messages               *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Windows message handler stuff                                                     *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *    3/20/96 3:05PM ST : Created                                                              *
 *=============================================================================================*/
int UDPInterfaceClass::Message_Handler(HWND, UINT message, UINT, LONG lParam)
{
	struct 	sockaddr_in addr;
	int	 	rc;
	int		addr_len;
	WinsockBufferType *packet;

	/*
	**	We only handle UDP events.
	*/
	if ( message != WM_UDPASYNCEVENT ) return(1);

	/*
	**	Handle UDP packet events
	*/
	switch ( WSAGETSELECTEVENT(lParam) ) {

		/*
		**	Read event. Winsock has data it would like to give us.
		*/
		case FD_READ: {
			/*
			**	Clear any outstanding errors on the socket.
			*/
			rc = WSAGETSELECTERROR(lParam);
			if (rc != 0) {
				Clear_Socket_Error (Socket);
				return(0);;
			}

			/*
			**	Call the Winsock recvfrom function to get the outstanding packet.
			*/
			rc = Receive_From ( (char*)ReceiveBuffer, sizeof (ReceiveBuffer), &addr );
			if (rc == SOCKET_ERROR) {
				Clear_Socket_Error (Socket);
				return(0);;
			}

			std::span<std::byte const> const datagram(reinterpret_cast<std::byte const *>(ReceiveBuffer), rc > 0 ? static_cast<std::size_t>(rc) : 0);
			NetAdmission::DatagramResult const admission = NetAdmission::Admit_Datagram(datagram, WS_INTERNET_BUFFER_LEN);
			if (!admission.Succeeded()) {
				switch (admission.ErrorCodeCode) {
					case NetAdmission::Error::DATAGRAM_TOO_LARGE:
						Record_Packet_Drop(WS_DROP_RECEIVE_TOO_LARGE);
						break;
					case NetAdmission::Error::BAD_CRC:
						Record_Packet_Drop(WS_DROP_BAD_CRC);
						break;
					default:
						Record_Packet_Drop(WS_DROP_RECEIVE_TOO_SHORT);
						break;
				}
				return(0);
			}

			{

				/*
				**	Make sure this packet didn't come from us. If it did then throw it away.
				*/
				for ( int i=0 ; i<LocalAddresses.Count() ; i++ ) {
					if ( ! memcmp (LocalAddresses[i], &addr.sin_addr.s_addr, 4) ) return(0);
				}

				/*
				**	Create a new buffer and store this packet in it.
				*/
				packet = (WinsockBufferType *)Get_New_In_Buffer();
				if (packet == NULL) {
					return(0);
				}
				packet->BufferLen = static_cast<int>(admission.Payload.size());
				packet->CRC = admission.WireCRC;
				memcpy(packet->Buffer, admission.Payload.data(), admission.Payload.size());

					/*
					**	Copy the address data into the holding buffer address area.
					*/
					IPXAddressClass source(addr.sin_addr.s_addr, addr.sin_port);
					memset ( packet->Address, 0, sizeof (packet->Address) );
					memcpy ( packet->Address, &source, sizeof (source) );

					/*
					**	Add the holding buffer to the packet list.
					*/
					InBuffers.Add (packet);
			}
			return(0);
		}


		/*
		**	Write event. We send ourselves this event when we have more data to send. This
		**	event will also occur automatically when a packet has finished being sent.
		*/
		case FD_WRITE:
			/*
			**	Clear any outstanding erros on the socket.
			*/
			rc = WSAGETSELECTERROR(lParam);
			if (rc != 0) {
				Clear_Socket_Error (Socket);
				return(0);;
			}

			/*
			**	If there are no packets waiting to be sent then bail.
			*/
			if ( OutBuffers.Count() == 0 ) return(0);
			int packetnum = 0;

			/*
			**	Get a pointer to the packet.
			*/
			packet = OutBuffers [ packetnum ];

			/*
			**	Set up the address structure of the outgoing packet
			*/
			IPXAddressClass destination;
			memcpy (&destination, packet->Address, sizeof (destination));

			addr.sin_family = AF_INET;
			addr.sin_addr.s_addr = destination.Get_IP();

			// An address without a port of its own goes to the port this socket was given.
			addr.sin_port = destination.Get_Port() != 0 ? destination.Get_Port()
				: (unsigned short) htons ( DestinationPortSet ? DestinationPort : (unsigned short)WestwoodOnline_PortNumber );

			/*
			**	Send it.
			**	If we get a WSAWOULDBLOCK error it means that Winsock is unable to accept the packet
			**	at this time. In this case, we clear the socket error and just exit. Winsock will
			**	send us another WRITE message when it is ready to receive more data.
			*/
			rc = Send_To ( ((char const *)packet->Buffer) - sizeof(packet->CRC), packet->BufferLen + sizeof(packet->CRC), &addr );

			if (rc == SOCKET_ERROR){
				if (LAST_ERROR != WSAEWOULDBLOCK) {
					Clear_Socket_Error (Socket);
					return(0);
				}
			}

			/*
			**	Delete the sent packet.
			*/
			OutBuffers.Delete_Index(0);
			if (packet->IsAllocated) {
				delete packet;
			} else {
				packet->InUse = false;
				OutBuffersUsed--;
			}
			return(0);
	}

	return(0);
}
