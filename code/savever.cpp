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

#include "dbgprint.h"
#include "session.h"

#include <comdef.h>


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
/// Saves the version information into a save file.
/// This routine is called while a save game is being written. It records every value into
/// the summary information property set and then again as one stream per value, so that a
/// reader which knows only the older layout can still identify the save.
/// </summary>
/// <returns>Returns with S_OK once every value has been written, otherwise the failure code
/// from the storage layer.</returns>
HRESULT SaveVersionInfo::Save(IStorage *storage)
{
	if (storage == NULL) {
		return(E_POINTER);
	}

	DebugString("Attempting to obtain PropertySetStorage interface\n");

	IPropertySetStoragePtr storageset;
	HRESULT res;

	res = storage->QueryInterface(IID_IPropertySetStorage, (void **)&storageset);
	if (SUCCEEDED(res)) {

		DebugString("Saving version information the NEW way.\n");

		res = Save_String_Set(storageset, PIDSI_SCEN_DESCRIP, ScenarioDescription);
		if (FAILED(res)) {
			return(res);
		}

		res = Save_String_Set(storageset, PIDSI_PLAYER_HOUSE, PlayerHouse);
		if (FAILED(res)) {
			return(res);
		}

		res = Save_Int_Set(storageset, PIDSI_G_VERSION, Version);
		if (FAILED(res)) {
			return(res);
		}

		res = Save_Int_Set(storageset, PIDSI_INTERNAL_VER, InternalVersion);
		if (FAILED(res)) {
			return(res);
		}

		res = Save_Time_Set(storageset, PIDSI_G_START_TIME, &StartTime);
		if (FAILED(res)) {
			return(res);
		}

		res = Save_Time_Set(storageset, PIDSI_LAST_SAVE_TIME, &LastSaveTime);
		if (FAILED(res)) {
			return(res);
		}

		res = Save_Time_Set(storageset, PIDSI_G_PLAY_TIME, &PlayTime);
		if (FAILED(res)) {
			return(res);
		}

		res = Save_String_Set(storageset, PIDSI_EXEC_NAME, ExecutableName);
		if (FAILED(res)) {
			return(res);
		}

		res = Save_String_Set(storageset, PIDSI_PLAYER_NAME1, PlayerName);
		if (FAILED(res)) {
			return(res);
		}

		res = Save_String_Set(storageset, PIDSI_PLAYER_NAME2, PlayerName);
		if (FAILED(res)) {
			return(res);
		}

		res = Save_Int_Set(storageset, PIDSI_SCENARIO_NUM, ScenarioNumber);
		if (FAILED(res)) {
			return(res);
		}

		res = Save_Int_Set(storageset, PIDSI_CAMPAIGN_NUM, CampaignNumber);
		if (FAILED(res)) {
			return(res);
		}

		res = Save_Int_Set(storageset, PIDSI_GAME_TYPE, GameType);
		if (FAILED(res)) {
			return(res);
		}

	} else {
		DebugString("\t***** FAILED!\n");
	}

	DebugString("Saving version information the old way.\n");

	res = Save_String(storage, PIDSI_SCEN_DESCRIP, ScenarioDescription);
	if (FAILED(res)) {
		return(res);
	}

	res = Save_String(storage, PIDSI_PLAYER_HOUSE, PlayerHouse);
	if (FAILED(res)) {
		return(res);
	}

	res = Save_Int(storage, PIDSI_G_VERSION, Version);
	if (FAILED(res)) {
		return(res);
	}

	res = Save_Int(storage, PIDSI_INTERNAL_VER, InternalVersion);
	if (FAILED(res)) {
		return(res);
	}

	res = Save_Time(storage, PIDSI_G_START_TIME, &StartTime);
	if (FAILED(res)) {
		return(res);
	}

	res = Save_Time(storage, PIDSI_LAST_SAVE_TIME, &LastSaveTime);
	if (FAILED(res)) {
		return(res);
	}

	res = Save_Time(storage, PIDSI_G_PLAY_TIME, &PlayTime);
	if (FAILED(res)) {
		return(res);
	}

	res = Save_String(storage, PIDSI_EXEC_NAME, ExecutableName);
	if (FAILED(res)) {
		return(res);
	}

	res = Save_String(storage, PIDSI_PLAYER_NAME1, PlayerName);
	if (FAILED(res)) {
		return(res);
	}

	res = Save_String(storage, PIDSI_PLAYER_NAME2, PlayerName);
	if (FAILED(res)) {
		return(res);
	}

	res = Save_Int(storage, PIDSI_SCENARIO_NUM, ScenarioNumber);
	if (FAILED(res)) {
		return(res);
	}

	res = Save_Int(storage, PIDSI_CAMPAIGN_NUM, CampaignNumber);
	if (FAILED(res)) {
		return(res);
	}

	res = Save_Int(storage, PIDSI_GAME_TYPE, GameType);
	if (FAILED(res)) {
		return(res);
	}

	return(S_OK);
}


/// <summary>
/// Loads the version information out of a save file.
/// This routine is called when a save game is scanned or restored. It prefers the property
/// set that the current game writes and falls back to the one stream per value layout that
/// older save files use, so that both generations of save file stay readable.
/// </summary>
/// <returns>Returns with S_OK once every value has been recovered, otherwise the failure
/// code from the storage layer.</returns>
HRESULT SaveVersionInfo::Load(IStorage *storage)
{
	char buf[256];

	if (storage == NULL) {
		return(E_POINTER);
	}

	IPropertySetStoragePtr storageset;
	HRESULT res;

	if (SUCCEEDED(storage->QueryInterface(IID_IPropertySetStorage, (void **)&storageset))
			&& SUCCEEDED(Load_String_Set(storageset, PIDSI_SCEN_DESCRIP, buf))) {

		strcpy(ScenarioDescription, buf);

		res = Load_String_Set(storageset, PIDSI_PLAYER_HOUSE, buf);
		if (FAILED(res)) {
			return(res);
		}
		strcpy(PlayerHouse, buf);

		res = Load_Int_Set(storageset, PIDSI_G_VERSION, &Version);
		if (FAILED(res)) {
			return(res);
		}

		res = Load_Int_Set(storageset, PIDSI_INTERNAL_VER, &InternalVersion);
		if (FAILED(res)) {
			return(res);
		}

		res = Load_Time_Set(storageset, PIDSI_G_START_TIME, &StartTime);
		if (FAILED(res)) {
			return(res);
		}

		res = Load_Time_Set(storageset, PIDSI_LAST_SAVE_TIME, &LastSaveTime);
		if (FAILED(res)) {
			return(res);
		}

		res = Load_Time_Set(storageset, PIDSI_G_PLAY_TIME, &PlayTime);
		if (FAILED(res)) {
			return(res);
		}

		res = Load_String_Set(storageset, PIDSI_EXEC_NAME, buf);
		if (FAILED(res)) {
			return(res);
		}
		strcpy(ExecutableName, buf);

		res = Load_String_Set(storageset, PIDSI_PLAYER_NAME1, buf);
		if (FAILED(res)) {
			return(res);
		}
		strcpy(PlayerName, buf);

		res = Load_Int_Set(storageset, PIDSI_SCENARIO_NUM, &ScenarioNumber);
		if (FAILED(res)) {
			return(res);
		}

		res = Load_Int_Set(storageset, PIDSI_CAMPAIGN_NUM, &CampaignNumber);
		if (FAILED(res)) {
			return(res);
		}

		res = Load_Int_Set(storageset, PIDSI_GAME_TYPE, &GameType);
		if (FAILED(res)) {
			return(res);
		}

	} else {

		res = Load_String(storage, PIDSI_SCEN_DESCRIP, buf);
		if (FAILED(res)) {
			return(res);
		}

		strcpy(ScenarioDescription, buf);

		res = Load_String(storage, PIDSI_PLAYER_HOUSE, buf);
		if (FAILED(res)) {
			return(res);
		}

		strcpy(PlayerHouse, buf);

		res = Load_Int(storage, PIDSI_G_VERSION, &Version);
		if (FAILED(res)) {
			return(res);
		}

		res = Load_Int(storage, PIDSI_INTERNAL_VER, &InternalVersion);
		if (FAILED(res)) {
			return(res);
		}

		res = Load_Time(storage, PIDSI_G_START_TIME, &StartTime);
		if (FAILED(res)) {
			return(res);
		}

		res = Load_Time(storage, PIDSI_LAST_SAVE_TIME, &LastSaveTime);
		if (FAILED(res)) {
			return(res);
		}

		res = Load_Time(storage, PIDSI_G_PLAY_TIME, &PlayTime);
		if (FAILED(res)) {
			return(res);
		}

		res = Load_String(storage, PIDSI_EXEC_NAME, buf);
		if (FAILED(res)) {
			return(res);
		}
		strcpy(ExecutableName, buf);

		res = Load_String(storage, PIDSI_PLAYER_NAME1, buf);
		if (FAILED(res)) {
			return(res);
		}
		strcpy(PlayerName, buf);

		res = Load_Int(storage, PIDSI_SCENARIO_NUM, &ScenarioNumber);
		if (FAILED(res)) {
			return(res);
		}

		res = Load_Int(storage, PIDSI_CAMPAIGN_NUM, &CampaignNumber);
		if (FAILED(res)) {
			return(res);
		}

		res = Load_Int(storage, PIDSI_GAME_TYPE, &GameType);
		if (FAILED(res)) {
			return(res);
		}
	}

	return(S_OK);
}


/// <summary>
/// Reads a string from a stream of its own.
/// The wide text held in the stream is narrowed into the caller's buffer, which is emptied
/// first. This is the old style counterpart of Load_String_Set, used for save files written
/// before the version information moved into a property set.
/// </summary>
/// <param name="id">The property identifier naming the stream to open.</param>
/// <returns>Returns with the result of the read. A failure means the stream is absent or
/// ended before the text was terminated.</returns>
/// <remarks>Be sure that the destination buffer is big enough to hold the string.</remarks>
HRESULT SaveVersionInfo::Load_String(IStorage *storage, int id, char *string)
{
	*string = '\0';

	HRESULT res;
	IStreamPtr stm;

	res = storage->OpenStream(Stream_Name_From_ID(id), NULL, STGM_SHARE_EXCLUSIVE, 0, &stm);
	if (FAILED(res)) {
		return(res);
	}

	WCHAR buf[128];
	ULONG count;

	int i = 0;
	for (; i < ARRAY_SIZE(buf); i++) {
		res = stm->Read(&buf[i], sizeof(buf[i]), &count);
		if (FAILED(res)) {
			return(res);
		}
		if (res != S_OK || count != sizeof(buf[i])) {
			return(E_FAIL);
		}
		if (buf[i] == '\0') {
			break;
		}
	}

	if (i == ARRAY_SIZE(buf)) {
		return(E_FAIL);
	}

	WideCharToMultiByte(CP_ACP, 0, buf, -1, string, ARRAY_SIZE(buf) - 1, 0, 0);

	return(S_OK);
}


/// <summary>
/// Reads a string from the save file's property set.
/// The wide text held in the property is narrowed back into the caller's buffer. That buffer
/// is emptied before the read is attempted, so a missing property yields an empty string.
/// </summary>
/// <param name="id">The summary information property identifier to read.</param>
/// <returns>Returns with the result of the read. A failure means the property set could not
/// be opened.</returns>
/// <remarks>Be sure that the destination buffer is big enough to hold the property text.</remarks>
HRESULT SaveVersionInfo::Load_String_Set(IPropertySetStorage *storageset, int id, char *string)
{
	*string = '\0';

	HRESULT res;
	IPropertyStoragePtr storage;

	res = storageset->Open(FMTID_SummaryInformation, STGM_SHARE_EXCLUSIVE|STGM_READWRITE, &storage);
	if (FAILED(res)) {
		return(res);
	}

	PROPSPEC propsec;
	propsec.ulKind = PRSPEC_PROPID;
	propsec.propid = id;
	PROPVARIANT propvar;

	res = storage->ReadMultiple(1, &propsec, &propvar);
	if (FAILED(res)) {
		return(res);
	}

	if (propvar.vt == VT_LPWSTR) {
		WideCharToMultiByte(CP_ACP, 0, propvar.pwszVal, -1, string, 128, 0, 0);
	}

	return(res);
}


/// <summary>
/// Reads an integer from a stream of its own.
/// This is the old style counterpart of Load_Int_Set, used for save files written before
/// the version information moved into a property set. The value is cleared first.
/// </summary>
/// <param name="id">The property identifier naming the stream to open.</param>
/// <returns>Returns with the result of the read. A failure means the stream is absent.</returns>
HRESULT SaveVersionInfo::Load_Int(IStorage *storage, int id, int *integer)
{
	*integer = 0;

	HRESULT res;
	IStreamPtr stm;

	res = storage->OpenStream(Stream_Name_From_ID(id), NULL, STGM_SHARE_EXCLUSIVE, 0, &stm);
	if (FAILED(res)) {
		return(res);
	}

	res = stm->Read(integer, sizeof(*integer), NULL);
	if (FAILED(res)) {
		return(res);
	}

	return(res);
}


/// <summary>
/// Reads an integer from the save file's property set.
/// The value is cleared before the read is attempted, so a save file that does not carry
/// the property leaves the caller with zero.
/// </summary>
/// <param name="id">The summary information property identifier to read.</param>
/// <returns>Returns with the result of the read. A failure means the property set could not
/// be opened.</returns>
HRESULT SaveVersionInfo::Load_Int_Set(IPropertySetStorage *storageset, int id, int *integer)
{
	*integer = 0;

	HRESULT res;
	IPropertyStoragePtr storage;

	res = storageset->Open(FMTID_SummaryInformation, STGM_SHARE_EXCLUSIVE|STGM_READWRITE, &storage);
	if (FAILED(res)) {
		return(res);
	}

	PROPSPEC propsec;
	propsec.ulKind = PRSPEC_PROPID;
	propsec.propid = id;
	PROPVARIANT propvar;

	res = storage->ReadMultiple(1, &propsec, &propvar);
	if (FAILED(res)) {
		return(res);
	}

	if (propvar.vt == VT_I4) {
		*integer = propvar.lVal;
	}

	return(res);
}


/// <summary>
/// Writes a string to a stream of its own.
/// The text is widened before it is written. This is the old style counterpart of
/// Save_String_Set, kept for readers that do not understand property sets.
/// </summary>
/// <param name="id">The property identifier naming the stream to create.</param>
/// <returns>Returns with the result of the write. A failure means the stream was never
/// committed.</returns>
HRESULT SaveVersionInfo::Save_String(IStorage *storage, int id, char *string)
{
	WCHAR buf[128];

	MultiByteToWideChar(CP_ACP, 0, string, -1, buf, ARRAY_SIZE(buf));

	IStreamPtr stm(NULL);

	HRESULT res = storage->CreateStream(Stream_Name_From_ID(id), STGM_SHARE_EXCLUSIVE|STGM_READWRITE, 0, 0, &stm);
	if (FAILED(res)) {
		return(res);
	}

	res = stm->Write(buf, sizeof(WCHAR) * wcslen(buf) + 2, NULL);
	if (FAILED(res)) {
		return(res);
	}
	res = stm->Commit(0);
	if (FAILED(res)) {
		return(res);
	}

	return(res);
}


/// <summary>
/// Writes a string into the save file's property set.
/// The text is widened before it is stored, since the summary information properties are
/// held as wide characters. The property set is created if the save file has none yet.
/// </summary>
/// <param name="id">The summary information property identifier to write.</param>
/// <returns>Returns with the result of the write. A failure means the property set could
/// neither be opened nor created.</returns>
HRESULT SaveVersionInfo::Save_String_Set(IPropertySetStorage *storageset, int id, const char *string)
{
	WCHAR buf[128];

	MultiByteToWideChar(CP_ACP, 0, string, -1, buf, ARRAY_SIZE(buf));

	HRESULT res;
	IPropertyStoragePtr storage;

	res = storageset->Open(FMTID_SummaryInformation, STGM_SHARE_EXCLUSIVE|STGM_READWRITE, &storage);
	if (FAILED(res)) {
		res = storageset->Create(FMTID_SummaryInformation, NULL, PROPSETFLAG_DEFAULT, STGM_SHARE_EXCLUSIVE|STGM_READWRITE|STGM_CREATE, &storage);
		if (FAILED(res)) {
			return(res);
		}
	}

	PROPSPEC propsec;
	propsec.ulKind = PRSPEC_PROPID;
	propsec.propid = id;
	PROPVARIANT propvar;

	propvar.vt = VT_LPWSTR;
	propvar.pwszVal = buf;

	res = storage->WriteMultiple(1, &propsec, &propvar, PID_FIRST_USABLE);
	if (FAILED(res)) {
		return(res);
	}

	return(res);
}


/// <summary>
/// Writes an integer to a stream of its own.
/// This is the old style counterpart of Save_Int_Set, kept so that a reader which does not
/// understand property sets can still recover the value.
/// </summary>
/// <param name="id">The property identifier naming the stream to create.</param>
/// <returns>Returns with the result of the write. A failure means the stream was never
/// committed.</returns>
HRESULT SaveVersionInfo::Save_Int(IStorage *storage, int id, int integer)
{
	IStreamPtr stm(NULL);

	HRESULT res = storage->CreateStream(Stream_Name_From_ID(id), STGM_SHARE_EXCLUSIVE|STGM_READWRITE, 0, 0, &stm);
	if (FAILED(res)) {
		return(res);
	}

	res = stm->Write(&integer, sizeof(integer), NULL);
	if (FAILED(res)) {
		return(res);
	}
	res = stm->Commit(STGM_READ);
	if (FAILED(res)) {
		return(res);
	}

	return(res);
}


/// <summary>
/// Writes an integer into the save file's property set.
/// This routine stores the value as a summary information property, creating the property
/// set first if the save file does not carry one yet.
/// </summary>
/// <param name="id">The summary information property identifier to write.</param>
/// <returns>Returns with the result of the write. A failure means the property set could
/// neither be opened nor created.</returns>
HRESULT SaveVersionInfo::Save_Int_Set(IPropertySetStorage *storageset, int id, int integer)
{
	HRESULT res;
	IPropertyStoragePtr storage;

	res = storageset->Open(FMTID_SummaryInformation, STGM_SHARE_EXCLUSIVE|STGM_READWRITE, &storage);
	if (FAILED(res)) {
		res = storageset->Create(FMTID_SummaryInformation, NULL, PROPSETFLAG_DEFAULT, STGM_SHARE_EXCLUSIVE|STGM_READWRITE|STGM_CREATE, &storage);
		if (FAILED(res)) {
			return(res);
		}
	}

	PROPSPEC propsec;
	propsec.ulKind = PRSPEC_PROPID;
	propsec.propid = id;
	PROPVARIANT propvar;

	propvar.vt = VT_I4;
	propvar.lVal = integer;

	res = storage->WriteMultiple(1, &propsec, &propvar, PID_FIRST_USABLE);
	if (FAILED(res)) {
		return(res);
	}

	return(res);
}


/// <summary>
/// Reads a time stamp from a stream of its own.
/// This is the old style counterpart of Load_Time_Set, used for save files written before
/// the version information moved into a property set. The time is cleared first.
/// </summary>
/// <param name="id">The property identifier naming the stream to open.</param>
/// <returns>Returns with the result of the read. A failure means the stream is absent.</returns>
HRESULT SaveVersionInfo::Load_Time(IStorage *storage, int id, FILETIME *time)
{
	time->dwLowDateTime = 0;
	time->dwHighDateTime = 0;

	HRESULT res;
	IStreamPtr stm;

	res = storage->OpenStream(Stream_Name_From_ID(id), NULL, STGM_SHARE_EXCLUSIVE, 0, &stm);
	if (FAILED(res)) {
		return(res);
	}

	res = stm->Read(time, sizeof(*time), NULL);
	if (FAILED(res)) {
		return(res);
	}

	return(res);
}


/// <summary>
/// Reads a time stamp from the save file's property set.
/// The time is cleared before the read is attempted, so a save file that does not carry the
/// property leaves the caller with a zero time rather than with garbage.
/// </summary>
/// <param name="id">The summary information property identifier to read.</param>
/// <returns>Returns with the result of the read. A failure means the property set could not
/// be opened.</returns>
HRESULT SaveVersionInfo::Load_Time_Set(IPropertySetStorage *storageset, int id, FILETIME *time)
{
	time->dwLowDateTime = 0;
	time->dwHighDateTime = 0;

	HRESULT res;
	IPropertyStoragePtr storage;

	res = storageset->Open(FMTID_SummaryInformation, STGM_SHARE_EXCLUSIVE|STGM_READWRITE, &storage);
	if (FAILED(res)) {
		return(res);
	}

	PROPSPEC propsec;
	propsec.ulKind = PRSPEC_PROPID;
	propsec.propid = id;
	PROPVARIANT propvar;

	res = storage->ReadMultiple(1, &propsec, &propvar);
	if (FAILED(res)) {
		return(res);
	}

	if (propvar.vt == VT_FILETIME) {
		*time = propvar.filetime;
	}

	return(res);
}


/// <summary>
/// Writes a time stamp to a stream of its own.
/// This is the old style counterpart of Save_Time_Set. The save routine records every value
/// this way as well, so that a reader which does not understand property sets can still
/// recover it.
/// </summary>
/// <param name="id">The property identifier naming the stream to create.</param>
/// <returns>Returns with the result of the write. A failure means the stream was never
/// committed.</returns>
HRESULT SaveVersionInfo::Save_Time(IStorage *storage, int id, FILETIME *time)
{
	IStreamPtr stm(NULL);

	HRESULT res = storage->CreateStream(Stream_Name_From_ID(id), STGM_SHARE_EXCLUSIVE|STGM_READWRITE, 0, 0, &stm);
	if (FAILED(res)) {
		return(res);
	}

	res = stm->Write(time, sizeof(*time), NULL);
	if (FAILED(res)) {
		return(res);
	}
	res = stm->Commit(STGM_READ);
	if (FAILED(res)) {
		return(res);
	}

	return(res);
}


/// <summary>
/// Writes a time stamp into the save file's property set.
/// This routine stores the time as a summary information property, creating the property
/// set first if the save file does not carry one yet.
/// </summary>
/// <param name="id">The summary information property identifier to write.</param>
/// <returns>Returns with the result of the write. A failure means the property set could
/// neither be opened nor created.</returns>
HRESULT SaveVersionInfo::Save_Time_Set(IPropertySetStorage *storageset, int id, FILETIME *time)
{
	HRESULT res;
	IPropertyStoragePtr storage;

	res = storageset->Open(FMTID_SummaryInformation, STGM_SHARE_EXCLUSIVE|STGM_READWRITE, &storage);
	if (FAILED(res)) {
		res = storageset->Create(FMTID_SummaryInformation, NULL, PROPSETFLAG_DEFAULT, STGM_SHARE_EXCLUSIVE|STGM_READWRITE|STGM_CREATE, &storage);
		if (FAILED(res)) {
			return(res);
		}
	}

	PROPSPEC propsec;
	propsec.ulKind = PRSPEC_PROPID;
	propsec.propid = id;
	PROPVARIANT propvar;

	propvar.vt = VT_FILETIME;
	propvar.filetime = *time;

	res = storage->WriteMultiple(1, &propsec, &propvar, PID_FIRST_USABLE);
	if (FAILED(res)) {
		return(res);
	}

	return(res);
}


/// <summary>
/// Fetches the stream name that a save version property is stored under.
/// This routine maps the summary information property identifiers onto the wide names used
/// by the old style save format, where every value lives in a stream of its own. The stream
/// based save and load helpers call it to name the stream they are about to open.
/// </summary>
/// <param name="id">The summary information property identifier to look up.</param>
/// <returns>Returns with the stream name for the property, or NULL if the identifier is not
/// one of the recorded version properties.</returns>
const WCHAR *Stream_Name_From_ID(int id)
{
	static struct pidsiStruct {
		int ID;
		WCHAR const *Name;
	} _ids[] = {
		{PIDSI_SCEN_DESCRIP,	L"Scenario Description"},
		{PIDSI_PLAYER_HOUSE,	L"Player House"},
		{PIDSI_G_VERSION,	L"Version"},
		{PIDSI_INTERNAL_VER,	L"Internal Version"},
		{PIDSI_G_START_TIME,	L"Start Time"},
		{PIDSI_LAST_SAVE_TIME,	L"Last Save Time"},
		{PIDSI_G_PLAY_TIME,		L"Play Time"},
		{PIDSI_EXEC_NAME,		L"Executable Name"},
		{PIDSI_PLAYER_NAME1,	L"Player Name"},
		{PIDSI_PLAYER_NAME2,	L"Player Name2"},
		{PIDSI_SCENARIO_NUM,	L"Scenario Number"},
		{PIDSI_CAMPAIGN_NUM,	L"Campaign"},
		{PIDSI_GAME_TYPE,		L"GameType"},
	};

	for (int i = 0; i < ARRAY_SIZE(_ids); i++) {
		if (_ids[i].ID == id) {
			return(_ids[i].Name);
		}
	}

	return(NULL);
}
