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

/* $Header: /CounterStrike/SCORE.H 1     3/03/97 10:25a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : SCORE.H                                                      *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : April 19, 1994                                               *
 *                                                                                             *
 *                  Last Update : April 19, 1994   [JLB]                                       *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "rect.h"
#include "stimer.h"
#include "timer.h"
#include "vector.h"

#include "color.hh"

class ScoreAnimClass;
class ScoreFontClass;
class ShapeSet;
class Straw;
class Pipe;
class Surface;
class HouseClass;
class ConvertClass;

template<class T> class DynamicVectorClass;


class SfxEntry {
	public:
		SfxEntry(char const * name, char const *filename);
		~SfxEntry(void);

		const char * Get_Name(void) { return(Name);}
		void * Get_Sample(void) { return(Sample);}

	private:
		/*
		 * This is the name the score screen asks for this sound by, which is not the file
		 * it came out of -- the presentation cues its sounds by role rather than by asset.
		 */
		char * Name;

		/*
		 * This points to the sample data the entry will play. It is NULL when audio is
		 * unavailable or the sound could not be found at all.
		 */
		void * Sample;

		/*
		 * If the sample had to be loaded from disk rather than fetched out of a mixfile,
		 * then this flag will be true and this entry owns the buffer. Mixfile samples are
		 * left alone when the entry is destroyed.
		 */
		bool IsAllocated;
};


class ScoreClass {
	public:
		ScoreClass(void) {};

		void Init(void);
		void Presentation(void);

		/*
		**	File I/O.
		*/
		bool Load(Straw & file);
		bool Save(Pipe & file) const;
		void Code_Pointers(void);
		void Decode_Pointers(void);

	private:
		void Score_Delay(int ticks);
		void Pulse_Bar_Graph(void);
		void Print_Graph_Title(int,int);
		void Print_Minutes(int time);
		Rect Count_Up_Print(Surface * surf, char *str, int percent, int max, int xpos, int ypos);
		void Show_Credits(void);
		void Do_Graph(int gdikilled, int nodkilled, int ypos);
		void Do_Nod_Casualties_Graph(void);
		void Do_Nod_Buildings_Graph(void);
		void Input_Name(char str[], int xpos, int ypos);

		int Alloc_Object(ScoreAnimClass *obj);
		void Call_Back_Delay(int time);
		void Cycle_Wait_Click(bool cycle=true);
		void Animate_Score_Objs(void);
		void Animate_Cursor(int pos, int ypos);

		void Draw(void);
		void Timing(void);
		void DoSound(const char * name, int volume);
		bool Score_Object_Not_Present(ScoreAnimClass * obj);
		__forceinline void Wait_For_Print(ScoreAnimClass * obj);

		unsigned int Do_Calc(HouseClass * house);
		void Do_Graphs(void);

	private:
		/*
		 * These are the screen coordinates of the upper left corner of the score screen's
		 * 640 by 400 layout. Every element of the presentation is placed relative to them,
		 * so the screen stays centered whatever resolution the game is running at.
		 */
		int XPos;
		int YPos;

		/*
		 * This points to the off screen surface holding the score screen's backdrop. It
		 * starts as the raw title art and takes the resting frame of each piece of box art
		 * as that art finishes animating, so it always carries the screen as it should look
		 * behind the text.
		 */
		Surface * SurfacePtr;

		/*
		 * These are the two fonts the presentation prints with -- the small one for the
		 * labels and tallies, and the large one for the headline figures.
		 */
		ScoreFontClass * FullFont;
		ScoreFontClass * BigFont;

		/*
		 * This is the palette converter every shape of the score screen is drawn through.
		 * It is built from the score screen's own palette rather than the game's.
		 */
		ConvertClass * Drawer;

		/*
		 * This is the raw pixel value the blinking cursor is drawn in while the player
		 * types a hall of fame name. It comes out of the score screen's own palette, so
		 * the cursor matches the artwork rather than the game's colors.
		 */
		int Color;

		/*
		 * These are the animating elements currently on the score screen -- the counters,
		 * the typewriter text and the looping graphics. Each is updated once per pass and
		 * deleted as soon as it reports that it has finished.
		 */
		DynamicVectorClass<ScoreAnimClass *> ScoreObjs;

		/*
		 * These are the sound effects the presentation plays, held by name so that a cue
		 * can be asked for by its role rather than by its file. They are loaded when the
		 * score screen starts and released when it is torn down.
		 */
		DynamicVectorClass<SfxEntry *> ScoreSnds;
};


class ScoreAnimClass {
	public:
		ScoreAnimClass(int x, int y, void const * data);
		int XPos;
		int YPos;
		CDTimerClass<SystemTimerClass> Timer;
		void const * DataPtr;
		virtual ~ScoreAnimClass(void) {/*DataPtr=0;*/} ;
		virtual void Stop(void) { Timer.Stop(); }
		virtual void Start(void) {Timer.Start();}
		virtual bool Update(Surface * surf) = 0;
};


class ScoreCredsClass : public ScoreAnimClass {
	public:
		int Stage;
		int MaxStage;
		int TimerReset;
		void const * CashTurn;
		void const * Clock1;

		ScoreCredsClass(int xpos, int ypos, void const * data, int max, int timer);
		virtual ~ScoreCredsClass(void) override {CashTurn=0;Clock1=0;};
		virtual bool Update(Surface * surf) override;
};


class ScoreTimeClass : public ScoreAnimClass {
	public:
		int Stage;
		int MaxStage;
		int TimerReset;

		/*
		 * This is the palette converter this animation's frames are drawn through. It is
		 * handed down from the score screen, which owns it.
		 */
		ConvertClass * Drawer;

		ScoreTimeClass(int xpos, int ypos, void const * data, int max, int timer, ConvertClass * drawer);
		virtual ~ScoreTimeClass(void) override {};
		virtual bool Update(Surface * surf) override;
};


class ScorePrintClass : public ScoreAnimClass {
	public:
		/*
		 * This is the count of characters of the string that have been revealed so far. It
		 * advances by one each time the timer expires, which is what produces the score
		 * screen's typewriter effect.
		 */
		int Pos;

		int Stage;

		/*
		 * If the text is to appear fully lit from the start rather than typing itself in a
		 * character at a time, then this flag will be true. The background behind the text
		 * is left alone in that case as well.
		 */
		bool State;

		/*
		 * This points to the score screen font the text is printed with, which decides both
		 * the look of the characters and the width the line will occupy.
		 */
		ScoreFontClass * Font;

		ScorePrintClass(void const * string, int xpos, int ypos, ScoreFontClass * font, bool is_fully_lit);
		ScorePrintClass( int string, int xpos, int ypos, ScoreFontClass * font, bool is_fully_lit);
		virtual ~ScorePrintClass(void) override {};
		virtual bool Update(Surface * surf) override;

		const char *Get_String(void) {return((const char *)DataPtr); }
};


class ScoreScaleClass : public ScoreAnimClass {
	public:
		int Stage;
		char const * Palette;
		ScoreScaleClass(void const * data, int xpos, int ypos, char const pal[]);
		virtual ~ScoreScaleClass(void) override {Palette=0;};
		virtual void Update(void);
};


class ScoreFontClass
{
	public:
		/*
		 * These are the nominal character cell dimensions of the font, in pixels. The font
		 * is proportional, so the width is only a rough advance -- the true width of each
		 * glyph comes from its own shape rectangle.
		 */
		int Width;
		int Height;

		/*
		 * This points to the shape set holding the font's glyphs. Each character occupies
		 * three consecutive frames, one per brightness step of the typewriter effect.
		 */
		ShapeSet const * ShapePtr;

		/*
		 * If the glyph shapes had to be loaded from disk rather than fetched out of a
		 * mixfile, then this flag will be true and this font owns them. Mixfile shapes are
		 * left alone when the font is destroyed.
		 */
		bool IsShapeAllocated;

		/*
		 * This is the palette converter the glyphs are drawn through. It is handed down
		 * from the score screen, which owns it.
		 */
		ConvertClass * Drawer;

		ScoreFontClass(void);
		ScoreFontClass(int w, int h, void const * data, ConvertClass * drawer);
		virtual ~ScoreFontClass(void);
		virtual int Char_Width(char32_t code);
		int Char_Width(char) = delete;
		virtual int String_Width(const char * string);
		virtual void Print_String(Surface *surf, const char * string, int x, int y, int brightness_frame);
		virtual void Print_Char(Surface *surf, char32_t code, int x, int y, int v, bool play_sound);
		void Print_Char(Surface *, char, int, int, int, bool) = delete;

		void Load_Sounds(void);
		int Get_Width(void) { return(Width); }
		int Get_Height(void) { return(Height); }

	private:
		int Glyph_Frame(char32_t code) const;
};


class ScoreFullFontClass : public ScoreFontClass
{
	public:
		ScoreFullFontClass(ConvertClass * drawer);
		virtual ~ScoreFullFontClass(void) override {}
};


class ScoreBigFontClass : public ScoreFontClass
{
	public:
		ScoreBigFontClass(ConvertClass * drawer);
		virtual ~ScoreBigFontClass(void) override {}
};


void Multi_Score_Presentation(void);
