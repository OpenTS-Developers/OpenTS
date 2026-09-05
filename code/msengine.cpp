/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "always.h"

#include "msengine.h"

#include "_surface.h"
#include "conquer.h"
#include "dbgprint.h"
#include "dsaudio.h"
#include "globals.h"
#include "goptions.h"
#include "video.h"
#include "msanim.h"
#include "msgloop.h"
#include "mssfx.h"
#include "rect.h"
#include "surface.h"
#include "win.h"

#include "color.hh"

#include <algorithm>
#include <climits>


/// <summary>
/// Creates an empty animation engine.
/// The list of pending update regions is sized here. The animations and sound effects
/// are supplied by whichever screen is driving the engine.
/// </summary>
MSEngine::MSEngine(void)
{
	RectCount = 0;

	Rects.Resize(20);
	Anims.Clear();
	Sounds.Clear();
}


/// <summary>
/// Destroys the animation engine.
/// Every animation and sound effect handed to the engine is destroyed along with it.
/// </summary>
MSEngine::~MSEngine(void)
{
	int index;

	for (index = 0; index < Sounds.Count(); index++) {
		delete Sounds[index];
	}
	Sounds.Clear();

	for (index = 0; index < Anims.Count(); index++) {
		delete Anims[index];
	}
	Anims.Clear();

	Rects.Clear();
}


// Set when the whole display is to be rebuilt from the alternate surface on the next
// advance, which is how the screen comes back after a dialog has covered it.
static bool _RebuildFromAlternate = false;


/// <summary>
/// Adds an animation to the engine.
/// The engine takes ownership of the animation and will advance and draw it from now
/// on, disposing of it when it finishes or when it is removed.
/// </summary>
/// <returns>Returns with the identifier of the animation, or -1 if it could not be
/// added.</returns>
int MSEngine::Add_Animation(MSAnim * anim)
{
	if (anim != NULL) {
		if (Anims.Add(anim)) {
			return(Anims.ID(anim));
		}
	}
	return(-1);
}


/// <summary>
/// Swaps one animation for another.
/// The replacement takes the old animation's place in the draw order, so whatever was
/// drawn over the old one stays on top of the new one. This is how a screen exchanges
/// artwork -- a highlighted button for a plain one, say.
/// </summary>
/// <param name="new_anim">The animation to put in place.</param>
/// <param name="old_anim">The animation to be replaced; it is destroyed.</param>
void MSEngine::Replace_Anim(MSAnim * new_anim, MSAnim * old_anim)
{
	Anims.Insert_After(Anims.ID(old_anim), new_anim);
	Remove_Anim(old_anim);
}


/// <summary>
/// Removes an animation from the engine and destroys it.
/// An animation the engine is not running is left alone, so a caller need not keep
/// track of which of its animations are still active.
/// </summary>
void MSEngine::Remove_Anim(MSAnim * anim)
{
	if (anim) {
		if (Anims.ID(anim) != -1) {
			Anims.Delete(anim);
			if (anim) {
				delete anim;
			}
		}
	}
}


/// <summary>
/// Waits for an animation to play out.
/// The screen keeps running while the wait lasts, so the other animations carry on and
/// the window stays responsive. The wait ends when the animation reports that it has
/// finished, or when it is no longer one the engine is running.
/// </summary>
/// <param name="anim">The animation to wait upon.</param>
void MSEngine::Wait_For_Anim(MSAnim * anim, unsigned delay)
{
	CDTimerClass<SystemTimerClass> timer = delay;

	while (true) {
		int id = Anims.ID(anim);
		if (id == -1) {
			break;
		}

		if (Anims[id]->Has_Finished()) {
			break;
		}

		if (delay <= 0) {
			break;
		}

		Wait_Delay(1);
	}
}


/// <summary>
/// Rebuilds the whole display and advances the animations.
/// Use this routine when the screen has been trampled on -- by a dialog, or by the
/// surfaces being lost -- and the entire picture has to be put back rather than just
/// the regions the animations touched.
/// </summary>
void MSEngine::Restore_And_Advance(void)
{
	_RebuildFromAlternate = true;
	Advance(HiddenSurface);
}


/// <summary>
/// Advances the animations by one frame.
/// Every animation is stepped, the region it disturbed is queued for the next blit,
/// and any animation that has run its course is disposed of. If the surfaces were lost
/// since the last call, the whole display is rebuilt from the alternate surface first.
/// </summary>
/// <param name="surface">The surface the animations are to draw onto.</param>
void MSEngine::Advance(Surface * surface)
{
	if (_RebuildFromAlternate == true) {
		_RebuildFromAlternate = false;
		surface->Fill(TBLACK);
		Rect rect = AlternateSurface->Get_Rect();
		rect.X = (surface->Get_Width() - rect.Width) / 2;
		rect.Y = (surface->Get_Height() - rect.Height) / 2;
		surface->Blit_From(AlternateSurface->Get_Rect(), *AlternateSurface, rect);
		for (int i = 0; i < Anims.Count(); i++) {
			Anims[i]->Redraw(surface);
		}
		RectCount = 0;
		Add_Update_Rect(surface->Get_Rect());
		Do_Custom_Draw(surface);
	}

	for (int i = 0; i < Anims.Count(); i++) {
		Rect rect;
		if (Anims[i]->Advance(surface, rect) == true) {
			delete Anims[i];
			Anims.Delete_Index(i);
		}
		Add_Update_Rect(rect);
	}
}


/// <summary>
/// Do the two rectangles overlap?
/// This is the test Add_Update_Rect uses to decide whether two dirty regions are worth
/// combining. Rectangles that merely abut count as overlapping, since merging them
/// costs nothing.
/// </summary>
inline bool Overlaps(Rect const & rect1, Rect const & rect2)
{
	bool overlapping = false;
	if (rect1.X < rect2.X) {
		if (rect1.X + rect1.Width >= rect2.X) {
			overlapping = true;
		}
	} else {
		if (rect2.X + rect2.Width >= rect1.X) {
			overlapping = true;
		}
	}

	if (rect1.Y < rect2.Y) {
		if (rect1.Y + rect1.Height < rect2.Y) {
			overlapping = false;
		}
	} else {
		if (rect2.Y + rect2.Height < rect1.Y) {
			overlapping = false;
		}
	}
	return(overlapping);
}


/// <summary>
/// Adds a region to the list of screen areas due to be blitted.
/// Regions that touch one another are combined, so the same pixels are not sent to the
/// screen twice. The list has a limited capacity; once it is full the new region is
/// folded into whichever entry it costs the least area to grow.
/// </summary>
/// <param name="rect">The region of the screen that needs updating.</param>
void MSEngine::Add_Update_Rect(Rect const & rect)
{
	int index;

	if (!rect.Is_Valid()) return;

	for (index = 0; index < RectCount; index++) {
		Rect & update_rect = Rects[index];
		if (Overlaps(rect, update_rect) == true) {
			int rect_right = rect.X + rect.Width;
			int rect_bottom = rect.Y + rect.Height;
			int i_rect_right = update_rect.X + update_rect.Width;
			int i_rect_bottom = update_rect.Y + update_rect.Height;

			Rect new_rect;
			new_rect.X = std::min(rect.X, update_rect.X);
			new_rect.Y = std::min(rect.Y, update_rect.Y);
			new_rect.Width = rect_right > i_rect_right ? rect_right - new_rect.X : i_rect_right - new_rect.X;
			new_rect.Height = rect_bottom > i_rect_bottom ? rect_bottom - new_rect.Y : i_rect_bottom - new_rect.Y;

			RectCount--;
			for (; index < RectCount; index++) {
				Rects[index] = Rects[index + 1];
			}

			Add_Update_Rect(new_rect);
			return;
		}
	}

	if (RectCount < Rects.Length()) {
		Rects[RectCount] = rect;
		RectCount++;
		return;
	}

	unsigned min_size = UINT_MAX;
	Rect min_rect(0, 0, 0, 0);
	int min_id = 0;

	for (index = 0; index < RectCount; index++) {
		Rect union_rect;

		int i_rect_left = Rects[index].X;
		int i_rect_width = Rects[index].Width;
		int i_rect_right = i_rect_left + i_rect_width;
		int i_rect_top = Rects[index].Y;
		int i_rect_height = Rects[index].Height;
		int i_rect_bottom = i_rect_top + i_rect_height;

		int union_x = std::min(rect.X, i_rect_left);
		int union_y = std::min(rect.Y, i_rect_top);
		int union_width = std::max(rect.X + rect.Width, i_rect_right);
		int union_height = i_rect_bottom;
		union_rect.Width = union_width - union_x;
		union_height = std::max(rect.Y + rect.Height, union_height);
		union_height -= union_y;
		union_rect.Height = union_height;
		union_width = union_rect.Width;
		union_height *= union_width;

		if ((unsigned)union_height < min_size) {
			min_size = union_height;
			min_id = index;
			min_rect.Set(union_x, union_y, union_width, union_rect.Height);
		}
	}

	Rects[min_id] = min_rect;
}


/// <summary>
/// Blits every pending update region to the visible surface.
/// The pending list is emptied as a result.
/// </summary>
/// <param name="surface">The surface holding the freshly drawn frame.</param>
void MSEngine::Blit_All(Surface * surface)
{
	if (RectCount > 0) {

		for (int i = 0; i < RectCount; i++) {
			VisibleSurface->Blit_From(Rects[i], *surface, Rects[i]);
		}

		RectCount = 0;

		Video_Present_If_Dirty();
	}
}


/// <summary>
/// Blits a single region to the visible surface.
/// Use this routine to push one region to the screen straight away rather than letting
/// it wait for the next Blit_All.
/// </summary>
/// <param name="surface">The surface to copy from.</param>
/// <param name="rect">The region of the surface to copy.</param>
void MSEngine::Blit_Rect(Surface * surface, Rect const & rect)
{
	if (rect.Is_Valid()) {

		VisibleSurface->Blit_From(rect, *surface, rect);

		Video_Present_If_Dirty();
	}
}


/// <summary>
/// Runs the screen for the specified length of time.
/// This is the engine's idle loop. The animations are advanced and blitted and the
/// Windows message queue is pumped until the delay has run out, so the window stays
/// alive while a screen is merely waiting. Time spent out of focus does not count
/// against the delay.
/// </summary>
/// <param name="delay">Number of timer ticks to keep running for.</param>
void MSEngine::Wait_Delay(int delay)
{
	CDTimerClass<SystemTimerClass> timer = 1;

	do {
		do {
			Call_Back();
			Process_Idle();
			Advance(HiddenSurface);
			Blit_All(HiddenSurface);
			Windows_Message_Handler();

			if (!GameInFocus) {
				timer.Stop();
				Wait_For_Focus();
				timer.Start();
			}

			Sleep(0);

		} while (timer.Value() > 0);

		timer = 1;
		delay--;

	} while (delay != 0);
}


/// <summary>
/// Waits until the game window has the focus again.
/// The animations are paused for the duration so that they do not race ahead while
/// the player is away, and are resumed before this routine returns.
/// </summary>
void MSEngine::Wait_For_Focus(void)
{
	int index;

	if (GameInFocus != true) {

		DebugString("MSEngine - Pausing animations\n");
		for (index = 0; index < Anims.Count(); index++) {
			Anims[index]->Pause();
		}

		while (!GameInFocus) {
			DebugString("MSEngine - Sleeping\n");
			Sleep(500);
			Windows_Message_Handler();
		}

		DebugString("MSEngine - Resuming animations\n");
		for (index = 0; index < Anims.Count(); index++) {
			Anims[index]->Resume();
		}
	}
}


/// <summary>
/// Registers a sound effect with the engine.
/// The screen driving this engine names its sounds rather than its files, so that
/// Play_Sound_Effect can be given something readable at each call site.
/// </summary>
/// <param name="name">Name the sound effect will be played by.</param>
/// <param name="file_name">Name of the sample file to play.</param>
/// <returns>bool; Was the sound effect registered?</returns>
bool MSEngine::Add_Sound_Effect(char const * name, char const * file_name)
{
	if (name == NULL || file_name == NULL) {
		return(false);
	}

	if (Sounds.Add(new MSSfx(name, file_name))) {
		return(true);
	}

	return(false);
}


/// <summary>
/// Plays the sound effect registered under the specified name.
/// The request is ignored if there is no audio, or if nothing has been registered
/// under that name.
/// </summary>
/// <param name="name">Name the sound effect was registered under.</param>
/// <param name="volume">Volume to play at, before the player's sound option is applied.</param>
void MSEngine::Play_Sound_Effect(char const * name, int volume)
{
	if (name == NULL || !AudioEngine.Is_Available()) {
		return;
	}

	for (int i = 0; i < Sounds.Count(); i++) {
		MSSfx * sfx = Sounds[i];
		if (stricmp(name, sfx->Get_Name()) == 0) {
			sfx->Play(volume);
			break;
		}
	}
}


/// <summary>
/// Restores the animation imagery over a region of the screen.
/// Callers use this routine after switching animations on or off, so that whatever is
/// still meant to be showing in that region is put back before the next update.
/// </summary>
/// <param name="rect">The region of the screen that has been disturbed.</param>
void MSEngine::Restore_Anims(Rect const & rect)
{
	for (int i = 0; i < Anims.Count(); i++) {
		Anims[i]->Restore(rect);
	}
}
