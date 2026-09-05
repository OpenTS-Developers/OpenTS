/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "always.h"

#include "msanim.h"

#include "_mixfile.h"
#include "_palette.h"
#include "_surface.h"
#include "ccfile.h"
#include "convert.h"
#include "data.h"
#include "dbgprint.h"
#include "draw.h"
#include "audio/audioengine.h"
#include "globals.h"
#include "goptions.h"
#include "mixfile.h"
#include "movies.h"
#include "msfont.h"
#include "ownrdraw.h"
#include "pcx.h"
#include "shapeset.h"
#include "srfcache.h"

#include <algorithm>


ShapeFlags_Type FadeStages[4] = { ShapeFlags_Type(SHAPE_WIN_REL | SHAPE_TRANSLUCENT75),
								  ShapeFlags_Type(SHAPE_WIN_REL | SHAPE_TRANSLUCENT50),
								  ShapeFlags_Type(SHAPE_WIN_REL | SHAPE_TRANSLUCENT25),
								  ShapeFlags_Type(SHAPE_WIN_REL | SHAPE_NORMAL) };


/// <summary>
/// Creates a shape drawer from the palette file named.
/// The palette is loaded and installed as the game palette, then a drawer is built
/// against the visible surface so that the anims have something to render their shapes
/// with. Be aware that the game palette is replaced as a side effect.
/// </summary>
/// <param name="palette_name">The name of the palette file to load.</param>
/// <returns>Returns with a pointer to the drawer created. If the palette cannot be
/// loaded, then NULL is returned.</returns>
ConvertClass * Create_Drawer(char const * palette_name)
{
	CCFileClass file;

	if (palette_name == NULL) {
		return(NULL);
	}

	file.Open(palette_name);
	if (!file.Is_Available()) {
		return(NULL);
	}

	PaletteClass * palette = (PaletteClass *)Load_Alloc_Data(file);
	if (palette == NULL) {
		return(NULL);
	}

	unsigned char * pal_ptr = ((unsigned char *)*palette);

	for (int gindex = 0; gindex < 256; gindex++) {
	CCPalette[gindex] = RGBClass(
		((unsigned char *)*palette)[gindex*3]<<2,
		((unsigned char *)*palette)[gindex*3+1]<<2,
		((unsigned char *)*palette)[gindex*3+2]<<2);
	}

	delete palette;

	return(new ConvertClass(CCPalette, CCPalette, *VisibleSurface));
}


/// <summary>
/// Destroys the anim.
/// </summary>
MSAnim::~MSAnim(void)
{
	//nothing
}


/// <summary>
/// Creates a shape anim from the shape file named.
/// The shape is fetched out of the mix file catalog where possible, and only loaded from
/// disk on its own account when the catalog does not carry it. The anim starts out set to
/// play every frame the shape holds.
/// </summary>
/// <param name="name">The name of the shape file to animate.</param>
/// <param name="drawer">The drawer to render the shape with.</param>
/// <param name="rate">The delay in game frames between animation frames.</param>
/// <param name="loop">Should the animation start over when it runs off the end?</param>
/// <param name="flags">The shape drawing flags to render with.</param>
MSShapeAnim::MSShapeAnim(char const * name, int x, int y, ConvertClass * drawer, int rate, bool loop, ShapeFlags_Type flags) :
	MSAnim(x, y),
	Drawer(drawer),
	Loop(loop),
	Rate(rate),
	CurFrame(0),
	ShapeFlags(flags),
	AllocLoaded(false)
{
	Shape = (ShapeSet *)MFCD::Retrieve(name);
	if (Shape == NULL) {
		CCFileClass file(name);
		Shape = (ShapeSet *)Load_Alloc_Data(file);
		AllocLoaded = true;
		DebugString("MSShapeAnim: AllocLoaded %s\n", name);
	}

	StartFrame = 0;
	StopFrame = Shape->Get_Count() - 1;
}


/// <summary>
/// Destroys the shape anim.
/// The shape data is only freed when this anim loaded it itself. A shape borrowed from
/// the mix file catalog is left alone.
/// </summary>
MSShapeAnim::~MSShapeAnim(void)
{
	if (Shape != NULL && AllocLoaded == true) {
		delete Shape;
	}
}


/// <summary>
/// Sets the frame the shape anim is displaying.
/// The area the shape occupies is restored from the backdrop first, so the frame being
/// left behind does not linger under the new one. A frame beyond the end of the shape is
/// clamped to the last one.
/// </summary>
/// <param name="frame">The shape frame to display.</param>
void MSShapeAnim::Set_Frame(unsigned frame)
{
	Rect rect = Get_Rect();
	HiddenSurface->Blit_From(rect, *AlternateSurface, rect);
	CurFrame = frame;
	if (CurFrame >= (unsigned)Shape->Get_Count()) {
		CurFrame = Shape->Get_Count() - 1;
	}
}


/// <summary>
/// Sets the first frame of the animation loop.
/// This is the frame a looping anim comes back to once it has run past its stop frame.
/// A frame beyond the end of the shape is clamped to the last one.
/// </summary>
/// <param name="frame">The frame the animation should start at.</param>
void MSShapeAnim::Set_Start_Frame(unsigned frame)
{
	StartFrame = frame;
	if (StartFrame >= (unsigned)Shape->Get_Count()) {
		StartFrame = Shape->Get_Count() - 1;
	}
}


/// <summary>
/// Sets the last frame of the animation loop.
/// The anim either loops back to its start frame or reports itself finished once it runs
/// past this one. A frame beyond the end of the shape is clamped to the last one.
/// </summary>
/// <param name="frame">The frame the animation should end at.</param>
void MSShapeAnim::Set_Stop_Frame(unsigned frame)
{
	StopFrame = frame;
	if (StopFrame >= (unsigned)Shape->Get_Count()) {
		StopFrame = Shape->Get_Count() - 1;
	}
}


/// <summary>
/// Advances the shape anim by one frame.
/// The area the previous frame occupied is restored from the backdrop and the next frame
/// is drawn in its place. A non-looping anim reports itself finished once it runs past
/// its stop frame.
/// </summary>
/// <param name="rect">Filled in with the area disturbed, for the caller to update.</param>
/// <returns>bool; Has the anim finished and become ready for deletion?</returns>
bool MSShapeAnim::Advance(Surface * surface, Rect & rect)
{
	if (Active == true && Timer == 0) {

		Timer = Rate;

		Rect draw_rect = Shape->Get_Rect(CurFrame);
		draw_rect.X += XPos;
		draw_rect.Y += YPos;
		if (ShapeFlags & SHAPE_CENTER) {
			draw_rect.X -= Shape->Get_Width() / 2;
			draw_rect.Y -= Shape->Get_Height() / 2;
		}

		surface->Blit_From(draw_rect, *AlternateSurface, draw_rect);

		if (++CurFrame > StopFrame) {
			if (!Loop) {
				rect = draw_rect;
				return(true);
			}
			CurFrame = StartFrame;
		}

		Draw_Shape(*surface, *Drawer, Shape, CurFrame, Point2D(XPos, YPos), surface->Get_Rect(), ShapeFlags_Type(ShapeFlags | SHAPE_WIN_REL));

		rect = Shape->Get_Rect(CurFrame);
		rect.X += XPos;
		rect.Y += YPos;
		if (ShapeFlags & SHAPE_CENTER) {
			rect.X -= Shape->Get_Width() / 2;
			rect.Y -= Shape->Get_Height() / 2;
		}

		rect = Union(rect, draw_rect);
		return(false);
	}

	rect.Set(0, 0, 0, 0);
	return(false);
}


/// <summary>
/// Draws the shape anim in its current state.
/// This routine is used when another anim has drawn over the area this one occupies.
/// </summary>
/// <param name="rect">The damaged region to draw within, or NULL to draw unconditionally.</param>
void MSShapeAnim::Redraw(Surface * surface, Rect const * rect)
{
	Rect shape_rect(0, 0, 0, 0);

	if (rect != NULL) {

		shape_rect = Shape->Get_Rect(CurFrame);
		shape_rect.X += XPos;
		shape_rect.Y += YPos;
		if (ShapeFlags & SHAPE_CENTER) {
			shape_rect.X -= Shape->Get_Width() / 2;
			shape_rect.Y -= Shape->Get_Height() / 2;
		}

		shape_rect = Intersect(*rect, shape_rect);

		if (!shape_rect.Is_Valid()) {
			return;
		}
	}

	if (Active) {
		Draw_Shape(*surface, *Drawer, Shape, CurFrame, Point2D(XPos, YPos), surface->Get_Rect(), ShapeFlags_Type(ShapeFlags | SHAPE_WIN_REL));
	}
}


/// <summary>
/// Fetches the area the shape anim currently occupies.
/// </summary>
/// <returns>Returns with the bounding rectangle of the frame on display.</returns>
Rect MSShapeAnim::Get_Rect(void) const
{
	Rect rect = Shape->Get_Rect(CurFrame);
	rect.X += XPos;
	rect.Y += YPos;
	if (ShapeFlags & SHAPE_CENTER) {
		rect.X -= Shape->Get_Width() / 2;
		rect.Y -= Shape->Get_Height() / 2;
	}
	return(rect);
}


/// <summary>
/// Creates a shape anim that fades itself in as it plays.
/// The shape is drawn through the translucency stages as the animation runs, so it
/// appears to materialize rather than simply arrive. Since it is see-through while it
/// fades, the siblings behind it have to be redrawn as it goes.
/// </summary>
/// <param name="name">The name of the shape file to animate.</param>
/// <param name="drawer">The drawer to render the shape with.</param>
/// <param name="rate">The delay in game frames between animation frames.</param>
/// <param name="flags">The shape drawing flags to render with.</param>
/// <param name="vector">The list of sibling anims to repair over this one.</param>
MSFadeAnim::MSFadeAnim(char const * name, int x, int y, ConvertClass * drawer, int rate, ShapeFlags_Type flags, MS_ANIM_LIST * vector) :
	MSShapeAnim(name, x, y, drawer, rate, false, flags),
	Anims(vector)
{
	//nothing
}


/// <summary>
/// Advances the fading shape anim by one frame.
/// The area the previous frame occupied is restored from the backdrop and every sibling
/// anim overlapping it is asked to repair itself, before the next frame goes down at
/// whatever translucency the fade has reached.
/// </summary>
/// <param name="rect">Filled in with the area disturbed, for the caller to update.</param>
/// <returns>bool; Has the anim finished and become ready for deletion?</returns>
bool MSFadeAnim::Advance(Surface * surface, Rect & rect)
{
	if (Active == true && Timer == 0) {

		Timer = Rate;

		ShapeFlags_Type flags;
		if (CurFrame < 4) {
			flags = ShapeFlags_Type(ShapeFlags | FadeStages[CurFrame]);
		} else {
			flags = ShapeFlags_Type(ShapeFlags | FadeStages[3]);
		}

		Rect draw_rect = Shape->Get_Rect(CurFrame);
		draw_rect.X += XPos;
		draw_rect.Y += YPos;
		if (flags & SHAPE_CENTER) {
			draw_rect.X -= Shape->Get_Width() / 2;
			draw_rect.Y -= Shape->Get_Height() / 2;
		}

		surface->Blit_From(draw_rect, *AlternateSurface, draw_rect);

		if (Anims != NULL) {
			int my_id = Anims->ID(this);
			for (int i = 0; i < Anims->Count(); i++) {
				if (i != my_id) {
					(*Anims)[i]->Redraw(surface, &draw_rect);
				}
			}
		}

		if (++CurFrame > StopFrame) {
			if (!Loop) {
				rect.Set(0, 0, 0, 0);
				return(true);
			}
			CurFrame = StartFrame;
		}

		if (CurFrame < 4) {
			flags = ShapeFlags_Type(ShapeFlags | FadeStages[CurFrame]);
		} else {
			flags = ShapeFlags_Type(ShapeFlags | FadeStages[3]);
		}

		Draw_Shape(*surface, *Drawer, Shape, CurFrame, Point2D(XPos, YPos), surface->Get_Rect(), flags);

		rect = Shape->Get_Rect(CurFrame);
		rect.X += XPos;
		rect.Y += YPos;
		if (flags & SHAPE_CENTER) {
			rect.X -= Shape->Get_Width() / 2;
			rect.Y -= Shape->Get_Height() / 2;
		}

		rect = Union(rect, draw_rect);
		return(false);
	}

	rect.Set(0, 0, 0, 0);
	return(false);
}


/// <summary>
/// Draws the fading shape anim in its current state.
/// This routine is used when another anim has drawn over the area this one occupies. The
/// shape is drawn at whatever translucency the fade has reached.
/// </summary>
/// <param name="rect">The damaged region to draw within, or NULL to draw unconditionally.</param>
void MSFadeAnim::Redraw(Surface * surface, Rect const * rect)
{
	Rect shape_rect(0, 0, 0, 0);

	if (rect != NULL) {

		shape_rect = Shape->Get_Rect(CurFrame);
		shape_rect.X += XPos;
		shape_rect.Y += YPos;
		if (ShapeFlags & SHAPE_CENTER) {
			shape_rect.X -= Shape->Get_Width() / 2;
			shape_rect.Y -= Shape->Get_Height() / 2;
		}

		shape_rect = Intersect(*rect, shape_rect);

		if (!shape_rect.Is_Valid()) {
			return;
		}
	}

	ShapeFlags_Type flags;
	if (CurFrame < 4) {
		flags = ShapeFlags_Type(ShapeFlags | FadeStages[CurFrame]);
	} else {
		flags = ShapeFlags_Type(ShapeFlags | FadeStages[3]);
	}

	Draw_Shape(*surface, *Drawer, Shape, CurFrame, Point2D(XPos, YPos), surface->Get_Rect(), flags);
}


/// <summary>
/// Creates an anim that fades a single shape frame into place.
/// Unlike its fading parent the shape does not animate -- one frame is faded up and then
/// stamped onto the backdrop, so it becomes part of the scene rather than something drawn
/// over it every pass.
/// </summary>
/// <param name="name">The name of the shape file to draw from.</param>
/// <param name="drawer">The drawer to render the shape with.</param>
/// <param name="rate">The delay in game frames between fade stages.</param>
/// <param name="vector">The list of sibling anims to repair over this one.</param>
/// <param name="persistent">Should the anim stay alive once the fade is done?</param>
/// <param name="frame">The shape frame to fade into place.</param>
MSOverlayAnim::MSOverlayAnim(char const * name, int x, int y, ConvertClass * drawer, int rate, MS_ANIM_LIST * vector, bool persistent, unsigned frame) :
	MSFadeAnim(name, x, y, drawer, rate, SHAPE_NORMAL, vector),
	Persistent(persistent),
	Frame(frame)
{
	//nothing
}


/// <summary>
/// Destroys the overlay anim.
/// </summary>
MSOverlayAnim::~MSOverlayAnim(void)
{

}


/// <summary>
/// Advances the overlay's fade by one stage.
/// Each pass draws the frame a little more solidly and asks the sibling anims to repair
/// themselves over it. Once the fade is done the frame is stamped onto the backdrop, so
/// it survives every restore that follows.
/// </summary>
/// <param name="rect">Filled in with the area disturbed, for the caller to update.</param>
/// <returns>bool; Has the anim finished and become ready for deletion?</returns>
bool MSOverlayAnim::Advance(Surface * surface, Rect & rect)
{
	if (Active == true && Timer == 0) {

		Timer = Rate;

		if (CurFrame < 4 && Frame >= 0) {

			rect = Shape->Get_Rect(Frame);
			rect.X += XPos;
			rect.Y += YPos;
			if (ShapeFlags & SHAPE_CENTER) {
				rect.X -= Shape->Get_Width() / 2;
				rect.Y -= Shape->Get_Height() / 2;
			}

			surface->Blit_From(rect, *AlternateSurface, rect);

			Draw_Shape(*surface, *Drawer, Shape, Frame, Point2D(XPos, YPos), surface->Get_Rect(), ShapeFlags_Type(ShapeFlags | FadeStages[CurFrame]));

			if (Anims != NULL) {
				int my_id = Anims->ID(this);
				for (int i = 0; i < Anims->Count(); i++) {
					if (i != my_id) {
						(*Anims)[i]->Redraw(surface, &rect);
					}
				}
			}

			if (++CurFrame >= 4 && Frame >= 0) {
				Draw_Shape(*AlternateSurface, *Drawer, Shape, Frame, Point2D(XPos, YPos), surface->Get_Rect(), ShapeFlags_Type(ShapeFlags | SHAPE_WIN_REL));
				return(!Persistent);
			}
		}

		return(false);
	}

	rect.Set(0, 0, 0, 0);
	return(false);
}


/// <summary>
/// Restarts the overlay's fade from the beginning.
/// </summary>
void MSOverlayAnim::Restart(void)
{
	CurFrame = 0;
}


/// <summary>
/// Sets the shape frame the overlay displays.
/// The fade starts over from the beginning, so the new frame appears the same way the
/// old one did.
/// </summary>
/// <param name="frame">The shape frame to fade into place, or a negative value to draw
/// nothing at all.</param>
void MSOverlayAnim::Set_Frame(int frame)
{
	CurFrame = 0;
	Frame = frame;
}


/// <summary>
/// Draws the overlay in its current state.
/// This routine is used when another anim has drawn over the area this one occupies. The
/// shape is drawn at whatever translucency the fade has reached.
/// </summary>
/// <param name="rect">The damaged region to draw within, or NULL to draw unconditionally.</param>
void MSOverlayAnim::Redraw(Surface * surface, Rect const * rect)
{
	Rect shape_rect(0, 0, 0, 0);

	if (!Active || Frame < 0) {
		return;
	}

	if (rect != NULL) {

		shape_rect = Shape->Get_Rect(Frame);
		shape_rect.X += XPos;
		shape_rect.Y += YPos;
		if (ShapeFlags & SHAPE_CENTER) {
			shape_rect.X -= Shape->Get_Width() / 2;
			shape_rect.Y -= Shape->Get_Height() / 2;
		}

		shape_rect = Intersect(*rect, shape_rect);

		if (!shape_rect.Is_Valid()) {
			return;
		}
	}

	int fading_stage = std::min(CurFrame, 3u);
	Draw_Shape(*surface, *Drawer, Shape, Frame, Point2D(XPos, YPos), surface->Get_Rect(), ShapeFlags_Type(ShapeFlags | FadeStages[fading_stage]));
}


/// <summary>
/// Stamps the overlay onto the backdrop surface.
/// This routine is used to make the overlay part of the scene, so that whatever is drawn
/// over it afterwards can be repaired without losing it.
/// </summary>
void MSOverlayAnim::Restore(Rect const & rect)
{
	int fading_stage = std::min(CurFrame, 3u);

	if (Active && Frame >= 0) {
		Draw_Shape(*AlternateSurface, *Drawer, Shape, Frame, Point2D(XPos, YPos), AlternateSurface->Get_Rect(), ShapeFlags_Type(ShapeFlags | FadeStages[fading_stage]));
	}
}


/// <summary>
/// Has the overlay finished fading into place?
/// </summary>
/// <returns>bool; Is the fade over, or is there nothing to fade?</returns>
bool MSOverlayAnim::Has_Finished(void) const
{
	return(!(Active && CurFrame < 4 && Frame >= 0));
}


/// <summary>
/// Creates an anim that plays a VQA movie.
/// The movie is centered on the surface handed over and plays at the current sound
/// volume. A PCX picture sharing the movie's name is loaded alongside it where one
/// exists, and serves as the still image the movie leaves behind when it ends.
/// </summary>
/// <param name="name">The name of the movie file to play.</param>
/// <param name="surface">The surface the movie is played onto.</param>
/// <param name="vector">The list of sibling anims to repair over the movie.</param>
/// <param name="persistent">Should the anim stay alive after the movie has ended?</param>
MSVQAnim::MSVQAnim(char const * name, Surface * surface, MS_ANIM_LIST * vector, bool persistent) :
	MSAnim(0, 0, false),
	Anims(vector),
	Movie(NULL),
	TargetSurface(surface),
	Background(NULL),
	Persistent(persistent),
	Done(false)
{
	if (name != NULL && surface != NULL) {
		Movie = Movie_Create(name, surface, Rect(0, 0, 0, 0), Rect(0, 0, 0, 0), 255, false);
		if (Movie != NULL) {
			Movie->InitialRect = Rect((surface->Get_Width() - 640) / 2, (surface->Get_Height() - 400) / 2, 640, 400);
			Movie->StretchRect = Rect((HiddenSurface->Get_Width() - 640) / 2, (HiddenSurface->Get_Height() - 400) / 2, 640, 400);
		}
		char pcx_name[64];
		strcpy(pcx_name, name);
		char *tok = strtok(pcx_name, ".");
		if (tok != NULL) {
			strcat(tok, ".PCX");
			CCFileClass file(pcx_name);
			if (file.Is_Available()) {
				Background = Read_PCX_File(file);
			}
		}
	}
}


/// <summary>
/// Destroys the movie anim.
/// The movie is shut down and the still image loaded alongside it is released.
/// </summary>
MSVQAnim::~MSVQAnim(void)
{
	if (Background != NULL) {
		delete Background;
	}

	if (Movie != NULL) {
		Movie_Destroy(Movie);
		delete Movie;
	}
}


/// <summary>
/// Advances the movie by one frame.
/// Every frame that arrives is put up on the surface and the sibling anims are asked to
/// repair themselves over it. When the movie runs out, the still image loaded alongside
/// it takes its place so the screen is not left blank.
/// </summary>
/// <param name="rect">Filled in with the area disturbed, for the caller to update.</param>
/// <returns>bool; Has the anim finished and become ready for deletion?</returns>
bool MSVQAnim::Advance(Surface * surface, Rect & rect)
{
	if (Movie != NULL) {
		bool is_done = false;
		if (!Done) {
			bool advanced = Movie_Advance_Frame(Movie, is_done);
			if (advanced == true) {
				Redraw(surface);
				rect = Movie->StretchRect;

				int my_id = Anims->ID(this);
				for (int i = 0; i < Anims->Count(); i++) {
					if (i != my_id) {
						(*Anims)[i]->Redraw(surface, &Movie->StretchRect);
					}
				}
			}

			if (is_done == true) {
				if (Background != NULL) {
					TargetSurface->Blit_From(Movie->InitialRect, *Background, Background->Get_Rect());
					Redraw(surface);
					rect = Movie->StretchRect;

					int my_id = Anims->ID(this);
					for (int i = 0; i < Anims->Count(); i++) {
						if (i != my_id) {
							(*Anims)[i]->Redraw(surface, &Movie->StretchRect);
						}
					}
				}
			}
		}

		if (is_done) {
			Done = is_done;
		}
		return(Done && !Persistent);
	}

	if (!Done) {
		if (Background != NULL) {
			Rect backdrop_rect = Background->Get_Rect();
			TargetSurface->Blit_From(backdrop_rect, *Background, backdrop_rect);
			Redraw(surface);
			rect = backdrop_rect;

			int my_id = Anims->ID(this);
			for (int i = 0; i < Anims->Count(); i++) {
				if (i != my_id) {
					(*Anims)[i]->Redraw(surface, &backdrop_rect);
				}
			}
		}
	}
	Done = true;

	return(!Persistent);
}


/// <summary>
/// Draws the movie's latest frame onto the surface specified.
/// This routine is used when another anim has drawn over the area the movie occupies.
/// A movie that has already run out draws nothing.
/// </summary>
/// <param name="rect">The damaged region to draw within, or NULL to draw unconditionally.</param>
void MSVQAnim::Redraw(Surface * surface, const Rect * rect)
{
	if (Movie != NULL && !Done) {
		if (rect == NULL || Intersect(*rect, Movie->StretchRect).Is_Valid()) {
			surface->Blit_From(Movie->InitialRect, *AlternateSurface, Movie->StretchRect);
		}
	}
}


/// <summary>
/// Restores the still image the finished movie left behind.
/// This routine is used to put the movie's parting picture back onto the backdrop after
/// something else has drawn over it.
/// </summary>
void MSVQAnim::Restore(const Rect & rect)
{
	if (Done && Movie != NULL && Background != NULL) {
		TargetSurface->Blit_From(Movie->InitialRect, *Background, Background->Get_Rect());
	}
}


/// <summary>
/// Fetches the area the movie occupies on the screen.
/// </summary>
/// <returns>Returns with the movie's screen rectangle. An empty rectangle is returned
/// when there is no movie to play.</returns>
Rect MSVQAnim::Get_Rect(void) const
{
	static Rect _rect_none(0,0,0,0);
	return(Movie != NULL ? Movie->StretchRect : _rect_none);
}


/// <summary>
/// Has the movie finished playing?
/// </summary>
/// <returns>bool; Has the movie run to its end?</returns>
bool MSVQAnim::Has_Finished(void) const
{
	return(Done);
}


/// <summary>
/// Creates a text printing anim from a string table entry.
/// The text is typed out a character at a time within the area specified. Use this
/// version when the words come from the game's string table rather than from code.
/// </summary>
/// <param name="text">The string table identifier of the text to print.</param>
/// <param name="font">The font to print the text with.</param>
/// <param name="rect">The area to center and clip the text within.</param>
/// <param name="start_delay">The delay in game frames before printing begins.</param>
/// <param name="print_delay">The delay in game frames between characters.</param>
/// <param name="fade_effect">Should the characters fade in as they are printed?</param>
/// <param name="center">Should the text be centered within its area?</param>
MSPrintAnim::MSPrintAnim(int text, int x, int y, MSFont * font, Rect const & rect, int start_delay, int print_delay, bool fade_effect, bool center) :
	MSAnim(x, y, true),
	String(Fetch_String(text)),
	PrintDelay(print_delay),
	Font(font),
	LineStart(0),
	PrintedCharCount(start_delay),
	CompletionCount(0),
	FadeEffect(fade_effect),
	Transient(false),
	Centered(center),
	Offset(0)
{
	Timer = start_delay;
	Set_Dimensions(rect);
	if (center) {
		Offset = (Area.Width - Get_Line_Width(String)) / 2;
	}
}


/// <summary>
/// Creates a text printing anim from the string handed over.
/// The text is typed out a character at a time within the area specified. Use this
/// version for text assembled in code rather than pulled from the string table.
/// </summary>
/// <param name="string">The text to print.</param>
/// <param name="font">The font to print the text with.</param>
/// <param name="rect">The area to center and clip the text within.</param>
/// <param name="start_delay">The delay in game frames before printing begins.</param>
/// <param name="print_delay">The delay in game frames between characters.</param>
/// <param name="fade_effect">Should the characters fade in as they are printed?</param>
/// <param name="center">Should the text be centered within its area?</param>
/// <remarks>The string is not copied. It must outlive the anim.</remarks>
MSPrintAnim::MSPrintAnim(char const * string, int x, int y, MSFont * font, Rect const & rect, int start_delay, int print_delay, bool fade_effect, bool center) :
	MSAnim(x, y, true),
	String(string),
	PrintDelay(print_delay),
	Font(font),
	LineStart(0),
	PrintedCharCount(start_delay),
	CompletionCount(0),
	FadeEffect(fade_effect),
	Transient(false),
	Centered(center),
	Offset(0)
{
	Timer = start_delay;
	Set_Dimensions(rect);
	if (center) {
		int width = 0;
		char const * str = String;
		int font_width = Font->Get_Font_Width();
		while (*str && *str != '\n') {
			width += Font->Get_Character_Width(*str);
			str++;
		}
		Offset = (Area.Width - font_width - width) / 2;
	}
}


/// <summary>
/// Establishes the rectangle the text will print within.
/// The string is measured with its font and the result is centered inside the area
/// handed over, then clipped to it. An invalid area leaves the text unclipped at the
/// anim's own position.
/// </summary>
/// <param name="rect">The area to center and clip the text within.</param>
void MSPrintAnim::Set_Dimensions(Rect const & rect)
{
	if (rect.Is_Valid()) {
		Font->Get_String_Rect(String, Area);

		XPos += (rect.Width - Area.Width) / 2;
		YPos += (rect.Height - (rect.Height / Font->Get_Font_Height() * Font->Get_Font_Height())) / 2;

		Area.X += XPos;
		Area.Y += YPos;

		Area = Intersect(rect, Area);
		XPos += Font->Get_Font_Width() / 2;

	} else {
		Font->Get_String_Rect(String, Area);
		Area.X = XPos;
		Area.Y = YPos;
	}
}


/// <summary>
/// Wraps a string to the line width given.
/// Spaces are turned into line breaks, in place, so that no line runs wider than the
/// width specified. Use the buffer aware version instead when the text may hold a
/// word too long to fit a line of its own.
/// </summary>
/// <param name="string">The text to wrap; line breaks are inserted in place.</param>
/// <param name="font">The font the text will be printed with.</param>
/// <param name="line_width">The width available for a single line, in pixels.</param>
/// <returns>Returns with the width of the widest line, in pixels.</returns>
int MSPrintAnim::Word_Wrap(char * string, MSFont * font, int line_width)
{
	if (string == NULL || font == NULL || line_width <= font->Get_Font_Width()) {
		return(0);
	}

	line_width -= font->Get_Font_Width();
	int max_width = 0;
	char * char_ptr = string;

	while (*char_ptr) {
		int current_width = 0;

		/// Calculate the width of the current line
		while (current_width < line_width && *char_ptr != '\0' && *char_ptr != '\n' && *char_ptr != '@') {
			int char_width = font->Get_Character_Width(*char_ptr++);
			current_width += char_width;
		}

		/// If the line exceeds the available width, backtrack to the last space
		if (current_width >= line_width) {
			while (*char_ptr != '\0' && *char_ptr != ' ' && *char_ptr != '\n' && char_ptr != string) {
				char_ptr--;
				current_width -= font->Get_Character_Width(*char_ptr);
			}
		}

		/// Update the maximum width encountered
		if (current_width > max_width) {
			max_width = current_width;
		}

		/// If we reached the end of the string, break
		if (!*char_ptr) {
			break;
		}

		/// Replace the current character with a newline and move to the next line
		*char_ptr = '\n';
		char_ptr++;
	}

	return(max_width);
}


/// <summary>
/// Wraps a string to the line width given, splitting long words if it must.
/// This is the buffer aware version of the word wrap routine. A word too long to fit
/// a line of its own is broken across two lines, which lengthens the string -- hence
/// the buffer size that the other version does without.
/// </summary>
/// <param name="string">The text to wrap; line breaks are inserted in place.</param>
/// <param name="length">The size of the buffer holding the string.</param>
/// <param name="font">The font the text will be printed with.</param>
/// <param name="line_width">The width available for a single line, in pixels.</param>
/// <returns>Returns with the width of the widest line, in pixels.</returns>
/// <remarks>Be sure the buffer has room to grow, or a long word will be truncated.</remarks>
int MSPrintAnim::Word_Wrap(char * string, int length, MSFont * font, int line_width)
{
	char * char_ptr = string;
	int max_width = 0;
	if (string == NULL || font == NULL || line_width <= font->Get_Font_Width()) {
		return(0);
	}

	line_width -= font->Get_Font_Width();
	unsigned int blen = length - strlen(string) - 1;

	while (*char_ptr) {
		int current_width = 0;
		char *start_ptr = char_ptr;

		/// Calculate the width of the current line
		while (current_width < line_width && *char_ptr != '\0' && *char_ptr != '\n' && *char_ptr != '@') {
			int char_width = font->Get_Character_Width(*char_ptr++);
			current_width += char_width;
		}

		/// If the line exceeds the available width, backtrack to the last space
		if (current_width >= line_width) {
			while (*char_ptr != '\0' && *char_ptr != ' ' && *char_ptr != '\n' && char_ptr != start_ptr) {
				char_ptr--;
				current_width -= font->Get_Character_Width(*char_ptr);
			}
		}

		if (char_ptr == start_ptr) {
			while (current_width < line_width && *char_ptr != '\0' && *char_ptr != ' ' && *char_ptr != '\n' && *char_ptr != '@' && *char_ptr != '-') {
				int char_width = font->Get_Character_Width(*char_ptr++);
				current_width += char_width;
			}

			if (current_width < line_width) {
				if (blen > 0) {
					blen--;
					char_ptr++;
					memmove(char_ptr + 1, char_ptr, strlen(char_ptr) + 1);
				}
			} else {
				char_ptr--;
				current_width -= font->Get_Character_Width(*char_ptr);
				if (blen > 0) {
					blen--;
					memmove(char_ptr + 1, char_ptr, strlen(char_ptr) + 1);
				} else {
					*(char_ptr + 1) = 0;
				}
			}
		}

		/// Update the maximum width encountered
		if (current_width > max_width) {
			max_width = current_width;
		}

		/// If we reached the end of the string, break
		if (!*char_ptr) {
			break;
		}

		/// Replace the current character with a newline and move to the next line
		*char_ptr = '\n';
		char_ptr++;
	}

	return(max_width);
}


/// <summary>
/// Breaks a string into pages of the height specified.
/// The line breaks that fall on a page boundary are turned into form feeds, in place,
/// so that the printing code knows where to pause.
/// </summary>
/// <param name="string">The text to paginate; modified in place.</param>
/// <param name="font">The font the text will be printed with.</param>
/// <param name="page_height">The height of a single page, in pixels.</param>
/// <returns>Returns with the number of page breaks inserted.</returns>
/// <remarks>The string must already be wrapped; this routine only breaks at a newline.</remarks>
int MSPrintAnim::Paginate(char * string, MSFont * font, int page_height)
{
	if (string != NULL && font != NULL && page_height >= font->Get_Font_Height()) {
		int overflow_count = 0; /// Count of height overflows
		int current_height = 0; /// Accumulated height
		char *str = string;
		if (str) {
			while (*str) {
				for (char c = *str; *str; c = *str) {
					if (c == '\n') {
						break;
					}
					str++;
				}

				if (*str == '\n') {
					current_height += font->Get_Font_Height();
					if (current_height >= page_height) {
						current_height = 0;
						overflow_count++;
						*str = '\f'; /// Replace newline with form feed
					}
					str++;
				}
			}
		}
		return(overflow_count);
	}
	return(0);
}


/// <summary>
/// Types out one more character of the text.
/// This routine prints the string a character at a time, trailing a short fade behind
/// the newest ones so the text appears to arrive over a teletype. When the text runs
/// past the bottom of its rectangle the topmost line is dropped and printing carries
/// on below.
/// </summary>
/// <param name="rect">Filled in with the region the text occupies.</param>
/// <returns>bool; Should this anim be removed from the list?</returns>
bool MSPrintAnim::Advance(Surface * surface, Rect & rect)
{
	rect.Set(0, 0, 0, 0);

	if (Timer != 0) return(false);

	if (Active != true) return(false);

	Timer = PrintDelay;

	/// We've reached the end of the string and let the facing anim finish
	if (PrintedCharCount > strlen(String) && CompletionCount > 2) {
		return(!Transient);
	}

	if (FadeEffect == true) {
		surface->Blit_From(Area, *AlternateSurface, Area);
	}

	bool should_center = Centered;
	int x = XPos;
	int y = YPos;

	for (unsigned char_index = LineStart; char_index <= PrintedCharCount; char_index++) {
		char ch = String[char_index];
		if (ch == '\0') {
			CompletionCount++;
			break;
		}

		if (should_center) {
			int font_width = Font->Get_Font_Width();
			int string_width = Get_Line_Width(String + char_index);
			Offset = (Area.Width - string_width - font_width) / 2;
			x += Offset;
			should_center = false;
		}

		if (ch == '\n') {
			should_center = Centered;
			x = XPos;
			y += Font->Get_Font_Height();

			if (y + Font->Get_Font_Height() - YPos > Area.Height) {
				int next_line_start = LineStart;
				while (String[next_line_start] && String[next_line_start] != '\n') {
					next_line_start++;
				}
				if (String[next_line_start] == '\n') {
					next_line_start++;
				}
				LineStart = next_line_start;
				break;
			}
		} else {
			Font->Draw_Character(surface, ch, x, y, (FadeEffect == false || PrintedCharCount - char_index >= 2) ? 2 : (PrintedCharCount - char_index), true);
			x += Font->Get_Character_Width(ch);
		}
	}

	rect = Area;
	PrintedCharCount++;

	return(false);
}


/// <summary>
/// Draws the text that has been typed out so far.
/// This routine is used to repair the anim after something has drawn over it. It
/// gives up rather than print past the bottom edge of the surface.
/// </summary>
/// <param name="rect">The damaged region to draw within, or NULL to draw unconditionally.</param>
void MSPrintAnim::Redraw(Surface * surface, Rect const * rect)
{
	if (rect == NULL || Intersect(*rect, Area).Is_Valid()) {
		bool should_center = Centered;
		int x = XPos;
		int y = YPos;

		unsigned printed = Get_Printed_Char_Count();
		unsigned end = std::min(printed, strlen(String));

		for (unsigned char_index = LineStart; char_index < end; char_index++) {
			if (String[char_index] == '\n') {
				should_center = Centered;
				x = XPos;
				y += Font->Get_Font_Height();
				if (y + Font->Get_Font_Height() >= surface->Get_Height()) {
					return;
				}
			} else {
				if (should_center) {
					int font_width = Font->Get_Font_Width();
					int string_width = Get_Line_Width(String + char_index);
					Offset = (Area.Width - string_width - font_width) / 2;
					x += Offset;
					should_center = false;
				}
				Font->Draw_Character(surface, String[char_index], x, y, 2, true);
				x += Font->Get_Character_Width(String[char_index]);
			}
		}
	}
}


/// <summary>
/// Brings up the next line of text.
/// Where the base class types the string out a character at a time, this routine puts
/// up a whole line at once and fades it in. A bleep sounds as each line lands, and
/// again when the last of the string has been printed.
/// </summary>
/// <param name="rect">Filled in with the region the current line occupies.</param>
/// <returns>bool; Has the whole string been printed?</returns>
bool MSWordAnim::Advance(Surface * surface, Rect & rect)
{
	if (Timer != 0 || Active != true) {
		rect.Set(0, 0, 0, 0);
		return(false);
	}

	Timer = PrintDelay;

	if (LineStart >= strlen(String)) {
		void const * sample = MFCD::Retrieve("BLEEP1.AUD");
		if (sample != NULL) {
			AudioEngine.Play_Sample(sample, AUDIO_GROUP_SFX, 64.0f / 255.0f, 10);
		}
		return(true);
	}

	if (FadeEffect == true && LineRect.Is_Valid()) {
		surface->Blit_From(LineRect, *AlternateSurface, LineRect);
	}

	int x = XPos;

	if (Centered) {
		x += Offset;
	}

	char character = String[LineStart];
	int line_end_index = LineStart + 1;
	while (character != '\n' && character != '\0') {
		Font->Draw_Character(surface, character, x, LineYPos, PrintedCharCount, false);
		x += Font->Get_Character_Width(character);
		character = String[line_end_index++];
	}

	rect.Set(Area.X, LineYPos, Area.Width, Font->Get_Font_Height());

	if (PrintedCharCount == 2) {
		LineRect.Set(0, 0, 0, 0);
		LineStart = line_end_index;
		if (character == '\n') {
			void const * sample = MFCD::Retrieve("BLEEP1.AUD");
			if (sample != NULL) {
				AudioEngine.Play_Sample(sample, AUDIO_GROUP_SFX, 64.0f / 255.0f, 10);
			}
			LineYPos += Font->Get_Font_Height();
		}
	} else {
		LineRect = rect;
	}

	if (FadeEffect == true) {
		PrintedCharCount++;
		if (PrintedCharCount > 2) {
			PrintedCharCount = 0;
		}
		return(false);
	}

	return(false);
}


/// <summary>
/// Fetches the screen rectangle the text prints within.
/// </summary>
/// <returns>Returns with the bounding rectangle the text was fitted to.</returns>
Rect MSPrintAnim::Get_Rect(void) const
{
	return(Area);
}


/// <summary>
/// Sets whether the text is a transient message.
/// A transient anim reports itself finished once its string has been printed, which
/// lets the owning screen decide when to retire it.
/// </summary>
void MSPrintAnim::Set_Transient(bool transient)
{
	if (transient != Transient) {
		Transient = transient;
	}
}


/// <summary>
/// Has the text finished printing itself?
/// Only a transient anim ever reports itself finished, and only once the whole string
/// has been typed out and the trailing fade has run its course.
/// </summary>
/// <returns>bool; Is the anim done with the screen?</returns>
bool MSPrintAnim::Has_Finished(void) const
{
	return(Transient && PrintedCharCount > strlen(this->String) && CompletionCount > 2);
}


/// <summary>
/// Determines the pixel width of a single line of text.
/// Measuring stops at the first newline, so the width returned is that of the line
/// beginning at the character handed over. This routine is used to center a line.
/// </summary>
/// <param name="str">Pointer to the first character of the line to measure.</param>
/// <returns>Returns with the width of the line in pixels.</returns>
int MSPrintAnim::Get_Line_Width(char const * str) const
{
	int width = 0;
	while (*str && *str != '\n') {
		width += Font->Get_Character_Width(*str);
		str++;
	}
	return(width);
}


/// <summary>
/// Draws the lines of text that have already been printed.
/// This routine is used to repair the anim after something has drawn over it. Only
/// the completed lines are put back -- the line currently being brought up is left to
/// the next advance.
/// </summary>
/// <param name="rect">The damaged region to draw within, or NULL to draw unconditionally.</param>
void MSWordAnim::Redraw(Surface * surface, Rect const * rect)
{
	if (rect == NULL || Intersect(*rect, Area).Is_Valid()) {
		int x = XPos;
		int y = YPos;

		unsigned end = LineStart;

		for (unsigned char_index = 0; char_index < end; char_index++) {
			if (String[char_index] == '\n') {
				x = XPos;
				y += Font->Get_Font_Height();
				if (y + Font->Get_Font_Height() >= surface->Get_Height()) {
					return;
				}
			} else {
				Font->Draw_Character(surface, String[char_index], x, y, 2, true);
				x += Font->Get_Character_Width(String[char_index]);
			}
		}
	}
}


/// <summary>
/// Creates a captioned button anim within the rectangle specified.
/// The button pulls its artwork out of the image cache handed over. It starts out
/// enabled and awaiting its first render.
/// </summary>
/// <param name="image_cache">The cache the button pulls its artwork from.</param>
/// <param name="rect">The screen area the button occupies.</param>
/// <param name="string">The caption to print on the button.</param>
/// <param name="height">The artwork height to request from the cache.</param>
/// <param name="left_cap_width">The width of the left hand end piece.</param>
/// <param name="right_cap_width">The width of the right hand end piece.</param>
MSButtonAnim::MSButtonAnim(SurfaceCacheClass * image_cache, Rect const & rect, char const * string, int height, int left_cap_width, int right_cap_width) :
	MSAnim(rect.X, rect.Y, true),
	ImageCache(image_cache),
	Pressed(true),
	Enabled(true),
	Area(rect),
	Height(height),
	LeftCapWidth(left_cap_width),
	RightCapWidth(right_cap_width),
	String(string),
	NeedsRedraw(true)
{

}


/// <summary>
/// Destroys the button anim.
/// </summary>
MSButtonAnim::~MSButtonAnim(void)
{

}


/// <summary>
/// Sets the enabled state of the button.
/// A disabled button draws itself with the grayed artwork and ignores its pressed
/// state entirely.
/// </summary>
void MSButtonAnim::Set_Enabled(bool enabled)
{
	if (enabled != Enabled) {
		Enabled = enabled;
		NeedsRedraw = true;
	}
}


/// <summary>
/// Draws the button whenever its appearance has changed.
/// The artwork is only rebuilt when the pressed or enabled state has been altered
/// since the last pass, so an idle button costs nothing.
/// </summary>
bool MSButtonAnim::Advance(Surface * surface, Rect & rect)
{
	if (Active) {
		rect.Set(0, 0, 0, 0);
		if (NeedsRedraw) {
			if (surface != NULL) {
				Render(surface);
				NeedsRedraw = false;
			}
		}
	}
	return(false);
}


/// <summary>
/// Draws the button onto the surface specified.
/// This routine is used when another anim has drawn over the area the button sits in.
/// </summary>
/// <param name="rect">The damaged region to draw within, or NULL to draw unconditionally.</param>
void MSButtonAnim::Redraw(Surface * surface, Rect const * rect)
{
	if (Active) {
		if (rect == NULL || Intersect(*rect, Get_Rect()).Is_Valid()) {
			if (surface != NULL) {
				Render(surface);
				NeedsRedraw = false;
			}
		}
	}
}


/// <summary>
/// Fetches the screen rectangle this button occupies.
/// </summary>
/// <returns>Returns with the bounding rectangle the button was created with.</returns>
Rect MSButtonAnim::Get_Rect(void) const
{
	return(Area);
}


/// <summary>
/// Draws the button's caption text.
/// The text shifts down and to the right while the button is showing its depressed
/// artwork, so that the label appears to sink along with it.
/// </summary>
void MSButtonAnim::Draw_Caption(Surface * surface)
{
	Rect rect(Area.X, Area.Y, Area.X + Area.Width - 2, Area.Y + Area.Height - 2);
	if (!Pressed) {
		rect.X += 3;
		rect.Y += 6;
	}
	OD_Draw_Text_Remap(*surface, String, rect, "dlgsys", ODColorText, 5, 0);
}


/// <summary>
/// Draws the button artwork onto the surface specified.
/// The button is assembled from cached artwork pieces chosen to suit its pressed and
/// enabled states, and the caption is printed on top of the result.
/// </summary>
void MSButtonAnim::Render(Surface * surface)
{
	char buffer[256];

	char char1 = (Pressed || !Enabled) ? 'u' : 'd'; /// u is for "up", d is for "down"
	char char2 = Enabled ? 'e' : 'd'; /// e is for "enabled", d is for "disabled"

	sprintf(buffer, "b%c%c_li%02d.pcx", char1, char2, Height);
	Surface * image = ImageCache->GetSurface(buffer);
	if (image != NULL) {
		Rect dr(Area.X, Area.Y, LeftCapWidth, Height);
		Rect sr(0, 0, dr.Width, dr.Height);
		surface->Blit_From(dr, *image, sr);
	}

	sprintf(buffer, "b%c%c_mi%02d.pcx", char1, char2, Height);
	image = ImageCache->GetSurface(buffer);
	if (image != NULL) {
		ImageCache->Draw(Rect(Area.X + LeftCapWidth, Area.Y, Area.Width - RightCapWidth - LeftCapWidth, Height), *surface, *image, 0, 0);
	}

	sprintf(buffer, "b%c%c_ri%02d.pcx", char1, char2, Height);
	image = ImageCache->GetSurface(buffer);
	if (image != NULL) {
		Rect dr(Area.X + Area.Width - 10, Area.Y, RightCapWidth, Height);
		Rect sr(0, 0, dr.Width, dr.Height);
		surface->Blit_From(dr, *image, sr);
	}

	Draw_Caption(surface);
}


/// <summary>
/// Flags the button as needing to be drawn again.
/// This routine is called when something has covered the button. The artwork is not
/// put back on the spot, but at the next advance.
/// </summary>
void MSButtonAnim::Restore(Rect const & rect)
{
	NeedsRedraw = true;
}


/// <summary>
/// Sets the pressed state of the button.
/// The button picks up the matching artwork the next time it is advanced or redrawn,
/// not immediately.
/// </summary>
void MSButtonAnim::Set_Pressed(bool pressed)
{
	if (Pressed != pressed) {
		Pressed = pressed;
		NeedsRedraw = true;
	}
}


/// <summary>
/// Creates a PCX image anim centered on the screen.
/// The artwork is loaded from the file matching the name given and placed in the
/// middle of the alternate surface.
/// </summary>
/// <param name="name">Name of the artwork file; the extension is replaced with ".PCX".</param>
/// <param name="vector">The anim list this anim belongs to.</param>
MSPCXAnim::MSPCXAnim(const char * name, MS_ANIM_LIST * vector, bool transient) :
	MSAnim(0, 0, false),
	TargetSurface(AlternateSurface),
	Anims(vector),
	Image(NULL),
	Transient(transient),
	Drawn(false),
	Area(0, 0, 0, 0)
{
	char buffer[64];

	Active = true;

	if (name != NULL) {
		strcpy(buffer, name);
		char * token = strtok(buffer, ".");
		if (token != NULL) {
			strcat(token, ".PCX");
			CCFileClass file(buffer);
			if (file.Is_Available()) {
				Image = Read_PCX_File(file);
				if (Image != NULL) {
					Area = Image->Get_Rect();
					Rect surface_rect = TargetSurface->Get_Rect();
					Area.X += (surface_rect.Width - Area.Width) / 2;
					Area.Y += (surface_rect.Height - Area.Height) / 2;
				}
			}
		}
	}
}


/// <summary>
/// Creates a PCX image anim at an explicit screen position.
/// The artwork is loaded from the file matching the name given. Use this constructor
/// when the image must land somewhere other than the middle of the screen.
/// </summary>
/// <param name="name">Name of the artwork file; the extension is replaced with ".PCX".</param>
/// <param name="vector">The anim list this anim belongs to.</param>
MSPCXAnim::MSPCXAnim(const char * name, MS_ANIM_LIST * vector, const Point2D & position, bool transient) :
	MSAnim(0, 0, false),
	TargetSurface(AlternateSurface),
	Anims(vector),
	Image(NULL),
	Transient(transient),
	Drawn(false),
	Area(0, 0, 0, 0)
{
	char buffer[64];

	Active = true;

	if (name != NULL) {
		strcpy(buffer, name);
		char * token = strtok(buffer, ".");
		if (token != NULL) {
			strcat(token, ".PCX");
			CCFileClass file(buffer);
			if (file.Is_Available()) {
				Image = Read_PCX_File(file);
				if (Image != NULL) {
					Area = Image->Get_Rect();
					Area.X = position.X;
					Area.Y = position.Y;
					TargetSurface->Get_Rect();
				}
			}
		}
	}
}


/// <summary>
/// Destroys the anim and frees the image it loaded.
/// </summary>
MSPCXAnim::~MSPCXAnim(void)
{
	if (Image) {
		delete Image;
	}
}


/// <summary>
/// Puts the PCX image up and lets the other anims repair themselves.
/// The artwork is drawn just the once. Every other anim on the list is then asked to
/// redraw over the region the image has covered.
/// </summary>
/// <param name="rect">Filled in with the region the image occupies.</param>
/// <returns>bool; Should this anim be removed from the list?</returns>
bool MSPCXAnim::Advance(Surface * surface, Rect & rect)
{
	if (!Drawn) {
		if (Active) {
			if (Image != NULL) {
				TargetSurface->Blit_From(Area, *Image, Image->Get_Rect());
				Redraw(surface);

				Rect rect2 = Area;
				rect = rect2;

				int my_id = Anims->ID(this);
				for (int i = 0; i < Anims->Count(); i++) {
					if (i != my_id) {
						(*Anims)[i]->Redraw(surface, &rect2);
					}
				}
			}
			Drawn = true;
		}
	}

	return(Drawn && !Transient);
}


/// <summary>
/// Draws the PCX image onto the surface specified.
/// The artwork is only put up while the anim is still pending; once it has been
/// committed to its backing surface this routine leaves the screen alone.
/// </summary>
/// <param name="rect">The damaged region to draw within, or NULL to draw unconditionally.</param>
void MSPCXAnim::Redraw(Surface * surface, const Rect * rect)
{
	if (Image != NULL && !Drawn && Active) {
		if (rect == NULL || Intersect(*rect, Area).Is_Valid()) {
			surface->Blit_From(Area, *Image, Image->Get_Rect());
		}
	}
}


/// <summary>
/// Restores the PCX image to the backing surface.
/// This routine is called when something else has covered the artwork and the screen
/// needs it put back.
/// </summary>
void MSPCXAnim::Restore(const Rect & rect)
{
	if (Image != NULL && Active) {
		TargetSurface->Blit_From(Area, *Image, Image->Get_Rect());
	}
}


/// <summary>
/// Fetches the screen rectangle this image occupies.
/// </summary>
/// <returns>Returns with the destination rectangle of the PCX image.</returns>
Rect MSPCXAnim::Get_Rect(void) const
{
	return(Area);
}


/// <summary>
/// Determines if the image has been put up on the screen.
/// </summary>
/// <returns>bool; Has the image been drawn?</returns>
bool MSPCXAnim::Has_Finished(void) const
{
	return(Drawn);
}
