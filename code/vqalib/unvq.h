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

#ifndef VQAUNVQ_H
#define VQAUNVQ_H

#include <cstddef>
#include <cstdint>
/****************************************************************************
*
*         C O N F I D E N T I A L -- W E S T W O O D  S T U D I O S
*
*----------------------------------------------------------------------------
*
* PROJECT
*     VQAPlay32 library. (32-Bit protected mode)
*
* FILE
*     unvq.h
*
* DESCRIPTION
*     VQ frame decompress definitions.
*
* PROGRAMMER
*     Denzil E. Long, Jr.
*
* DATE
*     Feburary 8, 1995
*
****************************************************************************/

/*---------------------------------------------------------------------------
 * FUNCTION PROTOTYPES
 *-------------------------------------------------------------------------*/

void __cdecl UnVQ2_C1_4x4(uint8_t *codebook, uint8_t *pointers,
		uint8_t *buffer, size_t blocksperrow,
		size_t numrows, size_t bufwidth);

void __cdecl UnVQ1_C4_4x4(uint8_t *codebook, uint8_t *pointers,
		uint8_t *buffer, size_t blocksperrow,
		size_t numrows, size_t bufwidth);

void __cdecl UnVQ2_C4_4x4(uint8_t *codebook, uint8_t *pointers,
		uint8_t *buffer, size_t blocksperrow,
		size_t numrows, size_t bufwidth);

void __cdecl UnVQ1_C4_4x2(uint8_t *codebook, uint8_t *pointers,
		uint8_t *buffer, size_t blocksperrow,
		size_t numrows, size_t bufwidth);

void __cdecl UnVQ2_C4_4x2(uint8_t *codebook, uint8_t *pointers,
		uint8_t *buffer, size_t blocksperrow,
		size_t numrows, size_t bufwidth);

void __cdecl UnVQ2_C0_4x4_TRANS(uint8_t *codebook, uint8_t *pointers,
		uint8_t *buffer, size_t blocksperrow,
		size_t numrows, size_t bufwidth);

void __cdecl UnVQ2_C0_4x4_KEY(uint8_t *codebook, uint8_t *pointers,
		uint8_t *buffer, size_t blocksperrow,
		size_t numrows, size_t bufwidth);

void __cdecl UnVQ2_C0_4x4_TRANS_HALF(uint8_t *codebook, uint8_t *pointers,
		uint8_t *buffer, size_t blocksperrow,
		size_t numrows, size_t bufwidth);

void __cdecl UnVQ2_C0_4x2_TRANS(uint8_t *codebook, uint8_t *pointers,
		uint8_t *buffer, size_t blocksperrow,
		size_t numrows, size_t bufwidth);

void __cdecl UnVQ2_C0_4x2_KEY(uint8_t *codebook, uint8_t *pointers,
		uint8_t *buffer, size_t blocksperrow,
		size_t numrows, size_t bufwidth);


#ifdef __cplusplus
extern "C" {
#endif
void __cdecl UnVQ1_C1_TABLE(uint8_t *codebook, uint8_t *pointers,
		uint8_t *buffer, size_t blocksperrow,
		size_t numrows, size_t bufwidth);

void __cdecl UnVQ1_C1_TABLE_ALT(uint8_t *codebook, uint8_t *pointers,
		uint8_t *buffer, size_t blocksperrow,
		size_t numrows, size_t bufwidth);

void __cdecl UnVQ_4x2(uint8_t *codebook, uint8_t *pointers,
		uint8_t *buffer, size_t blocksperrow,
		size_t numrows, size_t bufwidth);

void __cdecl UnVQ_4x4(uint8_t *codebook, uint8_t *pointers,
		uint8_t *buffer, size_t blocksperrow,
		size_t numrows, size_t bufwidth);

void __cdecl UnVQ_4x4_HALF(uint8_t *codebook, uint8_t *pointers,
		uint8_t *buffer, size_t blocksperrow,
		size_t numrows, size_t bufwidth);

void __cdecl UnVQ1_C1_4x4(uint8_t *codebook, uint8_t *pointers,
		uint8_t *buffer, size_t blocksperrow,
		size_t numrows, size_t bufwidth);

#ifdef __cplusplus
}
#endif

#endif /* VQAUNVQ_H */
