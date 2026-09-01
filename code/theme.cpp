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

/* $Header: /CounterStrike/THEME.CPP 3     3/11/97 4:03p Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : THEME.CPP                                                    *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : August 14, 1994                                              *
 *                                                                                             *
 *                  Last Update : August 12, 1996 [JLB]                                        *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   ThemeClass::AI -- Process the theme engine and restart songs.                             *
 *   ThemeClass::Base_Name -- Fetches the base filename for the theme specified.               *
 *   ThemeClass::From_Name -- Determines theme number from specified name.                     *
 *   ThemeClass::Full_Name -- Retrieves the full score name.                                   *
 *   ThemeClass::Is_Allowed -- Checks to see if the specified theme is legal.                  *
 *   ThemeClass::Next_Song -- Calculates the next song number to play.                         *
 *   ThemeClass::Play_Song -- Starts the specified song play NOW.                              *
 *   ThemeClass::Queue_Song -- Queues the song to the play queue.                              *
 *   ThemeClass::Scan -- Scans all scores for availability.                                    *
 *   ThemeClass::Set_Theme_Data -- Set the theme data for scenario and owner.                  *
 *   ThemeClass::Still_Playing -- Determines if music is still playing.                        *
 *   ThemeClass::Stop -- Stops the current theme from playing.                                 *
 *   ThemeClass::ThemeClass -- Default constructor for the theme manager class.                *
 *   ThemeClass::Theme_File_Name -- Constructs a filename for the specified theme.             *
 *   ThemeClass::Track_Length -- Calculates the length of the song (in seconds).               *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "theme.h"

#include "ccfile.h"
#include "ccini.h"
#include "ccrand.h"
#include "dbgprint.h"
#include "dsaudio.h"
#include "globals.h"
#include "houstype.h"
#include "incdec.h"
#include "session.h"
#include "vector.h"

#include <algorithm>


/// <summary>
/// Constructs a nameless and unavailable theme control.
/// The control is a blank slate until Fill_In supplies the real settings from the rules
/// database, so it belongs to no side and will not be considered for play.
/// </summary>
ThemeControl::ThemeControl(void) :
	Scenario(0),
	Duration(0),
	Normal(true),
	Repeat(false),
	Available(false),
	Owner(-1)
{
	Name[0] = '\0';
	Fullname[0] = '\0';

}


/// <summary>
/// Fetches this score's settings from the INI database.
/// This routine is used by Init_Themes to flesh out a theme control from the section
/// that bears its name. Any setting the section leaves out keeps the value it had.
/// </summary>
/// <param name="ini">The INI database to fetch the settings from.</param>
/// <returns>bool; Was a section for this score found?</returns>
bool ThemeControl::Fill_In(CCINIClass const & ini)
{
	if (ini.Is_Present(Name)) {
		ini.Get_String(Name, "Name", Fullname, Fullname, sizeof(Fullname));
		Scenario = ini.Get_Int(Name, "Scenario", Scenario);
		Duration = ini.Get_Float(Name, "Length", Duration);
		Normal = ini.Get_Bool(Name, "Normal", Normal);
		Repeat = ini.Get_Bool(Name, "Repeat", Repeat);
		Owner = ini.Get_Side(Name, "Side", (SideType)Owner);
		return(true);
	}
	return(false);
}


/// <summary>
/// Builds the theme list from the INI database.
/// This routine is used when the rules are read. A theme that is already known is
/// updated rather than duplicated, so a later rules file may amend the scores that an
/// earlier one declared as well as add scores of its own.
/// </summary>
/// <param name="ini">The INI database to fetch the theme list from.</param>
void ThemeClass::Init_Themes(CCINIClass const & ini)
{
	ThemeControl *ctrl;
	int count = ini.Entry_Count("Themes");
	for (int i = 0; i < count; i++) {
		char name[32];
		if (ini.Get_String("Themes", ini.Get_Entry("Themes", i), "", name, sizeof(name))) {

			ThemeType theme = From_Name(name);
			if (theme == THEME_NONE) {
				ctrl = new ThemeControl;
				strcpy(ctrl->Name, name);
				Themes.Add(ctrl);
			} else {
				ctrl = Themes[theme];
			}

			ctrl->Fill_In(ini);
		}
	}
}


/// <summary>
/// Frees the list of theme controls.
/// This routine is used before the rules are read afresh, and on the way out of the
/// game, so that the controls created by Init_Themes do not accumulate.
/// </summary>
void ThemeClass::Free_Themes(void)
{
	while (Themes.Count() > 0) {
		delete Themes[0];
		Themes.Delete_Index(0);
	}
}


/***********************************************************************************************
 * ThemeClass::Scan -- Scans all scores for availability.                                      *
 *                                                                                             *
 *    This routine should be called whenever a score mixfile is registered. It will scan       *
 *    to see if any score is unavailable. If this is the case, then the score will be so       *
 *    flagged in order not to appear on the play list. This condition is likely to occur       *
 *    when expansion mission disks contain a different score mix than the release version.     *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/04/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void ThemeClass::Scan(void)
{
	char name[_MAX_FNAME+_MAX_EXT];

	if (ScoresPresent && Audio_Available() && !Debug_Quiet) {
		for (ThemeType theme = THEME_FIRST; theme < Themes.Count(); theme++) {
			_makepath(name, NULL, NULL, Themes[theme]->Name, ".AUD");
			Themes[theme]->Available = CCFileClass(name).Is_Available();
		}
	}
}


/***********************************************************************************************
 * ThemeClass::Base_Name -- Fetches the base filename for the theme specified.                 *
 *                                                                                             *
 *    This routine is used to retrieve a pointer to the base filename for the theme            *
 *    specified.                                                                               *
 *                                                                                             *
 * INPUT:   theme -- The theme number to convert into a base filename.                         *
 *                                                                                             *
 * OUTPUT:  Returns with a pointer to the base filename for the theme specified. If the        *
 *          theme number is invalid, then a pointer to "No Theme" is returned instead.         *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/29/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
char const * ThemeClass::Base_Name(ThemeType theme) const
{
	if ((unsigned)theme < (unsigned)Themes.Count()) {
		return(Themes[theme]->Name);
	}
	return("No theme");
}


/***********************************************************************************************
 * ThemeClass::ThemeClass -- Default constructor for the theme manager class.                  *
 *                                                                                             *
 *    This is the default constructor for the theme class object.                              *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/16/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
ThemeClass::ThemeClass(void) :
	Current(-1),
	Score(THEME_NONE),
	Pending(THEME_NONE),
	Volume(255),
	IsRepeat(false),
	IsShuffle(false)
{
}


/***********************************************************************************************
 * ThemeClass::Full_Name -- Retrieves the full score name.                                     *
 *                                                                                             *
 *    This routine will fetch and return with a pointer to the full name of the theme          *
 *    specified.                                                                               *
 *                                                                                             *
 * INPUT:   theme -- The theme to fetch the full name for.                                     *
 *                                                                                             *
 * OUTPUT:  Returns with a pointer to the full name for this score. This pointer may point to  *
 *          EMS memory.                                                                        *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/16/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
char const * ThemeClass::Full_Name(ThemeType theme) const
{
	if ((unsigned)theme < (unsigned)Themes.Count()) {
		return(Themes[theme]->Fullname);
	}
	return(NULL);
}


/***********************************************************************************************
 * ThemeClass::AI -- Process the theme engine and restart songs.                               *
 *                                                                                             *
 *    This is a maintenance function that will restart an appropriate theme if the current one *
 *    has finished. This routine should be called frequently.                                  *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/08/1994 JLB : Created.                                                                 *
 *   01/23/1995 JLB : Picks new song just as it is about to play it.                           *
 *=============================================================================================*/
void ThemeClass::AI(void)
{
	if (Audio_Available() && !Debug_Quiet) {
		if (ScoresPresent && Volume > 0 && !Still_Playing()) {
			if (Pending != THEME_NONE && Pending != THEME_QUIET && !ScenarioInit) {
				/*
				**	If the pending song needs to be picked, then pick it now.
				*/
				if (Pending == THEME_PICK_ANOTHER) {
					Pending = Next_Song(Score);
					DebugString("Theme::AI(Next song = %d)\n", Pending);
				}

				/*
				**	Start the song playing and then flag it so that a new song will
				**	be picked when this one ends.
				*/
				Play_Song(Pending);
				Pending = THEME_PICK_ANOTHER;
			}
		}
		Audio.Sound_Callback();
	}
}


/***********************************************************************************************
 * ThemeClass::Next_Song -- Calculates the next song number to play.                           *
 *                                                                                             *
 *    use this routine to figure out what song number to play. It examines the option settings *
 *    for repeat and shuffle so that it can return the correct value.                          *
 *                                                                                             *
 * INPUT:   theme -- The origin (last) index. The new value is related to this for all but     *
 *                   the shuffling method of play.                                             *
 *                                                                                             *
 * OUTPUT:  Returns with the song number for the next song to play.                            *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/16/1995 JLB : Created.                                                                 *
 *   01/19/1995 JLB : Will not play the same song twice when in shuffle mode.                  *
 *=============================================================================================*/
ThemeType ThemeClass::Next_Song(ThemeType theme) const
{
	int i;

	// A score the game does not hold would repeat forever, and nothing else would be picked.
	if ((unsigned)theme >= (unsigned)Themes.Count() || !Themes[theme]->Available ||
		(!Themes[theme]->Repeat && !IsRepeat)) {
		if (IsShuffle == true) {

			/*
			**	Shuffle the theme, but never pick the same theme that was just
			**	playing.
			*/
			ThemeType newtheme;
			i = 0;
			do {
				newtheme = (ThemeType)NonCriticalRandomNumber(THEME_FIRST, Themes.Count() - 1);
				i++;
			} while (i < 1000 && (newtheme == theme || !Is_Allowed(newtheme)));
			if (i == 1000) {
				newtheme = THEME_FIRST;
			}
			return(newtheme);

		} else {
			i = Themes.Count();
			i++;
			/*
			**	Sequential score playing.
			*/
			do {
				theme = ThemeType(theme + 1);//theme++;
				if (theme >= Themes.Count()) {
					theme = THEME_FIRST;
				}
				i--;
				if (i == 0) {
					return(THEME_FIRST);
				}
			} while (!Is_Allowed(theme));
		}
	}
	return(theme);
}


/***********************************************************************************************
 * ThemeClass::Queue_Song -- Queues the song to the play queue.                                *
 *                                                                                             *
 *    This routine will cause the current song to fade and the specified song to start. This   *
 *    is the normal and friendly method of changing the current song.                          *
 *                                                                                             *
 * INPUT:   theme -- The song to start playing. If -1 is passed in, then just the current song.*
 *                   is faded.                                                                 *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/16/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void ThemeClass::Queue_Song(ThemeType theme)
{
	/*
	**	If there is no score file present, then abort.
	*/
	if (!ScoresPresent) return;

	/*
	**	If there is no sound driver or sounds have been specifically
	**	turned off, then abort.
	*/
	if (!Audio_Available() || Debug_Quiet) return;

	/*
	**	If the current score volumne is set to silent, then there is no need to play the
	**	specified theme.
	*/
	if (Volume == 0) return;

	/*
	**	If the pending theme is available to be set and the specified theme is valid, then
	**	set the queued theme accordingly.
	*/
	if (Pending == THEME_NONE || Pending == THEME_PICK_ANOTHER || theme == THEME_NONE || theme == THEME_QUIET) {
		Pending = theme;
		DebugString("Theme::QueueSong(%d)\n", theme);
		if (Still_Playing() == true) {
			Audio.Fade_Sample(Current, THEME_DELAY);
		}
	}
}


/***********************************************************************************************
 * ThemeClass::Play_Song -- Starts the specified song play NOW.                                *
 *                                                                                             *
 *    This routine is used to start the specified theme playing right now. If there is already *
 *    a theme playing, it is cut short so that this one may start.                             *
 *                                                                                             *
 * INPUT:   theme -- The theme number to start playing.                                        *
 *                                                                                             *
 * OUTPUT:  Returns with the sample play handle.                                               *
 *                                                                                             *
 * WARNINGS:   This cuts off any current song in a abrupt manner. Only use this routine when   *
 *             necessary.                                                                      *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/16/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int ThemeClass::Play_Song(ThemeType theme)
{
	if (ScoresPresent && Audio_Available() && !Debug_Quiet) {
		Stop(false);
		if (theme != THEME_NONE && theme != THEME_QUIET) {
			if (theme > THEME_NONE && Volume > 0) {
				Audio.StreamLowImpact = true;
				Current = Audio.File_Stream_Sample_Vol(Theme_File_Name(theme), Volume, true);
				Audio.StreamLowImpact = false;

				/*
				 * Stopping a score that never started does nothing, so recording one that
				 * failed to start as the current score would silence the game for good.
				 */
				if (Current == -1) {
					DebugString("Theme::PlaySong(%d) - Unavailable\n", theme);
					Score = THEME_NONE;
					Pending = THEME_NONE;
					return(Current);
				}

				Score = theme;
				DebugString("Theme::PlaySong(%d) - %s\n", Score, IsRepeat == true || Themes[theme]->Repeat == true ? "Repeating" : "Playing");
				if (IsRepeat == true || Themes[theme]->Repeat == true) {
					Pending = theme;
				}
			} else {
				Pending = theme;
			}
		}
	}
	return(Current);
}


/***********************************************************************************************
 * ThemeClass::Theme_File_Name -- Constructs a filename for the specified theme.               *
 *                                                                                             *
 *    This routine will construct (into a static buffer) a filename that matches the theme     *
 *    number specified. This constructed filename is returned as a pointer. The filename will  *
 *    remain valid until the next call to this routine.                                        *
 *                                                                                             *
 * INPUT:   theme -- The theme number to convert to a filename.                                *
 *                                                                                             *
 * OUTPUT:  Returns with a pointer to the constructed filename for the specified theme number. *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/16/1995 JLB : Created.                                                                 *
 *   05/09/1995 JLB : Theme variation support.                                                 *
 *=============================================================================================*/
char const * ThemeClass::Theme_File_Name(ThemeType theme)
{
	static char name[_MAX_FNAME+_MAX_EXT];

	if ((unsigned)theme < (unsigned)Themes.Count()) {
		_makepath(name, NULL, NULL, Themes[theme]->Name, ".AUD");
		return((char const *)(&name[0]));
	}

	return("");
}


/***********************************************************************************************
 * ThemeClass::Track_Length -- Calculates the length of the song (in seconds).                 *
 *                                                                                             *
 *    Use this routine to calculate the length of the song. The length is determined by        *
 *    reading the header of the song and dividing the sample rate into the sample length.      *
 *                                                                                             *
 * INPUT:   theme -- The song number to examine to find its length.                            *
 *                                                                                             *
 * OUTPUT:  Returns with the length of the specified theme. This length is in the form of      *
 *          seconds.                                                                           *
 *                                                                                             *
 * WARNINGS:   This routine goes to disk to fetch this information. Don't call frivolously.    *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/16/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int ThemeClass::Track_Length(ThemeType theme) const
{
	if ((unsigned)theme < (unsigned)Themes.Count()) {
		return(Themes[theme]->Duration * 60);
	}
	return(0);
}


/***********************************************************************************************
 * ThemeClass::Stop -- Stops the current theme from playing.                                   *
 *                                                                                             *
 *    Use this routine to stop the current theme. After this routine is called, no more music  *
 *    will play until the Start() function is called.                                          *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/08/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void ThemeClass::Stop(bool fade)
{
	if (ScoresPresent && Audio_Available() && !Debug_Quiet && Current != -1) {

		if (fade && Still_Playing() == true) {
			DebugString("Theme::Stop(%d) - Fading\n", Score);
			Audio.Fade_Sample(Current, THEME_DELAY);
		} else {
			DebugString("Theme::Stop(%d)\n", Score);
			Audio.Stop_Sample(Current);
		}
		Current = -1;
		Score = THEME_NONE;
		Pending = THEME_NONE;
	}
}


/// <summary>
/// Suspends the score that is currently playing.
/// Unlike Stop(), this routine remembers the score it silenced by leaving it pending, so
/// the next AI() pass will start the same track over again.
/// </summary>
void ThemeClass::Suspend(void)
{
	if (ScoresPresent && Audio_Available() && !Debug_Quiet && Current != -1) {
		DebugString("Theme::Suspend(%d)\n", Score);
		Audio.Stop_Sample(Current);
		Current = -1;
		Pending = Score;
		Score = THEME_NONE;
	}
}


/***********************************************************************************************
 * ThemeClass::Still_Playing -- Determines if music is still playing.                          *
 *                                                                                             *
 *    Use this routine to determine if music is still playing.                                 *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  bool; Is the music still audible?                                                  *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/20/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
bool ThemeClass::Still_Playing(void) const
{
	if (ScoresPresent && Audio_Available() && Volume > 0 && Current != -1 && !Debug_Quiet) {
		return(Audio.Sample_Status(Current));
	}
	return(false);
}


/***********************************************************************************************
 * ThemeClass::Is_Allowed -- Checks to see if the specified theme is legal.                    *
 *                                                                                             *
 *    Use this routine to determine if a theme is allowed to be played. A theme is not allowed *
 *    if the scenario is too early for that score, or the score only is allowed in special     *
 *    cases.                                                                                   *
 *                                                                                             *
 * INPUT:   index -- The score the check to see if it is allowed to play.                      *
 *                                                                                             *
 * OUTPUT:  Is the specified score allowed to play in the normal score playlist?               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/09/1995 JLB : Created.                                                                 *
 *   07/04/1996 JLB : Handles alternate playlist checking.                                     *
 *=============================================================================================*/
bool ThemeClass::Is_Allowed(ThemeType index) const
{
	if (index == THEME_QUIET || index == THEME_PICK_ANOTHER) return(true);

	if ((unsigned)index >= (unsigned)Themes.Count()) return(false);

	/*
	**	If the theme is not present, then it certainly isn't allowed.
	*/
	if (!Themes[index]->Available) return(false);

	/*
	**	Only normal themes (playable during battle) are considered allowed.
	*/
	if (!Themes[index]->Normal) return(false);

	/*
	**	If the theme is not allowed to be played by the player's house, then don't allow
	**	it. If the player's house hasn't yet been determined, then presume this test
	**	passes.
	*/
	if (PlayerPtr != NULL && Themes[index]->Owner != -1 && PlayerPtr->Class->Side != Themes[index]->Owner) return(false);

	/*
	**	If the scenario doesn't allow this theme yet, then return the failure flag. The
	**	scenario check only makes sense for solo play.
	*/
	if (Session.Type == GAME_NORMAL && Scen->Scenario < Themes[index]->Scenario) return(false);

	/*
	**	Since all tests passed, return with the "is allowed" flag.
	*/
	return(true);
}


/***********************************************************************************************
 * ThemeClass::From_Name -- Determines theme number from specified name.                       *
 *                                                                                             *
 *    Use this routine to convert a name (either the base filename of the theme, or a partial  *
 *    substring of the full name) into the matching ThemeType value. Typical use of this is    *
 *    when parsing the INI file for theme control values.                                      *
 *                                                                                             *
 * INPUT:   name  -- Pointer to base filename of theme or a partial substring of the full      *
 *                   theme name.                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the matching theme number. If no match could be found, then           *
 *          THEME_NONE is returned.                                                            *
 *                                                                                             *
 * WARNINGS:   If a filename is specified the comparison is case insensitive. When scanning    *
 *             the full theme name, the comparison is case sensitive.                          *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/29/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
ThemeType ThemeClass::From_Name(char const * name) const
{
	if (name && strlen(name) > 0) {
		/*
		**	First search for an exact name match with the filename
		**	of the theme. This is guaranteed to be unique.
		*/
		ThemeType theme;
		for (theme = THEME_FIRST; theme < Themes.Count(); theme = ThemeType(theme + 1)) {
			if (stricmp(Themes[theme]->Name, name) == 0) {
				return(theme);
			}
		}

		/*
		**	If the filename scan failed to find a match, then scan for
		**	a substring within the full name of the score. This might
		**	yield a match, but is not guaranteed to be unique.
		*/
		for (theme = THEME_FIRST; theme < Themes.Count(); theme = ThemeType(theme + 1)) {
			if (strstr(Themes[theme]->Fullname, name) != NULL) {
				return(theme);
			}
		}
	}

	return(THEME_NONE);
}


/// <summary>
/// Sets the volume that the music is played at.
/// This routine is used by the options screen. Any score that happens to be playing has
/// its volume adjusted right away rather than waiting for the next track to start.
/// </summary>
/// <param name="volume">The volume to play the music at. Anything louder than maximum
/// volume is quietly clipped.</param>
void ThemeClass::Set_Volume(int volume)
{
	Volume = std::min(volume, 255);

	if (Current != -1) {
		Audio.Set_Handle_Volume(Current, Volume);
	}
}
