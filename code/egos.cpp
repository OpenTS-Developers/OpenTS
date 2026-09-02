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

/* $Header: /counterstrike/EGOS.CPP 2     3/10/97 3:19p Steve_tall $ */
/*************************************************************************************
 **   C O N F I D E N T I A L --- W E S T W O O D    S T U D I O S                  **
 *************************************************************************************
 *                                                                                   *
 *                 Project Name : Command & Conquer - Red Alert                      *
 *                                                                                   *
 *                    File Name : EGOS.CPP                                           *
 *                                                                                   *
 *                   Programmer : Steve Tall                                         *
 *                                                                                   *
 *                   Start Date : September 4th, 1996                                *
 *                                                                                   *
 *                  Last Update : September 4th, 1996 [ST]                           *
 *                                                                                   *
 *-----------------------------------------------------------------------------------*
 * Overview:                                                                         *
 *                                                                                   *
 *   Scrolling movie style credits.                                                  *
 *                                                                                   *
 *-----------------------------------------------------------------------------------*
 * Functions:                                                                        *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "egos.h"

#include "_keyboar.h"
#include "_palette.h"
#include "_rect.h"
#include "_surface.h"
#include "_timer.h"
#include "addon.h"
#include "ccfile.h"
#include "conquer.h"
#include "dialog.h"
#include "dsurface.h"
#include "font.h"
#include "gadget.h"
#include "globals.h"
#include "goptions.h"
#include "gscreen.h"
#include "language/language.h"
#include "misc.h"
#include "ownrdraw.h"
#include "scheme.h"
#include "theme.h"
#include "vector.h"
#include "windlg.h"

#include "color.hh"
#include "dialog.hh"

#include <algorithm>
#include <cstring>

/*
**	List of Ego Class instances
**	There will be one instance for each line of text.
*/
DynamicVectorClass<EgoClass *> EgoList;

/*
**	Number of slideshow pictures
*/

#define NUM_SLIDES 17

/*
**	Length of time frame is displayed for
*/
#define FRAME_DELAY	150

/*
**	Number of frames that palete fade occurs over
*/
#define FADE_DELAY		37

/*
**	Original copy of slide (pref in video mem) that we use to undraw the text
*/
Surface *BackgroundSurface;


/***********************************************************************************************
 * EC::EgoClass -- EgoClass constructor                                                        *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    x position of text                                                                *
 *           y position of text                                                                *
 *           ptr to text string                                                                *
 *           flags to print text with                                                          *
 *                                                                                             *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *    9/9/96 11:53PM ST : Created                                                              *
 *=============================================================================================*/
EgoClass::EgoClass (int x, int y, char *text, TextPrintType flags)
{
	XPos = x;
	YPos = y;
	Flags= flags;
	Text = new char [strlen (text)+1];
	strcpy (Text, text);
}


/***********************************************************************************************
 * EC::~EgoClass -- EgoClass destructor                                                        *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *    9/9/96 11:54PM ST : Created                                                              *
 *=============================================================================================*/
EgoClass::~EgoClass(void)
{
	delete [] Text;
}


/***********************************************************************************************
 * EC::Scroll -- Apply the given distance to the y position of the text.                       *
 *               A positive distance scrolls up.                                               *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    distance in pixels to scroll up                                                   *
 *                                                                                             *
 * OUTPUT:   true if text scrolled beyond the top of the screen                                *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *    9/9/96 11:55PM ST : Created                                                              *
 *=============================================================================================*/
bool EgoClass::Scroll(int distance)
{
	YPos -= distance;
	if (YPos < -20) {
		return(true);
	}else{
		return(false);
	}
}


/***********************************************************************************************
 * EC::Render -- Draws the text to the logic page                                              *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *    9/9/96 11:57PM ST : Created                                                              *
 *=============================================================================================*/
void EgoClass::Render(bool fresh)
{
	if ((YPos < LogicalSurface->Get_Height() && YPos > LogicalSurface->Get_Height() - 52) || YPos >= -16 && YPos <= 32 || fresh) {
		static HFONT font;
		if (font == NULL) {
			HDC dc = GetDC(MainWindow);
			font = WS_Get_Font(dc, "Arial", 0, 16, 1);
			ReleaseDC(MainWindow, dc);
		}

		Rect textrect(XPos, YPos, VideoModeWidth, 0);

		int alignment = 0;
		if (Flags & TPF_CENTER) {
			alignment = OD_TEXT_ALIGN_CENTER;
		}else{
			if (Flags & TPF_RIGHT){
				alignment = OD_TEXT_ALIGN_MAX;
			}
		}

		if (GameInFocus) {
			OD_Draw_Text(RGB(255, 255, 128), font, textrect, Text, strlen(Text), alignment, 0, BackgroundSurface);
		}
	}
}


/***********************************************************************************************
 * EC::Wipe -- Wipes the previously rendered text by blitting a rectangle from the given       *
 *             background screen.                                                              *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    ptr to screen containing original background                                      *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *    9/9/96 11:58PM ST : Created                                                              *
 *=============================================================================================*/
void EgoClass::Wipe (Surface *background)
{
	int width = Font_From_TPF(Flags)->String_Pixel_Width (Text);
	int x = XPos;

	if (Flags & TPF_RIGHT) {
		x -= width;
	}else{
		if (Flags & TPF_CENTER){
			x -= width/2;
		}
	}

	Rect r(0, YPos, VideoModeWidth, 18);
	LogicalSurface->Blit_From(r, *background, r);
}


/***********************************************************************************************
 * Show_Who_Was_Responsible -- Main function to print the credits.                             *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *    9/10/96 0:20AM ST : Created                                                              *
 *=============================================================================================*/
void Show_Who_Was_Responsible (void)
{

	int	i;
	int	key = KN_NONE;

	/*
	**	Deault speed of credits scolling. This is the frame delay between pixel scrolls.
	*/
	static int speed = 2;

	if (Addon_Enabled(ADDON_FIRESTORM) == true) return;

	LogicalSurface = AlternateSurface;

	/*
	**	Read in the credits file to be displayed
	**
	**	Lines of text in CREDITS.TXT are treated as follows....
	**
	**	If the text starts and ends to the left of column 40 then text will be right justified.
	**	If the text starts before column 40 and ends after it then it will be centered.
	**	If the text starts after column 40 it will be right justified.
	*/
	CCFileClass creditsfile ("TSCREDIT.txt");
	if ( !creditsfile.Is_Available()) return;
	char *credits = new char [creditsfile.Size()+1];
	creditsfile.Read (credits, creditsfile.Size());

	/*
	**	Initialise the text printing system.
	*/
	GadgetClass::Set_Color_Scheme("Green");
	Fancy_Text_Print(TXT_NONE, *LogicalSurface, LogicalSurface->Get_Rect(), Point2D(0, 0), Fetch_Scheme_By_Name("LightGold"),
		TBLACK, TextPrintType(TPF_CENTER|TPF_METAL12|TPF_FULLSHADOW));


	/*
	**	Miscellaneous stuff for parsing the credits text file.
	*/
	int 				length = creditsfile.Size();
	int 				line   = 0;
	int 				column = 0;
	char				*cptr = credits;
	char				ch, lastchar, oldchar;
	char				*strstart, *strparse;
	bool				gotendstr;
	int				startcolumn, endcolumn, x;
	int				y=HiddenSurface->Get_Height()+2;
	int 				lastline = -2;
	EgoClass 		*ego;
	TextPrintType 	flags;


	/*
	**	Search through the text file and extract the strings, using each string to create
	**	a new EgoClass
	*/
	do {
		/*
		**	Search for text
		*/
		ch = *cptr++;
		length --;

		/*
		**	Look for a non whitespace character.
		*/
		switch ( ch ){

			case 13:
				/*
				**	Char was carriage return. Go on to the next line starting at column 0.
				*/
				line++;
				y += 16;
				column = 0;
				break;


			case 10:
				/*
				**	Ignore line feed. CR does both.
				*/
				break;

				/*
				**	Space character. Just advance the cursor and move on.
				*/
			case 32:
				column++;
				break;

				/*
				**	Tab char. Advance to the next tab column. Tabs are every 8 columns.
				*/
			case 9:
				column += 8;
				column &= 0xfffffff8;
				break;

			default:
				/*
				**	Found new string. Work out where it ends so we know how to treat it.
				*/
				lastchar = ch;
				strstart = cptr-1;
				strparse = cptr-1;
				endcolumn = startcolumn = column;
				gotendstr = false;

				do	{
					ch = *strparse++;
					switch ( ch ){
						case 9:
						case 10:
						case 13:
							gotendstr = true;
							break;

						case 32:
							if (lastchar == 32) gotendstr = true;
							//endcolumn++;
							break;

						default:
							//endcolumn++;
							break;
					}
					if (strparse >= cptr+length) gotendstr = true;

					lastchar = ch;
				}while (!gotendstr);


				if (strparse >= cptr+length) break;

				/*
				**	Strip off any trailing space.
				*/
				if (*(strparse-2) == 32){
					strparse--;
					//endcolumn -= 2;
				}


				/*
				**	If string straddles the center column then center it.
				**
				**	If string is on the left hand side then right justify it.
				**
				**	If string is on the right hand side then left justify it.
				*/
				flags = TextPrintType(TPF_METAL12 | TPF_FULLSHADOW );

				if (startcolumn > 3 && endcolumn < 8){
					flags = TextPrintType(flags | TPF_CENTER);
					x = HiddenSurface->Get_Width() / 2;
				}else{
					if (startcolumn <3){
						flags = TextPrintType(flags | TPF_RIGHT);
						//x = endcolumn *HiddenSurface->Get_Width() /80;
						x = (HiddenSurface->Get_Width() /2) - (HiddenSurface->Get_Width() /12);
					}else{
						//x = startcolumn * HiddenSurface->Get_Width() / 2;
						x = HiddenSurface->Get_Width() / 2;
					}
				}

				if (startcolumn > 3 && endcolumn < 8 && line - lastline > 1) {
					flags = TextPrintType(flags | TPF_LIGHTSHADOW);
				} else {
					flags = TextPrintType(flags | TPF_DROPSHADOW);
				}
				lastline = line;

				/*
				**	Temporarily terminate the string.
				*/
				oldchar = *(strparse-1);
				*(strparse-1) = 0;

				/*
				**	Create the new class and add it to our list.
				*/
				ego = new EgoClass (x, y, strstart, flags);

				EgoList.Add (ego);

				/*
				**	Restore the character that was lost when we added the terminator.
				*/
				*(strparse-1) = oldchar;

				/*
				**	Fix up our outer loop parsing variables.
				*/
				cptr = strparse;
				column += strparse - strstart;
				length -= strparse - strstart-1;

				if (ch == 13) {
					line++;
					y += 16;
					column = 0;
				}else{
					if (ch == 9){
						column += 7;
						column &= 0xfffffff8;
					}
				}
				break;
		}

	} while ( length>0 );

	/*
	**	Stop the music.
	*/
	Theme.Stop();

	/*
	**	Clear the Seen Page since we will not be blitting to all of it.
	*/
	HiddenSurface->Fill(TBLACK);
	Update_Visible_Surface();

	/*
	**	Create a new graphic buffer to restore the background from. Initialise it to black so
	**	we can start scrolling before the first slideshow picture is blitted.
	*/
	BackgroundSurface = new DSurface(VisibleRect.Width, VisibleRect.Height);
	BackgroundSurface->Blit_From(*HiddenSurface);

	/*
	**	Go away nasty keyboard.
	*/
	Keyboard->Clear();

	LogicalSurface = HiddenSurface;

	/*
	**	Start any old song.
	*/
	float oldvolume = Options.ScoreVolume;
	if ((double)oldvolume == 0) {
		Options.Set_Score_Volume(0.4f, false);
	}
	Theme.Queue_Song(Theme.From_Name("MADRAP"));

	/*
	**	Init misc timing variables.
	*/
	int time  = TickCount;
	int frame = 0;
	int picture_frame = 0;
	int slide_number = 0;
	bool fresh = false;

	Hide_Mouse();

	/*
	**	Main scrolling loop.
	**	Keeps going until all the EgoClass objects are deleted or esc is pressed.
	*/
	while ( EgoList.Count() ){

		frame++;

		/*
		**	Update the slideshow frame and switch to the next picture if its time.
		*/
		picture_frame++;

		if (picture_frame > FRAME_DELAY+50){
			if (slide_number <NUM_SLIDES-1){
				slide_number++;
				picture_frame = 0;
			}else{
				//slide_number = 0;
				//picture_frame = 0;
			}
		}

		/*
		**	Scroll the text. If any text goes off the top then delete that object.
		*/
		for (i=EgoList.Count()-1 ; i>=0 ; i--){
			//EgoList[i]->Wipe(BackgroundSurface);
			if ( EgoList[i]->Scroll(2) ){
				delete EgoList[i];
				EgoList.Delete_Index(i);
				break;
			}
		}

		/*
		 * The scroll is performed by copying the middle of the visible surface two
		 * scan lines upward. The top and bottom bands lie outside that copy, so they
		 * are cleared and then refreshed from the background surface every frame.
		 */
		Rect scroll_dest(0, 32, VideoModeWidth, VideoModeHeight - 66);
		Rect scroll_source(0, 34, VideoModeWidth, VideoModeHeight - 66);
		Rect top_band(0, 0, VideoModeWidth, 34);
		Rect bottom_band(0, VideoModeHeight - 34, VideoModeWidth, 34);
		BackgroundSurface->Fill_Rect(top_band, TBLACK);
		BackgroundSurface->Fill_Rect(bottom_band, TBLACK);

		/*
		**	Render all the text strings in their new positions.
		*/
		for (i=EgoList.Count()-1 ; i>=0 ; i--){
			EgoList[i]->Render(fresh);
		}

		int step = 0;
		unsigned short *bsurf = (unsigned short *)BackgroundSurface->Lock();
		if (bsurf) {
			step = BackgroundSurface->Stride() / 2;
			int fade_x;
			int fade_y = 0;
			int xidx;
			int yidx;
			for (int raw_alpha = 256; raw_alpha > 0; raw_alpha -= 8) {
				int alpha = std::min(raw_alpha, 255);
				xidx = step * fade_y;
				yidx = step * (VideoModeHeight - fade_y - 1);
				for (fade_x = 0; fade_x < VideoModeWidth; fade_x++) {
					(bsurf + xidx)[fade_x] = OD_Blend_Color((bsurf + xidx)[fade_x], 0, alpha);
					(bsurf + yidx)[fade_x] = OD_Blend_Color((bsurf + yidx)[fade_x], 0, alpha);
				}
				fade_y++;
			}
			BackgroundSurface->Unlock();
		}

		if (GameInFocus) {
			if (fresh) {
				VisibleSurface->Blit_From(*BackgroundSurface);
			} else {
				VisibleSurface->Blit_From(top_band, *BackgroundSurface, top_band);
				VisibleSurface->Blit_From(scroll_dest, *VisibleSurface, scroll_source);
				VisibleSurface->Blit_From(bottom_band, *BackgroundSurface, bottom_band);
			}
		}

		fresh = false;
		if (frame > 1000 && !Theme.Still_Playing()){
			Theme.Queue_Song(THEME_NONE);
		}

		/*
		**	Stop calling Theme.AI after a while so a different song doesnt start playing
		*/
		Call_Back();

		/*
		**	Kill any spare time before blitting the hid page forward.
		*/
		while (TickCount - time < frame * speed && !Keyboard->Check()) {}

		/*
		**	If user hits escape then break.
		*/
		key = KN_NONE;
		if (Keyboard->Check()){
			key = Keyboard->Get();
			if (key == KN_ESC){
				break;
			}
#if (0)
			if (key == KN_Z){
				speed--;
				if (speed <1 ) speed=1;
				time = TickCount;
				frame = 0;
			}
			if (key == KN_X){
				speed++;
				time = TickCount;
				frame = 0;
			}
#endif	//(0)

		}

	}

	if (key == KN_ESC){
		Theme.Fade_Out();
	}else{
		/*
		**	Wait for the picture to fade down
		*/
		while (picture_frame <= FADE_DELAY+FRAME_DELAY){
			if (picture_frame < FRAME_DELAY && picture_frame > 10+FADE_DELAY){
				picture_frame = FRAME_DELAY;
			}
			frame++;
			picture_frame++;

			Call_Back();

			/*
			**	Kill any spare time
			*/
			while (TickCount - time < frame * speed && !Keyboard->Check()) {}

		}
	}

	/*
	**	Tidy up.
	*/
	HiddenSurface->Fill(TBLACK);
	Update_Visible_Surface();

	Show_Mouse();

	GadgetClass::Set_Color_Scheme("Blue");

	Theme.Stop();
	Options.Set_Score_Volume(oldvolume, false);

	delete BackgroundSurface;

	delete [] credits;

	while (EgoList.Count() > 0) {
		delete EgoList[0];
		EgoList.Delete_Index(0);
	}
	EgoList.Clear();
}
