/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include "win.h"

struct IStorage;
struct IPropertySetStorage;

enum {
	PIDSI_SCEN_DESCRIP = 2,
	PIDSI_PLAYER_HOUSE = 3,
	PIDSI_PLAYER_NAME1 = 4,
	PIDSI_PLAYER_NAME2 = 8,
	PIDSI_G_VERSION = 9,
	PIDSI_G_PLAY_TIME = 10,
	PIDSI_G_START_TIME = 12,
	PIDSI_LAST_SAVE_TIME = 13,
	PIDSI_INTERNAL_VER = 16,
	PIDSI_EXEC_NAME = 18,

	PIDSI_SCENARIO_NUM = 100,
	PIDSI_CAMPAIGN_NUM,
	PIDSI_GAME_TYPE,
};

class SaveVersionInfo
{
	public:
		SaveVersionInfo(void);

		void Set_Version(int num);
		int Get_Version(void);

		void Set_Internal_Version(int num);
		int Get_Internal_Version(void);

		void Set_Scenario_Description(const char * desc);
		const char * Get_Scenario_Description(void);

		void Set_Player_House(const char * name);
		const char * Get_Player_House(void);

		void Set_Campaign_Number(int num);
		int Get_Campaign_Number(void);

		void Set_Scenario_Number(int num);
		int Get_Scenario_Number(void);

		void Set_Unknown_String(const char * name);
		const char * Get_Unknown_String(void);

		void Set_Player_Name(const char * name);
		const char * Get_Player_Name(void);

		void Set_Executable_Name(const char * name);
		const char * Get_Executable_Name(void);

		void Set_Start_Time(FILETIME &time);
		FILETIME Get_Start_Time(void);

		void Set_Play_Time(FILETIME &time);
		FILETIME Get_Play_Time(void);

		void Set_Last_Time(FILETIME &time);
		FILETIME Get_Last_Time(void);

		void Set_Game_Type(int id);
		int Get_Game_Type(void);

		HRESULT Save(IStorage *storage);
		HRESULT Load(IStorage *storage);

	private:
		HRESULT Load_String(IStorage *storage, int id, char *string, int size);
		HRESULT Load_String_Set(IPropertySetStorage *storageset, int id, char *string, int size);

		HRESULT Load_Int(IStorage *storage, int id, int *integer);
		HRESULT Load_Int_Set(IPropertySetStorage *storageset, int id, int *integer);

		HRESULT Save_String(IStorage *storage, int id, char *string);
		HRESULT Save_String_Set(IPropertySetStorage *storageset, int id, const char *string);

		HRESULT Save_Int(IStorage *storage, int id, int integer);
		HRESULT Save_Int_Set(IPropertySetStorage *storageset, int id, int integer);

		HRESULT Load_Time(IStorage *storage, int id, FILETIME *time);
		HRESULT Load_Time_Set(IPropertySetStorage *storageset, int id, FILETIME *time);

		HRESULT Save_Time(IStorage *storage, int id, FILETIME *time);
		HRESULT Save_Time_Set(IPropertySetStorage *storageset, int id, FILETIME *time);

	private:
		/*
		 * This is the version of the game that wrote the save. A file whose version the
		 * running build does not recognize is never offered to the player.
		 */
		int InternalVersion;

		/*
		 * This is the revision of this information block itself, as opposed to the version
		 * of the game that wrote it, which is kept in InternalVersion.
		 */
		int Version;

		/*
		 * This is the description the player gave the save, as shown in the load dialog.
		 */
		char ScenarioDescription[128];

		/*
		 * This is the given name of the house the player was commanding, recorded so the
		 * side a save belongs to can be told without reading the game itself back in.
		 */
		char PlayerHouse[64];

		/*
		 * These pin down the mission the save was made in, and a campaign number of -1
		 * means the game was not part of a campaign at all.
		 */
		int CampaignNumber;
		int ScenarioNumber;

		/*
		 * This is a spare string carried with the save information, reachable only through
		 * its own accessors. Neither the save nor the load routine records it.
		 */
		char UnknownString[260];

		/*
		 * This is the name of the player who made the save, which is recorded separately
		 * from the house so that the person and the side are both known.
		 */
		char PlayerName[64];

		/*
		 * This is the name of the program that wrote the save, so a file can be traced back
		 * to what produced it rather than merely to a version number.
		 */
		char ExecutableName[260];

		/*
		 * These are the times recorded with the save -- when the game was begun, how long
		 * it has been played, and when it was last written out.
		 */
		FILETIME StartTime;
		FILETIME PlayTime;
		FILETIME LastSaveTime;

		/*
		 * This is the kind of session the save was made in, and the one it is restored into.
		 */
		int GameType;
};

const WCHAR *Stream_Name_From_ID(int id);
