/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// The VQA player's audio handler on top of the audio engine. The player
// pushes its sound track a block at a time and slaves the picture to the
// clock this handler answers. The names are the ones the player calls.

#pragma once

#include "vqaplay.h"


struct AhandleInitParams
{
	unsigned short SampleRate;
	unsigned char Channels;
	unsigned char BitsPerSample;
	unsigned long Flags;
	void * Callback1;
	void * Callback2;
};

typedef long (__cdecl * AHANDLE_CALLBACK_1)(VQAHandle * vqa);
typedef long (__cdecl * AHANDLE_CALLBACK_2)(VQAHandle * vqa, void * buffer);

unsigned long __cdecl Simple_Timer_Callback_Audio_Handler(VQAHandle * vqa);
unsigned long __cdecl Timer_Callback_Audio_Handler(VQAHandle * vqa);

long __cdecl Lock_Audio_Handler(void);
long __cdecl Unlock_Audio_Handler(void);
long __cdecl Stream_Audio_Handler(VQAHandle * vqa, long action, void * buffer, long nbytes);
