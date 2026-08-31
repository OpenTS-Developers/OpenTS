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
 *                     $Archive:: /Sun/WSProto.h                                              $*
 *                                                                                             *
 *                      $Author:: Joe_b                                                       $*
 *                                                                                             *
 *                     $Modtime:: 8/12/97 5:42p                                               $*
 *                                                                                             *
 *                    $Revision:: 4                                                          $ *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "_wsproto.h"
#include "ipxaddr.h"
#include "vector.h"

/*
**	Include standard Winsock 1.0 header file.
*/
#include <winsock.h>

#ifndef fw_assert
#define fw_assert assert
#endif

#ifndef LAST_ERROR
#define LAST_ERROR WSAGetLastError()
#endif

/*
**	Misc defines
*/
#define WINSOCK_MINOR_VER		1   // Version of Winsock
#define WINSOCK_MAJOR_VER		1   //    that we require

#define WS_RECEIVE_BUFFER_LEN	1024		// Length of our temporary receive buffer.
#define SOCKET_BUFFER_SIZE		1024*128	// Length of winsocks internal buffer.

#define WS_INTERNET_BUFFER_LEN	768

#define WS_MAX_STATIC_BUFFERS	128

#define PLANET_WESTWOOD_HANDLE_MAX 20	// Max length of a WChat handle

/*
**	Define events for Winsock callbacks
*/
#define WM_UDPASYNCEVENT		(WM_USER + 116)	// UDP socket Async event


/*
**	Enum to identify the protocols supported by the Winsock interface.
*/
enum ProtocolEnum {
	PROTOCOL_NONE,
	PROTOCOL_UDP
};

/*
**
**	Class to interface with Winsock. This interface only supports connectionless packet protocols
**	like UDP & IPX. Connection orientated or streaming protocols like TCP are not supported by this
**	class.
**
*/
class WinsockInterfaceClass {

	public:
		enum PacketDropReasonType {
			WS_DROP_RECEIVE_TOO_SHORT,
			WS_DROP_RECEIVE_TOO_LARGE,
			WS_DROP_BAD_CRC,
			WS_DROP_READ_BUFFER_TOO_SMALL,
			WS_DROP_SEND_LENGTH,
			WS_DROP_SEND_ADDRESS,
			WS_DROP_COUNT
		};

		WinsockInterfaceClass(void);
		virtual ~WinsockInterfaceClass(void);

		bool Init(void);
		void Close(void);


		virtual void Close_Socket(void);

		virtual int  Read(void *buffer, int &buffer_len, void *address, int &address_len);
		virtual void WriteTo (void *buffer, int buffer_len, void *address, int address_len);
		virtual void Broadcast (void *buffer, int buffer_len);

		virtual void Discard_In_Buffers (void);
		virtual void Discard_Out_Buffers (void);

		virtual bool Start_Listening (void);
		virtual void Stop_Listening (void);

		virtual void Clear_Socket_Error(SOCKET socket);

		virtual bool Set_Socket_Options ( void );

		virtual void Set_Broadcast_Address ( const IPXAddressClass & ) {};
		virtual void Clear_Broadcast_Addresses(void) {};

		virtual ProtocolEnum Get_Protocol (void) {
			return(PROTOCOL_NONE);
		};

		virtual int Protocol_Event_Message (void) {
			return(0);
		};

		virtual bool Open_Socket ( SOCKET ) {
			return(false);
		};

		virtual int Message_Handler(HWND, UINT, UINT, LONG) {
			return(1);
		}

		virtual bool Get_Host_Name(char *name, int len);

		virtual int Get_Num_Local_Addresses(void) { return(0); }
		virtual unsigned char *Get_Local_Address(int index) { return(NULL); }

		enum ConnectStatusEnum {
			CONNECTED_OK = 0,
			NOT_CONNECTING,
			CONNECTING,
			UNABLE_TO_CONNECT_TO_SERVER,
			CONTACTING_SERVER,
			SERVER_ADDRESS_LOOKUP_FAILED,
			RESOLVING_HOST_ADDRESS,
			UNABLE_TO_ACCEPT_CLIENT,
			UNABLE_TO_CONNECT,
			CONNECTION_LOST
		};

		inline ConnectStatusEnum Get_Connection_Status(void) {return(ConnectStatus);}
		unsigned int Dropped_Packets(PacketDropReasonType reason) const;

	protected:

		/*
		**	This struct contains the information needed for each incoming and outgoing packet.
		**	It acts as a temporary control for these packets.
		*/
		struct WinsockBufferType {
			unsigned char		Address [64];                   // Address. IN_ADDR, IPXAddressClass etc.
			int					BufferLen;                      // Length of data in buffer
			bool				IsBroadcast;                    // Flag to broadcast this packet
			bool				InUse;                          // Useage state of buffer
			bool				IsAllocated;                    // false means statically allocated.
			unsigned int		CRC;                            // CRC of packet for extra sanity.
			unsigned char		Buffer[WS_INTERNET_BUFFER_LEN]; // Buffer to store packet in.
		};

		/*
		**	Packet buffer allocation.
		*/
		void *Get_New_Out_Buffer(void);
		void *Get_New_In_Buffer(void);

		/*
		**	Packet CRCs.
		*/
		virtual void Build_Packet_CRC(WinsockBufferType *packet);
		unsigned int Calculate_Packet_CRC(void const *buffer, int buffer_len) const;
		void Record_Packet_Drop(PacketDropReasonType reason);

		/*
		**	Array of buffers to temporarily store incoming and outgoing packets.
		*/
		DynamicVectorClass <WinsockBufferType *> InBuffers;
		DynamicVectorClass <WinsockBufferType *> OutBuffers;

		/*
		**	Array of buffers that are always available for incoming packets.
		*/
		WinsockBufferType StaticInBuffers[WS_MAX_STATIC_BUFFERS];
		WinsockBufferType StaticOutBuffers[WS_MAX_STATIC_BUFFERS];

		/*
		**	Pointers to allow circular use of the buffer arrays.
		*/
		int InBufferArrayPos;
		int OutBufferArrayPos;

		/*
		**	Usage count for each array.
		*/
		int InBuffersUsed;
		int OutBuffersUsed;

		/*
		**	Is Winsock present and initialised?
		*/
		bool 					WinsockInitialised;

		/*
		**	Socket that communications will take place over.
		*/
		SOCKET				Socket;

		/*
		**	Async object required for callbacks to our message handler.
		*/
		HANDLE				ASync;

		/*
		**	Temporary receive buffer to use when querying Winsock for incoming packets.
		*/
		unsigned char		ReceiveBuffer[WS_RECEIVE_BUFFER_LEN];

		/*
		**	Current connection status.
		*/
		ConnectStatusEnum	ConnectStatus;
		unsigned int		PacketDrops[WS_DROP_COUNT];
};
