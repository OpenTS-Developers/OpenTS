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

/***************************************************************************
 *                                                                         *
 *                 Project Name : Westwood Auto Registration App           *
 *                                                                         *
 *                    File Name : FIELD.H                                  *
 *                                                                         *
 *                   Programmer : Philip W. Gorrow                         *
 *                                                                         *
 *                   Start Date : 04/22/96                                 *
 *                                                                         *
 *                  Last Update : April 22, 1996 [PWG]                     *
 *                                                                         *
 * This module takes care of maintaining the field list used to process    *
 * packets.                                                                *
 *                                                                         *
 *-------------------------------------------------------------------------*
 * Functions:                                                              *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#if defined(_WIN32)
#include <winsock.h>
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#endif

#define FIELD_HEADER_SIZE	(sizeof(FieldClass) - (sizeof(void *) * 2))

#define TYPE_CHAR						1
#define TYPE_UNSIGNED_CHAR			2
#define TYPE_SHORT					3
#define TYPE_UNSIGNED_SHORT		4
#define TYPE_LONG						5
#define TYPE_UNSIGNED_LONG			6
#define TYPE_STRING					7
#define TYPE_CHUNK					20

class PacketClass;

class FieldClass {

	public:
		friend class PacketClass;
		//
		// Define constructors to be able to create all the different kinds
		// of fields.
		//
		FieldClass(void) {};
		FieldClass(char const *id, char data);
		FieldClass(char const *id, unsigned char data);
		FieldClass(char const *id, short data);
		FieldClass(char const *id, unsigned short data);
		FieldClass(char const *id, int data);
		FieldClass(char const *id, unsigned int data);
		FieldClass(char const *id, char const *data);
		FieldClass(char const *id, void *data, int length);

		~FieldClass(void);

		void Host_To_Net(void);
		void Net_To_Host(void);

	private:
		char				ID[4];  // id value of this field
		unsigned short	DataType;   // id of the data type we are using
		unsigned short Size;        // size of the data portion of this field
		void  			*Data;      // pointer to the data portion of this field
		FieldClass		*Next;      // pointer to the next field in the field list
};
