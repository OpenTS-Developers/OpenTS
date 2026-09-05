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

/* $Header: /CounterStrike/IPXCONN.CPP 1     3/03/97 10:24a Joe_bostic $ */
/***************************************************************************
 **   C O N F I D E N T I A L --- W E S T W O O D    S T U D I O S        **
 ***************************************************************************
 *                                                                         *
 *                 Project Name : Command & Conquer                        *
 *                                                                         *
 *                    File Name : IPXCONN.CPP                              *
 *                                                                         *
 *                   Programmer : Bill Randolph                            *
 *                                                                         *
 *                   Start Date : December 20, 1994                        *
 *                                                                         *
 *                  Last Update : April 9, 1995 [BRR]                      *
 *                                                                         *
 *-------------------------------------------------------------------------*
 * Functions:                                                              *
 *   IPXConnClass::IPXConnClass -- class constructor                       *
 *   IPXConnClass::~IPXConnClass -- class destructor                       *
 *   IPXConnClass::Init -- hardware-specific initialization routine        *
 *   IPXConnClass::Configure -- One-time initialization routine            *
 *   IPXConnClass::Start_Listening -- commands IPX to listen               *
 *   IPXConnClass::Stop_Listening -- commands IPX to stop  listen          *
 *   IPXConnClass::Send -- sends a packet; invoked by SequencedConnection  *
 *   IPXConnClass::Open_Socket -- opens communications socket              *
 *   IPXConnClass::Close_Socket -- closes the socket                       *
 *   IPXConnClass::Send_To -- sends the packet to the given address        *
 *   IPXConnClass::Broadcast -- broadcasts the given packet                *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "ipxconn.h"

#include "dbgprint.h"
#include "globals.h"
#include "goptions.h"
#include "utf8.h"
#include "wsproto.h"


/*
********************************* Globals ***********************************
*/
int			 		IPXConnClass::ConnectionNum;
int 					IPXConnClass::Configured = 0;
int 					IPXConnClass::SocketOpen = 0;
int 					IPXConnClass::Listening = 0;


/***************************************************************************
 * IPXConnClass::IPXConnClass -- class constructor                         *
 *                                                                         *
 * INPUT:                                                                  *
 *      numsend         desired # of entries for the send queue            *
 *      numreceive      desired # of entries for the receive queue         *
 *      maxlen         max length of an application packet                 *
 *      magicnum         the packet "magic number" for this connection     *
 *      address         address of destination (NULL = no address)         *
 *      id               connection's unique numerical ID                  *
 *      name            connection's name                                  *
 *      extralen         max size of app-specific extra bytes (optional)   *
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
IPXConnClass::IPXConnClass (int numsend, int numreceive, int maxlen,
	unsigned short magicnum, IPXAddressClass *address, int id, char const *name,
	int extralen) :
	BASECLASS (numsend, numreceive, maxlen, magicnum,
		2,          // retry delta
		-1,         // max retries
		60,         // timeout
		extralen)   // (currently, this is only used by the Global Channel)
{
	/*------------------------------------------------------------------------
	Save the values passed in
	------------------------------------------------------------------------*/
	if (address)
		Address = (*address);
	ID = id;
	UTF8::Copy(Name, sizeof(Name), name);
}	/* end of IPXConnClass */


/***************************************************************************
 * IPXConnClass::Init -- hardware-specific initialization routine          *
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
void IPXConnClass::Init (void)
{
	/*------------------------------------------------------------------------
	Invoke the parent's Init routine
	------------------------------------------------------------------------*/
	BASECLASS::Init();

}	/* end of Init */


/***************************************************************************
 * IPXConnClass::Configure -- One-time initialization routine              *
 *                                                                         *
 * This routine sets up static members that are shared by all              *
 * connections (ie those variables used by the Send/Listen/Broadcast       *
 * routines).                                                              *
 *                                                                         *
 * INPUT:                                                                  *
 *      conn_num            local connection number (0 = not logged in)    *
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
void IPXConnClass::Configure (int conn_num)
{
	/*------------------------------------------------------------------------
	Save the values passed in
	------------------------------------------------------------------------*/
	ConnectionNum = conn_num;

	Configured = 1;

}	/* end of Configure */


/***************************************************************************
 * IPXConnClass::Start_Listening -- commands IPX to listen                 *
 *                                                                         *
 * This routine may be used to start listening in polled mode (if the      *
 * ECB's Event_Service_Routine is NULL), or in interrupt mode; it's        *
 * up to the caller to fill the ECB in.  If in polled mode, Listening      *
 * must be restarted every time a packet comes in.                         *
 *                                                                         *
 * INPUT:                                                                  *
 *      none.                                                              *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      none.                                                              *
 *                                                                         *
 * WARNINGS:                                                               *
 *      - The ListenECB must have been properly filled in by the IPX Manager.*
 *      - Configure must be called before calling this routine.            *
 *                                                                         *
 * HISTORY:                                                                *
 *   12/16/1994 BR : Created.                                              *
 *=========================================================================*/
int IPXConnClass::Start_Listening(void)
{
	/*
	**	Open the socket.
	*/
	SocketOpen = Open_Socket();
	if (!SocketOpen)
		return(false);

	/*
	**	start listening on the socket.
	*/
	if ( PacketTransport->Start_Listening () ) {
		Listening =1;
		return(true);
	} else {
		Close_Socket();
		SocketOpen = false;
		return(false);
	}
}	/* end of Start_Listening */


/***************************************************************************
 * IPXConnClass::Stop_Listening -- commands IPX to stop  listen            *
 *                                                                         *
 * INPUT:                                                                  *
 *      none.                                                              *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      none.                                                              *
 *                                                                         *
 * WARNINGS:                                                               *
 *      - This routine MUST NOT be called if IPX is not listening already! *
 *                                                                         *
 * HISTORY:                                                                *
 *   12/16/1994 BR : Created.                                              *
 *=========================================================================*/
int IPXConnClass::Stop_Listening(void)
{
	if ( PacketTransport ) PacketTransport->Stop_Listening();
	Listening = 0;

	// All done.
	return(1);
}	/* end of Stop_Listening */


/***************************************************************************
 * IPXConnClass::Send -- sends a packet; invoked by SequencedConnection    *
 *                                                                         *
 * INPUT:                                                                  *
 *      buf         buffer to send                                         *
 *      buflen      length of buffer to send                               *
 *      extrabuf      (not used by this class)                             *
 *      extralen      (not used by this class)                             *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      1 = OK, 0 = error                                                  *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   12/16/1994 BR : Created.                                              *
 *=========================================================================*/
int IPXConnClass::Send(char *buf, int buflen, void *, int)
{
	/*------------------------------------------------------------------------
	Invoke our own Send_To routine, filling in our Address as the destination.
	------------------------------------------------------------------------*/
	return(Send_To (buf, buflen, &Address));

}	/* end of Send */



/***************************************************************************
 * IPXConnClass::Open_Socket -- opens communications socket                *
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
 *   12/16/1994 BR : Created.                                              *
 *=========================================================================*/
int IPXConnClass::Open_Socket(void)
{
	int rc = PacketTransport->Open_Socket(0);

	SocketOpen = rc;
	return( rc );
}	/* end of Open_Socket */


/***************************************************************************
 * IPXConnClass::Close_Socket -- closes the socket                         *
 *                                                                         *
 * INPUT:                                                                  *
 *      none.                                                              *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      none.                                                              *
 *                                                                         *
 * WARNINGS:                                                               *
 *      Calling this routine when the sockets aren't open may crash!       *
 *                                                                         *
 * HISTORY:                                                                *
 *   12/16/1994 BR : Created.                                              *
 *=========================================================================*/
void IPXConnClass::Close_Socket(void)
{
	PacketTransport->Close_Socket();
	SocketOpen = 0;
}	/* end of Close_Socket */


/***************************************************************************
 * IPXConnClass::Send_To -- sends the packet to the given address          *
 *                                                                         *
 * INPUT:                                                                  *
 *      buf         buffer to send                                         *
 *      buflen      length of buffer                                       *
 *      address      Address to send to                                    *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      1 = OK, 0 = error                                                  *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   12/16/1994 BR : Created.                                              *
 *=========================================================================*/
int IPXConnClass::Send_To(char *buf, int buflen, IPXAddressClass *address)
{
	IPXAddressClass addr = *address;

	if (PacketTransport) {
		PacketTransport->WriteTo ( (void*)buf, buflen, (void*) &addr , sizeof(IPXAddressClass));
	}
	return(true);
}	/* end of Send_To */


/***************************************************************************
 * IPXConnClass::Broadcast -- broadcasts the given packet                  *
 *                                                                         *
 * INPUT:                                                                  *
 *      socket      desired socket ID number                               *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      1 = OK, 0 = error                                                  *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   12/16/1994 BR : Created.                                              *
 *=========================================================================*/
int IPXConnClass::Broadcast(char *buf, int buflen)
{
	PacketTransport->Broadcast (buf, buflen);
	return(true);
}	/* end of Broadcast */

/************************** end of ipxconn.cpp *****************************/
