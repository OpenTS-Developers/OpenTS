/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "always.h"

#include "movies.h"

#include "_map.h"
#include "_surface.h"
#include "dsaudio.h"
#include "dsurface.h"
#include "surface.h"
#include "theme.h"
#include "vqa.h"
#include "vqoption.h"
#include "win.h"
#include "video.h"


DynamicVectorClass<char const *> Movies;

VQHandle *CurrentVQ = NULL;
int MovieInt1 = 0;


/// <summary>
/// Locks the drawing surface for the movie player.
/// This routine is handed to the movie player as its buffer callback. As well as mapping the
/// surface in, it tells the player how that surface is laid out and where within it the movie
/// frame belongs.
/// </summary>
/// <returns>Returns with a pointer to the locked surface memory.</returns>
void * Movie_Lock_Surface(void)
{
	Surface *surf = CurrentVQ->DrawSurface;
	void *buffptr = surf->Lock();
	VQAClass * vqa = CurrentVQ->VQA;

	vqa->Set_Draw_Buffer(0,
		surf->Stride() / surf->Bytes_Per_Pixel(),
		surf->Get_Height(),
		CurrentVQ->InitialRect.X,
		CurrentVQ->InitialRect.Y
	);

	return(buffptr);
}


/// <summary>
/// Releases the movie's drawing surface.
/// This is the other half of Movie_Lock_Surface, called by the movie player once it has
/// finished writing the frame.
/// </summary>
/// <returns>bool; Was the surface released?</returns>
bool Movie_Unlock_Surface(void)
{
	return(CurrentVQ->DrawSurface->Unlock());
}


/// <summary>
/// Copies the finished movie frame to the screen.
/// This routine is handed to the movie player as its frame callback when the movie is playing
/// fullscreen, so each frame appears as soon as it is decoded instead of waiting on the
/// game's own screen update.
/// </summary>
void Movie_Blit_To_Screen(void)
{
	if (!Get_Option(OPTION_NO_BUFFER)) {
		Rect area;
		area.Set(CurrentVQ->StretchRect.X, CurrentVQ->StretchRect.Y,
			CurrentVQ->StretchRect.Width, CurrentVQ->StretchRect.Height);

		VisibleSurface->Blit_From(
			area,
			*CurrentVQ->DrawSurface,
			CurrentVQ->InitialRect
		);

		Video_Present_If_Dirty();
	}
}


/// <summary>
/// Creates a playable movie from the file specified.
/// This routine opens the movie, matches its color mode to the display, and works out where
/// on the surface its frames will land. The handle that comes back is then given to
/// Movie_Play, or to Movie_Queue_Ingame for the little movies that play in the radar.
/// </summary>
/// <param name="name">Name of the movie file to play.</param>
/// <param name="surface">The surface the movie frames will be drawn to.</param>
/// <param name="rect1">The part of the movie frame to show. An empty rectangle takes the
/// whole frame.</param>
/// <param name="rect2">The area of the surface to show it in. An empty rectangle centers the
/// movie on the surface.</param>
/// <param name="volume">Volume to play the movie's sound track at.</param>
/// <param name="fullscreen">Should each finished frame go straight to the screen rather than
/// wait for the normal screen update?</param>
/// <returns>Returns with a pointer to the movie created. Otherwise, NULL is returned if the
/// file is missing or the movie could not be opened.</returns>
VQHandle * Movie_Create(char const * name, Surface * surface, Rect rect1, Rect rect2, int volume, bool fullscreen)
{
	VQA_SURF_DRAW_CALLBACK callback = NULL;
	VQHandle *handle = new VQHandle;
	if (handle != NULL) {
		if (!CCFileClass(name).Is_Available()) {
			return(NULL);
		}
		int flags = 0;

		if (!AudioEngine.Is_Available()) {
			Set_Option(OPTION_NO_AUDIO);
		}

		if (Get_Option(OPTION_PLAY_FROM_MIXFILE)) {
			flags |= VQACF_PLAY_FROM_MIXFILE;
		}

		if (MovieInt1 == 1) {
			flags |= VQACF_2;
		}

		if (fullscreen == true) {
			callback = Movie_Blit_To_Screen;
		}

		handle->VQA = new VQAClass(name, flags, Movie_Lock_Surface, Movie_Unlock_Surface, callback);
		if (handle->VQA == NULL) {
			delete handle;
			return(NULL);
		}

		handle->VQA->Set_VQA_Volume(volume);

		if (!handle->VQA->Open_And_Load_Buffers()) {
			delete handle;
			return(NULL);
		}

		int width = handle->VQA->Get_VQA_Width();
		int height = handle->VQA->Get_VQA_Height();
		int cmode = handle->VQA->Get_Desired_Color_Mode();

		if (cmode == 4) {
			cmode = 1;
		}

		if (rect1 == Rect(0,0,0,0)) {
			rect1 = Rect(Point2D(0, 0), width, height);
		}

		if (rect2 == Rect(0,0,0,0)) {
			rect2.Set((surface->Get_Width() - width) / 2, (surface->Get_Height() - height) / 2, width, height);
		}

		Rect irect1(0, 0, width, height);
		Rect irect2(0, 0, surface->Get_Width(), surface->Get_Height());

		handle->InitialRect = Intersect(rect1, irect1);
		handle->StretchRect = Intersect(rect2, irect2);

		if (cmode == 1 || cmode == 4) {
			handle->VQA->Set_Primary_Color_Mode(DSurface::Get_Primary_Color_Mode());
		}

		handle->DrawSurface = surface;
		handle->VQA->Set_Draw_Buffer(NULL, surface->Stride() / surface->Bytes_Per_Pixel(), surface->Get_Height());
		handle->IsInitialized = true;
		return(handle);
	}
	return(NULL);
}


/// <summary>
/// Shuts a movie down and frees what it was playing.
/// The handle is left behind as an empty shell that can no longer be played.
/// </summary>
/// <remarks>The handle itself is not freed -- the caller still owns it.</remarks>
void Movie_Destroy(VQHandle * handle)
{
	if (handle != NULL) {
		if (handle->VQA != NULL) {
			handle->VQA->Close_And_Free_VQA();
			delete handle->VQA;
			handle->VQA = NULL;
		}
		handle->IsInitialized = false;
	}
}


/// <summary>
/// Plays a movie through to the end.
/// This routine keeps the screen until the movie runs out or the player breaks out of it,
/// repeating the movie as many times as its handle asks for. Nothing happens if some other
/// movie already has the screen.
/// </summary>
/// <param name="hide_mouse">Should the mouse be hidden for the duration?</param>
/// <param name="theme">Music to queue up behind the movie, or THEME_NONE for none.</param>
/// <param name="user_break_not_allowed">Should the player be prevented from cutting the movie
/// short?</param>
void Movie_Play(VQHandle *movie, bool hide_mouse, ThemeType theme, bool user_break_not_allowed)
{
	if (CurrentVQ == NULL) {
		movie->IsInitialized = true;
		CurrentVQ = movie;
		int start_frame = 0;
		int end_frame = -1;
		int iterations = 0;
		if (Get_Option(OPTION_USE_LOOPS)) {
			start_frame = movie->StartFrame;
			if (movie->EndFrame >= 0) {
				end_frame = movie->EndFrame;
			}
			iterations = movie->Iterations;
			if (movie->PlayMode == 1) {
				if (movie->LoopMode == 0) {
					movie->VQA->Set_Loop(movie->LoopID, movie->Iterations);
				} else {
					movie->VQA->Set_Loop(start_frame, end_frame, movie->Iterations);
				}
			}
		}

		if (hide_mouse) {
			Hide_Mouse();
		}

		if (theme != THEME_NONE) {
			Theme.Queue_Song(theme);
		}

		while (true) {
			if (movie->PlayMode == 2 || start_frame != 0) {
				movie->VQA->Seek_To_Frame(start_frame);
			}
			int ret = movie->VQA->Play_VQA(end_frame, user_break_not_allowed);
			if (movie->PlayMode == 1) {
				iterations = 0;
			}
			movie->VQA->Reset_VQA();
			if (ret == 1) {
				break;
			}
			if (iterations > 0) {
				iterations--;
			}
			if (iterations == 0) {
				break;
			}
		}

		if (hide_mouse) {
			Show_Mouse();
		}

		CurrentVQ = NULL;
	}
}


/// <summary>
/// Advances a movie by a single frame.
/// This routine is used by the radar window and the mission screen animations, which run
/// their movies alongside the rest of the game rather than handing the whole movie over to
/// Movie_Play. Only one movie may be running at a time.
/// </summary>
/// <param name="finished">Set to true when the movie has run out and its handle can be torn
/// down.</param>
/// <returns>bool; Was a new frame drawn?</returns>
bool Movie_Advance_Frame(VQHandle * handle, bool &finished)
{
	if (CurrentVQ != NULL) {
		return(false);
	}
	CurrentVQ = handle;
	bool res = handle->VQA->Advance_Frame(finished);
	CurrentVQ = NULL;
	return(res);
}


/// <summary>
/// Determines if a movie currently owns the screen.
/// </summary>
/// <returns>bool; Is a movie playing at the moment?</returns>
bool Movie_Is_Playing(void)
{
	return(CurrentVQ != NULL);
}


/// <summary>
/// Pauses the movie where it stands.
/// This routine is used when the game loses the player's attention -- a dialog opens or the
/// window goes out of focus -- so that the movie does not run on unwatched.
/// </summary>
void Movie_Pause(VQHandle * handle)
{
	if (handle != NULL) {
		if (handle->VQA != NULL) {
			handle->VQA->Pause_VQA();
		}
	}
}


/// <summary>
/// Resumes a movie that was paused.
/// Playback picks up where it left off, so nothing of the movie is lost while the game was
/// looking elsewhere.
/// </summary>
void Movie_Resume(VQHandle * handle)
{
	if (handle != NULL) {
		if (handle->VQA != NULL) {
			handle->VQA->Resume_VQA();
		}
	}
}


/// <summary>
/// Queues a movie up to play in the radar window.
/// This routine positions the movie over the radar and hands it to the radar, which walks it
/// along a frame at a time as part of its normal update. Use this rather than Movie_Play for
/// the EVA messages that appear during a mission.
/// </summary>
void Movie_Queue_Ingame(VQHandle * handle)
{
	if (handle != NULL && handle->VQA != NULL) {
		handle->InitialRect.X = Map.RadX + Map.RadOffX;
		handle->InitialRect.Y = Map.RadY + Map.RadOffY;
		handle->StretchRect = handle->InitialRect;
		IngameVQ.Add(handle);
	}
}


/// <summary>
/// Rebuilds the screen underneath the movie that is playing.
/// This routine is used when something has disturbed the display while a movie owns the
/// screen. The game screen is redrawn onto the alternate surface and the movie is put back
/// over the top of it. It does nothing if no movie is playing.
/// </summary>
void Movie_Update_Visible_Surface(void)
{
	if (CurrentVQ != NULL) {
		AlternateSurface->Fill(0);
		Update_Visible_Surface(AlternateSurface);
		Movie_Blit_To_Screen();
	}
}


/// <summary>
/// Constructor for an empty movie handle.
/// The handle starts out with no movie attached and no playback settings of interest.
/// Movie_Create is what fills one in.
/// </summary>
VQHandle::VQHandle(void) :
	VQA(NULL),
	field_4(-1),
	DrawSurface(NULL),
	PlayMode(0),
	StartFrame(0),
	EndFrame(0),
	LoopMode(0),
	Iterations(1),
	LoopID(0),
	InitialRect(0,0,0,0),
	StretchRect(0,0,0,0),
	field_44(false),
	IsInitialized(false)
{
}


/// <summary>
/// Destructor for the movie handle.
/// Any movie still attached is shut down and thrown away with the handle.
/// </summary>
VQHandle::~VQHandle(void)
{
	if (VQA != NULL) {
		VQA->Close_And_Free_VQA();
		if (VQA != NULL) {
			delete VQA;
		}
	}
}
