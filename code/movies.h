/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include "rect.h"

#include "theme.hh"

class VQAClass;
class Surface;
struct VQHandle;
template<class T> class DynamicVectorClass;

void * Movie_Lock_Surface(void);
bool Movie_Unlock_Surface(void);
void Movie_Blit_To_Screen(void);
VQHandle * Movie_Create(char const * name, Surface * surface, Rect rect1, Rect rect2, int volume, bool fullscreen);
void Movie_Destroy(VQHandle * handle);
void Movie_Play(VQHandle *movie, bool hide_mouse, ThemeType theme, bool user_break_not_allowed);
bool Movie_Advance_Frame(VQHandle * handle, bool &finished);
bool Movie_Redraw_Paused_Frame(VQHandle * handle);
bool Movie_Is_Playing(void);
void Movie_Pause(VQHandle * handle);
void Movie_Resume(VQHandle * handle);
void Movie_Queue_Ingame(VQHandle * handle);
void Movie_Update_Visible_Surface(void);


struct VQHandle
{
	VQHandle(void);
	~VQHandle(void);

	/*
	 * Pointer to the player that decodes and shows this movie. It is NULL until the handle
	 * has been filled in, and NULL again once the movie has been destroyed.
	 */
	VQAClass *VQA;

	/// Unused
	int field_4;

	/*
	 * Pointer to the surface that the movie's frames are decoded onto before display.
	 */
	Surface * DrawSurface;

	/*
	 * This selects how the movie is played (0 - 2). Mode 1 hands the repetition over to the
	 * movie player's own looping, while mode 2 rewinds to the start frame before every pass.
	 */
	int PlayMode;

	/*
	 * These are the first and last frames of the movie to play. They are consulted only
	 * when looping is enabled, and an EndFrame below zero plays through to the end.
	 */
	int StartFrame;
	int EndFrame;

	/*
	 * This picks which kind of loop is set up when PlayMode calls for one. A zero runs the
	 * movie's own loop identified by LoopID; anything else loops over the StartFrame range.
	 */
	int LoopMode;

	/*
	 * This is the number of times the movie is played before it gives up the screen, and a
	 * negative count repeats it for as long as the player lets it run.
	 */
	int Iterations;

	/*
	 * This identifies which of the loops recorded in the movie file to run.
	 */
	int LoopID;

	/*
	 * These are the two halves of the movie's placement -- the part of the frame that is
	 * shown, in the frame's own coordinates, and the area of the display it is copied to.
	 */
	Rect InitialRect;
	Rect StretchRect;

	/// Unused
	bool field_44;

	/*
	 * If a movie is attached to this handle and ready to play, then this flag will be
	 * true. Destroying the movie clears it and leaves the handle an empty shell.
	 */
	bool IsInitialized;

};


extern VQHandle *CurrentVQ;

extern DynamicVectorClass<VQHandle *> IngameVQ;
