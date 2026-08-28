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
 *                     $Archive:: /G/wwlib/lcw.cpp                                            $*
 *                                                                                             *
 *                      $Author:: Neal_k                                                      $*
 *                                                                                             *
 *                     $Modtime:: 10/04/99 10:25a                                             $*
 *                                                                                             *
 *                    $Revision:: 4                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   LCW_Comp -- Performes LCW compression on a block of data.                                 *
 *   LCW_Uncomp -- Decompress an LCW encoded data block.                                       *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include	"always.h"
#include	"lcw.h"


/// <summary>
/// Decompresses an LCW encoded data block.
///
/// A leading zero byte selects the relative form, where the two offset-carrying copies reach
/// back from the current position instead of forward from the start of the destination. VQA
/// stores its palettes, codebooks and pointer data that way, because an absolute offset is a
/// word and cannot address past 64K.
/// </summary>
/// <param name="source">Compressed data.</param>
/// <param name="dest">Buffer to decompress into.</param>
/// <param name="length">Size of the destination buffer, or zero to decode without a bound. A
/// bounded call clamps every command against the room left, so a stream claiming more than the
/// caller allowed is truncated instead of running past the buffer.</param>
/// <returns>uint32_t; The number of destination bytes written.</returns>
uint32_t LCW_Uncomp(void const * source, void * dest, unsigned long length)
{
	/*
	** Uncompress data to the following codes in the format b = byte, w = word
	** n = byte code pulled from compressed data.
	**
	**   Command code, n        |Description
	** -----------------------------------------------------------------------
	** n=0xxxyyyy,yyyyyyyy      |short copy back y bytes and run x+3 from dest
	** n=10xxxxxx,n1,n2,...,nx+1|med length copy the next x+1 bytes from source
	** n=11xxxxxx,w1            |med copy from dest x+3 bytes from offset w1
	** n=11111111,w1,w2         |long copy from dest w1 bytes from offset w2
	** n=11111110,w1,b1         |long run of byte b1 for w1 bytes
	** n=10000000               |end of data reached
	*/

	unsigned char * source_ptr, * dest_ptr, * copy_ptr;
	unsigned char op_code, data;
	unsigned count;

	/* Copy the source and destination ptrs. */
	source_ptr = (unsigned char*) source;
	dest_ptr   = (unsigned char*) dest;

	bool const relative = (*source_ptr == 0);
	bool const bounded = (length != 0);
	unsigned char * const end = (unsigned char*) dest + length;

	if (relative) {
		source_ptr++;
	}

	for (;;) {

		long maxlen = 0;

		if (bounded) {
			maxlen = (long)(end - dest_ptr);

			if (maxlen <= 0) {
				return((uint32_t) (dest_ptr - (unsigned char*) dest));
			}
		}

		/* Read in the operation code. */
		op_code = *source_ptr++;

		if (!(op_code & 0x80)) {

			/* Do a short copy from destination. */
			count = (op_code >> 4) + 3;
			copy_ptr = dest_ptr - ((unsigned) *source_ptr++ + (((unsigned) op_code & 0x0f) << 8));

			if (bounded && (count > (unsigned) maxlen)) {
				count = (unsigned) maxlen;
			} 

			while (count--) {
				*dest_ptr++ = *copy_ptr++;
			} 

		} else {

			if (!(op_code & 0x40)) {

				if (op_code == 0x80) {

					/* Return # of destination bytes written. */
					return((unsigned long) (dest_ptr - (unsigned char*) dest));

				} else {

					/* Do a medium copy from source. */
					count = op_code & 0x3f;

					if (bounded && (count > (unsigned) maxlen)) {
						count = (unsigned) maxlen;
					}

					while (count--) {
						*dest_ptr++ = *source_ptr++;
					}
				}

			} else {

				if (op_code == 0xfe) {

					/* Do a long run. */
					count = *source_ptr + ((unsigned) *(source_ptr + 1) << 8);
					data = *(source_ptr + 2);
					source_ptr += 3;

					if (bounded && (count > (unsigned) maxlen)) {
						count = (unsigned) maxlen;
					}


					while (count--) { 
						*dest_ptr++ = data;
					}

				} else {

					if (op_code == 0xff) {

						/* Do a long copy from destination. */
						count = *source_ptr + ((unsigned) *(source_ptr + 1) << 8);
						size_t const offset = *(source_ptr + 2) + ((unsigned) *(source_ptr + 3) << 8);
						copy_ptr = relative ? (dest_ptr - offset) : ((unsigned char*)dest + offset);
						source_ptr += 4;

						if (bounded && (count > (unsigned) maxlen)) count = (unsigned) maxlen;

						while (count--) { 
							*dest_ptr++ = *copy_ptr++;
						}

					} else {

						/* Do a medium copy from destination. */
						count = (op_code & 0x3f) + 3;
						size_t const offset = *source_ptr + ((unsigned) *(source_ptr + 1) << 8);
						copy_ptr = relative ? (dest_ptr - offset) : ((unsigned char*)dest + offset);
						source_ptr += 2;

						if (bounded && (count > (unsigned) maxlen)) count = (unsigned) maxlen;

						while (count--) { 
							*dest_ptr++ = *copy_ptr++;
						}
					}
				}
			}
		}
	}
}


#if defined(_MSC_VER)


/***********************************************************************************************
 * LCW_Comp -- Performes LCW compression on a block of data.                                   *
 *                                                                                             *
 *    This routine will compress a block of data using the LCW compression method. LCW has     *
 *    the primary characteristic of very fast uncompression at the expense of very slow        *
 *    compression times.                                                                       *
 *                                                                                             *
 * INPUT:   source   -- Pointer to the source data to compress.                                *
 *                                                                                             *
 *          dest     -- Pointer to the destination location to store the compressed data       *
 *                      to.                                                                    *
 *                                                                                             *
 *          datasize -- The size (in bytes) of the source data to compress.                    *
 *                                                                                             *
 * OUTPUT:  Returns with the number of bytes of output data stored into the destination        *
 *          buffer.                                                                            *
 *                                                                                             *
 * WARNINGS:   Be sure that the destination buffer is big enough. The maximum size required    *
 *             for the destination buffer is (datasize + datasize/128).                        *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/20/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
/// Renegade guards this body with #ifdef _WINDOWS, which would force retval ahead of the
/// other locals; that shape is not reproduced here.
/*ARGSUSED*/
int LCW_Comp(void const * source, void * dest, int datasize)
{
	int inlen = 0;
	int a1stdest = 0;
	int a1stsrc = 0;
	int lenoff = 0;
	int ndest = 0;
	int count = 0;
	int matchoff = 0;
	int end_of_data = 0;
	int retval = 0;
#ifdef _DEBUG
	inlen = inlen;
	a1stdest = a1stdest;
	a1stsrc = a1stsrc;
	lenoff = lenoff;
	ndest = ndest;
	count = count;
	matchoff = matchoff;
	end_of_data = end_of_data;
#endif

	__asm {
		cld			// make sure all string commands are forward
		mov	edi,[dest]
		mov	esi,[source]
		mov	edx,[datasize]		// get length of data to compress

// compress data to the following codes in the format b = byte, w = word
// n = byte code pulled from compressed data
//   Bit field of n		command		description
// n=0xxxyyyy,yyyyyyyy		short run	back y bytes and run x+3
// n=10xxxxxx,n1,n2,...,nx+1	med length	copy the next x+1 bytes
// n=11xxxxxx,w1			med run		run x+3 bytes from offset w1
// n=11111111,w1,w2		long run	run w1 bytes from offset w2
// n=10000000			end		end of data reached

		mov	ebx,esi
		add	ebx,edx
		mov	[end_of_data],ebx
		mov	[inlen],1	//; set the in-length flag
		mov	[a1stdest],edi	//; save original dest offset for size calc
		mov	[a1stsrc],esi	//; save offset of first byte of data
		mov	[lenoff],edi	//; save the offset of the legth of this len
		sub	eax,eax
		mov	al,081h		//; the first byte is always a len
		stosb			//; write out a len of 1
		lodsb			//; get the byte
		stosb			//; save it

		loopstart:
		mov	[ndest],edi	//; save offset of compressed data
		mov	edi,[a1stsrc]	//; get the offset to the first byte of data
		mov	[count],1	//; set the count of run to 0

		searchloop:
		sub	eax,eax
		mov	al,[esi]	//; get the current byte of data
		cmp	al,[esi+64]
		jne	short notrunlength

		mov	ebx,edi

		mov	edi,esi
		mov	ecx,[end_of_data]
		sub	ecx,edi
		repe	scasb
		dec	edi
		mov	ecx,edi
		sub	ecx,esi
		cmp	ecx,65
		jb	short notlongenough

		mov	[inlen],0	//; clear the in-length flag
//		mov	[DWORD PTR inlen],0	//; clear the in-length flag
		mov	esi,edi
		mov	edi,[ndest]	//; get the offset of our compressed data

		mov	ah,al
		mov	al,0FEh
		stosb
		xchg	ecx,eax
		stosw
		mov	al,ch
		stosb

		mov	[ndest],edi	//; save offset of compressed data
		mov	edi,ebx
		jmp	searchloop

		notlongenough:
		mov	edi,ebx

		notrunlength:
		oploop:
		mov	ecx,esi		//; get the address of the last byte +1
		sub	ecx,edi		//; get the total number of bytes left to comp
		jz	short searchdone

		repne	scasb		//; look for a match
		jne	short searchdone	//; if we don't find one we're done

		mov	ebx,[count]
		mov	ah,[esi+ebx-1]
		cmp	ah,[edi+ebx-2]

		jne	oploop

		mov	edx,esi		//; save this spot for the next search
		mov	ebx,edi		//; save this spot for the length calc
		dec	edi		//; back up one for compare
		mov	ecx,[end_of_data]		//; get the end of data
		sub	ecx,esi		//; sub current source for max len

		repe	cmpsb		//; see how many bytes match

		jne	short notend	//; if found mismatch then di - bx = match count

		inc	edi		//; else cx = 0 and di + 1 - bx = match count

		notend:
		mov	esi,edx		//; restore si
		mov	eax,edi		//; get the dest
		sub	eax,ebx		//; sub the start for total bytes that match
		mov	edi,ebx		//; restore dest
		cmp	eax,[count]	//; see if its better than before
		jb	searchloop	//; if not keep looking

		mov	[count],eax	//; if so keep the count
		dec	ebx		//; back it up for the actual match offset
		mov	[matchoff],ebx //; save the offset for later
		jmp	searchloop	//; loop until we searched it all

		searchdone:
		mov	ecx,[count]	//; get the count of the longest run
		mov	edi,[ndest]	//; get the offset of our compressed data
		cmp	ecx,2		//; see if its not enough run to matter
		jbe	short lenin		//; if its 0,1, or 2 its too small

		cmp	ecx,10		//; if not, see if it would fit in a short
		ja	short medrun	//; if not, see if its a medium run

		mov	eax,esi		//; if its short get the current address
		sub	eax,[matchoff] //; sub the offset of the match
		cmp	eax,0FFFh	//; if its less than 12 bits its a short
		ja	short medrun	//; if its not, its a medium

		//shortrun:
		sub	ebx,ebx
		mov	bl,cl		//; get the length (3-10)
		sub	bl,3		//; sub 3 for a 3 bit number 0-7
		shl	bl,4		//; shift it left 4
		add	ah,bl		//; add in the length for the high nibble
		xchg	ah,al		//; reverse the bytes for a word store
		jmp	short srunnxt	//; do the run fixup code

		medrun:
		cmp	ecx,64		//; see if its a short run
		ja	short longrun	//; if not, oh well at least its long

		sub	cl,3		//; back down 3 to keep it in 6 bits
		or	cl,0C0h		//; the highest bits are always on
		mov	al,cl		//; put it in al for the stosb
		stosb			//; store it
		jmp	short medrunnxt //; do the run fixup code

		lenin:
		cmp	[inlen],0	//; is it doing a length?
//		cmp	[DWORD PTR inlen],0	//; is it doing a length?
		jnz	short len	//; if so, skip code

		lenin1:
		mov	[lenoff],edi	//; save the length code offset
		mov	al,80h		//; set the length to 0
		stosb			//; save it

		len:
		mov	ebx,[lenoff]	//; get the offset of the length code
		cmp	byte ptr [ebx],0BFh	//; see if its maxed out
//		cmp	[BYTE PTR ebx],0BFh	//; see if its maxed out
		je	lenin1	//; if so put out a new len code

		//stolen:
		inc	byte ptr [ebx] //; inc the count code
//		inc	[BYTE PTR ebx] //; inc the count code
		lodsb			//; get the byte
		stosb			//; store it
		mov	[inlen],1	//; we are now in a length so save it
//		mov	[DWORD PTR inlen],1	//; we are now in a length so save it
		jmp	short nxt	//; do the next code

		longrun:
		mov	al,0ffh		//; its a long so set a code of FF
		stosb			//; store it

		mov	eax,[count]	//; send out the count
		stosw			//; store it

		medrunnxt:
		mov	eax,[matchoff] //; get the offset
		sub	eax,[a1stsrc]	//; make it relative tot he start of data

		srunnxt:
		stosw			//; store it
		//; this code common to all runs
		add	esi,[count]	//; add in the length of the run to the source
		mov	[inlen],0	//; set the in leght flag to false
//		mov	[DWORD PTR inlen],0	//; set the in leght flag to false

		nxt:
		cmp	esi,[end_of_data]		//; see if we did the whole pic
		jae	short outofhere		//; if so, cool! were done

		jmp	loopstart

		outofhere:
		mov	ax,080h		//; remember to send an end of data code
		stosb			//; store it
		mov	eax,edi		//; get the last compressed address
		sub	eax,[a1stdest]	//; sub the first for the compressed size
		mov	[retval],eax
	}

	return(retval);
}
#endif
