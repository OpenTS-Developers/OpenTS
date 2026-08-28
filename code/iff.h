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
 *                     $Archive:: /G/wwlib/iff.h                                              $*
 *                                                                                             *
 *                      $Author:: Neal_k                                                      $*
 *                                                                                             *
 *                     $Modtime:: 9/23/99 1:46p                                               $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

class Buffer;

#define LZW_SUPPORTED			FALSE

/*=========================================================================*/
/* Iff and Load Picture system defines and enumerations							*/
/*=========================================================================*/

#define 	MAKE_ID(a,b,c,d)			((int) ((int) d << 24) | ((int) c << 16) | ((int) b <<  8) | (int)(a))
#define	IFFize_WORD(a)			Reverse_Word(a)
#define	IFFize_LONG(a)			Reverse_Long(a)


//lint -strong(AJX,PicturePlaneType)
enum PicturePlaneType {
	BM_AMIGA,   // Bit plane format (8K per bitplane).
	BM_MCGA,    // Byte per pixel format (64K).

	BM_DEFAULT=BM_MCGA	// Default picture format.
};

/*
**	This is the compression type code.  This value is used in the compressed
**	file header to indicate the method of compression used.  Note that the
**	LZW method may not be supported.
*/
//lint -strong(AJX,CompressionType)
enum CompressionType {
	NOCOMPRESS,		// No compression (raw data).
	LZW12,			// LZW 12 bit codes.
	LZW14,			// LZW 14 bit codes.
	HORIZONTAL,		// Run length encoding (RLE).
	LCW				// Westwood proprietary compression.
};

/*
**	Compressed blocks of data must start with this header structure.
**	Note that disk based compressed files have an additional two
**	leading bytes that indicate the size of the entire file.
*/
//lint -strong(AJX,CompHeaderType)
#pragma pack(push,1)
struct CompHeaderType {
	char	Method; // Compression method (CompressionType).
	char	pad;    // Reserved pad byte (always 0).
	int	Size;   // Size of the uncompressed data.
	short	Skip;   // Number of bytes to skip before data.
};
#pragma pack(pop)


/*=========================================================================*/
/* The following prototypes are for the file: IFF.CPP								*/
/*=========================================================================*/

int __cdecl Open_Iff_File(char const *filename);
void __cdecl Close_Iff_File(int fh);
unsigned int __cdecl Get_Iff_Chunk_Size(int fh, int id);
unsigned int __cdecl Read_Iff_Chunk(int fh, int id, void *buffer, unsigned int maxsize);
void __cdecl Write_Iff_Chunk(int file, int id, void *buffer, int length);


/*=========================================================================*/
/* The following prototypes are for the file: LOADPICT.CPP						*/
/*=========================================================================*/

//int __cdecl Load_Picture(char const *filename, BufferClass& scratchbuf, BufferClass& destbuf, unsigned char *palette=NULL, PicturePlaneType format=BM_DEFAULT);


/*=========================================================================*/
/* The following prototypes are for the file: LOAD.CPP							*/
/*=========================================================================*/

unsigned int __cdecl Load_Data(char const *name, void *ptr, unsigned int size);
unsigned int __cdecl Write_Data(char const *name, void *ptr, unsigned int size);
//void * __cdecl Load_Alloc_Data(char const *name, MemoryFlagType flags);
unsigned int __cdecl Load_Uncompress(char const *file, Buffer & uncomp_buff, Buffer & dest_buff, void *reserved_data=NULL);
unsigned int Uncompress_Data(void const *src, void *dst);
void __cdecl Set_Uncomp_Buffer(int buffer_segment, int size_of_buffer);

/*=========================================================================*/
/* The following prototypes are for the file: WRITELBM.CPP						*/
/*=========================================================================*/

//bool Write_LBM_File(int lbmhandle, BufferClass& buff, int bitplanes, unsigned char *palette);



/*========================= Assembly Functions ============================*/

#ifdef __cplusplus
extern "C" {
#endif

/*=========================================================================*/
/* The following prototypes are for the file: PACK2PLN.ASM						*/
/*=========================================================================*/

extern void __cdecl Pack_2_Plane(void *buffer, void * pageptr, int planebit);

/*=========================================================================*/
/* The following prototypes are for the file: LCWCOMP.ASM						*/
/*=========================================================================*/

extern unsigned long __cdecl LCW_Compress(void *source, void *dest, unsigned long length);

#ifdef __cplusplus
}
#endif
/*=========================================================================*/
