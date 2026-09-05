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

/* $Header: /CounterStrike/OPTIONS.CPP 1     3/03/97 10:25a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : OPTIONS.CPP                                                  *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : June 8, 1994                                                 *
 *                                                                                             *
 *                  Last Update : July 30, 1996 [JLB]                                          *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   OptionsClass::Adjust_Palette -- Adjusts the palette according to the settings specified.  *
 *   OptionsClass::Fixup_Palette -- Adjusts the real palette to match the palette sliders.     *
 *   OptionsClass::Get_Brightness -- Fetches the current brightness setting.                   *
 *   OptionsClass::Get_Contrast -- Gets the current contrast setting.                          *
 *   OptionsClass::Get_Game_Speed -- Fetches the current game speed setting.                   *
 *   OptionsClass::Get_Saturation -- Fetches the current color setting.                        *
 *   OptionsClass::Get_Scroll_Rate -- Fetches the current scroll rate setting.                 *
 *   OptionsClass::Get_Tint -- Fetches the current tint setting.                               *
 *   OptionsClass::Load_Settings -- reads options settings from the INI file                   *
 *   OptionsClass::Normalize_Delay -- Normalizes delay factor to keep rate constant.           *
 *   OptionsClass::Normalize_Volume -- Convert to a real volume value.                         *
 *   OptionsClass::One_Time -- This performs any one time initialization for the options class.*
 *   OptionsClass::OptionsClass -- The default constructor for the options class.              *
 *   OptionsClass::Process -- Handles all the options graphic interface.                       *
 *   OptionsClass::Save_Settings -- writes options settings to the INI file                    *
 *   OptionsClass::Set -- Sets options based on current settings                               *
 *   OptionsClass::Set_Brightness -- Sets the brightness level to that specified.              *
 *   OptionsClass::Set_Contrast -- Sets the contrast to the value specified.                   *
 *   OptionsClass::Set_Game_Speed -- Sets the game speed as specified.                         *
 *   OptionsClass::Set_Repeat -- Controls the score repeat option.                             *
 *   OptionsClass::Set_Saturation -- Sets the color to the value specified.                    *
 *   OptionsClass::Set_Score_Volume -- Sets the global score volume to that specified.         *
 *   OptionsClass::Set_Scroll_Rate -- Sets the scroll rate as specified.                       *
 *   OptionsClass::Set_Shuffle -- Controls the play shuffle setting.                           *
 *   OptionsClass::Set_Sound_Volume -- Sets the sound effects volume level.                    *
 *   OptionsClass::Set_Tint -- Sets the tint setting.                                          *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "options.h"

#include "_command.h"
#include "_map.h"
#include "_rules.h"
#include "audio/audioengine.h"
#include "ccfile.h"
#include "ccrand.h"
#include "command.h"
#include "dbgprint.h"
#include "dsurface.h"
#include "globals.h"
#include "init.h"
#include "ipxmgr.h"
#include "keyboard.h"
#include "language/language.h"
#include "mouse.h"
#include "msgbox.h"
#include "ownrdraw.h"
#include "rules.h"
#include "session.h"
#include "techno.h"
#include "theme.h"
#include "vector.h"
#include "video.h"
#include "vox.h"

#include "diff.hh"

#include <algorithm>


char const * const OptionsClass::HotkeyName = "WinHotkeys";


/***********************************************************************************************
 * OptionsClass::OptionsClass -- The default constructor for the options class.                *
 *                                                                                             *
 *    This is the constructor for the options class. It handles setting up all the globals     *
 *    necessary for the options. This includes setting them to their default state.            *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/21/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
OptionsClass::OptionsClass(void) :
	Difficulty(DIFF_NORMAL),
	GameSpeed(3),
	ScrollRate(3),
	SoundVolume(.7f),
	VoiceVolume(1.0f),
	ScoreVolume(.5f),
	AutoScroll(true),
	IsScoreRepeat(false),
	IsScoreShuffle(false),
	IsSidebarOnRight(true),
	SidebarCameoText(true),
	ActionLines(true),
	ToolTips(true),
	TextBackgroundColor(0),
	AutoSaveInterval(10800),
	ScreenWidth(-1),
	ScreenHeight(-1),
	ScrollMethod(0),
	DetailLevel(2),
	StretchMovies(0),
	Fullscreen(true),
	WindowWidth(-1),
	WindowHeight(-1),
	ScaleMode(VIDEO_SCALE_PIXELART),
	IntegerScaling(false),
	VSync(false),
	Renderer(0),
	CursorScale(0),
	SoundLatency(9),
	KeyForceMove1(KN_LALT),
	KeyForceMove2(KN_LALT),
	KeyForceAttack1(KN_LCTRL),
	KeyForceAttack2(KN_LCTRL),
	KeySelect1(KN_LSHIFT),
	KeySelect2(KN_LSHIFT),
	KeyQueueMove1(KN_Q),
	KeyQueueMove2(KN_Q)
{
}


/***********************************************************************************************
 * OptionsClass::One_Time -- This performs any one time initialization for the options class.  *
 *                                                                                             *
 *    This routine should be called only once and it will perform any initializations for the  *
 *    options class that is needed. This may include things like file loading and memory       *
 *    allocation.                                                                              *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   Only call this routine once.                                                    *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/21/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void OptionsClass::One_Time(void)
{
}


/***********************************************************************************************
 * OptionsClass::Set_Shuffle -- Controls the play shuffle setting.                             *
 *                                                                                             *
 *    This routine will control the score shuffle flag. The setting to use is provided as      *
 *    a parameter. When shuffling is on, the score play order is scrambled.                    *
 *                                                                                             *
 * INPUT:   on -- Should the shuffle option be activated?                                      *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/19/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void OptionsClass::Set_Shuffle(bool on)
{
	IsScoreShuffle = on;
	Theme.Set_Shuffle(on);
	DebugString("ScoreShuffle is %s\n", IsScoreShuffle == true ? "ON" : "OFF");
}


/***********************************************************************************************
 * OptionsClass::Set_Repeat -- Controls the score repeat option.                               *
 *                                                                                             *
 *    This routine is used to control whether scores repeat or not. The setting to use for     *
 *    the repeat flag is provided as a parameter.                                              *
 *                                                                                             *
 * INPUT:   on -- Should the scores repeat?                                                    *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/19/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void OptionsClass::Set_Repeat(bool on)
{
	IsScoreRepeat = on;
	Theme.Set_Repeat(on);
	DebugString("ScoreRepeat is %s\n", IsScoreRepeat == true ? "ON" : "OFF");
}


/***********************************************************************************************
 * OptionsClass::Set_Score_Volume -- Sets the global score volume to that specified.           *
 *                                                                                             *
 *    This routine will set the global score volume to the value specified. The value ranges   *
 *    from zero to 255.                                                                        *
 *                                                                                             *
 * INPUT:   volume   -- The new volume setting to use for scores.                              *
 *                                                                                             *
 *          feedback -- Should a feedback sound effect be generated?                           *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/19/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void OptionsClass::Set_Score_Volume(float volume, bool feedback)
{
	ScoreVolume = std::min(volume, 1.0f);
	Theme.Set_Volume(ScoreVolume * 255.0);
	if (feedback && !Theme.Still_Playing()) {
		Sound_Effect(Rule->GenericBeep, ScoreVolume);
	}
	DebugString("ScoreVolume = %f\n", ScoreVolume);
}


/***********************************************************************************************
 * OptionsClass::Set_Sound_Volume -- Sets the sound effects volume level.                      *
 *                                                                                             *
 *    This routine will set the sound effect volume level as indicated. It can generate a      *
 *    sound effect for feedback purposes if desired. The volume setting can range from zero    *
 *    to 255. The value of 255 is the loudest.                                                 *
 *                                                                                             *
 * INPUT:   volume   -- The volume setting to use for the new value. 0 to 255.                 *
 *                                                                                             *
 *          feedback -- Should a feedback sound effect be generated?                           *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/19/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void OptionsClass::Set_Sound_Volume(float volume, bool feedback)
{
	SoundVolume = std::min(volume, 1.0f);
	AudioEngine.Set_Group_Gain(AUDIO_GROUP_SFX, SoundVolume);
	AudioEngine.Set_Group_Gain(AUDIO_GROUP_MOVIE, SoundVolume);
	if (feedback) {
		Sound_Effect(Rule->GenericBeep);
	}
	DebugString("SoundVolume = %f\n", SoundVolume);
}


/// <summary>
/// Sets the speech volume level.
/// This routine will set the volume that the EVA and taunt speech plays back at. It can
/// generate a feedback sound if desired -- a taunt while a game is in progress, and a plain
/// beep otherwise.
/// </summary>
/// <param name="volume">The volume setting to use for the new value. Zero to one.</param>
/// <param name="feedback">Should a feedback sound be generated?</param>
void OptionsClass::Set_Voice_Volume(float volume, bool feedback)
{
	VoiceVolume = std::min(volume, 1.0f);
	Set_Speech_Volume(VoiceVolume * 255.0);
	if (feedback) {
		if (GameActive == true) {
			if (!Is_Speaking()) {
				Speak(Sim_Random_Pick(VOX_GDI_TAUNT_01, VOX_NOD_TAUNT_10), true);
			}
		} else {
			Voice_Sound_Effect(Rule->GenericBeep, VoiceVolume);
		}
	}
	DebugString("VoiceVolume = %f\n", VoiceVolume);
}


/// <summary>
/// Turns the name of a frame filter into the filter itself.
/// </summary>
/// <param name="name">The name as it appears in the settings file.</param>
/// <param name="fallback">What to return when the name is not one of them.</param>
/// <returns>int; One of the VideoScaleMode values.</returns>
static int Scale_Mode_From_Name(char const * name, int fallback)
{
	if (stricmp(name, "Nearest") == 0) return(VIDEO_SCALE_NEAREST);
	if (stricmp(name, "Linear") == 0) return(VIDEO_SCALE_LINEAR);
	if (stricmp(name, "PixelArt") == 0) return(VIDEO_SCALE_PIXELART);

	return(fallback);
}


/// <summary>
/// Names the frame filter setting for writing back.
/// </summary>
/// <param name="mode">One of the VideoScaleMode values.</param>
/// <returns>The name the settings file uses for that filter.</returns>
static char const * Scale_Mode_Name(int mode)
{
	switch (mode) {
		case VIDEO_SCALE_NEAREST:
			return("Nearest");

		case VIDEO_SCALE_LINEAR:
			return("Linear");

		default:
			return("PixelArt");
	}
}


/***********************************************************************************************
 * OptionsClass::Load_Settings -- reads options settings from the INI file                     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *      none.                                                                                  *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *      none.                                                                                  *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *      none.                                                                                  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/14/1995 BR : Created.                                                                  *
 *   07/03/1996 JLB : Reworked to use new INI handler.                                         *
 *   07/30/1996 JLB : Handles hotkeys.                                                         *
 *=============================================================================================*/
void OptionsClass::Load_Settings(void)
{
	DebugString("--------- Loading SUN.INI settings ---------------\n");

	/*
	**	Read in the Options values
	*/
	GameSpeed = ConfigINI.Get_Int("Options", "GameSpeed", GameSpeed);
	DebugString("GameSpeed = %d\n", GameSpeed);

	Difficulty = ConfigINI.Get_Int("Options", "Difficulty", Difficulty);
	Difficulty = std::min(Difficulty, (int)DIFF_COUNT - 1);
	Difficulty = std::max(Difficulty, 0);
	DebugString("Difficulty = %d\n", Difficulty);

	ScrollMethod = ConfigINI.Get_Int("Options", "ScrollMethod", ScrollMethod);
	DebugString("ScrollMethod = %d\n", ScrollMethod);

	ScrollRate = ConfigINI.Get_Int("Options", "ScrollRate", ScrollRate);
	DebugString("ScrollRate = %d\n", ScrollRate);

	AutoScroll = ConfigINI.Get_Bool("Options", "AutoScroll", AutoScroll);
	DebugString("AutoScroll is %s\n", AutoScroll == true ? "ON" : "OFF");

	DetailLevel = ConfigINI.Get_Int("Options", "DetailLevel", DetailLevel);
	DetailLevel = std::min(DetailLevel, 2);
	DetailLevel = std::max(DetailLevel, 0);
	DebugString("DetailLevel = %d\n", DetailLevel);

	IsSidebarOnRight = true;
	DebugString("SideBar on %s\n", IsSidebarOnRight == true ? "RIGHT" : "LEFT");

	SidebarCameoText = ConfigINI.Get_Bool("Options", "SidebarCameoText", SidebarCameoText);
	DebugString("Sidebar Text is %s\n", SidebarCameoText == true ? "ON" : "OFF");

	ActionLines = ConfigINI.Get_Bool("Options", "UnitActionLines", ActionLines);
	DebugString("ActionLines are %s\n", ActionLines == true ? "ON" : "OFF");

	ToolTips = ConfigINI.Get_Bool("Options", "ToolTips", ToolTips);
	DebugString("ToolTips are %s\n", ToolTips == true ? "ON" : "OFF");

	TextBackgroundColor = ConfigINI.Get_Int("Options", "TextBackgroundColor", TextBackgroundColor);
	DebugString("TextBackgroundColor = %d\n", TextBackgroundColor);

	AutoSaveInterval = ConfigINI.Get_Int("Options", "AutoSaveInterval", AutoSaveInterval);
	DebugString("AutoSaveInterval = %d\n", AutoSaveInterval);

	ScreenWidth = ConfigINI.Get_Int("Video", "ScreenWidth", ScreenWidth);
	ScreenHeight = ConfigINI.Get_Int("Video", "ScreenHeight", ScreenHeight);
	DebugString("Resolution = %d X %d\n", ScreenWidth, ScreenHeight);

	StretchMovies = ConfigINI.Get_Bool("Video", "StretchMovies", StretchMovies);
	DebugString("StretchMovies is %s\n", StretchMovies == true ? "ON" : "OFF");

	IntegerScaling = ConfigINI.Get_Bool("Video", "IntegerScaling", IntegerScaling);

	char scalename[32];
	ConfigINI.Get_String("Video", "ScaleMode", (char *)Scale_Mode_Name(ScaleMode), scalename, sizeof(scalename));
	ScaleMode = Scale_Mode_From_Name(scalename, ScaleMode);
	DebugString("ScaleMode is %d, IntegerScaling is %s\n", ScaleMode, IntegerScaling == true ? "ON" : "OFF");

	CursorScale = ConfigINI.Get_Int("Video", "CursorScale", CursorScale);

	Set_Sound_Volume(ConfigINI.Get_Float("Audio", "SoundVolume", SoundVolume), false);
	Set_Voice_Volume(ConfigINI.Get_Float("Audio", "VoiceVolume", VoiceVolume), false);
	Set_Score_Volume(ConfigINI.Get_Float("Audio", "ScoreVolume", ScoreVolume), false);

	Set_Repeat(ConfigINI.Get_Bool("Audio", "IsScoreRepeat", IsScoreRepeat));
	Set_Shuffle(ConfigINI.Get_Bool("Audio", "IsScoreShuffle", IsScoreShuffle));

	SoundLatency = ConfigINI.Get_Int("Audio", "SoundLatency", SoundLatency);
	DebugString("Emulated sound card latency default = %d\n", SoundLatency);

	DebugString("--------- Complete -------------------------------\n");

	Map.Toggle_Cameo_Text(SidebarCameoText);
	TechnoClass::Set_Action_Lines(ActionLines);
}


/***********************************************************************************************
 * OptionsClass::Save_Settings -- writes options settings to the INI file                      *
 *                                                                                             *
 * INPUT:                                                                                      *
 *      none.                                                                                  *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *      none.                                                                                  *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *      none.                                                                                  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/14/1995 BR : Created.                                                                  *
 *   07/03/1996 JLB : Revamped and tightened up.                                               *
 *   07/30/1996 JLB : Handles hotkeys.                                                         *
 *=============================================================================================*/
void OptionsClass::Save_Settings (void)
{
	CCFileClass file(CONFIG_FILE_NAME);

	DebugString("Saving game settings\n");

	/*
	**	Save Options settings
	*/
	ConfigINI.Put_Int("Options", "GameSpeed", GameSpeed);
	ConfigINI.Put_Int("Options", "Difficulty", Difficulty);
	ConfigINI.Put_Int("Options", "ScrollMethod", ScrollMethod);
	ConfigINI.Put_Int("Options", "ScrollRate", ScrollRate);
	ConfigINI.Put_Bool("Options", "AutoScroll", AutoScroll);
	ConfigINI.Put_Int("Options", "DetailLevel", DetailLevel);
	ConfigINI.Put_Bool("Options", "SidebarCameoText", SidebarCameoText);
	ConfigINI.Put_Bool("Options", "UnitActionLines", ActionLines);
	ConfigINI.Put_Bool("Options", "ToolTips", ToolTips);
	ConfigINI.Put_Int("Options", "TextBackgroundColor", TextBackgroundColor);
	ConfigINI.Put_Int("Options", "AutoSaveInterval", AutoSaveInterval);
	ConfigINI.Put_Int("Video", "ScreenWidth", ScreenWidth);
	ConfigINI.Put_Int("Video", "ScreenHeight", ScreenHeight);
	ConfigINI.Put_Bool("Video", "StretchMovies", StretchMovies);
	ConfigINI.Put_Bool("Video", "Fullscreen", Fullscreen);
	ConfigINI.Put_Int("Video", "WindowWidth", WindowWidth);
	ConfigINI.Put_Int("Video", "WindowHeight", WindowHeight);
	ConfigINI.Put_String("Video", "ScaleMode", (char *)Scale_Mode_Name(ScaleMode));
	ConfigINI.Put_Bool("Video", "IntegerScaling", IntegerScaling);
	ConfigINI.Put_Bool("Video", "VSync", VSync);
	ConfigINI.Put_Int("Video", "Renderer", Renderer);
	ConfigINI.Put_Int("Video", "CursorScale", CursorScale);
	ConfigINI.Put_Float("Audio", "SoundVolume", SoundVolume);
	ConfigINI.Put_Float("Audio", "VoiceVolume", VoiceVolume);
	ConfigINI.Put_Float("Audio", "ScoreVolume", ScoreVolume);
	ConfigINI.Put_Bool("Audio", "IsScoreRepeat", IsScoreRepeat);
	ConfigINI.Put_Bool("Audio", "IsScoreShuffle", IsScoreShuffle);
	ConfigINI.Put_Int("Audio", "SoundLatency", SoundLatency);

	/*
	**	Write the INI data out to a file.
	*/
	ConfigINI.Save(file, false);
}


/***********************************************************************************************
 * OptionsClass::Set -- Sets options based on current settings                                 *
 *                                                                                             *
 * Use this routine to adjust the palette or sound settings after a fresh scenario load.       *
 * It assumes the values needed are already loaded into OptionsClass.                          *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/24/1995 BRR : Created.                                                                 *
 *=============================================================================================*/
void OptionsClass::Set(void)
{
	Set_Sound_Volume(SoundVolume, false);
	Set_Voice_Volume(VoiceVolume, false);
	Set_Score_Volume(ScoreVolume, false);
	Set_Repeat(IsScoreRepeat);
	Set_Shuffle(IsScoreShuffle);
	Map.Toggle_Cameo_Text(SidebarCameoText);
	TechnoClass::Set_Action_Lines(ActionLines);
}


/***********************************************************************************************
 * OptionsClass::Normalize_Delay -- Normalizes delay factor to keep rate constant.             *
 *                                                                                             *
 *    This routine is used to adjust delay factors that MUST be synchronized on all machines   *
 *    but should maintain a speed as close to constant as possible. Building animations are    *
 *    a good example of this.                                                                  *
 *                                                                                             *
 * INPUT:   delay -- The normal delay factor.                                                  *
 *                                                                                             *
 * OUTPUT:  Returns with the delay to use that has been modified so that a reasonably constant *
 *          rate will result.                                                                  *
 *                                                                                             *
 * WARNINGS:   This calculation is crude due to the coarse resolution that a 1/15 second timer *
 *             allows.                                                                         *
 *                                                                                             *
 *             Use of this routine ASSUMES that the GameSpeed is synchronized on all machines. *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/18/1995 JLB : Created.                                                                 *
 *   06/30/1995 JLB : Handles low values in a more consistent manner.                          *
 *=============================================================================================*/
int OptionsClass::Normalize_Delay(int delay) const
{
	static int _adjust[][MAX_SPEED_SETTING + 1] = {
		{2,2,1,1,1,1,1,1},
		{3,3,3,2,2,2,1,1},
		{5,4,4,3,3,2,2,1},
		{7,6,5,4,4,4,3,2}
	};
	if (delay) {
		if (delay < 5) {
			delay = _adjust[delay-1][GameSpeed];
		} else {
			delay = ((delay * (MAX_SPEED_SETTING + 1)) / (GameSpeed+1));
		}
	}
	return(delay);
}


/***********************************************************************************************
 * OptionsClass::Normalize_Volume -- Convert to a real volume value.                           *
 *                                                                                             *
 *    This routine will take a relative volume value and convert it to the real volume value   *
 *    to use. This allows all the game volumes to be corrected to the correct global volume.   *
 *                                                                                             *
 * INPUT:   volume   -- Requested volume level.                                                *
 *                                                                                             *
 * OUTPUT:  Returns with the actual volume level to use.                                       *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/22/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int OptionsClass::Normalize_Volume(int volume) const
{
	return(volume * SoundVolume);
}


/*
 * Internal state-machine messages the hotkey-configuration dialog posts to
 * itself. wParam/lParam are unused; each just triggers the matching refresh.
 */
#define HKD_FILL_COMMANDS	(WM_USER + 100)		/// rebuild the command listbox for the selected category
#define HKD_SHOW_COMMAND	(WM_USER + 101)		/// refresh the description / assigned-key panel for the selected command
#define HKD_APPLY_HOTKEY	(WM_USER + 102)		/// assign the hotkey edit's key to the selected command
#define HKD_REINIT			(WM_USER + 103)		/// full refresh: repopulate the category combo and reset


/// <summary>
/// Handles the messages for the keyboard configuration dialog.
/// This routine drives the category, command and hotkey controls, and hands the reassigned
/// keys back to the hotkey command list. Accepting the dialog writes the assignments out to
/// KEYBOARD.INI; canceling puts the previous assignments back.
/// </summary>
/// <returns>Returns with TRUE if the message was consumed by this dialog.</returns>
BOOL CALLBACK Hotkey_Dialog_Proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	char buffer[64];
	int * retval;
	static int current_selection = -1;

	int result = OwnerDraw::Default_Dialog_Proc(window, message, wparam, lparam);
	if (result) {
		return(result);
	}

	retval = (int *)GetWindowLong(window, DWL_USER);

	switch (message) {
		case WM_COMMAND:
			switch (LOWORD(wparam)) {
				case IDOK:
					if (HIWORD(wparam) == BN_CLICKED) {
						CCINIClass ini;
						ini.Clear();

						for (int i = 0; i < HotkeyCommands.Count(); i++) {
							CommandClass const * cmd = HotkeyCommands.Fetch_By_Position(i);
							int key = HotkeyCommands.Fetch_ID_By_Position(i);
							ini.Put_Int("Hotkey", cmd->Get_Unique_Name(), key);
						}

						CDFileClass file("Keyboard.ini");
						ini.Save(file, false);
						*retval = IDOK;
						return(TRUE);
					}
					break;

				case IDCANCEL:
					if (HIWORD(wparam) == BN_CLICKED) {
						Init_Hotkeys();
						*retval = 2;
						return(TRUE);
					}
					break;

				case IDC_KEY_COMMANDS:
					if (HIWORD(wparam) == LBN_SELCHANGE) {
						SendMessage(window, HKD_SHOW_COMMAND, 0, 0);
						HWND hotkey = GetDlgItem(window, IDC_KEY_HOTKEY);
						if (hotkey != NULL) {
							SetFocus(hotkey);
							return(TRUE);
						}
					}
					break;

				case IDC_KEY_ASSIGN:
					SendMessage(window, HKD_APPLY_HOTKEY, 0, 0);
					SendMessage(window, HKD_SHOW_COMMAND, 0, 0);
					return(TRUE);

				case IDC_KEY_HOTKEY:
					if (HIWORD(wparam) == EN_CHANGE) {
						int key = SendMessage((HWND)lparam, HKM_GETHOTKEY, 0, 0);
						char const * key_name;
						if (HotkeyCommands.Is_Present(key)) {
							key_name = HotkeyCommands[key]->Get_Display_Name();
							if (key_name == NULL) {
								key_name = "";
							}
						} else {
							key_name = "";
						}
						HWND hotkey_name = GetDlgItem(window, IDC_KEY_ASSIGNED_TO);
						SetWindowText(hotkey_name, key_name);
						return(TRUE);
					}
					break;

				case IDC_KEY_RESET_ALL:
					if (HIWORD(wparam) == BN_CLICKED) {
						if (WWMessageBox()._Process(TXT_RESET_HOTKEYS, IDOK, TXT_YES, TXT_NO, TXT_NONE, false) == 0) {
							DebugString("Deleting users KEYBOARD.INI\n");
							// Only the player's own file is discarded; the defaults a
							// deployment ships are what the reset falls back on.
							CCFileClass file("KEYBOARD.INI");
							file.Delete();
							Init_Hotkeys();
							SendMessage(window, HKD_REINIT, 0, 0);
							return(TRUE);
						}
					}
					break;

				case IDC_KEY_CATEGORY:
					if (HIWORD(wparam) == CBN_SELCHANGE) {
						SendMessage(window, HKD_FILL_COMMANDS, 0, 0);
						return(TRUE);
					}
					break;
			}
			return(TRUE);

		case HKD_APPLY_HOTKEY: {
			HWND list_commands = GetDlgItem(window, IDC_KEY_COMMANDS);
			int selection = ListBox_GetCurSel(list_commands);
			if (selection != LB_ERR) {
				CommandClass const * cmd = (CommandClass const *)ListBox_GetItemData(list_commands, selection);
				for (int i = 0; i < HotkeyCommands.Count(); i++) {
					if (HotkeyCommands.Fetch_By_Position(i) == cmd) {
						HotkeyCommands.Remove_Index(HotkeyCommands.Fetch_ID_By_Position(i));
						break;
					}
				}
				HWND hotkey = GetDlgItem(window, IDC_KEY_HOTKEY);
				int key = SendMessage(hotkey, HKM_GETHOTKEY, 0, 0);
				if (key != 0) {
					HotkeyCommands.Remove_Index(key);
					HotkeyCommands.Add_Index(key, cmd);
					return(TRUE);
				}
			}
			return(TRUE);
		}

		case HKD_SHOW_COMMAND: {
			HWND list_commands = GetDlgItem(window, IDC_KEY_COMMANDS);
			int selection = ListBox_GetCurSel(list_commands);
			if (selection != LB_ERR) {
				CommandClass const * cmd = (CommandClass const *)ListBox_GetItemData(list_commands, selection);
				HWND description = GetDlgItem(window, IDC_KEY_DESCRIPTION);
				SetWindowText(description, cmd->Get_Description());

				int key = 0;
				for (int i = 0; i < HotkeyCommands.Count(); i++) {
					if (HotkeyCommands.Fetch_By_Position(i) == cmd) {
						key = HotkeyCommands.Fetch_ID_By_Position(i);
						break;
					}
				}

				HWND key_label = GetDlgItem(window, IDC_KEY_CURRENT_SHORTCUT);
				Build_Hotkey_String((KeyNumType)key, buffer);
				SetWindowText(key_label, buffer);

				HWND hotkey = GetDlgItem(window, IDC_KEY_HOTKEY);
				SendMessage(hotkey, HKM_SETHOTKEY, 0, 0);

				HWND hotkey_name = GetDlgItem(window, IDC_KEY_ASSIGNED_TO);
				SetWindowText(hotkey_name, "");
				return(TRUE);
			}
			return(TRUE);
		}

		case HKD_FILL_COMMANDS: {
			HWND cmb_category = GetDlgItem(window, IDC_KEY_CATEGORY);
			if (ComboBox_GetCurSel(cmb_category) != current_selection) {
				current_selection = ComboBox_GetCurSel(cmb_category);
				GetWindowText(cmb_category, buffer, sizeof(buffer));
				HWND list_commands = GetDlgItem(window, IDC_KEY_COMMANDS);
				ListBox_ResetContent(list_commands);
				for (int i = 0; i < AllCommands.Count(); i++) {
					CommandClass const * cmd = AllCommands[i];
					if (stricmp(cmd->Get_Category(), buffer) == 0) {
						int index = ListBox_AddString(list_commands, cmd->Get_Display_Name());
						if (index != LB_ERR) {
							ListBox_SetItemData(list_commands, index, (LPARAM)cmd);
						}
					}
				}
				HWND description = GetDlgItem(window, IDC_KEY_DESCRIPTION);
				SetWindowText(description, "");
				ListBox_SetCurSel(description, 0);
				SendMessage(window, HKD_SHOW_COMMAND, 0, 0);
				return(TRUE);
			}
			return(TRUE);
		}

		case HKD_REINIT: {
			HWND cmb_category = GetDlgItem(window, IDC_KEY_CATEGORY);
			ComboBox_ResetContent(cmb_category);
			for (int i = 0; i < AllCommands.Count(); i++) {
				CommandClass const * cmd = AllCommands[i];
				const char * s = cmd->Get_Category();
				if (ComboBox_FindString(cmb_category, 0, s) == CB_ERR) {
					s = cmd->Get_Category();
					ComboBox_AddString(cmb_category, s);
				}
			}
			ComboBox_SetCurSel(cmb_category, 0);
			SendMessage(window, HKD_FILL_COMMANDS, 0, 0);
			current_selection = -1;
			return(TRUE);
		}

		case WM_INITDIALOG:
			SendMessage(window, HKD_REINIT, 0, 0);
			return(FALSE);
	}

	return(FALSE);
}


/// <summary>
/// Displays the keyboard configuration dialog.
/// This routine brings up the hotkey assignment dialog and does not return until the player
/// dismisses it. The title screen is kept refreshed while the dialog is up outside of a
/// game.
/// </summary>
bool OptionsClass::Hotkey_Dialog(void)
{
	HWND handle;
	int res = -1;

	handle = OwnerDraw::Begin_Dialog(IDD_OPT_KEYBOARD, Hotkey_Dialog_Proc);

	if (handle != NULL) {
		SetWindowLong(handle, DWL_USER, (LONG)&res);
		OwnerDraw::Display_Dialog(handle);

		while (res < 0) {
			if (OwnerDraw::Dialog_Message_Handler() == true) {
				res = 2;
			}
			if (!GameActive) {
				Title_Screen_Restore();
			}
		}
		OwnerDraw::End_Dialog(handle);
	}

	return(true);
}


/// <summary>
/// Fetches the tactical map scrolling method the player prefers.
/// </summary>
int OptionsClass::Get_Scroll_Method(void) const
{
	return(ScrollMethod);
}
