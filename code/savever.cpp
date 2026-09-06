/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "always.h"

#include "savever.h"

#include "savefile.h"

#include "dbgprint.h"
#include "session.h"


/// <summary>
/// Creates an empty save file information block.
/// Every value starts blank so that a block which is only partly filled in -- either by the
/// game before a save or by the loader reading an older save -- still reads sensibly.
/// </summary>
SaveVersionInfo::SaveVersionInfo(void) :
	InternalVersion(0),
	Version(0),
	CampaignNumber(-1),
	ScenarioNumber(0),
	GameType(GAME_NORMAL)
{
	ScenarioDescription[0] = '\0';
	PlayerHouse[0] = '\0';
	UnknownString[0] = '\0';
	PlayerName[0] = '\0';
	ExecutableName[0] = '\0';

	StartTime.dwLowDateTime = 0;
	StartTime.dwHighDateTime = 0;

	PlayTime.dwLowDateTime = 0;
	PlayTime.dwHighDateTime = 0;

	LastSaveTime.dwLowDateTime = 0;
	LastSaveTime.dwHighDateTime = 0;

}


/// <summary>
/// Records the version stamp of the save file.
/// This is the save file's own version, kept alongside the internal build version that the
/// load dialog tests compatibility against.
/// </summary>
/// <param name="num">The version number to stamp the save with.</param>
void SaveVersionInfo::Set_Version(int num)
{
	Version = num;
}


/// <summary>
/// Fetches the version stamp of the save file.
/// </summary>
/// <returns>Returns with the version stamp recorded in the save.</returns>
int SaveVersionInfo::Get_Version(void)
{
	return(Version);
}


/// <summary>
/// Records the build version of the game writing the save.
/// The load dialog tests this value against the version it expects and hides any save it
/// does not recognize, so this is what decides whether a save can be offered at all.
/// </summary>
/// <param name="num">The internal version number to stamp the save with.</param>
void SaveVersionInfo::Set_Internal_Version(int num)
{
	InternalVersion = num;
}


/// <summary>
/// Fetches the build version of the game that wrote the save.
/// </summary>
/// <returns>Returns with the internal version recorded in the save.</returns>
int SaveVersionInfo::Get_Internal_Version(void)
{
	return(InternalVersion);
}


/// <summary>
/// Records the description to show for this save.
/// This is the text the load game dialog lists the save under. It is truncated if it will
/// not fit the buffer it is kept in.
/// </summary>
void SaveVersionInfo::Set_Scenario_Description(const char * desc)
{
	ScenarioDescription[ARRAY_SIZE(ScenarioDescription) - 1] = 0;
	strncpy(ScenarioDescription, desc, ARRAY_SIZE(ScenarioDescription) - 1);
}


/// <summary>
/// Fetches the description shown for this save.
/// </summary>
/// <returns>Returns with the description recorded in the save.</returns>
const char * SaveVersionInfo::Get_Scenario_Description(void)
{
	return(ScenarioDescription);
}


/// <summary>
/// Records the house the player was commanding.
/// The name is truncated if it will not fit the buffer it is kept in.
/// </summary>
void SaveVersionInfo::Set_Player_House(const char * name)
{
	PlayerHouse[ARRAY_SIZE(PlayerHouse) - 1] = 0;
	strncpy(PlayerHouse, name, ARRAY_SIZE(PlayerHouse) - 1);
}


/// <summary>
/// Fetches the house the player was commanding.
/// </summary>
/// <returns>Returns with the house name recorded in the save.</returns>
const char * SaveVersionInfo::Get_Player_House(void)
{
	return(PlayerHouse);
}


/// <summary>
/// Records the campaign this save was made in.
/// </summary>
/// <param name="num">The campaign number, or -1 when the game is not part of a campaign.</param>
void SaveVersionInfo::Set_Campaign_Number(int num)
{
	CampaignNumber = num;
}


/// <summary>
/// Fetches the campaign this save was made in.
/// </summary>
/// <returns>Returns with the campaign number recorded in the save, or -1 if the game was
/// not part of a campaign.</returns>
int SaveVersionInfo::Get_Campaign_Number(void)
{
	return(CampaignNumber);
}


/// <summary>
/// Records the scenario this save was made in.
/// </summary>
/// <param name="num">The scenario number within the campaign.</param>
void SaveVersionInfo::Set_Scenario_Number(int num)
{
	ScenarioNumber = num;
}


/// <summary>
/// Fetches the scenario this save was made in.
/// </summary>
/// <returns>Returns with the scenario number recorded in the save.</returns>
int SaveVersionInfo::Get_Scenario_Number(void)
{
	return(ScenarioNumber);
}


/// <summary>
/// Records the spare string kept with the save information.
/// The string is truncated if it will not fit the buffer it is kept in.
/// </summary>
void SaveVersionInfo::Set_Unknown_String(const char * str)
{
	UnknownString[sizeof(UnknownString) - 1] = 0;
	strncpy(UnknownString, str, sizeof(UnknownString) - 1);
}


/// <summary>
/// Fetches the spare string kept with the save information.
/// Neither the save nor the load routine records this string, so it only ever holds what
/// the current session put there.
/// </summary>
/// <returns>Returns with the string most recently set.</returns>
const char * SaveVersionInfo::Get_Unknown_String(void)
{
	return(UnknownString);
}


/// <summary>
/// Records the name of the player making the save.
/// The name is truncated if it will not fit the buffer it is kept in.
/// </summary>
void SaveVersionInfo::Set_Player_Name(const char * name)
{
	PlayerName[sizeof(PlayerName) - 1] = 0;
	strncpy(PlayerName, name, sizeof(PlayerName) - 1);
}


/// <summary>
/// Fetches the name of the player who made the save.
/// </summary>
/// <returns>Returns with the player name recorded in the save.</returns>
const char * SaveVersionInfo::Get_Player_Name(void)
{
	return(PlayerName);
}


/// <summary>
/// Records the name of the program writing the save.
/// The name is truncated if it will not fit the buffer it is kept in.
/// </summary>
void SaveVersionInfo::Set_Executable_Name(const char * name)
{
	ExecutableName[sizeof(ExecutableName) - 1] = 0;
	strncpy(ExecutableName, name, sizeof(ExecutableName) - 1);
}


/// <summary>
/// Fetches the name of the program that wrote the save.
/// </summary>
/// <returns>Returns with the executable name recorded in the save.</returns>
const char * SaveVersionInfo::Get_Executable_Name(void)
{
	return(ExecutableName);
}


/// <summary>
/// Records the time this game was begun.
/// </summary>
void SaveVersionInfo::Set_Start_Time(FILETIME &time)
{
	StartTime = time;
}


/// <summary>
/// Fetches the time this game was begun.
/// </summary>
/// <returns>Returns with the time stamp taken when the game was started.</returns>
FILETIME SaveVersionInfo::Get_Start_Time(void)
{
	return(StartTime);
}


/// <summary>
/// Records how long this game has been played.
/// </summary>
void SaveVersionInfo::Set_Play_Time(FILETIME &time)
{
	PlayTime = time;
}


/// <summary>
/// Fetches how long this game has been played.
/// </summary>
/// <returns>Returns with the accumulated play time recorded in the save.</returns>
FILETIME SaveVersionInfo::Get_Play_Time(void)
{
	return(PlayTime);
}


/// <summary>
/// Records the time this game was last saved.
/// </summary>
void SaveVersionInfo::Set_Last_Time(FILETIME &time)
{
	LastSaveTime = time;
}


/// <summary>
/// Fetches the time this game was last saved.
/// </summary>
/// <returns>Returns with the time stamp of the most recent save.</returns>
FILETIME SaveVersionInfo::Get_Last_Time(void)
{
	return(LastSaveTime);
}


/// <summary>
/// Records the kind of game being saved.
/// </summary>
/// <param name="type">The session type the game is being played as.</param>
void SaveVersionInfo::Set_Game_Type(int type)
{
	GameType = type;
}


/// <summary>
/// Fetches the kind of game this save was made in.
/// </summary>
/// <returns>Returns with the session type recorded when the game was saved.</returns>
int SaveVersionInfo::Get_Game_Type(void)
{
	return(GameType);
}


/// <summary>
/// Writes every listing field into the file's field table.
/// </summary>
void SaveVersionInfo::Save(SaveFileClass & file) const
{
	file.Set_String(PIDSI_SCEN_DESCRIP, ScenarioDescription);
	file.Set_String(PIDSI_PLAYER_HOUSE, PlayerHouse);
	file.Set_Int(PIDSI_G_VERSION, Version);
	file.Set_Int(PIDSI_INTERNAL_VER, InternalVersion);
	file.Set_Time(PIDSI_G_START_TIME, StartTime);
	file.Set_Time(PIDSI_LAST_SAVE_TIME, LastSaveTime);
	file.Set_Time(PIDSI_G_PLAY_TIME, PlayTime);
	file.Set_String(PIDSI_EXEC_NAME, ExecutableName);
	file.Set_String(PIDSI_PLAYER_NAME1, PlayerName);
	file.Set_String(PIDSI_PLAYER_NAME2, PlayerName);
	file.Set_Int(PIDSI_SCENARIO_NUM, ScenarioNumber);
	file.Set_Int(PIDSI_CAMPAIGN_NUM, CampaignNumber);
	file.Set_Int(PIDSI_GAME_TYPE, GameType);
}


/// <summary>
/// Reads the listing fields the file carries; a field the file lacks keeps its default.
/// </summary>
/// <returns>bool; Does the file record an internal version at all?</returns>
bool SaveVersionInfo::Load(SaveFileClass const & file)
{
	file.Get_String(PIDSI_SCEN_DESCRIP, ScenarioDescription, sizeof(ScenarioDescription));
	file.Get_String(PIDSI_PLAYER_HOUSE, PlayerHouse, sizeof(PlayerHouse));
	file.Get_Int(PIDSI_G_VERSION, &Version);
	file.Get_Time(PIDSI_G_START_TIME, &StartTime);
	file.Get_Time(PIDSI_LAST_SAVE_TIME, &LastSaveTime);
	file.Get_Time(PIDSI_G_PLAY_TIME, &PlayTime);
	file.Get_String(PIDSI_EXEC_NAME, ExecutableName, sizeof(ExecutableName));
	file.Get_String(PIDSI_PLAYER_NAME1, PlayerName, sizeof(PlayerName));
	file.Get_Int(PIDSI_SCENARIO_NUM, &ScenarioNumber);
	file.Get_Int(PIDSI_CAMPAIGN_NUM, &CampaignNumber);
	file.Get_Int(PIDSI_GAME_TYPE, &GameType);

	return(file.Get_Int(PIDSI_INTERNAL_VER, &InternalVersion));
}
