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

/****************************************************************************
*
*        C O N F I D E N T I A L -- W E S T W O O D  S T U D I O S
*
*----------------------------------------------------------------------------
*
* PROJECT
*     VQAPlay32 library.
*
* FILE
*     dstream.c
*
* DESCRIPTION
*     DOS IO handler.
*
* PROGRAMMER
*     Denzil E. Long, Jr.
*
* DATE
*     April 10, 1995
*
*----------------------------------------------------------------------------
*
* PUBLIC
*     VQA_InitAsDOS - Initialize IO with the standard DOS handler.
*
* PRIVATE
*     VQADOSHandler - Standard DOS IO handler.
*
****************************************************************************/

#include	"vqaplayp.h"
#include	<stdint.h>
#include	<stdio.h>
#include	<fcntl.h>
#include	<io.h>
#include	<string.h>


long __cdecl Disk_VQA_Stream_Handler(VQAHandle *vqa, long action, void *buffer, long nbytes)
{
	long fh;
	long error = 0;
	int temp;

	fh = ((VQAHandleP*)vqa)->Config.StreamFileHandle;

	/* Perform the action specified by the IO command */
	switch (action) {

		/* VQACMD_OPEN asks that you open the file for access. */
		case VQACMD_OPEN:
			error = open((char *)buffer, (O_RDONLY|O_BINARY));

			if (error != -1) {
				((VQAHandleP*)vqa)->Config.StreamFileHandle = error;
				error = 0;
			}
			break;

		/* VQACMD_READ means read NBytes and place it in the memory
		 * pointed to by Buffer.
		 *
		 * Any error code returned will be remapped by VQA library into
		 * VQAERR_READ.
		 */
		case VQACMD_READ:
			error = (read(fh, buffer, nbytes) != nbytes);
			break;

		/* VQACMD_WRITE is analogous to VQACMD_READ.
		 *
		 * Writing is not allowed to the VQA file, VQA library will remap the
		 * error into VQAERR_WRITE.
		 */
		case VQACMD_WRITE:
			error = 1;
			break;

		/* VQACMD_SEEK asks that you perform a seek relative to the current
		 * position. NBytes is a signed number, indicating seek direction
		 * (positive for forward, negative for backward). Buffer has no meaning
		 * here.
		 *
		 * Any error code returned will be remapped by VQA library into
		 * VQAERR_SEEK.
		 */
		case VQACMD_SEEK:
			error = (lseek(fh, nbytes, (int)(intptr_t)buffer) == -1);
			break;

		case VQACMD_SEEKPEEK:
			if (nbytes > 0) {
				error = lseek(fh, nbytes - 1, (int)(intptr_t)buffer) == -1;
				if (error == 0) {
					error = read(fh, &temp, 1) != 1;
				}
			} else {
				error = lseek(fh, nbytes, (int)(intptr_t)buffer) == -1;
				if (error == 0) {
					error = read(fh, &temp, 1) != 1;
				}
				if (error == 0) {
					error = lseek(fh, -1, 1) == -1;
				}
			}
			break;

		case VQACMD_SIZE:
			*((unsigned int *)buffer) = filelength(fh);
			error = 0;
			break;

		case VQACMD_CLOSE:
			close(fh);
			error = 0;
			break;

		/* VQACMD_INIT means to prepare your IO for reading. This is used for
		 * certain IOs that can't be read immediately upon opening, and need
		 * further preparation. This operation is allowed to fail; the error code
		 * will be returned directly to the client.
		 */
		case VQACMD_INIT:
			error = 0;
			break;

		/* IFFCMD_CLEANUP means to terminate the transaction with the associated
		 * IO. This is used for IOs that can't simply be closed. This operation
		 * is not allowed to fail; any error returned will be ignored.
		 */
		case VQACMD_CLEANUP:
			error = 0;
			break;
	}

	return(error);
}




long __cdecl Memory_VQA_Stream_Handler(VQAHandle *vqa, long action, void *buffer, long nbytes)
{
	long error = 0;
	int p;
	int bytes;
	VQAHandleP *vqap = (VQAHandleP *)vqa;
	VQALoopCache *cache = &vqap->LoopCache;

	/* Perform the action specified by the IO command */
	switch (action) {

		/* VQACMD_OPEN asks that you open the file for access. */
		case VQACMD_OPEN:
			error = 0;
			break;

		/* VQACMD_READ means read NBytes and place it in the memory
		 * pointed to by Buffer.
		 *
		 * Any error code returned will be remapped by VQA library into
		 * VQAERR_READ.
		 */
		case VQACMD_READ:
			bytes = cache->Bytes;
			p = cache->Offset;
			if (p + nbytes <= bytes) {
				memcpy(buffer, &cache->Ptr[p], nbytes);
				cache->Offset += nbytes;
				error = 0;
				break;
			}
			error = 1;
			break;

		/* VQACMD_SEEK asks that you perform a seek relative to the current
		 * position. NBytes is a signed number, indicating seek direction
		 * (positive for forward, negative for backward). Buffer has no meaning
		 * here.
		 *
		 * Any error code returned will be remapped by VQA library into
		 * VQAERR_SEEK.
		 */
		case VQACMD_SEEK:
		case VQACMD_SEEKPEEK:
			switch ((int)(intptr_t)buffer) {

				case 1:
					cache->Offset += nbytes;
					error = 0;
					break;

				case 0:
					p = (int)cache->Buffer;
					if (nbytes >= p) {
						cache->Offset = nbytes - p;
						break;
					}

				default:
					error = 1;
					break;
			}
			break;

		case VQACMD_SIZE:
			error = 0;
			break;


		case VQACMD_CLOSE:
			error = 0;
			break;

		/* VQACMD_INIT means to prepare your IO for reading. This is used for
		 * certain IOs that can't be read immediately upon opening, and need
		 * further preparation. This operation is allowed to fail; the error code
		 * will be returned directly to the client.
		 */
		case VQACMD_INIT:

		/* IFFCMD_CLEANUP means to terminate the transaction with the associated
		 * IO. This is used for IOs that can't simply be closed. This operation
		 * is not allowed to fail; any error returned will be ignored.
		 */
		case VQACMD_CLEANUP:
			error = 0;
			break;

		/* VQACMD_WRITE is analogous to VQACMD_READ.
		 *
		 * Writing is not allowed to the VQA file, VQA library will remap the
		 * error into VQAERR_WRITE.
		 */
		case VQACMD_WRITE:
			error = 1;
			break;

		default:
			break;
	}

	return(error);
}
