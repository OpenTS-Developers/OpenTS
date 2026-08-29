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

/* $Header: /counterstrike/SCORE.CPP 3     3/14/97 12:02a Steve_tall $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : SCORE.CPP                                                    *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : April 19, 1994                                               *
 *                                                                                             *
 *                  Last Update : May 3, 1995   [BWG]                                          *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   Call_Back_Delay -- Combines Call_Back() and Delay() functions                             *
 *   Draw_Bar_Graphs -- Draw "Casualties" bar graphs                                           *
 *   Draw_InfantryMan -- Draw one guy in score screen, update animation                        *
 *   Draw_Infantrymen -- Draw all the guys on the score screen                                 *
 *   New_Infantry_Anim -- Start up a new animation for one of the infantrymen                  *
 *   ScoreClass::Count_Up_Print -- Prints a number (up to its max) into a string, cleanly      *
 *   ScoreClass::DO_GDI_GRAPH -- Show # of people or buildings killed on GDI score screen      *
 *   ScoreClass::Delay -- Pauses waiting for keypress.                                         *
 *   ScoreClass::Presentation -- Main routine to display score screen.                         *
 *   ScoreClass::Print_Graph_Title -- Prints title on score screen.                            *
 *   ScoreClass::Print_Minutes -- Print out hours/minutes up to max                            *
 *   ScoreClass::Pulse_Bar_Graph -- Pulses the bargraph color.                                 *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "score.h"

#include "_keyboar.h"
#include "_mixfile.h"
#include "_palette.h"
#include "_surface.h"
#include "_timer.h"
#include "conquer.h"
#include "convert.h"
#include "data.h"
#include "dbgprint.h"
#include "draw.h"
#include "dsaudio.h"
#include "dsurface.h"
#include "goptions.h"
#include "houstype.h"
#include "keyboard.h"
#include "language\language.h"
#include "misc.h"
#include "mixfile.h"
#include "movie.h"
#include "msgloop.h"
#include "scenario.h"
#include "session.h"
#include "shapeset.h"
#include "surface.h"
#include "theme.h"
#include "windlg.h"
#include "winstub.h"

#include <algorithm>


#define SIZEGBAR			140
#define HALLFAME_X		11
#define HALLFAME_Y		242

#define NUMFAMENAMES				9
#define MAX_FAMENAME_LENGTH	104

void const * Beepy6;
int ControlQ;	// cheat key to skip past score/mapsel screens
bool StillUpdating;

struct Fame {
	char	name[MAX_FAMENAME_LENGTH];
	int	score;
	int	level;
	int	side;
};


/***********************************************************************************************
 * ScoreClass::Presentation -- Main routine to display score screen.                           *
 *                                                                                             *
 *    This is the main routine that displays the score screen graphics.                        *
 *    It gets called at the end of each scenario and is used to present                        *
 *    the results and a rating of the player's battle.                                         *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/02/1994     : Created.                                                                 *
 *=============================================================================================*/
void ScoreClass::Presentation(void)
{
	int i;
	int index;
	int total;
	int x, y;
	const char *str;
	ScoreAnimClass *obj;

	CCFileClass file;
	struct Fame hallfame[NUMFAMENAMES];

	while (WS_Destroy_Dialog(NULL, NULL)) {
		;
	}

	XPos = (HiddenSurface->Get_Width() - 640) / 2;
	YPos = (HiddenSurface->Get_Height() - 400) / 2;

	Hide_Mouse();
	Keyboard->Clear();

	Theme.Stop();
	Theme.Play_Song(Theme.From_Name("SCORE"));

	file.Open("SIDEFNT3.PAL");
	PaletteClass *fontpal = (PaletteClass *)Load_Alloc_Data(file);
	file.Close();

	/*
	 * The score-screen palette is stored in 6-bit VGA gun values. Scale every
	 * component up to 8-bit (value * 4) as it is copied into the game palette.
	 */
	for (i = 0; i < 256; i++) {
		CCPalette[i] = RGBClass(
			((unsigned char *)*fontpal)[i*3]<<2,
			((unsigned char *)*fontpal)[i*3+1]<<2,
			((unsigned char *)*fontpal)[i*3+2]<<2);
	}

	Color = DSurface::Build_Hicolor_Pixel((*fontpal)[68].Get_Red()<<2, (*fontpal)[68].Get_Green()<<2, (*fontpal)[68].Get_Blue()<<2);

	delete fontpal;

	ConvertClass *drawer = new ConvertClass(CCPalette, CCPalette, *VisibleSurface);
	SurfacePtr = new DSurface(HiddenSurface->Get_Width(), HiddenSurface->Get_Height());

	SurfacePtr->Fill(0);
	Load_Title_Screen("SCORE.PCX", SurfacePtr, &CCPalette);

	/*
	**	Background's up, so now load various shapes and animations
	*/

	file.Open("BEST01.SHP");
	ShapeSet *best01shape = (ShapeSet *)Load_Alloc_Data(file);
	file.Close();

	file.Open("BEST02.SHP");
	ShapeSet *best02shape = (ShapeSet *)Load_Alloc_Data(file);
	file.Close();

	Call_Back();

	file.Open("LOGO01.SHP");
	ShapeSet *logo01shape = (ShapeSet *)Load_Alloc_Data(file);
	file.Close();

	file.Open("EFIC01.SHP");
	ShapeSet *efic01shape = (ShapeSet *)Load_Alloc_Data(file);
	file.Close();

	file.Open("TIME01.SHP");
	ShapeSet *time01shape = (ShapeSet *)Load_Alloc_Data(file);
	file.Close();

	file.Open("CASU01.SHP");
	ShapeSet *casu01shape = (ShapeSet *)Load_Alloc_Data(file);
	file.Close();

	Call_Back();

	file.Open("CASU02.SHP");
	ShapeSet *casu02shape = (ShapeSet *)Load_Alloc_Data(file);
	file.Close();

	file.Open("CURR01.SHP");
	ShapeSet *curr01shape = (ShapeSet *)Load_Alloc_Data(file);
	file.Close();

	ScoreSnds = {
		new SfxEntry("Wipe", "WIPE.AUD"),
		new SfxEntry("Emblem", "EMBLEM.AUD"),
		new SfxEntry("BestBox", "BESTBOX.AUD"),
		new SfxEntry("Efficiency", "EFFICIEN.AUD"),
		new SfxEntry("BarGraph", "BARGRAPH.AUD"),
		new SfxEntry("Type", "TYPE.AUD"),
		new SfxEntry("Back", "SCOLD8.AUD")
	};

	FullFont = new ScoreFullFontClass(drawer);
	BigFont = new ScoreBigFontClass(drawer);

	Call_Back();

	ScoreObjs.Clear();

	Keyboard->Clear();

	DoSound("Wipe", 256);

	Play_Movie("SCORE.VQA", THEME_NONE, false, false, true);

	HiddenSurface->Blit_From(*SurfacePtr);
	AlternateSurface->Blit_From(*SurfacePtr);

	Drawer = new ConvertClass(CCPalette, CCPalette, *VisibleSurface);

	Alloc_Object(new ScoreTimeClass(XPos + 7, YPos + 6, logo01shape, 60, 3, Drawer));

	DoSound("Efficiency", 128);

	/*
	 * Animate the efficiency emblem (box art), then restore the area behind it.
	 */
	for (i = 0; i < 10; i++) {
		Draw_Shape(*HiddenSurface, *Drawer, efic01shape, i, Point2D(XPos + 480, YPos + 23), HiddenSurface->Get_Rect(), SHAPE_WIN_REL);
		Call_Back_Delay(4);
	}
	Rect rect(XPos + 480, YPos + 23, 160, 120);
	SurfacePtr->Blit_From(rect, *HiddenSurface, rect);
	AlternateSurface->Blit_From(rect, *HiddenSurface, rect);

	str = Fetch_String(TXT_MISSION_EFFICIENCY);
	FullFont->String_Width(str);
	x = XPos - FullFont->String_Width(str) / 2 + 552;
	Alloc_Object(obj = new ScorePrintClass(str, x, YPos + 13, FullFont, false));

	Wait_For_Print(obj);
	Keyboard->Clear();

	/*
	**	Determine economy rating.
	*/
	total = Do_Calc(PlayerPtr);
	char Dest[56];
	sprintf(Dest, "%3d%%", total);
	Alloc_Object(obj = new ScorePrintClass(Dest, XPos + 520, YPos + 60, BigFont, false));

	Wait_For_Print(obj);

	/*
	 * Animate the currency box art, then restore the area behind it.
	 */
	for (i = 0; i < 10; i++) {
		Draw_Shape(*HiddenSurface, *Drawer, curr01shape, i, Point2D(XPos + 481, YPos + 148), HiddenSurface->Get_Rect(), SHAPE_WIN_REL);
		Call_Back_Delay(4);
	}
	rect.Set(XPos + 481, YPos + 148, 144, 86);
	SurfacePtr->Blit_From(rect, *HiddenSurface, rect);
	AlternateSurface->Blit_From(rect, *HiddenSurface, rect);

	str = Fetch_String(TXT_CURRENCY);
	FullFont->String_Width(str);
	x = XPos - FullFont->String_Width(str) / 2 + 560;
	Alloc_Object(obj = new ScorePrintClass(str, x, YPos + 193, FullFont, false));

	Wait_For_Print(obj);

	rect.Set(XPos + 481, YPos + 148, 150, 120);
	SurfacePtr->Blit_From(rect, *HiddenSurface, rect);
	AlternateSurface->Blit_From(rect, *HiddenSurface, rect);

	Show_Credits();

	/*
	 * Animate the mission-time box art, then restore the area behind it.
	 */
	for (i = 0; i < 10; i++) {
		Draw_Shape(*HiddenSurface, *Drawer, time01shape, i, Point2D(XPos + 444, YPos + 273), HiddenSurface->Get_Rect(), SHAPE_WIN_REL);
		Call_Back_Delay(4);
	}
	rect.Set(XPos + 444, YPos + 273, 196, 118);
	SurfacePtr->Blit_From(rect, *HiddenSurface, rect);
	AlternateSurface->Blit_From(rect, *HiddenSurface, rect);

	str = Fetch_String(TXT_MISSION_TIME_LAPSE);
	FullFont->String_Width(str);
	x = XPos - FullFont->String_Width(str) / 2 + 542;
	Alloc_Object(obj = new ScorePrintClass(str, x, YPos + 366, FullFont, false));

	Wait_For_Print(obj);

	Print_Minutes(Scen->ElapsedTimer / 60);

	DoSound("Emblem", 256);

	/*
	 * Animate the casualties box art (two emblems), then restore the area behind it.
	 */
	for (i = 0; i < 10; i++) {
		Draw_Shape(*HiddenSurface, *Drawer, casu01shape, i, Point2D(XPos + 186, YPos + 54), HiddenSurface->Get_Rect(), SHAPE_WIN_REL);
		Draw_Shape(*HiddenSurface, *Drawer, casu02shape, i, Point2D(XPos + 186, YPos + 254), HiddenSurface->Get_Rect(), SHAPE_WIN_REL);
		Call_Back_Delay(4);
	}
	rect.Set(XPos + 186, YPos + 54, 262, 326);
	SurfacePtr->Blit_From(rect, *HiddenSurface, rect);
	AlternateSurface->Blit_From(rect, *HiddenSurface, rect);

	/*
	** Show stats on # of units killed
	*/
	str = Fetch_String(TXT_CASUALTIES);
	FullFont->String_Width(str);
	x = XPos - FullFont->String_Width(str) / 2 + 316;
	Alloc_Object(obj = new ScorePrintClass(str, x, YPos + 300, FullFont, false));

	Wait_For_Print(obj);

	/*
	** Print out stats on buildings destroyed
	*/
	str = Fetch_String(TXT_STRUCTURES);
	x = XPos + 194;
	Alloc_Object(obj = new ScorePrintClass(str, x, YPos + 247, FullFont, false));
	x = XPos + 328;
	Alloc_Object(obj = new ScorePrintClass(str, x, YPos + 247, FullFont, false));

	Wait_For_Print(obj);

	str = Fetch_String(TXT_UNITS);
	x = XPos - FullFont->String_Width(str) / 2 + 290;
	Alloc_Object(obj = new ScorePrintClass(str, x, YPos + 247, FullFont, false));
	x = XPos - FullFont->String_Width(str) / 2 + 422;
	Alloc_Object(obj = new ScorePrintClass(str, x, YPos + 247, FullFont, false));

	Wait_For_Print(obj);

	Do_Graphs();

	// Wait for text printing to complete
	while (StillUpdating) {
		Call_Back_Delay(1);
	}

	Keyboard->Clear();

	DoSound("BestBox", 256);

	/*
	 * Animate the best-scores box art (two emblems), then restore the area behind it.
	 */
	for (i = 0; i < 10; i++) {
		Draw_Shape(*HiddenSurface, *Drawer, best01shape, i, Point2D(XPos, YPos + 188), HiddenSurface->Get_Rect(), SHAPE_WIN_REL);
		Draw_Shape(*HiddenSurface, *Drawer, best02shape, i, Point2D(XPos, YPos + 388), HiddenSurface->Get_Rect(), SHAPE_WIN_REL);
		Call_Back_Delay(4);
	}

	/*
	** Hall of fame display and processing
	*/
	rect.Set(XPos, YPos + 188, 182, 212);
	SurfacePtr->Blit_From(rect, *HiddenSurface, rect);
	AlternateSurface->Blit_From(rect, *HiddenSurface, rect);

	str = Fetch_String(TXT_BEST_SCORES);
	FullFont->String_Width(str);
	x = XPos - FullFont->String_Width(str) / 2 + 84;
	Alloc_Object(obj = new ScorePrintClass(str, x, YPos + 217, FullFont, false));

	memset(hallfame, 0, sizeof(hallfame));
	file.Close();
	file.Set_Name(FAME_FILE_NAME);
	if (file.Is_Available() == true) {
		file.Read(hallfame, sizeof(hallfame));
		file.Close();
	}

	/*
	**	If the player's score is good enough to bump someone off the list,
	**	remove their data, move everyone down a notch, and set index = where
	**	their info goes
	*/
	if (hallfame[NUMFAMENAMES-1].score >= total)
								hallfame[NUMFAMENAMES-1].score = 0;
	for (index = 0; index < NUMFAMENAMES; index++) {
		if (total > hallfame[index].score) {
			if (index < (NUMFAMENAMES-1)) for (i = (NUMFAMENAMES-1); i > index; i--) hallfame[i] = hallfame[i-1];
			hallfame[index].name[0] = 0;	// blank out the name
			hallfame[index].score = total;
			hallfame[index].level = Scen->Scenario;
			hallfame[index].side = PlayerPtr->Class->Side;
			break;
		}
	}

	/*
	**	Now display the hall of fame
	*/
	char maststr[NUMFAMENAMES*32];
	for (i = 0; i < NUMFAMENAMES; i++) {
		Alloc_Object(obj = new ScorePrintClass(hallfame[i].name, XPos + HALLFAME_X - 4, YPos + HALLFAME_Y + (i*16), FullFont, false));
		if (hallfame[i].score) {
			sprintf(maststr + i*32, "%d%%", hallfame[i].score);
			Alloc_Object(obj = new ScorePrintClass(maststr + i*32, XPos + HALLFAME_X + 98, YPos + HALLFAME_Y + (i*16), FullFont, false));
			char *levelstr = maststr + i*32 + 16;
			sprintf(levelstr, "%02d", hallfame[i].level);
			Alloc_Object(obj = new ScorePrintClass(levelstr, XPos + HALLFAME_X + 140, YPos + HALLFAME_Y + (i*16), FullFont, false));
			Wait_For_Print(obj);
		}
		Call_Back_Delay(1);
	}

	// Wait for text printing to complete
	while (StillUpdating) {
		Call_Back_Delay(1);
	}

	/*
	**	If the player's on the hall of fame, have him enter his name now
	*/
	Keyboard->Clear();
	Show_Mouse();

	if (index < NUMFAMENAMES) {
		Input_Name(hallfame[index].name, XPos + HALLFAME_X - 4, YPos + HALLFAME_Y + (index * 16));
	} else {
		str = Fetch_String(TXT_CLICK_CONTINUE);
		x = XPos + (640 - FullFont->String_Width(str)) / 2;
		y = YPos - FullFont->Get_Height() / 2 + 357;
		Alloc_Object(obj = new ScorePrintClass(str, x, y, FullFont, false));
		Cycle_Wait_Click();
	}

	Keyboard->Clear();

	if (file.Open(FAME_FILE_NAME, FileClass::WRITE)) {
		file.Write(hallfame, sizeof(hallfame));
		file.Close();
	}

	Theme.Stop(true);

	/*
	 * get rid of all the objects
	 */

	for (i = 0; i < ScoreSnds.Count(); i++) {
		if (ScoreSnds[i]) {
			delete ScoreSnds[i];
			//ScoreSnds[i] = NULL;
		}
	}
	ScoreSnds.Clear();

	for (i = 0; i < ScoreObjs.Count(); i++) {
		if (ScoreObjs[i]) {
			delete ScoreObjs[i];
			//ScoreObjs[i] = NULL;
		}
	}
	ScoreObjs.Clear();

	if (Drawer != NULL) {
		delete Drawer;
		Drawer = NULL;
	}

	if (FullFont != NULL) {
		delete FullFont;
		FullFont = NULL;
	}

	if (BigFont != NULL) {
		delete BigFont;
		BigFont = NULL;
	}

	if (SurfacePtr != NULL) {
		delete SurfacePtr;
		SurfacePtr = NULL;
	}

	delete drawer;

	HiddenSurface->Fill(0);

	Draw();

	Theme.Queue_Song(THEME_NONE);
}


/// <summary>
/// Adds an object to the score screen's animation list.
/// The score screen owns the object from here on and will delete it once the object
/// reports that it has finished.
/// </summary>
/// <returns>Returns with true if the object was accepted into the list.</returns>
int ScoreClass::Alloc_Object(ScoreAnimClass *obj)
{
	return(ScoreObjs.Add(obj));
}


/// <summary>
/// Has this object left the score screen's animation list?
/// An object is dropped from the list as soon as it reports that it has finished, so this
/// is how the score screen tells that a queued piece of text has printed itself out.
/// </summary>
/// <returns>bool; Is the object gone from the animation list?</returns>
bool ScoreClass::Score_Object_Not_Present(ScoreAnimClass * obj)
{
	if (ScoreObjs.ID(obj) == -1) {
		return(true);
	}
	return(false);
}


/// <summary>
/// Waits for a queued score screen object to finish.
/// This routine is used after handing a piece of text to the score screen, so that the
/// screen builds itself up one line at a time rather than all at once. The score screen
/// keeps ticking over while it waits.
/// </summary>
/// <param name="obj">The score screen object to wait on.</param>
void ScoreClass::Wait_For_Print(ScoreAnimClass * obj)
{
	if (!Score_Object_Not_Present(obj)) {
		do {
			Call_Back_Delay(1);
		} while (!Score_Object_Not_Present(obj));
	}
}


/// <summary>
/// Waits for the player to press a key before moving on.
/// A minimum idle period is served first, so that a keystroke left over from the mission
/// cannot skip the screen the instant it appears.
/// </summary>
void ScoreClass::Cycle_Wait_Click(bool cycle)
{
	int counter = 0;
	int minclicks = 20;

	Keyboard->Clear();
	while (minclicks || (!Keyboard->Check()) ) {

		Call_Back_Delay(1);
		if (minclicks) {
			minclicks--;
			Keyboard->Clear();
		}
	}
	Keyboard->Clear();
}


/// <summary>
/// Determines the efficiency rating for the score screen.
/// The rating weighs how much of what the house and its allies built came through the
/// mission alive against how much of the money they had to spend went unspent.
/// </summary>
/// <param name="house">The house being rated -- normally the player.</param>
/// <returns>Returns with the efficiency rating, expressed as a percentage.</returns>
unsigned int ScoreClass::Do_Calc(HouseClass * house)
{
	int remain = 0;
	int total = 0;

	for (int i = 0; i < Houses.Count(); i++) {
		HouseClass * hptr = Houses[i];
		if (house->Is_Ally(hptr) && hptr->Is_Ally(house) || house == hptr) {
			remain += hptr->ABQuantity.Total();
			remain += hptr->AUQuantity.Total();
			remain += hptr->AIQuantity.Total();
			remain += hptr->AAQuantity.Total();
			total += hptr->BuildingsLost;
			total += hptr->UnitsLost;
		}
	}

	total += remain;
	DebugString("Remaining: %ld, Total: %ld\n", remain, total);

	float build_eco = total > 0 ? ((float)remain / total) : 0.0f;

	DebugString("BuildEconomy = %f\n", build_eco);

	DebugString("Money: %ld, Harvested: %ld, Initial: %ld\n", house->Available_Money(), house->HarvestedCredits, house->Control.InitialCredits);

	total = house->HarvestedCredits + house->Control.InitialCredits;
	float money_eco = total > 0 ? (float)house->Available_Money() / (float)total : 0.0f;

	DebugString("MoneyEconomy = %f\n", money_eco);

	float eco = money_eco + build_eco;

	unsigned int efficiency = (unsigned int)((eco / 2.0) * 100.0);

	DebugString("Efficiency: %ld\n", efficiency);

	return(efficiency);
}


/// <summary>
/// Draws the casualty bar graphs onto the score screen.
/// Losses are tallied for the player and its allies on one hand and for everybody else on
/// the other, then handed to Do_Graph as a buildings pair and a units pair. Which tally
/// is drawn on which bar depends on the player's side, so that each house's own color
/// always lands in the same place.
/// </summary>
void ScoreClass::Do_Graphs(void)
{
	int allyu = 0;
	int allyb = 0;
	int enemyu = 0;
	int enemyb = 0;

	for (int hous = 0; hous < Houses.Count(); hous++) {

		HouseClass *hows = Houses[hous];
		DebugString("Stats: %s - UnitsLost %ld, BuildingsLost %ld\n", hows->Class->IniName.c_str(), hows->UnitsLost, hows->BuildingsLost);

		if ((PlayerPtr->Is_Ally(hows) && hows->Is_Ally(PlayerPtr)) || PlayerPtr == hows) {
			allyu += hows->UnitsLost;
			allyb += hows->BuildingsLost;
		} else {
			enemyu += hows->UnitsLost;
			enemyb += hows->BuildingsLost;
		}

		DebugString("AllyUnits %ld, AllyBuildings %ld, EnemyUnits %ld, EnemyBldgs %ld\n", allyu, allyb, enemyu, enemyb);
	}

	if (PlayerPtr->Class->Side == SIDE_GDI) {
		Do_Graph(allyb, enemyb, 227);
		Do_Graph(allyu, enemyu, 283);
	} else {
		Do_Graph(enemyb, allyb, 227);
		Do_Graph(enemyu, allyu, 283);
	}
}


/***************************************************************************
* DO_GDI_GRAPH -- Show # of people or buildings killed on GDI score screen*
*                                                                         *
*                                                                         *
*                                                                         *
* INPUT:   yellowptr, redptr = pointers to shape file for graphs          *
*                                                                         *
* OUTPUT:                                                                 *
*                                                                         *
* WARNINGS:                                                               *
*                                                                         *
* HISTORY:                                                                *
*   05/03/1995 BWG : Created.                                             *
*=========================================================================*/

void ScoreClass::Do_Graph(int gkilled, int nkilled, int xpos)
{
	Rect rect1(0,0,0,0);
	Rect rect2(0,0,0,0);

	int maxval;

	xpos = XPos + xpos;
	int ypos = YPos + 244;

	int gdikilled = gkilled, nodkilled=nkilled;

	maxval = std::max(gdikilled, nodkilled);
	if (!maxval) maxval=1;

	gdikilled = (gdikilled * SIZEGBAR) / maxval;
	nodkilled = (nodkilled * SIZEGBAR) / maxval;
	if (maxval < 20) {
		gdikilled = gkilled * 5;
		nodkilled = nkilled * 5;
	}

	maxval = std::max(gdikilled, nodkilled);
	if (!maxval) maxval=1;

	Rect trect;

	trect.Set(xpos, ypos - gdikilled, 4, gdikilled);
	SurfacePtr->Fill_Rect_Trans(trect, RGBClass(253, 181, 28), 25);
	trect.Set(xpos + 4, ypos - gdikilled, 7, gdikilled);
	SurfacePtr->Fill_Rect_Trans(trect, RGBClass(253, 181, 28), 50);
	trect.Set(xpos + 11, ypos - gdikilled, 4, gdikilled);
	SurfacePtr->Fill_Rect_Trans(trect, RGBClass(253, 181, 28), 25);

	trect.Set(xpos + 132, ypos - nodkilled, 4, nodkilled);
	SurfacePtr->Fill_Rect_Trans(trect, RGBClass(250, 28, 28), 25);
	trect.Set(xpos + 132 + 4, ypos - nodkilled, 7, nodkilled);
	SurfacePtr->Fill_Rect_Trans(trect, RGBClass(250, 28, 28), 50);
	trect.Set(xpos + 132 + 11, ypos - nodkilled, 4, nodkilled);
	SurfacePtr->Fill_Rect_Trans(trect, RGBClass(250, 28, 28), 25);


	int gdicount = 1;
	int nodcount = 1;
	int nodrun = nkilled;
	int gdirun = gkilled;
	int nodadd = 2 * nkilled;
	int gdiadd = 2 * gkilled;

	for (int i = ypos - 1; gdicount <= std::max(gdikilled, nodkilled); ) {

		if (rect1.Is_Valid()) {
			HiddenSurface->Blit_From(rect1, *AlternateSurface, rect1);
		}

		if (gdicount <= gdikilled) {
			trect.Set(xpos, i, 15, gdicount);
			HiddenSurface->Blit_From(trect, *SurfacePtr, trect);
			rect1 = Count_Up_Print(HiddenSurface, (char *)"%d", gdirun / maxval, gkilled, xpos, i);
		} else {
			rect1 = Count_Up_Print(HiddenSurface, (char *)"%d", gkilled, gkilled, xpos, ypos - gdikilled);
		}

		if (rect2.Is_Valid()) {
			HiddenSurface->Blit_From(rect2, *AlternateSurface, rect2);
		}

		if (nodcount <= nodkilled) {
			trect.Set(xpos + 132, i, 15, nodcount);
			HiddenSurface->Blit_From(trect, *SurfacePtr, trect);
			rect2 = Count_Up_Print(HiddenSurface, (char *)"%d", nodrun / maxval, nkilled, xpos + 132, i);
		} else {
			rect2 = Count_Up_Print(HiddenSurface, (char *)"%d", nkilled, nkilled, xpos + 132, ypos - nodkilled);
		}

		DoSound("BarGraph", 96);
		Call_Back_Delay(1);

		gdicount += 2;
		nodcount += 2;
		gdirun += gdiadd;
		i -= 2;
		nodrun += nodadd;
	}

	/*
	** Make sure accurate count is printed at end
	*/

	if (rect1.Is_Valid()) {
		HiddenSurface->Blit_From(rect1, *AlternateSurface, rect1);
	}
	trect.Set(xpos, ypos - gdikilled, 15, gdikilled);
	AlternateSurface->Blit_From(trect, *SurfacePtr, trect);
	Count_Up_Print(HiddenSurface, (char *)"%d", gkilled, gkilled, xpos, ypos - gdikilled);
	Count_Up_Print(AlternateSurface, (char *)"%d", gkilled, gkilled, xpos, ypos - gdikilled);

	if (rect2.Is_Valid()) {
		HiddenSurface->Blit_From(rect2, *AlternateSurface, rect2);
	}
	trect.Set(xpos + 132, ypos - nodkilled, 15, nodkilled);
	AlternateSurface->Blit_From(trect, *SurfacePtr, trect);
	Count_Up_Print(HiddenSurface, (char *)"%d", nkilled, nkilled, xpos + 132, ypos - nodkilled);
	Count_Up_Print(AlternateSurface, (char *)"%d", nkilled, nkilled, xpos + 132, ypos - nodkilled);

	Call_Back_Delay(20);
}


/// <summary>
/// Prints the player's leftover credits onto the score screen.
/// This routine does not return until the figure has finished typing itself out, so that
/// the score screen fills in one line at a time.
/// </summary>
void ScoreClass::Show_Credits(void)
{
	char str[20];

	sprintf(str, "%d", PlayerPtr->Available_Money());

	int x = 590 - (12 * strlen(str));

	ScoreAnimClass *obj = new ScorePrintClass(str, XPos + x, YPos + 214, FullFont, false);
	Alloc_Object(obj);

	/*
	** Print out total credits left at end of scenario
	*/
	Wait_For_Print(obj);
}


/***************************************************************************
 * SCORECLASS::PRINT_MINUTES -- Print out hours/minutes up to max          *
 *                                                                         *
 *    Same as count-up-print, but for the time                             *
 *                                                                         *
 * INPUT:   current minute count and maximum                               *
 *                                                                         *
 * OUTPUT:                                                                 *
 *                                                                         *
 * WARNINGS:                                                               *
 *                                                                         *
 * HISTORY:                                                                *
 *   04/13/1995 BWG : Created.                                             *
 *=========================================================================*/
void ScoreClass::Print_Minutes(int time)
{
	char str[20];

	//if ((time/60) > 9) time = (9*60 + 59);
	if (((unsigned)time/3600) > 99) time = 359999;

	int hours	 = int((unsigned)time / 3600);
	time		-= hours * 3600;
	int minutes	 = int((unsigned)time / 60);
	time		-= minutes * 60;
	int seconds	 = (unsigned)time;

	sprintf(str, "%02ld:%02ld:%02ld", hours, minutes, seconds);

	int x = XPos - (BigFont->String_Width(str) / 2) + 542;

	ScoreAnimClass *obj = new ScorePrintClass(str, x, YPos + 322, BigFont, false);
	Alloc_Object(obj);

	Wait_For_Print(obj);
}


/***********************************************************************************************
 * ScoreClass::Count_Up_Print -- Prints a number (up to its max) into a string, cleanly.       *
 *                                                                                             *
 *    This routine prints out a number (like 70) or its maximum number, into a string,   onto  *
 *    the screen, on a clean section of the screen, and blits it forward to the seenpage so you*
 *    can print without flashing and can print over something (to count up %'s).               *
 *                                                                                             *
 * INPUT:   str = string to print into                                                         *
 *            percent = # to print                                                             *
 *            max = # to print if percent > max                                                *
 *            xpos = x pixel coord                                                             *
 *            ypos = y pixel coord                                                             *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   04/07/1995 BWG : Created.                                                                 *
 *=============================================================================================*/
Rect ScoreClass::Count_Up_Print(Surface * surf, char *str, int percent, int maxval, int xpos, int ypos)
{
	char destbuf[64];

	sprintf(destbuf, str, percent >= maxval ? maxval : percent);
	ScoreFontClass * font = FullFont;
	ypos = ypos - FullFont->Get_Height();
	FullFont->Print_String(surf, destbuf, xpos, ypos, 2);
	return(Rect(xpos, ypos, FullFont->String_Width(destbuf), FullFont->Get_Height()));
}


/***********************************************************************************************
 * ScoreClass::Input_Name -- Gets the name from the keyboard                                   *
 *                                                                                             *
 *      This routine handles keyboard input, and does a nifty zooming letter effect too.       *
 *                                                                                             *
 * INPUT:   str = string to put user's typing into                                             *
 *            xpos = x pixel coord                                                             *
 *            ypos = y pixel coord                                                             *
 *            pal  = text remapping palette to print using                                     *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/15/1995 BWG : Created.                                                                 *
 *=============================================================================================*/
void ScoreClass::Input_Name(char str[], int xpos, int ypos)
{
	int x = xpos;
	int key = 0;
	int index = 0;

	do {

		Timing();

		if (Keyboard->Check()) {
			key = Keyboard->To_ASCII(Keyboard->Get()) & 0xFF;
			Call_Back();

			/*
			 * Once the cursor has run off the right edge of the name field,
			 * throw away any further queued keys.
			 */
			if (x >= xpos + 90) {
				while (Keyboard->Check()) {
					Keyboard->Get();
				}
			}

			/*
			** If they hit 'backspace' when they're on the last letter,
			** turn it into a space instead.
			*/
			if (key == KN_BACKSPACE && x >= xpos + 84 && str[index] != 0 && str[index] != 32) {
				Rect rect(x, ypos + 1, FullFont->Char_Width(str[index]) + 1, 16);
				HiddenSurface->Blit_From(rect, *SurfacePtr, rect);
				AlternateSurface->Blit_From(rect, *SurfacePtr, rect);
				key = 32;
			}

			if (key == KN_BACKSPACE) {

				/*
				 * Any other backspace erases the previous character.
				 */
				if (index) {
					char letter = str[--index];
					int cw = FullFont->Char_Width(letter) + 1;
					x -= cw;
					Rect rect(x, ypos + 1, cw, 16);
					HiddenSurface->Blit_From(rect, *SurfacePtr, rect);
					AlternateSurface->Blit_From(rect, *SurfacePtr, rect);
					str[index] = 0;
					DoSound("Back", 192);
				}

			} else if (key != KN_KEYPAD_RETURN && key >= KN_SPACE) {

				/*
				 * Draw the new (or overwritten) character and advance the cursor.
				 */
				int cw = FullFont->Char_Width(key) + 1;
				if (x + FullFont->Get_Width() <= xpos + 90) {
					if (x + cw >= xpos + 84) {
						Rect rect(x, ypos + 1, FullFont->Char_Width(str[index]) + 1, 16);
						HiddenSurface->Blit_From(rect, *SurfacePtr, rect);
						AlternateSurface->Blit_From(rect, *SurfacePtr, rect);
					}
					str[index] = key;
					str[index + 1] = 0;
					Alloc_Object(new ScorePrintClass(str + index, x, ypos, FullFont, false));
					DoSound("Type", 128);
					if (x < xpos + 87) {
						index++;
						x += FullFont->Char_Width(key) + 1;
					}
				}
			}
		}

		Animate_Cursor(x, ypos);
		Call_Back_Delay(1);

	} while (key != KN_RETURN);
}


/// <summary>
/// Blinks the hall of fame name entry cursor.
/// The cursor alternates between a drawn underline and the artwork behind it. Moving to a
/// new position erases the cursor from the old one first.
/// </summary>
/// <param name="pos">The horizontal pixel position to draw the cursor at.</param>
/// <param name="ypos">The top of the letter the cursor sits beneath.</param>
/// <remarks>Call this routine every frame while the player is entering a name.</remarks>
void ScoreClass::Animate_Cursor(int pos, int ypos)
{
	static int _lastpos = 0, _state;
	static CDTimerClass<SystemTimerClass> _timer;

	Rect rect;

	ypos += 14;	// move cursor to bottom of letter

	// If they moved the cursor, erase old one and force state=0, to make green draw right away
	if (pos != _lastpos) {
		rect.Set(_lastpos, ypos, HALLFAME_X, 1);
		HiddenSurface->Blit_From(rect, *AlternateSurface, rect);
		_lastpos = pos;
		//_state = 0;
	}

	if (_state) {
		HiddenSurface->Draw_Line(Point2D(pos, ypos), Point2D(pos + HALLFAME_X - 1, ypos), Color);
	} else {
		rect.Set(pos, ypos, HALLFAME_X, 1);
		HiddenSurface->Blit_From(rect, *SurfacePtr, rect);
	}

	/*
	**	Toggle the color of the cursor, green or black, if it's time to do so.
	*/
	if (!_timer) {
		_state ^= 1;
		_timer = 5;
	}
}


/***************************************************************************
 * Call_Back_Delay -- Combines Call_Back() and Delay() functions           *
 *                                                                         *
 *    This is just to cut down on code size and typing a little.           *
 *                                                                         *
 * INPUT:                                                                  *
 *                                                                         *
 * OUTPUT:                                                                 *
 *                                                                         *
 * WARNINGS:                                                               *
 *                                                                         *
 * HISTORY:                                                                *
 *   04/13/1995 BWG : Created.                                             *
 *=========================================================================*/
void ScoreClass::Call_Back_Delay(int time)
{
	Rect rect;
	CDTimerClass<SystemTimerClass> cd = 1;

	do {
		Animate_Score_Objs();

		Draw();

		do {

			Call_Back();
			Windows_Message_Handler();

			if (!GameInFocus) {

				cd.Stop();

				Timing();

				cd.Start();
			}

			Sleep(0);

		} while (cd > 0);

		cd = 1;
		time--;

	} while (time > 0);
}


/// <summary>
/// Updates every animating object on the score screen.
/// Each object is given its chance to advance, and any that report they have finished are
/// deleted. The screen is rebuilt from the alternate surface first if the display was
/// lost while the game sat in the background.
/// </summary>
void ScoreClass::Animate_Score_Objs(void)
{
	StillUpdating = false;

	for (int i = 0; i < ScoreObjs.Count(); i++) {
		if (ScoreObjs[i]->Update(SurfacePtr) == true) {
			delete ScoreObjs[i];
			ScoreObjs.Delete_Index(i);
		}
	}
}


/// <summary>
/// Blits the finished score screen to the visible surface.
/// </summary>
void ScoreClass::Draw(void)
{
	Rect rect1(XPos, YPos, 640, 400);
	Rect rect2(XPos, YPos, 640, 400);

	VisibleSurface->Blit_From(rect1, *HiddenSurface, rect2);
}


/// <summary>
/// Holds the score screen still while the game is out of focus.
/// Every animating object is stopped, the routine idles until the window comes back to
/// the foreground, and then the objects are started again. Without this the tallies would
/// count themselves up while the player was away.
/// </summary>
void ScoreClass::Timing(void)
{
	if (GameInFocus == true) {
		return;
	}

	int i;

	for (i = 0; i < ScoreObjs.Count(); i++) {
		ScoreObjs[i]->Stop();
	}

	while (!GameInFocus) {
		Sleep(500);
		Windows_Message_Handler();
	}

	for (i = 0; i < ScoreObjs.Count(); i++) {
		ScoreObjs[i]->Start();
	}
}


/// <summary>
/// Plays one of the score screen's sound effects by name.
/// The sound is looked up in the list the score screen loaded at startup. A name that is
/// not in the list, or one whose sample failed to load, simply plays nothing.
/// </summary>
/// <param name="name">The name the sound was registered under.</param>
/// <param name="volume">The volume to play at, before the player's sound setting is
/// applied.</param>
void ScoreClass::DoSound(const char * name, int volume)
{
	if (name != NULL && Audio_Available()) {

		for (int i = 0; i < ScoreSnds.Count(); i++) {
			SfxEntry * snd = ScoreSnds[i];

			if (stricmp(name, snd->Get_Name()) == 0) {
				if (snd->Get_Sample() != NULL) {
					Audio.Play_Sample(snd->Get_Sample(), 255, int(volume * Options.SoundVolume));
				}
				break;
			}
		}
	}
}

int score_font_count;
struct ScoreTextSoundStruct {
	void * mSound;
	bool mAllocated;
} text_sounds[3];


/// <summary>
/// Constructs an empty score screen font.
/// The derived font classes build on this form and fill in their own glyph shapes and
/// dimensions afterwards.
/// </summary>
ScoreFontClass::ScoreFontClass(void) :
	Width(0),
	Height(0),
	ShapePtr(NULL),
	Drawer(NULL)
{
	Load_Sounds();
	score_font_count++;
}


/// <summary>
/// Constructs a score screen font from glyph shapes that already exist.
/// The font does not own the shape data in this form, so it will not release it.
/// </summary>
/// <param name="w">The nominal glyph width.</param>
/// <param name="h">The nominal glyph height.</param>
/// <param name="data">The shape set holding the font's glyphs.</param>
/// <param name="drawer">The palette converter to draw the glyphs with.</param>
ScoreFontClass::ScoreFontClass(int w, int h, void const * data, ConvertClass * drawer) :
	Width(w),
	Height(h),
	ShapePtr((ShapeSet const *)data),
	Drawer(drawer)
{
	Load_Sounds();
	score_font_count++;
}


/// <summary>
/// Destroys a score screen font.
/// Any glyph shapes this font loaded for itself are released, and when the last font goes
/// away the typing sounds they share are stopped and freed as well.
/// </summary>
ScoreFontClass::~ScoreFontClass(void)
{
	if (ShapePtr != NULL && IsShapeAllocated == true) {
		delete (char *)ShapePtr;
		ShapePtr = NULL;
		IsShapeAllocated = false;
	}

	score_font_count--;
	if (score_font_count == 0) {
		for (int i = 0; i < 3; i++) {
			if (text_sounds[i].mSound != NULL) {
				Audio.Stop_Sample_Playing(text_sounds[i].mSound);
				if (text_sounds[i].mAllocated == true) {
					delete text_sounds[i].mSound;
				}
			}
			text_sounds[i].mSound = NULL;
			text_sounds[i].mAllocated = false;
		}
	}
}


/// <summary>
/// Loads the typing sounds used by the score screen fonts.
/// The samples come from the mixfiles where possible and from disk otherwise. They are
/// shared by every font and are not released until the last font is destroyed.
/// </summary>
void ScoreFontClass::Load_Sounds(void)
{
	if (Audio_Available()) {
		CCFileClass file;
		text_sounds[0].mSound = (void *)MFCD::Retrieve("TEXT1.AUD");
		if (text_sounds[0].mSound == NULL) {
			file.Open("TEXT1.AUD", FileClass::READ);
			text_sounds[0].mSound = Load_Alloc_Data(file);
			text_sounds[0].mAllocated = true;
			file.Close();
			DebugString("ScoreScreen: Loaded TEXT1.AUD\n");
		}
		text_sounds[1].mSound = (void *)MFCD::Retrieve("TEXT2.AUD");
		if (text_sounds[1].mSound == NULL) {
			file.Open("TEXT2.AUD", FileClass::READ);
			text_sounds[1].mSound = Load_Alloc_Data(file);
			text_sounds[1].mAllocated = true;
			file.Close();
			DebugString("ScoreScreen: Loaded TEXT2.AUD\n");
		}
		text_sounds[2].mSound = (void *)MFCD::Retrieve("TEXT3.AUD");
		if (text_sounds[2].mSound == NULL) {
			file.Open("TEXT3.AUD", FileClass::READ);
			text_sounds[2].mSound = Load_Alloc_Data(file);
			text_sounds[2].mAllocated = true;
			file.Close();
			DebugString("ScoreScreen: Loaded TEXT3.AUD\n");
		}
	}
}


/// <summary>
/// Fetches the printed width of a string in this font.
/// This routine is used to center and right align the score screen's figures, which are
/// not known until the mission is over.
/// </summary>
/// <returns>Returns with the width in pixels that the string would occupy.</returns>
int ScoreFontClass::String_Width(const char * string)
{
	const char * str = string;
	int w = 0;
	while (*str) {
		w += Char_Width((char)*str++) + 1;
	}
	return(w);
}


/// <summary>
/// Fetches the printed width of a character in this font.
/// The font is proportional, so every glyph must be measured rather than assumed.
/// </summary>
/// <returns>Returns with the width in pixels that the character occupies.</returns>
int ScoreFontClass::Char_Width(char ch)
{
	char out[1];

	if (ch == 32) {
		return(8);
	}

	CharToOemBuff(&ch, out, sizeof(out));

	int frame;
	frame = std::max((out[0] & 0xFF) - 33, 0);
	frame = 3 * std::min(frame, 216);

	return(ShapePtr->Get_Rect(frame + 2).Width);
}


/// <summary>
/// Prints a single character with this font.
/// This is the low level glyph blitter that the score screen's typewriter effect is built
/// out of. It will also play one of the typing sounds as the character lands.
/// </summary>
/// <param name="v">The brightness frame to draw the glyph with.</param>
/// <param name="play_sound">Should a typing sound be played along with the character?</param>
void ScoreFontClass::Print_Char(Surface *surf, char ch, int x, int y, int v, bool play_sound)
{
	CharToOemBuff(&ch, &ch, sizeof(ch));

	if (ch != 32) {
		int frame;
		frame = std::max((ch & 0xFF) - 33, 0);
		frame = 3 * std::min(frame, 216);

		if (play_sound == true && v == 0) {
			void *snd = text_sounds[rand() % 3].mSound;
			if (snd != NULL) {
				Audio.Play_Sample(snd, 255, Options.SoundVolume * 128);
			}
		}
		Draw_Shape(*surf, *Drawer, ShapePtr, frame + v, Point2D(x - ShapePtr->Get_Rect(frame + 2).X, y), surf->Get_Rect(), SHAPE_WIN_REL);
	}
}


/// <summary>
/// Prints a whole string with this font.
/// This routine draws the string in one go and in silence, unlike the typewriter effect
/// the score screen builds out of Print_Char.
/// </summary>
/// <param name="brightness_frame">The brightness frame to draw the glyphs with.</param>
void ScoreFontClass::Print_String(Surface *surf, const char * string, int x, int y, int brightness_frame)
{
	unsigned char buf[2];
	while (*string != '\0') {
		if (*string != 32) {
			CharToOemBuff(string, (LPSTR)buf, sizeof(char));
			int frame = 3 * std::min(std::max(buf[0] - 33, 0), 216);

			Draw_Shape(*surf, *Drawer, ShapePtr, frame + brightness_frame, Point2D(x - ShapePtr->Get_Rect(frame + 2).X, y), surf->Get_Rect(), SHAPE_WIN_REL);
		}
		x += Char_Width(*string) + 1;
		string++;
	}
}


/// <summary>
/// Constructs the standard score screen font.
/// The glyph shapes are fetched from the mixfiles, or loaded from disk if they are not
/// there. This is the font the score screen prints its labels and tallies with.
/// </summary>
/// <param name="drawer">The palette converter to draw the glyphs with.</param>
ScoreFullFontClass::ScoreFullFontClass(ConvertClass * drawer) :
	ScoreFontClass()
{
	IsShapeAllocated = false;
	ShapePtr = (const ShapeSet *)MFCD::Retrieve("FULLFNT3.SHP");
	if (ShapePtr == NULL) {
		CCFileClass file("FULLFNT3.SHP");
		ShapePtr = (ShapeSet const *)Load_Alloc_Data(file);
		IsShapeAllocated = true;
		file.Close();
		DebugString("ScoreScreen: Loaded FULLFNT3.SHP\n");
	}
	Width = 11;
	Drawer = drawer;
	Height = 21;
}


/// <summary>
/// Constructs the large score screen font.
/// The glyph shapes are fetched from the mixfiles, or loaded from disk if they are not
/// there. This is the font the score screen prints its headline figures with.
/// </summary>
/// <param name="drawer">The palette converter to draw the glyphs with.</param>
ScoreBigFontClass::ScoreBigFontClass(ConvertClass * drawer) :
	ScoreFontClass()
{
	IsShapeAllocated = false;
	ShapePtr = (const ShapeSet *)MFCD::Retrieve("BIGFONT.SHP");
	if (ShapePtr == NULL) {
		CCFileClass file("BIGFONT.SHP");
		ShapePtr = (ShapeSet const *)Load_Alloc_Data(file);
		IsShapeAllocated = true;
		file.Close();
		DebugString("ScoreScreen: Loaded BIGFONT.SHP\n");
	}
	Width = 20;
	Drawer = drawer;
	Height = 35;
}


/// <summary>
/// Constructs a looping animated graphic for the score screen.
/// </summary>
/// <param name="data">The shape set holding the animation frames.</param>
/// <param name="maxval">The number of frames in the animation loop.</param>
/// <param name="xtimer">The delay between frames.</param>
/// <param name="drawer">The palette converter to draw the shape with.</param>
ScoreTimeClass::ScoreTimeClass(int xpos, int ypos, void const * data, int maxval, int xtimer, ConvertClass * drawer) :
	ScoreAnimClass(xpos, ypos, data),
	Stage(0),
	MaxStage(maxval),
	TimerReset(xtimer),
	Drawer(drawer)
{
}


/// <summary>
/// Advances the animation of a looping score screen graphic.
/// The next frame is drawn each time the object's timer expires, onto the alternate
/// surface as well as the hidden one so that the animation survives a background restore.
/// </summary>
/// <returns>bool; Has the object finished? Never -- these graphics loop until the score
/// screen tears them down.</returns>
bool ScoreTimeClass::Update(Surface * surf)
{
	if (!Timer) {
		Timer = TimerReset;
		if (++Stage >= MaxStage) Stage = 0;
		Draw_Shape(*HiddenSurface, *Drawer, (ShapeSet *)DataPtr, Stage, Point2D(XPos, YPos), HiddenSurface->Get_Rect(), SHAPE_WIN_REL);
		Draw_Shape(*AlternateSurface, *Drawer, (ShapeSet *)DataPtr, Stage, Point2D(XPos, YPos), AlternateSurface->Get_Rect(), SHAPE_WIN_REL);
	}
	return(false);
}


/// <summary>
/// Constructs a score screen text object from a string table entry.
/// This is the form used for the fixed labels of the score screen, which must come out of
/// the string table so that they follow the language the game was installed in.
/// </summary>
/// <param name="string">The string table identifier of the text to print.</param>
/// <param name="font">The score screen font to print the text with.</param>
/// <param name="is_fully_lit">Should the text appear fully lit instead of typing itself in?</param>
ScorePrintClass::ScorePrintClass(int string, int xpos, int ypos, ScoreFontClass * font, bool is_fully_lit) :
	ScoreAnimClass(xpos, ypos, Fetch_String(string)),
	Pos(0),
	Stage(0),
	State(is_fully_lit),
	Font(font)
{
}


/// <summary>
/// Constructs a score screen text object from a literal string.
/// This is the form used for the figures the score screen builds for itself, such as the
/// credit and time totals.
/// </summary>
/// <param name="font">The score screen font to print the text with.</param>
/// <param name="is_fully_lit">Should the text appear fully lit instead of typing itself in?</param>
/// <remarks>The string is not copied, so its buffer must outlive this object.</remarks>
ScorePrintClass::ScorePrintClass(void const * string, int xpos, int ypos, ScoreFontClass * font, bool is_fully_lit) :
	ScoreAnimClass(xpos, ypos, string),
	Pos(0),
	Stage(0),
	State(is_fully_lit),
	Font(font)
{
}


/// <summary>
/// Advances the typewriter printing of this text.
/// One more character is revealed each time the object's timer expires, the trailing few
/// drawn in the brightening frames that give the score screen its typewriter look. The
/// settled characters are laid into the alternate surface as well, so they survive a
/// background restore.
/// </summary>
/// <returns>bool; Has the text finished printing, so that the object may be deleted?</returns>
bool ScorePrintClass::Update(Surface * surf)
{

	StillUpdating = true;

	if (!Timer) {
		Timer = 2;

		if ((unsigned)Pos > strlen((const char *)DataPtr) && Stage > 2) {
			return(true);
		}

		if (State == false) {
			Rect rect;
			int w = Font->String_Width((const char *)DataPtr);
			rect.Set(XPos - 8 <= 0 ? 0 : XPos - 8, YPos, w + 10, Font->Get_Height());
			HiddenSurface->Blit_From(rect, *AlternateSurface, rect);
		}

		int x = XPos;
		for (int i = 0; i <= Pos; i++) {

			char localstr;
			localstr = ((char *)DataPtr)[i];
			if (localstr == '\0') {
				Stage++;
				break;
			}

			int v;
			if (State != true) {
				v = (Pos - i >= 2) ? 2 : (Pos - i);
			} else {
				v = 2;
			}

			Font->Print_Char(HiddenSurface, localstr, x, YPos, v, true);
			if (v == 2) {
				Font->Print_Char(AlternateSurface, localstr, x, YPos, v, false);
			}
			x += Font->Char_Width(localstr) + 1;
		}

		Pos++;
	}

	return(false);
}


/// <summary>
/// Constructs a score screen sound effect entry.
/// The sample is fetched from the mixfiles when it lives there, and otherwise loaded from
/// disk with this entry taking ownership of the buffer. Nothing is loaded at all when
/// audio is unavailable.
/// </summary>
/// <param name="name">The name the score screen will ask for this sound by.</param>
/// <param name="filename">The file the sample is loaded from.</param>
SfxEntry::SfxEntry(char const * name, char const *filename) :
	Name(NULL),
	Sample(NULL),
	IsAllocated(false)
{
	if (Audio_Available()) {
		Name = strdup(name);
		Sample = (void *)MFCD::Retrieve(filename);
		if (Sample == NULL) {
			CCFileClass file(filename);
			Sample = Load_Alloc_Data(file);
			IsAllocated = true;
			file.Close();
			DebugString("ScoreScreen: Loaded %s\n", filename);
		}
	}
}


/// <summary>
/// Destroys a score screen sound effect entry.
/// Any playback still in progress is stopped before the sample is released, so a sound
/// can never outlive the buffer it is playing from. Samples that came out of a mixfile
/// are left alone.
/// </summary>
SfxEntry::~SfxEntry(void)
{
	if (Sample != NULL) {
		Audio.Stop_Sample_Playing(Sample);
		if (IsAllocated == true) {
			delete Sample;
		}
	}
	if (Name != NULL) {
		free(Name);
	}
}


/// <summary>
/// Constructs a score screen animation object.
/// This is the base for every animated element of the score screen -- the counters, the
/// typewriter text and the looping graphics all derive from it. Derived classes supply
/// the shape or string data and drive the object from their own Update routine.
/// </summary>
/// <param name="data">The shape or string data this object will draw from.</param>
ScoreAnimClass::ScoreAnimClass(int x, int y, void const * data) :
	XPos(x),
	YPos(y),
	Timer(0),
	DataPtr(data)
{

}
