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

#include "always.h"

#include "_keyboar.h"
#include "_map.h"
#include "_mixfile.h"
#include "_pk.h"
#include "_rect.h"
#include "_rules.h"
#include "_wsproto.h"
#include "addon.h"
#include "arraylist.h"
#include "conquer.h"
#include "data.h"
#include "dbgprint.h"
#include "dict.h"
#include "gamedirs.h"
#include "globals.h"
#include "goptions.h"
#include "houstype.h"
#include "init.h"
#include "initguid.h"
#include "ipxmgr.h"
#include "language\language.h"
#include "mapgen.h"
#include "mixfile.h"
#include "mplayer.h"
#include "netshare.h"
#include "newmenu.h"
#include "ownrdraw.h"
#include "rules.h"
#include "scenario.h"
#include "sendfile.h"
#include "session.h"
#include "sounddlg.h"
#include "srfcache.h"
#include "stats.h"
#include "stimer.h"
#include "theme.h"
#include "windlg.h"
#include "winfix.h"
#include "winstub.h"
#define IID_DEFINED
#include "wolapi\chatdefs.h"
#include "wolapi\downloaddefs.h"
#include "wolapi\ftpdefs.h"
#include "wolapi\netutildefs.h"
#include "wolapi\wolapi.h"
#include "wolapi\wolapi_i.c"
#include "wonline.h"
#include "worlddom.h"
#include "wsproto.h"
#include "wstring.h"

#include <algorithm>
#include <atlbase.h>  /// ERROR: atlimpl.cpp is obsolete. Please remove it from your project.
extern CComModule _Module;  // Required for COM - must be between atlbase.h and atlcom.h.  Funky, no?
#include <atlcom.h>
#include <commctrl.h> /// Needed for the PBM_ progress bar messages.
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <shellapi.h>
#include <windows.h>

#define strncasecmp _strnicmp
#define strcasecmp	_stricmp

/*
 * Private custom messages for the Westwood Online dialogs. wParam/lParam are
 * unused; each is posted to a WOL dialog to drive a refresh.
 */
#define WOL_GENERATE_RANDMAP	(WM_USER + 401)		/// options_str (host): generate the WDT random map; posted to itself by WM_INITDIALOG
#define WOL_REFRESH_GAMELIST	(WM_USER + 500)		/// find-game: rebuild the channel list (IDC_CHANNELS) applying the current filters
#define WOL_SHOW_GAMEDETAILS	(WM_USER + 501)		/// find-game: populate the game-details panel for the selected channel
#define WOL_LOGIN_OK			(WM_USER + 512)		/// login: success notification sent to the WOL login dialog

/// The download event sink signals WS_Top_Window() with the find-game pair's values, so
/// whichever dialog is on top interprets them; these name the download dialog's readings.
#define WOL_DOWNLOAD_COMPLETE	(WM_USER + 500)		/// download: transfer finished -- close the dialog
#define WOL_DOWNLOAD_FAILED		(WM_USER + 501)		/// download: transfer failed -- notify, abort, and close



#define MAX_USERNAME_LEN 10
#define MAX_PASSWORD_LEN 9

#define WOL_LOBBY_NUM 6
#define WOL_LOBBY_PASSWORD "zotclot9"

#define MAX_USER_COUNT 16

#define APP_REG_KEY "SOFTWARE\\Westwood\\Tiberian Sun"

// forward declarations
void Show_Wait_Window(unsigned int event, bool block = true, const char * text = NULL);
void Draw_Player_List(int keep_selection = false);
BOOL CALLBACK WOL_Waiting_Dialog_Proc(HWND win, UINT uMsg, WPARAM wParam, LPARAM lParam);
bool WOL_Wait_Callback(void);
void Close_Wait_Window(unsigned int event);
void Handle_User_Leave(struct User & user);
int Join_Lobby(void);
BOOL CALLBACK WOL_Download_Dialog_Proc(HWND win, UINT uMsg, WPARAM wParam, LPARAM lParam);
User & Get_Channel_Host(void);
void Shutdown_Chat(void);
BOOL CALLBACK WOL_Find_Page_Dialog_Proc(HWND win, UINT uMsg, WPARAM wParam, LPARAM lParam);
void Fill_Session_Players(struct User * users);
bool Handle_Preview_Download(void);
int Get_WDT_State_Silent(void);
BOOL CALLBACK WOL_Main_Dialog_Proc(HWND win, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK WOL_Login_Dialog_Proc(HWND win, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK WOL_New_Nick_Dialog_Proc(HWND win, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK WOL_Begin_Nick_Dialog_Proc(HWND win, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK WOL_Find_Game_Dialog_Proc(HWND win, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK WOL_Options_Dialog_Proc(HWND win, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK WOL_Ladder_Dialog_Proc(HWND win, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK WOL_Guest_Dialog_Proc(HWND win, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK WOL_Password_Dialog_Proc(HWND win, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK WOL_New_Chat_Dialog_Proc(HWND win, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK WOL_New_Game_Dialog_Proc(HWND win, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK WOL_Game_Options_Proc(HWND win, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL WOL_Button_Bar_Proc(HWND win, UINT uMsg, WPARAM wParam, LPARAM lParam);

void NewLogin(HWND win);
void FetchLogin(const char *str, HWND win);
int StoreLogin(const char *loginName, const char *password, bool passwordPlaintext, int locale);

/// static inits

/**
 ** OLEInitializer class - Init and shutdown OLE & COM as a global
 ** object.  Scary, nasty stuff, COM.  /me shivers.
 **/
class OLEInitializer
{
	public:
		OLEInitializer(void) { OleInitialize(NULL); }
		~OLEInitializer(void) { OleUninitialize(); }
};
OLEInitializer g_OLEInitializer;
CComModule _Module;

ArrayList<Server> g_Servers;
ArrayList<Channel> g_UserChannels;
ArrayList<Channel> g_GameChannelList;
ArrayList<User> g_UserList;
ArrayList<unsigned int> g_ChanListQueue;
Dictionary<Wstring, Ladder> g_BattleClansLadderDict(Wstring_Hash);
Dictionary<Wstring, Ladder> g_WDTLadderDict(Wstring_Hash);
Dictionary<Wstring, Ladder> g_GeneralLadderDict(Wstring_Hash);
Dictionary<Wstring, int> g_WDTTeams(Wstring_Hash);
ArrayList<Wstring> g_WDTTeamLookupQueue;
Dictionary<Wstring, int> g_UserLocales(Wstring_Hash);
ArrayList<Wstring> g_UserLocaleLookupQueue;

ArrayList<Wstring> g_WDT_NodWinEmphasisSounds;
ArrayList<Wstring> g_WDT_GDIWinEmphasisSounds;
ArrayList<Wstring> g_WDT_NodLoseEmphasisSounds;
ArrayList<Wstring> g_WDT_GDILoseEmphasisSounds;
ArrayList<Wstring> g_WDT_NodTerritoryWinSounds;
ArrayList<Wstring> g_WDT_GDITerritoryWinSounds;
ArrayList<Wstring> g_WDT_NodTerritoryLoseSounds;
ArrayList<Wstring> g_WDT_GDITerritoryLoseSounds;

char g_WDTHost[128];
int g_WDTPort;
WorldDominationTour::Voices::Sample * g_WDTVoices;
bool g_WDTSoundsInited;

#define CHANNELTYPE_CHAT 0
#define CHANNELTYPE_GAME 18

/// Most names are exposed by debug prints in RA2, rest guessed
enum {
	EV_ABORT,
	EV_GOT_SERVERS,
	EV_CONNECTED,
	EV_CHAN_LIST,
	EV_CHAN_JOINED,
	EV_ERROR,
	EV_EXIT,
	EV_STARTGAME,
	EV_QUITGAME,
	EV_NETIGNORE,
	EV_CHAN_JOIN_FAIL,
	EV_11, // continue waiting?
	EV_12,
	EV_13,
	EV_LOCALE,
	EV_SKIRMISH,
	EV_COUNT,
};

enum {
	WOL_WAIT_0 = 1 << 0,
	WOL_WAIT_CHANNEL_JOIN = 1 << 1,
	WOL_WAIT_CHANNEL_CREATE = 1 << 2,
	WOL_WAIT_3 = 1 << 3,
	WOL_WAIT_LOGIN_DONE = 1 << 4,
	WOL_WAIT_LOGOUT_DONE = 1 << 5,
	WOL_WAIT_CHANNEL_LEAVE = 1 << 6,
	WOL_WAIT_IDLE = 1 << 7,
	WOL_WAIT_NEW_NICK = 1 << 8,
	WOL_WAIT_9 = 1 << 9,
	WOL_WAIT_ALL = 0xFFFFFFFF,
};

BOOL g_AllowPage = FALSE;
BOOL g_AllowFind = FALSE;
BOOL g_LangFilter = FALSE;
BOOL g_LobbyMusic = FALSE;
BOOL g_ShowAllChannels = FALSE;

HANDLE g_WaitEventHandles[EV_COUNT];

int WDT_TERRITORY_LOBBY_INDEXES[WOL_LOBBY_NUM];

#define WOL_MAX_PLAYERS 4

int g_LastChatListTime = 0;
int g_LastGameListTime = 0;
//
//
int g_UserAge = 0;
int g_UserConsent = 0;
unsigned int g_ActiveWaitFlags = 0;
int g_MaxPlayers = 0;
int g_SuspendChatPump;
bool g_PlayingNetGame = false;
bool g_ConnectionLost = false;
bool g_IgnoreNetStatus = false;
int g_LastIgnoredStatus;
bool g_GameStartRequested;
char g_PendingChanPass[20];

/*
 * column offsets
 */

/*
 * Ladder columns
 */
int g_Col_Rung;
int g_Col_Name;
int g_Col_Points;
int g_Col_Wins;
int g_Col_Losses;
int g_Col_Disconnects;

/*
 * Host/Guest columns
 */
int g_Col_Accept;
int g_Col_House;
int g_Col_Gamename;
int g_Col_Gameinfo;
int g_Col_GamePing;

/*
 * Channel columns
 */
int g_Col_ChanIcon;
int g_Col_ChanPrivate;
int g_Col_ChanName;
int g_Col_PingTime;

/*
 * User columns
 */
int g_Col_Username;
int g_Col_Chanop;
int g_Col_Squelch;
int g_Col_Rank;

/*
 * Find game columns
 */
int g_Col_GameIcon = 2;
int g_Col_GamePrivate = 19;
int g_Col_GameName = 34;
int g_Col_GamePingTime = 172;


int WestwoodOnline_Tournament = 0;
int WestwoodOnline_GameID = 0;
int WestwoodOnline_StartTime = 0;
int WestwoodOnline_GameSKU_TS = 0;
int WestwoodOnline_GameSKU_FS = 0;
int WestwoodOnline_GameSKU_WDT = 0;
int WestwoodOnline_PingTimes[MAX_PLAYERS];

char WestwoodOnline_LoginName[36];
char WestwoodOnline_UserName[16];

char WestwoodOnline_Clan1_Players[136];
char WestwoodOnline_Clan2_Players[136];
char g_OwnSquadName[41];

char g_GameServerHost[128];
int g_GameServerPort;

char g_LadderServerHost[128];
int g_LadderServerPort;

int g_LadderPos;
int g_LadderLocale;

Channel g_CurrentChannel;
Server g_GameServer;
Server g_WDTServer;

bool g_GameStarted;

char g_NickName[40];
char g_LoginPassword[40];

bool g_RecievingPreview = false;

DWORD g_dwChatAdvise;
DWORD g_dwNetUtilAdvise;
DWORD g_dwDownloadAdvise;

int g_PingsSent;
int g_PingsReceived;

HRESULT g_OnNewNickResult;
HRESULT g_AgeCheckResult;
HRESULT g_OnConnectionResult;

int g_ChannelCount;
int g_IsChannelCreator;
int g_LanguageCode;
int g_OwnSquadID;
int g_RequestingOwnSquadInfo;
int g_SquadInfoRequests;
int g_ListIconHeight = 14;
bool g_ShowMOTD = true;
bool g_PlaintextPassword = true;
int g_ServerType = -1;
char g_LastErrorMessage[256];
char * g_MessageOfTheDay;

struct ChannelUserInfo
{
	char name[20];
	int color;
	int house;
	int accepted;
	char pad[0x70];
};

ChannelUserInfo g_UserInfo[MAX_USER_COUNT];

Channel s_TempChannel;

char gWDTLobbies[WOL_LOBBY_NUM][32];
const char * g_Lobbies[WOL_LOBBY_NUM];
int g_CurrentLobby;

int g_RuleCRC;
int g_ArtCRC;
int g_AICRC;

Dictionary<Wstring, Ladder> * g_ActiveLadder;
int g_CurrentServerIndex;
int g_TargetServerIndex;
int g_JoinLobbyNow;
unsigned g_LastChannelLocaleRequestTime;
unsigned g_LastWDTUserTeamRequestTime;
int g_NextWDTStatePollTime;

#define TIBERIAN_SUN_SKU 0x1200
#define FIRESTORM_SKU 0x1C00
#define WORLDDOM_SKU 0x1D00
#define BATTLECLANS 0x800000

//	SKU, reported to WOLAPI for the purpose of finding patches.
#define WOL_GAME_SKU		TIBERIAN_SUN_SKU

#define WOL_GAME_VERSION	0x00020003
#define WOL_GAME_TYPE		21

#define LOB_PREFIX			"Lob_18_"

#define LADDER_CODE(x) (x & 0xFFFF00)

int g_GameSKU = TIBERIAN_SUN_SKU;
int g_SelectedLadderSKU;
int g_InstalledLadderSKU;

enum WOL_LEVEL {
	WOL_LEVEL_SERVERS,				///	Viewing server choices.
	WOL_LEVEL_ROOT,					//	Viewing top level menu choices.
	WOL_LEVEL_USERCHAT,				//	Viewing user chat channels.
	WOL_LEVEL_OFFICIALCHAT,			//	Viewing official chat channels.
	WOL_LEVEL_LOBBIES,				//	Viewing the game lobbies.
	WOL_LEVEL_GAMES,				//	Viewing types (skus) of games.
	WOL_LEVEL_OTHER_GAMES,			///	Viewing other kind of games.

	WOL_LEVEL_BACK_SERVERS,			/// Go back to servers.
	WOL_LEVEL_BACK_ROOT,			/// Go back to top level.
	WOL_LEVEL_BACK_LOBBIES,			/// Go back to lobbies.
	WOL_LEVEL_BACK_USERCHAT,		/// Go back to user chat channels.
	WOL_LEVEL_BACK_OFFICIALCHAT,	/// Go back to official chat channels.
};
WOL_LEVEL CurrentLevel = WOL_LEVEL_SERVERS;


/// <summary>
/// Opens the Find Page dialog, waits for it to close, then restores the input-ignore flag
/// and forces the tactical map to fully redraw.
/// </summary>
void DoFindPage(void)
{
	HWND dlg = WS_Create_Dialog(ProgramInstance, IDD_WOL_FINDPAGE, MainWindow, (DLGPROC)WOL_Find_Page_Dialog_Proc, FALSE);
	Center_Window_Within_Window(dlg);
	OwnerDraw::Subclass_Dialog(dlg, 0);
	SendMessage(dlg, OD_SETTOP, 0, 1);
	IgnoreInput = true;
	Keyboard->Clear();
	ShowWindow(dlg, SW_NORMAL);
	WS_Wait_Dialog(dlg, OwnerDraw::Dialog_Message_Handler);
	Keyboard->Clear();
	IgnoreInput = Scen->IsInputLocked;
	Map.Flag_To_Redraw(GS_REDRAW_ALL);
}

/// <summary>
/// Searches the global user list for an entry whose name matches the given name
/// case-insensitively.
/// </summary>
/// <param name="name">Name to search for.</param>
/// <returns>Index of the matching user in g_UserList, or -1 if no user was found.</returns>
int Player_Name_To_Index(char * name)
{
	int i;
	User * user = 0;
	for (i = 0; i < g_UserList.length(); i++) {
		g_UserList.getPointer(&user, i);
		if (stricmp((char *)name, (char *)user->name) == 0) {
			return(i);
		}
	}
	return(-1);
}

/// <summary>
/// Displays a web page in the player's own browser.
/// This routine hands the page to whatever program the system has registered for HTML and
/// then waits on it, pumping the wait callback throughout so the Westwood Online connection
/// does not lapse while the player reads. The game window is brought back to the front once
/// the browser is finished with. If no browser is installed the player is told so.
/// </summary>
/// <param name="name">URL or page name passed to the browser as the "[open]" argument.</param>
/// <param name="no_ask">Should the browser be launched without asking the player first?</param>
void ViewHTML(const char * name, int no_ask)
{
	/*
	 * Create a throwaway HTML file so FindExecutable can tell us which browser handles it.
	 */
	char filename[160];
	tmpnam(filename);
	strcat(filename, ".html");

	FILE * file = fopen(filename, "w");
	fprintf(file, "<title>Hi there.</title>");
	fclose(file);

	char exeName[MAX_PATH + 1];
	int result = (int)FindExecutable(filename, NULL, exeName);

	_unlink(filename);

	/// Ask the user for confirmation (unless no_ask forces an immediate launch), then spawn the browser.
	if ((result > 32) && (no_ask || (ODMessageBox(Fetch_String(TXT_LAUNCHBROWSER), MB_YESNO, WOL_Wait_Callback) == IDYES))) {
		
		char commandLine[MAX_PATH + 10];
		sprintf(commandLine, "[open] %s", name);
		
		STARTUPINFO startupInfo;
		memset(&startupInfo, 0, sizeof(startupInfo));
		startupInfo.cb = sizeof(startupInfo);

		PROCESS_INFORMATION processInfo;

		BOOL createSuccess = CreateProcess(
			exeName,
			commandLine,
			NULL,
			NULL,
			FALSE,
			0,
			NULL,
			NULL,
			&startupInfo,
			&processInfo);

		if (createSuccess) {
			HANDLE process = processInfo.hProcess;
			if (process) {
				int done_waiting = FALSE;

				WOL_Wait_Callback();

				WaitForInputIdle(process, 5000);

				/// Wait for the browser to exit, or for the game window to regain focus.
				while (!done_waiting) {
					WOL_Wait_Callback();
					Sleep(500);

					DWORD exitCode;
					GetExitCodeProcess(process, &exitCode);
					if (exitCode != STILL_ACTIVE) {
						done_waiting = TRUE;
					}

					if (GameInFocus || GetTopWindow(NULL) == MainWindow || GetForegroundWindow() == MainWindow) {
						done_waiting = TRUE;
					}
					WOL_Wait_Callback();
				}

				/*
				 * Bring the game window back to the foreground now that the browser is done.
				 */
				WOL_Wait_Callback();
	
				if (GetTopWindow(NULL) != MainWindow) {
					WOL_Wait_Callback();
					SetForegroundWindow(MainWindow);
					ShowWindow(MainWindow, SW_RESTORE);
					WOL_Wait_Callback();
				}

				int waits = 10;
				while (waits) {
					Sleep(100);
					WOL_Wait_Callback();
					waits--;
				};
			}
			CloseHandle(processInfo.hThread);
			CloseHandle(processInfo.hProcess);
			return;
		}
	} else if (result <= 32) {
		/*
		 * No handler found for this file type -- tell the user there's no browser installed.
		 */
		ODMessageBox(Fetch_String(TXT_NOBROWSER), MB_OK, WOL_Wait_Callback);
	}
}

/// <summary>
/// Determines the sort order of two channel users.
/// Channel owners come first, then users with voice, then everyone else; equals are listed
/// alphabetically. This is the order the user list box is kept in.
/// </summary>
/// <returns>Returns with a negative value if the u1 user sorts first, a positive value if
/// the u2 one does, and zero if the two cannot be told apart.</returns>
static int Compare_Users(User * u1, User * u2)
{
	User * users[2];
	int value[2];
	users[0] = u1;
	users[1] = u2;
	value[0] = 0;
	value[1] = 0;

	/// Score each user: channel owner outranks voice, which outranks a plain user.
	for (int i = 0; i < 2; i++) {
		if (users[i]->flags & CHAT_USER_CHANNELOWNER) value[i] += 4;
		if (users[i]->flags & CHAT_USER_VOICE) value[i] += 2;
	}
	if (value[0] < value[1]) {
		return(1);
	} else if (value[0] > value[1]) {
		return(-1);
	}
	return(stricmp((char *)u1->name, (char *)u2->name));
}

/// <summary>
/// Moves the chat connection to another Westwood Online server.
/// This routine logs out of the server we are on, connects to the new one under the same
/// credentials, and refreshes both channel lists before it returns, so the caller can carry
/// on as though nothing had moved. Asking for the server we are already on does nothing.
/// </summary>
/// <param name="type">CHANNELTYPE_CHAT for the default chat server, or CHANNELTYPE_GAME for
/// the ladder server currently being targeted.</param>
/// <returns>bool; Was the switch completed? A false return means it was aborted
/// along the way.</returns>
bool Switch_Server(int type)
{
	int wait_result;
	int i;
	Server * server = NULL;
	g_LastErrorMessage[0] = '\0';

	/// Already on the requested server (or there's nowhere else to switch to) -- nothing to do.
	if (type == g_ServerType && (g_CurrentServerIndex == g_TargetServerIndex || type == CHANNELTYPE_CHAT) || g_Servers.length() <= 1) {
		return(true);
	}

	/*
	 * Enable/disable the Find Game button depending on whether we're headed to the ladder server.
	 */
	HWND findgame = WS_Find_Dialog(IDD_WOL_MAIN);
	if (type == CHANNELTYPE_CHAT) {
		if (findgame != NULL) {
			EnableWindow(GetDlgItem(findgame, IDC_FINDGAME), FALSE);
		}
	} else {
		if (findgame != NULL) {
			EnableWindow(GetDlgItem(findgame, IDC_FINDGAME), TRUE);
		}
	}

	/*
	 * Log out of the current server and put up the connecting-to-server wait popup.
	 */
	g_IgnoreNetStatus = true;
	g_LastIgnoredStatus = 0;
	g_pChat->RequestLogout();
	Show_Wait_Window(WOL_WAIT_LOGIN_DONE, false);
	Set_Wait_Dialog_Text((char *)Fetch_String(TXT_CONNECTING_SERVER));

	for (i = 0; i < ARRAY_SIZE(g_WaitEventHandles); i++) {
		ResetEvent(g_WaitEventHandles[i]);
	}

	/// Wait for the server to confirm the logout, or for an abort.
	while (true) {
		wait_result = WaitForMultipleObjects(EV_COUNT, g_WaitEventHandles, FALSE, 100);
		while (wait_result == WAIT_TIMEOUT) {
			WOL_Wait_Callback();
			wait_result = WaitForMultipleObjects(EV_COUNT, g_WaitEventHandles, FALSE, 100);
		}
		ResetEvent(g_WaitEventHandles[wait_result]);

		if (wait_result == EV_NETIGNORE && g_LastIgnoredStatus == CHAT_S_CON_DISCONNECTED) {
			break;
		} else if (wait_result == EV_ABORT) {
			goto abort;
		}
	}

	g_IgnoreNetStatus = false;
	g_ShowMOTD = false;

	/// Pick the server record to connect to based on the requested switch type.
	if (type == CHANNELTYPE_CHAT) {
		g_Servers.getPointer(&server, 0);
	} else {
		if (type == CHANNELTYPE_GAME) {
			g_Servers.getPointer(&server, g_TargetServerIndex);
		}
	}

	/*
	 * Request a connection to the new server using the current login credentials.
	 */
	strcpy((char *)server->login, g_NickName);
	strcpy((char *)server->password, g_LoginPassword);
	g_pChat->RequestConnection(server, 20, ((int &)g_PlaintextPassword) & 0xFF);

	wait_result = WaitForMultipleObjects(EV_COUNT, g_WaitEventHandles, FALSE, 100);
	while (wait_result == WAIT_TIMEOUT) {
		WOL_Wait_Callback();
		wait_result = WaitForMultipleObjects(EV_COUNT, g_WaitEventHandles, FALSE, 100);
	}
	ResetEvent(g_WaitEventHandles[wait_result]);

	if (wait_result == EV_ABORT) {
		goto abort;
	}

	g_ShowMOTD = true;
	g_CurrentServerIndex = g_TargetServerIndex;
	Set_Wait_Dialog_Text((char *)Fetch_String(TXT_REQ_CHANLIST));

	/// Request the normal user-channel list for the new server.
	if (g_pChat->RequestChannelList(CHANNELTYPE_CHAT, 0) == S_OK) {
		unsigned type = CHANNELTYPE_CHAT;
		g_ChanListQueue.addTail(type);
		g_LastChatListTime = time(0);
	}

	wait_result = WaitForMultipleObjects(EV_COUNT, g_WaitEventHandles, FALSE, 100);
	while (wait_result == WAIT_TIMEOUT) {
		WOL_Wait_Callback();
		wait_result = WaitForMultipleObjects(EV_COUNT, g_WaitEventHandles, FALSE, 100);
	}
	ResetEvent(g_WaitEventHandles[wait_result]);

	if (wait_result == EV_ABORT) {
		goto abort;
	}

	/// Also request the ladder/game channel list.
	if (g_pChat->RequestChannelList(CHANNELTYPE_GAME, 1) == S_OK) {
		unsigned requested_type = CHANNELTYPE_GAME;
		g_ChanListQueue.addTail(requested_type);
		g_LastGameListTime = time(0);
	}

	Close_Wait_Window(WOL_WAIT_ALL);
	g_ServerType = type;

	return(true);

abort:
	Close_Wait_Window(WOL_WAIT_ALL);
	ODMessageBox(g_LastErrorMessage, 0, WOL_Wait_Callback);
	return(false);
}

/// <summary>
/// Loads the WOL lobby settings (paging, finding, language filter, lobby music, show-all)
/// from the config INI into their global flags.
/// </summary>
void Read_WOL_Settings(void)
{
	g_AllowPage = ConfigINI.Get_Int("WOnline", "AllowPage", TRUE);
	g_AllowFind = ConfigINI.Get_Int("WOnline", "AllowFind", TRUE);
	g_LangFilter = ConfigINI.Get_Int("WOnline", "LangFilter", TRUE);
	g_LobbyMusic = ConfigINI.Get_Int("WOnline", "LobMusic", TRUE);
	g_ShowAllChannels = ConfigINI.Get_Int("WOnline", "ShowAll", TRUE);
}

/// <summary>
/// Writes the current WOL lobby settings (paging, finding, language filter, lobby music,
/// show-all) to the config INI and saves it to disk.
/// </summary>
void Write_WOL_Settings(void)
{
	std::string const path = User_File_Write_Name(CONFIG_FILE_NAME);
	RawFileClass file(path.c_str());
	ConfigINI.Put_Int("WOnline", "AllowPage", g_AllowPage);
	ConfigINI.Put_Int("WOnline", "AllowFind", g_AllowFind);
	ConfigINI.Put_Int("WOnline", "LangFilter", g_LangFilter);
	ConfigINI.Put_Int("WOnline", "LobMusic", g_LobbyMusic);
	ConfigINI.Put_Int("WOnline", "ShowAll", g_ShowAllChannels);
	ConfigINI.Save(file, false);
}

/// <summary>
/// Applies the current WOL settings to the active chat session: pushes the language filter
/// and find/page flags to the chat client, and starts, continues, or fades out the lobby
/// music depending on the LobMusic setting.
/// </summary>
void Apply_WOL_Settings(void)
{
	if (g_pChat) {
		g_pChat->SetLangFilter(g_LangFilter);
		g_pChat->SetFindPage(g_AllowFind, g_AllowPage);

		/// Manage the lobby music according to the LobMusic setting.
		if ((g_LobbyMusic) && (Theme.Still_Playing() == false)) {
			Theme.Stop();
			Theme.Queue_Song(THEME_PICK_ANOTHER);
		} else if (g_LobbyMusic) {
			if (Theme.What_Is_Playing() == Fetch_Main_Menu_Theme() || Theme.What_Is_Playing() == Fetch_Map_Select_Theme()) {
				Theme.Stop();
				Theme.Queue_Song(THEME_PICK_ANOTHER);
			}
		} else if (!g_LobbyMusic) {
			Theme.Fade_Out();
		}
	}
}

/// <summary>
/// Sends a chat message on the player's behalf.
/// The message goes privately to whoever is selected in the user list box, or out to the
/// whole channel when nothing is selected.
/// </summary>
/// <param name="msg">Text of the message to send.</param>
/// <returns>Returns with 0 if the message went to the whole channel, or 1 if it went
/// privately to the selected users.</returns>
int Send_Chat_Message(char * msg)
{
	static int sel_user_indexes[128];
	HWND users = GetDlgItem(WS_Top_Window(), IDC_USERS);
	int count = ListBox_GetSelItems(users, (WPARAM)32, (LPARAM)sel_user_indexes);

	if (count == 0) {
		/*
		 * No users selected -- broadcast to the whole channel.
		 */
		g_pChat->RequestPublicMessage((LPCSTR)msg);
		return(0);
	} else {
		/*
		 * Build a linked list of the selected users and send them a private message.
		 */
		User * last = NULL;
		User * head = NULL;
		User * item = NULL;
		User source;
		memset(&source, 0, sizeof(source));
		for (int i = 0; i < count; i++) {
			g_UserList.get(source, sel_user_indexes[i]);
			item = new User;
			*item = source;
			item->next = NULL;
			if (i == 0) head = item;
			if (last) last->next = item;
			last = item;
		}

		if (head) g_pChat->RequestPrivateMessage(head, (LPCSTR)msg);

		/// Free the temporary user list.
		while (head) {
			item = head;
			head = head->next;
			delete (item);
		}
		return(1);
	}
}

/// <summary>
/// Sends a chat action on the player's behalf.
/// The action goes privately to whoever is selected in the user list box, or out to the
/// whole channel when nothing is selected.
/// </summary>
/// <param name="msg">Text of the action to send.</param>
/// <returns>Returns with 0 if the action went to the whole channel, or 1 if it went
/// privately to the selected users.</returns>
int Send_Chat_Action(char * msg)
{
	static int sel_user_indexes[128];
	HWND users = GetDlgItem(WS_Top_Window(), IDC_USERS);
	int count = ListBox_GetSelItems(users, (WPARAM)32, (LPARAM)sel_user_indexes);

	if (count == 0) {
		/*
		 * No users selected -- broadcast to the whole channel.
		 */
		g_pChat->RequestPublicAction((LPCSTR)msg);
		return(0);
	} else {
		/*
		 * Build a linked list of the selected users and send them a private action.
		 */
		User * last = NULL;
		User * head = NULL;
		User * item = NULL;
		User source;
		memset(&source, 0, sizeof(source));
		for (int i = 0; i < count; i++) {
			g_UserList.get(source, sel_user_indexes[i]);
			item = new User;
			*item = source;
			item->next = NULL;
			if (i == 0) head = item;
			if (last) last->next = item;
			last = item;
		}

		if (head) g_pChat->RequestPrivateAction(head, (LPCSTR)msg);

		/// Free the temporary user list.
		while (head) {
			item = head;
			head = head->next;
			delete (item);
		}
		return(1);
	}
}

/// <summary>
/// Encodes the current game options and scenario info, followed by each user's name,
/// house, and color, into a single comma-separated string for transmission over WOL.
/// </summary>
/// <param name="out">Buffer that receives the encoded options string.</param>
void Encode_Game_Options(char * out)
{
	static char useroptions[512];

	/// Encode the game options and scenario info.
	sprintf(out,
			"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,"
			"%s,%d,%d,%s,%s:",
			Session.Options.UnitCount, BuildLevel, Session.Options.Credits, Session.Options.FogOfWar, Session.Options.BridgeDestruction, Session.Options.Goodies, Session.Options.MCVRedeploy,
			Session.Options.AlliesAllowed, Session.Options.HarvTruce, Session.Options.Bases, Session.Options.CTF, Seed, Options.GameSpeed, Session.Options.AIPlayers, Session.Options.AIDifficulty,
			Session.Options.ShortGame, Session.Options.CrapEngineers, Session.Options.GameSpeed, strlen(Session.Options.ScenarioDescription) == 0 ? " " : Session.Options.ScenarioDescription,
			Session.ScenarioIsOfficial, Session.ScenarioFileLength, strlen(Session.ScenarioFileName) == 0 ? " " : Session.ScenarioFileName,
			strlen(Session.ScenarioDigest) == 0 ? " " : Session.ScenarioDigest);

	/*
	 * Append each user's name, house, and color.
	 */
	memset(useroptions, 0, sizeof(useroptions));
	for (int i = 0; i < g_UserList.length(); i++) {
		sprintf(useroptions + strlen(useroptions), "%s,%d,%d,", g_UserInfo[i].name, g_UserInfo[i].house, g_UserInfo[i].color);
	}

	strcat(out, useroptions);
}

/// <summary>
/// Combines the rules, AI, and art CRCs into a single hash used to verify that all players
/// are running matching game data.
/// </summary>
/// <returns>Combined CRC hash.</returns>
int GetINIHash(void)
{
	return(g_RuleCRC ^ g_AICRC ^ g_ArtCRC);
}

/// <summary>
/// Encodes extended game/session info -- build number, data hash, options, and a flag
/// describing which house(s) still have room -- into a comma-separated string for
/// transmission over WOL.
/// </summary>
/// <param name="out">Buffer that receives the encoded info string.</param>
void Encode_Channel_ExInfo(char * out)
{
	int i = 0;
	int house1_count = 0;
	int house0_count = 0;

	/// Count how many users are currently on each house.
	for (int j = 0; j < (int)g_CurrentChannel.currentUsers; j++) {
		int house = g_UserInfo[j].house;
		if (house == HOUSE_GOOD) {
			house0_count++;
		} else if (house == HOUSE_BAD) {
			house1_count++;
		}
	}

	/// Flag which house(s) still have free slots.
	if (house1_count < (int)g_CurrentChannel.maxUsers / 2) {
		i = 2;
	}

	if (house0_count < (int)g_CurrentChannel.maxUsers / 2) {
		i++;
	}

	sprintf(out, "%d,%d,%d,%d,%d,%d,%d,%d,%s", Build_Number(), GetINIHash(), BuildLevel, Session.Options.Credits, Session.Options.AlliesAllowed, Session.Options.Bases,
			Session.IsWDT ? Session.WDTTerritory : Session.Options.CTF, i, Session.ScenarioFileName);
}

/// <summary>
/// Records whether the given user (or the local player if who is NULL) has accepted an
/// invitation/request, then refreshes the user list display.
/// </summary>
/// <param name="who">Name of the user to update, or NULL for the local player.</param>
/// <param name="status">New accepted status to store.</param>
void SetPlayerAccepted(char * who, int status)
{
	if (who == NULL) who = g_NickName;

	int offset = Player_Name_To_Index((char *)who);

	if (offset != -1) {
		g_UserInfo[offset].accepted = status;
		Draw_Player_List();
	}
}

/// <summary>
/// Looks up whether the given user (or the local player if who is NULL) has accepted an
/// invitation/request.
/// </summary>
/// <param name="who">Name of the user to query, or NULL for the local player.</param>
/// <returns>The user's accepted status, or -1 if the user could not be found.</returns>
int GetPlayerAccepted(char * who)
{
	if (who == NULL) who = g_NickName;

	int offset = Player_Name_To_Index((char *)who);

	if (offset != -1) {
		return(g_UserInfo[offset].accepted);
	} else {
		return(-1);
	}
}

/// <summary>
/// Updates a user's chosen house and color and refreshes the user list display. If the
/// user is the local player, also syncs the session's preferred color/house and updates the
/// color combo box in the game-options or guest dialog.
/// </summary>
/// <param name="who">Name of the user whose house/color changed.</param>
/// <param name="house">New house selection.</param>
/// <param name="color">New color selection.</param>
/// <returns>TRUE if the user's house selection actually changed, 0 otherwise.</returns>
int Assign_House_And_Color(char * who, int house, int color)
{
	int offset = Player_Name_To_Index((char *)who);
	int house_changed = 0;
	int color_changed = 0;
	int local_player = 0;

	if (strcasecmp(who, g_NickName) == 0) {
		local_player = 1;
	}

	/// Update the stored house/color for the user and note whether the house actually changed.
	if (offset != -1) {
		if (g_UserInfo[offset].house != house) house_changed = TRUE;
		g_UserInfo[offset].house = house;
		if (g_UserInfo[offset].color != color) color_changed = TRUE;
		g_UserInfo[offset].color = color;
		Draw_Player_List();
	} else {
		DebugString("Assign_House_And_Color failed for '%s'!\n", who);
	}

	/// If this is the local player, keep the color combo box UI in sync.
	if (local_player) {
		HWND win = WS_Find_Dialog(IDD_WOL_GAMEOPT);
		Session.PrefColor = color;
		if (win == NULL) win = WS_Find_Dialog(IDD_WOL_GUEST);
		if ((SendDlgItemMessage(win, IDC_YOURCOLOR, CB_GETCURSEL, 0, 0) != color) && (SendDlgItemMessage(win, IDC_YOURCOLOR, CB_GETDROPPEDSTATE, 0, 0) == FALSE)) {
			SendDlgItemMessage(win, IDC_YOURCOLOR, CB_SETCURSEL, color, 0);
		}
	}
	/// Also update the local session's color/house state.
	if (local_player) {
		Session.ColorIdx = color;
		Session.House = house;
	}
	return(house_changed);
}


/// <summary>
/// Moves the current top-level lobby list (CurrentLevel) back to its parent view, implementing
/// the lobby's "Back" navigation.
/// </summary>
void GoBack(void)
{
	if (CurrentLevel == WOL_LEVEL_USERCHAT) {
		CurrentLevel = WOL_LEVEL_ROOT;
	} else if (CurrentLevel == WOL_LEVEL_OFFICIALCHAT) {
		CurrentLevel = WOL_LEVEL_ROOT;
	} else if (CurrentLevel == WOL_LEVEL_USERCHAT) {
		CurrentLevel = WOL_LEVEL_ROOT;
	} else if (CurrentLevel == WOL_LEVEL_GAMES) {
		CurrentLevel = WOL_LEVEL_LOBBIES;
	} else if (CurrentLevel == WOL_LEVEL_LOBBIES) {
		CurrentLevel = WOL_LEVEL_ROOT;
	} else if (CurrentLevel == WOL_LEVEL_OTHER_GAMES) {
		CurrentLevel = WOL_LEVEL_ROOT;
	} else if (CurrentLevel == WOL_LEVEL_BACK_SERVERS) {
		CurrentLevel = WOL_LEVEL_SERVERS;
	} else if (CurrentLevel == WOL_LEVEL_BACK_ROOT) {
		CurrentLevel = WOL_LEVEL_ROOT;
	} else if (CurrentLevel == WOL_LEVEL_BACK_LOBBIES) {
		CurrentLevel = WOL_LEVEL_LOBBIES;
	} else if (CurrentLevel == WOL_LEVEL_ROOT) {
		CurrentLevel = WOL_LEVEL_SERVERS;
	} else if (CurrentLevel == WOL_LEVEL_BACK_OFFICIALCHAT) {
		CurrentLevel = WOL_LEVEL_OFFICIALCHAT;
	} else if (CurrentLevel == WOL_LEVEL_BACK_USERCHAT) {
		CurrentLevel = WOL_LEVEL_USERCHAT;
	}
}

/// <summary>
/// Idle-time callback for the WOL lobby screens. Pumps the Windows message queue, then
/// performs periodic multiplayer housekeeping: re-requests channel lists for the active view,
/// flushes the WDT team and user-locale lookup queues a few entries at a time, polls WDT
/// server state, pumps the chat/netutil/download COM interfaces, refreshes the current
/// channel's extended info, and pings the users in the current channel.
/// </summary>
/// <returns>Always false.</returns>
bool WOL_Wait_Callback(void)
{
	Title_Screen_Restore();

	const unsigned request_interval = 60;
	static int prev_exinfo = 0;
	static int prev_ping = 0;

	/*
	 * Drain up to 100 pending Windows messages before doing periodic housekeeping.
	 */
	MSG msg;
	int counter = 0;
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		counter++;
		if (counter == 100) break;
	}

	/*
	 * If the main lobby or find-game dialog is on top, periodically re-request the
	 * channel list for whichever view is currently showing (games or chat/lobbies).
	 */
	int top_window_id = WS_Top_Window_ID();
	if ((top_window_id == IDD_WOL_MAIN) || (top_window_id == IDD_WOL_FINDGAME)) {
		if ((CurrentLevel == WOL_LEVEL_GAMES) || (top_window_id == IDD_WOL_FINDGAME)) {
			if ((time(NULL) - g_LastGameListTime) > request_interval) {
				if (g_pChat->RequestChannelList(CHANNELTYPE_GAME, 1) == S_OK) {
					unsigned int type = CHANNELTYPE_GAME;
					g_ChanListQueue.addTail(type);
				}
				g_LastGameListTime = time(NULL);
			}
		}

		else if ((CurrentLevel == WOL_LEVEL_LOBBIES) || (CurrentLevel == WOL_LEVEL_USERCHAT) || (CurrentLevel == WOL_LEVEL_OFFICIALCHAT)) {
			if ((time(NULL) - g_LastChatListTime) > request_interval) {
				if (g_pChat->RequestChannelList(CHANNELTYPE_CHAT, 0) == S_OK) {
					unsigned int type = CHANNELTYPE_CHAT;
					g_ChanListQueue.addTail(type);
				}
				g_LastChatListTime = time(NULL);
			}
		}
	}

	/*
	 * Flush the pending WDT team-lookup queue in batches of up to 10 names, escalating
	 * how soon a request is sent as the queue grows longer.
	 */
	if (g_WDTTeamLookupQueue.length() > 200) {
		g_WDTTeamLookupQueue.clear();
	}

	if (g_LastWDTUserTeamRequestTime < (unsigned)time(0) - 2 && g_WDTTeamLookupQueue.length() > 65 ||
		g_LastWDTUserTeamRequestTime < (unsigned)time(0) - 3 && g_WDTTeamLookupQueue.length() > 10 ||
		g_LastWDTUserTeamRequestTime < (unsigned)time(0) - 15 && g_WDTTeamLookupQueue.length() > 0) {

		User * list = NULL;
		User * last_user = NULL;
		for (int i = 0; i < 10; i++) {
			if (g_WDTTeamLookupQueue.length() > 0) {
				User * new_user = new User;
				memset(new_user, 0, sizeof(User));
				Wstring name;
				g_WDTTeamLookupQueue.removeTail(name);
				char * rawName = name.get();
				if (strlen(rawName) > 9) {
					rawName[9] = '\0';
				}
				strcpy((char *)new_user->name, rawName);
				if (last_user == NULL) {
					list = new_user;
				} else {
					last_user->next = new_user;
				}
				last_user = new_user;
			} else {
				break;
			}
		}

		if (g_pChat != NULL) {
			g_pChat->RequestUserTeam(list);
		}

		User * d = list;
		while (d != NULL) {
			User * next = d->next;
			delete d;
			d = next;
		}

		g_LastWDTUserTeamRequestTime = time(0);
	}

	/*
	 * Flush the pending user-locale lookup queue the same way as the team-lookup queue above.
	 */
	if (g_UserLocaleLookupQueue.length() > 200) {
		g_UserLocaleLookupQueue.clear();
	}

	if (g_LastChannelLocaleRequestTime < (unsigned)time(0) - 2 && g_UserLocaleLookupQueue.length() > 65 ||
		g_LastChannelLocaleRequestTime < (unsigned)time(0) - 3 && g_UserLocaleLookupQueue.length() > 10 ||
		g_LastChannelLocaleRequestTime < (unsigned)time(0) - 15 && g_UserLocaleLookupQueue.length() > 0) {

		User * list = NULL;
		User * last_user = NULL;
		for (int i = 0; i < 10; i++) {
			if (g_UserLocaleLookupQueue.length() > 0) {
				User * new_user = new User;
				memset(new_user, 0, sizeof(User));
				Wstring name;
				g_UserLocaleLookupQueue.removeTail(name);
				char * rawName = name.get();
				if (strlen(rawName) > 9) {
					rawName[9] = '\0';
				}
				strcpy((char *)new_user->name, rawName);
				if (last_user == NULL) {
					list = new_user;
				} else {
					last_user->next = new_user;
				}
				last_user = new_user;
			} else {
				break;
			}
		}

		if (g_pChat != NULL) {
			g_pChat->RequestUserLocale(list);
		}

		User * d = list;
		while (d != NULL) {
			User * next = d->next;
			delete d;
			d = next;
		}

		g_LastChannelLocaleRequestTime = time(0);
	}

	/// Poll the WDT server state every 30 seconds while playing a WDT campaign.
	if (Session.IsWDT && time(0) > g_NextWDTStatePollTime) {
		g_NextWDTStatePollTime = time(0) + 30;
		Get_WDT_State_Silent();
	}

	/*
	 * Pump the chat, netutil, and download COM event sinks.
	 */
	if ((g_pChat) && (g_SuspendChatPump == 0)) g_pChat->PumpMessages();
	if (g_pNetUtil) g_pNetUtil->PumpMessages();
	if (g_pDownload) g_pDownload->PumpMessages();

	Call_Back();

	if ((g_CurrentChannel.name[0]) && (g_CurrentChannel.type == CHANNELTYPE_GAME) && (g_IsChannelCreator)) PumpGameopts(false);

	/// If we created the current channel, refresh its extended info every 10 seconds.
	if ((g_pChat != NULL) && (g_CurrentChannel.type != CHANNELTYPE_CHAT) && (g_IsChannelCreator)) {
		if ((time(NULL) - prev_exinfo) > 10) {
			char exinfo_out[80];
			memset(exinfo_out, 0, 80);
			Encode_Channel_ExInfo(exinfo_out);
			exinfo_out[40] = 0;
			g_pChat->SetChannelExInfo(exinfo_out);
			prev_exinfo = time(NULL);
		}
	}

	/*
	 * Every 10 seconds, ping every other user in the current channel to keep latency
	 * information up to date.
	 */
	if ((g_pChat != NULL) && (g_CurrentChannel.type != CHANNELTYPE_CHAT) && ((time(NULL) - prev_ping) > 10)) {
		User user;
		int handle;
		int i;
		in_addr addr;

		for (i = 0; i < g_UserList.length(); i++) {
			g_UserList.get(user, i);
			if (stricmp((char *)user.name, g_NickName) == 0) continue;
			addr.s_addr = user.ipaddr;
			if (user.ipaddr != 0) g_pNetUtil->RequestPing(inet_ntoa(addr), 1000, &handle);
		}
		prev_ping = time(NULL);
	}

	Handle_Preview_Download();
	return(false);
}

/// <summary>
/// Resets all Westwood Online global state back to its defaults for a fresh session: channel
/// and user lists, ladder dictionaries, lookup queues and their throttle timers, cached
/// server/channel structs, the localized lobby names, COM interface pointers and advise
/// cookies, and the rules/art/AI CRCs used for version checking.
/// </summary>
void Reset_WOL_Globals(void)
{
	/*
	 * Clear the channel and user lists.
	 */
	g_UserChannels.clear();
	g_GameChannelList.clear();
	g_UserList.clear();

	/*
	 * Clear the WDT team and user-locale lookup queues and their cached results.
	 */
	g_WDTTeamLookupQueue.clear();
	g_WDTTeams.clear();

	g_UserLocaleLookupQueue.clear();
	g_UserLocales.clear();

	/*
	 * Reset the lookup-queue request throttles and push the next WDT state poll a day out.
	 */
	g_NextWDTStatePollTime = time(0) + 86400;
	g_LastChannelLocaleRequestTime = 0;
	g_LastWDTUserTeamRequestTime = 0;

	/*
	 * Clear the ladder dictionaries; which one becomes active is decided below.
	 */
	g_BattleClansLadderDict.clear();
	g_WDTLadderDict.clear();
	g_GeneralLadderDict.clear();

	if (Addon_Enabled(ADDON_FIRESTORM) && Session.IsWDT) {
		int sku = WORLDDOM_SKU;
		g_InstalledLadderSKU = sku;
		g_SelectedLadderSKU = sku;
		g_ActiveLadder = &g_WDTLadderDict;
	} else {
		int sku = g_GameSKU;
		g_InstalledLadderSKU = sku;
		g_SelectedLadderSKU = sku;
		g_ActiveLadder = &g_GeneralLadderDict;
	}

	WS_Clear_Saved_Values();

	g_LastErrorMessage[0] = 0;

	/*
	 * Zero out the cached current-channel, server, and event-handle structs.
	 */
	memset(&g_CurrentChannel, 0, sizeof(g_CurrentChannel));
	memset(&g_GameServer, 0, sizeof(g_GameServer));
	memset(&g_WDTServer, 0, sizeof(g_WDTServer));
	memset(&g_WaitEventHandles, 0, sizeof(g_WaitEventHandles));

	/// Clear the discovered server list and server-selection indices.
	g_Servers.setEmpty();

	g_CurrentServerIndex = 0;
	g_TargetServerIndex = 0;

	/*
	 * Reset session state flags back to their defaults.
	 */
	g_IsChannelCreator = 0;
	g_ActiveWaitFlags = 0;

	g_GameStartRequested = false;

	g_IgnoreNetStatus = false;
	g_ShowMOTD = true;

	g_PlayingNetGame = false;
	g_ConnectionLost = false;

	g_ServerType = -1;

	g_SuspendChatPump = 0;

	g_LastChatListTime = 0xFFFFFFFF;
	g_LastGameListTime = 0xFFFFFFFF;

	/*
	 * Reload the localized lobby names and clear the WDT territory-to-lobby index map.
	 */
	g_Lobbies[0] = Fetch_String(TXT_LOB_1);
	g_Lobbies[1] = Fetch_String(TXT_LOB_2);
	g_Lobbies[2] = Fetch_String(TXT_LOB_3);
	g_Lobbies[3] = Fetch_String(TXT_LOB_4);
	g_Lobbies[4] = Fetch_String(TXT_LOB_5);
	g_Lobbies[5] = Fetch_String(TXT_LOB_6);

	memset(WDT_TERRITORY_LOBBY_INDEXES, 0xFFFFFFFF, sizeof(WDT_TERRITORY_LOBBY_INDEXES));

	/*
	 * Release the cached chat/netutil/download interface pointers and their advise cookies.
	 */
	g_pChat = NULL;
	g_pNetUtil = NULL;
	g_pDownload = NULL;

	g_dwChatAdvise = 0;
	g_dwNetUtilAdvise = 0;
	g_dwDownloadAdvise = 0;

	/*
	 * Reset squad-lookup request state.
	 */
	g_SquadInfoRequests = 0;
	g_RequestingOwnSquadInfo = 0;
	g_OwnSquadID = 0;
	memset(g_OwnSquadName, 0, sizeof(g_OwnSquadName));

	g_ChannelCount = 0;

	/*
	 * Reload the persisted WOL settings and reset the remaining session counters/flags.
	 */
	Read_WOL_Settings();

	g_LadderPos = 1;

	g_JoinLobbyNow = 0;

	g_ChanListQueue.setEmpty();

	g_GameStarted = false;

	g_UserAge = 0;
	g_UserConsent = 0;

	CurrentLevel = WOL_LEVEL_SERVERS;

	/*
	 * Recompute the rules/art/AI CRCs used to verify game version compatibility online.
	 */
	g_RuleCRC = RulesClass::Get_Rule_Unique_ID();
	RulesClass::Load_Art_INI();
	g_ArtCRC = RulesClass::Get_Art_Unique_ID();
	g_AICRC = RulesClass::Get_AI_Unique_ID();
}

/// <summary>
/// Rebuilds the owner-drawn IDC_CHANNELS list box to reflect the current lobby view
/// (servers, the root menu, games, or chat channels), based on CurrentLevel. Updates the
/// current-channel label and the New button, then repopulates the list with the
/// appropriate icons and text for each row before restoring the previous selection.
/// </summary>
void Draw_Channel_List(void)
{
	int i, j;
	int count = 0;
	int topindex = 0;
	char info[80];
	Channel chan;

	memset(&chan, 0, sizeof(chan));

	HWND win = NULL;
	if (win == NULL) {
		/*
		 * If the find-game dialog is up, just ask it to refresh its own list instead of
		 * rebuilding the main dialog's channel list box.
		 */
		win = WS_Find_Dialog(IDD_WOL_FINDGAME);
		if (win) {
			SendMessage(win, WOL_REFRESH_GAMELIST, 0, 0);
			return;
		}
	}
	if (win == NULL) win = WS_Find_Dialog(IDD_WOL_MAIN);

	HWND chan_list = GetDlgItem(win, IDC_CHANNELS);

	/*
	 * Update the current-channel label, expanding internal LOB_PREFIX names to their
	 * display name.
	 */
	{
		HWND cur_chan_textbox = GetDlgItem(win, IDC_CURCHAN);
		if (cur_chan_textbox) {
			char name[128];
			strcpy(name, (char *)g_CurrentChannel.name);
			if (strncmp((char *)name, LOB_PREFIX, strlen(LOB_PREFIX)) == 0) {
				int num = atol(((char *)name) + strlen(LOB_PREFIX));
				int num2 = num / WOL_LOBBY_NUM;
				int num1 = num % WOL_LOBBY_NUM;
				if (num2 == 0) {
					sprintf(name, "%s", g_Lobbies[num1]);
				} else {
					sprintf(name, "%s %d", g_Lobbies[num1], num2 + 1);
				}
			}

			SendMessage(cur_chan_textbox, WM_SETTEXT, 0, (LPARAM)name);
		}
	}

	/// Configure the New button's enabled state and caption for the current view.
	{
		HWND new_button = GetDlgItem(win, IDC_NEWCHAN);
		if (new_button) {
			if (CurrentLevel == WOL_LEVEL_GAMES) {
				EnableWindow(new_button, TRUE);
				SendMessage(new_button, WM_SETTEXT, 0, (LPARAM) Fetch_String(TXT_NEW_GAME));
			} else if (CurrentLevel == WOL_LEVEL_USERCHAT) {
				EnableWindow(new_button, TRUE);
				SendMessage(new_button, WM_SETTEXT, 0, (LPARAM) Fetch_String(TXT_NEW_CHAT));
			} else {
				EnableWindow(new_button, FALSE);
				SendMessage(new_button, WM_SETTEXT, 0, (LPARAM) Fetch_String(TXT_NEW));
			}
		}
	}

	/*
	 * Remember the current selection and scroll position so they can be restored once the
	 * list box has been rebuilt, then clear it while repainting is suspended.
	 */
	Dictionary<Wstring, bool> lbdict(Wstring_Hash);
	LBSaveSelections(chan_list, lbdict);

	topindex = SendDlgItemMessage(win, IDC_CHANNELS, LB_GETTOPINDEX, 0, 0);

	SendDlgItemMessage(win, IDC_CHANNELS, OD_DISABLEPAINT, 0, 1);

	SendDlgItemMessage(win, IDC_CHANNELS, LB_RESETCONTENT, 0, 0);

	OwnerDraw::CellData thecell;
	thecell.type = OwnerDraw::CellData::SURFACE;
	thecell.surf = SurfaceCache.GetSurface("gt-1.bmp");

	/// Every view except the top-level server list gets a ".." entry to navigate back up.
	if (CurrentLevel != WOL_LEVEL_SERVERS) {
		thecell.type = OwnerDraw::CellData::TEXT;
		char back_text[32];
		sprintf(back_text, "..\\%s", Fetch_String(TXT_BACK));
		thecell.string.set(back_text);
		SendDlgItemMessage(win, IDC_CHANNELS, LB_INSERTSTRING, (WPARAM)-1, (LPARAM) back_text);
		SendDlgItemMessage(win, IDC_CHANNELS, OD_SETCELL, MAKEWPARAM(g_Col_ChanIcon, 0), (LPARAM)&thecell);

		thecell.type = OwnerDraw::CellData::INVALID;
		thecell.hint.set("");
		SendDlgItemMessage(win, IDC_CHANNELS, OD_SETCELL, MAKEWPARAM(g_Col_ChanPrivate, 0), (LPARAM)&thecell);
	}

	thecell.type = OwnerDraw::CellData::SURFACE;

	/// Populate the list box rows for the currently active lobby view.
	switch (CurrentLevel) {
		/// List every known server, marking the one we're currently connected to.
		case WOL_LEVEL_SERVERS: {
			Server * server = NULL;
			for (int i = 1; i < g_Servers.length(); i++) {
				char serverName[128];
				g_Servers.getPointer(&server, i);
				sprintf(serverName, "Server %d", i);
				char * semicolon = strchr((char *)server->name, ':');
				if (semicolon != NULL) {
					strcpy(serverName, semicolon + 1);
				}
				SendDlgItemMessage(win, IDC_CHANNELS, LB_INSERTSTRING, (WPARAM)-1, (LPARAM) serverName);
				if (i != g_CurrentServerIndex) {
					thecell.surf = SurfaceCache.GetSurface("gt-1.bmp");
				} else {
					thecell.surf = SurfaceCache.GetSurface("wolacpt.pcx");
				}
				SendDlgItemMessage(win, IDC_CHANNELS, OD_SETCELL, MAKEWPARAM(g_Col_ChanIcon, i - 1), (LPARAM)&thecell);
			}
		} break;

		/// Top-level menu on a server: game channels, other games, official chat, user chat.
		case WOL_LEVEL_ROOT: {
			thecell.surf = SurfaceCache.GetSurface("gt-1.bmp");
			SendDlgItemMessage(win, IDC_CHANNELS, LB_INSERTSTRING, (WPARAM)-1, (LPARAM) Fetch_String(TXT_GAME_CHAN));
			SendDlgItemMessage(win, IDC_CHANNELS, OD_SETCELL, MAKEWPARAM(g_Col_ChanIcon, 1), (LPARAM)&thecell);
			SendDlgItemMessage(win, IDC_CHANNELS, LB_INSERTSTRING, (WPARAM)-1, (LPARAM) Fetch_String(TXT_OTHER_GAME));
			SendDlgItemMessage(win, IDC_CHANNELS, OD_SETCELL, MAKEWPARAM(g_Col_ChanIcon, 2), (LPARAM)&thecell);
			SendDlgItemMessage(win, IDC_CHANNELS, LB_INSERTSTRING, (WPARAM)-1, (LPARAM) Fetch_String(TXT_OFFICIAL_CHAT));
			SendDlgItemMessage(win, IDC_CHANNELS, OD_SETCELL, MAKEWPARAM(g_Col_ChanIcon, 3), (LPARAM)&thecell);
			SendDlgItemMessage(win, IDC_CHANNELS, LB_INSERTSTRING, (WPARAM)-1, (LPARAM) Fetch_String(TXT_USER_CHAT));
			SendDlgItemMessage(win, IDC_CHANNELS, OD_SETCELL, MAKEWPARAM(g_Col_ChanIcon, 4), (LPARAM)&thecell);
		} break;

		/// Transitional back-navigation states; they add no rows of their own.
		case WOL_LEVEL_BACK_ROOT:
		case WOL_LEVEL_BACK_LOBBIES:
		case WOL_LEVEL_BACK_OFFICIALCHAT:
		case WOL_LEVEL_BACK_USERCHAT: {

		} break;

		/// List joinable game channels, filtered by lobby and by addon/WDT eligibility.
		case WOL_LEVEL_GAMES: {
			int visible_count = -1;
			count = g_GameChannelList.length();

			for (i = 0; i < count; i++) {
				g_GameChannelList.get(chan, i);

				if ((!g_ShowAllChannels) && ((chan.reserved & 0xFF) != (unsigned)g_CurrentLobby)) continue;

				/*
				 * Determine whether this game is allowed for the current client: WDT
				 * territory rules take precedence, otherwise fall back to addon ownership.
				 */
				bool allowed = false;
				if (Session.IsWDT) {
					char buffer[128];
					strcpy(buffer, (char const *)chan.exInfo);
					char * token = strtok(buffer, ",");
					for (int j = 0; j < 6; j++) {
						token = strtok(NULL, ",");
					}
					if (token != NULL && Session.WDTTerritory != atol(token)) {
						continue;
					}
				}

				if (!allowed) {
					AddonType addon = (AddonType)((chan.reserved >> 12) & 0xF);
					allowed = true;
					if (addon != ADDON_BASE_GAME && Addon_Enabled(addon) == false) {
						allowed = false;
					} else if (Addon_Enabled(ADDON_FIRESTORM) == true && addon == ADDON_BASE_GAME) {
						allowed = false;
					}
					// allowed = !(addon != ADDON_BASE_GAME && !Addon_Enabled(addon) || Addon_Enabled(ADDON_FIRESTORM) && addon == ADDON_BASE_GAME);
				}

				if (!allowed) continue;

				visible_count++;
				sprintf(info, "%s  %d/%d", chan.name, chan.currentUsers, chan.maxUsers);
				SendDlgItemMessage(win, IDC_CHANNELS, LB_INSERTSTRING, (WPARAM)-1, (LPARAM)chan.name);

				thecell.type = OwnerDraw::CellData::TEXT;
				thecell.string.set(info);
				thecell.hint.set("");
				thecell.color = ColorSystem;
				if (chan.currentUsers == chan.maxUsers) {
					thecell.color = ColorNoJoin;
				} else {
					if (Session.IsWDT) {
						char buffer[132];
						strcpy(buffer, (char const *)chan.exInfo);
						char * token = strtok(buffer, ",");
						for (int j = 0; j < 7; j++) {
							if (token != NULL) {
								token = strtok(NULL, ",");
							}
						}
						if (token != NULL) {
							int housenum = atol(token);
							if (Session.House == HOUSE_GOOD && housenum == 2 || Session.House == HOUSE_BAD && housenum == 1) {
								thecell.color = ColorNoJoin;
							}
						}
					}
				}
				SendDlgItemMessage(win, IDC_CHANNELS, OD_SETCELL, MAKEWPARAM(g_Col_ChanName, visible_count + 1), (LPARAM)&thecell);

				thecell.type = OwnerDraw::CellData::SURFACE;
				thecell.hint.set("");
				if (chan.tournament) {
					thecell.surf = SurfaceCache.GetSurface("woltrny.pcx");
					thecell.hint.set((char *)Fetch_String(TXT_TOURNAMENT_GAME));
				} else {
					if (chan.reserved & 0x100) {
						thecell.surf = SurfaceCache.GetSurface("wolclan.pcx");
						thecell.hint.set((char *)Fetch_String(TXT_BATTLECLAN_GAME));
					} else {
						thecell.surf = SurfaceCache.GetSurface("gt18.bmp");
					}
				}
				SendDlgItemMessage(win, IDC_CHANNELS, OD_SETCELL, MAKEWPARAM(g_Col_ChanIcon, visible_count + 1), (LPARAM)&thecell);

				thecell.hint.set("");
				if (chan.flags & CHAN_MODE_KEY) {
					thecell.surf = SurfaceCache.GetSurface("wolpriv.pcx");
					thecell.hint.set((char *)Fetch_String(TXT_GAME_PASSWORD));
				} else {
					thecell.type = OwnerDraw::CellData::INVALID;
				}
				SendDlgItemMessage(win, IDC_CHANNELS, OD_SETCELL, MAKEWPARAM(g_Col_ChanPrivate, visible_count + 1), (LPARAM)&thecell);

				thecell.pingtime = chan.latency;
				thecell.type = OwnerDraw::CellData::PING;
				sprintf(info, "Ping = %d ms", chan.latency);
				if (chan.latency == -1) {
					sprintf(info, Fetch_String(TXT_UNKNOWN_PING));
				}
				thecell.hint.set(info);
				SendDlgItemMessage(win, IDC_CHANNELS, OD_SETCELL, MAKEWPARAM(g_Col_PingTime, visible_count + 1), (LPARAM)&thecell);

				thecell.hint.set("");
				SendDlgItemMessage(win, IDC_CHANNELS, LB_SETITEMDATA, visible_count + 1, i);
			}

		} break;

		/// List the other game types the chat server reports as available.
		case WOL_LEVEL_OTHER_GAMES: {

			LPCSTR gametype_list;
			g_pChat->GetGametypeList(&gametype_list);
			char * buffer = strdup(gametype_list);
			char * cptr = strtok(buffer, ",");
			int gtype;
			unsigned char * bitmap;
			int bmp_bytes;
			LPCSTR name;
			LPCSTR url;
			char bitmap_name[64];
			int row;

			while (cptr) {
				gtype = atol(cptr);
				cptr = strtok(NULL, ",");
				if ((gtype <= 0) || (gtype == CHANNELTYPE_GAME)) continue;

				g_pChat->GetGametypeInfo(gtype, g_ListIconHeight, &bitmap, &bmp_bytes, &name, &url);
				row = SendDlgItemMessage(win, IDC_CHANNELS, LB_INSERTSTRING, (WPARAM)-1, (LPARAM)name);

				SendDlgItemMessage(win, IDC_CHANNELS, LB_SETITEMDATA, row, gtype);

				sprintf(bitmap_name, "gt%d.bmp", gtype);
				thecell.surf = SurfaceCache.GetSurface(bitmap_name);
				SendDlgItemMessage(win, IDC_CHANNELS, OD_SETCELL, MAKEWPARAM(g_Col_ChanIcon, row), (LPARAM)&thecell);
			}
			free(buffer);
		} break;

		/*
		 * List chat channels: official-only, user-created-only, or the internal lobby
		 * channels (named "Lob_18_*"), depending on which of the three views is active.
		 */
		case WOL_LEVEL_OFFICIALCHAT:
		case WOL_LEVEL_USERCHAT:
		case WOL_LEVEL_LOBBIES: {

			count = g_UserChannels.length();
			for (i = 0; i < count; i++) {
				g_UserChannels.get(chan, i);
				if ((chan.official == 0) && (CurrentLevel == WOL_LEVEL_OFFICIALCHAT)) continue;
				if ((chan.official != 0) && (CurrentLevel == WOL_LEVEL_USERCHAT)) continue;
				if (strncmp((char *)chan.name, LOB_PREFIX, strlen(LOB_PREFIX)) == 0) {
					if (CurrentLevel != WOL_LEVEL_LOBBIES) continue;
				} else if (CurrentLevel == WOL_LEVEL_LOBBIES) {
					continue;
				}

				/// Lobby channels are named/numbered from their internal "Lob_18_N" name.
				if (CurrentLevel == WOL_LEVEL_LOBBIES) {
					int num = atol(((char *)chan.name) + strlen(LOB_PREFIX));
					int num2 = num / WOL_LOBBY_NUM;
					int num1 = num % WOL_LOBBY_NUM;
					if (num2 == 0) {
						sprintf(info, "%s  (%d)", g_Lobbies[num1], chan.currentUsers);
					} else {
						sprintf(info, "%s_%d  (%d)", g_Lobbies[num1], num2 + 1, chan.currentUsers);
					}
				} else {
					sprintf(info, "%s  (%d)", chan.name, chan.currentUsers);
				}

				j = SendDlgItemMessage(WS_Top_Window(), IDC_CHANNELS, LB_INSERTSTRING, (WPARAM)-1, (LPARAM)chan.name);

				thecell.type = OwnerDraw::CellData::TEXT;
				thecell.string.set(info);
				thecell.hint.set("");
				SendDlgItemMessage(WS_Top_Window(), IDC_CHANNELS, OD_SETCELL, MAKEWPARAM(g_Col_ChanPrivate, j), (LPARAM)&thecell);

				if (CurrentLevel == WOL_LEVEL_LOBBIES) {
					thecell.type = OwnerDraw::CellData::SURFACE;
					thecell.surf = SurfaceCache.GetSurface("gt18.bmp");
					SendDlgItemMessage(win, IDC_CHANNELS, OD_SETCELL, MAKEWPARAM(g_Col_ChanIcon, j), (LPARAM)&thecell);
				}

				SendDlgItemMessage(WS_Top_Window(), IDC_CHANNELS, LB_SETITEMDATA, j, i);
			}
		} break;

		default:
			break;
	}

	/*
	 * Restore the previous selection and scroll position, then re-enable painting.
	 */
	LBRestoreSelections(chan_list, lbdict);

	SendDlgItemMessage(win, IDC_CHANNELS, LB_SETTOPINDEX, (WPARAM)topindex, 0);

	SendDlgItemMessage(win, IDC_CHANNELS, OD_DISABLEPAINT, 0, 0);

	/// Only enable the Join/Leave button when a real, selectable channel is highlighted.
	if ((g_CurrentChannel.name[0] == 0) || (CurrentLevel == WOL_LEVEL_GAMES)) {
		int idx = SendDlgItemMessage(win, IDC_CHANNELS, LB_GETCURSEL, 0, 0);
		int enable = FALSE;
		if (idx != LB_ERR && idx != 0) enable = TRUE;
		if (SendDlgItemMessage(win, IDC_CHANNELS, LB_GETSEL, idx, 0) <= 0) enable = FALSE;
		EnableWindow(GetDlgItem(win, IDC_JOINLEAVE), enable);
	}

	InvalidateRect(chan_list, NULL, 0);
	UpdateWindow(chan_list);
}

/// <summary>
/// Rebuilds the owner-drawn IDC_USERS list box for the current channel: for a chat channel
/// it lists usernames with rank/locale/team-color info plus operator, voice, and squelch
/// icons; for a game channel it lists players with house, acceptance, and ping icons. Also
/// updates the Join/Leave button, the kick/ban button availability for the local user, and
/// the user-count label, restoring the previous selection unless told not to.
/// </summary>
/// <param name="keep_selection">Nonzero to skip restoring the previous list selection.</param>
void Draw_Player_List(int keep_selection)
{
	int i;
	int slot_index;
	char info[80];
	User user;
	int house;
	int star;

	HWND win = NULL;
	win = GameoptWindow();
	if (win == NULL) win = WS_Find_Dialog(IDD_WOL_MAIN);

	HWND userwin = GetDlgItem(win, IDC_USERS);

	if ((win == NULL) || (userwin == NULL)) return;

	/// Set the Join/Leave button to match whether we're currently in a channel.
	if ((g_CurrentChannel.name[0]) && (CurrentLevel != WOL_LEVEL_GAMES)) {
		SendDlgItemMessage(win, IDC_JOINLEAVE, WM_SETTEXT, 0, (LPARAM) Fetch_String(TXT_LEAVE));
		EnableWindow(GetDlgItem(win, IDC_JOINLEAVE), TRUE);
	} else {
		SendDlgItemMessage(win, IDC_JOINLEAVE, WM_SETTEXT, 0, (LPARAM) Fetch_String(TXT_JOIN));
	}

	/*
	 * Remember the current selection and scroll position, then clear the list box while
	 * repainting is suspended.
	 */
	Dictionary<Wstring, bool> lbdict(Wstring_Hash);

	LBSaveSelections(userwin, lbdict);

	int topindex = SendDlgItemMessage(win, IDC_USERS, LB_GETTOPINDEX, 0, 0);

	SendDlgItemMessage(win, IDC_USERS, OD_DISABLEPAINT, 0, 1);

	SendDlgItemMessage(win, IDC_USERS, LB_RESETCONTENT, 0, 0);

	Wstring ladder_name;
	Ladder ladder;
	char rank_string[80];
	static char localestring[64];
	int average_ping;

	memset(&ladder, 0, sizeof(ladder));
	OwnerDraw::CellData thecell;

	for (i = 0; i < g_UserList.length(); i++) {
		g_UserList.get(user, i);

		/// If this row is the local user, enable Kick/Ban only when we're the channel owner.
		if (strcasecmp((char *)user.name, g_NickName) == 0) {
			HWND hwnd = NULL;
			DWORD style = 0;
			hwnd = GetDlgItem(win, IDC_KICK);
			if (hwnd) {
				style = GetWindowLong(hwnd, GWL_STYLE);
				if (user.flags & CHAT_USER_CHANNELOWNER) {
					style &= ~WS_DISABLED;
				} else {
					style |= WS_DISABLED;
				}
				SetWindowLong(hwnd, GWL_STYLE, style);
				InvalidateRect(hwnd, NULL, 0);
			}
			hwnd = GetDlgItem(win, IDC_BAN);
			if (hwnd) {
				style = GetWindowLong(hwnd, GWL_STYLE);
				if (user.flags & CHAT_USER_CHANNELOWNER) {
					style &= ~WS_DISABLED;
				} else {
					style |= WS_DISABLED;
				}
				SetWindowLong(hwnd, GWL_STYLE, style);
				InvalidateRect(hwnd, NULL, 0);
			}
		}

		/*
		 * A chat channel: list the username, colored by WDT team if applicable,
		 * with its locale and ladder rank appended.
		 */
		if (g_CurrentChannel.type == CHANNELTYPE_CHAT) {

			sprintf(info, "%s", user.name);
			SendDlgItemMessage(win, IDC_USERS, LB_INSERTSTRING, (WPARAM)-1, (LPARAM)info);

			thecell.type = OwnerDraw::CellData::TEXT;
			thecell.hint.set((char *)Fetch_String(TXT_UNRANKED));
			thecell.string.set((char *)user.name);

			/// Color the name by WDT team, unless this user is the channel owner.
			if (Session.IsWDT) {
				if ((user.flags & CHAT_USER_CHANNELOWNER) == 0) {
					int color;
					if (g_WDTTeams.contains(thecell.string)) {
						g_WDTTeams.getValue(thecell.string, color);
						thecell.color = PlayerColorTable[color];
					} else {
						thecell.color = ColorSystem;
					}
				} else {
					thecell.color = ColorSystem;
				}
			}

			/// Append the user's locale (if known) and ladder rank to the name cell's subtitle.
			ladder_name.set((char *)user.name);
			ladder_name.toLower();
			int locale = 0;
			memset(localestring, 0, sizeof(localestring));
			char * loc = NULL;
			if (g_UserLocales.contains(ladder_name) && (g_pChat)) {
				g_UserLocales.getValue(ladder_name, locale);
				g_pChat->GetLocaleString((LPCSTR *)&loc, (Locale)locale);
			}
			if (locale) {
				sprintf(localestring, "(%s)", loc);
			} else {
				memset(localestring, 0, sizeof(localestring));
			}
			if (g_UserLocales.contains(ladder_name)) {
				if (locale) {
					thecell.hint.insert(" ", 0);
				}
				thecell.hint.insert(localestring, 0);
			}
			if (g_ActiveLadder->getValue(ladder_name, ladder)) {
				sprintf(rank_string, "%s %d", Fetch_String(TXT_RANK), ladder.rung);
				thecell.hint.set(rank_string);
				if (g_UserLocales.contains(ladder_name)) {
					if (locale) {
						thecell.hint.insert(" ", 0);
					}
					thecell.hint.insert(localestring, 0);
				}
			}
			SendDlgItemMessage(win, IDC_USERS, OD_SETCELL, MAKEWPARAM(g_Col_Username, i), (LPARAM)&thecell);

			/// Separate rank column: the squad tag takes priority over the ladder rank.
			thecell.type = OwnerDraw::CellData::TEXT;
			ladder_name.set((char *)user.name);
			ladder_name.toLower();
			rank_string[0] = 0;

			if (strlen((char *)user.squadabbrev)) {
				sprintf(rank_string, "[%s]", user.squadabbrev);
				thecell.hint.set((char *)user.squadname);
			} else if (g_ActiveLadder->getValue(ladder_name, ladder)) {
				sprintf(rank_string, "(%s %d)", Fetch_String(TXT_RANK), ladder.rung);
				thecell.hint.set("");
			}
			thecell.string.set(rank_string);
			SendDlgItemMessage(win, IDC_USERS, OD_SETCELL, MAKEWPARAM(g_Col_Rank, i), (LPARAM)&thecell);

			/// Show an operator or voice icon for privileged users.
			thecell.type = OwnerDraw::CellData::TEXT;
			if (user.flags & CHAT_USER_CHANNELOWNER) {
				thecell.type = OwnerDraw::CellData::SURFACE;
				thecell.surf = SurfaceCache.GetSurface("woloper.pcx");
				thecell.hint.set((char *)Fetch_String(TXT_OPER));
			} else if (user.flags & CHAT_USER_VOICE) {
				thecell.type = OwnerDraw::CellData::SURFACE;
				thecell.surf = SurfaceCache.GetSurface("wolvoice.pcx");
				thecell.hint.set((char *)Fetch_String(TXT_VOICE));
			} else {
				thecell.string.set("");
				thecell.hint.set("");
			}
			SendDlgItemMessage(win, IDC_USERS, OD_SETCELL, MAKEWPARAM(g_Col_Chanop, i), (LPARAM)&thecell);

			/// Show a squelch (ignore) icon if we have this user muted.
			thecell.type = OwnerDraw::CellData::TEXT;
			if (!g_pChat || g_pChat->GetSquelch(&user) != S_OK) {
				thecell.string.set("");
				thecell.hint.set("");
			} else {
				thecell.type = OwnerDraw::CellData::SURFACE;
				thecell.surf = SurfaceCache.GetSurface("wolsqlch.pcx");
				thecell.hint.set((char *)Fetch_String(TXT_IGNOREUSER));
			}
			SendDlgItemMessage(win, IDC_USERS, OD_SETCELL, MAKEWPARAM(g_Col_Squelch, i), (LPARAM)&thecell);
			thecell.type = OwnerDraw::CellData::TEXT;
		} else {
			/*
			 * A game channel: list players with their chosen house, ready/host star, and
			 * ping instead of chat-channel icons.
			 */
			if (HouseTypes.Count() <= 1) return;

			/// Look up this player's house selection and acceptance/host status.
			slot_index = Player_Name_To_Index((char *)user.name);

			house = HOUSE_GOOD;
			star = 0;
			if (slot_index != -1) {
				house = g_UserInfo[slot_index].house;
				if (g_UserInfo[slot_index].accepted) star = 1;

				if (slot_index == 0) {
					g_UserInfo[slot_index].accepted = 1;
					star = 2;
				}
			}

			house = std::max<int>(house, HOUSE_FIRST);

			ladder_name.set((char *)user.name);
			ladder_name.toLower();

			sprintf(info, "%s", user.name);
			for (int j = 0; j < (int)strlen(info); j++) {
				info[j] = tolower(info[j]);
			}

			SendDlgItemMessage(win, IDC_USERS, LB_INSERTSTRING, (WPARAM)-1, (LPARAM)info);

			/// Primary name cell: colored by player slot, with the ladder rank as a subtitle.
			thecell.type = OwnerDraw::CellData::PRIMARY;
			thecell.color = PlayerColorTable[g_UserInfo[slot_index].color];
			thecell.hint.set((char *)Fetch_String(TXT_UNRANKED));
			if (g_ActiveLadder->getValue(ladder_name, ladder)) {
				sprintf(rank_string, "%s %d", Fetch_String(TXT_RANK), ladder.rung);
				thecell.hint.set(rank_string);
			}
			SendDlgItemMessage(win, IDC_USERS, OD_SETCELL, MAKEWPARAM(g_Col_Gamename, i), (LPARAM)&thecell);

			/// Secondary info cell: squad tag takes priority over the ladder rank.
			thecell.type = OwnerDraw::CellData::TEXT;
			if (strlen((char *)user.squadabbrev) != 0) {
				sprintf(info, "[%s]", user.squadabbrev);
				thecell.hint.set((char *)user.squadname);
			} else {
				if (g_ActiveLadder->getValue(ladder_name, ladder)) {
					sprintf(info, "(%s %d)", Fetch_String(TXT_RANK), ladder.rung);
				} else {
					sprintf(info, "");
				}
				thecell.hint.set("");
			}
			thecell.string.set(info);
			SendDlgItemMessage(win, IDC_USERS, OD_SETCELL, MAKEWPARAM(g_Col_Gameinfo, i), (LPARAM)&thecell);

			/// House icon: GDI or Nod.
			thecell.type = OwnerDraw::CellData::SURFACE;
			if (house == HOUSE_GOOD) {
				thecell.surf = SurfaceCache.GetSurface("gdii.pcx");
				sprintf(info, "%s", Fetch_String(TXT_GDI));
			} else {
				thecell.surf = SurfaceCache.GetSurface("nodi.pcx");
				sprintf(info, "%s", Fetch_String(TXT_NOD));
			}
			thecell.hint.set(info);
			SendDlgItemMessage(win, IDC_USERS, OD_SETCELL, MAKEWPARAM(g_Col_House, i), (LPARAM)&thecell);

			/// Acceptance star: the host gets one icon, an accepted non-host player another.
			thecell.type = OwnerDraw::CellData::SURFACE;
			if (star) {
				if (star == 2) {
					thecell.surf = SurfaceCache.GetSurface("wolhost.pcx");
				} else {
					thecell.surf = SurfaceCache.GetSurface("wolacpt.pcx");
				}
				thecell.hint.set((char *)Fetch_String(TXT_ACCEPTED));
			} else {
				thecell.type = OwnerDraw::CellData::INVALID;
				thecell.hint.set("");
			}
			SendDlgItemMessage(win, IDC_USERS, OD_SETCELL, MAKEWPARAM(g_Col_Accept, i), (LPARAM)&thecell);

			/*
			 * Ping column; hidden for our own row since pinging ourselves is meaningless.
			 */
			average_ping = 0;
			g_pNetUtil->GetAvgPing(user.ipaddr, &average_ping);
			if (average_ping < 0) average_ping = 0;
			thecell.type = OwnerDraw::CellData::PING;
			thecell.pingtime = average_ping;
			char ping[64];
			sprintf(ping, "Ping = %dms", average_ping);
			if (average_ping == 0) {
				sprintf(ping, Fetch_String(TXT_UNKNOWN_PING));
			}
			thecell.hint.set(ping);
			if (stricmp(g_NickName, (char *)user.name) == 0) {
				thecell.type = OwnerDraw::CellData::INVALID;
			}
			SendDlgItemMessage(win, IDC_USERS, OD_SETCELL, MAKEWPARAM(g_Col_GamePing, i), (LPARAM)&thecell);
		}
	}

	/*
	 * Restore the previous selection unless the caller asked us not to.
	 */
	if (keep_selection == 0) LBRestoreSelections(userwin, lbdict);

	/*
	 * Update the user-count label, including the selected count when anything is selected.
	 */
	int sel_count = SendDlgItemMessage(win, IDC_USERS, LB_GETSELCOUNT, 0, 0);
	int user_count = g_UserList.length();
	char str[20];
	if (sel_count > 0) {
		sprintf(str, "%d  (%d)", user_count, sel_count);
	} else {
		sprintf(str, "%d", user_count);
	}

	SendDlgItemMessage(win, IDC_USERCOUNT, WM_SETTEXT, 0, (LPARAM)str);

	SendDlgItemMessage(win, IDC_USERS, LB_SETTOPINDEX, (WPARAM)topindex, 0);

	SendDlgItemMessage(win, IDC_USERS, OD_DISABLEPAINT, 0, 0);

	InvalidateRect(GetDlgItem(win, IDC_USERS), NULL, 0);
}

/// <summary>
/// Wrapper that refreshes the users list box using the default selection-restore behavior.
/// </summary>
void Display_Users(void)
{
	Draw_Player_List();
}

/*
 * CNetUtilEventSink
 */

void WDT_Resume_Campaign(WDTState *state);

class CNetUtilEventSink : public CComObjectRoot, public INetUtilEvent
{
	public:
		CNetUtilEventSink(void) {}
		BEGIN_COM_MAP(CNetUtilEventSink)
		COM_INTERFACE_ENTRY(INetUtilEvent)
		END_COM_MAP()

		/// <summary>
		/// Processes a World-Domination-Tour state packet from the server. Validates the
		/// result/length, builds a WDTState, handles NACK / previous-cycle / current-cycle
		/// cases, diffs against the cached state, starts a new campaign, plays territory
		/// win/lose sounds when the front line moves, leaves the game channel if the cycle
		/// or territory changed, and updates the current-channel label of the main dialog.
		/// </summary>
		/// <param name="res">Result code from the server; negative signals an error.</param>
		/// <param name="state">Pointer to the raw WDT state packet data.</param>
		/// <param name="length">Length of the packet data; must be positive.</param>
		/// <returns>S_OK.</returns>
		STDMETHOD(OnWDTState)(HRESULT res, unsigned char * state, int length)
		{
			if (res >= 0 && length > 0) {
				WDTState * newstate = new WDTState(state);

				if (newstate->Has_Owner_History()) {
					if (newstate->Is_Previous_Cycle()) {
						/*
						 * The server sent stale data for a cycle we already finished; just
						 * let the caller's WDT continue-campaign handling take over.
						 */
						DebugString("OnWDTState: Previous cycle data received\n");
						WDT_Resume_Campaign(newstate);
						SetEvent(g_WaitEventHandles[EV_11]);
						return(S_OK);
					}

					DebugString("OnWDTState: Current cycle data received\n");

					g_NextWDTStatePollTime = newstate->TickTime + time(0);

					WDTState * oldstate = WDT_Get_New_State();
					if (oldstate != NULL) {
						if (newstate->CycleID != oldstate->CycleID || newstate->NumTicks != oldstate->NumTicks) {
							/*
							 * The cycle advanced or a new tick came in; snapshot the old
							 * state before starting the new campaign, since WDT_Start_New_Campaign
							 * will replace it.
							 */
							oldstate = WDT_Get_New_State();
							unsigned int oldticks = oldstate->NumTicks;
							int oldterritory = Session.WDTTerritory;
							unsigned int oldcycle = oldstate->CycleID;

							WDT_Start_New_Campaign(newstate);
							Init_WDT();

							int newterritory = WDT_TERRITORY_LOBBY_INDEXES[g_CurrentLobby];
							Session.WDTTerritory = newterritory;

							/*
							 * If a new tick happened for the same cycle while we are not
							 * in a net game, and our territory changed, tell the player
							 * the front line moved and play the appropriate win/lose
							 * sound for their side.
							 */
							if (!g_PlayingNetGame && newstate->CycleID == oldcycle && oldticks != newstate->NumTicks) {
								if (newterritory != oldterritory) {
									if (WS_Find_Dialog(IDD_WOL_MAIN)) {
										PMessagePrintf(ColorSystem, Fetch_String(TXT_WDT_FRONT_LINE_CHANGED));

										unsigned char owner = newstate->OwnerHistory[newstate->NumTicks - 1][oldterritory];
										if (owner == 2) {
											if (Session.House == HOUSE_GOOD) {
												Play_WDT_Sound(g_WDT_GDITerritoryLoseSounds);
											} else if (Session.House == HOUSE_BAD) {
												Play_WDT_Sound(g_WDT_NodTerritoryWinSounds);
											}
										} else if (owner == 1) {
											if (Session.House == HOUSE_GOOD) {
												Play_WDT_Sound(g_WDT_GDITerritoryWinSounds);
											} else if (Session.House == HOUSE_BAD) {
												Play_WDT_Sound(g_WDT_NodTerritoryLoseSounds);
											}
										}
									}
								}
							}

							/*
							 * Territory or cycle changed underneath us; leave the current
							 * game channel since it no longer applies.
							 */
							if (Session.WDTTerritory != oldterritory || newstate->CycleID != oldcycle) {
								if (g_CurrentChannel.type == CHANNELTYPE_GAME) {
									if (g_pChat != NULL) {
										g_pChat->RequestChannelLeave();
									}
								}
							}

							/*
							 * Refresh the current-channel label on the main dialog. If
							 * we are sitting in a numbered WDT lobby channel, replace the
							 * raw channel name with the friendly lobby name (and cycle
							 * number, if past the first).
							 */
							HWND mainwin = WS_Find_Dialog(IDD_WOL_MAIN);
							HWND cur_chan_textbox = GetDlgItem(mainwin, IDC_CURCHAN);
							if (cur_chan_textbox != NULL) {
								char name[32];
								strcpy(name, (char *)g_CurrentChannel.name);
								if (strncmp(name, LOB_PREFIX, strlen(LOB_PREFIX)) == 0) {
									int num = atol(name + strlen(LOB_PREFIX));
									int num2 = num / WOL_LOBBY_NUM;
									int num1 = num % WOL_LOBBY_NUM;
									if (num2 == 0) {
										sprintf(name, "%s", g_Lobbies[num1]);
									} else {
										sprintf(name, "%s %d", g_Lobbies[num1], num2 + 1);
									}
								}

								SendMessageA(cur_chan_textbox, WM_SETTEXT, 0, (LPARAM)name);
								SetEvent(g_WaitEventHandles[EV_11]);
								return(S_OK);
							}
						}
					} else {
						WDT_Start_New_Campaign(newstate);
					}

					SetEvent(g_WaitEventHandles[EV_11]);
					return(S_OK);
				} else {
					DebugString("OnWDTState: NACK received from server\n");
					SetEvent(g_WaitEventHandles[EV_11]);
					return(S_OK);
				}
			}

			/*
			 * Bad result code or zero-length packet -- signal the error event instead.
			 */
			SetEvent(g_WaitEventHandles[EV_ERROR]);
			return(S_OK);
		}

		/// <summary>
		/// Called when a game result report has been sent to the server. No-op.
		/// </summary>
		/// <returns>S_OK.</returns>
		STDMETHOD(OnGameresSent)(HRESULT) { return(S_OK); }

		/// <summary>
		/// Processes a ladder rung list from the server: rebuilds the IDC_LADRUNGS list box
		/// (or, if the ladder dialog isn't open, refreshes the user list instead), and caches
		/// each entry into the appropriate per-SKU ladder dictionary (BattleClans, World
		/// Domination, or general).
		/// </summary>
		/// <param name="res">Result code from the server; non-S_OK clears the list.</param>
		/// <param name="list">Head of the linked list of ladder rung entries, or NULL.</param>
		/// <param name="keyRung">Rung of the entry that should end up selected in the list box.</param>
		/// <returns>S_OK.</returns>
		STDMETHOD(OnLadderList)(HRESULT res, Ladder * list, int, long, int keyRung)
		{
			if (res != S_OK) {
				list = NULL;
			}

			if (g_PlayingNetGame) return(S_OK);

			HWND rung_list = GetDlgItem(WS_Find_Dialog(IDD_WOL_LADDER), IDC_LADRUNGS);
			//if (rung_list) SendMessage(rung_list, LB_RESETCONTENT, NULL, NULL);

			if ((rung_list) && ((keyRung == -1) && (res == S_OK))) rung_list = NULL;

			/*
			 * If there's no data at all for the ladder window, show the "not in ladder"
			 * placeholder row instead of an empty list.
			 */
			if (rung_list) {
				SendMessage(rung_list, LB_RESETCONTENT, NULL, NULL);
				if (list == NULL) {
					SendMessage(rung_list, LB_INSERTSTRING, (WPARAM)-1, (LPARAM)((const char *)Fetch_String(TXT_NOT_IN_LADDER)));
					InvalidateRect(rung_list, NULL, false);
					return(S_OK);
				}
			} else if (list == NULL) {
				return(S_OK);
			}

			Ladder * temp = list;
			char list_string[128];
			int top_rung = list->rung;

			Wstring ladder_name;
			OwnerDraw::CellData thecell;

			/*
			 * Cap the appropriate per-SKU ladder dictionary's size before caching new
			 * entries, so it doesn't grow without bound across repeated ladder requests.
			 */
			if (list->sku == (BATTLECLANS | TIBERIAN_SUN_SKU)) {
				if (g_BattleClansLadderDict.getEntries() > 2000) {
					g_BattleClansLadderDict.clear();
				}
			} else if (list->sku == WORLDDOM_SKU) {
				if (g_WDTLadderDict.getEntries() > 2000) {
					g_WDTLadderDict.clear();
				}
			} else {
				if (g_GeneralLadderDict.getEntries() > 2000) {
					g_GeneralLadderDict.clear();
				}
			}

			if (list && rung_list) {
				g_LadderPos = list->rung;
			}

			/*
			 * Walk the rung list, inserting a row per entry (skipping entries with no
			 * recorded points) and caching each entry into its per-SKU dictionary.
			 */
			int row = 0;
			while (temp) {
				if (temp->points == (unsigned int)-1) {
					temp = temp->next;
					continue;
				}

				if (rung_list) {
					thecell.type = OwnerDraw::CellData::PRIMARY;
					sprintf(list_string, "%d", temp->rung);

					SendMessage(rung_list, LB_INSERTSTRING, (WPARAM)-1, (LPARAM)list_string);
					SendMessage(rung_list, OD_SETCELL, MAKEWPARAM(g_Col_Rung, row), (LPARAM)&thecell);

					thecell.type = OwnerDraw::CellData::TEXT;
					thecell.string.set((char *)temp->login_name);
					SendMessage(rung_list, OD_SETCELL, MAKEWPARAM(g_Col_Name, row), (LPARAM)&thecell);

					sprintf(list_string, "%d", temp->points);
					thecell.string.set(list_string);
					SendMessage(rung_list, OD_SETCELL, MAKEWPARAM(g_Col_Points, row), (LPARAM)&thecell);

					sprintf(list_string, "%d", temp->wins);
					thecell.string.set(list_string);
					SendMessage(rung_list, OD_SETCELL, MAKEWPARAM(g_Col_Wins, row), (LPARAM)&thecell);

					sprintf(list_string, "%d", temp->losses);
					thecell.string.set(list_string);
					SendMessage(rung_list, OD_SETCELL, MAKEWPARAM(g_Col_Losses, row), (LPARAM)&thecell);

					sprintf(list_string, "%d", temp->disconnects);
					thecell.string.set(list_string);
					SendMessage(rung_list, OD_SETCELL, MAKEWPARAM(g_Col_Disconnects, row), (LPARAM)&thecell);

					row++;
				}

				ladder_name.set((char *)temp->login_name);
				ladder_name.toLower();

				if (list->sku == (BATTLECLANS | TIBERIAN_SUN_SKU)) {
					g_BattleClansLadderDict.remove(ladder_name);
					g_BattleClansLadderDict.add(ladder_name, *temp);
				} else if (list->sku == WORLDDOM_SKU) {
					g_WDTLadderDict.remove(ladder_name);
					g_WDTLadderDict.add(ladder_name, *temp);
				} else {
					g_GeneralLadderDict.remove(ladder_name);
					g_GeneralLadderDict.add(ladder_name, *temp);
				}

				temp = temp->next;
			}

			if (rung_list) {
				SendMessage(rung_list, LB_SETCURSEL, (WPARAM)keyRung - top_rung, 0);
				InvalidateRect(rung_list, NULL, false);
			} else {
				Draw_Player_List();
			}

			return(S_OK);
		}

		/// <summary>
		/// Handles the result of a new-nickname creation request. On success, stores the
		/// login locally and tells the login dialog to proceed; either way, shows the
		/// server's status message (or a generic fallback) in a message box.
		/// </summary>
		/// <param name="res">Result code from the server; S_OK means the nick was created.</param>
		/// <param name="message">Status message text from the server, or NULL.</param>
		/// <param name="nick">The nickname that was requested.</param>
		/// <param name="pass">The password associated with the nickname.</param>
		/// <returns>S_OK.</returns>
		STDMETHOD(OnNewNick)(HRESULT res, LPCSTR message, LPCSTR nick, LPCSTR pass)
		{
			DebugString("OnNewNick\n");
			g_OnNewNickResult = res;

			if (res == S_OK) {
				StoreLogin(nick, pass, 1, Session.Locale);
				Session.Write_MultiPlayer_Settings();
				HWND dlg = WS_Find_Dialog(IDD_WOL_LOGIN);
				if (dlg) {
					SendMessage(dlg, WOL_LOGIN_OK, 0, 0);
				}
			}

			if (message) {
				ODMessageBox(message, MB_OK, WOL_Wait_Callback);
			} else {
				ODMessageBox(Fetch_String(TXT_UNKNOWN_STATUS), MB_OK, WOL_Wait_Callback);
			}

			Close_Wait_Window(WOL_WAIT_NEW_NICK);
			return(S_OK);
		}

		/// <summary>
		/// Handles the result of an age-check request. On success, stores the reported age
		/// and parental-consent flag for later use.
		/// </summary>
		/// <param name="res">Result code from the server; S_OK means age/consent are valid.</param>
		/// <param name="age">The user's reported age.</param>
		/// <param name="consent">Non-zero if parental consent was given.</param>
		/// <returns>S_OK.</returns>
		STDMETHOD(OnAgeCheck)(HRESULT res, int age, int consent)
		{
			DebugString("OnAgeCheck");
			g_AgeCheckResult = res;
			if (res == S_OK) {
				g_UserAge = age;
				g_UserConsent = consent;
			}
			Close_Wait_Window(WOL_WAIT_NEW_NICK);
			return(S_OK);
		}

		/// <summary>
		/// Handles a ping reply from the server. While in a net game with pings outstanding,
		/// just counts valid replies; otherwise treats the ping as a keep-alive and refreshes
		/// the current channel's user list.
		/// </summary>
		/// <param name="res">Result code from the server; S_OK means the ping time is valid.</param>
		/// <param name="time">Round-trip ping time in milliseconds.</param>
		/// <returns>S_OK.</returns>
		STDMETHOD(OnPing)(HRESULT res, int time, unsigned long, int)
		{
			/// During a net game, pings are used purely to count how many replies came back.
			if (g_PlayingNetGame && g_PingsSent) {
				if (res == S_OK && time >= 0 && time < 1000) {
					g_PingsReceived++;
				}
				return(S_OK);
			}

			/// Otherwise, treat this as a channel keep-alive and refresh the user list.
			if (g_CurrentChannel.type != CHANNELTYPE_CHAT) {
				Draw_Player_List();
			}

			return(S_OK);
		}
};

///////////////////////////////////////////////////////////
// CDownloadEventSink

class CDownloadEventSink : public CComObjectRoot, public IDownloadEvent
{
	public:
		CDownloadEventSink(void) {}
		BEGIN_COM_MAP(CDownloadEventSink)
		COM_INTERFACE_ENTRY(IDownloadEvent)
		END_COM_MAP()

		/// <summary>
		/// Called when a patch/file download finishes. Tells the top window to refresh the
		/// game list.
		/// </summary>
		/// <returns>S_OK.</returns>
		STDMETHOD(OnEnd)(void)
		{
			SendMessage(WS_Top_Window(), WOL_REFRESH_GAMELIST, 0, 0);
			return(S_OK);
		}

		/// <summary>
		/// Called when a patch/file download fails. Tells the top window to show the game
		/// details dialog along with the error code.
		/// </summary>
		/// <param name="error">Error code describing why the download failed.</param>
		/// <returns>S_OK.</returns>
		STDMETHOD(OnError)(int error)
		{
			SendMessage(WS_Top_Window(), WOL_SHOW_GAMEDETAILS, 0, error);
			return(S_OK);
		}

		/// <summary>
		/// Called periodically while a patch/file download is in progress. Updates the
		/// progress bar to reflect bytes read out of the total, and refreshes the status
		/// text (with or without a time-remaining estimate) at most once per second.
		/// </summary>
		/// <param name="bytesread">Number of bytes downloaded so far.</param>
		/// <param name="totalsize">Total size of the download in bytes.</param>
		/// <param name="timeleft">Estimated seconds remaining, or 0/negative if unknown.</param>
		/// <returns>S_OK.</returns>
		STDMETHOD(OnProgressUpdate)(int bytesread, int totalsize, int, int timeleft)
		{
			char stat[200];
			static time_t prev_stat = 0;
			stat[0] = 0;
			SendDlgItemMessage(WS_Top_Window(), IDC_PROGRESS, PBM_SETPOS, (WPARAM)(bytesread * 100) / totalsize, 0);
			if (timeleft > 0) {
				sprintf(stat, (const char *)Fetch_String(TXT_BYTES_W_TIME), bytesread, totalsize, timeleft);
			} else {
				sprintf(stat, (const char *)Fetch_String(TXT_BYTES_WO_TIME), bytesread, totalsize);
			}
			/// Throttle the status text update to at most once per second.
			if ((time(NULL) - prev_stat)) {
				SendDlgItemMessage(WS_Top_Window(), IDC_STATUS, WM_SETTEXT, 0, (LPARAM)stat);
				prev_stat = time(NULL);
			}
			return(S_OK);
		}

		/// <summary>
		/// Called when a patch/file download's status changes. Updates the status text with
		/// a message describing the new status (connecting, finding the patch, etc).
		/// </summary>
		/// <param name="status">The new download status.</param>
		/// <returns>S_OK.</returns>
		STDMETHOD(OnStatusUpdate)(int status)
		{
			char stat[200];
			stat[0] = 0;

			switch (status) {
				case DOWNLOADSTATUS_CONNECTING:
					sprintf(stat, Fetch_String(TXT_CONNECTING));
					break;

				case DOWNLOADSTATUS_FINDINGFILE:
					sprintf(stat, Fetch_String(TXT_FINDING_PATCH));
					break;

				default:
					break;
			}
			SendDlgItemMessage(WS_Top_Window(), IDC_STATUS, WM_SETTEXT, 0, (LPARAM)stat);
			return(S_OK);
		}

		/// <summary>
		/// Called to ask whether an interrupted download should resume.
		/// </summary>
		/// <returns>DOWNLOADEVENT_RESUME to always resume the download.</returns>
		STDMETHOD(OnQueryResume)(void) { return(DOWNLOADEVENT_RESUME); }
};

/// <summary>
/// Decodes an inbound public game-options string into the local session and, if we are a
/// guest waiting on a random-map preview, either requests the preview from the host
/// (throttled by the caller's timer) or clears the "bad map" guest dialog status lines.
/// </summary>
/// <param name="user">The user (channel host) who sent the options.</param>
/// <param name="options">Encoded public game-options string.</param>
/// <param name="_timer">Preview request throttle timer owned by the caller.</param>
/// <returns>Always S_OK.</returns>
static __forceinline HRESULT Handle_Public_Game_Options(struct User * user, const char * options, BasicTimerClass<SystemTimerClass> & _timer)
{
	char *buffer;

	/*
	 * Decode the host's game options into our local session state.
	 */
	buffer = strdup(options);
	bool scenario_changed = DecodePubGameopt(buffer, (char *)user->name);
	free(buffer);

	bool isnew = false;
	if (!Is_Channel_Owner(g_NickName) && !MultiplayerMapPreview) {

		/*
		 * For a random map, throttle preview requests and only ask again if the
		 * timer expired or a new scenario just appeared.
		 */
		if (!stricmp(Session.ScenarioFileName, RANDOM_MAP_FILE_NAME)) {
			char options_str[64];
			if (scenario_changed) {
				isnew = true;
				_timer = 0;
			} else {
				isnew = isnew;
			}
			if (!((_timer > 15 * TIMER_SECOND && (!MultiplayerMapPreview || !MultiplayerMapPreview->Get_Preview_Surface()) && !g_RecievingPreview) || isnew)) {
				return(S_OK);
			}
			sprintf(options_str, "P%s", Session.ScenarioFileName);

			User host;
			g_UserList.getHead(host);
			g_pChat->RequestPrivateGameOptions(&host, options_str);
			DebugString("Sent private game option packet requesting a preview download\n");
			Poke_The_Host();
			return(S_OK);
		}

		/// For a non-random map, check whether the scenario file already exists locally.
		if (Find_Local_Scenario(Session.ScenarioFileName, Session.ScenarioFileLength, Session.ScenarioDigest, Session.ScenarioIsOfficial)) {
			RawFileClass file(Session.ScenarioFileName);
			file.Is_Available();
		}

		/*
		 * Clear the guest dialog's placeholder "bad map" status lines now that
		 * the scenario is available.
		 */
		if (WS_Top_Window_ID() == IDD_WOL_GUEST) {
			HWND statwin = GetDlgItem(WS_Top_Window(), IDC_WOLGUEST_STAT1);
			if (statwin) {
				SendMessage(statwin, WM_SETTEXT, 0, (LPARAM)Fetch_String(TXT_BAD_MAP_01));
			}
			statwin = GetDlgItem(WS_Top_Window(), IDC_WOLGUEST_STAT2);
			if (statwin) {
				SendMessage(statwin, WM_SETTEXT, 0, (LPARAM)Fetch_String(TXT_BAD_MAP_02));
			}
			statwin = GetDlgItem(WS_Top_Window(), IDC_WOLGUEST_STAT3);
			if (statwin) {
				SendMessage(statwin, WM_SETTEXT, 0, (LPARAM)Fetch_String(TXT_BAD_MAP_03));
			}
			statwin = GetDlgItem(WS_Top_Window(), IDC_WOLGUEST_STAT4);
			if (statwin) {
				SendMessage(statwin, WM_SETTEXT, 0, (LPARAM)Fetch_String(TXT_BAD_MAP_04));
			}
			statwin = GetDlgItem(WS_Top_Window(), IDC_WOLGUEST_STAT5);
			if (statwin) {
				SendMessage(statwin, WM_SETTEXT, 0, (LPARAM)Fetch_String(TXT_BAD_MAP_05));
			}
			InvalidateRect(WS_Top_Window(), NULL, FALSE);
		}
	}

	return(S_OK);
}


/// CChatEventSink

class CChatEventSink : public CComObjectRoot, public IChatEvent
{
	public:
		CChatEventSink(void) {}
		BEGIN_COM_MAP(CChatEventSink)
		COM_INTERFACE_ENTRY(IChatEvent)
		END_COM_MAP()

		/// <summary>
		/// Handles the WOL server list callback. On success, walks the linked list of servers,
		/// sorting each entry by its connection label into the chat server list (with SKU-based
		/// random selection among matching chat servers), the ladder server, the primary game
		/// server, and the WDT server, then stamps the current login name and password onto
		/// every stored chat server entry.
		/// </summary>
		/// <param name="res">Result of the server list request; failure aborts login.</param>
		/// <param name="servers">Head of the linked list of servers returned by WOL.</param>
		/// <returns>Always S_OK.</returns>
		STDMETHOD(OnServerList)(HRESULT res, Server * servers)
		{
			Server * tmp = servers;
			Server * server = NULL;
			int chat_server_index = 0;
			int skumatchcount = 0;
			int game_srv_found = 0;
			char namebuf[128];
			char conndata[128];
			char conndata2[128];
			char token[128];

			srand(time(0));

			/// Failed to get a server list at all -- abort the login attempt.
			if (FAILED(res)) {
				SetEvent(g_WaitEventHandles[EV_ABORT]);
				sprintf(g_LastErrorMessage, Fetch_String(TXT_NO_SERV_LIST));
				return(S_OK);
			}

			/*
			 * Walk the server list, sorting each entry into the appropriate global list
			 * by its connection label.
			 */
			while (tmp != NULL) {
				if (strcmp((char *)tmp->connlabel, "IRC") == 0) {
					DebugString("%s", (char *)tmp->connlabel);

					if (chat_server_index == 0) {
						g_Servers.addTail(*tmp);
						g_CurrentServerIndex = 0;
						chat_server_index = 1;
					} else {
						g_Servers.addTail(*tmp);

						/*
						 * Look for a SKU tag matching our game in the comma-separated name
						 * field; if found, randomly favor this server (reservoir sampling)
						 * so all matching servers get an equal chance of being picked.
						 */
						char * nameToken;
						strcpy(namebuf, (char *)tmp->name);
						int sku = -1;
						nameToken = strtok(namebuf, ",");
						while (nameToken != NULL) {
							sku = atol(nameToken);
							if (sku == (unsigned char)g_GameSKU) {
								break;
							}
							if (strchr(nameToken, ':') != NULL) {
								break;
							}
							nameToken = strtok(NULL, ",");
						}

						if (sku == (unsigned char)g_GameSKU) {
							double pick_roll = rand();
							pick_roll /= 32767.0;
							double pick_chance = 1.0 / (double)++skumatchcount;
							if (pick_roll <= pick_chance) g_CurrentServerIndex = chat_server_index;
						}
						g_TargetServerIndex = g_CurrentServerIndex;
						chat_server_index++;
					}
				} else if (strcmp((char *)tmp->connlabel, "LAD") == 0) {

					/*
					 * Ladder server -- parse its host and port out of the connection data.
					 */
					char * host;
					char * port;
					strcpy(token, (char *)tmp->conndata);
					strtok(token, ";");
					host = strtok(NULL, ";");
					if (host != NULL) {
						strcpy(g_LadderServerHost, host);
					}
					port = strtok(NULL, ";");
					if (port != NULL) {
						g_LadderServerPort = atol(port);
					}
				} else if (strcmp((char *)tmp->connlabel, "GAM") == 0 && game_srv_found == 0) {

					/*
					 * Primary game server -- parse its host and port out of the connection data.
					 */
					char * host;
					char * port;
					memcpy(&g_GameServer, tmp, sizeof(g_GameServer));
					strcpy(conndata, (char *)g_GameServer.conndata);
					strtok(conndata, ";");
					host = strtok(NULL, ";");
					if (host != NULL) {
						strcpy(g_GameServerHost, host);
					}
					port = strtok(NULL, ";");
					if (port != NULL) {
						g_GameServerPort = atol(port);
					}
					game_srv_found = 1;
				} else if (strcmp((char *)tmp->connlabel, "WDT") == 0) {

					/*
					 * WDT (World Domination Tour) server -- parse host and port.
					 */
					char * host;
					char * port;
					memcpy(&g_WDTServer, tmp, sizeof(g_WDTServer));
					strcpy(conndata2, (char *)g_WDTServer.conndata);
					strtok(conndata2, ";");
					host = strtok(NULL, ";");
					if (host != NULL) {
						strcpy(g_WDTHost, host);
					}
					port = strtok(NULL, ";");
					if (port != NULL) {
						g_WDTPort = atol(port);
					}
				}
				tmp = tmp->next;
			}

			/*
			 * Stamp the current login name and password onto every stored chat server entry.
			 */
			server = NULL;
			for (int i = 0; i < g_Servers.length(); i++) {
				g_Servers.getPointer(&server, i);
				strcpy((char *)server->login, g_NickName);
				strcpy((char *)server->password, g_LoginPassword);
			}

			SetEvent(g_WaitEventHandles[EV_GOT_SERVERS]);
			return(S_OK);
		}

		/// <summary>
		/// Handles the WOL update-availability callback. If updates are pending, confirms the
		/// upgrade with the user, then downloads each patch file in turn (showing a progress
		/// dialog per file that the user may cancel), and finally prompts for a game restart.
		/// </summary>
		/// <param name="r">Result of the update list request; failure or a null list aborts.</param>
		/// <param name="updates">Head of the linked list of pending patch updates.</param>
		/// <returns>Always S_OK.</returns>
		STDMETHOD(OnUpdateList)(HRESULT r, Update * updates)
		{
			int i = 0;
			Update * tmp = NULL;
			int patch_total = 0;
			char updatestring[200];
			char fullpath[257];
			char localfile[257];
			char working_dir[257];

			/// Request failed, or there is nothing to update -- nothing to do.
			if (FAILED(r) || updates == NULL) {
				// An error occurred, bail out
				SetEvent(g_WaitEventHandles[EV_ABORT]);
				return(S_OK);
			}

			g_SuspendChatPump = TRUE;

			/*
			 * Count the pending updates for the "downloading X of Y" progress text.
			 */
			tmp = updates;
			while (tmp != NULL) {
				tmp = tmp->next;
				patch_total++;
			}

			/*
			 * Confirm the upgrade with the user before downloading anything.
			 */
			if (ODMessageBox(Fetch_String(TXT_UPGRADEREQUIRED), MB_YESNO, WOL_Wait_Callback, true) == IDNO) {
				/*
				 * The download was required and the player declined it, so quit.
				 */
				SetEvent(g_WaitEventHandles[EV_ABORT]);
				return(S_OK);
			}

			GetCurrentDirectory(sizeof(working_dir)-1, working_dir);

			/*
			 * Download each pending patch file in turn, showing a progress dialog that the
			 * user can cancel out of.
			 */
			i = 0;
			while (updates != NULL) {
				// Work out the full file name
				sprintf(fullpath, "%s/%s", updates->patchpath, updates->patchfile);
				sprintf(localfile, "%s\\%s", updates->localpath, updates->patchfile);

				// Create the directory
				CreateDirectory((char *)updates->localpath, NULL);

				g_pDownload->DownloadFile((char *)updates->server, (char *)updates->login, (char *)updates->password, fullpath, localfile, "SOFTWARE\\Westwood\\Tiberian Sun");

				i++;
				sprintf(updatestring, Fetch_String(TXT_DOWNLOADING_X_OF_Y), i, patch_total);

				HWND dlg = WS_Create_Dialog(ProgramInstance, IDD_WOL_DOWNLOAD, MainWindow, (DLGPROC)WOL_Download_Dialog_Proc, FALSE);
				Center_Window_Within_Window(dlg);
				SendDlgItemMessage(dlg, IDC_DOWNLOADTITLE, WM_SETTEXT, 0, (LPARAM)updatestring);
				OwnerDraw::Subclass_Dialog(dlg, 0);
				SendMessage(dlg, OD_SETTOP, 0, 1);
				ShowWindow(dlg, SW_NORMAL);

				if (WS_Wait_Dialog(dlg, WOL_Wait_Callback) == IDCANCEL) {
					// Download failed
					SetEvent(g_WaitEventHandles[EV_ABORT]);
					SetCurrentDirectory(working_dir);
					return(S_OK);
				}
				updates = updates->next;
			}

			/*
			 * All updates installed -- the game needs to restart to pick them up.
			 */
			ODMessageBox(Fetch_String(TXT_GAME_RESTART), 0, WOL_Wait_Callback);
			SetEvent(g_WaitEventHandles[EV_QUITGAME]);
			SetCurrentDirectory(working_dir);

			return(S_OK);
		}

		/// <summary>
		/// Handles the reply to a squad/clan info request. Updates our own squad name if this
		/// reply was for our own pending request, then stamps the squad's abbreviation and name
		/// onto every user in the channel user list that belongs to it, and refreshes the user
		/// list display once all pending squad requests have completed.
		/// </summary>
		/// <param name="res">Result of the squad info request.</param>
		/// <param name="id">ID of the squad this reply describes.</param>
		/// <param name="squad">Squad details (name and abbreviation).</param>
		/// <returns>Always S_OK.</returns>
		STDMETHOD(OnSquadInfo)(HRESULT res, unsigned long id, Squad * squad)
		{
			User * user;

			g_SquadInfoRequests--;

			/// This reply is for our own squad info request.
			if (g_RequestingOwnSquadInfo) {
				g_OwnSquadID = id;
				g_RequestingOwnSquadInfo = 0;
				if (res == S_OK) {
					strcpy((char *)g_OwnSquadName, (char *)squad->abbreviation);
				}
			}

			if (res != S_OK) return(S_OK);

			/// Stamp this squad's info onto every user in the list that belongs to it.
			for (int i = 0; i < g_UserList.length(); i++) {
				user = NULL;
				g_UserList.getPointer(&user, i);
				if (user->squadID == id) {
					strcpy((char *)user->squadabbrev, (char *)squad->abbreviation);
					strcpy((char *)user->squadname, (char *)squad->name);
				}
			}

			if (g_SquadInfoRequests == 0) Draw_Player_List();

			return(S_OK);
		}

		/// <summary>
		/// Handles notification that a user has logged out of WOL, removing them from our
		/// current channel if we were sharing one with them.
		/// </summary>
		/// <param name="r">Result of the logout notification.</param>
		/// <param name="user">The user who logged out.</param>
		/// <returns>Always S_OK.</returns>
		STDMETHOD(OnLogout)(HRESULT r, User * user)
		{
			if (FAILED(r)) {
				return(S_OK);
			}

			Handle_User_Leave(*user);
			return(S_OK);
		}

		/// <summary>Unused server error callback stub; does nothing.</summary>
		/// <returns>Always S_OK.</returns>
		STDMETHOD(OnServerError)(HRESULT, LPCSTR msg) { return(S_OK); }

		/// <summary>
		/// Handles the result of the WOL login/connection attempt. On success, caches every
		/// game type's icon bitmap the first time a connection succeeds, stores the
		/// message-of-the-day if requested, resets the current channel, and signals that login
		/// has completed. On failure, stores a localized error message and signals the abort
		/// event.
		/// </summary>
		/// <param name="r">Result of the connection attempt; selects which case is handled.</param>
		/// <param name="motd">Message-of-the-day text sent by the server on success.</param>
		/// <returns>Always S_OK.</returns>
		STDMETHOD(OnConnection)(HRESULT r, const char * motd)
		{
			char * buffer = NULL;
			char * cptr = NULL;
			static BOOL icons_loaded = FALSE;

			g_OnConnectionResult = r;

			/// A failed connection stops the periodic network-status polling.
			if (FAILED(r)) {
				g_IgnoreNetStatus = true;
			}

			switch (r) {
				case S_OK: {

					/*
					 * First successful connection this session -- fetch and cache the
					 * icon bitmap for every game type.
					 */
					if (icons_loaded == 0) {
						LPCSTR gametype_list;
						g_pChat->GetGametypeList(&gametype_list);
						buffer = new char[strlen(gametype_list) + 32];
						strcpy(buffer, gametype_list);
						strcat(buffer, ",-1,0,18");
						char * cptr = strtok(buffer, ",");
						int gtype = 0;
						while (cptr) {
							gtype = -1;
							gtype = atol(cptr);

							unsigned char * bitmap;
							int bmp_bytes;
							LPCSTR name;
							LPCSTR url;
							g_pChat->GetGametypeInfo(gtype, g_ListIconHeight, &bitmap, &bmp_bytes, &name, &url);
							char bitmap_name[64];
							sprintf(bitmap_name, "gt%d.bmp", gtype);
							SurfaceCache.CacheBMP(bitmap_name, bitmap, bmp_bytes);
							cptr = strtok(NULL, ",");
						}
						delete[] (buffer);
						icons_loaded = TRUE;
					}

					/// Stash the message-of-the-day for later display, if requested.
					if (g_ShowMOTD) {
						g_MessageOfTheDay = strdup(motd);
					}

					/*
					 * Reset the current channel state and mark login as complete.
					 */
					memset(&g_CurrentChannel, 0, sizeof(Channel));
					SetEvent(g_WaitEventHandles[EV_CONNECTED]);
				} break;

				/// Various failure reasons -- store a localized error message and abort.
				case CHAT_E_NICKINUSE:
					strcpy(g_LastErrorMessage, Fetch_String(TXT_LOGIN_USED));
					SetEvent(g_WaitEventHandles[EV_ABORT]);
					break;

				case CHAT_E_BADPASS:
					strcpy(g_LastErrorMessage, Fetch_String(TXT_BADPASS));
					SetEvent(g_WaitEventHandles[EV_ABORT]);
					break;

				case CHAT_E_BANNED:
					strcpy(g_LastErrorMessage, Fetch_String(TXT_BANNED));
					SetEvent(g_WaitEventHandles[EV_ABORT]);
					break;

				case CHAT_E_DISABLED:
					strcpy(g_LastErrorMessage, Fetch_String(TXT_DISABLED));
					SetEvent(g_WaitEventHandles[EV_ABORT]);
					break;

				case CHAT_E_SERIALBANNED:
					strcpy(g_LastErrorMessage, Fetch_String(TXT_BANNED));
					SetEvent(g_WaitEventHandles[EV_ABORT]);
					break;

				case CHAT_E_SERIALDUP:
					strcpy(g_LastErrorMessage, Fetch_String(TXT_SERIALDUP));
					SetEvent(g_WaitEventHandles[EV_ABORT]);
					break;

				case CHAT_E_SERIALUNKNOWN:
					strcpy(g_LastErrorMessage, Fetch_String(TXT_SERIALUNKNOWN));
					SetEvent(g_WaitEventHandles[EV_ABORT]);
					break;

				case CHAT_E_SKUSERIALMISMATCH:
					strcpy(g_LastErrorMessage, Fetch_String(TXT_SKUSERIALMISMATCH));
					SetEvent(g_WaitEventHandles[EV_ABORT]);
					break;

				default:
					strcpy(g_LastErrorMessage, Fetch_String(TXT_CANT_CONNECT));
					SetEvent(g_WaitEventHandles[EV_ABORT]);
					break;
			}

			return(S_OK);
		}

		/// <summary>
		/// Handles the message-of-the-day callback, printing it to the chat window one line
		/// at a time.
		/// </summary>
		/// <param name="r">Result of the request; failure suppresses printing.</param>
		/// <param name="motd">Message-of-the-day text, split into lines by "\n".</param>
		/// <returns>Always S_OK.</returns>
		STDMETHOD(OnMessageOfTheDay)(HRESULT r, const char * motd)
		{
			if (FAILED(r)) {
				return(S_OK);
			}

			/*
			 * Print the message-of-the-day to the chat window one line at a time.
			 */
			char * buffer = strdup(motd);
			char * cptr = strtok(buffer, "\n");
			while (cptr) {
				PMessagePrintf(ColorSystem, cptr);
				cptr = strtok(NULL, "\n");
			}
			free(buffer);
			PMessagePrintf(ColorSystem, "\n");

			return(S_OK);
		}

		/// <summary>
		/// Handles the result of a channel/game creation request. On failure, reports the
		/// error and clears the pending-creation state. On success, records the new channel as
		/// the current channel -- seeding our own player slot in the game options if it is a
		/// game channel -- marks us as the channel's creator/host, and publishes our encoded
		/// extended info string to the channel.
		/// </summary>
		/// <param name="r">Result of the channel creation request.</param>
		/// <param name="channel">The newly created channel, or undefined on failure.</param>
		/// <returns>Always S_OK.</returns>
		STDMETHOD(OnChannelCreate)(HRESULT r, struct Channel * channel)
		{
			/// Creation failed -- report it and clear the pending-creation state.
			if (FAILED(r)) {
				PMessagePrintf(ColorSystem, Fetch_String(TXT_CHANCREATE_FAILED));
				g_IsChannelCreator = 0;
				g_CurrentChannel.name[0] = '\0';
				Close_Wait_Window(WOL_WAIT_CHANNEL_CREATE);
				return(S_OK);
			}

			/*
			 * A plain chat channel just needs recording; a game channel also needs our own
			 * player slot in the game options seeded.
			 */
			if (channel->type == CHANNELTYPE_CHAT) {
				PMessagePrintf(ColorSystem, Fetch_String(TXT_CREATED_CHAN), channel->name);
				memcpy(&g_CurrentChannel, channel, sizeof(Channel));
			} else {
				memset(g_UserInfo, 0, sizeof(g_UserInfo));
				strcpy(g_UserInfo[0].name, g_NickName);
				g_UserInfo[0].house = Session.House;
				g_UserInfo[0].color = Session.ColorIdx;
				g_UserInfo[0].accepted = 1;

				memcpy(&g_CurrentChannel, channel, sizeof(Channel));
			}

			g_IsChannelCreator = 1;
			g_ChannelCount++;

			/// Publish our encoded extended info (house, color, etc.) to the channel.
			if (g_pChat && channel->type != CHANNELTYPE_CHAT) {
				char exinfo_out[80];
				memset(exinfo_out, 0, sizeof(exinfo_out));
				Encode_Channel_ExInfo(exinfo_out);
				exinfo_out[(sizeof(exinfo_out) / 2)] = 0;
				g_pChat->SetChannelExInfo(exinfo_out);
			}

			Close_Wait_Window(WOL_WAIT_CHANNEL_CREATE);

			return(S_OK);
		}

		/// <summary>
		/// Handles the result of joining a channel (either our own join or another user's).
		/// On failure, reports why, cleans up any now-stale cached channel entries, and backs
		/// the UI out to the previous screen. On success, either finishes setting up our own
		/// join (showing the guest dialog for a game channel) or, for another user joining,
		/// plays a sound and queues squad/locale/ladder lookups for them; either way the user
		/// is inserted into the sorted channel user list and, for game channels, given a game
		/// options slot (assigning a free color if we are the channel's creator).
		/// </summary>
		/// <param name="r">Result of the join request.</param>
		/// <param name="channel">The channel being joined, or NULL/undefined on some failures.</param>
		/// <param name="user">The user who joined (ourselves or another player).</param>
		/// <returns>Always S_OK.</returns>
		STDMETHOD(OnChannelJoin)(HRESULT r, struct Channel * channel, struct User * user)
		{
			/// A status error means the join is still in progress -- nothing to do yet.
			if (r == CHAT_E_STATUSERROR) {
				return(S_OK);
			}

			int i;
			int players;
			char joinname[64];
			joinname[0] = '\0';

			/*
			 * Resolve the channel's display name, translating internal lobby channel names
			 * ("Lob_18_N") into their friendly lobby name and page number.
			 */
			if (channel) {
				strcpy(joinname, (char *)channel->name);
				if (strncmp(joinname, LOB_PREFIX, sizeof(LOB_PREFIX) - 1) == 0) {
					int d;
					int m;
					g_CurrentLobby = atol(joinname + strlen(LOB_PREFIX));
					d = g_CurrentLobby / WOL_LOBBY_NUM;
					m = g_CurrentLobby % WOL_LOBBY_NUM;
					if (d == 0) {
						sprintf(joinname, "%s", g_Lobbies[m]);
					} else {
						sprintf(joinname, "%s %d", g_Lobbies[m], d + 1);
					}
				}
			}

			/*
			 * Join failed -- report why, clean up any now-stale channel-list entries for a
			 * game that has since closed, and back the UI out to the previous screen.
			 */
			if (FAILED(r)) {
				if (r == CHAT_E_CHANNELDOESNOTEXIST) {
					Channel* chan = NULL;

					/// Remove the now-invalid channel from our cached channel lists.
					for (i = 0; i < g_UserChannels.length(); i++) {
						if (g_UserChannels.getPointer(&chan, i) && (strcmp((char *)s_TempChannel.name, (char *)chan->name) == 0)) {
							g_UserChannels.remove(i);
							break;
						}
					}

					for (i = 0; i < g_GameChannelList.length(); i++) {
						if (g_GameChannelList.getPointer(&chan, i) && (strcmp((char *)s_TempChannel.name, (char *)chan->name) == 0)) {
							g_GameChannelList.remove(i);
							break;
						}
					}

					ODMessageBox(Fetch_String(TXT_GAME_CLOSED), 0, WOL_Wait_Callback, 0);
				} else if (r == CHAT_E_BADCHANNELPASSWORD) {
					ODMessageBox(Fetch_String(TXT_BADPASS), 0, WOL_Wait_Callback, 0);
				} else if (r == CHAT_E_CHANNELFULL) {
					ODMessageBox(Fetch_String(TXT_CHANNEL_FULL), 0, WOL_Wait_Callback, 0);
				} else if (r == CHAT_E_BANNED) {
					ODMessageBox(Fetch_String(TXT_JOINBAN), 0, WOL_Wait_Callback, 0);
				} else {
					PMessagePrintf(ColorSystem, Fetch_String(TXT_CANT_JOINCHAN), r);
				}

				g_IsChannelCreator = 0;
				g_CurrentChannel.name[0] = '\0';
				Close_Wait_Window(WOL_WAIT_CHANNEL_JOIN);

				if (CurrentLevel == WOL_LEVEL_BACK_LOBBIES) {
					g_JoinLobbyNow = 1;
				}

				GoBack();
				Draw_Channel_List();
				Draw_Player_List();
				SetEvent(g_WaitEventHandles[EV_CHAN_JOIN_FAIL]);
				return(S_OK);
			}

			/// This is our own join confirmation.
			if (user->flags & CHAT_USER_MYSELF) {

				memcpy(&g_CurrentChannel, channel, sizeof(Channel));
				Close_Wait_Window(WOL_WAIT_CHANNEL_JOIN);

				g_ChannelCount++;

				/*
				 * A game channel shows the guest waiting dialog; a plain chat channel just
				 * prints a "joined" message.
				 */
				if (channel->type == CHANNELTYPE_GAME) {
					HWND dlg = WS_Create_Dialog(ProgramInstance, IDD_WOL_GUEST, MainWindow, (DLGPROC)WOL_Guest_Dialog_Proc, FALSE);
					Center_Window_Within_Window(dlg);
					OwnerDraw::Subclass_Dialog(dlg, 0);

					HWND main = WS_Find_Dialog(IDD_WOL_MAIN);
					if (main) {
						ShowWindow(main, SW_HIDE);
					}
					SendMessage(dlg, OD_SETTOP, 0, 1);
					ShowWindow(dlg, SW_NORMAL);
				} else {
					PMessagePrintf(ColorSystem, Fetch_String(TXT_JOINED_S), joinname);
				}
			} else {

				/// Someone else joined our game channel -- announce it with a sound effect.
				if (g_CurrentChannel.type == CHANNELTYPE_GAME) {
					Sound_Effect(Rule->PlayerJoined, 1.0, 0);
				}

				/// Request their squad info if we don't already have their abbreviation.
				if ((user->squadID != 0) && (strlen((char *)user->squadabbrev) == 0)) {
					g_SquadInfoRequests++;
					g_pChat->RequestSquadInfo(user->squadID);
				}

				Wstring ladder_name;
				ladder_name.set((char *)user->name);
				ladder_name.toLower();

				/// Queue this user for a WDT team lookup if we haven't seen them before.
				if (Session.IsWDT) {
					Wstring name;
					bool found = false;
					for (i = 0; i < g_WDTTeamLookupQueue.length(); i++) {
						if (g_WDTTeamLookupQueue.get(name, i) && (name == ladder_name)) {
							found = true;
							break;
						}
					}
					if (!found) {
						g_WDTTeamLookupQueue.addHead(ladder_name);
					}
				}

				/// Queue this user for a locale lookup if we don't already have it cached.
				if (!g_UserLocales.contains(ladder_name)) {
					Wstring name;
					bool found = false;
					for (i = 0; i < g_UserLocaleLookupQueue.length(); i++) {
						if (g_UserLocaleLookupQueue.get(name, i) && (name == ladder_name)) {
							found = true;
							break;
						}
					}
					if (!found) {
						g_UserLocaleLookupQueue.addHead(ladder_name);
					}
				}

				/// Request their ladder info if we don't already have it cached.
				if (!g_ActiveLadder->contains(ladder_name)) {
					g_pNetUtil->RequestLadderList(g_LadderServerHost, g_LadderServerPort, ladder_name.get(), LADDER_CODE(g_InstalledLadderSKU), -1, 0, 0);
				}

				/// A new player joining a game channel resets everyone's "accepted" state.
				if (channel->type != CHANNELTYPE_CHAT) {
					for (i = 1; i < (sizeof(g_UserInfo) / sizeof(ChannelUserInfo)); i++) {
						g_UserInfo[i].accepted = 0;
						EnableWindow(GetDlgItem(GameoptWindow(), IDC_ACCEPT), TRUE);
					}
				}
			}

			/// Insert the user into the sorted channel user list.
			int index = g_UserList.length();
			User temp_user;
			for (i = 0; i < g_UserList.length(); i++) {
				g_UserList.get(temp_user, i);
				if (Compare_Users(user, &temp_user) < 0) {
					index = i;
					break;
				}
			}
			g_UserList.add(*user, index);

			players = g_UserList.length();

			/*
			 * For a game channel, find this user's slot in the parallel per-user game
			 * options info array (making room for it and seeding the name).
			 */
			int userinfoindex = 0;
			if (channel->type != CHANNELTYPE_CHAT) {
				userinfoindex = Player_Name_To_Index((char *)user->name);
				memmove(&g_UserInfo[userinfoindex + 1], g_UserInfo + userinfoindex, (sizeof(ChannelUserInfo)) * (MAX_USER_COUNT - userinfoindex - 1));
				memset(&g_UserInfo[userinfoindex], 0, sizeof(ChannelUserInfo));
				strcpy((char *)g_UserInfo[userinfoindex].name, (char *)user->name);
			}

			/*
			 * As the channel's creator, assign the new player the lowest color index not
			 * already taken by another player.
			 */
			if ((channel->type != CHANNELTYPE_CHAT) && (g_IsChannelCreator == 1)) {
				int color = 0;
				int taken = 0;

				while(true) {
					g_UserInfo[userinfoindex].color = color;
					taken = 0;
					for (i = 0; i < players; i++) {
						if (i != userinfoindex && g_UserInfo[i].color == color) {
							color++;
							taken = 1;
						}
					}
					if (taken==0) {
						break;
					}
				}
				PumpGameopts(TRUE, FALSE);
			}

			SetEvent(g_WaitEventHandles[EV_CHAN_JOINED]);

			Draw_Player_List();
			if (user->flags & CHAT_USER_MYSELF) {
				Draw_Channel_List();
			}

			return(S_OK);
		}

		/// <summary>
		/// Handles notification that a user has left the current channel, removing them from
		/// our channel user list.
		/// </summary>
		/// <param name="r">Result of the leave notification.</param>
		/// <param name="user">The user who left the channel.</param>
		/// <returns>Always S_OK.</returns>
		STDMETHOD(OnChannelLeave)(HRESULT r, struct Channel *, struct User * user)
		{
			if (FAILED(r)) {
				return(S_OK);
			}

			Handle_User_Leave(*user);

			return(S_OK);
		}

		/// <summary>
		/// Handles (re)delivery of a channel's user list, rebuilding the channel user list box
		/// and our internal user list from the linked list of users. For each user, queues any
		/// needed squad/locale/ladder lookups and, for a game channel, seeds their game-options
		/// info slot. Afterwards refreshes the user list display and leaves the channel if it
		/// no longer has an owner, or if we are the last user remaining in it.
		/// </summary>
		/// <param name="r">Result of the user list request; failure aborts.</param>
		/// <param name="channel">The channel this user list belongs to (unused here).</param>
		/// <param name="user">Head of the linked list of users currently in the channel.</param>
		/// <returns>Always S_OK.</returns>
		STDMETHOD(OnUserList)(HRESULT r, struct Channel * channel, struct User * user)
		{
			channel = channel;

			/*
			 * Already in a network game -- this lobby user-list refresh doesn't apply.
			 */
			if (g_PlayingNetGame) return(S_OK);

			char buffer[64];

			static char ladder_request_keys[512];
			memset(ladder_request_keys, 0, sizeof(ladder_request_keys));
			Wstring ladder_name;

			if (FAILED(r)) return(S_OK);

			/*
			 * Clear the existing list box and internal user list before rebuilding them.
			 */
			SendDlgItemMessage(WS_Top_Window(), IDC_USERS, LB_RESETCONTENT, 0, 0);
			g_UserList.clear();

			unsigned int flags;
			int index;

			int batch_count = 0;
			int user_count = 0;

			/*
			 * Walk the linked list of users, rebuilding our internal user list and queuing
			 * any squad/locale/ladder lookups still needed for each one.
			 */
			bool saw_owner = false;
			while (user != NULL) {
				user_count++;

				/// Request squad info if we don't already have this user's abbreviation.
				if ((user->squadID != 0) && (strlen((char *)user->squadabbrev) == 0)) {

					g_SquadInfoRequests++;
					g_pChat->RequestSquadInfo(user->squadID);
				}

				flags = user->flags;
				strcpy(buffer, (char *)user->name);
				g_UserList.addTail(*user);

				if (flags & CHAT_USER_CHANNELOWNER) saw_owner = true;

				ladder_name.set((char *)user->name);
				ladder_name.toLower();

				/// Queue this user for a WDT team lookup if we haven't seen them before.
				if (Session.IsWDT) {
					if (!g_WDTTeams.contains(ladder_name)) {
						g_WDTTeamLookupQueue.addHead(ladder_name);
					}
				}

				/// Queue this user for a locale lookup if we don't already have it cached.
				if (!g_UserLocales.contains(ladder_name)) {
					g_UserLocaleLookupQueue.addHead(ladder_name);
				}

				/*
				 * Batch this user into the pending ladder info request, up to a reasonable
				 * per-request user count.
				 */
				if ((g_ActiveLadder->contains(ladder_name) == FALSE) && (user_count < 100)) {
					ladder_name.set((char *)user->name);
					ladder_name.toLower();
					strcat(ladder_request_keys, ladder_name.get());
					strcat(ladder_request_keys, ":");
					batch_count++;
				}

				/// Flush the batched ladder request once it gets large enough.
				if (batch_count > 20) {
					g_pNetUtil->RequestLadderList(g_LadderServerHost, g_LadderServerPort, ladder_request_keys, LADDER_CODE(g_InstalledLadderSKU), -1, 0, 0);
					batch_count = 0;
					memset(ladder_request_keys, 0, sizeof(ladder_request_keys));
				}

				/// As a non-creator in a game channel, seed this user's game-options info slot.
				if ((g_CurrentChannel.type != CHANNELTYPE_CHAT) && !g_IsChannelCreator) {
					index = Player_Name_To_Index((char *)user->name);
					memset(&g_UserInfo[index], 0, sizeof(ChannelUserInfo));
					strcpy((char *)g_UserInfo[index].name, (char *)user->name);
				}

				user = user->next;
			}

			/// Flush any remaining batched ladder request.
			if (strlen(ladder_request_keys)) {
				g_pNetUtil->RequestLadderList(g_LadderServerHost, g_LadderServerPort, ladder_request_keys, LADDER_CODE(g_InstalledLadderSKU), -1, 0, 0);
			}

			Draw_Player_List();

			if (g_CurrentChannel.type != CHANNELTYPE_CHAT) {

				/*
				 * Nobody in the list is flagged as the channel owner -- the channel is
				 * effectively gone, so leave it.
				 */
				if (saw_owner == false) {
					User user;
					for (int i = 0; i < g_UserList.length(); i++) {
						if ((g_UserList.get(user, i)) && (stricmp((char *)user.name, g_NickName) == 0)) {
							Handle_User_Leave(user);
							break;
						}
					}
				}

				if (g_CurrentChannel.type != CHANNELTYPE_CHAT) {

					/// We are the last (non-creator) user left in the channel -- leave it.
					if (!g_IsChannelCreator && g_UserList.length() == 1 && (stricmp((char *)user->name, g_NickName) == 0)) {
						Handle_User_Leave(*user);
					}

					if (g_CurrentChannel.type != CHANNELTYPE_CHAT) {
						/*
						 * Force the side/house combo box to refresh now that the user list
						 * has changed.
						 */
						SendMessage(GameoptWindow(), WM_COMMAND, MAKEWPARAM(IDC_YOURSIDE, CBN_SELCHANGE), (LPARAM)GetDlgItem(GameoptWindow(), IDC_YOURSIDE));
						IsColorChangePending = 1;
					}
				}
			}

			return(S_OK);
		}

		/// <summary>
		/// Handles the WOL "game start" notification. Stores the game/session identifiers used
		/// for stats reporting, builds the local session player list from the channel's users,
		/// derives frame-sync timing (MaxAhead, frame rate, latency fudge) from the selected
		/// game speed and the players' ping times, builds clan/team player-name strings for
		/// tournament and World Domination Tour games, then signals the game-start event and
		/// begins the pregame setup.
		/// </summary>
		/// <param name="r">Result code from the server (unused).</param>
		/// <param name="channel">The game channel the match is starting in.</param>
		/// <param name="users">Head of the linked list of users playing in the game.</param>
		/// <param name="gameid">Unique identifier assigned to this game by the server.</param>
		/// <returns>Always S_OK.</returns>
		STDMETHOD(OnGameStart)(HRESULT r, struct Channel * channel, struct User * users, int gameid)
		{
			int i;

			/*
			 * Stamp the game/SKU identifiers and login/user names used by the stats
			 * and replay reporting system.
			 */
			WestwoodOnline_GameID = gameid;
			WestwoodOnline_GameSKU_TS = g_GameSKU;
			WestwoodOnline_GameSKU_FS = FIRESTORM_SKU;
			WestwoodOnline_GameSKU_WDT = WORLDDOM_SKU;
			WestwoodOnline_Tournament = g_CurrentChannel.tournament;

			if (g_CurrentChannel.reserved & 0x100) {
				WestwoodOnline_Tournament = 2;
			}
			strcpy(WestwoodOnline_LoginName, g_NickName);
			User *user = NULL;
			if (g_UserList.getPointer(&user, 0) && user) {
				strcpy(WestwoodOnline_UserName, (char *)user->name);
			}
			WestwoodOnline_StartTime = time(0);

			DebugString("OnGameStart\n");

			/*
			 * Record which lobby channel this game was launched from, then build the
			 * session's player list from the channel's user list.
			 */
			g_CurrentLobby = (channel->reserved & 0xFF);

			Fill_Session_Players(users);

			g_GameStarted = true;

			DebugString("Starting a %d player game.\n", Session.NumPlayers);

			strcpy(Session.MasterPlayerName, (char *)Get_Channel_Host().name);
			Session.MasterPlayerID = -1;

			Sync_Scenario_With_Guests();

			if (!Session.IsWDT) {
				Session.House = HOUSE_GOOD;
			}

			/*
			 * Derive the frame-ahead/frame-rate/send-rate parameters from the selected
			 * game speed setting.
			 */
			Session.FrameSendRate = 5;

			switch (Session.Options.GameSpeed) {
				case 0:
					Session.MaxAhead = 40;
					Session.PrecalcDesiredFrameRate = 60;
					Session.FrameSendRate = 10;
					Session.PrecalcMaxAhead = 0;
					break;

				case 1:
					Session.MaxAhead = 40;
					Session.PrecalcDesiredFrameRate = 45;
					Session.FrameSendRate = 10;
					Session.PrecalcMaxAhead = 0;
					break;

				case 2:
					Session.MaxAhead = 30;
					Session.PrecalcDesiredFrameRate = 30;
					Session.PrecalcMaxAhead = 0;
					break;

				case 3:
					Session.MaxAhead = 20;
					Session.PrecalcDesiredFrameRate = 20;
					Session.PrecalcMaxAhead = 0;
					break;

				case 4:
					Session.MaxAhead = 20;
					Session.PrecalcDesiredFrameRate = 15;
					Session.PrecalcMaxAhead = 0;
					break;

				case 5:
					Session.MaxAhead = 20;
					Session.PrecalcDesiredFrameRate = 12;
					Session.PrecalcMaxAhead = 0;
					break;

				default:
					Session.MaxAhead = 10;
					Session.PrecalcDesiredFrameRate = 10;
					Session.PrecalcMaxAhead = 0;
					break;
			}

			if (!Is_Channel_Owner(g_NickName)) {
				Session.PrecalcDesiredFrameRate = 0;
			}
			/*
			 * Find the worst (highest) ping time among all pinged users.
			 */
			int worst_ping = 0;
			for (i = 0; i < ARRAY_SIZE(WestwoodOnline_PingTimes); i++) {
				if (WestwoodOnline_PingTimes[i] > worst_ping) {
					worst_ping = WestwoodOnline_PingTimes[i];
				}
			}

			/*
			 * Turn the worst observed ping into a latency fudge factor and, if we are
			 * hosting, a precalculated MaxAhead value; also tunes the IPX connection's
			 * ack/retry timing.
			 */
			Session.LatencyFudge = 2;
			if (worst_ping != 0) {
				DebugString("Max ping is %d ms\n", worst_ping);
				if (worst_ping >= 1000) {
					Session.LatencyFudge = 3;
				} else if (worst_ping < 600) {
					Session.LatencyFudge = 1;
				}
				unsigned int timing = 120 * (TIMER_SECOND * worst_ping / 1000) / 100 + (TIMER_SECOND * worst_ping / 1000);
				Ipx.Set_Timing(timing, (unsigned int)-1, timing * 8);
				if (Is_Channel_Owner(g_NickName)) {
					switch (Session.LatencyFudge) {
						case 0:
						case 1:
							Session.PrecalcMaxAhead = (unsigned)Session.MaxAhead;
							break;
						case 2:
							Session.PrecalcMaxAhead = ((unsigned)Session.MaxAhead / 2) + Session.MaxAhead;
							break;
						case 3:
							Session.PrecalcMaxAhead = (unsigned)Session.MaxAhead * 2;
							break;
						default:
							break;
					}
					Session.PrecalcMaxAhead = 10 * ((unsigned)Session.PrecalcMaxAhead / 10);
				}
			} else {
				Session.PrecalcDesiredFrameRate = 0;
				DebugString("Ping times not available\n");
			}

			DebugString("PrecalcMaxAhead is %d\n", Session.PrecalcMaxAhead);
			DebugString("PrecalcDesiredFrameRate is %d\n", Session.PrecalcDesiredFrameRate);
			DebugString("LatencyFudge is %d\n", Session.LatencyFudge);
			DebugString("FrameSendRate is %d\n", Session.FrameSendRate);

			Session.MaxMaxAhead = Session.MaxAhead;
			memset(Session.ConnectionStats, 0, sizeof(Session.ConnectionStats));
			Options.GameSpeed = Session.Options.GameSpeed;
			WestwoodOnline_PortNumber = 1234;
			Frame = 0;

			WestwoodOnline_Clan1_Players[0] = 0;
			WestwoodOnline_Clan2_Players[0] = 0;

			/*
			 * Build comma-separated clan/team player name lists for tournament games
			 * (split by squad ID) and World Domination Tour games (split by house).
			 */
			if ((g_CurrentChannel.reserved & 0x100) != 0) {
				User *user = NULL;
				int squad1 = 0;
				int squad2 = 0;
				for (i = 0; i < g_UserList.length(); i++) {
					g_UserList.getPointer(&user, i);
					if (squad1 == 0 || squad1 == (int)user->squadID) {
						strcat(WestwoodOnline_Clan1_Players, (const char *)user->name);
						strcat(WestwoodOnline_Clan1_Players, ",");
						squad1 = user->squadID;
					} else if (squad2 == 0 || squad2 == (int)user->squadID) {
						strcat(WestwoodOnline_Clan2_Players, (const char *)user->name);
						strcat(WestwoodOnline_Clan2_Players, ",");
						squad2 = user->squadID;
					}
				}
			}

			if (Session.IsWDT) {
				User *user = NULL;
				for (i = 0; i < g_UserList.length(); i++) {
					g_UserList.getPointer(&user, i);

					if (g_UserInfo[i].house == HOUSE_GOOD) {
						strcat(WestwoodOnline_Clan1_Players, (const char *)user->name);
						strcat(WestwoodOnline_Clan1_Players, ",");
					} else {
						strcat(WestwoodOnline_Clan2_Players, (const char *)user->name);
						strcat(WestwoodOnline_Clan2_Players, ",");
					}
				}
			}

			/*
			 * Signal that the game has started and hand off to pregame setup.
			 */
			SetEvent(g_WaitEventHandles[EV_STARTGAME]);

			g_pChat->RequestChannelLeave();

			PregameSetup();

			return(S_OK);
		}

		/// <summary>
		/// Handles an inbound public game-options string broadcast by the channel host.
		/// Decodes the options into the local session, and if we are a guest waiting on a
		/// random-map preview, either requests the preview from the host (throttled by a
		/// timer) or, once the scenario is available locally, clears the "bad map" guest
		/// dialog status lines.
		/// </summary>
		/// <param name="r">Result code from the server; failure or an in-progress game aborts.</param>
		/// <param name="user">The user (channel host) who sent the options.</param>
		/// <param name="options">Encoded public game-options string.</param>
		/// <returns>Always S_OK.</returns>
		STDMETHOD(OnPublicGameOptions)(HRESULT r, struct Channel *, struct User * user, const char * options)
		{
			static BasicTimerClass<SystemTimerClass> _timer;

			if (!g_PlayingNetGame && !g_GameStarted && SUCCEEDED(r)) {
				return(Handle_Public_Game_Options(user, options, _timer));
			}

			return(S_OK);
		}

		/// <summary>
		/// Handles a private game-options string sent to us by a channel guest. Only the
		/// channel creator/host acts on it: a 'P' request asks for a map preview to be sent to
		/// the guest, while an 'R' request applies the guest's requested house and color
		/// (reassigning the color if it collides with another player's) and republishes the
		/// game options.
		/// </summary>
		/// <param name="user">The guest who sent the private options.</param>
		/// <param name="options">Encoded private options string; the first character selects
		/// the request type ('P' = preview request, 'R' = house/color request).</param>
		/// <returns>Always S_OK.</returns>
		STDMETHOD(OnPrivateGameOptions)(HRESULT, struct User * user, const char * options)
		{
			if (g_PlayingNetGame) return(S_OK);

			if (g_GameStarted) return(S_OK);

			if (g_IsChannelCreator == 1) {
				int index = Player_Name_To_Index((char *)user->name);
				if (index == -1) return(S_OK);

				/// Dispatch on the request type encoded in the first character.
				switch (options[0]) {

					case 'P': {
						DebugString("Received private preview request from channel guest\n");
						if (g_UserList.length() >= 2) {
							User temp;

							/*
							 * Only honor the request if we are genuinely hosting a
							 * random-map game with a preview ready to send.
							 */
							if (strcmpi((char *)Get_Channel_Host().name, g_NickName) == 0 && !g_GameStarted && g_CurrentChannel.type != CHANNELTYPE_CHAT && MultiplayerMapPreview != NULL &&
								MultiplayerMapPreview->Get_Preview_Surface() != NULL && IsRandomMap) {
								int i = Player_Name_To_Index((char *)user->name);
								if (i != -1) {
									g_UserList.get(temp, i);
									temp.next = NULL;
									Fill_Session_Players(&temp);
									int old = g_SuspendChatPump;
									g_SuspendChatPump = true;
									Send_Preview_To_Guests();
									g_SuspendChatPump = old;
								}
							}
						}
						break;
					}

					case 'R': {
						char * buffer = strdup(options + 1);

						char * cptr = strtok(buffer, ",");
						if (cptr) {
							int requested_house = atol(cptr);
							g_UserInfo[index].house = requested_house;
						}

						int taken = 0;
						int i;

						cptr = strtok(NULL, ",");
						if (cptr) {
							int reqcolor = atol(cptr);

							/*
							 * Find a free color, bumping to the next one (wrapping at 8)
							 * whenever it collides with another player's chosen color.
							 */
							while (1) {
								g_UserInfo[index].color = reqcolor;
								taken = 0;
								for (i = 0; i < g_UserList.length(); i++) {
									if (i == index) continue;
									if (g_UserInfo[i].color == reqcolor) {
										reqcolor++;
										taken = 1;
									}
								}
								if (taken == 0) break;
								reqcolor %= MAX_MPLAYER_COLORS;
							}

							g_UserInfo[index].color = reqcolor;
							PumpGameopts(TRUE);

							Draw_Player_List();
						}
						free(buffer);
						break;
					}
				}
			}
			return(S_OK);
		}

		/// <summary>
		/// Prints a public chat message received in the current channel to the chat window,
		/// coloring the sender's name/text according to whether they are the channel owner.
		/// </summary>
		/// <param name="r">Result code from the server; failure suppresses printing.</param>
		/// <param name="user">The user who sent the message.</param>
		/// <param name="text">Text of the message.</param>
		/// <returns>Always S_OK.</returns>
		STDMETHOD(OnPublicMessage)(HRESULT r, struct Channel *, struct User * user, const char * text)
		{
			if (g_PlayingNetGame) return(S_OK);

			if (FAILED(r)) {
				return(S_OK);
			}

			/*
			 * Channel owners get a distinct highlight color.
			 */
			int color = ColorUser;
			if (user->flags & CHAT_USER_CHANNELOWNER) color = ColorOp;

			PMessagePrintf(color, "[%s] : %s", user->name, text);
			return(S_OK);
		}

		/// <summary>
		/// Prints a private (whispered) chat message from another user to the chat window,
		/// coloring the sender's name/text according to whether they are the channel owner.
		/// </summary>
		/// <param name="r">Result code from the server; failure suppresses printing.</param>
		/// <param name="user">The user who sent the message.</param>
		/// <param name="text">Text of the message.</param>
		/// <returns>Always S_OK.</returns>
		STDMETHOD(OnPrivateMessage)(HRESULT r, struct User * user, const char * text)
		{
			if (g_PlayingNetGame) return(S_OK);

			if (FAILED(r)) {
				return(S_OK);
			}

			/*
			 * Channel owners get a distinct highlight color.
			 */
			int color = ColorPriv;
			if (user->flags & CHAT_USER_CHANNELOWNER) color = ColorOp;

			PMessagePrintf(color, "%s : %s", user->name, text);
			return(S_OK);
		}

		/// <summary>
		/// Handles an incoming WOL page (whisper-style notification). While in a net game,
		/// queues it as an in-game HUD message in addition to printing it to the observer chat
		/// window; otherwise prints it to both the public and system chat windows. Either way,
		/// plays the incoming-message sound.
		/// </summary>
		/// <param name="r">Result code from the server; failure suppresses handling.</param>
		/// <param name="user">The user who sent the page.</param>
		/// <param name="text">Text of the page.</param>
		/// <returns>Always S_OK.</returns>
		STDMETHOD(OnPaged)(HRESULT r, struct User * user, LPCSTR text)
		{
			if (FAILED(r)) {
				return(S_OK);
			}

			if (g_PlayingNetGame) {
				char msg[256];
				sprintf(msg, "(%s) %s", user->name, text);
				int color = GadgetClass::Get_Color_Scheme();
				Session.Messages.Add_Message(NULL, 0, msg, color, TPF_TEXT, int(Rule->MessageDelay * TICKS_PER_MINUTE));
				Map.Flag_To_Redraw();
				SMessagePrintf(ColorPaged, "(%s) %s", user->name, text);
			} else {
				PMessagePrintf(ColorPaged, "(%s) %s", user->name, text);
				SMessagePrintf(ColorPaged, "(%s) %s", user->name, text);
			}
			Sound_Effect(Rule->IncomingMessage);
			return(S_OK);
		}

		/// <summary>
		/// Reports the outcome of a page (whisper notification) send attempt to the system
		/// chat window.
		/// </summary>
		/// <param name="r">Result of the page send: success, paging disabled, target user gone,
		/// or a general failure.</param>
		/// <returns>Always S_OK.</returns>
		STDMETHOD(OnPageSend)(HRESULT r)
		{
			if (r == S_OK) {
				SMessagePrintf(ColorSystem, Fetch_String(TXT_PAGESENT));
			} else if (r == CHAT_S_PAGE_OFF) {
				SMessagePrintf(ColorSystem, Fetch_String(TXT_PAGEDISABLED));
			} else if (r == CHAT_S_PAGE_NOTHERE) {
				SMessagePrintf(ColorSystem, Fetch_String(TXT_PAGEUSERGONE));
			} else {
				SMessagePrintf(ColorSystem, Fetch_String(TXT_PAGEFAILED));
			}
			return(S_OK);
		}

		/// <summary>
		/// Handles the reply to a channel "find" request. On success, translates a numbered
		/// lobby channel name ("Lob_18_N") back into its localized lobby name (with a
		/// duplicate-lobby number suffix if needed) and reports where the searched-for user
		/// was found; otherwise reports why the find failed.
		/// </summary>
		/// <param name="r">Result of the find request; selects the failure message when no
		/// channel was found.</param>
		/// <param name="chan">The channel the searched-for user is in, or NULL on failure.</param>
		/// <returns>Always S_OK.</returns>
		STDMETHOD(OnFind)(HRESULT r, Channel * chan)
		{
			if (g_PlayingNetGame) {
				return(S_OK);
			}

			char name[64];
			name[0] = 0;
			if (chan) {
				strcpy(name, (char *)chan->name);

				/*
				 * Translate a numbered lobby channel name back into its localized
				 * display name.
				 */
				if (strncmp(name, LOB_PREFIX, sizeof(LOB_PREFIX)-1) == 0) {
					int v = atol(((char *)name) + strlen(LOB_PREFIX));
					int d = v / WOL_LOBBY_NUM;
					int m = v % WOL_LOBBY_NUM;
					if (d == 0) {
						sprintf(name, "%s", g_Lobbies[m]);
					} else {
						sprintf(name, "%s %d", g_Lobbies[m], d + 1);
					}
				}
				SMessagePrintf(ColorSystem, Fetch_String(TXT_FINDOK), name);
			}
			else if (r == CHAT_S_FIND_NOCHAN) {
				SMessagePrintf(ColorSystem, Fetch_String(TXT_FINDNONE));
			}
			else if (r == CHAT_S_FIND_NOTHERE) {
				SMessagePrintf(ColorSystem, Fetch_String(TXT_FINDGONE));
			}
			else if (r == CHAT_S_FIND_OFF) {
				SMessagePrintf(ColorSystem, Fetch_String(TXT_FINDDISABLED));
			}
			else {
				SMessagePrintf(ColorSystem, Fetch_String(TXT_FINDFAILED));
			}

			return(S_OK);
		}

		/// <summary>
		/// Prints a system message from the server to the chat window.
		/// </summary>
		/// <param name="r">Result code from the server; failure suppresses printing.</param>
		/// <param name="text">Text of the system message.</param>
		/// <returns>Always S_OK.</returns>
		STDMETHOD(OnSystemMessage)(HRESULT r, const char * text)
		{
			if (g_PlayingNetGame) return(S_OK);

			if (FAILED(r)) {
				return(S_OK);
			}

			PMessagePrintf(ColorSystem, ">>> %s", text);
			return(S_OK);
		}

		/// <summary>
		/// Handles a network status change/error from the chat connection. If status
		/// notifications are being deliberately ignored (e.g. during a synchronous wait),
		/// records the status and signals the waiting event instead of acting on it.
		/// Otherwise, on a disconnect, either silently notes an already-known connection loss,
		/// warns the player and queues an in-game message the first time it happens during a
		/// net game, or aborts back out of the lobby with a message box if not in a game.
		/// </summary>
		/// <param name="status">The new network status.</param>
		/// <returns>Always S_OK.</returns>
		STDMETHOD(OnNetStatus)(HRESULT status)
		{
			if (g_IgnoreNetStatus) {
				SetEvent(g_WaitEventHandles[EV_NETIGNORE]);
				g_LastIgnoredStatus = status;
				return(S_OK);
			}

			/// Only a disconnect requires any action here.
			if (status == CHAT_S_CON_DISCONNECTED) {
				if ((g_PlayingNetGame) && (g_ConnectionLost)) {
					//
				} else if (g_PlayingNetGame) {
					char txt[256];
					txt[0] = '\0';
					sprintf(txt, Fetch_String(TXT_WOL_CONNLOST));
					int scheme = GadgetClass::Get_Color_Scheme();
					Session.Messages.Add_Message(NULL, 0, txt, scheme, TPF_TEXT, Rule->MessageDelay * TICKS_PER_MINUTE);
					Map.Flag_To_Redraw();
					g_ConnectionLost = true;
					g_ChannelCount = 0;
				} else {
					if (WS_Find_Dialog(IDD_WOL_MAIN)) {
						ODMessageBox(Fetch_String(TXT_YOURE_DISCON), 0, WOL_Wait_Callback);
					}
					g_IgnoreNetStatus = true;
					SetEvent(g_WaitEventHandles[EV_ABORT]);
					g_ChannelCount = 0;
				}
			}
			return(S_OK);
		}

		/// <summary>
		/// Receives a channel list from the server (either a normal channel-list reply or a
		/// game-list ping reply) and splits it into the chat-channel and game-channel lists.
		/// For each game channel, queues WDT team and user-locale lookups for its name if not
		/// already cached, and batches ladder-list requests for names not yet in the ladder
		/// dictionary (flushing every 20 names and again at the end). Finally signals that the
		/// channel list arrived and refreshes the channel list display.
		/// </summary>
		/// <param name="r">Result code identifying the list type (ping/game list) and whether
		/// the request failed.</param>
		/// <param name="channels">Head of the linked list of channels returned by the server.</param>
		/// <returns>Always S_OK.</returns>
		STDMETHOD(OnChannelList)(HRESULT r, Channel * channels)
		{
			Wstring name;
			char ladder_request_keys[512+1];
			memset(ladder_request_keys, 0, sizeof(ladder_request_keys));
			int counter = 0;

			if (g_PlayingNetGame) {
				return(S_OK);
			}

			unsigned int requested_type = CHANNELTYPE_CHAT;

			/*
			 * A ping-list reply is always the game channel list; otherwise the list type
			 * comes off the pending request queue.
			 */
			if (r == CHAT_S_PINGLIST) {
				requested_type = CHANNELTYPE_GAME;
			} else {
				g_ChanListQueue.removeHead(requested_type);
			}

			if ( FAILED(r)) {
				return(S_OK);
			}

			if (requested_type == CHANNELTYPE_CHAT) {
				g_UserChannels.clear();
			} else {
				g_GameChannelList.clear();
			}

			/*
			 * Split the server's channel list into chat channels and game channels,
			 * queuing lookups for each game channel's name along the way.
			 */
			while (channels != NULL) {
				if (channels->type == CHANNELTYPE_CHAT) {
					g_UserChannels.addTail(*channels);
				} else {
					g_GameChannelList.addTail(*channels);

					name.set((char *)channels->name);
					name.truncate('\'');
					name.toLower();
					if (Session.IsWDT) {
						Wstring temp;
						bool found = false;
						for (int i = 0; i < g_WDTTeamLookupQueue.length(); i++) {
							g_WDTTeamLookupQueue.get(temp, i);
							if (temp == name) {
								found = true;
								break;
							}
						}
						if (!found) {
							g_WDTTeamLookupQueue.addHead(name);
						}
					}

					if (!g_UserLocales.contains(name)) {
						Wstring temp;
						bool found = false;
						for (int i = 0; i < g_UserLocaleLookupQueue.length(); i++) {
							g_UserLocaleLookupQueue.get(temp, i);
							if (temp == name) {
								found = true;
								break;
							}
						}
						if (!found) {
							g_UserLocaleLookupQueue.addHead(name);
						}
					}

					/// Batch up ladder-list requests for names not yet cached.
					if (!g_ActiveLadder->contains(name)) {
						strcat(ladder_request_keys, name.get());
						strcat(ladder_request_keys, ":");
						counter++;
					}
				}

				/*
				 * Flush the batch once it gets large, to keep the request packet a
				 * reasonable size.
				 */
				if (counter > 20) {
					g_pNetUtil->RequestLadderList(g_LadderServerHost, g_LadderServerPort, ladder_request_keys, LADDER_CODE(g_InstalledLadderSKU), -1, 0, 0);
					counter = 0;
					memset(ladder_request_keys, 0, sizeof(ladder_request_keys)-1);
				}
				channels = channels->next;
			}

			/// Flush any remaining names once the list is exhausted.
			if (counter != 0) {
				g_pNetUtil->RequestLadderList(g_LadderServerHost, g_LadderServerPort, ladder_request_keys, LADDER_CODE(g_InstalledLadderSKU), -1, 0, 0);
				memset(ladder_request_keys, 0, sizeof(ladder_request_keys)-1);
			}

			SetEvent(g_WaitEventHandles[EV_CHAN_LIST]);

			Draw_Channel_List();

			return(S_OK);
		}

		/// <summary>
		/// Prints a private (whispered) chat action/emote from another user to the chat
		/// window.
		/// </summary>
		/// <param name="r">Result code from the server; failure suppresses printing.</param>
		/// <param name="user">The user who performed the action.</param>
		/// <param name="action">Text of the action.</param>
		/// <returns>Always S_OK.</returns>
		STDMETHOD(OnPrivateAction)(HRESULT r, struct User * user, const char * action)
		{
			if (g_PlayingNetGame) return(S_OK);

			if (FAILED(r)) {
				return(S_OK);
			}

			PMessagePrintf(ColorPrivAction, "%s %s", user->name, action);
			return(S_OK);
		}

		/// <summary>
		/// Prints a public chat action/emote performed in the current channel to the chat
		/// window.
		/// </summary>
		/// <param name="r">Result code from the server; failure suppresses printing.</param>
		/// <param name="user">The user who performed the action.</param>
		/// <param name="action">Text of the action.</param>
		/// <returns>Always S_OK.</returns>
		STDMETHOD(OnPublicAction)(HRESULT r, struct Channel *, struct User * user, const char * action)
		{
			if (g_PlayingNetGame) return(S_OK);

			if (FAILED(r)) {
				return(S_OK);
			}

			PMessagePrintf(ColorAction, "%s %s", user->name, action);
			return(S_OK);
		}

		/// <summary>Unused channel-topic callback stub; does nothing.</summary>
		/// <returns>Always S_OK.</returns>
		STDMETHOD(OnChannelTopic)(HRESULT, struct Channel *, const char *) { return(S_OK); }

		/// <summary>Unused user-IP callback stub; does nothing.</summary>
		/// <returns>Always S_OK.</returns>
		STDMETHOD(OnUserIP)(HRESULT r, User *) { return(S_OK); }

		/// <summary>
		/// Reports that the server has banned us from a channel, showing a message box with
		/// the ban expiration time.
		/// </summary>
		/// <param name="r">Result code; only S_OK shows the ban message.</param>
		/// <param name="bannedTill">Time the ban expires.</param>
		/// <returns>Always S_OK.</returns>
		STDMETHOD(OnServerBannedYou)(HRESULT r, time_t bannedTill)
		{
			if (g_PlayingNetGame) return(S_OK);

			if (r == S_OK) {
				char ban_text[200];
				sprintf(ban_text, Fetch_String(TXT_BANNEDTILL), ctime(&bannedTill));
				ODMessageBox(ban_text, MB_OK, WOL_Wait_Callback);
			}
			return(S_OK);
		}

		/// <summary>
		/// Reports that a user has been banned or unbanned from the current channel.
		/// </summary>
		/// <param name="name">Name of the user whose ban status changed.</param>
		/// <param name="banned">Non-zero if the user was banned, zero if unbanned.</param>
		/// <returns>Always S_OK.</returns>
		STDMETHOD(OnChannelBan)(HRESULT, LPCSTR name, int banned)
		{
			if (g_PlayingNetGame) return(S_OK);

			if (banned) {
				PMessagePrintf(ColorSystem, Fetch_String(TXT_CHAN_BAN), name);
			} else {
				PMessagePrintf(ColorSystem, Fetch_String(TXT_CHAN_UNBAN), name);
			}
			return(S_OK);
		}

		/// <summary>
		/// Updates a user's cached channel flags (e.g. operator/voice/squelch) in the local
		/// user list and refreshes the user list display.
		/// </summary>
		/// <param name="r">Result code from the server; only S_OK applies the update.</param>
		/// <param name="name">Name of the user whose flags changed.</param>
		/// <param name="flags">New flag bits to set.</param>
		/// <param name="mask">Mask of flag bits being replaced; existing bits outside the
		/// mask are preserved.</param>
		/// <returns>Always S_OK.</returns>
		STDMETHOD(OnUserFlags)(HRESULT r, LPCSTR name, unsigned flags, unsigned mask)
		{
			int i;
			User user;

			if (r == S_OK) {

				/// Find the matching user by name and merge in the new flag bits.
				for (i = 0; i < g_UserList.length(); i++) {
					if ((g_UserList.get(user, i)) && (strcmpi((char *)user.name, name) == 0)) {
						user.flags = ((user.flags & mask) | flags);
						g_UserList.replace(user, i);
					}
				}
				Draw_Player_List();
			}
			return(S_OK);
		}

		/// <summary>
		/// Handles notification that a user has been kicked from a channel. Builds a display
		/// name for the channel (translating a numbered lobby channel back to its localized
		/// name), reports the kick to the chat window -- taking us back to the previous screen
		/// if we were the one kicked -- then removes the kicked user from our channel user
		/// list and refreshes the user/channel list displays.
		/// </summary>
		/// <param name="r">Result code from the server; failure suppresses handling.</param>
		/// <param name="channel">The channel the kick occurred in.</param>
		/// <param name="kicked">The user who was kicked.</param>
		/// <param name="kicker">The user (channel owner) who performed the kick.</param>
		/// <returns>Always S_OK.</returns>
		STDMETHOD(OnUserKick)(HRESULT r, struct Channel * channel, User * kicked, User * kicker)
		{

			if (g_PlayingNetGame) {
				return(S_OK);
			}

			if (FAILED(r)) {
				return(S_OK);
			}

			static char _kickmessage[64];
			memset(_kickmessage, 0, sizeof(_kickmessage));

			/// Translate a numbered lobby channel name back into its localized display name.
			if (strnicmp((char *)channel->name, "lob_18_0", strlen("lob_18_0")) == 0) {
				int v = atol(((char *)channel->name) + strlen("lob_18_"));
				int d = v / WOL_LOBBY_NUM;
				int m = v % WOL_LOBBY_NUM;
				if (d == 0) {
					sprintf(_kickmessage, "%s", g_Lobbies[m]);
				} else {
					sprintf(_kickmessage, "%s %d", g_Lobbies[m], d + 1);
				}
			} else {
				sprintf(_kickmessage, "%s", channel->name);
			}

			/*
			 * Report the kick, backing out of the channel ourselves if we were the one
			 * kicked.
			 */
			if (kicked->flags & CHAT_USER_MYSELF) {
				PMessagePrintf(ColorSystem, Fetch_String(TXT_YOURE_KICKED), _kickmessage, kicker->name);
				GoBack();
			} else {
				PMessagePrintf(ColorSystem, Fetch_String(TXT_USER_KICKED), kicked->name, _kickmessage, kicker->name);
			}

			Handle_User_Leave(*kicked);
			Draw_Player_List();
			Draw_Channel_List();

			return(S_OK);
		}

		/// <summary>
		/// Processes a reply to a user-locale request, caching each user's locale by
		/// lowercased name and, if our own locale was included, updating the session's locale
		/// and signaling that it is known.
		/// </summary>
		/// <param name="r">Result code from the server; negative signals a failure.</param>
		/// <param name="users">Head of the linked list of users whose locales were returned.</param>
		/// <returns>Always S_OK.</returns>
		STDMETHOD(OnUserLocale)(HRESULT r, User * users)
		{
			bool set = false;
			if (r < 0) {
				DebugString("OnUserLocale callback failed!\n");
				return(S_OK);
			}

			User *user = users;

			/// Re-cache each user's locale, replacing any previous entry.
			while (user != NULL) {
				Wstring name;
				name.set((char *)user->name);
				name.toLower();

				g_UserLocales.remove(name);
				int locale = user->locale;
				g_UserLocales.add(name, locale);

				/*
				 * If we're one of the users in this reply, adopt the reported locale as
				 * our own.
				 */
				if (strcmp((const char *)users->name, g_NickName) == 0) {
					Session.Locale = (int)users->locale;
					DebugString("OnUserLocale callback - Locale is %d\n", Session.Locale);
					set = true;
				}
				user = user->next;
			}

			if (set) {
				SetEvent(g_WaitEventHandles[EV_LOCALE]);
			}

			return(S_OK);
		}

		/// <summary>
		/// Applies a locale change for the local player: updates the session's locale if it
		/// differs, then re-caches our own locale entry under our handle.
		/// </summary>
		/// <param name="r">Result code from the server (unused).</param>
		/// <param name="newlocale">The locale to apply.</param>
		/// <returns>Always S_OK.</returns>
		STDMETHOD(OnSetLocale)(HRESULT r, Locale newlocale)
		{
			if (newlocale != Session.Locale) {
				Session.Locale = (int)newlocale;
			}

			Wstring name;
			name.set(Session.Handle);
			name.toLower();

			if (g_UserLocales.contains(name)) {
				g_UserLocales.remove(name);
			}
			g_UserLocales.add(name, Session.Locale);

			return(S_OK);
		}

		/// <summary>
		/// Processes a reply to a user-team request (World Domination Tour), caching each
		/// user's team by name and refreshing the user list display.
		/// </summary>
		/// <param name="r">Result code from the server; failure only logs and returns.</param>
		/// <param name="users">Head of the linked list of users whose teams were returned.</param>
		/// <returns>Always S_OK.</returns>
		STDMETHOD(OnUserTeam)(HRESULT r, User * users)
		{
			if (SUCCEEDED(r)) {

				User *user = users;

				Wstring name;

				/// Re-cache each user's team, replacing any previous entry.
				while (user != NULL) {
					name.set((char *)user->name);

					if (g_WDTTeams.contains(name)) {
						g_WDTTeams.remove(name);
					}
					int team = user->team;
					g_WDTTeams.add(name, team);

					user = user->next;
				}
				Draw_Player_List();
				return(S_OK);
			}

			DebugString("OnUserTeam FAILED!\n");

			return(S_OK);
		}

		/// <summary>
		/// Applies a team (house) change for the local player: updates the session's house,
		/// then re-caches our own team entry under our handle.
		/// </summary>
		/// <param name="r">Result code from the server; only S_OK applies the change.</param>
		/// <param name="newteam">The team/house value to apply.</param>
		/// <returns>Always S_OK.</returns>
		STDMETHOD(OnSetTeam)(HRESULT r, int newteam)
		{
			if (r == S_OK) {
				Session.House = newteam;
				Wstring name;
				name.set(Session.Handle);
				name.toLower();

				if (g_WDTTeams.contains(name)) {
					g_WDTTeams.remove(name);
				}
				g_WDTTeams.add(name, newteam);

				return(S_OK);
			}

			DebugString("OnSetTeam FAILED: %d\n", newteam);

			return(S_OK);
		}
};

CComObject<CChatEventSink> * g_pChatSink;

CComObject<CNetUtilEventSink> * g_pNetUtilSink;

CComObject<CDownloadEventSink> * g_pDownloadSink;

/// <summary>
/// Creates and initializes the WOL COM interfaces (Chat, NetUtil, and Download), wiring up
/// each one's event sink via AtlAdvise, verifies the Chat interface reports a new enough
/// version, and applies the registered product SKU to it.
/// </summary>
/// <param name="hInstance">Application instance handle used to initialize the ATL module.</param>
/// <returns>0 on success, or -1 if any interface/sink could not be created or the installed
/// API version is too old.</returns>
int Startup_Chat(HINSTANCE hInstance)
{
	HRESULT hresult = NULL;
	_Module.Init(NULL, hInstance);

	g_pChat = NULL;
	g_pNetUtil = NULL;
	g_pDownload = NULL;
	g_dwChatAdvise = 0;
	g_dwNetUtilAdvise = 0;
	g_dwDownloadAdvise = 0;

	/*
	 * Create the Chat interface and hook up its event sink.
	 */
	CoCreateInstance(CLSID_Chat, NULL, CLSCTX_INPROC_SERVER, IID_IChat, (void **)&g_pChat);
	if (g_pChat) {
		CComObject<CChatEventSink>::CreateInstance(&g_pChatSink);
		hresult = AtlAdvise(g_pChat, g_pChatSink->GetUnknown(), IID_IChatEvent, &g_dwChatAdvise);
	}
	if ((g_pChat == NULL) || (hresult != S_OK)) {
		ODMessageBox(Fetch_String(TXT_APIMISSING), MB_OK, NULL);
		return(-1);
	}

	/*
	 * Create the NetUtil interface and hook up its event sink.
	 */
	CoCreateInstance(CLSID_NetUtil, NULL, CLSCTX_INPROC_SERVER, IID_INetUtil, (void **)&g_pNetUtil);
	if (g_pNetUtil) {
		CComObject<CNetUtilEventSink>::CreateInstance(&g_pNetUtilSink);
		hresult = AtlAdvise(g_pNetUtil, g_pNetUtilSink->GetUnknown(), IID_INetUtilEvent, &g_dwNetUtilAdvise);
	}
	if ((g_pNetUtil == NULL) || (hresult != S_OK)) {
		ODMessageBox(Fetch_String(TXT_APIMISSING), MB_OK, NULL);
		return(-1);
	}

	/*
	 * Create the Download interface and hook up its event sink.
	 */
	CoCreateInstance(CLSID_Download, NULL, CLSCTX_INPROC_SERVER, IID_IDownload, (void **)&g_pDownload);
	if (g_pDownload) {
		CComObject<CDownloadEventSink>::CreateInstance(&g_pDownloadSink);
		hresult = AtlAdvise(g_pDownload, g_pDownloadSink->GetUnknown(), IID_IDownloadEvent, &g_dwDownloadAdvise);
	}
	if ((g_pDownload == NULL) || (hresult != S_OK)) {
		ODMessageBox(Fetch_String(TXT_APIMISSING), MB_OK, NULL);
		return(-1);
	}

	/*
	 * Refuse to continue if the installed Chat API is older than we require.
	 */
	unsigned long version;
	g_pChat->GetVersion(&version);
	if (version < 0x10009) {
		ODMessageBox(Fetch_String(TXT_APIWRONGVERSION), MB_OK, NULL);
		return(-1);
	}

	/*
	 * Point the Chat interface at our registry key, then read our product SKU back
	 * out of the registry and pass it along too.
	 */
	g_pChat->SetAttributeValue("RegPath", APP_REG_KEY);

	HKEY rKey;
	char buf[256];
	if (APP_REG_KEY != NULL && strlen(APP_REG_KEY)) {
		strcpy(buf, APP_REG_KEY);
		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, buf, 0, KEY_READ, &rKey) == ERROR_SUCCESS) {
			DWORD type;
			DWORD value;
			DWORD valuesize = sizeof(value);
			if (RegQueryValueEx(rKey, "SKU", NULL, &type, (LPBYTE)&value, &valuesize) == ERROR_SUCCESS) {
				g_pChat->SetProductSKU(value);
			}
			RegCloseKey(rKey);
		}
	}

	return(0);
}

/// <summary>
/// Handles a user leaving the current channel. If the departing user is ourselves, resets all
/// local channel/session state and returns to the lobby (closing any open guest or game-options
/// dialogs); otherwise removes the given user from our local user list and updates the
/// game-options acceptance state before refreshing the user list display.
/// </summary>
/// <param name="user">The user who left the channel.</param>
void Handle_User_Leave(struct User & user)
{
	int i, j;

	if (g_CurrentChannel.type != CHANNELTYPE_CHAT) Sound_Effect(Rule->PlayerLeft);

	/// We are the one leaving -- tear down the current channel and return to the lobby.
	if (user.flags & CHAT_USER_MYSELF) {
		/*
		 * Translate a numbered lobby channel name back into its display name for the message below.
		 */
		char name[64];
		strcpy(name, (char *)g_CurrentChannel.name);
		if (strncmp((char *)name, LOB_PREFIX, sizeof(LOB_PREFIX)-1) == 0) {
			int num = atol(((char *)name) + strlen(LOB_PREFIX));
			int num2 = num / WOL_LOBBY_NUM;
			int num1 = num % WOL_LOBBY_NUM;
			if (num2 == 0) {
				sprintf(name, "%s", g_Lobbies[num1]);
			} else {
				sprintf(name, "%s %d", g_Lobbies[num1], num2 + 1);
			}
		}
		PMessagePrintf(ColorSystem, Fetch_String(TXT_YOULEFT), name);
		g_ChannelCount--;
		g_ChannelCount = 0;
		g_UserList.clear();
		g_IsChannelCreator = 0;
		memset(&g_CurrentChannel, 0, sizeof(Channel));
		memset(g_UserInfo, 0, sizeof(ChannelUserInfo) * MAX_USER_COUNT);
		Close_Wait_Window(WOL_WAIT_CHANNEL_LEAVE);

		Session.Options.ScenarioDescription[0] = 0;
		Session.ScenarioFileLength = 0;
		Session.ScenarioDigest[0] = 0;
		HWND guest_dlg = WS_Find_Dialog(IDD_WOL_GUEST);
		if (guest_dlg) {
			WS_Destroy_Dialog(guest_dlg, IDCANCEL);
			Join_Lobby();
		}
		HWND gamewin = WS_Find_Dialog(IDD_WOL_GAMEOPT);
		if (gamewin) {
			WS_Destroy_Dialog(gamewin, IDCANCEL);
			Join_Lobby();
		}
		HWND mainwin = WS_Find_Dialog(IDD_WOL_MAIN);
		if (mainwin) {
			ShowWindow(mainwin, SW_SHOW);
		}
		Draw_Channel_List();
		Draw_Player_List();

		return;
	}

	/*
	 * Someone else left -- remove them from our local user list.
	 */
	User temp;
	for (i = 0; i < g_UserList.length(); i++) {
		g_UserList.get(temp, i);
		if (strcmpi((char *)temp.name, (char *)user.name) == 0) {
			g_UserList.remove(i);
			if (g_CurrentChannel.type != CHANNELTYPE_CHAT) {

				memmove(g_UserInfo + i, g_UserInfo + i + 1, (sizeof(ChannelUserInfo)) * (MAX_USER_COUNT - i - 1));

				for (j = 1; j < (sizeof(g_UserInfo) / sizeof(ChannelUserInfo)); j++) {
					g_UserInfo[j].accepted = 0;
					EnableWindow(GetDlgItem(GameoptWindow(), IDC_ACCEPT), TRUE);
				}
			}

			if ((i == 0) && (g_CurrentChannel.type != CHANNELTYPE_CHAT) && (g_IsChannelCreator == 0)) {
				g_pChat->RequestChannelLeave();
			}

			break;
		}
	}
	Draw_Player_List();
}

/// <summary>
/// Logs the local player out of the WOL chat session. Sends a logout request to the Chat
/// interface and waits (with timeout retries) for it to complete or be aborted, then, for
/// World Domination Tour sessions, also closes the event handles and fully shuts down chat.
/// </summary>
void Logout_WOnline(void)
{
	int i;
	DWORD wait_result;

	g_IgnoreNetStatus = TRUE;

	/// Clear any cached WDT team assignments.
	if (Session.IsWDT) {
		g_WDTTeams.clear();
	}

	/// Only attempt to log out if we actually have a chat connection.
	if (g_pChat != NULL) {
		for (i = 0; i < EV_COUNT; i++) {
			ResetEvent(g_WaitEventHandles[i]);
		}
		WOL_Wait_Callback();
		/*
		 * Ask the server to log us out, then wait (retrying while the wait times out) for
		 * confirmation or an abort.
		 */
		if (g_pChat->RequestLogout() == S_OK) {
			Show_Wait_Window(WOL_WAIT_LOGOUT_DONE,false);
			Set_Wait_Dialog_Text((char *)Fetch_String(TXT_WOL_LOGGING_OUT));
			for (i = 0; i < 5; i++) {
				while ((wait_result = WaitForMultipleObjects(EV_COUNT, g_WaitEventHandles, FALSE, 100)) == WAIT_TIMEOUT) {
					WOL_Wait_Callback();
				}
				ResetEvent(g_WaitEventHandles[wait_result]);
				if (wait_result == EV_ABORT) {
					break;
				}
				if (wait_result == EV_NETIGNORE) {
					break;
				}
			}
		}

		Close_Wait_Window(WOL_WAIT_LOGOUT_DONE);

		/// For WDT sessions the chat session isn't needed after logging out -- shut it down completely.
		if (Session.IsWDT) {
			for (i = 0; i < EV_COUNT; i++) {
				CloseHandle(g_WaitEventHandles[i]);
			}
			Shutdown_Chat();
		}
	}
}

/// <summary>
/// Tears down the WOL COM interfaces: unadvises each event sink, releases the Chat, NetUtil,
/// and Download interfaces, clears the global pointers, and terminates the ATL module.
/// </summary>
void Shutdown_Chat(void)
{
	AtlUnadvise(g_pChat, IID_IChatEvent, g_dwChatAdvise);
	g_pChat->Release();

	AtlUnadvise(g_pNetUtil, IID_INetUtilEvent, g_dwNetUtilAdvise);
	g_pNetUtil->Release();

	AtlUnadvise(g_pDownload, IID_IDownloadEvent, g_dwDownloadAdvise);
	g_pDownload->Release();

	g_pChat = NULL;
	g_pNetUtil = NULL;
	g_pDownload = NULL;

	_Module.Term();
}

/// <summary>
/// Requests the World Domination Tour state.
/// This routine puts up no wait window of its own, which is what lets the wait callback
/// poll the tour state in the background while the player is doing something else. The
/// language code is refreshed from the installed product's SKU on the way through.
/// </summary>
/// <returns>Returns with 1 if a request was issued, or 0 if there is no NetUtil connection
/// to issue it on.</returns>
int Get_WDT_State_Silent(void)
{
	DWORD value = 0;

	/*
	 * Read the product SKU from the registry.
	 */
	HKEY rKey;
	char buf[256];
	if (APP_REG_KEY != NULL && strlen(APP_REG_KEY)) {
		strcpy(buf, APP_REG_KEY);
		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, buf, 0, KEY_READ, &rKey) == ERROR_SUCCESS) {
			DWORD type;
			DWORD valuesize = sizeof(value);
			RegQueryValueEx(rKey, "SKU", NULL, &type, (LPBYTE)&value, &valuesize);
			RegCloseKey(rKey);
		}
	}

	/*
	 * lang code is lowest byte of SKU
	 */
	g_LanguageCode = value & 0xFF;
	if (g_pChat != NULL) {
		g_pChat->SetProductSKU(value);
	}

	/// Ask the WDT server for full state, if we have a NetUtil connection.
	if (g_pNetUtil != NULL) {
		DebugString("Get_WDT_State_Silent: WDTHost is %s, port is %d\n", g_WDTHost, g_WDTPort);
		g_pNetUtil->RequestWDTState(g_WDTHost, g_WDTPort, WDT_REQ_STATE_EVERYTHING);
		return(1);
	}
	return(0);
}


/// <summary>
/// Logs the player in to Westwood Online.
/// This routine is used by the multiplayer menu when the player asks for an internet game.
/// It brings up the chat system, gathers the server list, runs the login dialog, and then
/// requests a connection to the chosen server. Any failure is reported to the player here --
/// a banned or disabled account is offered the matching support page.
/// </summary>
/// <returns>Returns with WONLINE_OK if the player reached the server. WONLINE_DOWNLOAD_PATCH
/// is returned when a patch must be installed first, and WONLINE_BACK when the player backed
/// out or the login could not be completed.</returns>
WonlineResult Login_WOL(void)
{
	DWORD wait_result;
	WonlineResult retval = WONLINE_BACK;
	int wdt_result;
	int i;
	int score;
	HWND dlg;
	Server * server = NULL;
	BYTE data[4];
	HKEY rKey;
	char regkey[256];

	memset(data, 0, sizeof(data));
	bool skip_error = false;
	g_OnConnectionResult = 0;

	if (APP_REG_KEY != NULL && strlen(APP_REG_KEY)) {
		strcpy(regkey, APP_REG_KEY);
		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, regkey, 0, KEY_READ, &rKey) == ERROR_SUCCESS) {
			DWORD type;
			DWORD valuesize = sizeof(data);
			RegQueryValueEx(rKey, "SKU", NULL, &type, (LPBYTE)data, &valuesize);
			RegCloseKey(rKey);
		}
	}

	score = Theme.What_Is_Playing();
	int dat = data[0];
	((DWORD &)data) = data[0];
	g_LanguageCode = ((DWORD &)data);

	g_GameSKU = dat + TIBERIAN_SUN_SKU;
	if (score == Fetch_Main_Menu_Theme()) {
		Theme.Queue_Song(THEME_QUIET);
	}
	if (Theme.What_Is_Playing() == Fetch_Map_Select_Theme()) {
		Theme.Queue_Song(THEME_QUIET);
	}

	Reset_WOL_Globals();
	g_IgnoreNetStatus = true;
	if (Startup_Chat(ProgramInstance) == -1) {
		return(WONLINE_BACK);
	}

	for (i = 0; i < EV_COUNT; i++) {
		g_WaitEventHandles[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
	}
	wdt_result = Request_WDT_Server_List();

	switch (wdt_result) {

		case 0: {

			DebugString("Creating login dlg\n");
			dlg = WS_Create_Dialog(ProgramInstance, IDD_WOL_LOGIN, MainWindow, (DLGPROC)WOL_Login_Dialog_Proc, FALSE);
			Center_Window_Within_Window(dlg);
			OwnerDraw::Subclass_Dialog(dlg, 0);
			ShowWindow(dlg, SW_NORMAL);
			if (WS_Wait_Dialog(dlg, WOL_Wait_Callback) == IDCANCEL) {
				goto cleanup;
			}

			WOL_Wait_Callback();
			g_IgnoreNetStatus = false;
			Show_Wait_Window(WOL_WAIT_LOGIN_DONE, false);
			Set_Wait_Dialog_Text((char *)Fetch_String(TXT_CONNECTING_SERVER));

			g_Servers.getPointer(&server, g_CurrentServerIndex);

			strcpy((char *)server->login, g_NickName);
			strcpy((char *)server->password, g_LoginPassword);
			g_pChat->RequestConnection(server, 20, ((int &)g_PlaintextPassword) & 0xFF);

			while ((wait_result = WaitForMultipleObjects(7, g_WaitEventHandles, FALSE, 100)) == WAIT_TIMEOUT) {
				WOL_Wait_Callback();
			}
			ResetEvent(g_WaitEventHandles[wait_result]);
			if (wait_result != EV_ABORT) {
				g_pChat->SetClientVersion(0x1000B);
				DebugString("About to RequestSetLocale - Locale is %d\n", Session.Locale);
				g_pChat->RequestSetLocale((Locale)Session.Locale);
				g_IgnoreNetStatus = false;

				Close_Wait_Window(WOL_WAIT_ALL);
				return(WONLINE_OK);
			} else {
				goto cleanup;
			}
		}


		default:

		case 1:
			skip_error = true;
			goto cleanup;

		case 2:
			skip_error = true;
			if (wdt_result == 2) {
				retval = WONLINE_DOWNLOAD_PATCH;
			}
			goto cleanup;

	}

cleanup:
	Close_Wait_Window(WOL_WAIT_ALL);

	if (!skip_error) {
		if (g_OnConnectionResult == CHAT_E_BANNED ||
			g_OnConnectionResult == CHAT_E_DISABLED ||
			g_OnConnectionResult == CHAT_E_SERIALBANNED ||
			g_OnConnectionResult == CHAT_E_SERIALDUP) {
			if (ODMessageBox(g_LastErrorMessage, MB_YESNO, WOL_Wait_Callback, 0) == IDYES) {
				char url[260];
				int langcode = g_LanguageCode;
				switch (g_OnConnectionResult) {
					case CHAT_E_BANNED:
						sprintf(url, "http://apiregister.westwood.com/support/banned_%d.html", langcode);
						break;
					case CHAT_E_DISABLED:
						sprintf(url, "http://apiregister.westwood.com/support/disabled_%d.html", langcode);
						break;
					case CHAT_E_SERIALBANNED:
						sprintf(url, "http://apiregister.westwood.com/support/serialbanned_%d.html", langcode);
						break;
					case CHAT_E_SERIALDUP:
						sprintf(url, "http://apiregister.westwood.com/support/serialdup_%d.html", langcode);
						break;
					default:
						break;
				}
				ViewHTML(url, 1);
			}
		} else {
			ODMessageBox(g_LastErrorMessage, MB_OK, WOL_Wait_Callback, 0);
		}
	}

	for (i = 0; i < EV_COUNT; i++) {
		CloseHandle(g_WaitEventHandles[i]);
	}

	Shutdown_Chat();

	g_IgnoreNetStatus = true;

	return(retval);
}

/// <summary>
/// Puts the player into a Westwood Online lobby.
/// This routine reconnects if the connection was dropped, brings up the main online dialog,
/// and then runs the lobby until the player starts a game, backs out, or quits. Every way
/// out passes through the same cleanup, so the chat session is torn down unless a game is
/// genuinely starting.
/// </summary>
/// <param name="win">Handle of an existing main WOL dialog to reuse, or 0 to create a
/// new one.</param>
/// <returns>Returns with 1 if a game was started from the lobby, or 0 if the lobby was left
/// normally. A failed reconnect returns its WonlineResult instead.</returns>
int Join_WOL_Lobby(HWND win)
{
	int i;
	int retval = 0;

	/*
	 * If we just finished a WDT match, play the appropriate win/lose emphasis sound for
	 * the local player before returning to the lobby.
	 */
	if (g_PlayingNetGame && Session.IsWDT && Session.Winner != -1 && Session.Winner < MAX_PLAYERS) {
		if (strnicmp(Session.Score[Session.Winner].Name, Session.Handle, sizeof(Session.Handle)) == 0) {
			if (Session.House == HOUSE_GOOD) {
				Play_WDT_Sound(g_WDT_GDIWinEmphasisSounds);
			} else if (Session.House == HOUSE_BAD) {
				Play_WDT_Sound(g_WDT_NodWinEmphasisSounds);
			}
		} else if (Session.House == HOUSE_GOOD) {
			Play_WDT_Sound(g_WDT_GDILoseEmphasisSounds);
		}
	}

	g_PlayingNetGame = false;
	g_GameStarted = false;
	/// For WDT sessions, tell the server our current team and make sure the WDT sounds are loaded.
	if (Session.IsWDT) {
		g_pChat->RequestSetTeam(Session.House);
		if (!g_WDTSoundsInited) {
			Init_WDT_Sounds();
		}
	}

	/// Reconnect to the WOL server first if we previously lost the connection.
	if (g_ConnectionLost) {
		WonlineResult result = Login_WOL();
		if (result != WONLINE_OK) {
			return(result);
		}
	}

	{
		for (i = 0; i < EV_COUNT; i++) {
			ResetEvent(g_WaitEventHandles[i]);
		}

		/// Create the main WOL dialog if one wasn't already supplied, and print any message of the day.
		if (win == 0) {
			win = WS_Create_Dialog(ProgramInstance, IDD_WOL_MAIN, MainWindow, WOL_Main_Dialog_Proc, FALSE);
			Center_Window_Within_Window(win);
			OwnerDraw::Subclass_Dialog(win, 0);
			if (g_MessageOfTheDay != NULL) {
				char *token = strtok(g_MessageOfTheDay, "\n");
				while (token != NULL) {
					PMessagePrintf(ColorSystem, token);
					token = strtok(NULL, "\n");
				}
				free(g_MessageOfTheDay);
				PMessagePrintf(ColorSystem, "\n");
				g_MessageOfTheDay = NULL;
			}
			Show_Wait_Window(WOL_WAIT_LOGIN_DONE, false);
		}

		/*
		 * Request our squad info and force music shuffle while we're in the lobby.
		 */
		g_SquadInfoRequests++;
		g_RequestingOwnSquadInfo = true;
		g_pChat->RequestSquadInfo(0);

		bool wasshuffle = Theme.Is_Shuffle();
		Theme.Set_Shuffle(true);

		Apply_WOL_Settings();

		g_ServerType = CHANNELTYPE_GAME;

		/*
		 * Ask the server for the current channel list.
		 */
		Set_Wait_Dialog_Text((char *)Fetch_String(TXT_REQ_CHANLIST));

		if (!g_pChat->RequestChannelList(CHANNELTYPE_CHAT, 0)) {
			unsigned type = CHANNELTYPE_CHAT;
			g_ChanListQueue.addTail(type);
		}

		/*
		 * Wait for the channel list to arrive, aborting if it takes too long or the
		 * connection is aborted.
		 */
		int timeout = 0;
		DWORD idx;
		do {
			while ((idx = WaitForMultipleObjects(EV_COUNT, g_WaitEventHandles, FALSE, 100)) == WAIT_TIMEOUT) {
				timeout += 100;
				if (timeout > 30000) {
					sprintf(g_LastErrorMessage, Fetch_String(TXT_CANT_CONNECT));
					SetEvent(g_WaitEventHandles[EV_ABORT]);
					idx = EV_ABORT;
					break;
				}
				WOL_Wait_Callback();
			}
			if (idx == (DWORD)-1) {
				DebugString("WaitForMultipleObjects() failed in Join_WOL_Lobby");
			}
			ResetEvent(g_WaitEventHandles[idx]);
		} while (idx != EV_ABORT && idx != EV_CHAN_LIST);

		if (idx == EV_ABORT) {

			/*
			 * Aborted while waiting for the channel list.
			 */
			goto cleanup;
		}

		/// Channel list received -- initialize WDT state if this is a WDT session.
		if (Session.IsWDT) {
			Init_WDT();
		}

		/*
		 * Join our assigned lobby channel and wait for confirmation.
		 */
		Set_Wait_Dialog_Text((char *)Fetch_String(TXT_JOINLOB));

		if (Join_Lobby() != -1) {
			while ((idx = WaitForMultipleObjects(EV_COUNT, g_WaitEventHandles, FALSE, 100)) == WAIT_TIMEOUT) {
				WOL_Wait_Callback();
			}
			ResetEvent(g_WaitEventHandles[idx]);
			if (idx == EV_ABORT) {

				/*
				 * Lobby join was aborted.
				 */
				goto logout;
			}
			CurrentLevel = WOL_LEVEL_LOBBIES;
			if (idx == EV_CHAN_JOINED) {
				CurrentLevel = WOL_LEVEL_GAMES;
			}
		}

		/*
		 * Lobby joined -- show the main WOL window.
		 */
		Close_Wait_Window(WOL_WAIT_ALL);
		SetFocus(MainWindow);
		ShowWindow(win, SW_SHOWNORMAL);
		SetFocus(win);
		g_LastGameListTime = 0;
		g_LastChatListTime = time(0);

		/*
		 * Main lobby loop: process background callbacks until the user starts a game,
		 * aborts, or exits.
		 */
		while (true) {
			while ((idx = WaitForMultipleObjects(EV_COUNT, g_WaitEventHandles, FALSE, 20)) == WAIT_TIMEOUT) {
				WOL_Wait_Callback();
				Title_Screen_Restore(false);
				if (g_JoinLobbyNow) {
					Join_Lobby();
					g_JoinLobbyNow = 0;
				}
			}
			ResetEvent(g_WaitEventHandles[idx]);
			if (idx != EV_ABORT) {
				if (idx != EV_EXIT) {
					if (idx != EV_STARTGAME) {
						continue;
					}
					retval = 1;
					g_pChat->RequestChannelLeave();
					g_GameStartRequested = false;
					g_LastErrorMessage[0] = 0;
					goto cleanup;
				}
				g_LastErrorMessage[0] = 0;
				goto logout;
			}
			goto cleanup;
		}

	logout:
		WS_Destroy_Dialog(win, 0);
		win = NULL;
		if (g_pChat != NULL) {
			g_pChat->RequestChannelLeave();
		}
		Draw_Menu_Background();
		if (!Session.IsWDT) {
			Logout_WOnline();
		}

		/*
		 * Common cleanup: report any error, destroy any leftover dialogs, and, unless a
		 * game was started, tear down the WOL chat session entirely.
		 */
	cleanup:
		Close_Wait_Window(WOL_WAIT_ALL);
		ODMessageBox(g_LastErrorMessage, MB_OK, WOL_Wait_Callback, 0);
		if (win != NULL) {
			WS_Destroy_Dialog(win, 0);
		}
		while (WS_Destroy_Dialog(NULL, 0)) {
		}

		if (!Session.IsWDT) {
			if (retval == 0) {
				for (i = 0; i < EV_COUNT; i++) {
					CloseHandle(g_WaitEventHandles[i]);
				}
				Shutdown_Chat();
			} else {
				g_PlayingNetGame = true;
			}
		} else if (retval) {
			g_PlayingNetGame = true;
		}

		if (Session.IsWDT) {
			g_WDTTeams.clear();
		}

		Theme.Set_Shuffle(wasshuffle);
		return(retval);
	}
}

/// <summary>
/// Determines which lobby channel to join.
/// This routine is used by the lobby join code to settle on a lobby that still has room in
/// it. A World Domination Tour session prefers the lobby named after the player's own
/// territory, so that the people fighting over it end up in the same room.
/// </summary>
/// <returns>Returns with the channel list index of the chosen lobby, or -1 if none
/// had room.</returns>
int Pick_Lobby(void)
{
	int count = g_UserChannels.length();
	int user_cap = 25;
	int i = g_CurrentLobby;
	char lobby_name[64];
	Channel chan;
	memset(&chan, 0, sizeof(chan));

	char name[32];
	char def[32];
	char entry[32];
	char buf[32];

	/*
	 * For WDT sessions, look up the display name of the player's assigned territory from
	 * WDTMAP.INI so it can be matched against the lobby channel names below.
	 */
	if (Session.IsWDT) {
		memset(buf, 0, sizeof(buf));
		memset(name, 0, sizeof(name));
		memset(entry, 0, sizeof(entry));
		memset(def, 0, sizeof(def));

		MFCD * mix = new MFCD("WDT.MIX", &FastKey);

		INIClass ini;
		CCFileClass file("WDTMAP.INI");
		ini.Load(file, false);
		sprintf(entry, "Territory%02d", Session.WDTTerritory);
		strcpy(def, entry);
		if (WDT_Get_New_State()->MapID == 0) {
			ini.Get_String("NorthAmerica", entry, def, buf, sizeof(def));
		} else {
			ini.Get_String("Europe", entry, def, buf, sizeof(def));
		}
		ini.Get_String(buf, "Name", "A territory", name, sizeof(buf));
		static char * _tername;
		_tername = strdup(name);
		delete mix;
	}

	sprintf(lobby_name, "Lob_18_%d", g_CurrentLobby);

	/// First pass: exact match against the territory's display name.
	for (i = 0; i < count; i++) {
		g_UserChannels.get(chan, i);
		if (Session.IsWDT) {
			if (strcmp((char *)g_Lobbies[i], name) == 0) {
				if ((int)chan.currentUsers <= (int)user_cap) return(i);
			}
		}
	}

	/// Second pass: fall back to a prefix match, or any lobby with room, in channel order.
	for (i = 0; i < count; i++) {
		g_UserChannels.get(chan, i);
		if (Session.IsWDT) {
			if (strncmp((char *)g_Lobbies[i], name, strlen(name)) == 0) {
				if ((int)chan.currentUsers <= (int)user_cap) {
					g_CurrentLobby = atol((char *)chan.name + strlen(LOB_PREFIX));
					return(i);
				}
			} else {
				continue;
			}
		}
		if (strncmp((char *)chan.name, LOB_PREFIX, strlen(LOB_PREFIX)) == 0) {
			if (chan.currentUsers <= (unsigned)user_cap) {
				g_CurrentLobby = atol((char *)chan.name + strlen(LOB_PREFIX));
				return(i);
			}
		}
	}
	return(-1);
}

/// <summary>
/// Leaves the current channel (waiting for the leave to complete) and joins the lobby channel
/// selected by Pick_Lobby.
/// </summary>
/// <returns>The index of the joined lobby channel, or -1 if the leave timed out, the join
/// failed, or no lobby was available.</returns>
int Join_Lobby(void)
{
	HRESULT res;

	/*
	 * Leave whatever channel we're currently in.
	 */
	res = g_pChat->RequestChannelLeave();
	if (res == CHAT_E_NOTCONNECTED) {
		return(-1);
	}
	if (g_ChannelCount) Show_Wait_Window(WOL_WAIT_CHANNEL_LEAVE, false);

	/*
	 * Wait for the leave to be confirmed (channel count dropping to zero), with a timeout.
	 */
	int i = 0;
	while (g_ChannelCount) {
		WOL_Wait_Callback();
		Sleep(100);
		i++;
		if (i == 150) {
			return(-1);
		}
	}

	/*
	 * Pick a lobby and request to join it.
	 */
	int lobby_choice = Pick_Lobby();
	if (lobby_choice != -1) {
		Channel chan;
		g_UserChannels.get(chan, lobby_choice);
		strcpy((char *)chan.key, WOL_LOBBY_PASSWORD);
		s_TempChannel = chan;
		HRESULT res = g_pChat->RequestChannelJoin(&chan);
		if (res != S_OK) {
			lobby_choice = -1;
		} else {
			CurrentLevel = WOL_LEVEL_GAMES;
			Draw_Channel_List();
			return(lobby_choice);
		}
	}
	if (lobby_choice == -1) {
		CurrentLevel = WOL_LEVEL_LOBBIES;
		Draw_Channel_List();
		return(-1);
	}

	return(-1);
}

/// <summary>
/// Runs the new-account signup flow: refreshes the server list, then walks the player through
/// the "begin nickname" dialog (birthdate/email/newsletter) and an age check, followed by the
/// appropriate new-nick dialog (with or without parental-consent/newsletter fields, chosen by
/// the returned age), and submits the new nickname request to the server.
/// </summary>
/// <param name="win">Handle of the login dialog to hide/show around the signup dialogs.</param>
void NewLogin(HWND win)
{
	char nick[32];
	char pass[32];
	char verify_pass[32];
	char email[128];
	char parent_email[128];
	int newsletter = 1;
	int shareinfo = 0;
	int month = 1;
	int day = 0;
	int year = 0;
	char ladder_search_key[128];
	HWND dlg = NULL;

	for (int i = 0; i < EV_COUNT; i++) {
		ResetEvent(g_WaitEventHandles[i]);
	}

	memset(nick, 0, sizeof(nick));
	memset(pass, 0, sizeof(pass));
	memset(verify_pass, 0, sizeof(verify_pass));
	memset(email, 0, sizeof(email));
	memset(parent_email, 0, sizeof(parent_email));

	/*
	 * Refresh the server list before starting the signup dialogs.
	 */
	Show_Wait_Window(WOL_WAIT_IDLE, false);

	DebugString("Newnick update check\n");

	g_pChat->RequestServerList(g_GameSKU, WOL_GAME_VERSION, "TibSun", "TibPass99", 35);

	/*
	 * Wait for the server list request to complete, or for the user to abort/quit.
	 */
	DWORD wait_result;
	while(1) {
		wait_result = WaitForMultipleObjects(EV_COUNT, g_WaitEventHandles, FALSE, 100);
		while (wait_result == WAIT_TIMEOUT) {
			WOL_Wait_Callback();
			wait_result = WaitForMultipleObjects(EV_COUNT, g_WaitEventHandles, FALSE, 100);
		}
		ResetEvent(g_WaitEventHandles[wait_result]);
		if (wait_result==EV_ABORT) {
			break;
		}
		if (wait_result==EV_GOT_SERVERS) {
			break;
		}
	}
	Close_Wait_Window(WOL_WAIT_ALL);

	if (wait_result == EV_ABORT) {
		return;
	}

	if (wait_result == EV_QUITGAME) {
		SetEvent(g_WaitEventHandles[EV_QUITGAME]);
		return;
	}

	/*
	 * Prompt for birthdate, email, and newsletter preference.
	 */
first_nick_retry:
	dlg = WS_Create_Dialog(ProgramInstance, IDD_WOL_BEGINNICK, MainWindow, (DLGPROC) WOL_Begin_Nick_Dialog_Proc, FALSE);
	Center_Window_Within_Window(dlg);
	OwnerDraw::Subclass_Dialog(dlg, 0);
	SendDlgItemMessage(dlg, IDC_BMONTH, CB_SETCURSEL, (WPARAM)month-1, 0);
	if (day > 0) {
		sprintf(ladder_search_key,"%d",day);
		SendDlgItemMessage(dlg, IDC_BDAY,WM_SETTEXT, 0, (LPARAM)ladder_search_key);
	}
	if (year > 0) {
		sprintf(ladder_search_key,"%d",year);
		SendDlgItemMessage(dlg, IDC_BYEAR, WM_SETTEXT, 0, (LPARAM)ladder_search_key);
	}
	SendDlgItemMessage(dlg, IDC_EMAIL, WM_SETTEXT, 0, (LPARAM)email);
	SendDlgItemMessage(dlg, IDC_NEWSLETTER, BM_SETCHECK, (WPARAM)newsletter, 0);
	ShowWindow(win, SW_HIDE);
	ShowWindow(dlg, SW_NORMAL);

	if (WS_Wait_Dialog(dlg, WOL_Wait_Callback) == IDOK) {
		WS_Get_Saved_Value(IDC_EMAIL, (unsigned char *)email, sizeof(email)-1);
		WS_Get_Saved_Value(IDC_BMONTH, (unsigned char *)&month, sizeof(month));
		month++;
		WS_Get_Saved_Value(IDC_BDAY, (unsigned char *)ladder_search_key, sizeof(ladder_search_key));
		day = atol(ladder_search_key);
		WS_Get_Saved_Value(IDC_BYEAR, (unsigned char *)ladder_search_key, sizeof(ladder_search_key));
		year = atol(ladder_search_key);
		WS_Get_Saved_Value(IDC_NEWSLETTER,  (unsigned char *)&newsletter, sizeof(newsletter));

		/// All fields are required before continuing.
		if ((month <= 0) || (day <= 0) || (year <= 0) || (strlen(email) < 6)) {
			ODMessageBox(Fetch_String(TXT_REQUIRED_FIELD), MB_OK, WOL_Wait_Callback);
			goto first_nick_retry;
		}

		/*
		 * Ask the server to validate the entered age/email.
		 */
		g_UserAge = -1;
		g_UserConsent = 0;
		if (g_pNetUtil->RequestAgeCheck(month, day, year, email) != S_OK) {
			ODMessageBox(Fetch_String(TXT_CANT_CONNECT), MB_OK, WOL_Wait_Callback);
			/*
			 * Show the appropriate new-nick dialog for the checked age: full newsletter
			 * signup for adults, parental-consent fields for minors old enough to
			 * register, or the basic dialog for children who are too young.
			 */
new_nick_retry:
			int dlgid;
			if (g_UserAge < 13) {
				dlgid = IDD_WOL_NEWNICK;
			} else {
				if (g_UserAge >= 18) {
					dlgid = IDD_WOL_NEWNICK_NEWSLETTER;
				} else {
					dlgid = IDD_WOL_NEWNICK_CONSENT;
				}
			}
			dlg = WS_Create_Dialog(ProgramInstance, dlgid, MainWindow, (DLGPROC)WOL_New_Nick_Dialog_Proc, FALSE);
			SendDlgItemMessage(dlg, IDC_NICKNAME, WM_SETTEXT, 0, (LPARAM)nick);
			SendDlgItemMessage(dlg, IDC_CONSENT_PARENT_EMAIL, WM_SETTEXT, 0, (LPARAM)parent_email);
			SendDlgItemMessage(dlg, IDC_PASSWORD, WM_SETTEXT, 0, (LPARAM)pass);
			SendDlgItemMessage(dlg, IDC_VERIFY, WM_SETTEXT, 0, (LPARAM)verify_pass);

			Center_Window_Within_Window(dlg);
			OwnerDraw::Subclass_Dialog(dlg, 0);
			ShowWindow(dlg, SW_NORMAL);

			if (WS_Wait_Dialog(dlg, WOL_Wait_Callback) == IDOK) {
				WS_Get_Saved_Value(IDC_NICKNAME, (unsigned char *)nick, sizeof(nick)-1);
				WS_Get_Saved_Value(IDC_PASSWORD, (unsigned char *)pass, sizeof(pass)-1);
				WS_Get_Saved_Value(IDC_VERIFY, (unsigned char *)verify_pass, sizeof(verify_pass)-1);
				parent_email[0] = 0;
				WS_Get_Saved_Value(IDC_CONSENT_PARENT_EMAIL, (unsigned char *)parent_email, sizeof(parent_email)-1);
				shareinfo = 0;
				WS_Get_Saved_Value(IDC_NEWSLETTER_CONSENT, (unsigned char *)&shareinfo, sizeof(shareinfo));

				/// Validate the password fields before submitting.
				if (strcmp(pass, verify_pass) != 0) {
					ODMessageBox(Fetch_String(TXT_PASSWORD_VERIFY), MB_OK, WOL_Wait_Callback);
					goto new_nick_retry;
				}

				if (strlen(pass) != 8) {
					ODMessageBox(Fetch_String(TXT_PASSWORD_TOO_SHORT), MB_OK, WOL_Wait_Callback);
					goto new_nick_retry;
				}

				/// Submit the new nickname request and wait for the result.
				if (g_pNetUtil->RequestNewNick(nick, pass, email, parent_email, newsletter, shareinfo) != S_OK) {
					ODMessageBox(Fetch_String(TXT_CANT_CONNECT), MB_OK, WOL_Wait_Callback);
					return;
				}
				Show_Wait_Window(WOL_WAIT_NEW_NICK, true, Fetch_String(TXT_REQ_NICK));
				if (FAILED(g_OnNewNickResult)) {
					goto new_nick_retry;
				}
				ShowWindow(win, SW_SHOW);
				return;
			} else {
				goto first_nick_retry;
			}
		} else {
			/*
			 * Age check failed or is still pending -- handle each outcome.
			 */
			Show_Wait_Window(WOL_WAIT_NEW_NICK);
			if (g_UserAge < 0) {
				if (g_AgeCheckResult == NETUTIL_E_INVALIDFIELD) {
					ODMessageBox(Fetch_String(TXT_REQUIRED_FIELD), MB_OK, WOL_Wait_Callback);
					goto first_nick_retry;
				} else {
					ODMessageBox(Fetch_String(TXT_CANT_CONNECT), MB_OK, WOL_Wait_Callback);
					goto first_nick_retry;
				}
			} else {
				/// Under 13 without consent -- point the user at the consent form.
				if (g_UserAge < 13 && !g_UserConsent) {
					if (ODMessageBox(Fetch_String(TXT_CONSENT_REQUIRED), MB_YESNO, WOL_Wait_Callback) == IDYES) {
						char url[256];
						sprintf(url, "http://apiregister.westwood.com/consent_form/index_%d.html", g_LanguageCode);
						ViewHTML(url, 1);
					}
					goto first_nick_retry;
				}
				goto new_nick_retry;
			}
		}
	} else {
		ShowWindow(win, SW_NORMAL);
	}
}

/// <summary>
/// Fills a login dialog in from a remembered login.
/// This routine is used to spare the player from retyping credentials. If the nickname is
/// one Westwood Online has kept, its password and locale are recalled into the dialog's
/// controls along with it; if it is not, the dialog is left as it was.
/// </summary>
/// <param name="name">Nickname to search for among the stored logins, or NULL to skip
/// the lookup.</param>
void FetchLogin(const char *name, HWND win)
{
	LPCSTR nick = NULL;
	LPCSTR pass = NULL;
	int nicknum = 0;
	HWND item;

	/// Search the stored logins (slots 1-32) for a matching nickname.
	if (name != NULL) {
		for (int i = 1; i <= 32; i++) {
			nick = NULL;
			pass = NULL;
			g_pChat->GetNick(i, &nick, &pass);
			if (nick != NULL) {
				if (strcmp(nick, name) == 0) {
					item = GetDlgItem(win, IDC_NICKEDIT);
					if (item == NULL) {
						item = GetDlgItem(win, IDC_NICKNAME);
					}
					SetWindowText(item, nick);
					SetWindowText(GetDlgItem(win, IDC_PASSWORD), pass);
					nicknum = i;
					break;
				}
			}
		}

	}
	/*
	 * Populate the locale/server combo box, preferring the matched login's own locale.
	 */
	item = GetDlgItem(win, IDC_SERVER);
	if (item) {
		const char *locname = NULL;
		if (nicknum != 0) {
			Locale locale;
			if (g_pChat->GetNickLocale(nicknum, &locale) == 0 && g_pChat->GetLocaleString(&locname, locale) == 0) {
				Session.Locale = locale;
			}
		}
		if (locname == NULL) {
			g_pChat->GetLocaleString(&locname, (Locale)Session.Locale);
		}
		int sel = ComboBox_FindString(item, -1, locname);
		if (sel < 0) {
			sel = 0;
		}
		ComboBox_SetCurSel(item, sel);
	}
}

/// <summary>
/// Saves a login's nickname, password, and locale into one of the 32 stored login slots.
/// Reuses whichever slot already stores this nickname if one exists, otherwise the first
/// empty slot, otherwise cycles to the slot after the session's last-used nickname slot.
/// </summary>
/// <param name="loginName">Nickname to store.</param>
/// <param name="password">Password to store alongside the nickname.</param>
/// <param name="passwordPlaintext">Whether the password is stored/sent as plaintext.</param>
/// <param name="locale">Locale value to associate with this login.</param>
/// <returns>The result of setting the nick's locale (from SetNickLocale), or the computed
/// slot number if no slot ended up being written to.</returns>
int StoreLogin(const char *loginName, const char *password, bool passwordPlaintext, int locale)
{
	int last;
	const char *nick;
	const char *pass;

	int nicknum = 0;

	/// Look for an existing slot already storing this nickname.
	for (int i = 1; i <= 32; i++) {
		nick = NULL;
		pass = NULL;
		g_pChat->GetNick(i, &nick, &pass);
		if (nick && strcmp(nick, loginName) == 0) {
			nicknum = i;
			break;
		}
	}
	/// Not found -- use the first empty slot instead.
	if (nicknum == 0) {
		for (int j = 1; j <= 32; j++) {
			nick = NULL;
			pass = NULL;
			g_pChat->GetNick(j, &nick, &pass);
			if (nick == NULL || strlen(nick) == 0) {
				nicknum = j;
				break;
			}
		}
	}

	/// Save into whichever slot was found above.
	if (nicknum != 0) {
		g_pChat->SetNick(nicknum, loginName, password, passwordPlaintext);
		return(g_pChat->SetNickLocale(nicknum, (Locale)locale));
	}

	/// All 32 slots are in use -- cycle to the slot after the last one we wrote.
	int num = Session.LastNicknameSlot;
	if (Session.LastNicknameSlot == -1) {
		num = 1;
	}
	last = num++;
	Session.LastNicknameSlot = num;
	if (num > 32) {
		Session.LastNicknameSlot = 1;
	}
	if (last != NULL) {
		g_pChat->SetNick(last, loginName, password, passwordPlaintext);
		return(g_pChat->SetNickLocale(last, (Locale)locale));
	}
	return(num);
}

/// <summary>
/// Dialog procedure for the WOL login dialog (IDD_WOL_LOGIN). Populates the stored-nickname,
/// locale, and server list controls, lets the player pick or delete a stored login, create a
/// new account, and handles the Login/Cancel buttons.
/// </summary>
/// <param name="win">Handle of the dialog window.</param>
/// <param name="uMsg">Window message being processed.</param>
/// <param name="wParam">Message-specific parameter.</param>
/// <param name="lParam">Message-specific parameter.</param>
/// <returns>TRUE if the message was handled, FALSE otherwise.</returns>
BOOL CALLBACK WOL_Login_Dialog_Proc(HWND win, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LPCSTR nick = NULL;
	LPCSTR pass = NULL;

	switch (uMsg) {

		/*
		 * Re-populate the login dialog's nick/locale/server controls. This is
		 * triggered both when the dialog is first subclassed and when a delayed
		 * WOL login-OK notification arrives.
		 */
		case OD_SUBCLASSED:
		case WOL_LOGIN_OK: {
			SendDlgItemMessage(win, IDC_NICKNAME, CB_LIMITTEXT, 9, 0);
			SendDlgItemMessage(win, IDC_PASSWORD, EM_LIMITTEXT, 8, 0);
			SendDlgItemMessage(win, IDC_STORELOGIN, BM_SETCHECK, Session.StoreNickname, 0);
			EnableWindow(GetDlgItem(win, 3), FALSE);

			/*
			 * Fill the nickname combo box from the stored nick list.
			 */
			SendDlgItemMessage(win, IDC_NICKNAME, CB_RESETCONTENT, 0, 0);
			LPCSTR initnick;
			LPCSTR initpass;
			for (int i = 1; i <= 32; i++) {
				initnick = NULL;
				initpass = NULL;
				g_pChat->GetNick(i, &initnick, &initpass);
				if (initnick != NULL) {
					if (strlen(initnick) != 0) {
						int item = SendDlgItemMessage(win, IDC_NICKNAME, CB_ADDSTRING, 0, (LPARAM)initnick);
						SendDlgItemMessage(win, IDC_NICKNAME, CB_SETITEMDATA, item, i);
						if (strcmp(initnick, Session.Handle) == 0) {
							SendDlgItemMessage(win, IDC_NICKNAME, CB_SETCURSEL, item, 0);
							SetDlgItemText(win, IDC_PASSWORD, initpass);
							EnableWindow(GetDlgItem(win, 3), TRUE);
						}
					}
				}
			}

			/*
			 * Fill the locale combo box.
			 */
			SendDlgItemMessage(win, IDC_SERVER, CB_RESETCONTENT, 0, 0);
			int count = 0;
			g_pChat->GetLocaleCount(&count);
			if (count > 0) {
				for (int loc = count - 1; loc >= 0; loc--) {
					const char *locname;
					if (g_pChat->GetLocaleString(&locname, (Locale)loc) == 0) {
						DebugString("Adding country: %s\n", locname);
						int item = SendDlgItemMessage(win, IDC_SERVER, loc < 2 ? CB_INSERTSTRING : CB_ADDSTRING, 0, (LPARAM)locname);
						SendDlgItemMessage(win, IDC_SERVER, CB_SETITEMDATA, item, loc);
						if (loc == Session.Locale) {
							SendDlgItemMessage(win, IDC_SERVER, CB_SETCURSEL, item, 0);
						}
					}
				}
			}

			/*
			 * Fill the server list box.
			 */
			SendDlgItemMessage(win, IDC_SERVERLIST, LB_RESETCONTENT, 0, 0);
			Server * server;
			for (int j = (g_Servers.length() != 1); j < g_Servers.length(); j++) {
				CHAR text[256];
				server = NULL;
				g_Servers.getPointer(&server, j);
				DebugString("Server found %s\n", (char *)server->name);

				char * sep = strchr((char *)server->name, ':');
				if (sep == NULL) {
					sep = (char *)server->name;
				} else {
					sep = sep + 1;
				}
				strcpy(text, sep);

				int item = SendDlgItemMessage(win, IDC_SERVERLIST, LB_INSERTSTRING, -1, (LPARAM)text);
				SendDlgItemMessage(win, IDC_SERVERLIST, LB_SETITEMDATA, item, j);
				if (Session.PreferredServer != NULL) {
					if (strcmp(text, Session.PreferredServer) == 0) {
						SendDlgItemMessage(win, IDC_SERVERLIST, LB_SETCURSEL, item, 0);
						if (Session.PreferredServer != NULL) {
							delete Session.PreferredServer;
						}
						Session.PreferredServer = new char[strlen(text) + 1];
						strcpy(Session.PreferredServer, text);
						g_CurrentServerIndex = j;
						g_TargetServerIndex = j;
						Session.Write_MultiPlayer_Settings();
					}
				}
			}

			int sel = SendDlgItemMessage(win, IDC_SERVERLIST, LB_GETCURSEL, -1, 0);
			if (sel > -1) {
				SendDlgItemMessage(win, IDC_SERVERLIST, LB_SETTOPINDEX, sel, 0);
			} else {
				SendDlgItemMessage(win, IDC_SERVERLIST, LB_SETCURSEL, 0, 0);
			}
		}
			return(FALSE);

		/*
		 * Dispatch button/control notifications for the login dialog.
		 */
		case WM_COMMAND: {
			switch (LOWORD(wParam)) {

				/*
				 * Login button -- validate the locale, save the entered nickname and
				 * password (optionally storing them for next time), then close the
				 * dialog reporting success.
				 */
				case IDOK: {
					if (Session.Locale == 0) {
						if (ODMessageBox(Fetch_String(TXT_WARNING_LOCALE_UNKNOWN), MB_OKCANCEL, WOL_Wait_Callback) == IDCANCEL) {
							return(FALSE);
						}
					}

					memset(g_NickName, 0, 30);
					memset(g_LoginPassword, 0, 30);
					GetDlgItemText(win, IDC_NICKNAME, g_NickName, 29);
					GetDlgItemText(win, IDC_PASSWORD, g_LoginPassword, 29);

					g_PlaintextPassword = TRUE;
					strcpy(Session.Handle, g_NickName);
					for (int i = 1; i <= 32; i++) {
						nick = NULL;
						pass = NULL;
						g_pChat->GetNick(i, &nick, &pass);
						if (nick != NULL) {
							if (strlen(nick) != 0 && strcmp(pass, g_LoginPassword) == 0) {
								g_PlaintextPassword = FALSE;
								break;
							}
						}
					}

					if (SendDlgItemMessage(win, IDC_STORELOGIN, BM_GETCHECK, 0, 0)) {
						Session.StoreNickname = TRUE;
						StoreLogin(g_NickName, g_LoginPassword, g_PlaintextPassword, Session.Locale);
					} else {
						Session.StoreNickname = FALSE;
						if (Session.Locale == 0) {
							Session.Locale = Session.LastLocale;
						}
					}

					Session.Write_MultiPlayer_Settings();
					WS_Destroy_Dialog(win, IDOK);
					return(TRUE);
				}

				/// Cancel button -- close the dialog without logging in.
				case IDCANCEL: {
					WS_Destroy_Dialog(win, IDCANCEL);
					return(TRUE);
				}

				/*
				 * Delete the currently selected nickname from the stored list.
				 */
				case 3: {
					char delnick[256];
					if (GetDlgItemText(win, IDC_NICKNAME, delnick, 256) > 0) {
						char message[256];
						sprintf(message, Fetch_String(TXT_DELETE_USER), delnick);
						if (ODMessageBox(message, MB_YESNO, WOL_Wait_Callback) == IDYES) {
							int found = 0;
							LPCSTR delcurnick;
							LPCSTR delcurpass;
							for (int i = 1; i <= 32; i++) {
								delcurnick = NULL;
								delcurpass = NULL;
								g_pChat->GetNick(i, &delcurnick, &delcurpass);
								if (delcurnick != NULL && strcmp(delcurnick, delnick) == 0) {
									found = i;
									break;
								}
							}
							if (found != 0) {
								g_pChat->SetNick(found, "", "", 0);
								EnableWindow(GetDlgItem(win, 3), FALSE);
								SendMessage(win, WOL_LOGIN_OK, 0, 0);
								return(FALSE);
							}
						}
					}
				}
					return(FALSE);

				/// "New Account" button -- open the new-nickname dialog.
				case IDC_NEWNICK: {
					NewLogin(win);
				}
					return(FALSE);

				/*
				 * A different stored nickname was picked from the combo box.
				 */
				case IDC_NICKNAME: {
					if (HIWORD(wParam) == CBN_SELCHANGE) {
						char selnick[256];
						int item = SendDlgItemMessage(win, IDC_NICKNAME, CB_GETCURSEL, 0, 0);
						if (SendDlgItemMessage(win, IDC_NICKNAME, CB_GETLBTEXT, item, (LPARAM)selnick) > 0) {
							FetchLogin(selnick, win);
							EnableWindow(GetDlgItem(win, 3), TRUE);
							return(FALSE);
						}
					}
				}
					return(FALSE);

				/*
				 * A different locale was picked from the combo box.
				 */
				case IDC_SERVER: {
					if (HIWORD(wParam) == CBN_SELCHANGE) {
						int item = SendDlgItemMessage(win, IDC_SERVER, CB_GETCURSEL, 0, 0);
						int locale = SendDlgItemMessage(win, IDC_SERVER, CB_GETITEMDATA, item, 0);
						if (locale < 0) {
							locale = 0;
						}
						Session.Locale = locale;
						if (locale != 0) {
							Session.Write_MultiPlayer_Settings();
						}
						if (Session.Locale >= 0) {
							int sel = SendDlgItemMessage(win, IDC_NICKNAME, CB_GETCURSEL, 0, 0);
							if (sel > -1) {
								int nicknum = SendDlgItemMessage(win, IDC_NICKNAME, CB_GETITEMDATA, sel, 0);
								if (nicknum >= 1) {
									g_pChat->SetNickLocale(nicknum, (Locale)Session.Locale);
									return(FALSE);
								}
							}
						}
					}
				}
					return(FALSE);

				/*
				 * A different server was picked from the list box.
				 */
				case IDC_SERVERLIST: {
					if (HIWORD(wParam) == LBN_SELCHANGE) {
						char text[256];
						int sel = SendDlgItemMessage(win, IDC_SERVERLIST, LB_GETCURSEL, 0, 0);
						if (sel > -1) {
							SendDlgItemMessage(win, IDC_SERVERLIST, LB_GETTEXT, sel, (LPARAM)text);
							if (Session.PreferredServer != NULL) {
								delete Session.PreferredServer;
							}
							Session.PreferredServer = new char[strlen(text) + 1];
							strcpy(Session.PreferredServer, text);
							g_CurrentServerIndex = SendDlgItemMessage(win, IDC_SERVERLIST, LB_GETITEMDATA, sel, 0);
							g_TargetServerIndex = g_CurrentServerIndex;
							Session.Write_MultiPlayer_Settings();
							return(FALSE);
						}
					}
				}
					return(FALSE);

				default:
					return(FALSE);
			}
		}
			return(FALSE);

		/// Forward owner-draw controls to the shared owner-draw painter.
		case WM_DRAWITEM:
			OwnerDraw::Draw_Item((LPDRAWITEMSTRUCT)lParam);
			return(TRUE);

		/// Suppress the default background erase; WM_PAINT redraws it fully.
		case WM_ERASEBKGND:
			return(TRUE);

		/// Repaint the dialog's background image.
		case WM_PAINT:
			OwnerDraw::Draw_Dialog_Back(win);
			ValidateRect(win, NULL);
			return(TRUE);

		default:
			return(FALSE);
	}
	return(FALSE);
}

/// <summary>
/// Dialog procedure for the main WOL lobby window (IDD_WOL_MAIN). Drives the channel/user
/// list panels and chat input, and dispatches per-control commands (join/leave a channel,
/// create a channel or game, kick/ban/squelch users, speak/emote, resize the list panels,
/// refresh the channel list, quit) from its nested WM_COMMAND switch. Shared toolbar buttons
/// are handled first by forwarding to WOL_Button_Bar_Proc.
/// </summary>
/// <param name="win">Handle of the dialog window.</param>
/// <param name="uMsg">Window message being processed.</param>
/// <param name="wParam">Message-specific parameter.</param>
/// <param name="lParam">Message-specific parameter.</param>
/// <returns>TRUE if the message was handled, FALSE otherwise.</returns>
BOOL CALLBACK WOL_Main_Dialog_Proc(HWND win, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	/// Shared toolbar (help/ladder/options/find/privacy) is handled by WOL_Button_Bar_Proc first.
	if (WOL_Button_Bar_Proc(win, uMsg, wParam, lParam)) {
		return(TRUE);
	}

	switch (uMsg) {

		/*
		 * Dispatch button/list-control notifications for the lobby window. Most handlers
		 * return(FALSE) directly; falling through with break reaches the shared channel
		 * list refresh below the switch (used by IDC_REFCHAN).
		 */
		case WM_COMMAND: {
			switch (LOWORD(wParam)) {

				/// Quit button -- signal the main loop to exit.
				case IDEXIT: {
					SetEvent(g_WaitEventHandles[EV_EXIT]);
					break;
				}

				/*
				 * Toggle between the "Join" and "Leave" buttons for the currently
				 * selected channel by re-issuing the channel selection notification.
				 */
				case IDC_JOINLEAVE: {
					if ((g_CurrentChannel.name[0]) && (CurrentLevel != WOL_LEVEL_SERVERS)) {
						SendMessage(win, WM_COMMAND, MAKEWPARAM(IDC_CHANNELS, LBN_DBLCLK), (LPARAM)NULL);
					} else if (g_CurrentChannel.name[0] == 0) {
						SendMessage(win, WM_COMMAND, MAKEWPARAM(IDC_CHANNELS, LBN_DBLCLK), (LPARAM)NULL);
					}
					break;
				}

				/*
				 * Refresh the channel list.
				 */
				case IDC_REFCHAN: {
					int requested_type = CHANNELTYPE_CHAT;
					if (CurrentLevel == WOL_LEVEL_GAMES) requested_type = CHANNELTYPE_GAME;
					if (g_pChat->RequestChannelList(requested_type, 1) == S_OK) {
						unsigned int type = requested_type;
						g_ChanListQueue.addTail(type);
					}
					break;
				}

				/*
				 * Resize the channel/user panels by sliding the divider up or down.
				 */
				case IDC_UP:
				case IDC_DOWN: {
					int i;
					HWND hwnd;
					RECT rect;
					RECT display_rect;
					int move = 50;

					if (LOWORD(wParam) == IDC_UP) {
						move = -50;
					}

					hwnd = GetDlgItem(win, IDC_USERS);
					Get_Display_Rect(hwnd, &rect);
					if (rect.bottom - rect.top - move <= 64) {
						break;
					}
					hwnd = GetDlgItem(win, IDC_CHANNELS);
					Get_Display_Rect(hwnd, &rect);
					if (rect.bottom - rect.top + move <= 64) {
						break;
					}

					Get_Display_Rect(win, &display_rect);

					int windows_to_move[] = {IDC_USERSTATIC, IDC_USERCOUNT, IDC_UP, IDC_DOWN};
					for (i = 0; i < 4; i++) {
						hwnd = GetDlgItem(win, windows_to_move[i]);
						ShowWindow(hwnd, SW_HIDE);
						if (hwnd != NULL) {
							Get_Display_Rect(hwnd, &rect);
							MoveWindow(hwnd, rect.left - display_rect.left, rect.top + move - display_rect.top, rect.right - rect.left, rect.bottom - rect.top, TRUE);
						}
					}

					HWND userswin = GetDlgItem(win, IDC_USERS);
					HWND chan_list = GetDlgItem(win, IDC_CHANNELS);
					ShowWindow(userswin, SW_HIDE);
					ShowWindow(chan_list, SW_HIDE);
					Get_Display_Rect(userswin, &rect);
					MoveWindow(userswin, rect.left - display_rect.left, rect.top + move - display_rect.top, rect.right - rect.left, rect.bottom - rect.top - move, TRUE);
					Get_Display_Rect(chan_list, &rect);
					MoveWindow(chan_list, rect.left - display_rect.left, rect.top - display_rect.top, rect.right - rect.left, rect.bottom - rect.top + move, TRUE);
					ShowWindow(chan_list, SW_SHOW);
					ShowWindow(userswin, SW_SHOW);

					for (i = 0; i < 4; i++) {
						ShowWindow(GetDlgItem(win, windows_to_move[i]), SW_SHOW);
					}

					WOL_Wait_Callback();
					WOL_Wait_Callback();
					Draw_Player_List();
					Draw_Channel_List();
					break;
				}

				/*
				 * Create a new chat channel or hosted game.
				 */
				case IDC_NEWCHAN: {
					/// Creating a user chat channel: prompt for a name, then request creation.
					if (CurrentLevel == WOL_LEVEL_USERCHAT) {
						HWND dlg = WS_Create_Dialog(ProgramInstance, IDD_WOL_NEWCHAT, MainWindow, (DLGPROC)WOL_New_Chat_Dialog_Proc, FALSE);
						Center_Window_Within_Window(dlg);
						OwnerDraw::Subclass_Dialog(dlg, 0);
						SendMessage(dlg, OD_SETTOP, 0, 1);
						ShowWindow(dlg, SW_NORMAL);

						if (WS_Wait_Dialog(dlg, WOL_Wait_Callback) == IDCANCEL) {
							break;
						}

						g_pChat->RequestChannelLeave();

						Channel new_chan;
						memset(&new_chan, 0, sizeof(new_chan));

						char chan_name[128];
						WS_Get_Saved_Value(IDC_CHANNAME, (unsigned char *)chan_name, sizeof(chan_name));
						strcpy((char *)new_chan.name, chan_name);
						new_chan.type = CHANNELTYPE_CHAT;

						if (g_ChannelCount) {
							Show_Wait_Window(WOL_WAIT_CHANNEL_LEAVE | WOL_WAIT_IDLE, false);
						}

						while (g_ChannelCount) {
							Sleep(20);
							WOL_Wait_Callback();
						}

						HRESULT retval = g_pChat->RequestChannelCreate(&new_chan);
						if (retval == CHAT_E_PARAMERROR) {
							Close_Wait_Window(WOL_WAIT_IDLE);
							ODMessageBox(Fetch_String(TXT_BAD_CHANNAME), MB_OK, WOL_Wait_Callback);
						} else if (retval == S_OK) {
							Show_Wait_Window(WOL_WAIT_CHANNEL_CREATE, false);
							Close_Wait_Window(WOL_WAIT_IDLE);
							Show_Wait_Window(WOL_WAIT_CHANNEL_CREATE);
							CurrentLevel = WOL_LEVEL_BACK_USERCHAT;
							Draw_Channel_List();
						} else {
							Close_Wait_Window(WOL_WAIT_IDLE);
							ODMessageBox(Fetch_String(TXT_CANTCREATE_CHAN), MB_OK, WOL_Wait_Callback);
						}
					} else if (CurrentLevel == WOL_LEVEL_GAMES) {

						/*
						 * Creating a hosted game: prompt for game options, build the
						 * channel record (players, key, exinfo, tournament/clan flags),
						 * then request creation and open the game options dialog.
						 */
						if (Session.IsWDT) {
							strncpy(Session.Options.ScenarioDescription, Fetch_String(TXT_RANDOM_MAP_DESCRIPTION), sizeof(Session.Options.ScenarioDescription));
							strncpy(Session.ScenarioFileName, "RandMap.Sed", sizeof(Session.ScenarioFileName));
							Session.Options.ScenarioIndex = -1;
						}

						HWND dlg = WS_Create_Dialog(ProgramInstance, Session.IsWDT ? IDD_WOL_NEWGAME_WDT : IDD_WOL_NEWGAME, MainWindow, (DLGPROC)WOL_New_Game_Dialog_Proc, FALSE);
						Center_Window_Within_Window(dlg);

						OwnerDraw::Subclass_Dialog(dlg, 0);
						SendMessage(dlg, OD_SETTOP, 0, 1);
						ShowWindow(dlg, SW_NORMAL);

						if (WS_Wait_Dialog(dlg, WOL_Wait_Callback) == IDCANCEL) {
							break;
						}
						g_pChat->RequestChannelLeave();

						Channel new_chan;
						memset(&new_chan, 0, sizeof(new_chan));
						sprintf((char *)new_chan.name, "%s's_game", g_NickName);
						new_chan.type = CHANNELTYPE_GAME;
						new_chan.minUsers = 1;

						if (g_MaxPlayers < 2) {
							g_MaxPlayers = MAX_PLAYERS;
						}
						strcpy((char *)new_chan.key, g_PendingChanPass);

						new_chan.maxUsers = g_MaxPlayers;

						char exinfo[80];
						memset(exinfo, 0, sizeof(exinfo));
						Encode_Channel_ExInfo(exinfo);
						exinfo[0] = 0;
						strcpy((char *)new_chan.exInfo, exinfo);

						if (!Session.IsWDT) {
							int tourney_check_state = 0;
							WS_Get_Saved_Value(IDC_NEWGAME_TOURNAMENT, (unsigned char *)&tourney_check_state, sizeof(tourney_check_state));
							new_chan.tournament = tourney_check_state;
							int battleclancheck = 0;
							WS_Get_Saved_Value(IDC_NEWGAME_BATTLECLAN, (unsigned char *)&battleclancheck, sizeof(battleclancheck));
							if (battleclancheck) {
								new_chan.reserved |= 0x100;
							}
						} else {
							new_chan.tournament = 1;
						}
						new_chan.reserved += g_CurrentLobby;
						if (Addon_Enabled(ADDON_FIRESTORM)) {
							new_chan.reserved |= 0x1000;
						}

						if (g_ChannelCount) {
							Show_Wait_Window(WOL_WAIT_CHANNEL_LEAVE | WOL_WAIT_IDLE, false);
							while (g_ChannelCount) {
								Sleep(20);
								WOL_Wait_Callback();
							}
						}

						HRESULT retval = g_pChat->RequestChannelCreate(&new_chan);

						if (retval == CHAT_E_PARAMERROR) {
							ODMessageBox(Fetch_String(TXT_BAD_CHANKEY), MB_OK, WOL_Wait_Callback);
						} else if (retval == S_OK) {
							Show_Wait_Window(WOL_WAIT_CHANNEL_CREATE, false);
							Close_Wait_Window(WOL_WAIT_IDLE);
							Show_Wait_Window(WOL_WAIT_CHANNEL_CREATE);

							ShowWindow(win, SW_HIDE);

							dlg = WS_Create_Dialog(ProgramInstance, IDD_WOL_GAMEOPT, MainWindow, (DLGPROC)WOL_Game_Options_Proc, FALSE);
							Center_Window_Within_Window(dlg);
							OwnerDraw::Subclass_Dialog(dlg, 0);
							SendMessage(dlg, OD_SETTOP, 0, 1);
							ShowWindow(dlg, SW_NORMAL);
							Draw_Player_List(0);
							return(FALSE);
						} else {
							ODMessageBox(Fetch_String(TXT_CANTCREATE_CHAN), MB_OK, WOL_Wait_Callback);
						}
						Close_Wait_Window(WOL_WAIT_IDLE);
					} else {
						PMessagePrintf(-1, "You can't create a channel here");
					}
				}
				break;

				/*
				 * Squelch, ban, or kick the selected user(s).
				 */
				case IDC_SQUELCH:
				case IDC_KICK:
				case IDC_BAN: {
					User user;
					int squelch;
					Wstring name;
					bool selected = false;
					bool not_operator = false;
					int i;
					HWND userwin = GetDlgItem(WS_Find_Dialog(IDD_WOL_MAIN), IDC_USERS);

					if (LOWORD(wParam) == IDC_KICK || LOWORD(wParam) == IDC_BAN) {
						for (i = 0; i < g_UserList.length(); i++) {
							g_UserList.get(user, i);
							if (stricmp((char *)user.name, g_NickName) == 0 && (user.flags & CHAT_USER_CHANNELOWNER) == 0) {
								PMessagePrintf(-1, Fetch_String(TXT_NOTCHANOP));
								not_operator = true;
								break;
							}
						}
						if (not_operator) {
							break;
						}
					}

					Dictionary<Wstring, bool> lbdict(Wstring_Hash);
					LBSaveSelections(userwin, lbdict);

					if (lbdict.getEntries()) {
						Sound_Effect(Rule->GenericBeep);
					}

					bool found = false;
					while (lbdict.removeAny(name, selected)) {
						strcpy((char *)user.name, name.get());
						if (stricmp((char *)user.name, g_NickName) == 0) {
							continue;
						}
						found = true;

						if (LOWORD(wParam) == IDC_SQUELCH) {
							if (g_pChat->GetSquelch(&user) == S_OK) {
								squelch = 0;
							} else {
								squelch = 1;
							}
							g_pChat->SetSquelch(&user, squelch);
						} else if (LOWORD(wParam) == IDC_KICK) {
							g_pChat->RequestUserKick(&user);
						} else if (LOWORD(wParam) == IDC_BAN) {
							g_pChat->RequestChannelBan((char *)user.name, 1);
							g_pChat->RequestUserKick(&user);
						}
					}
					if (!found) {
						PMessagePrintf(-1, Fetch_String(TXT_SEL_USER));
					}
					Draw_Player_List(1);

					return(TRUE);
				}

				/*
				 * The user list selection changed -- update the user count caption.
				 */
				case IDC_USERS: {
					if (HIWORD(wParam) == LBN_SELCHANGE) {
						int sel_count = SendDlgItemMessage(win, IDC_USERS, LB_GETSELCOUNT, 0, 0);
						//int user_count = g_UserList.length();
						char str[20];
						if (sel_count > 0) {
							sprintf(str, "%d  (%d)", g_UserList.length(), sel_count);
						} else {
							sprintf(str, "%d", g_UserList.length());
						}
						SendDlgItemMessage(win, IDC_USERCOUNT, WM_SETTEXT, 0, (LPARAM)str);
					}
					break;
				}

				/*
				 * A channel was selected, double-clicked, or navigation requested.
				 */
				case IDC_CHANNELS: {
					int cindex = SendDlgItemMessage(win, IDC_CHANNELS, LB_GETCURSEL, 0, 0);
					if (cindex < 0) {
						cindex = 0;
					}
					int extra = SendDlgItemMessage(win, IDC_CHANNELS, LB_GETITEMDATA, cindex, 0);

					Channel chan;

					if (HIWORD(wParam) == LBN_SELCHANGE) {
						if (g_CurrentChannel.name[0] == '\0' || CurrentLevel == WOL_LEVEL_GAMES) {

							int idx = SendDlgItemMessage(win, IDC_CHANNELS, LB_GETCURSEL, 0, 0);
							int enable = false;
							if (idx != LB_ERR && idx != 0) {
								enable = true;
							}
							if (SendDlgItemMessage(win, IDC_CHANNELS, LB_GETSEL, idx, 0) <= 0) {
								enable = false;
							}
							EnableWindow(GetDlgItem(win, IDC_JOINLEAVE), enable);
						}
					}

					if (HIWORD(wParam) == LBN_DBLCLK) {
						if (CurrentLevel != WOL_LEVEL_GAMES) {
							g_pChat->RequestChannelLeave();
						}

						if (cindex == 0) {
							if (Session.IsWDT) {
								SetEvent(g_WaitEventHandles[EV_EXIT]);
								return(TRUE);
							}
							if (CurrentLevel != WOL_LEVEL_SERVERS) {
								if (CurrentLevel == WOL_LEVEL_GAMES) {
									g_pChat->RequestChannelLeave();
								}
								GoBack();
								Draw_Channel_List();
								return(FALSE);
							}
						}

						/*
						 * Pick the top-level list root when a root entry is double-clicked.
						 */
						if (CurrentLevel == WOL_LEVEL_SERVERS) {
							CurrentLevel = WOL_LEVEL_ROOT;
							g_TargetServerIndex = cindex + 1;
						} else if (CurrentLevel == WOL_LEVEL_ROOT) {
							if (cindex != 2) {
								int type = CHANNELTYPE_CHAT;
								if (cindex == 1) {
									type = CHANNELTYPE_GAME;
								}
								bool retval = Switch_Server(type);
								if (retval == false) {
									SetEvent(g_WaitEventHandles[EV_EXIT]);
								}
							}
							if (cindex == 1) {
								CurrentLevel = WOL_LEVEL_LOBBIES;
							} else if (cindex == 2) {
								CurrentLevel = WOL_LEVEL_OTHER_GAMES;
							} else if (cindex == 3) {
								CurrentLevel = WOL_LEVEL_OFFICIALCHAT;
							} else if (cindex == 4) {
								CurrentLevel = WOL_LEVEL_USERCHAT;
							}
						} else if (CurrentLevel == WOL_LEVEL_LOBBIES) {
							/*
							 * Entering a lobby -- switch to its game list and auto-join its
							 * (fixed-key) chat channel.
							 */
							CurrentLevel = WOL_LEVEL_GAMES;
							if (extra >= 0 && extra < g_UserChannels.length()) {
								g_UserChannels.get(chan, extra);
								strcpy((char *)chan.key, WOL_LOBBY_PASSWORD);
								if (g_ChannelCount) {
									Show_Wait_Window(WOL_WAIT_CHANNEL_LEAVE);
								}
								s_TempChannel = chan;
								g_pChat->RequestChannelJoin(&chan);
							}
						/// Join the selected user or official chat channel.
						} else if (CurrentLevel == WOL_LEVEL_USERCHAT || CurrentLevel == WOL_LEVEL_OFFICIALCHAT) {
							if (CurrentLevel == WOL_LEVEL_USERCHAT) {
								CurrentLevel = WOL_LEVEL_BACK_USERCHAT;
							}
							if (CurrentLevel == WOL_LEVEL_OFFICIALCHAT) {
								CurrentLevel = WOL_LEVEL_BACK_OFFICIALCHAT;
							}
							memset(&chan, 0, sizeof(chan));
							g_UserChannels.get(chan, extra);
							if (g_ChannelCount) {
								Show_Wait_Window(WOL_WAIT_CHANNEL_LEAVE);
							}
							s_TempChannel = chan;
							g_pChat->RequestChannelJoin(&chan);
						/*
						 * Join the selected hosted game: check clan/password gating, then
						 * confirm the host's build number and rules CRC (and, for WDT,
						 * the chosen house has a free slot) before actually joining.
						 */
						} else if (CurrentLevel == WOL_LEVEL_GAMES) {
							bool may_join = true;
							char password[17];
							memset(password,0,sizeof(password));
							memset(&chan, 0, sizeof(chan));

							g_GameChannelList.get(chan, extra);
							if ((chan.reserved & 0x100) != 0 && g_OwnSquadID < 1) {
								PMessagePrintf(-1, Fetch_String(TXT_NO_CLAN));
								may_join = false;
							}
							if (chan.flags & CHAN_MODE_KEY) {
								HWND dlg = WS_Create_Dialog(ProgramInstance, IDD_WOL_PASSWORD, MainWindow, (DLGPROC)WOL_Password_Dialog_Proc, FALSE);
								Center_Window_Within_Window(dlg);
								OwnerDraw::Subclass_Dialog(dlg, 0);
								ShowWindow(dlg, SW_NORMAL);
								if (WS_Wait_Dialog(dlg, WOL_Wait_Callback) == IDCANCEL) {
									may_join = false;
								} else {
									WS_Get_Saved_Value(IDC_PASSWORD, (unsigned char *)password, sizeof(password));
								}
							}
							if (may_join) {
								char exinfo_copy[256];
								strcpy(exinfo_copy, (char *)chan.exInfo);
								int buildnum = Build_Number();
								int hostbuild = 0;
								char * cptr = strtok(exinfo_copy, ",");
								if (cptr != NULL) {
									hostbuild = atol(cptr);
								}
								int hostcrc = 0;
								int crc = GetINIHash();
								cptr = strtok(NULL, ",");
								if (cptr != NULL) {
									hostcrc = atol(cptr);
								}
								if (buildnum != hostbuild || crc != hostcrc) {
									ODMessageBox(Fetch_String(TXT_MISMATCH), 0, WOL_Wait_Callback, 0);
									may_join = false;
								}
								bool start_game = false;
								if (Session.IsWDT) {
									if (may_join) {
										strtok(NULL, ",");
										strtok(NULL, ",");
										strtok(NULL, ",");
										strtok(NULL, ",");
										strtok(NULL, ",");
										cptr = strtok(NULL, ",");
										if (cptr != NULL) {
											int house;
											house = atol(cptr);
											if (house == 0) {
												may_join = false;
											}
											bool nofree = false;
											if (Session.House == HOUSE_GOOD) {
												if (house == 2) {
													nofree = true;
												}
											} else if (Session.House == HOUSE_BAD) {
												if (house == 1) {
													nofree = true;
												}
											}
											if (nofree || !may_join) {
												ODMessageBox(Fetch_String(TXT_NO_FREE_SLOTS), 0, WOL_Wait_Callback, 0);
												Draw_Channel_List();
												return(FALSE);
											}
											start_game = true;
										}
									}
								}
								if (!start_game && may_join) {
									start_game = true;
								}
								if (start_game) {
									g_pChat->RequestChannelLeave();
									PMessagePrintf(-1, Fetch_String(TXT_JOININGCHAN), chan.name);
									CurrentLevel = WOL_LEVEL_BACK_LOBBIES;
									if (g_ChannelCount) {
										Show_Wait_Window(WOL_WAIT_CHANNEL_LEAVE);
									}
									strcpy((char *)chan.key, password);
									s_TempChannel = chan;
									g_pChat->RequestChannelJoin(&chan);
								}
							}
						/// "Other games" entry -- open its info URL in a browser instead of joining.
						} else if (CurrentLevel == WOL_LEVEL_OTHER_GAMES) {
							int gtype = extra;
							unsigned char * bitmap;
							int bmp_bytes;
							LPCSTR name;
							LPCSTR url;
							g_pChat->GetGametypeInfo(gtype, g_ListIconHeight, &bitmap, &bmp_bytes, &name, &url);
							ViewHTML(url, 0);
						}
						Draw_Channel_List();
					}
					break;
				}

				/*
				 * Speak: send the typed message to the current channel.
				 */
				case IDC_INPUT: {
					char input[257];
					int len;

					SendDlgItemMessage(win, IDC_INPUT, WM_GETTEXT, sizeof(input)-1, (LPARAM)input);
					len = strlen(input);
					if (HIWORD(wParam) == EN_MAXTEXT) {
						SendDlgItemMessage(win, IDC_INPUT, WM_SETTEXT, 0, (LPARAM)"");
						if (len > 2) {
							if (strlen((char *)g_CurrentChannel.name) == 0) {
								PMessagePrintf(-1, Fetch_String(TXT_NOT_IN_CHAN));
							} else {
								int color = ColorMe;
								if (Send_Chat_Message(input) == 1) {
									color = ColorPriv;
								}
								PMessagePrintf(color, "[%s] %s", g_NickName, input);
							}
						}
					}
					break;
				}

				/*
				 * Action: send the typed message as an emote action.
				 */
				case IDC_ACTION: {
					char input[257];
					SendDlgItemMessage(win, IDC_INPUT, WM_GETTEXT, sizeof(input)-1, (LPARAM)input);
					SendDlgItemMessage(win, IDC_INPUT, WM_SETTEXT, 0, (LPARAM)"");
					SetFocus(GetDlgItem(win, IDC_INPUT));
					int color = ColorAction;
					if (strlen((char *)g_CurrentChannel.name) == 0) {
						PMessagePrintf(-1, Fetch_String(TXT_NOT_IN_CHAN));
					} else if (strlen(input)) {
						if (Send_Chat_Action(input)) {
							color = ColorPrivAction;
						}
						PMessagePrintf(color, "%s %s", g_NickName, input);
					} else {
						PMessagePrintf(-1, Fetch_String(TXT_ENTER_MESSAGE));
					}
					break;
				}

				/*
				 * Send a private squad message.
				 */
				case IDC_SQUADMSG: {
					char input[257];
					SendDlgItemMessage(win, IDC_INPUT, WM_GETTEXT, sizeof(input)-1, (LPARAM)input);
					SendDlgItemMessage(win, IDC_INPUT, WM_SETTEXT, 0, (LPARAM)"");
					SetFocus(GetDlgItem(win, IDC_INPUT));

					if (g_OwnSquadID) {
						User user;
						memset(&user, 0, sizeof(user));
						strcpy((char *)user.name, "0");
						g_pChat->RequestPrivateMessage(&user, input);
					} else {
						PMessagePrintf(-1, Fetch_String(TXT_NOSQUAD));
					}
					break;
				}

				default:
					return(FALSE);
			}
			break;
		}

		/*
		 * Dialog first subclassed -- set up the owner-draw button images/tooltips,
		 * define the channel and user list-view columns, then populate both lists.
		 */
		case OD_SUBCLASSED:
			OwnerDraw::Draw_Dialog_Back(win);

			SendDlgItemMessage(win, IDC_UP, OD_SETIMAGE, 0, (LPARAM)SurfaceCache.GetSurface("uparrow.pcx"));
			SendDlgItemMessage(win, IDC_DOWN, OD_SETIMAGE, 0, (LPARAM)SurfaceCache.GetSurface("dnarrow.pcx"));

			SendDlgItemMessage(win, IDC_USERS, OD_TOOLTIPS, 0, (LPARAM)1);
			SendDlgItemMessage(win, IDC_CHANNELS, OD_TOOLTIPS, 0, (LPARAM)1);

			SendDlgItemMessage(win, IDC_UP, OD_SETIMAGE, 0, (LPARAM)SurfaceCache.GetSurface("arrow_uu.pcx"));
			SendDlgItemMessage(win, IDC_UP, OD_SETALTIMAGE, 0, (LPARAM)SurfaceCache.GetSurface("arrow_du.pcx"));
			SendDlgItemMessage(win, IDC_DOWN, OD_SETIMAGE, 0, (LPARAM)SurfaceCache.GetSurface("arrow_ud.pcx"));
			SendDlgItemMessage(win, IDC_DOWN, OD_SETALTIMAGE, 0, (LPARAM)SurfaceCache.GetSurface("arrow_dd.pcx"));

			SendDlgItemMessage(win, IDC_ACTION, OD_SETIMAGE, 0, (LPARAM)SurfaceCache.GetSurface("wouact.pcx"));
			SendDlgItemMessage(win, IDC_ACTION, OD_SETALTIMAGE, 0, (LPARAM)SurfaceCache.GetSurface("wodact.pcx"));

			SendDlgItemMessage(win, IDC_REFCHAN, OD_SETIMAGE, 0, (LPARAM)SurfaceCache.GetSurface("wouref.pcx"));
			SendDlgItemMessage(win, IDC_REFCHAN, OD_SETALTIMAGE, 0, (LPARAM)SurfaceCache.GetSurface("wodref.pcx"));

			SendDlgItemMessage(win, IDC_ACTION, OD_TOOLTIPS, 0, (LPARAM)1);
			SendDlgItemMessage(win, IDC_REFCHAN, OD_TOOLTIPS, 0, (LPARAM)1);

			g_Col_ChanIcon = 2;
			g_Col_ChanPrivate = 19;
			g_Col_ChanName = 34;
			g_Col_PingTime = 157;
			SendDlgItemMessage(win, IDC_CHANNELS, OD_ADDCOLUMN, 0, (LPARAM)g_Col_ChanPrivate);
			SendDlgItemMessage(win, IDC_CHANNELS, OD_ADDCOLUMN, (WPARAM)(g_Col_PingTime - g_Col_ChanName), (LPARAM)g_Col_ChanName);
			SendDlgItemMessage(win, IDC_CHANNELS, OD_ADDCOLUMN, 0, (LPARAM)g_Col_ChanIcon);
			SendDlgItemMessage(win, IDC_CHANNELS, OD_ADDCOLUMN, 0, (LPARAM)g_Col_PingTime);

			g_Col_Username = 35;
			g_Col_Chanop = 2;
			g_Col_Squelch = 18;
			g_Col_Rank = 133;
			SendDlgItemMessage(win, IDC_USERS, OD_ADDCOLUMN, (WPARAM)(g_Col_Rank - g_Col_Username), (LPARAM)g_Col_Username);
			SendDlgItemMessage(win, IDC_USERS, OD_ADDCOLUMN, 0, (LPARAM)g_Col_Chanop);
			SendDlgItemMessage(win, IDC_USERS, OD_ADDCOLUMN, 0, (LPARAM)g_Col_Squelch);
			SendDlgItemMessage(win, IDC_USERS, OD_ADDCOLUMN, 0, (LPARAM)g_Col_Rank);

			Draw_Channel_List();
			Draw_Player_List(0);
			break;

		/// Forward owner-draw controls to the shared owner-draw painter.
		case WM_DRAWITEM: {
			DRAWITEMSTRUCT * di = (LPDRAWITEMSTRUCT)lParam;
			OwnerDraw::Draw_Item(di);
			return(TRUE);
		}


		/// Suppress the default background erase; WM_PAINT redraws it fully.
		case WM_ERASEBKGND:
			return(TRUE);


		/// Repaint the dialog background; if this isn't the top window, let it repaint instead.
		case WM_PAINT:
			OwnerDraw::Draw_Dialog_Back(win);
			ValidateRect(win, NULL);
			if (g_TopWindow != win) {
				PostMessage(g_TopWindow, WM_PAINT, 0, 0);
				return(FALSE);
			}
			break;

	default:
		return(FALSE);
	}

	return(FALSE);
}

/// <summary>
/// Window procedure for the shared button-bar toolbar embedded in the lobby dialogs.
/// Configures each button's tooltip and normal/pressed images on subclass, supplies
/// tooltip text on request, and dispatches WM_COMMAND to open the Find Game, Ladder,
/// Find Page, Help, Battle Clan/Privacy, and Options dialogs.
/// </summary>
/// <param name="win">Handle of the dialog window.</param>
/// <param name="uMsg">Window message being processed.</param>
/// <param name="wParam">Message-specific parameter.</param>
/// <param name="lParam">Message-specific parameter.</param>
/// <returns>TRUE if the message was handled, FALSE otherwise.</returns>
BOOL WOL_Button_Bar_Proc(HWND win, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
		/// Assign the tooltip text and the normal/pressed images for every toolbar button.
		case OD_SUBCLASSED: {
			SendDlgItemMessage(win, IDC_HELPBTN, OD_TOOLTIPS, 0, (LPARAM)1);
			SendDlgItemMessage(win, IDC_HELPBTN, OD_SETIMAGE, 0, (LPARAM)SurfaceCache.GetSurface("wouhelp.pcx"));
			SendDlgItemMessage(win, IDC_HELPBTN, OD_SETALTIMAGE, 0, (LPARAM)SurfaceCache.GetSurface("wodhelp.pcx"));

			SendDlgItemMessage(win, IDC_LADDER, OD_TOOLTIPS, 0, (LPARAM)1);
			SendDlgItemMessage(win, IDC_LADDER, OD_SETIMAGE, 0, (LPARAM)SurfaceCache.GetSurface("woutrny.pcx"));
			SendDlgItemMessage(win, IDC_LADDER, OD_SETALTIMAGE, 0, (LPARAM)SurfaceCache.GetSurface("wodtrny.pcx"));

			SendDlgItemMessage(win, IDC_OPTIONBTN, OD_TOOLTIPS, 0, (LPARAM)1);
			SendDlgItemMessage(win, IDC_OPTIONBTN, OD_SETIMAGE, 0, (LPARAM)SurfaceCache.GetSurface("wouopt.pcx"));
			SendDlgItemMessage(win, IDC_OPTIONBTN, OD_SETALTIMAGE, 0, (LPARAM)SurfaceCache.GetSurface("wodopt.pcx"));

			SendDlgItemMessage(win, IDC_FINDGAME, OD_TOOLTIPS, 0, (LPARAM)1);
			SendDlgItemMessage(win, IDC_FINDGAME, OD_SETIMAGE, 0, (LPARAM)SurfaceCache.GetSurface("woufgame.pcx"));
			SendDlgItemMessage(win, IDC_FINDGAME, OD_SETALTIMAGE, 0, (LPARAM)SurfaceCache.GetSurface("wodfgame.pcx"));

			SendDlgItemMessage(win, IDC_PRIVACY, OD_TOOLTIPS, 0, (LPARAM)1);
			SendDlgItemMessage(win, IDC_PRIVACY, OD_SETIMAGE, 0, (LPARAM)SurfaceCache.GetSurface("wouclan.pcx"));
			SendDlgItemMessage(win, IDC_PRIVACY, OD_SETALTIMAGE, 0, (LPARAM)SurfaceCache.GetSurface("wodclan.pcx"));

			SendDlgItemMessage(win, IDC_KICK, OD_TOOLTIPS, 0, (LPARAM)1);
			SendDlgItemMessage(win, IDC_KICK, OD_SETIMAGE, 0, (LPARAM)SurfaceCache.GetSurface("woukick.pcx"));
			SendDlgItemMessage(win, IDC_KICK, OD_SETALTIMAGE, 0, (LPARAM)SurfaceCache.GetSurface("wodkick.pcx"));

			SendDlgItemMessage(win, IDC_BAN, OD_TOOLTIPS, 0, (LPARAM)1);
			SendDlgItemMessage(win, IDC_BAN, OD_SETIMAGE, 0, (LPARAM)SurfaceCache.GetSurface("wouban.pcx"));
			SendDlgItemMessage(win, IDC_BAN, OD_SETALTIMAGE, 0, (LPARAM)SurfaceCache.GetSurface("wodban.pcx"));

			SendDlgItemMessage(win, IDC_SQUELCH, OD_TOOLTIPS, 0, (LPARAM)1);
			SendDlgItemMessage(win, IDC_SQUELCH, OD_SETIMAGE, 0, (LPARAM)SurfaceCache.GetSurface("wousqlch.pcx"));
			SendDlgItemMessage(win, IDC_SQUELCH, OD_SETALTIMAGE, 0, (LPARAM)SurfaceCache.GetSurface("wodsqlch.pcx"));

			SendDlgItemMessage(win, IDC_FINDPAGE, OD_TOOLTIPS, 0, (LPARAM)1);
			SendDlgItemMessage(win, IDC_FINDPAGE, OD_SETIMAGE, 0, (LPARAM)SurfaceCache.GetSurface("woufind.pcx"));
			SendDlgItemMessage(win, IDC_FINDPAGE, OD_SETALTIMAGE, 0, (LPARAM)SurfaceCache.GetSurface("wodfind.pcx"));

			SendDlgItemMessage(win, IDC_FINDGAME, OD_TOOLTIPS, 0, (LPARAM)1);
		} break;

		/// Supply the tooltip text for the button under the cursor.
		case OD_GETTIPTEXT: {
			char * string = (char *)lParam;
			HWND tip_item = GetDlgItem(win, wParam);
			GetWindowText(tip_item, string, 127);
			return(0);
		} break;

		/// Dispatch toolbar button clicks to open the corresponding dialog.
		case WM_COMMAND: {
			switch (LOWORD(wParam)) {
				case IDC_FINDGAME: {
					HWND dlg = WS_Create_Dialog(ProgramInstance, IDD_WOL_FINDGAME, MainWindow, (DLGPROC)WOL_Find_Game_Dialog_Proc, FALSE);
					SetFocus(dlg);
					Center_Window_Within_Window(dlg);
					OwnerDraw::Subclass_Dialog(dlg, 0);
					SendMessage(dlg, OD_SETTOP, 0, 1);
					ShowWindow(dlg, SW_NORMAL);
				} break;

				case IDC_LADDER: {
					ShowWindow(WS_Top_Window(), SW_HIDE);
					Draw_Menu_Background();
					HWND dlg = WS_Create_Dialog(ProgramInstance, IDD_WOL_LADDER, MainWindow, (DLGPROC)WOL_Ladder_Dialog_Proc, FALSE);
					Center_Window_Within_Window(dlg);
					OwnerDraw::Subclass_Dialog(dlg, 0);
					SendMessage(dlg, OD_SETTOP, 0, 1);
					ShowWindow(dlg, SW_NORMAL);
				} break;

				case IDC_FINDPAGE: {
					HWND top = MainWindow;
					HWND dlg = WS_Create_Dialog(ProgramInstance, IDD_WOL_FINDPAGE, top, (DLGPROC)WOL_Find_Page_Dialog_Proc, FALSE);
					Center_Window_Within_Window(dlg);
					OwnerDraw::Subclass_Dialog(dlg, 0);
					SendMessage(dlg, OD_SETTOP, 0, 1);
					ShowWindow(dlg, SW_NORMAL);
					WS_Wait_Dialog(dlg, WOL_Wait_Callback);
				} break;

				case IDC_HELPBTN: {
					char * help_url;
					g_pChat->GetHelpURL((const char **)&help_url);
					ViewHTML(help_url, 0);
				} break;

				case IDC_PRIVACY: {
					char help_url[256];
					sprintf(help_url, "http://battleclans.westwood.com/tibsun/index_%d.html", g_LanguageCode);
					ViewHTML(help_url, 0);
				} break;

				case IDC_OPTIONBTN: {
					HWND dlg = WS_Create_Dialog(ProgramInstance, IDD_WOL_OPTIONS, MainWindow, (DLGPROC)WOL_Options_Dialog_Proc, FALSE);
					Center_Window_Within_Window(dlg);

					OwnerDraw::Subclass_Dialog(dlg, 0);
					SendMessage(dlg, OD_SETTOP, 0, 1);
					ShowWindow(dlg, SW_NORMAL);

					if (WS_Wait_Dialog(dlg, WOL_Wait_Callback) == IDCANCEL) break;

					/*
					 * Dialog closed with OK -- pull the checkbox states back out and apply them.
					 */
					int checked;
					WS_Get_Saved_Value(IDC_PAGEME, (unsigned char *)&checked, sizeof(int));
					g_AllowPage = checked;

					WS_Get_Saved_Value(IDC_FINDME, (unsigned char *)&checked, sizeof(int));
					g_AllowFind = checked;

					WS_Get_Saved_Value(IDC_LANGFILT, (unsigned char *)&checked, sizeof(int));
					g_LangFilter = checked;

					WS_Get_Saved_Value(IDC_MUSIC, (unsigned char *)&checked, sizeof(int));
					g_LobbyMusic = checked;

					WS_Get_Saved_Value(IDC_LOBGAME, (unsigned char *)&checked, sizeof(int));
					g_ShowAllChannels = checked;

					Write_WOL_Settings();
					Apply_WOL_Settings();
				} break;

				default:
					return(FALSE);
			}
			return(TRUE);
		} break;
	}
	return(FALSE);
}

/// <summary>
/// Window procedure for the "Find Game" dialog. Sets up the filter controls (player count,
/// ping, rank, tech level, tournament/team/map mode checkboxes, and locale combo) and the
/// game-channel list, rebuilds the filtered channel list on demand, shows details for the
/// selected game, and handles joining a game (build/CRC and WDT slot checks, password
/// prompt, leaving the current channel, and requesting the join).
/// </summary>
/// <param name="win">Handle of the dialog window.</param>
/// <param name="uMsg">Window message being processed.</param>
/// <param name="wParam">Message-specific parameter.</param>
/// <param name="lParam">Message-specific parameter.</param>
/// <returns>TRUE if the message was handled, FALSE otherwise.</returns>
BOOL CALLBACK WOL_Find_Game_Dialog_Proc(HWND win, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static int Col_ChanPrivate = 19;
	static int Col_ChanName = 34;
	static int Col_ChanIcon = 2;
	static int Col_PingTime = 172;

	switch (uMsg) {
		case WM_COMMAND: {
			switch (LOWORD(wParam)) {
				/// Cancel -- go back to the main lobby dialog.
				case IDCANCEL: {
					ShowWindow(WS_Find_Dialog(IDD_WOL_MAIN), SW_SHOW);
					WS_Destroy_Dialog(win, IDCANCEL);
					break;
				}

				/// Request a fresh game-channel list from the server.
				case IDC_REFCHAN: {
					if (g_pChat->RequestChannelList(CHANNELTYPE_GAME, 1) == S_OK) {
						unsigned int type = CHANNELTYPE_GAME;
						g_ChanListQueue.addTail(type);
					}
				} break;

				/// Filter checkboxes -- any change re-applies the filters to the list.
				case IDC_NOTOURNEY:
				case IDC_SQUADTRNY:
				case IDC_TOURNEY:
				case IDC_MAPRANDOM:
				case IDC_MAPOFFICIAL:
				case IDC_TEAMS:
				case IDC_STANDARD:
				case IDC_SHOWALL: {
					SendMessage(win, WOL_REFRESH_GAMELIST, 0, 0);
				} break;

				/// Locale filter combo changed -- re-apply the filters to the list.
				case IDC_FINDGAME_LOCATION: {
					if (HIWORD(wParam) == CBN_SELCHANGE) {
						SendMessage(win, WOL_REFRESH_GAMELIST, 0, 0);
					}
				} break;

				/// Join the selected game, either via the Join button or a double-click.
				case IDC_JOINGAME:
				case IDC_CHANNELS: {
					if ((LOWORD(wParam) == IDC_JOINGAME) || (HIWORD(wParam) == LBN_DBLCLK)) {
						Channel chan;
						int cindex = SendDlgItemMessage(win, IDC_CHANNELS, LB_GETCURSEL, 0, 0);
						if (cindex < 0) break;
						int extra = SendDlgItemMessage(win, IDC_CHANNELS, LB_GETITEMDATA, cindex, 0);
						g_GameChannelList.get(chan, extra);

						char exinfo_copy[256];
						strcpy(exinfo_copy, (char const *)chan.exInfo);

						/*
						 * Reject the join if our build number or rules/AI/art CRC does not
						 * match what the host advertised in the channel's extra info.
						 */
						int buildnum = Build_Number();
						int hostbuild = 0;
						char * cptr = strtok(exinfo_copy, ",");
						if (cptr) {
							hostbuild = atol(cptr);
						}

						int crc = GetINIHash();
						int hostcrc = 0;
						cptr = strtok(NULL, ",");
						if (cptr) {
							hostcrc = atol(cptr);
						}

						if (buildnum != hostbuild || crc != hostcrc) {
							ODMessageBox(Fetch_String(TXT_MISMATCH), 0, WOL_Wait_Callback, 0);
							break;
						}

						/*
						 * World Domination Tour games reserve slots per house -- refuse
						 * the join if the matching house's slot is already taken.
						 */
						if (Session.IsWDT) {
							strtok(NULL, ",");
							strtok(NULL, ",");
							strtok(NULL, ",");
							strtok(NULL, ",");
							strtok(NULL, ",");
							cptr = strtok(NULL, ",");
							if (cptr) {
								int house = atol(cptr);
								if ((house != 0 && Session.House == HOUSE_GOOD && house == 2) ||
									(Session.House == HOUSE_BAD && house == 1) ||
									house == 0) {
									ODMessageBox(Fetch_String(TXT_NO_FREE_SLOTS), 0, WOL_Wait_Callback, 0);
									break;
								}
							}
						}

						/// Password-protected game -- prompt for the channel key before joining.
						if (chan.flags & CHAN_MODE_KEY) {
							HWND dlg = WS_Create_Dialog(ProgramInstance, IDD_WOL_PASSWORD, MainWindow, (DLGPROC)WOL_Password_Dialog_Proc, FALSE);
							Center_Window_Within_Window(dlg);
							OwnerDraw::Subclass_Dialog(dlg, 0);
							ShowWindow(dlg, SW_NORMAL);
							if (WS_Wait_Dialog(dlg, WOL_Wait_Callback) == IDCANCEL) {
								break;
							}
							char password[17];
							WS_Get_Saved_Value(IDC_PASSWORD, (unsigned char *)password, sizeof(password));
							strcpy((char *)chan.key, password);
						}

						/*
						 * Leave the current channel and request the join.
						 */
						CurrentLevel = WOL_LEVEL_BACK_LOBBIES;
						g_pChat->RequestChannelLeave();
						PMessagePrintf(-1, Fetch_String(TXT_JOININGCHAN), chan.name);
						if (g_ChannelCount) {
							Show_Wait_Window(WOL_WAIT_CHANNEL_LEAVE);
						}

						memcpy(&s_TempChannel, &chan, sizeof(s_TempChannel));
						g_pChat->RequestChannelJoin(&chan);

						ShowWindow(WS_Find_Dialog(IDD_WOL_MAIN), SW_SHOW);
						WS_Destroy_Dialog(win, IDCANCEL);
					}
					if (HIWORD(wParam) == LBN_SELCHANGE) {
						SendMessage(win, WOL_SHOW_GAMEDETAILS, 0, 0);
					}
				} break;

				default:
					return(FALSE);
			}
		} break;


		/*
		 * Dialog init -- set up the filter sliders/checkboxes, the locale combo, the
		 * game-channel list columns, and request the first channel list refresh.
		 */
		case OD_SUBCLASSED: {
			ShowWindow(WS_Find_Dialog(IDD_WOL_MAIN), SW_HIDE);

			SendDlgItemMessage(win, IDC_PLAYERMIN, TBM_SETRANGE, 0, MAKELONG(2, WOL_MAX_PLAYERS));
			SendDlgItemMessage(win, IDC_PLAYERMAX, TBM_SETRANGE, 0, MAKELONG(2, WOL_MAX_PLAYERS));
			SendDlgItemMessage(win, IDC_PLAYERMAX, TBM_SETPOS, 0, 4);
			SendDlgItemMessage(win, IDC_PLAYERMIN, TBM_SETPOS, 0, 2);
			SendDlgItemMessage(win, IDC_MAXPING, TBM_SETRANGE, 0, MAKELONG(0, 1000));
			SendDlgItemMessage(win, IDC_MAXPING, TBM_SETPOS, 0, 1000);

			SendDlgItemMessage(win, IDC_NOTOURNEY, BM_SETCHECK, BST_CHECKED, 0);
			SendDlgItemMessage(win, IDC_TOURNEY, BM_SETCHECK, BST_CHECKED, 0);
			SendDlgItemMessage(win, IDC_SQUADTRNY, BM_SETCHECK, BST_CHECKED, 0);

			SendDlgItemMessage(win, IDC_FINDGAME_RANKMIN, TBM_SETRANGE, 0, MAKELONG(0, 10000));
			SendDlgItemMessage(win, IDC_FINDGAME_RANKMIN, TBM_SETPOS, 0, 0);
			SendDlgItemMessage(win, IDC_FINDGAME_RANKMIN, OD_SETTRACKSTEP, 0, 100);
			SendDlgItemMessage(win, IDC_FINDGAME_RANKMAX, TBM_SETRANGE, 0, MAKELONG(0, 10000));
			SendDlgItemMessage(win, IDC_FINDGAME_RANKMAX, TBM_SETPOS, 0, 10000);
			SendDlgItemMessage(win, IDC_FINDGAME_RANKMAX, OD_SETTRACKSTEP, 0, 100);

			SendDlgItemMessage(win, IDC_TEAMS, BM_SETCHECK, BST_CHECKED, 0);
			SendDlgItemMessage(win, IDC_STANDARD, BM_SETCHECK, BST_CHECKED, 0);
			SendDlgItemMessage(win, IDC_MAPRANDOM, BM_SETCHECK, BST_CHECKED, 0);
			SendDlgItemMessage(win, IDC_MAPOFFICIAL, BM_SETCHECK, BST_CHECKED, 0);

			SendDlgItemMessage(win, IDC_MINTECH, TBM_SETRANGE, 0, MAKELONG(1, MPLAYER_BUILD_LEVEL_MAX));
			SendDlgItemMessage(win, IDC_MINTECH, TBM_SETPOS, 0, 1);
			SendDlgItemMessage(win, IDC_MAXTECH, TBM_SETRANGE, 0, MAKELONG(1, MPLAYER_BUILD_LEVEL_MAX));
			SendDlgItemMessage(win, IDC_MAXTECH, TBM_SETPOS, 0, MPLAYER_BUILD_LEVEL_MAX);

			SendMessage(win, OD_SETTOP, 0, 1);

			/*
			 * Populate the locale filter combo with every known locale.
			 */
			SendDlgItemMessage(win, IDC_FINDGAME_LOCATION, CB_RESETCONTENT, 0, 0);
			int localecount;// = 0;
			g_pChat->GetLocaleCount(&localecount);
			if (localecount > 0) {
				for (int i = localecount - 1; i >= 0; i--) {
					LPCSTR locname = NULL;
					if (g_pChat->GetLocaleString(&locname, (Locale)i) != S_OK) {
						break;
					}
					char * copied;
					char buf[64];
					if (i == Session.Locale) {
						copied = strcpy(buf, locname);
					}
					if (i != 1) {
						if (i == 0) {
							SendDlgItemMessage(win, IDC_FINDGAME_LOCATION, CB_INSERTSTRING, 0, (LPARAM)Fetch_String(TXT_ALL_LOCATIONS));
						} else {
							SendDlgItemMessage(win, IDC_FINDGAME_LOCATION, CB_ADDSTRING, 0, (LPARAM)locname);
						}
					}
				}
			}
			int locsel = SendDlgItemMessage(win, IDC_FINDGAME_LOCATION, CB_SELECTSTRING, (WPARAM)-1, (LPARAM)Fetch_String(TXT_ALL_LOCATIONS));
			SendDlgItemMessage(win, IDC_FINDGAME_LOCATION, CB_SETCURSEL, locsel, 0);
			SendDlgItemMessage(win, IDC_FINDGAME_LOCATION, CB_SETTOPINDEX, locsel, 0);

			SendDlgItemMessage(win, IDC_CHANNELS, OD_ADDCOLUMN, 0, (LPARAM)Col_ChanPrivate);
			SendDlgItemMessage(win, IDC_CHANNELS, OD_ADDCOLUMN, (WPARAM)(Col_PingTime - Col_ChanName), (LPARAM)Col_ChanName);
			SendDlgItemMessage(win, IDC_CHANNELS, OD_ADDCOLUMN, 0, (LPARAM)Col_ChanIcon);
			SendDlgItemMessage(win, IDC_CHANNELS, OD_ADDCOLUMN, 0, (LPARAM)Col_PingTime);

			SendDlgItemMessage(win, IDC_MAXPING, OD_SETTRACKSTEP, 0, 25);
			SendDlgItemMessage(win, IDC_CHANNELS, OD_TOOLTIPS, 0, (LPARAM)1);
			SendDlgItemMessage(win, IDC_REFCHAN, OD_SETIMAGE, 0, (LPARAM)SurfaceCache.GetSurface("wouref.pcx"));
			SendDlgItemMessage(win, IDC_REFCHAN, OD_SETALTIMAGE, 0, (LPARAM)SurfaceCache.GetSurface("wodref.pcx"));
			SendDlgItemMessage(win, IDC_REFCHAN, OD_TOOLTIPS, 0, (LPARAM)1);

			SendMessage(win, WOL_REFRESH_GAMELIST, 0, 0);
		} break;

		/// A slider is being dragged -- debounce it and refresh once dragging settles.
		case WM_HSCROLL: {
			KillTimer(win, 0);
			SetTimer(win, 0, 300, 0);
		} return(FALSE);

		/// Rebuild the game-channel listbox from the current filter settings.
		case WM_TIMER:
		case WOL_REFRESH_GAMELIST: {
			int sel_index = SendDlgItemMessage(win, IDC_CHANNELS, LB_GETCURSEL, 0, 0);
			if (sel_index == -1) sel_index = 0;
			int topindex = SendDlgItemMessage(win, IDC_CHANNELS, LB_GETTOPINDEX, 0, 0);

			SendDlgItemMessage(win, IDC_CHANNELS, OD_DISABLEPAINT, 0, 1);

			SendDlgItemMessage(win, IDC_CHANNELS, LB_RESETCONTENT, 0, 0);

			/// Snapshot every filter control's current value.
			int index = -1;
			int count = g_GameChannelList.length();
			int filter_min_players = SendDlgItemMessage(win, IDC_PLAYERMIN, TBM_GETPOS, 0, 0);
			int filter_max_players = SendDlgItemMessage(win, IDC_PLAYERMAX, TBM_GETPOS, 0, 0);
			int not_tourney = 0;
			int filter_tourney = 0;
			int filter_squad_tourney = 0;
			if (SendDlgItemMessage(win, IDC_TOURNEY, BM_GETCHECK, 0, 0) == BST_CHECKED) filter_tourney = 1;
			if (SendDlgItemMessage(win, IDC_NOTOURNEY, BM_GETCHECK, 0, 0) == BST_CHECKED) not_tourney = 1;
			if (SendDlgItemMessage(win, IDC_SQUADTRNY, BM_GETCHECK, 0, 0) == BST_CHECKED) filter_squad_tourney = 1;
			int worst_ping = SendDlgItemMessage(win, IDC_MAXPING, TBM_GETPOS, 0, 0);
			int show_all_flag = 0;
			if (SendDlgItemMessage(win, IDC_SHOWALL, BM_GETCHECK, 0, 0) == BST_CHECKED) show_all_flag = 1;
			int localefilter = SendDlgItemMessage(win, IDC_FINDGAME_LOCATION, CB_GETCURSEL, 0, 0);
			int filter_min_tech = SendDlgItemMessage(win, IDC_MINTECH, TBM_GETPOS, 0, 0);
			int filter_max_tech = SendDlgItemMessage(win, IDC_MAXTECH, TBM_GETPOS, 0, 0);
			int minrank = SendDlgItemMessage(win, IDC_FINDGAME_RANKMIN, TBM_GETPOS, 0, 0);
			int maxrank = SendDlgItemMessage(win, IDC_FINDGAME_RANKMAX, TBM_GETPOS, 0, 0);
			int teams = SendDlgItemMessage(win, IDC_TEAMS, BM_GETCHECK, 0, 0);
			int randommap = SendDlgItemMessage(win, IDC_MAPRANDOM, BM_GETCHECK, 0, 0);
			int officialmap = SendDlgItemMessage(win, IDC_MAPOFFICIAL, BM_GETCHECK, 0, 0);
			int standard = SendDlgItemMessage(win, IDC_STANDARD, BM_GETCHECK, 0, 0);

			Channel chan;
			memset(&chan, 0, sizeof(chan));
			char info[256];
			OwnerDraw::CellData thecell;
			thecell.type = OwnerDraw::CellData::SURFACE;

			Wstring ladder_name;
			Ladder ladder;
			bool show_channel;
			bool mode_ok;
			bool tourney_ok;
			char * cptr;
			char exinfo_copy[100];
			int team_flag;
			/// Walk every known game channel, applying all active filters in turn.
			for (int i = 0; i < count; i++) {
				g_GameChannelList.get(chan, i);
				strcpy(exinfo_copy, (char *)chan.exInfo);
				cptr = exinfo_copy;

				show_channel = true;
				if (((int)chan.maxUsers < filter_min_players) || ((int)chan.maxUsers > filter_max_players)) {
					show_channel = false;
				}
				if (((int)chan.latency != -1) && ((int)chan.latency > worst_ping)) {
					show_channel = false;
				}

				/// Look up the channel's ladder rank for the rank filter.
				ladder.rung = 10000;
				ladder_name.set((char *)chan.name);
				ladder_name.truncate('\'');
				ladder_name.toLower();
				if (g_ActiveLadder->getValue(ladder_name, ladder)) {
					if (ladder.rung > 10000) {
						ladder.rung = 10000;
					}
				}
				if ((int)ladder.rung < minrank || (int)ladder.rung > maxrank) {
					show_channel = false;
				}

				/*
				 * Locale filter -- resolve the combo selection back to a Locale index and
				 * check it against the channel host's registered locale.
				 */
				if (localefilter > 0) {
					char localetext[256];
					memset(localetext, 0, sizeof(localetext));
					SendDlgItemMessage(win, IDC_FINDGAME_LOCATION, CB_GETLBTEXT, localefilter, (LPARAM)localetext);

					int localecount = 0;
					g_pChat->GetLocaleCount(&localecount);
					int locindex = localefilter;
					for (int l = 0; l < localecount; l++) {
						LPCSTR locname = NULL;
						if (g_pChat->GetLocaleString(&locname, (Locale)l) == S_OK && !strcmp(locname, localetext)) {
							locindex = l;
							break;
						}
					}

					int hostlocale = 0;
					if (!g_UserLocales.getValue(ladder_name, hostlocale) || hostlocale != locindex) {
						show_channel = false;
					}
				}

				/*
				 * Tournament / squad-tournament / non-tournament filter.
				 */
				tourney_ok = false;
				if (chan.tournament && (filter_tourney)) tourney_ok = true;
				if ((chan.reserved & 0x100) && filter_squad_tourney) tourney_ok = true;
				if ((chan.tournament || (chan.reserved & 0x100) || !not_tourney)) {
					if (!tourney_ok && !Session.IsWDT) {
						show_channel = false;
					}
				}

				/*
				 * Parse the channel's comma-separated extra info: tech level, team mode,
				 * and map name.
				 */
				if (strlen(cptr)) {
					cptr = strtok(exinfo_copy, ",");
					cptr = strtok(NULL, ",");
					cptr = strtok(NULL, ",");
					if (((atol(cptr) < filter_min_tech) || (atol(cptr) > filter_max_tech))) {
						show_channel = false;
					}

					mode_ok = false;
					cptr = strtok(NULL, ",");
					cptr = strtok(NULL, ",");
					team_flag = atol(cptr);
					if ((team_flag) && teams) mode_ok = true;

					cptr = strtok(NULL, ",");
					cptr = strtok(NULL, ",");
					atol(cptr);

					cptr = strtok(NULL, ",");
					cptr = strtok(NULL, ",");
					if (cptr) {
						if (!strcmp(cptr, RANDOM_MAP_FILE_NAME)) {
							if (!randommap) {
								show_channel = false;
							}
						} else if (!officialmap) {
							show_channel = false;
						}
					}

					if ((team_flag || !standard) && !mode_ok) {
						show_channel = false;
					}
				}

				/// Addon (Firestorm) compatibility filter.
				AddonType addon = (AddonType)((chan.reserved >> 12) & 0xF);
				if (!Addon_Enabled(addon)) {
					show_channel = false;
				}
				bool addonpass = show_channel;
				if (Addon_Enabled(ADDON_FIRESTORM) && addon == ADDON_BASE_GAME) {
					addonpass = false;
				}

				/*
				 * Channel passed the filters (or Show All is set) -- add it to the list
				 * and fill in its name/icon/privacy/ping columns.
				 */
				if (show_all_flag || addonpass) {
					sprintf(info, "%s  %d/%d", chan.name, chan.currentUsers, chan.maxUsers);

					index = SendDlgItemMessage(win, IDC_CHANNELS, LB_INSERTSTRING, (WPARAM)-1, (LPARAM)chan.name);

					thecell.type = OwnerDraw::CellData::TEXT;
					thecell.string.set(info);
					thecell.hint.set("");
					SendDlgItemMessage(win, IDC_CHANNELS, OD_SETCELL, MAKEWPARAM(Col_ChanName, index), (LPARAM)&thecell);

					thecell.type = OwnerDraw::CellData::SURFACE;
					thecell.hint.set("");
					if (chan.tournament) {
						thecell.surf = SurfaceCache.GetSurface("woltrny.pcx");
						thecell.hint.set((char *)Fetch_String(TXT_TOURNAMENT_GAME));
					} else if (chan.reserved & 0x100) {
						thecell.surf = SurfaceCache.GetSurface("wolclan.pcx");
						thecell.hint.set((char *)Fetch_String(TXT_BATTLECLAN_GAME));
					} else {
						thecell.surf = SurfaceCache.GetSurface("gt18.bmp");
					}
					SendDlgItemMessage(win, IDC_CHANNELS, OD_SETCELL, MAKEWPARAM(Col_ChanIcon, index), (LPARAM)&thecell);

					thecell.hint.set("");
					if (chan.flags & CHAN_MODE_KEY) {
						thecell.surf = SurfaceCache.GetSurface("wolpriv.pcx");
						thecell.hint.set((char *)Fetch_String(TXT_GAME_PASSWORD));
					} else {
						thecell.type = OwnerDraw::CellData::INVALID;
					}
					SendDlgItemMessage(win, IDC_CHANNELS, OD_SETCELL, MAKEWPARAM(Col_ChanPrivate, index), (LPARAM)&thecell);

					thecell.type = OwnerDraw::CellData::PING;
					thecell.pingtime = chan.latency;
					sprintf(info, "Ping = %d ms", chan.latency);
					thecell.hint.set(info);
					SendDlgItemMessage(win, IDC_CHANNELS, OD_SETCELL, MAKEWPARAM(Col_PingTime, index), (LPARAM)&thecell);

					thecell.hint.set("");
					SendDlgItemMessage(win, IDC_CHANNELS, LB_SETITEMDATA, index, i);
				}
			}
			SendDlgItemMessage(win, IDC_CHANNELS, LB_SETCURSEL, sel_index, 0);

			SendDlgItemMessage(win, IDC_CHANNELS, LB_SETTOPINDEX, (WPARAM)topindex, 0);

			SendDlgItemMessage(win, IDC_CHANNELS, OD_DISABLEPAINT, 0, 0);

			HWND chan_list = GetDlgItem(win, IDC_CHANNELS);
			InvalidateRect(chan_list, NULL, 0);
			UpdateWindow(chan_list);
		}

		/// Populate the details listbox for the currently selected game channel.
		case WOL_SHOW_GAMEDETAILS: {
			char exinfo_copy[100];
			char scenarioname[256];

			Channel chan;
			memset(&chan, 0, sizeof(chan));
			int cindex = SendDlgItemMessage(win, IDC_CHANNELS, LB_GETCURSEL, 0, 0);
			if (cindex < 0) break;
			int extra = SendDlgItemMessage(win, IDC_CHANNELS, LB_GETITEMDATA, cindex, 0);
			static char details[1024];
			g_GameChannelList.get(chan, extra);

			SendDlgItemMessage(win, IDC_FINDGAME_DETAILS, LB_RESETCONTENT, 0, 0);
			sprintf(details, "%s %s", Fetch_String(TXT_NAME_COLON), (char const *)chan.name);
			SendDlgItemMessage(win, IDC_FINDGAME_DETAILS, LB_INSERTSTRING, (WPARAM)-1, (LPARAM)details);

			/// Parse the channel's extra info to reach the map/scenario filename field.
			strcpy(exinfo_copy, (char const *)chan.exInfo);
			strtok(exinfo_copy, ",");
			strtok(NULL, ",");
			char * cptr = strtok(NULL, ",");
			if (cptr) atol(cptr);
			cptr = strtok(NULL, ",");
			if (cptr) atol(cptr);
			cptr = strtok(NULL, ",");
			if (cptr) atol(cptr);
			cptr = strtok(NULL, ",");
			if (cptr) atol(cptr);
			strtok(NULL, ",");
			strtok(NULL, ",");
			cptr = strtok(NULL, ",");

			scenarioname[0] = 0;
			if (cptr) {
				strcpy(scenarioname, cptr);
			}

			/// Match the map filename to a known scenario for its display description.
			for (int i = 0; i < Session.Scenarios.Count(); i++) {
				MultiMission * scen = Session.Scenarios[i];
				if (!stricmp(scenarioname, scen->Get_Filename())) {
					strcpy(scenarioname, scen->Description());
					break;
				}
			}

			if (!strcmp(scenarioname, RANDOM_MAP_FILE_NAME)) {
				strcpy(scenarioname, Fetch_String(TXT_RANDOM_MAP_DESCRIPTION));
			}

			sprintf(details, "%s %s", Fetch_String(TXT_SCENARIO_COLON), scenarioname);
			SendDlgItemMessage(win, IDC_FINDGAME_DETAILS, LB_INSERTSTRING, (WPARAM)-1, (LPARAM)details);

			sprintf(details, "%s %s", Fetch_String(TXT_TOURNEY_COLON), Fetch_String(TXT_NO));
			if (chan.tournament) {
				sprintf(details, "%s %s", Fetch_String(TXT_TOURNEY_COLON), Fetch_String(TXT_YES));
			} else if (chan.reserved & 0x100) {
				sprintf(details, "%s %s", Fetch_String(TXT_TOURNEY_COLON), Fetch_String(TXT_BATTLE_CLAN));
			}
			SendDlgItemMessage(win, IDC_FINDGAME_DETAILS, LB_INSERTSTRING, (WPARAM)-1, (LPARAM)details);

			/*
			 * Show the host's ladder rank, if known.
			 */
			Wstring name;
			name.set((char *)chan.name);
			name.truncate('\'');
			name.toLower();

			Ladder ladder;
			if (g_ActiveLadder->getValue(name, ladder) && ladder.rung > 0) {
				sprintf(details, "%s: %d", Fetch_String(TXT_RANK), ladder.rung);
			} else {
				sprintf(details, "%s %s", Fetch_String(TXT_HOST_RANK), Fetch_String(TXT_UNRANKED));
			}
			SendDlgItemMessage(win, IDC_FINDGAME_DETAILS, LB_INSERTSTRING, (WPARAM)-1, (LPARAM)details);
		} break;

		/// Supply the tooltip text for the control under the cursor.
		case OD_GETTIPTEXT: {
			HWND ctrl = GetDlgItem(win, wParam);
			GetWindowText(ctrl, (char *)lParam, 127);
		} return(FALSE);

		case WM_DRAWITEM: {
			DRAWITEMSTRUCT * di = (LPDRAWITEMSTRUCT)lParam;
			OwnerDraw::Draw_Item(di);
		}
			return(true);

		case WM_PAINT:
			OwnerDraw::Draw_Dialog_Back(win);
			ValidateRect(win, NULL);
			return(false);

		/*
		 * The dialog's controls are set up in the OD_SUBCLASSED handler, so there is
		 * nothing to do at WM_INITDIALOG.
		 */
		case WM_INITDIALOG:
			return(false);

		default:
			return(false);
	}
	return(false);
}

/// <summary>
/// Window procedure for the "Locate a User" (paging) dialog. Lets the user look up
/// another user by name, send them a private page message, or broadcast a message to
/// the user's squad.
/// </summary>
/// <param name="win">Handle of the dialog window.</param>
/// <param name="uMsg">Window message being processed.</param>
/// <param name="wParam">Message-specific parameter.</param>
/// <param name="lParam">Message-specific parameter.</param>
/// <returns>TRUE if the message was handled, FALSE otherwise.</returns>
BOOL CALLBACK WOL_Find_Page_Dialog_Proc(HWND win, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
		case WM_COMMAND: {
			switch (LOWORD(wParam)) {
				case IDCANCEL: {
					WS_Destroy_Dialog(win, IDCANCEL);
				} break;

				/// Look up whether the named user is currently online.
				case IDC_FIND: {
					if (g_pChat) {
						char name[64];
						memset(name, 0, sizeof(name));
						SendDlgItemMessage(win, IDC_NAME, WM_GETTEXT, 60, (LPARAM)name);
						name[MAX_USERNAME_LEN - 1] = 0;
						User user;
						strcpy((char *)user.name, name);
						if (strlen((char *)user.name)) {
							g_pChat->RequestFind(&user);
						} else {
							SMessagePrintf(-1, Fetch_String(TXT_NAME_ERROR));
						}
					}
				} break;

				/// Send a private page message to the named user.
				case IDC_PAGE: {
					if (g_pChat) {
						char name[64];
						memset(name, 0, sizeof(name));
						SendDlgItemMessage(win, IDC_NAME, WM_GETTEXT, 60, (LPARAM)name);
						char message[200];
						memset(message, 0, sizeof(message));
						SendDlgItemMessage(win, IDC_INPUT, WM_GETTEXT, 195, (LPARAM)message);
						SendDlgItemMessage(win, IDC_INPUT, WM_SETTEXT, 0, (LPARAM) "");
						SetFocus(GetDlgItem(win, IDC_INPUT));
						name[MAX_USERNAME_LEN - 1] = 0;
						User user;
						strcpy((char *)user.name, name);

						if (strlen(message) && strlen((char *)user.name)) {
							g_pChat->RequestPage(&user, message);
							SMessagePrintf(ColorMe, "[%s] %s", g_NickName, message);
						} else if (strlen((char *)user.name)) {
							SMessagePrintf(-1, Fetch_String(TXT_ENTER_MESSAGE));
						} else {
							SMessagePrintf(-1, Fetch_String(TXT_NAME_ERROR));
						}
					}
				} break;

				/// Broadcast a message to every member of the local player's squad.
				case IDC_SQUADPAGE: {
					if (g_pChat) {
						if (g_OwnSquadID == 0) {
							SMessagePrintf(-1, Fetch_String(TXT_NOSQUAD));
						} else {
							char message[200];
							memset(message, 0, 200);
							SendDlgItemMessage(win, IDC_INPUT, WM_GETTEXT, 195, (LPARAM)message);
							SendDlgItemMessage(win, IDC_INPUT, WM_SETTEXT, 0, (LPARAM) "");
							SetFocus(GetDlgItem(win, IDC_INPUT));
							User user;
							strcpy((char *)user.name, "0");
							if (strlen(message)) {
								g_pChat->RequestPage(&user, message);
							} else {
								SMessagePrintf(-1, Fetch_String(TXT_ENTER_MESSAGE));
							}
						}
					}
				} break;

				default:
					return(false);
			}
		} break;

		case WM_INITDIALOG: {
			SendDlgItemMessage(win, IDC_NAME, EM_LIMITTEXT, 9, 0L);
		} break;

		case WM_DRAWITEM: {
			DRAWITEMSTRUCT * di = (LPDRAWITEMSTRUCT)lParam;
			OwnerDraw::Draw_Item(di);
		}
			return(true);

		case WM_PAINT:
			OwnerDraw::Draw_Dialog_Back(win);
			ValidateRect(win, NULL);
			return(false);

		default:
			return(false);
	}

	return(false);
}

/// <summary>
/// Window procedure for the ladder/rankings dialog. Populates the SKU (game/battle-clan,
/// Tiberian Sun/Firestorm) and locale combos, requests ladder rung searches from the
/// ladder server as the SKU/locale/search text change or the user pages through rungs,
/// and opens the related tournament/player web pages.
/// </summary>
/// <param name="win">Handle of the dialog window.</param>
/// <param name="uMsg">Window message being processed.</param>
/// <param name="wParam">Message-specific parameter.</param>
/// <param name="lParam">Message-specific parameter.</param>
/// <returns>TRUE if the message was handled, FALSE otherwise.</returns>
BOOL CALLBACK WOL_Ladder_Dialog_Proc(HWND win, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	char buffer[256];

	static int _ladder_step = 25;

	switch (uMsg) {
		case WM_COMMAND: {
			switch (LOWORD(wParam)) {
				case IDOK: {
					WS_Destroy_Dialog(win, IDOK);
					ShowWindow(WS_Top_Window(), SW_SHOW);
				} break;

				/*
				 * Page forward/back through the ladder rungs, then re-request the search
				 * (locale-aware for the TS/FS player ladders, plain otherwise).
				 */
				case IDNEXT: {
					g_LadderPos += _ladder_step;
					char ladder_search_key[10];
					sprintf(ladder_search_key, "%d", g_LadderPos);

					if (g_SelectedLadderSKU == TIBERIAN_SUN_SKU || g_SelectedLadderSKU == FIRESTORM_SKU) {
						int locale = g_LadderLocale;
						if (locale == LOC_UNKNOWN) {
							locale = g_LadderLocale - 1;
						}
						g_pNetUtil->RequestLocaleLadderSearch(g_LadderServerHost, g_LadderServerPort, ladder_search_key, LADDER_CODE(g_SelectedLadderSKU), -1, 0, 0, 25, 0, (Locale)locale);
					} else {
						g_pNetUtil->RequestLadderSearch(g_LadderServerHost, g_LadderServerPort, ladder_search_key, LADDER_CODE(g_SelectedLadderSKU), -1, 0, 0, 25, 0);
					}
					SendDlgItemMessage(win, IDC_LADRUNGS, LB_RESETCONTENT, NULL, NULL);
					SendDlgItemMessage(win, IDC_LADRUNGS, LB_INSERTSTRING, (WPARAM)-1, (LPARAM)((const char *)Fetch_String(TXT_SEARCHING)));
				} break;

				case IDBACK: {
					g_LadderPos -= _ladder_step;
					g_LadderPos = std::max(1, g_LadderPos);
					char ladder_search_key[10];
					sprintf(ladder_search_key, "%d", g_LadderPos);

					if (g_SelectedLadderSKU == TIBERIAN_SUN_SKU || g_SelectedLadderSKU == FIRESTORM_SKU) {
						int locale = g_LadderLocale;
						if (locale == LOC_UNKNOWN) {
							locale = g_LadderLocale - 1;
						}
						g_pNetUtil->RequestLocaleLadderSearch(g_LadderServerHost, g_LadderServerPort, ladder_search_key, LADDER_CODE(g_SelectedLadderSKU), -1, 0, 0, 25, 0, (Locale)locale);
					} else {
						g_pNetUtil->RequestLadderSearch(g_LadderServerHost, g_LadderServerPort, ladder_search_key, LADDER_CODE(g_SelectedLadderSKU), -1, 0, 0, 25, 0);
					}
					SendDlgItemMessage(win, IDC_LADRUNGS, LB_RESETCONTENT, NULL, NULL);
					SendDlgItemMessage(win, IDC_LADRUNGS, LB_INSERTSTRING, (WPARAM)-1, (LPARAM)((const char *)Fetch_String(TXT_SEARCHING)));
				} break;

				/// Search for a rung by name once the user finishes typing.
				case IDC_SEARCH: {
					char input[257];
					int len;

					SendDlgItemMessage(win, IDC_SEARCH, WM_GETTEXT, 256, (LPARAM)input);

					while ((input[strlen(input) - 1] == '\r') || (input[strlen(input) - 1] == '\n'))
						input[strlen(input) - 1] = 0;

					len = strlen(input);

					if (HIWORD(wParam) == EN_MAXTEXT) {
						SendDlgItemMessage(win, IDC_SEARCH, WM_SETTEXT, 0, 0);
						if (len) {
							int locale = g_LadderLocale;
							if (locale == LOC_UNKNOWN) {
								locale = g_LadderLocale - 1;
							}
							if (g_SelectedLadderSKU == TIBERIAN_SUN_SKU || g_SelectedLadderSKU == FIRESTORM_SKU) {
								g_pNetUtil->RequestLocaleLadderSearch(g_LadderServerHost, g_LadderServerPort, input, LADDER_CODE(g_SelectedLadderSKU), -1, 0, 0, 25, 10, (Locale)locale);
							} else {
								g_pNetUtil->RequestLadderSearch(g_LadderServerHost, g_LadderServerPort, input, LADDER_CODE(g_SelectedLadderSKU), -1, 0, 0, 25, 10);
							}
							SendDlgItemMessage(win, IDC_LADRUNGS, LB_RESETCONTENT, NULL, NULL);

							sprintf(buffer, Fetch_String(TXT_SEARCHING_FOR), input);
							SendDlgItemMessage(win, IDC_LADRUNGS, LB_INSERTSTRING, (WPARAM)-1, (LPARAM)buffer);
						}
					}
				} break;

				/// Open the battle-clan tournament web page.
				case IDC_ACTION: {
					sprintf(buffer, "http://www.westwood.com/westwoodonline/tournaments/tibsun/index_clan_%d.html", g_LanguageCode);
					ViewHTML(buffer, 0);
					return(FALSE);
				} break;

				/*
				 * The SKU combo (Firestorm/TS x battle-clan/player) changed -- resolve the
				 * selection to a SKU flag set, reset paging, and re-request the ladder.
				 */
				case IDC_LADDER_TYPE: {
					char ladder_search_key[10];
					if (HIWORD(wParam)==LBN_SELCHANGE) {

						int sku;
						int idx = SendDlgItemMessage(win, IDC_LADDER_TYPE, CB_GETCURSEL, 0, 0);
						if (idx == CB_ERR) {
							DebugString("CB_GETCURSEL(IDC_LADDERTYPE) returned CB_ERR\n");
							sku = g_SelectedLadderSKU;
						} else {
							if (idx == 1) {
								sku = BATTLECLANS | TIBERIAN_SUN_SKU;
							} else if (idx == 2) {
								sku = FIRESTORM_SKU;
							} else {
								sku = idx != 3 ? TIBERIAN_SUN_SKU : (BATTLECLANS | FIRESTORM_SKU);
							}
							g_SelectedLadderSKU = sku;
						}
						g_LadderPos = 1;
						if (sku == (BATTLECLANS | TIBERIAN_SUN_SKU) || sku == (BATTLECLANS | FIRESTORM_SKU))
						{
							int ladder_sel = SendDlgItemMessage(win, IDC_LADDER_LOCATION, CB_SELECTSTRING, 0xFFFFFFFF, (LPARAM)Fetch_String(TXT_ALL_LOCATIONS));
							SendDlgItemMessage(win, IDC_LADDER_LOCATION, CB_SETCURSEL, ladder_sel, 0);
							SendDlgItemMessage(win, IDC_LADDER_LOCATION, CB_SETTOPINDEX, ladder_sel, 0);
							EnableWindow(GetDlgItem(win, IDC_LADDER_LOCATION), 0);
							g_LadderLocale = LOC_UNKNOWN;
							sprintf(ladder_search_key, "%d", g_LadderPos);
							g_pNetUtil->RequestLadderSearch(g_LadderServerHost, g_LadderServerPort, ladder_search_key, LADDER_CODE(g_SelectedLadderSKU), -1, 0, 0, 25, 0);
							return(FALSE);
						} else {
							EnableWindow(GetDlgItem(win, IDC_LADDER_LOCATION), 1);
							sprintf(ladder_search_key, "%d", g_LadderPos);
							int ladderlocale = g_LadderLocale;
							if (ladderlocale == 0) {
								ladderlocale = g_LadderLocale - 1;
							}
							g_pNetUtil->RequestLocaleLadderSearch(g_LadderServerHost, g_LadderServerPort, ladder_search_key, LADDER_CODE(g_SelectedLadderSKU), -1, 0, 0, 25, 0, (Locale)ladderlocale);
							return(FALSE);
						}
					}
				} break;

				/// Open the player ladder web page.
				case IDC_LADDER_PLAYER_WEBPAGE: {
					sprintf(buffer, "http://www.westwood.com/westwoodonline/tournaments/tibsun/index_%d.html", g_LanguageCode);
					ViewHTML(buffer, 0);
					return(FALSE);
				} break;

				/*
				 * The locale filter combo changed -- resolve it to a Locale index and
				 * re-request the ladder search for that locale.
				 */
				case IDC_LADDER_LOCATION: {
					char ladder_search_key[10];
					if (HIWORD(wParam)==CBN_SELCHANGE) {
						int sel = SendDlgItemMessage(win, IDC_LADDER_LOCATION, CB_GETCURSEL, 0, 0);

						if (sel == -1) {
							DebugString("CB_GETCURSEL returned CB_ERR\n");
							return(FALSE);
						} else {
							int localecount = 0;

							memset(buffer, 0, sizeof(buffer));

							SendDlgItemMessage(win, IDC_LADDER_LOCATION, CB_GETLBTEXT, sel, (LPARAM)buffer);
							g_pChat->GetLocaleCount(&localecount);

							int last = g_LadderLocale;

							for (int i = 0; i < localecount; i++) {
								const char *locname;
								if (g_pChat->GetLocaleString(&locname, ((Locale)i)) == 0 && strcmp(locname, buffer) == 0) {
									sel = i;
									break;
								}
							}

							g_LadderLocale = sel;
							int newsel = sel;
							if (sel == 0) {
								newsel = -1;
							}
							if (last != sel) {
								g_LadderPos = 1;
							}

							sprintf(ladder_search_key, "%d", g_LadderPos);
							g_pNetUtil->RequestLocaleLadderSearch(g_LadderServerHost, g_LadderServerPort, ladder_search_key, LADDER_CODE(g_SelectedLadderSKU), -1, 0, 0, 25, 0, (Locale)newsel);
							return(FALSE);
						}
					}
				} break;

				default:
					return(FALSE);
			}
		} break;

		/// Dialog init -- set the fixed-width font used for the rungs list and add its columns.
		case OD_SUBCLASSED: {
			HDC hdc = GetDC(win);
			HFONT list_font = WS_Get_Font(hdc, "Fixedsys", 8, 16, 0);
			ReleaseDC(win, hdc);
			SendDlgItemMessage(win, IDC_LADRUNGS, WM_SETFONT, (WPARAM)list_font, 0);

			g_Col_Rung = 10;
			g_Col_Name = 90;
			g_Col_Points = 200;
			g_Col_Wins = 270;
			g_Col_Losses = 340;
			g_Col_Disconnects = 410;

			SendDlgItemMessage(win, IDC_LADRUNGS, LB_RESETCONTENT, NULL, NULL);

			SendDlgItemMessage(win, IDC_LADRUNGS, OD_ADDCOLUMN, 0, (LPARAM)g_Col_Rung);
			SendDlgItemMessage(win, IDC_LADRUNGS, OD_ADDCOLUMN, 0, (LPARAM)g_Col_Name);
			SendDlgItemMessage(win, IDC_LADRUNGS, OD_ADDCOLUMN, 0, (LPARAM)g_Col_Points);
			SendDlgItemMessage(win, IDC_LADRUNGS, OD_ADDCOLUMN, 0, (LPARAM)g_Col_Wins);
			SendDlgItemMessage(win, IDC_LADRUNGS, OD_ADDCOLUMN, 0, (LPARAM)g_Col_Losses);
			SendDlgItemMessage(win, IDC_LADRUNGS, OD_ADDCOLUMN, 0, (LPARAM)g_Col_Disconnects);

			SendDlgItemMessage(win, IDC_LADRUNGS, LB_INSERTSTRING, (WPARAM)-1, (LPARAM)((const char *)Fetch_String(TXT_SEARCHING)));
		} break;

		case WM_INITDIALOG: {
			int sel;
			char ladder_search_key[10];

			g_LadderPos = 1;

			SendDlgItemMessage(win, IDC_SEARCH, EM_SETLIMITTEXT, 9, 0);

			sprintf(ladder_search_key, "%d", g_LadderPos);

			/*
			 * Populate the SKU combo and pick the default (Firestorm players if installed
			 * and enabled, otherwise Tiberian Sun players).
			 */
			SendDlgItemMessage(win, IDC_LADDER_TYPE, CB_RESETCONTENT, 0, 0);
			SendDlgItemMessage(win, IDC_LADDER_TYPE, CB_INSERTSTRING, 0, (LPARAM)Fetch_String(TXT_FS_BATTLECLANS));
			SendDlgItemMessage(win, IDC_LADDER_TYPE, CB_INSERTSTRING, 0, (LPARAM)Fetch_String(TXT_FS_PLAYERS));
			SendDlgItemMessage(win, IDC_LADDER_TYPE, CB_INSERTSTRING, 0, (LPARAM)Fetch_String(TXT_TS_BATTLECLANS));
			SendDlgItemMessage(win, IDC_LADDER_TYPE, CB_INSERTSTRING, 0, (LPARAM)Fetch_String(TXT_TS_PLAYERS));
			if (Addon_Installed(ADDON_FIRESTORM) && Addon_Enabled(ADDON_FIRESTORM)) {
				g_SelectedLadderSKU = FIRESTORM_SKU;
				sel = SendDlgItemMessage(win, IDC_LADDER_TYPE, CB_SELECTSTRING, -1, (LPARAM)Fetch_String(TXT_FS_PLAYERS));
			} else {
				sel = SendDlgItemMessage(win, IDC_LADDER_TYPE, CB_SELECTSTRING, -1, (LPARAM)Fetch_String(TXT_TS_PLAYERS));
				g_SelectedLadderSKU = TIBERIAN_SUN_SKU;
			}
			SendDlgItemMessage(win, IDC_LADDER_TYPE, CB_SETCURSEL, sel, 0);
			SendDlgItemMessage(win, IDC_LADDER_TYPE, CB_SETTOPINDEX, sel, 0);

			/*
			 * Populate the locale filter combo with every known locale.
			 */
			SendDlgItemMessage(win, IDC_LADDER_LOCATION, CB_RESETCONTENT, 0, 0);

			int numlocales;
			g_pChat->GetLocaleCount(&numlocales);
			if (numlocales > 0) {
				int index = numlocales - 1;
				while (index >= 0) {
					LPCSTR name;
					if (g_pChat->GetLocaleString(&name, (Locale)index) != S_OK) {
						break;
					}
					if (index == (int)Session.Locale) {
						strcpy(buffer, name);
					}
					if (index != LOC_OTHER) {
						if (index == LOC_UNKNOWN) {
							SendDlgItemMessage(win, IDC_LADDER_LOCATION, CB_INSERTSTRING, 0, (LPARAM)Fetch_String(TXT_ALL_LOCATIONS));
						} else {
							SendDlgItemMessage(win, IDC_LADDER_LOCATION, CB_ADDSTRING, 0, (LPARAM)name);
						}
					}
					index--;
				}

			}

			sel = SendDlgItemMessage(win, IDC_LADDER_LOCATION, CB_SELECTSTRING, -1, (LPARAM)Fetch_String(TXT_ALL_LOCATIONS));
			SendDlgItemMessage(win, IDC_LADDER_LOCATION, CB_SETCURSEL, sel, 0);
			SendDlgItemMessage(win, IDC_LADDER_LOCATION, CB_SETTOPINDEX, sel, 0);

			/*
			 * Request the first page of the ladder for the default SKU/locale.
			 */
			g_pNetUtil->RequestLocaleLadderSearch(g_LadderServerHost, g_LadderServerPort, ladder_search_key, LADDER_CODE(g_SelectedLadderSKU), -1, 0, 0, 25, 0, (Locale)-1);

			return(false);
		}
;

		case WM_DRAWITEM:
			OwnerDraw::Draw_Item((LPDRAWITEMSTRUCT)lParam);
			return(true);

		case WM_PAINT:
			OwnerDraw::Draw_Dialog_Back(win);
			ValidateRect(win, NULL);
			return(false);

		default:
			return(false);
	}
	return(false);
}

/// <summary>
/// Window procedure for the patch/file download progress dialog. Lets the user cancel
/// an in-progress download, and closes the dialog when the download finishes or fails.
/// </summary>
/// <param name="win">Handle of the dialog window.</param>
/// <param name="uMsg">Window message being processed.</param>
/// <param name="wParam">Message-specific parameter.</param>
/// <param name="lParam">Message-specific parameter.</param>
/// <returns>TRUE if the message was handled, FALSE otherwise.</returns>
BOOL CALLBACK WOL_Download_Dialog_Proc(HWND win, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDCANCEL:
					g_pDownload->Abort();
					WS_Destroy_Dialog(win, IDCANCEL);
					break;

				default:
					return(FALSE);
			}
			break;

		case WM_INITDIALOG: {
		} break;

		/// Download completed successfully -- close the dialog.
		case WOL_DOWNLOAD_COMPLETE:
			WS_Destroy_Dialog(win, IDOK);
			break;

		/// Download failed -- notify the user, abort, and close the dialog.
		case WOL_DOWNLOAD_FAILED:
			ODMessageBox(Fetch_String(TXT_DOWNLOAD_FAILED), 0, WOL_Wait_Callback);
			g_pDownload->Abort();
			WS_Destroy_Dialog(win, IDCANCEL);
			break;

		case WM_DRAWITEM:
			OwnerDraw::Draw_Item((LPDRAWITEMSTRUCT)lParam);
			return(true);

		case WM_ERASEBKGND: {
			return(true);
		}

		case WM_PAINT:
			OwnerDraw::Draw_Dialog_Back(win);
			ValidateRect(win, NULL);
			return(false);

		default:
			return(false);
	}
	return(true);
}

/// <summary>
/// Dialog procedure for the "New Game" creation dialog. Handles owner-draw
/// painting, sets up the player-count controls (a slider, or WDT 1on1/2on2
/// radio buttons for World Domination Tour games), enforces tournament and
/// battle-clan slider interlocks, and stores the result on OK.
/// </summary>
/// <param name="win">Dialog window handle.</param>
/// <param name="uMsg">Window message identifier.</param>
/// <param name="wParam">Message-specific first parameter.</param>
/// <param name="lParam">Message-specific second parameter.</param>
/// <returns>TRUE if the message was handled, FALSE otherwise.</returns>
BOOL CALLBACK WOL_New_Game_Dialog_Proc(HWND win, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {

		case WM_COMMAND: {
			switch (LOWORD(wParam)) {
				case IDOK:
					if (Session.IsWDT) {
						g_MaxPlayers = IsDlgButtonChecked(win, IDC_WDT_2ON2) ? 4 : 2;
					} else {
						g_MaxPlayers = SendDlgItemMessage(win, IDC_MAXPLAYERS, TBM_GETPOS, 0, 0);
					}
					SendDlgItemMessage(win, IDC_PASSWORD, WM_GETTEXT, MAX_PASSWORD_LEN, (LPARAM)g_PendingChanPass);
					WS_Destroy_Dialog(win, IDOK);
					break;

				case IDCANCEL:
					WS_Destroy_Dialog(win, IDCANCEL);
					break;

				/// WDT 1-on-1 / 2-on-2 radio buttons -- keep them mutually exclusive.
				case IDC_WDT_1ON1:
					if (IsDlgButtonChecked(win, IDC_WDT_2ON2)) {
						CheckDlgButton(win, IDC_WDT_2ON2, 0);
					} else {
						CheckDlgButton(win, IDC_WDT_1ON1, 1);
					}
					return(false);

				case IDC_WDT_2ON2:
					if (IsDlgButtonChecked(win, IDC_WDT_1ON1)) {
						CheckDlgButton(win, IDC_WDT_1ON1, 0);
					} else {
						CheckDlgButton(win, IDC_WDT_2ON2, 1);
					}
					return(false);

				/*
				 * Battle-clan and tournament checkboxes are mutually exclusive; battle-clan
				 * games also force the player-count slider to an even value.
				 */
				case IDC_NEWGAME_BATTLECLAN:
					if (SendDlgItemMessage(win, IDC_NEWGAME_TOURNAMENT, BM_GETCHECK, 0, 0) == 1) {
						SendDlgItemMessage(win, IDC_NEWGAME_TOURNAMENT, BM_SETCHECK, 0, 0);
						EnableWindow(GetDlgItem(win, IDC_MAXPLAYERS), TRUE);
					}
					if (SendDlgItemMessage(win, IDC_NEWGAME_BATTLECLAN, BM_GETCHECK, 0, 0) == 1) {
						LRESULT pos = SendDlgItemMessage(win, IDC_MAXPLAYERS, TBM_GETPOS, 0, 0);
						if ((pos & 1) != 0) {
							SendDlgItemMessage(win, IDC_MAXPLAYERS, TBM_SETPOS, 0, pos + 1);
						}
						SendDlgItemMessage(win, IDC_MAXPLAYERS, OD_SETTRACKSTEP, 0, 2);
						return(false);
					}
					SendDlgItemMessage(win, IDC_MAXPLAYERS, OD_SETTRACKSTEP, 0, 1);
					return(false);

				/// Tournament games are fixed at 2 players and disable the slider.
				case IDC_NEWGAME_TOURNAMENT:
					if (SendDlgItemMessage(win, IDC_NEWGAME_TOURNAMENT, BM_GETCHECK, 0, 0) == 1) {
						SendDlgItemMessage(win, IDC_MAXPLAYERS, TBM_SETPOS, 1, 2);
						EnableWindow(GetDlgItem(win, IDC_MAXPLAYERS), FALSE);
						SendDlgItemMessage(win, IDC_NEWGAME_BATTLECLAN, BM_SETCHECK, 0, 0);
						return(false);
					}
					EnableWindow(GetDlgItem(win, IDC_MAXPLAYERS), TRUE);
					SendDlgItemMessage(win, IDC_MAXPLAYERS, OD_SETTRACKSTEP, 0, 1);
					return(false);

				default:
					return(FALSE);
			}
		} break;


		case WM_DRAWITEM:
			OwnerDraw::Draw_Item((LPDRAWITEMSTRUCT)lParam);
			return(true);

		case WM_ERASEBKGND:
			return(true);

		case WM_PAINT:
			OwnerDraw::Draw_Dialog_Back(win);
			ValidateRect(win, NULL);
			return(false);

		case WM_INITDIALOG: {
			/*
			 * World Domination Tour games pick 1on1/2on2 from the territory's player
			 * count; regular games get a plain player-count slider instead.
			 */
			if (Session.IsWDT) {
				WDTTerritory * territory = WDT_Get_Territory(Session.WDTTerritory);
				if (territory != NULL) {
					bool players4 = (g_MaxPlayers == 0) ? (territory->NumPlayers == 4) : (g_MaxPlayers == 4);
					CheckDlgButton(win, IDC_WDT_1ON1, players4 == false);
					CheckDlgButton(win, IDC_WDT_2ON2, players4 != false);
					if ((territory->UserModBooleans & 0x4000) == 0) {
						EnableWindow(GetDlgItem(win, IDC_WDT_1ON1), FALSE);
						EnableWindow(GetDlgItem(win, IDC_WDT_2ON2), FALSE);
					}
				} else {
					CheckDlgButton(win, IDC_WDT_1ON1, 1);
					CheckDlgButton(win, IDC_WDT_2ON2, 0);
				}
			} else {
				SendDlgItemMessage(win, IDC_MAXPLAYERS, TBM_SETRANGE, 1, MAKELONG(2, WOL_MAX_PLAYERS));
			}

			SendDlgItemMessage(win, IDC_PASSWORD, EM_LIMITTEXT, MAX_PASSWORD_LEN - 1, 0L);
			SetFocus(GetDlgItem(win, IDC_PASSWORD));

			if (!Session.IsWDT && (int)g_OwnSquadID < 1) {
				EnableWindow(GetDlgItem(win, IDC_NEWGAME_BATTLECLAN), FALSE);
			}
		}
			return(FALSE);

		default:
			return(false);
	}
	return(false);
}

/// <summary>
/// Window procedure for the channel-password entry dialog, used when joining a
/// password-protected game channel.
/// </summary>
/// <param name="win">Handle of the dialog window.</param>
/// <param name="uMsg">Window message being processed.</param>
/// <param name="wParam">Message-specific parameter.</param>
/// <param name="lParam">Message-specific parameter.</param>
/// <returns>TRUE if the message was handled, FALSE otherwise.</returns>
BOOL CALLBACK WOL_Password_Dialog_Proc(HWND win, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
		case WM_COMMAND: {
			switch (LOWORD(wParam)) {
				case IDOK:
					WS_Destroy_Dialog(win, IDOK);
					break;

				case IDCANCEL:
					WS_Destroy_Dialog(win, IDCANCEL);
					break;

				default:
					return(false);
			}
		} break;

		case WM_DRAWITEM:
			OwnerDraw::Draw_Item((LPDRAWITEMSTRUCT)lParam);
			return(true);

		case WM_PAINT:
			OwnerDraw::Draw_Dialog_Back(win);
			ValidateRect(win, NULL);
			return(false);

		/// Limit the password field length and give it initial focus.
		case WM_INITDIALOG: {
			SendDlgItemMessage(win, IDC_PASSWORD, EM_LIMITTEXT, MAX_PASSWORD_LEN - 1, 0L);
			SetFocus(GetDlgItem(win, IDC_PASSWORD));
		}
			return(false);

		default:
			return(false);
	}
	return(false);
}

/// <summary>
/// Window procedure for the initial nickname/birthdate prompt dialog shown the first
/// time a player enters Westwood Online. Populates the birth-month combo and lets the
/// user open the privacy policy web page.
/// </summary>
/// <param name="win">Handle of the dialog window.</param>
/// <param name="uMsg">Window message being processed.</param>
/// <param name="wParam">Message-specific parameter.</param>
/// <param name="lParam">Message-specific parameter.</param>
/// <returns>TRUE if the message was handled, FALSE otherwise.</returns>
BOOL CALLBACK WOL_Begin_Nick_Dialog_Proc(HWND win, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
		case WM_COMMAND: {
			switch (LOWORD(wParam)) {
				case IDOK:
					WS_Destroy_Dialog(win, IDOK);
					break;

				case IDCANCEL:
					WS_Destroy_Dialog(win, IDCANCEL);
					break;

				/// Open the privacy policy web page.
				case IDC_PRIVACY:
					ViewHTML("http://privacypolicy.westwood.com", 1);
					return(false);

				default:
					return(false);
			}
		} break;

		case WM_DRAWITEM:
			OwnerDraw::Draw_Item((LPDRAWITEMSTRUCT)lParam);
			return(true);

		case WM_PAINT:
			OwnerDraw::Draw_Dialog_Back(win);
			ValidateRect(win, NULL);
			return(false);

		case OD_SUBCLASSED:
			SendMessage(win, OD_SETTOP, 0, 1);
			break;

		case WM_INITDIALOG: {
			SendDlgItemMessage(win, IDC_BDAY, EM_LIMITTEXT, 2, 0L);
			SendDlgItemMessage(win, IDC_BYEAR, EM_LIMITTEXT, 4, 0L);
			SetFocus(GetDlgItem(win, IDC_BDAY));

			/*
			 * Populate the birth-month combo.
			 */
			SendDlgItemMessage(win, IDC_BMONTH, CB_RESETCONTENT, 0, 0);
			SendDlgItemMessage(win, IDC_BMONTH, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)Fetch_String(TXT_JANUARY));
			SendDlgItemMessage(win, IDC_BMONTH, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)Fetch_String(TXT_FEBRUARY));
			SendDlgItemMessage(win, IDC_BMONTH, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)Fetch_String(TXT_MARCH));
			SendDlgItemMessage(win, IDC_BMONTH, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)Fetch_String(TXT_APRIL));
			SendDlgItemMessage(win, IDC_BMONTH, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)Fetch_String(TXT_MAY));
			SendDlgItemMessage(win, IDC_BMONTH, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)Fetch_String(TXT_JUNE));
			SendDlgItemMessage(win, IDC_BMONTH, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)Fetch_String(TXT_JULY));
			SendDlgItemMessage(win, IDC_BMONTH, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)Fetch_String(TXT_AUGUST));
			SendDlgItemMessage(win, IDC_BMONTH, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)Fetch_String(TXT_SEPTEMBER));
			SendDlgItemMessage(win, IDC_BMONTH, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)Fetch_String(TXT_OCTOBER));
			SendDlgItemMessage(win, IDC_BMONTH, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)Fetch_String(TXT_NOVEMBER));
			SendDlgItemMessage(win, IDC_BMONTH, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)Fetch_String(TXT_DECEMBER));
			SendDlgItemMessage(win, IDC_BMONTH, CB_SETCURSEL, (WPARAM)0, NULL);
		}
			return(false);

		default:
			return(false);
	}
	return(false);
}

/// <summary>
/// Window procedure for the "create new nickname" dialog, letting a player register a
/// new online nickname/password.
/// </summary>
/// <param name="win">Handle of the dialog window.</param>
/// <param name="uMsg">Window message being processed.</param>
/// <param name="wParam">Message-specific parameter.</param>
/// <param name="lParam">Message-specific parameter.</param>
/// <returns>TRUE if the message was handled, FALSE otherwise.</returns>
BOOL CALLBACK WOL_New_Nick_Dialog_Proc(HWND win, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
		case WM_COMMAND: {
			switch (LOWORD(wParam)) {
				case IDOK:
					WS_Destroy_Dialog(win, IDOK);
					break;

				case IDCANCEL:
					WS_Destroy_Dialog(win, IDCANCEL);
					break;

				default:
					return(false);
			}
		} break;

		case WM_DRAWITEM:
			OwnerDraw::Draw_Item((LPDRAWITEMSTRUCT)lParam);
			return(true);

		case WM_PAINT:
			OwnerDraw::Draw_Dialog_Back(win);
			ValidateRect(win, NULL);
			return(false);

		case OD_SUBCLASSED:
			SendMessage(win, OD_SETTOP, 0, 1); /// stay on top
			break;

		/// Limit the nickname/password/verify field lengths and focus the nickname field.
		case WM_INITDIALOG: {
			SendDlgItemMessage(win, IDC_NICKNAME, EM_LIMITTEXT, MAX_USERNAME_LEN - 1, 0L);
			SendDlgItemMessage(win, IDC_PASSWORD, EM_LIMITTEXT, MAX_PASSWORD_LEN - 1, 0L);
			SendDlgItemMessage(win, IDC_VERIFY, EM_LIMITTEXT, MAX_PASSWORD_LEN - 1, 0L);
			SetFocus(GetDlgItem(win, IDC_NICKNAME));
		}
			return(false);

		default:
			return(false);
	}
	return(false);
}

/// <summary>
/// Window procedure for the WOL options dialog (allow-page, allow-find, language
/// filter, lobby music, and show-all-games checkboxes). Also lets the user open the
/// modal sound options dialog.
/// </summary>
/// <param name="win">Handle of the dialog window.</param>
/// <param name="uMsg">Window message being processed.</param>
/// <param name="wParam">Message-specific parameter.</param>
/// <param name="lParam">Message-specific parameter.</param>
/// <returns>TRUE if the message was handled, FALSE otherwise.</returns>
BOOL CALLBACK WOL_Options_Dialog_Proc(HWND win, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static LONG sound_result = 0;

	switch (uMsg) {
		case WM_COMMAND: {
			switch (LOWORD(wParam)) {
				case IDOK:
					WS_Destroy_Dialog(win, IDOK);
					break;

				case IDCANCEL:
					WS_Destroy_Dialog(win, IDCANCEL);
					break;

				/// Open the sound options dialog and pump messages until it closes.
				case IDC_SOUNDOPT: {
					GameActive = false;
					HWND dialog = WS_Create_Dialog(ProgramInstance, IDD_SOUND_OPTIONS_DIALOG_LITE, MainWindow, (DLGPROC)SoundControlsClass::Sound_Option_Dialog_Func, FALSE);
					sound_result = 0;
					SetWindowLong(dialog, DWL_USER, (LONG)&sound_result);
					SendMessage(dialog, OD_SETTOP, 0, 1);
					ShowWindow(dialog, SW_SHOWNORMAL);
					SetFocus(dialog);

					while (sound_result == 0)
						WOL_Wait_Callback();
					WS_Destroy_Dialog(dialog, 0);
				} break;

				default:
					return(FALSE);
			}
		} break;

		/// Reflect the current global settings into the checkboxes.
		case WM_INITDIALOG: {
			if (g_AllowPage) SendDlgItemMessage(win, IDC_PAGEME, BM_SETCHECK, BST_CHECKED, 0);
			if (g_AllowFind) SendDlgItemMessage(win, IDC_FINDME, BM_SETCHECK, BST_CHECKED, 0);
			if (g_LangFilter) SendDlgItemMessage(win, IDC_LANGFILT, BM_SETCHECK, BST_CHECKED, 0);
			if (g_LobbyMusic) SendDlgItemMessage(win, IDC_MUSIC, BM_SETCHECK, BST_CHECKED, 0);
			if (g_ShowAllChannels) SendDlgItemMessage(win, IDC_LOBGAME, BM_SETCHECK, BST_CHECKED, 0);
		} break;

		/*
		 * Restore the game-active state that was cleared while the sound options
		 * dialog (if any) was open.
		 */
		case WM_DESTROY:
			GameActive = true;
			break;

		case WM_DRAWITEM:
			OwnerDraw::Draw_Item((LPDRAWITEMSTRUCT)lParam);
			return(true);

		case WM_PAINT:
			OwnerDraw::Draw_Dialog_Back(win);
			ValidateRect(win, NULL);
			return(false);

		default:
			return(false);
	}
	return(false);
}

/// <summary>
/// Window procedure for the "create new chat channel" dialog, letting a player enter a
/// name for a new chat channel to create.
/// </summary>
/// <param name="win">Handle of the dialog window.</param>
/// <param name="uMsg">Window message being processed.</param>
/// <param name="wParam">Message-specific parameter.</param>
/// <param name="lParam">Message-specific parameter.</param>
/// <returns>TRUE if the message was handled, FALSE otherwise.</returns>
BOOL CALLBACK WOL_New_Chat_Dialog_Proc(HWND win, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
		case WM_COMMAND: {
			switch (LOWORD(wParam)) {
				case IDOK:
					WS_Destroy_Dialog(win, IDOK);
					break;

				case IDCANCEL:
					WS_Destroy_Dialog(win, IDCANCEL);
					break;

				default:
					return(false);
			}
		} break;

		case WM_DRAWITEM: {
			DRAWITEMSTRUCT * di = (LPDRAWITEMSTRUCT)lParam;
			OwnerDraw::Draw_Item(di);
		}
			return(true);

		case WM_PAINT:
			OwnerDraw::Draw_Dialog_Back(win);
			ValidateRect(win, NULL);
			return(false);

		/// Limit the channel-name field length and give it initial focus.
		case WM_INITDIALOG: {
			SendDlgItemMessage(win, IDC_CHANNAME, EM_LIMITTEXT, 16, 0L);
			SetFocus(GetDlgItem(win, IDC_CHANNAME));
		}
			return(false);

		default:
			return(false);
	}
	return(false);
}

/// <summary>
/// Dialog procedure for the host's game-options screen (IDD_WOL_GAMEOPT). Handles the
/// option sliders (AI difficulty/count, unit count, tech level, credits, game speed) and
/// checkboxes (fog of war, bases, bridge destruction, crates, MCV redeploy, allies,
/// harvester truce, short game), scenario/random-map selection, the player house and
/// color combo boxes, chat input, and the Go/kick/ban/squelch buttons. Changing most
/// options re-publishes them to the channel via PumpGameopts and clears every other
/// player's "accepted" flag so they must re-confirm.
/// </summary>
/// <param name="win">Handle of the dialog window.</param>
/// <param name="uMsg">Window message being processed.</param>
/// <param name="wParam">Message-specific parameter.</param>
/// <param name="lParam">Message-specific parameter.</param>
/// <returns>TRUE or FALSE depending on the message handled; see individual cases.</returns>
BOOL CALLBACK WOL_Game_Options_Proc(HWND win, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int i;

	/*
	 * Let the shared button bar (chat/user-list resize bar) handle its own messages first.
	 */
	if (WOL_Button_Bar_Proc(win, uMsg, wParam, lParam)) return(TRUE);

	/*
	 * While the game has not been started yet, process option changes coming from the
	 * sliders and checkboxes below and mirror them into Session.Options. Note this
	 * function switches on uMsg three separate times in sequence (this block, the
	 * window-teardown switch, and the main control switch further down), so a single
	 * message such as WM_INITDIALOG can be handled by more than one of them.
	 */
	if (!g_GameStarted) {
		bool changed = false;

		switch (uMsg) {
			/*
			 * Slider messages: AI difficulty, AI player count, unit count, tech level,
			 * credits, and game speed.
			 */
			case WM_HSCROLL:
				if (GetDlgItem(win, IDC_AILEVEL_SLIDER)) {
					Session.Options.AIDifficulty = (DiffType)SendDlgItemMessage(win, IDC_AILEVEL_SLIDER, TBM_GETPOS, 0, 0);
				}
				if (GetDlgItem(win, IDC_AIPLAYERS)) {
					Session.Options.AIPlayers = SendDlgItemMessage(win, IDC_AIPLAYERS, TBM_GETPOS, 0, 0);
				}
				if (GetDlgItem(win, IDC_UNITCOUNT)) {
					Session.Options.UnitCount = SendDlgItemMessage(win, IDC_UNITCOUNT, TBM_GETPOS, 0, 0);
				}
				if (GetDlgItem(win, IDC_TECHLEVEL)) {
					BuildLevel = SendDlgItemMessage(win, IDC_TECHLEVEL, TBM_GETPOS, 0, 0);
				}
				if (GetDlgItem(win, IDC_CREDITS)) {
					Session.Options.Credits = SendDlgItemMessage(win, IDC_CREDITS, TBM_GETPOS, 0, 0);
				}
				if (GetDlgItem(win, IDC_GAME_SPEED_SLIDER)) {
					Session.Options.GameSpeed = 6 - SendDlgItemMessage(win, IDC_GAME_SPEED_SLIDER, TBM_GETPOS, 0, 0);
				}

				changed = true;
				break;

			/// Checkbox toggles: read the new checked state into Session.Options.
			case WM_COMMAND:
				switch (LOWORD(wParam)) {

					/// Destroyable bridges checkbox.
					case IDC_BRIDGE_DESTROY:
						Session.Options.BridgeDestruction = false;
						if (SendDlgItemMessage(win, IDC_BRIDGE_DESTROY, BM_GETCHECK, 0, 0) == BST_CHECKED) {
							Session.Options.BridgeDestruction = true;
						}
						changed = true;
						break;

					/// Allow allied teams checkbox.
					case IDC_ALLIES:
						Session.Options.AlliesAllowed = false;
						if (SendDlgItemMessage(win, IDC_ALLIES, BM_GETCHECK, 0, 0) == BST_CHECKED) {
							Session.Options.AlliesAllowed = true;
						}
						changed = true;
						break;

					/// Harvester truce (no attacking harvesters) checkbox.
					case IDC_HARVTRUCE:
						Session.Options.HarvTruce = false;
						if (SendDlgItemMessage(win, IDC_HARVTRUCE, BM_GETCHECK, 0, 0) == BST_CHECKED) {
							Session.Options.HarvTruce = true;
						}
						changed = true;
						break;

					/// Fog of war checkbox.
					case IDC_FOG_OF_WAR:
						Session.Options.FogOfWar = false;
						if (SendDlgItemMessage(win, IDC_FOG_OF_WAR, BM_GETCHECK, 0, 0) == BST_CHECKED) {
							Session.Options.FogOfWar = true;
						}
						changed = true;
						break;

					/// Multi-engineer (weakened engineer capture) checkbox.
					case IDC_MULTI_ENGINEER:
						Session.Options.CrapEngineers = false;
						if (SendDlgItemMessage(win, IDC_MULTI_ENGINEER, BM_GETCHECK, 0, 0) == BST_CHECKED) {
							Session.Options.CrapEngineers = true;
						}
						changed = true;
						break;

					/*
					 * Bases checkbox; turning bases off also forces short game off (short
					 * game requires bases).
					 */
					case IDC_BASES:
						Session.Options.Bases = false;
						if (SendDlgItemMessage(win, IDC_BASES, BM_GETCHECK, 0, 0) == BST_CHECKED) {
							Session.Options.Bases = true;
						} else if (!Session.Options.Bases) {
							Session.Options.ShortGame = false;
							SendDlgItemMessage(win, IDC_SHORT_GAME, BM_SETCHECK, BST_UNCHECKED, 0);
						}
						changed = true;
						break;

					/// Crates/goodies checkbox.
					case IDC_CRATES:
						Session.Options.Goodies = false;
						if (SendDlgItemMessage(win, IDC_CRATES, BM_GETCHECK, 0, 0) == BST_CHECKED) {
							Session.Options.Goodies = true;
						}
						changed = true;
						break;

					/// MCV redeploy allowed checkbox.
					case IDC_REDEPLOY_MCV:
						Session.Options.MCVRedeploy = false;
						if (SendDlgItemMessage(win, IDC_REDEPLOY_MCV, BM_GETCHECK, 0, 0) == BST_CHECKED) {
							Session.Options.MCVRedeploy = true;
						}
						changed = true;
						break;

					/*
					 * Short game checkbox; enabling it also forces bases on (short game
					 * requires bases).
					 */
					case IDC_SHORT_GAME:
						Session.Options.ShortGame = false;
						if (SendDlgItemMessage(win, IDC_SHORT_GAME, BM_GETCHECK, 0, 0) == BST_CHECKED) {
							Session.Options.ShortGame = true;
							if (!Session.Options.Bases) {
								Session.Options.Bases = true;
								SendDlgItemMessage(win, IDC_BASES, BM_SETCHECK, BST_CHECKED, 0);
							}
						}
						changed = true;
						break;
				}
				break;

			/*
			 * Clan/tournament channels (reserved bit 0x100, or a tournament channel)
			 * disable AI players and allied teams entirely, then redisplay all controls.
			 */
			case WM_INITDIALOG:
			case OD_SUBCLASSED:
				if ((g_CurrentChannel.reserved & 0x100) != 0 || g_CurrentChannel.tournament) {
					EnableWindow(GetDlgItem(win, IDC_AIPLAYERS), FALSE);
					EnableWindow(GetDlgItem(win, IDC_AILEVEL_SLIDER), FALSE);
					EnableWindow(GetDlgItem(win, IDC_ALLIES), FALSE);
					Session.Options.AIPlayers = 0;
					Session.Options.AlliesAllowed = 0;
				}
				DisplayGameopts(win, TRUE);
				break;
		}

		/// An option changed: every other player must re-accept the new settings.
		if (changed) {
			for (i = 1; i < MAX_PLAYERS; i++) {
				g_UserInfo[i].accepted = 0;
			}
			Draw_Player_List();
		}
	}

	/*
	 * Main dialog switch: window-level messages, one-time setup, scenario/random-map
	 * selection, and every WM_COMMAND control handler.
	 */
	switch (uMsg) {
		/// Per-control command handlers.
		case WM_COMMAND: {
			switch (LOWORD(wParam)) {
				case IDOK:
					WS_Destroy_Dialog(win, IDOK);
					return(false);

				/// Leave the channel and return to the main lobby window.
				case IDCANCEL: {
					if (!g_GameStarted) {
						g_pChat->RequestChannelLeave();

						HWND main_window = WS_Find_Dialog(IDD_WOL_MAIN);
						ShowWindow(main_window, SW_RESTORE);

						WS_Destroy_Dialog(win, IDCANCEL);
						g_JoinLobbyNow = 1;
					}
					return(false);
				}

				/// "/me"-style action command typed into the chat action box.
				case IDC_ACTION: {
					char input[257];
					SendDlgItemMessage(win, IDC_INPUT, WM_GETTEXT, 256, (LPARAM)input);
					SendDlgItemMessage(win, IDC_INPUT, WM_SETTEXT, 0, (LPARAM)"");
					SetFocus(GetDlgItem(win, IDC_INPUT));

					int color = ColorAction;
					if (strlen((char *)g_CurrentChannel.name) == 0) {
						PMessagePrintf(-1, Fetch_String(TXT_NOT_IN_CHAN));
					} else if (strlen(input)) {
						if (Send_Chat_Action(input)) {
							color = ColorPrivAction;
						}
						PMessagePrintf(color, "%s %s", g_NickName, input);
					} else {
						PMessagePrintf(-1, Fetch_String(TXT_ENTER_MESSAGE));
					}
					return(false);
				}

				/*
				 * Chat input box: on EN_MAXTEXT, read the typed text, clear the box, and
				 * broadcast it to the channel (as a private message if Send_Chat_Message says so).
				 */
				case IDC_INPUT: {
					char input[257];
					SendDlgItemMessage(win, IDC_INPUT, WM_GETTEXT, 256, (LPARAM)input);
					int len = strlen(input);
					if (HIWORD(wParam) == EN_MAXTEXT) {
						SendDlgItemMessage(win, IDC_INPUT, WM_SETTEXT, 0, (LPARAM)"");
						if (len > 2) {
							int color = ColorMe;
							if (Send_Chat_Message(input) == 1) {
								color = ColorPriv;
							}
							PMessagePrintf(color, "[%s] %s", g_NickName, input);
						}
					}
					return(false);
				}

				/// House selection combo box.
				case IDC_YOURSIDE: {
					if (HIWORD(wParam) != CBN_SELCHANGE || g_GameStarted) {
						return(false);
					}

					Session.House = SendDlgItemMessage(win, IDC_YOURSIDE, CB_GETCURSEL, 0, 0);
					int index = Player_Name_To_Index(g_NickName);
					if (index != -1) {
						g_UserInfo[index].house = Session.House;
						PumpGameopts(TRUE, FALSE);
					}
					Draw_Player_List();
					return(false);
				}

				/// Player color selection combo box.
				case IDC_YOURCOLOR: {
					if (HIWORD(wParam) != CBN_SELCHANGE || g_GameStarted) {
						return(false);
					}

					Session.ColorIdx = SendDlgItemMessage(win, IDC_YOURCOLOR, CB_GETCURSEL, 0, 0);
					int reqcolor = Session.ColorIdx;
					int index = Player_Name_To_Index(g_NickName);
					int taken=0;

					/// Walk forward from the requested color until an untaken one is found.
					while (1) {
						g_UserInfo[index].color = reqcolor;
						taken = false;
						for (i = 0; i < g_UserList.length(); i++) {
							if (i == index) continue;
							if (g_UserInfo[i].color == reqcolor) {
								reqcolor++;
								taken = true;
							}
						}

						if (taken == 0) break;
						reqcolor %= MAX_MPLAYER_COLORS;
					}

					if (reqcolor != Session.ColorIdx) {
						PMessagePrintf(ColorSystem, Fetch_String(TXT_COLOR_IN_USE));
					}

					Session.ColorIdx = reqcolor;
					SendDlgItemMessage(win, IDC_YOURCOLOR, CB_SETCURSEL, (WPARAM)reqcolor, 0);
					Draw_Player_List();
					PumpGameopts(TRUE, FALSE);
					break;
				}

				/*
				 * Map/scenario selection button: hide the dialog, run the scenario picker
				 * (or the random-map generator for a WDT/ladder game), then either restore
				 * the previous scenario on cancel or apply the newly chosen one.
				 */
				case IDC_MULTIMAP: {
					if (g_GameStarted) {
						return(false);
					}

					int old_scenario_index = Session.Options.ScenarioIndex;
					char old_scenario_file[sizeof(Session.ScenarioFileName)];
					char old_scenario_desc[sizeof(Session.Options.ScenarioDescription)];
					strcpy(old_scenario_file, Session.ScenarioFileName);
					strcpy(old_scenario_desc, Session.Options.ScenarioDescription);

					ShowWindow(win, SW_HIDE);
					IsRandomMap = false;

					bool cancelled;
					if (Session.Type == GAME_INTERNET && Session.IsWDT && WDT_Get_Territory(Session.WDTTerritory)) {
						cancelled = CreateRandomMap() != -1;
					} else {
						cancelled = Scenario_Dialog(MainWindow) == 2;
					}

					/// Selection was cancelled: restore the previous scenario and preview.
					if (cancelled) {
						Session.Options.ScenarioIndex = old_scenario_index;
						Set_Scenario_Info_From_Index(old_scenario_index);
						Update_Network_Dialog_Preview(win);
						IsRandomMap = true;
						ShowWindow(win, SW_SHOW);
						if (!stricmp(Session.Scenarios[Session.Options.ScenarioIndex]->Get_Filename(), RANDOM_MAP_FILE_NAME)) {
							delete MultiplayerMapPreview;
							MultiplayerMapPreview = new MapPreviewClass;
							MultiplayerMapPreview->Read_PCX_Preview("RandMap.img");
							if (MultiplayerMapPreview->Get_Preview_Surface() == NULL) {
								Update_Network_Dialog_Preview(win);
							}
							InvalidateRect(win, NULL, FALSE);
						} else {
							Update_Network_Dialog_Preview(win);
						}
					} else {
						/*
						 * New scenario/map accepted: apply it, and if it actually differs
						 * from the old one, every other player must re-accept.
						 */
						if (!Set_Scenario_Info_From_Index(Session.Options.ScenarioIndex)) {
							Session.Options.ScenarioIndex = old_scenario_index;
						}

						IsRandomMap = true;
						ShowWindow(win, SW_SHOW);

						if (strcmp(old_scenario_file, Session.ScenarioFileName) || strcmp(old_scenario_desc, Session.Options.ScenarioDescription)) {
							for (i = 1; i < MAX_PLAYERS; i++) {
								g_UserInfo[i].accepted = 0;
							}
						}

						Draw_Player_List();
						SendDlgItemMessage(win, IDC_SCENARIONAME, WM_SETTEXT, 0, (LPARAM)Session.Options.ScenarioDescription);

						if (!stricmp(Session.Scenarios[Session.Options.ScenarioIndex]->Get_Filename(), RANDOM_MAP_FILE_NAME)) {
							delete MultiplayerMapPreview;
							MultiplayerMapPreview = new MapPreviewClass;
							MultiplayerMapPreview->Read_PCX_Preview("RandMap.img");
							if (MultiplayerMapPreview->Get_Preview_Surface() == NULL) {
								Update_Network_Dialog_Preview(win);
							}
							InvalidateRect(win, NULL, FALSE);
						} else {
							Update_Network_Dialog_Preview(win);
						}
					}

					PumpGameopts(TRUE, FALSE);
					InvalidateRect(win, NULL, FALSE);
					return(false);
				}

				/*
				 * Start-game button. Validates settings and player state, then hands the
				 * user list to the chat layer to actually launch the game.
				 */
				case IDC_GO: {
					if (g_GameStartRequested == 1) {
						return(false);
					}

					Draw_Player_List();

					/// Need at least two players to start.
					if (g_UserList.length() < 2) {
						PMessagePrintf(-1, Fetch_String(TXT_ONLY_ONE));
						return(false);
					}

					/// Every player must have accepted the current game options.
					for (i = 0; i < g_UserList.length(); i++) {
						if (g_UserInfo[i].accepted == 0) {
							PMessagePrintf(-1, Fetch_String(TXT_ACCEPTFIRST));
							return(false);
						}
					}

					/// WDT (ladder) games require a full channel with an even split of houses.
					if (Session.IsWDT) {
						if (g_UserList.length() < (int)g_CurrentChannel.maxUsers) {
							PMessagePrintf(-1, Fetch_String(TXT_NEED_PLAYERS_TO_START), g_CurrentChannel.maxUsers);
							return(false);
						}

						int nonzero_houses = 0;
						int zero_houses = 0;
						for (i = 0; i < g_UserList.length(); i++) {
							if (g_UserInfo[i].house == HOUSE_GOOD) {
								zero_houses++;
							} else {
								nonzero_houses++;
							}
						}
						if (nonzero_houses != zero_houses) {
							PMessagePrintf(-1, Fetch_String(TXT_NEED_EQUAL_TEAMS));
							return(false);
						}
					}

					/// The map must have enough waypoints for the human players plus AI players.
					int waypoints = RandomMapWaypointCount(Session.Options.ScenarioIndex);
					if (waypoints < g_UserList.length() + SendDlgItemMessage(win, IDC_AIPLAYERS, TBM_GETPOS, 0, 0)) {
						PMessagePrintf(-1, Fetch_String(TXT_SCENARIO_TOO_SMALL), waypoints);
						return(false);
					}

					/*
					 * Clan/tournament channels (reserved bit 0x100) require exactly two
					 * evenly sized squads among the players.
					 */
					if ((g_CurrentChannel.reserved & 0x100) != 0) {
						User *user = NULL;
						unsigned int squad1 = 0;
						unsigned int squad2 = 0;
						int squad1_count = 0;
						int squad2_count = 0;

						for (i = 0; i < g_UserList.length(); i++) {
							g_UserList.getPointer(&user, i);

							if (user->squadID < 1) {
								PMessagePrintf(-1, Fetch_String(TXT_CLAN_NOSTART));
								return(false);
							}

							if (!squad1 || squad1 == user->squadID) {
								squad1 = user->squadID;
								squad1_count++;
							} else {
								if (squad2 && squad2 != user->squadID) {
									PMessagePrintf(-1, Fetch_String(TXT_CLAN_NOSTART));
									return(false);
								}
								squad2 = user->squadID;
								squad2_count++;
							}
						}

						if (squad1_count != squad2_count) {
							PMessagePrintf(-1, Fetch_String(TXT_CLAN_NOSTART));
							return(false);
						}
					}

					/*
					 * All checks passed: publish the final options, mark the game started,
					 * and hand a copy of the user list to the chat layer to start the game.
					 */
					PumpGameopts(TRUE, TRUE);
					g_GameStarted = true;

					User *temp, *head = NULL;
					User slot_user;
					memset(&slot_user, 0, sizeof(slot_user));

					for (i = 0; i < g_UserList.length(); i++) {
						g_UserList.get(slot_user, i);
						temp = new User;
						*temp = slot_user;
						temp->next = head;
						head = temp;
					}

					g_pChat->RequestGameStart(head);
					g_GameStartRequested = true;
					temp = head;
					while (temp) {
						head = temp->next;
						delete (temp);
						temp = head;
					}
					return(false);
				}

				/*
				 * Squelch/Kick/Ban buttons: apply the requested action to every selected
				 * user in the player list.
				 */
				case IDC_SQUELCH:
				case IDC_KICK:
				case IDC_BAN: {
					User user;
					int squelch;
					Wstring name;
					bool selected = false;
					bool found = false;
					bool not_operator = false;
					int i;
					HWND userwin = GetDlgItem(WS_Find_Dialog(IDD_WOL_GAMEOPT), IDC_USERS);

					/// Kick/Ban require the local user to be the channel owner.
					if ((LOWORD(wParam) == IDC_KICK) || (LOWORD(wParam) == IDC_BAN)) {

						for (i = 0; i < g_UserList.length(); i++) {
							g_UserList.get(user, i);
							if ((strcasecmp((char *)user.name, g_NickName) == 0) && ((user.flags & CHAT_USER_CHANNELOWNER) == 0)) {
								PMessagePrintf(-1, Fetch_String(TXT_NOTCHANOP));
								not_operator = true;
								break;
							}
						}
						if (not_operator) break;
					}

					Dictionary<Wstring, bool> lbdict(Wstring_Hash);
					LBSaveSelections(userwin, lbdict);

					if (lbdict.getEntries()) Sound_Effect(Rule->GenericBeep);

					/*
					 * Apply the action to each selected user (skipping the local user).
					 */
					found = false;
					while (lbdict.removeAny(name, selected)) {
						strcpy((char *)user.name, name.get());

						if (stricmp((char *)user.name, g_NickName) == 0) {
							continue;
						}
						found = true;

						if (LOWORD(wParam) == IDC_SQUELCH) {
							if (g_pChat->GetSquelch(&user) == S_OK) {
								squelch = 0;
							} else {
								squelch = 1;
							}
							g_pChat->SetSquelch(&user, squelch);
						} else if (LOWORD(wParam) == IDC_KICK) {
							g_pChat->RequestUserKick(&user);
						} else if (LOWORD(wParam) == IDC_BAN) {
							g_pChat->RequestChannelBan((char *)user.name, 1);
							g_pChat->RequestUserKick(&user);
						}
					}
					if (!found) {
						PMessagePrintf(-1, Fetch_String(TXT_SEL_USER));
					}
					Draw_Player_List(1);
					return(TRUE);
				}

				default:
					return(FALSE);
			}
			break;
		}

		case WM_MOVE:
			break;

		/*
		 * Dialog initialization: populate the house and color combo boxes, then select
		 * the initial scenario (or kick off random-map generation for a WDT/ladder game).
		 */
		case WM_INITDIALOG: {
			SendDlgItemMessage(win, IDC_YOURSIDE, CB_RESETCONTENT, 0, 0);
			for (int h = 0; h < HouseTypes.Count(); h++) {
				if (HouseTypes[h]->IsMultiplay) {
					SendDlgItemMessage(win, IDC_YOURSIDE, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)((const char *)HouseTypes[h]->GivenName));
				}
			}
			SendDlgItemMessage(win, IDC_YOURSIDE, CB_SETCURSEL, Session.House, 0);

			SendDlgItemMessage(win, IDC_YOURCOLOR, CB_RESETCONTENT, 0, 0);
			SendDlgItemMessage(win, IDC_YOURCOLOR, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)Fetch_String(TXT_GOLD));
			SendDlgItemMessage(win, IDC_YOURCOLOR, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)Fetch_String(TXT_RED));
			SendDlgItemMessage(win, IDC_YOURCOLOR, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)Fetch_String(TXT_BLUE));
			SendDlgItemMessage(win, IDC_YOURCOLOR, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)Fetch_String(TXT_GREEN));
			SendDlgItemMessage(win, IDC_YOURCOLOR, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)Fetch_String(TXT_ORANGE));
			SendDlgItemMessage(win, IDC_YOURCOLOR, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)Fetch_String(TXT_SKY_BLUE));
			SendDlgItemMessage(win, IDC_YOURCOLOR, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)Fetch_String(TXT_PURPLE));
			SendDlgItemMessage(win, IDC_YOURCOLOR, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)Fetch_String(TXT_PINK));
			SendDlgItemMessage(win, IDC_YOURCOLOR, CB_SETCURSEL, Session.ColorIdx, 0);

			int scenario = Session.IsWDT ? -1 : 0;
			Set_Scenario_Info_From_Index(scenario);
			Session.Options.ScenarioIndex = scenario;
			IsRandomMap = false;
			if (scenario == -1) {
				delete MultiplayerMapPreview;
				MultiplayerMapPreview = NULL;
				PostMessage(win, WOL_GENERATE_RANDMAP, 0, 0);
			}

			SendDlgItemMessage(win, IDC_SCENARIONAME, WM_SETTEXT, 0, (LPARAM)Session.Options.ScenarioDescription);
			Update_Network_Dialog_Preview(win);
			Session.PlayingAgainstVersion = VerNum.Version_Number();
			return(false);
		}

		case WM_DRAWITEM:
			OwnerDraw::Draw_Item((LPDRAWITEMSTRUCT)lParam);
			return(true);

		/// Background is repainted in WM_PAINT below; suppress the default erase.
		case WM_ERASEBKGND:
			return(true);

		/*
		 * Ownerdraw subclassing complete: add the user-list report columns, register the
		 * player color swatches, and show the initial scenario.
		 */
		case OD_SUBCLASSED:
			g_Col_Accept = 3;
			g_Col_House = 20;
			g_Col_Gamename = 37;
			g_Col_Gameinfo = 115;
			g_Col_GamePing = 190;
			SendDlgItemMessage(win, IDC_USERS, OD_ADDCOLUMN, 0, (LPARAM)g_Col_Gamename);
			SendDlgItemMessage(win, IDC_USERS, OD_ADDCOLUMN, 0, (LPARAM)g_Col_Gameinfo);
			SendDlgItemMessage(win, IDC_USERS, OD_ADDCOLUMN, 0, (LPARAM)g_Col_House);
			SendDlgItemMessage(win, IDC_USERS, OD_ADDCOLUMN, 0, (LPARAM)g_Col_Accept);
			SendDlgItemMessage(win, IDC_USERS, OD_ADDCOLUMN, 0, (LPARAM)g_Col_GamePing);
			for (i = 0; i < ARRAY_SIZE(PlayerColorTable); i++) {
				SendDlgItemMessage(win, IDC_YOURCOLOR, OD_SETCOLOR, (WPARAM)i, (LPARAM)PlayerColorTable[i]);
			}
			DebugString("****** Dlg Init *******\n");
			SendDlgItemMessage(win, IDC_USERS, OD_TOOLTIPS, 0, (LPARAM)1);
			Set_Scenario_Info_From_Index(Session.Options.ScenarioIndex);
			Draw_Player_List();
			return(false);

		/// Free the scenario preview bitmap when the dialog is destroyed.
		case WM_DESTROY:
			if (MultiplayerMapPreview == NULL) {
				return(false);
			}
			delete MultiplayerMapPreview;
			MultiplayerMapPreview = NULL;
			return(false);

		/// Redraw the dialog background and user list, then blit the scenario/random-map preview.
		case WM_PAINT:
			OwnerDraw::Draw_Dialog_Back(win);
			Draw_Player_List();
			if (MultiplayerMapPreview != NULL) {
				MultiplayerMapPreview->Blit_Preview(win);
			} else if (Session.Options.ScenarioIndex == -1 && RandomMapGen.MapPreview != NULL) {
				RandomMapGen.MapPreview->Blit_Preview(win);
			}
			ValidateRect(win, NULL);
			return(false);

		/*
		 * Posted by WM_INITDIALOG for a WDT (ladder) game: generate a fresh random map,
		 * make it the current scenario, and refresh the preview and scenario list so
		 * other players see it.
		 */
		case WOL_GENERATE_RANDMAP: {
			if (Session.Options.ScenarioIndex != -1) {
				return(false);
			}

			SendDlgItemMessage(win, IDC_SCENARIONAME, WM_SETTEXT, 0, (LPARAM)RandomMapGen.SeedData.MapDescription);
			IsRandomMap = false;
			RandomMapGen.SeedData.NumPlayers = 4;
			strcpy(Session.ScenarioFileName, RANDOM_MAP_FILE_NAME);
			Do_Random_Map(win, WOL_Wait_Callback);
			RandomMapGen.SeedData.Save(RANDOM_MAP_FILE_NAME);
			IsRandomMap = true;

			if (MultiplayerMapPreview != NULL) {
				delete MultiplayerMapPreview;
			}
			MultiplayerMapPreview = new MapPreviewClass;
			MultiplayerMapPreview->Read_PCX_Preview("RandMap.img");

			bool found = false;
			int index = 0;
			for (; index < Session.Scenarios.Count(); index++) {
				if (stricmp(Session.Scenarios[index]->Get_Filename(), RANDOM_MAP_FILE_NAME) == 0) {
					found = true;
					break;
				}
			}

			char * digest = CalcRandomMapDigest();
			if (!found) {
				Session.Scenarios.Add(new MultiMission(RANDOM_MAP_FILE_NAME, RandomMapGen.SeedData.MapDescription, digest, true));
			} else {
				Session.Scenarios[index]->Set_Digest(digest);
				Session.Scenarios[index]->Set_Description(RandomMapGen.SeedData.MapDescription);
			}
			delete digest;

			if (Set_Scenario_Info_From_Index(index) == true) {
				Session.Options.ScenarioIndex = index;
			}
			if (MultiplayerMapPreview->Get_Preview_Surface() == NULL) {
				Update_Network_Dialog_Preview(win);
			}
			PumpGameopts(TRUE, FALSE);
			InvalidateRect(win, NULL, FALSE);
			IsRandomMap = true;
			return(false);
		}

		default:
			return(false);

	}
	return(false);
}

/// <summary>
/// Dialog procedure for the guest's (non-host) game-options dialog. Displays the host's
/// chosen game options and the current channel's user list, lets the guest pick a preferred
/// house/color and send chat messages or actions, and handles the Accept button that tells
/// the host the guest is ready. Also sets up the owner-draw user list columns and draws the
/// map preview on WM_PAINT.
/// </summary>
/// <param name="win">Handle of the dialog window.</param>
/// <param name="uMsg">Window message being processed.</param>
/// <param name="wParam">Message-specific parameter.</param>
/// <param name="lParam">Message-specific parameter.</param>
/// <returns>TRUE if the message was handled, FALSE otherwise.</returns>
BOOL CALLBACK WOL_Guest_Dialog_Proc(HWND win, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (WOL_Button_Bar_Proc(win, uMsg, wParam, lParam)) return(TRUE);

	switch (uMsg) {
		case WM_COMMAND: {
			switch (LOWORD(wParam)) {
				case IDOK:
					WS_Destroy_Dialog(win, IDOK);
					break;

				/*
				 * Ask to leave the channel; if the game hasn't started and the request
				 * couldn't even be sent, destroy the dialog immediately.
				 */
				case IDCANCEL: {
					if (!g_GameStarted && FAILED(g_pChat->RequestChannelLeave())) {
						WS_Destroy_Dialog(win, IDCANCEL);
					}
				} break;

				/// Mark ourselves as accepted and tell the host, so every user's list updates.
				case IDC_ACCEPT: {
					int offset = Player_Name_To_Index(g_NickName);

					g_UserInfo[offset].accepted = 1;

					char options_str[64];

					sprintf(options_str, "A%d", g_UserInfo[offset].accepted);
					g_pChat->RequestPublicGameOptions(options_str);

					EnableWindow(GetDlgItem(win, IDC_ACCEPT), 0);

					Draw_Player_List();
				} break;

				/*
				 * Guest changed the preferred house/color combo; store it locally and ask
				 * the host to broadcast the change to everyone else.
				 */
				case IDC_YOURCOLOR:
				case IDC_YOURSIDE: {
					if (HIWORD(wParam) != CBN_SELCHANGE) break;

					int reqcolor = SendDlgItemMessage(win, IDC_YOURCOLOR, CB_GETCURSEL, 0, 0);
					int requested_house = SendDlgItemMessage(win, IDC_YOURSIDE, CB_GETCURSEL, 0, 0);
					Session.House = requested_house;
					Session.PrefColor = reqcolor;

					char options_str[64];

					sprintf(options_str, "R%d,%d", requested_house, reqcolor);

					if (g_UserList.length() > 0) {
						User host;
						g_UserList.get(host, 0);
						g_pChat->RequestPrivateGameOptions(&host, options_str);
					}
				} break;

				/*
				 * Chat input box; when the text hits its length limit on Enter, send the
				 * typed line as a public or private chat message.
				 */
				case IDC_INPUT: {
					char input[257];
					int len;

					SendDlgItemMessage(win, IDC_INPUT, WM_GETTEXT, 256, (LPARAM)input);
					len = strlen(input);

					if (HIWORD(wParam) == EN_MAXTEXT) {
						SendDlgItemMessage(win, IDC_INPUT, WM_SETTEXT, 0, (LPARAM) "");
						if (len > 2) {
							int color = ColorMe;
							if (Send_Chat_Message(input) == 1) color = ColorPriv;
							PMessagePrintf(color, "[%s] %s", g_NickName, input);
						}
					}
				} break;

				/// "/me" style action line; requires that we are actually in a channel.
				case IDC_ACTION: {
					char input[257];
					SendDlgItemMessage(win, IDC_INPUT, WM_GETTEXT, 256, (LPARAM)input);
					SendDlgItemMessage(win, IDC_INPUT, WM_SETTEXT, 0, (LPARAM) "");
					SetFocus(GetDlgItem(win, IDC_INPUT));
					int color = ColorAction;
					if (strlen((char *)g_CurrentChannel.name) == 0) {
						PMessagePrintf(-1, Fetch_String(TXT_NOT_IN_CHAN));
					} else if (strlen(input)) {
						if (Send_Chat_Action(input)) color = ColorPrivAction;
						PMessagePrintf(color, "%s %s", g_NickName, input);
					} else {
						PMessagePrintf(-1, Fetch_String(TXT_ENTER_MESSAGE));
					}
				} break;

				default:
					return(FALSE);
			}
		} break;

		/// Populate the house and color combo boxes and refresh the user list display.
		case WM_INITDIALOG: {
			SendDlgItemMessage(win, IDC_YOURSIDE, CB_RESETCONTENT, 0, 0);
			for (int i = 0; i < HouseTypes.Count(); i++)
				if (HouseTypes[i]->IsMultiplay) {
					SendDlgItemMessage(win, IDC_YOURSIDE, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)((const char *)HouseTypes[i]->GivenName));
				}
			SendDlgItemMessage(win, IDC_YOURSIDE, CB_SETCURSEL, (WPARAM)Session.House, NULL);

			SendDlgItemMessage(win, IDC_YOURCOLOR, CB_RESETCONTENT, 0, 0);
			SendDlgItemMessage(win, IDC_YOURCOLOR, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)Fetch_String(TXT_GOLD));
			SendDlgItemMessage(win, IDC_YOURCOLOR, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)Fetch_String(TXT_RED));
			SendDlgItemMessage(win, IDC_YOURCOLOR, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)Fetch_String(TXT_BLUE));
			SendDlgItemMessage(win, IDC_YOURCOLOR, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)Fetch_String(TXT_GREEN));
			SendDlgItemMessage(win, IDC_YOURCOLOR, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)Fetch_String(TXT_ORANGE));
			SendDlgItemMessage(win, IDC_YOURCOLOR, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)Fetch_String(TXT_SKY_BLUE));
			SendDlgItemMessage(win, IDC_YOURCOLOR, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)Fetch_String(TXT_PURPLE));
			SendDlgItemMessage(win, IDC_YOURCOLOR, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)Fetch_String(TXT_PINK));
			SendDlgItemMessage(win, IDC_YOURCOLOR, CB_SETCURSEL, (WPARAM)Session.ColorIdx, NULL);

			Session.Options.ScenarioDescription[0] = 0;

			Draw_Player_List();
		}
			return(false);

		/// Set up the owner-draw user list's columns and the color combo's swatches.
		case OD_SUBCLASSED: {
			g_Col_Accept = 3;
			g_Col_House = 20;
			g_Col_Gamename = 37;
			g_Col_Gameinfo = 115;
			g_Col_GamePing = 190;
			SendDlgItemMessage(win, IDC_USERS, OD_ADDCOLUMN, 0, (LPARAM)g_Col_Gamename);
			SendDlgItemMessage(win, IDC_USERS, OD_ADDCOLUMN, 0, (LPARAM)g_Col_Gameinfo);
			SendDlgItemMessage(win, IDC_USERS, OD_ADDCOLUMN, 0, (LPARAM)g_Col_House);
			SendDlgItemMessage(win, IDC_USERS, OD_ADDCOLUMN, 0, (LPARAM)g_Col_Accept);
			SendDlgItemMessage(win, IDC_USERS, OD_ADDCOLUMN, 0, (LPARAM)g_Col_GamePing);
			for (int i = 0; i < ARRAY_SIZE(PlayerColorTable); i++) {
				SendDlgItemMessage(win, IDC_YOURCOLOR, OD_SETCOLOR, (WPARAM)i,(LPARAM)PlayerColorTable[i]);
			}
			SendDlgItemMessage(win, IDC_USERS, OD_TOOLTIPS, 0, (LPARAM)1);
			EnableWindow(GetDlgItem(win, IDC_ACCEPT), FALSE);
			DisplayGameopts(win, TRUE);
		} break;

		case WM_DESTROY:
			if (MultiplayerMapPreview) {
				delete MultiplayerMapPreview;
				MultiplayerMapPreview = NULL;
			}
			break;

		case WM_PAINT:
			OwnerDraw::Draw_Dialog_Back(win);
			/*
			 * Draw the multiplayer map preview if there is one.
			 */
			if (MultiplayerMapPreview != NULL) MultiplayerMapPreview->Blit_Preview(win);
			ValidateRect(win, NULL);
			return(false);

		case WM_DRAWITEM:
			OwnerDraw::Draw_Item((LPDRAWITEMSTRUCT)lParam);
			return(true);

		default:
			return(false);
	}
	return(false);
}


/// <summary>
/// Shows (or updates) the modal "please wait" popup used while a network event is pending.
/// If the wait window is already open, ORs the new event bit into the pending set; otherwise
/// creates and shows the window with an optional status text. Optionally blocks the caller
/// until the wait is resolved.
/// </summary>
/// <param name="event">Event bit(s) to add to the pending wait mask.</param>
/// <param name="block">If true, blocks (pumping callbacks) until the wait is resolved.</param>
/// <param name="text">Optional initial status text to show in the wait window.</param>
void Show_Wait_Window(unsigned int event, bool block, const char * text)
{
	HWND dlg = WS_Find_Dialog(IDD_WOL_WAITING);
	if (dlg) {
		g_ActiveWaitFlags |= event;
	} else if (event) {
		dlg = WS_Create_Dialog(ProgramInstance, IDD_WOL_WAITING, MainWindow, (DLGPROC)WOL_Waiting_Dialog_Proc, FALSE);
		Center_Window_Within_Window(dlg);
		OwnerDraw::Subclass_Dialog(dlg, 0);
		SendMessage(dlg, OD_SETTOP, 0, 1);
		if (text != NULL) {
			SendDlgItemMessage(dlg, IDC_TEXT, WM_SETTEXT, 0, (LPARAM)text);
		}
		ShowWindow(dlg, SW_NORMAL);
		g_ActiveWaitFlags = event;
	}
	if (block && g_ActiveWaitFlags) {
		WS_Wait_Dialog(dlg, WOL_Wait_Callback);
	} else if (g_ActiveWaitFlags) {
		SendMessage(dlg, OD_SETTOP, 0, 1);
	}
}

/// <summary>
/// Dialog procedure for the "please wait" popup. The only user action is the Disconnect
/// button, which signals the abort event and closes the dialog; otherwise just handles
/// standard owner-draw painting.
/// </summary>
/// <param name="win">Handle of the dialog window.</param>
/// <param name="uMsg">Window message being processed.</param>
/// <param name="wParam">Message-specific parameter.</param>
/// <param name="lParam">Message-specific parameter.</param>
/// <returns>TRUE if the message was handled, FALSE otherwise.</returns>
BOOL CALLBACK WOL_Waiting_Dialog_Proc(HWND win, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_DISCON:
					SetEvent(g_WaitEventHandles[EV_ABORT]);
					WS_Destroy_Dialog(win, IDOK);
					break;

				default:
					return(false);
			}
			break;

		case WM_INITDIALOG:
			return(false);

		case WM_DRAWITEM:
			OwnerDraw::Draw_Item((LPDRAWITEMSTRUCT)lParam);
			return(true);

		case WM_ERASEBKGND:
			return(true);

		case WM_PAINT:
			OwnerDraw::Draw_Dialog_Back(win);
			ValidateRect(win, NULL);
			return(false);

		default:
			return(false);
	}

	return(false);
}

/// <summary>
/// Updates the status text shown in the "please wait" popup, if it is currently open.
/// </summary>
/// <param name="status_text">New status text to display.</param>
void Set_Wait_Dialog_Text(char * status_text)
{
	HWND dlg = WS_Find_Dialog(IDD_WOL_WAITING);
	if (dlg) {
		SendDlgItemMessage(dlg, IDC_TEXT, WM_SETTEXT, 0, (LPARAM)status_text);
	}
}

/// <summary>
/// Clears the given event bit(s) from the pending wait mask, and closes the "please wait"
/// popup once no events remain pending.
/// </summary>
/// <param name="event">Event bit(s) to clear from the pending wait mask.</param>
void Close_Wait_Window(unsigned int event)
{
	HWND win = WS_Find_Dialog(IDD_WOL_WAITING);
	if (win) {
		g_ActiveWaitFlags &= ~event;
		if (g_ActiveWaitFlags == 0) {
			WS_Destroy_Dialog(win, 0);
		}
	}
}

/// <summary>
/// Converts the chat channel's users into session players.
/// This routine is used just before a network game starts. Every user in the lobby becomes
/// a session player carrying the house, color and squad they picked there, the channel
/// owner becomes the host, and each remote address is handed to the packet transport so
/// the game can reach them once the chat session is gone.
/// </summary>
/// <param name="users">Linked list of users to convert, or NULL to use the global
/// user list.</param>
void Fill_Session_Players(struct User * users)
{
	User * temp = NULL;
	User user;

	Session.NumPlayers = 0;
	Clear_Vector(&Session.Players);
	memset(WestwoodOnline_PingTimes, 0, sizeof(WestwoodOnline_PingTimes));

	NodeNameType * who;

	/*
	 * Build the local player's own node entry first, using our house/color/squad info.
	 */
	who = new NodeNameType;
	strcpy(who->Name, (char *)g_NickName);

	int index = Player_Name_To_Index((char *)g_NickName);
	if (index >= 0) {
		who->Player.House = g_UserInfo[index].house;
		who->Player.Color = g_UserInfo[index].color;
	} else {
		delete who;
		return;
	}

	g_UserList.getPointer(&temp, index);
	who->Player.SquadID = temp->squadID;

	// Port zero leaves the transport to send to the port it was configured with.
	who->Address.Set_Address((unsigned int)temp->ipaddr, 0);
	Session.Players.Add(who);
	Session.NumPlayers++;

	/*
	 * Walk the supplied linked list of users, or if none was given, the global channel
	 * user list, adding each remaining player as a node entry.
	 */
	int uindex = 0;
	User * next = users;

	while (true) {
		if (next == NULL && users != NULL) {
			break;
		}
		if (users == NULL) {
			if (!g_UserList.get(user, uindex)) {
				break;
			}
			uindex++;
		} else {
			user = *next;
			next = next->next;
		}

		index = Player_Name_To_Index((char *)user.name);

		if (index != -1) {
			g_UserList.get(user, index);

			/// Skip ourselves; we already added our own entry above.
			if (stricmp((char *)user.name, (char *)g_NickName) == 0) {
				continue;
			}

			who = new NodeNameType;
			strcpy(who->Name, (char *)user.name);
			who->Player.SquadID = user.squadID;
			who->Player.House = g_UserInfo[index].house;
			who->Player.Color = g_UserInfo[index].color;

			who->Address.Set_Address((unsigned int)user.ipaddr, 0);

			/// The channel owner's address becomes the session's host address.
			if (user.flags & CHAT_USER_CHANNELOWNER) {
				Session.HostAddress = who->Address;
			}

			Session.Players.Add(who);
			int avg = 0;
			g_pNetUtil->GetAvgPing(user.ipaddr, &avg);
			if (avg < 0) {
				avg = 0;
			}
			WestwoodOnline_PingTimes[Session.NumPlayers] = avg;
			Session.NumPlayers++;
		}
	}

	/// Register every remote player's address as a broadcast address for the transport.
	PacketTransport->Clear_Broadcast_Addresses();
	for (int p = 1; p < Session.Players.Count(); p++) {
		PacketTransport->Set_Broadcast_Address(Session.Players[p]->Address);
	}
}

/// <summary>
/// Checks whether the given user name is the current channel's host (owner).
/// </summary>
/// <param name="name">Name to test, or NULL to test the local login name.</param>
/// <returns>True if the named user is the channel host, false otherwise.</returns>
bool Is_Channel_Owner(char * name)
{
	if (name == NULL) name = g_NickName;

	if (!stricmp((char *)Get_Channel_Host().name, name)) {
		return(true);
	}
	return(false);
}

/// <summary>
/// Scans the current channel's user list for the user flagged as channel owner.
/// </summary>
/// <returns>Reference to a static User holding the channel owner's data (or, if no owner
/// flag was found, whichever entry the scan last examined).</returns>
User & Get_Channel_Host(void)
{
	static User user;
	for (int i = 0; i < g_UserList.length(); i++) {
		g_UserList.get(user, i);
		if (user.flags & CHAT_USER_CHANNELOWNER) {
			break;
		}
	}
	return(user);
}

/// <summary>
/// Negotiates the scenario (map) file transfer between the host and guests before a game
/// starts. As host, tells every player to go, waits for their go-responses, and sends the
/// scenario file to whoever requested it. As a guest, checks whether the scenario is already
/// present locally, tells the host we are ready to go if so, or requests the file from the
/// host (unless it's an official scenario we can identify locally without it).
/// </summary>
void Sync_Scenario_With_Guests(void)
{
	GlobalPacketType packet;

	Ipx.Set_Timing (25*2, (unsigned int) -1, 1000*5);

	/*
	 * Host: drive the go/ready handshake with every player and ship the scenario file
	 * to whoever needs it.
	 */
	if (Is_Channel_Owner(NULL)) {

		/*
		**	Send all players the NET_GO packet.  Wait until all ACK's have been
		**	received.
		*/
		memset(&packet, 0, sizeof(packet));
		packet.Command = NET_GO;
		for (int i = 1; i < Session.Players.Count(); i++) {
			do {
				Call_Back();
			} while (!Ipx.Send_Global_Message(&packet, sizeof(packet), 1, &(Session.Players[i]->Address)));
		}

		/*
		**	Wait for the go responses from each player in case someone needs the scenario
		**	file to be sent.
		*/
		int responses[MAX_PLAYERS];
		memset (responses, 0, sizeof (responses));
		int num_responses = 0;
		bool send_scenario = false;
		DebugString("About to wait for 'GO' response.\n");

		CDTimerClass <SystemTimerClass> response_timer;		// timeout timer for waiting for responses
		//response_timer = 60 * 10;		// Wait for 10 seconds. If we dont hear by then assume someone crashed
		response_timer = TIMER_SECOND * 30;		// Wait for 10 seconds. If we dont hear by then assume someone crashed

		do {
			Call_Back();
			int retcode = Ipx.Get_Global_Message (&Session.GPacket, &Session.GPacketlen, &Session.GAddress, &Session.GProductID);

			if (retcode && Session.GProductID == IPXGlobalConnClass::COMMAND_AND_CONQUER2) {
				DebugString("Packet is C&C2 packet\n");

				for (int i = 1; i < Session.Players.Count (); i++) {
					if (Session.Players[i]->Address == Session.GAddress) {
						if (!responses[i]) {
							if (Session.GPacket.Command == NET_REQ_SCENARIO) {
								DebugString("Packet type is NET_REQ_SCENARIO\n");
								responses[i] = Session.GPacket.Command;
								send_scenario = true;
								num_responses++;
							}
							if (Session.GPacket.Command == NET_READY_TO_GO) {
								DebugString("Packet type is NET_READY_TO_GO\n");
								responses[i] = Session.GPacket.Command;
								num_responses++;
							}
						}
					}
				}
			}
		} while (num_responses < Session.Players.Count () - 1 && response_timer);

		/*
		**	If one of the machines requested that the scenario be sent then send it.
		*/
		if (send_scenario) {
			memset (Session.ScenarioRequests, 0, sizeof (Session.ScenarioRequests));
			Session.RequestCount = 0;
			for (int i = 1; i < Session.Players.Count (); i++) {
				if (responses[i] == NET_REQ_SCENARIO) {
					Session.ScenarioRequests[Session.RequestCount++] = (char)i;
				}
			}
			Send_Remote_File (Scen->ScenarioName, false, true);
		}
	/*
	 * Guest: make sure we have the scenario the host is about to start, requesting it
	 * from the host if we don't.
	 */
	} else {

		/*
		**	If the scenario that the host wants to play doesnt exist locally then we
		**	need to request that it is sent. If we can identify the scenario locally then
		**	we need to fix up the file name so we load the right one.
		*/
		if (Find_Local_Scenario (Session.ScenarioFileName,
			Session.ScenarioFileLength,
			Session.ScenarioDigest,
			Session.ScenarioIsOfficial) == true) {

			/*
			**	We have the scenario. Tell the host that I am ready to go.
			*/
			memset ((void *)&packet, 0, sizeof (packet));
			packet.Command = NET_READY_TO_GO;
			Ipx.Send_Global_Message (&packet, sizeof (packet), 1, &Session.HostAddress);

			DebugString("Sent NET_READY_TO_GO\n");

			CDTimerClass<SystemTimerClass> response_timer;
			response_timer = TIMER_SECOND * 30;

			while (Ipx.Global_Num_Send() > 0 && response_timer) {
				Call_Back();
			}

		} else {
			/*
			**	Oh dear. Thats a scenario I don't have. Request that the host sends the
			**	scenario to me provided it's not an official scenario.
			**
			**	If the file is received OK then we will get a true return value and the
			**	actual file name to load will be in Session.ScenarioFileName
			*/
			if (Session.ScenarioIsOfficial && stricmp(Session.ScenarioFileName, RANDOM_MAP_FILE_NAME)) {

				Get_File_From_Host(Session.ScenarioFileName, true);
			} else {
				if (!Get_File_From_Host(Session.ScenarioFileName, true)) {
					//break;
				} else {
					/*
					**	Make sure we dont time-out because of the download
					*/
				}
			}
		}
		strcpy (Scen->ScenarioName, Session.ScenarioFileName);
	}

	/*
	**	Init network timing values, using previous response times as a measure
	**	of what our retry delta & timeout should be.
	*/
	Ipx.Set_Timing (Ipx.Global_Response_Time () + 2, (unsigned int) -1, std::max<unsigned>(2 * TIMER_SECOND, Ipx.Global_Response_Time () * 8));
}

/// <summary>
/// Checks for and starts a random-map preview download sent by the host. Only applies to a
/// guest sitting in a live game channel before the game has started; sets up the session's
/// player/host address from the channel's first user, then checks for a pending
/// NET_PREVIEW_MODE packet from the host and, if found, receives the preview data.
/// </summary>
/// <returns>True if a preview was received from the host, false otherwise.</returns>
bool Handle_Preview_Download(void)
{
	GlobalPacketType receive_packet;
	int packet_len;
	unsigned short product_id;
	IPXAddressClass sender_address;

	/// Only relevant for a guest with at least one other user in a live game channel.
	if (g_UserList.length() < 2) return(false);

	if (Is_Channel_Owner(g_NickName)) return(false);

	if (g_GameStarted) return(false);

	if (g_CurrentChannel.type == CHANNELTYPE_CHAT) return(false);

	User user;
	g_UserList.get(user, 0);
	user.next = NULL;
	Fill_Session_Players(&user);
	Call_Back();

	/*
	 * Check for a preview packet from the host and receive it if one is waiting.
	 */
	packet_len = sizeof(receive_packet);
	if (Ipx.Get_Global_Message(&receive_packet, &packet_len, &sender_address, &product_id)) {

		if (receive_packet.Command == NET_PREVIEW_MODE && sender_address == Session.HostAddress && product_id == IPXGlobalConnClass::COMMAND_AND_CONQUER2) {

			DebugString("Received NET_PREVIEW_MODE packet from host\n");

			Ipx.Set_Timing(50, (unsigned int)-1, 5000);
			g_RecievingPreview = true;
			Receive_Random_Map_Preview();
			g_RecievingPreview = false;
			Ipx.Set_Timing(TIMER_SECOND / 2, (unsigned int)-1, 10 * TIMER_SECOND);
			return(true);
		}
	}
	return(false);
}

/// <summary>
/// Nudges the host with a NET_PING packet while waiting in a live game channel as a guest.
/// Sets up the session's player/host address from the channel's first user, then sends a
/// single ping packet to the host address.
/// </summary>
void Poke_The_Host(void)
{
	IPXAddressClass address;

	/// Only relevant for a guest with at least one other user in a live game channel.
	if (g_UserList.length() >= 2) {
		if (!Is_Channel_Owner(g_NickName) && !g_GameStarted && g_CurrentChannel.type != CHANNELTYPE_CHAT) {
			User user;
			g_UserList.get(user, 0);
			user.next = NULL;
			Fill_Session_Players(&user);
			Call_Back();
			Ipx.Set_Timing(50, (unsigned int)-1, 5000);

			DebugString("Poke_The_Host - Host address is %s\n", Session.HostAddress.As_String());

			GlobalPacketType packet;
			memset(&packet, 0, sizeof(packet));
			packet.Command = NET_PING;
			Ipx.Send_Global_Message(&packet, sizeof(packet), 1, &(Session.HostAddress));
			Call_Back();
		}
	}
}

/*
 * WDT
 */
BOOL CALLBACK Select_WDT_Server_Proc(HWND win, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK Select_WDT_Location_Proc(HWND win, UINT uMsg, WPARAM wParam, LPARAM lParam);

/// <summary>
/// Initializes World Domination Tour lobby state. Builds the list of selectable WDT lobby
/// names (one per contested/unowned territory, skipping territories already owned by a
/// side) by reading territory names out of WDTMAP.INI, up to the current channel count (or
/// a default of 6 if there are no channels yet).
/// </summary>
void Init_WDT(void)
{
	memset(gWDTLobbies, 0, sizeof(gWDTLobbies));

	WDTState * state = WDT_Get_New_State();

	/// Number of lobbies to build: one per existing game channel, or a default of 6.
	int count = g_UserChannels.length();
	if (count == 0) {
		count = 6;
	}

	int slot = 0;
	{
		unsigned char * owner_history;
		for (unsigned int n = 0; n < state->NumTerritories; n++) {
			owner_history = state->OwnerHistory[state->NumTicks - 1];
		}
	}
	int territory = 0;
	unsigned char * owners;
	char section[32];
	char defaultname[32];
	char region[32];

	/// Load the WDT territory-name lookup data used to build the lobby list below.
	MixFileClass * mix = new MixFileClass("WDT.MIX", &FastKey);

	INIClass ini;
	CCFileClass file("WDTMAP.INI");
	ini.Load(file);

	memset(WDT_TERRITORY_LOBBY_INDEXES, 0xFF, sizeof(WDT_TERRITORY_LOBBY_INDEXES));

	if (count > 0) {
		for (slot; slot < count && slot < 6; ) {
			char * buffer = gWDTLobbies[slot];
			owners = state->OwnerHistory[state->NumTicks - 1];

			/*
			 * Skip past any territories that are already owned by a side. Only
			 * contested (un-owned) territories become selectable lobbies.
			 */
			if (owners[territory]) {
				while (true) {
					if (territory >= (int)state->NumTerritories) {
						break;
					}
					territory++;
					if (!owners[territory]) {
						break;
					}
				}
			}

			if (territory >= (int)state->NumTerritories) {
				break;
			}

			/*
			 * Look up this territory's display name from WDTMAP.INI.
			 */
			char * name = (char *)operator new(32);

			sprintf(section, "Territory%02d", territory);

			strcpy(defaultname, section);

			if (!state->MapID) {
				ini.Get_String("NorthAmerica", section, defaultname, region, 32);
			} else {
				ini.Get_String("Europe", section, defaultname, region, 32);
			}
			ini.Get_String(region, "Name", "A territory", name, 32);

			sprintf(buffer, "%s", name);
			g_Lobbies[slot] = buffer;
			WDT_TERRITORY_LOBBY_INDEXES[slot] = territory;
			slot++;

			operator delete(name);
			territory++;
		}
	}

	delete mix;
}

/// <summary>
/// Shows the World Domination Tour "available servers" dialog and waits for it to close.
/// </summary>
/// <returns>Result code returned by the dialog (see WS_Wait_Dialog/WS_Destroy_Dialog).</returns>
int Select_WDT_Server(void)
{
	Hide_Mouse();
	Draw_Menu_Background();
	Show_Mouse();
	HWND dlg = WS_Create_Dialog(ProgramInstance, IDD_WOL_AVAILABLE_SERVERS, MainWindow, (DLGPROC)Select_WDT_Server_Proc, FALSE);
	Center_Window_Within_Window(dlg);
	OwnerDraw::Subclass_Dialog(dlg, 0);
	ShowWindow(dlg, SW_NORMAL);
	return(WS_Wait_Dialog(dlg, WOL_Wait_Callback));
}

/// <summary>
/// Dialog procedure for the World Domination Tour "available servers" list. Populates the
/// list box on OD_SUBCLASSED (stripping any "region:" prefix from each server name and
/// pre-selecting the player's preferred server), and on OK stores the chosen server as the
/// new preferred server and saves the multiplayer settings.
/// </summary>
/// <param name="win">Handle of the dialog window.</param>
/// <param name="uMsg">Window message being processed.</param>
/// <param name="wParam">Message-specific parameter.</param>
/// <param name="lParam">Message-specific parameter.</param>
/// <returns>TRUE if the message was handled, FALSE otherwise.</returns>
BOOL CALLBACK Select_WDT_Server_Proc(HWND win, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static int firstserver = 0;
	Server * server = NULL;

	switch (uMsg) {
		case WM_NCDESTROY:
			On_WM_NCDESTROY(win);
			return(false);

		case WM_PAINT:
			OwnerDraw::Draw_Dialog_Back(win);
			ValidateRect(win, NULL);
			return(false);

		case WM_ERASEBKGND:
			return(true);

		case WM_DRAWITEM:
			OwnerDraw::Draw_Item((LPDRAWITEMSTRUCT)lParam);
			return(true);

		case OD_SUBCLASSED: {
			int newsel = g_CurrentServerIndex;

			SendDlgItemMessage(win, IDC_SERVERLIST, LB_RESETCONTENT, 0, 0);

			/// Skip the first (default) entry when there's more than one real server.
			firstserver = g_Servers.length() != 1;

			/// Populate the list box, stripping any "region:" prefix from each name.
			for (int i = firstserver; i < g_Servers.length(); i++) {
				char text[200];
				g_Servers.getPointer(&server, i);
				DebugString("Server found %s\n", (char *)server->name);

				char * sep = strchr((char *)server->name, ':');
				if (sep == NULL) {
					sep = (char *)server->name;
				} else {
					sep = sep + 1;
				}
				strcpy(text, sep);

				SendDlgItemMessage(win, IDC_SERVERLIST, LB_INSERTSTRING, -1, (LPARAM)text);

				if (Session.PreferredServer) {
					if (strcmp(text, Session.PreferredServer) == 0) {
						newsel = i;
					}
				}
			}

			if (g_Servers.length() && g_CurrentServerIndex >= 0 && g_CurrentServerIndex < g_Servers.length()) {
				SendDlgItemMessage(win, IDC_SERVERLIST, LB_SETCURSEL, newsel - firstserver, 0);
				SendDlgItemMessage(win, IDC_SERVERLIST, LB_SETTOPINDEX, newsel - firstserver, 0);
			}
		} break;

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				/// Store the chosen server as the new preferred server and start using it.
				case IDOK: {
					int index = SendDlgItemMessage(win, IDC_SERVERLIST, LB_GETCURSEL, 0, 0);
					if (index >= 0 && index < g_Servers.length() - firstserver) {
						int textlen = SendDlgItemMessage(win, IDC_SERVERLIST, LB_GETTEXTLEN, index, 0);
						if (Session.PreferredServer) {
							delete Session.PreferredServer;
						}
						Session.PreferredServer = new char[textlen + 2];
						SendDlgItemMessage(win, IDC_SERVERLIST, LB_GETTEXT, index, (LPARAM)Session.PreferredServer);
						g_CurrentServerIndex = index + firstserver;
						g_TargetServerIndex = index + firstserver;
						Session.Write_MultiPlayer_Settings();
						WS_Destroy_Dialog(win, IDOK);
					}
				} break;

				case IDCANCEL:
					WS_Destroy_Dialog(win, IDCANCEL);
					return(false);

				default:
					return(false);
			}
			break;

		default:
			return(false);
	}

	return(false);
}

/// <summary>
/// Shows the World Domination Tour "select country/locale" dialog and waits for it to close,
/// then restores the given window.
/// </summary>
/// <param name="window">Window to restore (show normally) once the dialog closes.</param>
/// <returns>Result code returned by the dialog (see WS_Wait_Dialog/WS_Destroy_Dialog).</returns>
int Select_WDT_Location(HWND window)
{
	HWND dlg = WS_Create_Dialog(ProgramInstance, IDD_WOL_SELECT_LOCATION, MainWindow, (DLGPROC)Select_WDT_Location_Proc, FALSE);
	Center_Window_Within_Window(dlg);
	OwnerDraw::Subclass_Dialog(dlg, 0);
	ShowWindow(dlg, SW_NORMAL);
	int res = WS_Wait_Dialog(dlg, WOL_Wait_Callback);
	ShowWindow(window, SW_NORMAL);
	return(res);
}

/// <summary>
/// Dialog procedure for the WorldDomination Tour "select country/locale" dialog.
/// Populates the locale list from the chat server on OD_SUBCLASSED, and on OK
/// looks up the chosen country and stores it as the player's locale.
/// </summary>
/// <param name="win">Dialog window handle.</param>
/// <param name="uMsg">Window message.</param>
/// <param name="wParam">Message-specific WPARAM (control/notification id for WM_COMMAND).</param>
/// <param name="lParam">Message-specific LPARAM.</param>
/// <returns>TRUE if the message was handled, FALSE otherwise.</returns>
BOOL CALLBACK Select_WDT_Location_Proc(HWND win, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int i = 0;
	static int localecount = 0;
	LPCSTR loc = NULL;

	switch (uMsg) {
		case WM_NCDESTROY:
			On_WM_NCDESTROY(win);
			return(false);

		case WM_PAINT:
			OwnerDraw::Draw_Dialog_Back(win);
			ValidateRect(win, NULL);
			return(false);

		case WM_ERASEBKGND:
			return(true);

		case WM_DRAWITEM:
			OwnerDraw::Draw_Item((LPDRAWITEMSTRUCT)lParam);
			return(true);

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				/// Find the chosen locale by matching its display string, and save it.
				case IDOK: {
					char buffer[256];

					int index = SendDlgItemMessage(win, IDC_LOCATION_LIST, LB_GETCURSEL, 0, 0);
					if (index >= 0 && index < localecount && SendDlgItemMessage(win, IDC_LOCATION_LIST, LB_GETTEXT, index, (LPARAM)buffer) > 0) {
						while (i < localecount) {
							if (g_pChat->GetLocaleString(&loc, (Locale)i) == S_OK && strcmp(loc, buffer) == 0) {
								index = i;
								break;
							}
							i++;
						}

						Session.Locale = index;
						if (index != 0) {
							Session.Write_MultiPlayer_Settings();
						}
					}
					WS_Destroy_Dialog(win, IDOK);
				} break;

				case IDCANCEL:
					WS_Destroy_Dialog(win, IDCANCEL);
					return(false);

				default:
					return(false);
			}
			break;

		/// Populate the locale list box, pre-selecting the current locale.
		case OD_SUBCLASSED: {
			char buffer[256];

			SendDlgItemMessage(win, IDC_LOCATION_LIST, LB_RESETCONTENT, 0, 0);

			g_pChat->GetLocaleCount(&localecount);

			if (localecount > 0) {
				for (int i = localecount - 1; i >= 0; i--) {
					if (g_pChat->GetLocaleString(&loc, (Locale)i)) {
						break;
					}
					DebugString("Adding country: %s\n", loc);
					if (i == Session.Locale) {
						strcpy(buffer, loc);
					}
					if (i < 2) {
						SendDlgItemMessage(win, IDC_LOCATION_LIST, LB_INSERTSTRING, 0, (LPARAM)loc);
					} else {
						SendDlgItemMessage(win, IDC_LOCATION_LIST, LB_ADDSTRING, 0, (LPARAM)loc);
					}
				}
			}

			int sel = SendDlgItemMessage(win, IDC_LOCATION_LIST, LB_SELECTSTRING, (WPARAM)-1, (LPARAM)buffer);
			SendDlgItemMessage(win, IDC_LOCATION_LIST, LB_SETCURSEL, sel, 0);
			SendDlgItemMessage(win, IDC_LOCATION_LIST, LB_SETTOPINDEX, sel, 0);
		} break;

		default:
			return(false);
	}

	return(false);
}

/// <summary>
/// Requests the World Domination Tour server list from the chat server (choosing the SKU
/// based on whether Firestorm is installed/enabled and whether this is a WDT session), then
/// waits for the server list to arrive, aborting with an error message if the request times
/// out after 30 seconds.
/// </summary>
/// <returns>0 on success, 1 if aborted, 2 if the game was quit while waiting.</returns>
int Request_WDT_Server_List(void)
{
	DWORD wait_result;
	int retval;

	Show_Wait_Window(WOL_WAIT_LOGIN_DONE, false);
	Set_Wait_Dialog_Text((char *)Fetch_String(TXT_FETCHING_SERVLIST));

	g_Servers.setEmpty();
	g_CurrentServerIndex = 0;
	g_TargetServerIndex = 0;

	/// Pick the product SKU to request the server list for.
	if (Addon_Installed(ADDON_FIRESTORM) && Addon_Enabled(ADDON_FIRESTORM)) {
		if (Session.IsWDT) {
			g_pChat->RequestServerList(g_LanguageCode | WORLDDOM_SKU, WOL_GAME_VERSION, "TibSun", "TibPass99", 35);
		} else {
			g_pChat->RequestServerList(g_LanguageCode | FIRESTORM_SKU, WOL_GAME_VERSION, "TibSun", "TibPass99", 35);
		}
	} else {
		g_pChat->RequestServerList(g_LanguageCode | TIBERIAN_SUN_SKU, WOL_GAME_VERSION, "TibSun", "TibPass99", 35);
	}

	g_pChat->SetProductSKU(g_GameSKU);

	/*
	 * Wait for the server list to arrive, giving up after 30 seconds of no response.
	 */
	int timeout = 0;
	while (true) {
		wait_result = WaitForMultipleObjects(EV_COUNT, g_WaitEventHandles, FALSE, 50);
		while (wait_result == WAIT_TIMEOUT) {
			timeout += 50;
			if (timeout > 30000) {
				ODMessageBox(Fetch_String(TXT_CANT_CONNECT), MB_OK, WOL_Wait_Callback, 0);
				SetEvent(g_WaitEventHandles[EV_ABORT]);
				wait_result = EV_ABORT;
				break;
			}
			WOL_Wait_Callback();
			wait_result = WaitForMultipleObjects(EV_COUNT, g_WaitEventHandles, FALSE, 50);
		}

		ResetEvent(g_WaitEventHandles[wait_result]);

		if (wait_result == EV_ABORT) {
			retval = 1;
			break;
		}
		if (wait_result == EV_QUITGAME) {
			retval = 2;
			break;
		}
		if (wait_result == EV_GOT_SERVERS) {
			retval = 0;
			break;
		}
	}

	Close_Wait_Window(WOL_WAIT_ALL);

	return(retval);
}

/// <summary>
/// Requests the current World Domination Tour cycle state from the WDT server and waits
/// for it to arrive. If the server reports the requested state is stale (the "previous
/// cycle" event), re-requests the last-completed cycle's state instead and waits again.
/// </summary>
/// <returns>True if the cycle state was obtained, false on error/abort/timeout-of-retry.</returns>
bool Request_WDT_Cycle(void)
{
	if (g_WDTServer.conndata == NULL) {
		ODMessageBox(Fetch_String(TXT_WDT_NET_ERR), 0, WOL_Wait_Callback);
	} else {
		int wait_result;
		for (wait_result = 0; wait_result < EV_COUNT; wait_result++) {
			ResetEvent(g_WaitEventHandles[wait_result]);
		}

		/*
		 * Request the full state of the current cycle.
		 */
		HRESULT res = g_pNetUtil->RequestWDTState(g_WDTHost, g_WDTPort, WDT_REQ_STATE_EVERYTHING);
		DebugString("WDTHost is %s, port is %d\n", g_WDTHost, g_WDTPort);
		DebugString("Requesting current WDT cycle\n");

		if (res == S_OK) {
			Show_Wait_Window(WOL_WAIT_LOGIN_DONE, false);
			Set_Wait_Dialog_Text((char *)Fetch_String(TXT_FETCHING_WDT));
			while (true) {
				wait_result = WaitForMultipleObjects(EV_COUNT, g_WaitEventHandles, FALSE, 100);
				while (wait_result == WAIT_TIMEOUT) {
					WOL_Wait_Callback();
					wait_result = WaitForMultipleObjects(EV_COUNT, g_WaitEventHandles, FALSE, 100);
				}

				ResetEvent(g_WaitEventHandles[wait_result]);

				if (wait_result == EV_ABORT) {
					Close_Wait_Window(WOL_WAIT_ALL);
					return(0);
				}
				if (wait_result == EV_ERROR) {
					ODMessageBox(Fetch_String(TXT_WDT_NET_ERR), 0, WOL_Wait_Callback);
					Close_Wait_Window(WOL_WAIT_ALL);
					return(0);
				}
				/*
				 * The server reports this state is stale; re-request the last-completed
				 * cycle's state instead and wait for it.
				 */
				if (wait_result == EV_11)
				{
					res = g_pNetUtil->RequestWDTState(g_WDTHost, g_WDTPort, WDT_REQ_STATE_LAST_CYCLE);
					DebugString("Requesting Previous Cycle\n");

					if (res == S_OK) {
						while (true) {
							wait_result = WaitForMultipleObjects(EV_COUNT, g_WaitEventHandles, FALSE, 100);
							while (wait_result == WAIT_TIMEOUT) {
								WOL_Wait_Callback();
								wait_result = WaitForMultipleObjects(EV_COUNT, g_WaitEventHandles, FALSE, 100);
							}
							ResetEvent(g_WaitEventHandles[wait_result]);
							if (wait_result == EV_ABORT) {
								Close_Wait_Window(WOL_WAIT_ALL);
								return(0);
							}
							if (wait_result == EV_ERROR) {
								ODMessageBox(Fetch_String(TXT_WDT_NET_ERR), 0, WOL_Wait_Callback);
								Close_Wait_Window(WOL_WAIT_ALL);
								return(0);
							}
							if (wait_result == EV_11) {
								Close_Wait_Window(WOL_WAIT_ALL);
								return(1);
							}
						}
					} else {
						Close_Wait_Window(WOL_WAIT_ALL);
						return(0);
					}
				}
			}
		} else {
			ODMessageBox(Fetch_String(TXT_WDT_NET_ERR), 0, WOL_Wait_Callback);
		}
	}
	return(0);
}

/// <summary>
/// Loads (once) the World Domination Tour win/lose emphasis and territory sound name lists
/// into their per-category ArrayLists. Does nothing if already initialized.
/// </summary>
void Init_WDT_Sounds(void)
{
	if (!g_WDTSoundsInited) {
		static Wstring _sound_names[16];

		/// Sound file names for each category, grouped by list below.
		_sound_names[0].set("01-W024.V01");
		_sound_names[1].set("01-W026.V01");
		_sound_names[2].set("01-W028.V01");

		_sound_names[3].set("01-W018.V01");
		_sound_names[4].set("01-W020.V01");
		_sound_names[5].set("01-W022.V01");

		_sound_names[6].set("00-W022.V00");
		_sound_names[7].set("00-W024.V00");
		_sound_names[8].set("00-W026.V00");

		_sound_names[9].set("00-W017.V00");
		_sound_names[10].set("00-W018.V00");
		_sound_names[11].set("00-W020.V00");

		_sound_names[12].set("01-W032.V01");

		_sound_names[13].set("01-W030.V01");

		_sound_names[14].set("00-W031.V00");

		_sound_names[15].set("00-W029.V00");

		g_WDT_NodWinEmphasisSounds.clear();
		g_WDT_GDIWinEmphasisSounds.clear();
		g_WDT_NodLoseEmphasisSounds.clear();
		g_WDT_GDILoseEmphasisSounds.clear();
		g_WDT_NodTerritoryWinSounds.clear();
		g_WDT_GDITerritoryWinSounds.clear();
		g_WDT_NodTerritoryLoseSounds.clear();
		g_WDT_GDITerritoryLoseSounds.clear();

		/// Sort the sound names into their respective category lists.
		g_WDT_NodWinEmphasisSounds.addHead(_sound_names[0]);
		g_WDT_NodWinEmphasisSounds.addHead(_sound_names[1]);
		g_WDT_NodWinEmphasisSounds.addHead(_sound_names[2]);

		g_WDT_NodLoseEmphasisSounds.addHead(_sound_names[3]);
		g_WDT_NodLoseEmphasisSounds.addHead(_sound_names[4]);
		g_WDT_NodLoseEmphasisSounds.addHead(_sound_names[5]);

		g_WDT_GDIWinEmphasisSounds.addHead(_sound_names[6]);
		g_WDT_GDIWinEmphasisSounds.addHead(_sound_names[7]);
		g_WDT_GDIWinEmphasisSounds.addHead(_sound_names[8]);

		g_WDT_GDILoseEmphasisSounds.addHead(_sound_names[9]);
		g_WDT_GDILoseEmphasisSounds.addHead(_sound_names[10]);
		g_WDT_GDILoseEmphasisSounds.addHead(_sound_names[11]);

		g_WDT_NodTerritoryWinSounds.addHead(_sound_names[12]);

		g_WDT_NodTerritoryLoseSounds.addHead(_sound_names[13]);

		g_WDT_GDITerritoryWinSounds.addHead(_sound_names[14]);

		g_WDT_GDITerritoryLoseSounds.addHead(_sound_names[15]);

		g_WDTSoundsInited = true;
	}
}

/// <summary>
/// Clears all World Domination Tour sound category lists and marks them as uninitialized.
/// </summary>
void Deinit_WDT_Sounds(void)
{
	g_WDT_NodWinEmphasisSounds.clear();
	g_WDT_GDIWinEmphasisSounds.clear();
	g_WDT_NodLoseEmphasisSounds.clear();
	g_WDT_GDILoseEmphasisSounds.clear();
	g_WDT_NodTerritoryWinSounds.clear();
	g_WDT_GDITerritoryWinSounds.clear();
	g_WDT_NodTerritoryLoseSounds.clear();
	g_WDT_GDITerritoryLoseSounds.clear();
	g_WDTSoundsInited = false;
}

/// <summary>
/// Plays a random sound from the given World Domination Tour sound category list, unless a
/// WDT voice sample is already playing. Lazily initializes the sound category lists if
/// needed, then loads the WDT sound resources and starts playback, replacing any sample
/// that was previously loaded.
/// </summary>
/// <param name="list">Sound category list to pick a random sound from.</param>
void Play_WDT_Sound(ArrayList<Wstring> list)
{
	Wstring name;

	if (g_WDTVoices != NULL && g_WDTVoices->Playing()) {
		return;
	}

	if (!g_WDTSoundsInited) {
		Init_WDT_Sounds();
	}

	int count = list.length();
	if (count <= 0) {
		DebugString("Play_WDT_Sound: passed empty sound category\n");
		return;
	}

	/// Open the WDT sound resources and pick a random sound from the category.
	MixFileClass * wdtmix = new MixFileClass("WDT.MIX", &FastKey);
	MixFileClass * wdtvoxmix = new MixFileClass("WDTVOX.MIX", &FastKey);

	if (g_WDTVoices != NULL) {
		delete g_WDTVoices;
	}

	list.get(name, rand() % count);

	g_WDTVoices = new WorldDominationTour::Voices::Sample(name.get());

	g_WDTVoices->Start();

	delete wdtmix;
	delete wdtvoxmix;
}
