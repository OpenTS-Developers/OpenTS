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
 *                     $Archive:: /Commando/Code/Library/PCX.cpp                              $*
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 9/28/98 12:06p                                              $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "pcx.h"

#include "dsurface.h"

#include <algorithm>
#include <cstdlib>

/// <summary>
/// Reads how large a picture is without decoding it.
/// </summary>
/// <returns>bool; Was a picture found to measure?</returns>
bool Read_PCX_Size(FileClass & file, int & width, int & height)
{
	PCX_HEADER header;

	if (!file.Is_Available()) {
		return(false);
	}

	if (!file.Open(FileClass::READ)) {
		return(false);
	}

	int read = file.Read(&header, sizeof(header));
	file.Close();

	if (read != sizeof(header) || header.id != 10) {
		return(false);
	}

	width = header.width - header.x + 1;
	height = header.height - header.y + 1;

	return(width > 0 && height > 0);
}


/***************************************************************************
 * READ_PCX_FILE -- read a pcx file into a Graphic Buffer                  *
 *                                                                         *
 *   GraphicBufferClass* Read_PCX_File (char* name, char* palette,void *Buff, long size );*
 *                                                                         *
 *                                                                         *
 * INPUT: name is a NULL terminated string of the format [xxxx.pcx]        *
 *        palette is optional, if palette != NULL the the color palette of *
 *                the pcx file will be place in the memory block pointed   *
 *               by palette.                                               *
 *          Buff is optional, if Buff == NULL a new memory Buffer          *
 *                will be allocated, otherwise the file will be placed     *
 *                at location pointed by Buffer;                           *
 *         Size is the size in bytes of the memory block pointed by Buff   *
 *              is also optional;                                          *
 * OUTPUT: on success a pointer to a GraphicBufferClass containing the     *
 *         pcx file, NULL otherwise.                                       *
 *                                                                         *
 * WARNINGS:                                                               *
 *         Appears to be a comment-free zone                               *
 *                                                                         *
 * HISTORY:                                                                *
 *   05/03/1995 JRJ : Created.                                             *
 *   04/30/1996 ST : Tidied up and modified to use CCFileClass             *
 *=========================================================================*/
#define	POOL_SIZE 2048
#define	READ_CHAR()  *file_ptr++ ; \
							 if ( file_ptr	>= & pool [ POOL_SIZE ]	) { \
								 file_handle.Read (pool, POOL_SIZE ); \
								 file_ptr = pool ; \
							 }
#define	READ_CHARx()  *file_ptr++ ; \
							 if ( file_ptr	>= & pool [ POOL_SIZE ]	) { \
								 file_handle.Read (pool, POOL_SIZE ); \
							 }


Surface * Read_PCX_File(FileClass & file_handle, PaletteClass * palette, void * Buff, int Size)
{
	unsigned					i, j;
	unsigned char			rle;
	unsigned char			color;
	unsigned					scan_pos;
	char						*file_ptr;
	unsigned					width;
	unsigned					height;
	char						*buffer;
	PCX_HEADER				header;
	char						pool [POOL_SIZE];
	BSurface * pic;

	if (!file_handle.Is_Available()) return(NULL);

	file_handle.Open(FileClass::READ);

	file_handle.Read (&header, sizeof (PCX_HEADER));

	if (header.id != 10 &&  header.version != 5 && header.pixelsize != 8 ) return(NULL);

	width = header.width - header.x + 1;
	height = header.height - header.y + 1;
	unsigned in_line = header.color_planes * header.byte_per_line;
	unsigned bytes_per_pixel = (header.color_planes == 1) ? 1 : 2;

	if (Buff != NULL) {
		i = Size / width;
		height = std::min((int)(i - 1), (int)height);
		Buffer b(Buff, Size);
		pic = new BSurface(width, height, bytes_per_pixel, &b);
	} else {
		pic = new BSurface(width, height, bytes_per_pixel);
	}
	if (pic == NULL) return(NULL);

	buffer = (char *)pic->Lock();
	if (buffer != NULL) {
		file_ptr = pool ;
		file_handle.Read (pool, POOL_SIZE);

		unsigned short * hibuffer = (unsigned short *)buffer;

		if ( header.color_planes == 3 ) {

			/*
			 * The pcx file is stored as three separate red, green and blue
			 * planes per scanline. Decode each scanline into a temporary
			 * line buffer, then interleave the three planes into hicolor
			 * pixels on the surface.
			 */
			unsigned char * line = new unsigned char [in_line];
			if (line == NULL) {
				pic->Unlock();
				delete pic;
				return(NULL);
			}

			for ( j = 0 ; j < height ; j ++ ) {
				i = 0;
				do {
					rle = READ_CHAR ();
					if ( (rle & 192) == 192 ) {
						rle &= 63 ;
						color = READ_CHAR ();
						do {
							line[i++] = color;
						} while (--rle);
					} else {
						line[i++] = rle;
					}
				} while ( i < in_line );

				i = 0;
				if (i < width) {
					unsigned char * rptr = line;
					unsigned char * gptr = line + header.byte_per_line;
					unsigned char * bptr = line + header.byte_per_line * 2;
					while ( i < width ) {
						*hibuffer++ = (unsigned short)DSurface::Build_Hicolor_Pixel(*rptr++, *gptr++, *bptr++);
						i++;
					}
				}
			}

			delete [] line;
			pic->Unlock();

		} else if ( header.byte_per_line != width ) {

			i = 0;
			rle = 0;
			for ( scan_pos = j = 0 ; j < height ; j ++, scan_pos += width ) {
				for ( i = 0 ; i < header.byte_per_line ; ) {
					rle = READ_CHAR ();
					if ( (rle & 192) == 192 ) {
						rle &= 63 ;
						color =	READ_CHAR (); ;
						for ( unsigned k = 0 ; k < rle ; k ++ ) {
							if ( k + i < width ) {
								*(buffer + scan_pos + k + i) = color;
							}
						}
						i += rle;
					} else {
						if ( i < width ) {
							*(buffer+scan_pos + i++ ) = (char)rle;
						}
					}
				}
			}

			if ( i == width ) {
				rle = READ_CHAR ();
			}
			if ( (rle & 192) == 192 ) {
				READ_CHARx();
			}
			pic->Unlock();

		} else {

			for ( i = 0 ; i < width * height ; ) {
				rle = READ_CHAR ();
				if ( (rle & 192) == 192 ) {
					rle &= 63 ;
					color = READ_CHAR ();
					memset ( buffer + i, color, rle );
					i += rle ;
				} else {
					*(buffer + i++) = (char)rle;
				}
			}
			pic->Unlock();
		}
	}

	if ( palette && header.color_planes == 1 ) {
		file_handle.Seek (- (256 * (int)sizeof(RGB)), SEEK_END );
		file_handle.Read (palette, 256L * sizeof ( RGB ));
	}

	file_handle.Close();
	return(pic);
}
