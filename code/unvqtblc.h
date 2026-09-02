/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include <cstddef>
#include <cstdint>

bool Hicolor_Init_Table(int cmode);
void Hicolor_Clear_Table(void);
void Hicolor_Translate(void *buffer, int size);

void __cdecl UnVQ2_4x4_Table(uint8_t *codebook, uint8_t *pointers, uint8_t *buffer, size_t blocksperrow, size_t numrows, size_t bufwidth);
void __cdecl UnVQ2_4x2_Table(uint8_t *codebook, uint8_t *pointers, uint8_t *buffer, size_t blocksperrow, size_t numrows, size_t bufwidth);

void __cdecl UnVQ1_4x4_Table(uint8_t *codebook, uint8_t *pointers, uint8_t *buffer, size_t blocksperrow, size_t numrows, size_t bufwidth);
void __cdecl UnVQ1_4x2_Table(uint8_t *codebook, uint8_t *pointers, uint8_t *buffer, size_t blocksperrow, size_t numrows, size_t bufwidth);
