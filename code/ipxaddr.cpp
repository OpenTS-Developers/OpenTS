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

/* $Header: /CounterStrike/IPXADDR.CPP 1     3/03/97 10:24a Joe_bostic $ */
/***************************************************************************
 **   C O N F I D E N T I A L --- W E S T W O O D    S T U D I O S        **
 ***************************************************************************
 *                                                                         *
 *                 Project Name : Command & Conquer                        *
 *                                                                         *
 *                    File Name : IPXADDR.CPP                              *
 *                                                                         *
 *                   Programmer : Bill Randolph                            *
 *                                                                         *
 *                   Start Date : December 19, 1994                        *
 *                                                                         *
 *                  Last Update : December 19, 1994   [BR]                 *
 *                                                                         *
 *-------------------------------------------------------------------------*
 * Functions:                                                              *
 *   IPXAddressClass::IPXAddressClass -- class constructor                 *
 *   IPXAddressClass::IPXAddressClass -- class constructor form 2          *
 *   IPXAddressClass::IPXAddressClass -- class constructor form 3          *
 *   IPXAddressClass::Set_Address -- sets the IPX address values           *
 *   IPXAddressClass::Set_Address -- sets the IPX values from a header     *
 *   IPXAddressClass::Get_Address -- retrieves the IPX address values      *
 *   IPXAddressClass::Get_Address -- copies address into an IPX header     *
 *   IPXAddressClass::Is_Broadcast -- tells if this is a broadcast address *
 *   IPXAddressClass::operator== -- overloaded comparison operator         *
 *   IPXAddressClass::operator!= -- overloaded comparison operator         *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "ipxaddr.h"

#include <cstdio>
#include <cstring>
#if defined(_WIN32)
#include <winsock.h>
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#endif


/***************************************************************************
 * IPXAddressClass::IPXAddressClass -- class constructor                   *
 *                                                                         *
 * This default constructor generates a broadcast address.                 *
 *                                                                         *
 * INPUT:                                                                  *
 *      net      Network Number for this address                           *
 *      node      Node Address for this address                            *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      none.                                                              *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   12/19/1994 BR : Created.                                              *
 *=========================================================================*/
IPXAddressClass::IPXAddressClass(void)
{
	IP = INADDR_BROADCAST;
	Port = 0;

}	/* end of IPXAddressClass */


/***************************************************************************
 * IPXAddressClass::IPXAddressClass -- class constructor form 2            *
 *                                                                         *
 * INPUT:                                                                  *
 *      ip       IPv4 address, in network byte order                       *
 *      port     port number, in network byte order                        *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      none.                                                              *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   12/19/1994 BR : Created.                                              *
 *=========================================================================*/
IPXAddressClass::IPXAddressClass(uint32_t ip, uint16_t port)
{
	IP = ip;
	Port = port;

}	/* end of IPXAddressClass */


/***************************************************************************
 * IPXAddressClass::Set_Address -- sets the address values                 *
 *                                                                         *
 * INPUT:                                                                  *
 *      ip       IPv4 address, in network byte order                       *
 *      port     port number, in network byte order                        *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      none.                                                              *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   12/19/1994 BR : Created.                                              *
 *=========================================================================*/
void IPXAddressClass::Set_Address(uint32_t ip, uint16_t port)
{
	IP = ip;
	Port = port;

}	/* end of Set_Address */


/***************************************************************************
 * IPXAddressClass::Is_Broadcast -- tells if this is a broadcast address   *
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
 *   12/19/1994 BR : Created.                                              *
 *=========================================================================*/
int IPXAddressClass::Is_Broadcast(void) const
{
	return(IP == INADDR_BROADCAST);

}	/* end of Is_Broadcast */


/// <summary>
/// Compares two addresses for equality.
/// A port of zero matches any port, because one side of a comparison often knows only
/// the IP: Westwood Online hands out the players' addresses without ports, while a
/// received packet always carries the port it was sent from.
/// </summary>
bool IPXAddressClass::operator == (IPXAddressClass & addr)
{
	if (IP != addr.IP) {
		return(false);
	}
	return(Port == 0 || addr.Port == 0 || Port == addr.Port);
}


/// <summary>
/// Compares two addresses for inequality.
/// </summary>
bool IPXAddressClass::operator != (IPXAddressClass & addr)
{
	return(!(*this == addr));
}


/// <summary>
/// Converts this address into a displayable string.
/// This routine is used by the debug output when a network address has to be shown in a
/// human readable form. A tunnelled game has no address of its own to show, so its
/// players are named by the tunnel ID that stands in for one.
/// </summary>
/// <returns>Returns with a pointer to the formatted address text.</returns>
/// <remarks>The text is built in a shared static buffer, so it only survives until the next
/// call to this routine.</remarks>
const char *IPXAddressClass::As_String(void)
{
	static char _addr_str[128];

	const unsigned char *quad = reinterpret_cast<const unsigned char *>(&IP);

	if (IP == 0) {
		sprintf(_addr_str, "tunnel id %d", ntohs(Port));
	} else {
		sprintf(_addr_str, "%d.%d.%d.%d:%d", quad[0], quad[1], quad[2], quad[3], ntohs(Port));
	}

	return(_addr_str);
}

/************************** end of ipxaddr.cpp *****************************/
