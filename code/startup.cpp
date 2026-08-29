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

/* $Header: /counterstrike/STARTUP.CPP 6     3/15/97 7:18p Steve_tall $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : STARTUP.CPP                                                  *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : October 3, 1994                                              *
 *                                                                                             *
 *                  Last Update : September 30, 1996 [JLB]                                     *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   Prog_End -- Cleans up library systems in prep for game exit.                              *
 *   main -- Initial startup routine (preps library systems).                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#define INCLUDE_COM
#include "always.h"

#include "_alpha.h"
#include "_command.h"
#include "_convert.h"
#include "_font.h"
#include "_keyboar.h"
#include "_mixfile.h"
#include "_rect.h"
#include "_rules.h"
#include "_surface.h"
#include "_tactica.h"
#include "_winfix.h"
#include "_zbuffer.h"
#include "aircraft.h"
#include "airctype.h"
#include "aitrig.h"
#include "alphashp.h"
#include "anim.h"
#include "animtype.h"
#include "blight.h"
#include "brain.h"
#include "building.h"
#include "builtype.h"
#include "bullet.h"
#include "bullettype.h"
#include "campaign.h"
#include "cell.h"
#include "classfactory.h"
#include "command.h"
#include "conquer.h"
#include "cstream.h"
#include "data.h"
#include "dbgprint.h"
#include "dllver.h"
#include "drive.h"
#include "droppod.h"
#include "dsaudio.h"
#include "dsurface.h"
#include "empulse.h"
#include "except.h"
#include "factory.h"
#include "fly.h"
#include "fog.h"
#include "goptions.h"
#include "house.h"
#include "houstype.h"
#include "hover.h"
#include "iblowfish.h"
#include "infantry.h"
#include "infatype.h"
#include "init.h"
#include "ionblast.h"
#include "ipxmgr.h"
#include "isotype.h"
#include "jumpjet.h"
#include "language\language.h"
#include "levitate.h"
#include "light.h"
#include "lightcon.h"
#include "mech.h"
#include "mixfile.h"
#include "misc.h"
#include "movie.h"
#include "msgloop.h"
#include "netdlg.h" // for Shutdown_Network.
#include "overlay.h"
#include "overtype.h"
#include "ovrlight.h"
#include "particle.h"
#include "partsys.h"
#include "psystype.h"
#include "ptype.h"
#include "rules.h"
#include "scenario.h"
#include "scheme.h"
#include "script.h"
#include "session.h"
#include "shapeset.h"
#include "side.h"
#include "sidebar.h"
#include "spawner.h"
#include "smudge.h"
#include "smudtype.h"
#include "sun.h"
#include "super.h"
#include "suprtype.h"
#include "surface.h"
#include "tactical.h"
#include "taction.h"
#include "tag.h"
#include "tagtype.h"
#include "taskforc.h"
#include "team.h"
#include "teamtype.h"
#include "teleport.h"
#include "terrain.h"
#include "terrtype.h"
#include "tevent.h"
#include "theme.h"
#include "tiberium.h"
#include "trigger.h"
#include "trigtype.h"
#include "trim.h"
#include "tube.h"
#include "tunnel.h"
#include "unit.h"
#include "unittype.h"
#include "vanim.h"
#include "vanimtype.h"
#include "vector.h"
#include "video.h"
#include "walk.h"
#include "warhead.h"
#include "wave.h"
#include "waypoint.h"
#include "weapon.h"
#include "win.h"
#include "winstub.h"
#include "wwfont.h"
#include "wwmouse.h"
#include "zbuffer.h"

#include <conio.h>
#include <io.h>
#include <cfloat>

extern	HINSTANCE LanguageResources;

#define APP_GUID "29e3bb2a-2f36-11d3-a72c-0090272fa661"
#define AUTOPLAY_GUID "b350c6d2-2f36-11d3-a72c-0090272fa661"


#ifndef NO_BLOWFISH_DLL
const struct RegStruct {
	const GUID *clsid;
	const char *name;
} RegisterTheseDLLs[] = {
	{ &CLSID_BlowfishObject, "blowfish.dll" }
};
#endif

HANDLE AppMutex;
HANDLE AutoPlayMutex;

DynamicVectorClass<DWORD> RegisteredClasses;

//WinTimerClass * WinTimer;

/// <summary>
/// Destroys the drawing surfaces and drops the video mode.
/// This routine is used on the way out of the game so that the display is handed back to
/// Windows in a sane state, rather than left sitting in the game's own video mode.
/// </summary>
/// <remarks>It is safe to call this routine more than once; only the first call does any
/// work, since several shutdown paths lead here.</remarks>
void Reset_Surfaces(void)
{
	static bool surfaces_reset = false;

	if (!surfaces_reset) {
		if (HiddenSurface) {
			delete HiddenSurface;
			HiddenSurface = NULL;
		}
		if (AlternateSurface) {
			delete AlternateSurface;
			AlternateSurface = NULL;
		}
		if (TileSurface) {
			delete TileSurface;
			TileSurface = NULL;
		}
		if (SidebarSurface) {
			delete SidebarSurface;
			SidebarSurface = NULL;
		}
		if (CompositeSurface) {
			delete CompositeSurface;
			CompositeSurface = NULL;
		}
		if (VisibleSurface) {
			delete VisibleSurface;
			VisibleSurface = NULL;
		}

		Video_Shutdown();

		surfaces_reset = true;
	}
}

/// <summary>
/// Registers the game's COM classes with OLE.
/// This routine is called during startup, before anything that lives in the object
/// database can be created. It first ensures the support DLLs are present, asking any
/// that OLE cannot yet instantiate to register themselves, and then publishes a class
/// factory for every persistent game class so that objects can be created by CLSID. The
/// player is told by way of a message box if a support DLL could not be prepared.
/// </summary>
/// <returns>bool; Did the preparation fail? Note the sense -- true means trouble.</returns>
static bool RegisterClasses(void)
{

	bool failed = false;
#ifndef NO_BLOWFISH_DLL
	for (int i = 0; i < ARRAY_SIZE(RegisterTheseDLLs); i++) {
		IUnknownPtr ptr;
		HRESULT result = ptr.CreateInstance(*RegisterTheseDLLs[i].clsid, NULL, CLSCTX_ALL);
		failed = FAILED(result);
		if (failed) {
			failed = false;
			HINSTANCE hModule = LoadLibrary(RegisterTheseDLLs[i].name);
			if (hModule != NULL) {
				FARPROC fprocDllReg = (FARPROC)GetProcAddress(hModule, "DllRegisterServer");
				if (!fprocDllReg || (fprocDllReg(), FAILED(ptr.CreateInstance(*RegisterTheseDLLs[i].clsid, NULL, CLSCTX_ALL)))) {
					failed = true;
				}
				FreeLibrary(hModule);
			} else {
				failed = true;
			}
		}
		if (failed) {
			break;
		}
		ptr.Release();
	}
#endif

	DWORD dwRegister;
	IClassFactory *t;

	/// Handy macros to easily register the class factories.

	/// Register a class-object with OLE.
	#define REGISTER_CLASS(_class, _clsid) \
		{ \
			t = new TClassFactory<_class>; \
			CoRegisterClassObject(_clsid, t, CLSCTX_INPROC_SERVER, REGCLS_MULTIPLEUSE, &dwRegister); \
			RegisteredClasses.Add(dwRegister); \
		} \

	REGISTER_CLASS(CStreamClass, CLSID_CompressStream);
	REGISTER_CLASS(WaveClass, CLSID_WaveClass);
	REGISTER_CLASS(TerrainTypeClass, CLSID_TerrainTypeClass);
	REGISTER_CLASS(TerrainClass, CLSID_TerrainClass);
	REGISTER_CLASS(SuperWeaponTypeClass, CLSID_SuperWeaponTypeClass);
	REGISTER_CLASS(SuperClass, CLSID_SuperWeaponClass);
	REGISTER_CLASS(Tactical, CLSID_TacticalMapClass);
	REGISTER_CLASS(CellClass, CLSID_CellClass);
	REGISTER_CLASS(EMPulseClass, CLSID_EMPulseClass);
	REGISTER_CLASS(LightSourceClass, CLSID_LightSource);
	REGISTER_CLASS(SideClass, CLSID_SideClass);
	REGISTER_CLASS(TiberiumClass, CLSID_TiberiumClass);
	REGISTER_CLASS(TubeClass, CLSID_TubeClass);
	REGISTER_CLASS(CampaignClass, CLSID_CampaignClass);
	REGISTER_CLASS(BuildingLightClass, CLSID_BuildingLightClass);
	REGISTER_CLASS(WaypointPathClass, CLSID_WaypointPath);
	REGISTER_CLASS(TEventClass, CLSID_EventClass);
	REGISTER_CLASS(VoxelAnimTypeClass, CLSID_VoxelAnimTypeClass);
	REGISTER_CLASS(VoxelAnimClass, CLSID_VoxelAnimClass);
	REGISTER_CLASS(TActionClass, CLSID_ActionClass);
	REGISTER_CLASS(TriggerClass, CLSID_TriggerClass);
	REGISTER_CLASS(TriggerTypeClass, CLSID_TriggerTypeClass);
	REGISTER_CLASS(ScriptClass, CLSID_ScriptClass);
	REGISTER_CLASS(ScriptTypeClass, CLSID_ScriptTypeClass);
	REGISTER_CLASS(TagClass, CLSID_TagClass);
	REGISTER_CLASS(TagTypeClass, CLSID_TagTypeClass);
	REGISTER_CLASS(TeamClass, CLSID_TeamClass);
	REGISTER_CLASS(TeamTypeClass, CLSID_TeamTypeClass);
	REGISTER_CLASS(TaskForceClass, CLSID_TaskForceClass);
	REGISTER_CLASS(UnitTypeClass, CLSID_UnitTypeClass);
	REGISTER_CLASS(BuildingTypeClass, CLSID_BuildingTypeClass);
	REGISTER_CLASS(AircraftTypeClass, CLSID_AircraftTypeClass);
	REGISTER_CLASS(InfantryTypeClass, CLSID_InfantryTypeClass);
	REGISTER_CLASS(BulletTypeClass, CLSID_BulletTypeClass);
	REGISTER_CLASS(IsometricTileTypeClass, CLSID_IsometricTileTypeClass);
	REGISTER_CLASS(OverlayTypeClass, CLSID_OverlayTypeClass);
	REGISTER_CLASS(SmudgeTypeClass, CLSID_SmudgeTypeClass);
	REGISTER_CLASS(UnitClass, CLSID_UnitClass);
	REGISTER_CLASS(BuildingClass, CLSID_BuildingClass);
	REGISTER_CLASS(AircraftClass, CLSID_AircraftClass);
	REGISTER_CLASS(InfantryClass, CLSID_InfantryClass);
	REGISTER_CLASS(AnimClass, CLSID_AnimClass);
	REGISTER_CLASS(AnimTypeClass, CLSID_AnimTypeClass);
	REGISTER_CLASS(HouseTypeClass, CLSID_HouseTypeClass);
	REGISTER_CLASS(HouseClass, CLSID_HouseClass);
	REGISTER_CLASS(DriveLocomotionClass, CLSID_DriveLocomotion);
	REGISTER_CLASS(JumpjetLocomotionClass, CLSID_JumpjetLocomotion);
	REGISTER_CLASS(HoverLocomotionClass, CLSID_HoverLocomotion);
	REGISTER_CLASS(TunnelLocomotionClass, CLSID_TunnelLocomotion);
	REGISTER_CLASS(WalkLocomotionClass, CLSID_WalkLocomotion);
	REGISTER_CLASS(DropPodLocomotionClass, CLSID_BallisticLocomotion);
	REGISTER_CLASS(FlyLocomotionClass, CLSID_FlyerLocomotion);
	REGISTER_CLASS(TeleportLocomotionClass, CLSID_TeleportLocomotion);
	REGISTER_CLASS(MechLocomotionClass, CLSID_MechLocomotion);
	REGISTER_CLASS(LevitateLocomotionClass, CLSID_LevitateLocomotion);
	REGISTER_CLASS(BulletClass, CLSID_BulletClass);
	REGISTER_CLASS(FactoryClass, CLSID_FactoryClass);
	REGISTER_CLASS(WarheadTypeClass, CLSID_WarheadTypeClass);
	REGISTER_CLASS(WeaponTypeClass, CLSID_WeaponTypeClass);
	REGISTER_CLASS(ParticleClass, CLSID_ParticleClass);
	REGISTER_CLASS(ParticleTypeClass, CLSID_ParticleTypeClass);
	REGISTER_CLASS(ParticleSystemClass, CLSID_ParticleSystemClass);
	REGISTER_CLASS(ParticleSystemTypeClass, CLSID_ParticleSystemTypeClass);
	REGISTER_CLASS(AITriggerTypeClass, CLSID_AITriggerTypeClass);
	REGISTER_CLASS(NeuronClass, CLSID_NeuronClass);
	REGISTER_CLASS(FoggedObjectClass, CLSID_FoggedObjectClass);
	REGISTER_CLASS(AlphaShapeClass, CLSID_AlphaShapeClass);

	if (failed) {
		MessageBox(NULL, Fetch_String(TXT_PREPARECOM_FAILED), Fetch_String(TXT_SHORT_TITLE), MB_ICONEXCLAMATION);
	}

	return(failed);

}

/***********************************************************************************************
 * main -- Initial startup routine (preps library systems).                                    *
 *                                                                                             *
 *    This is the routine that is first called when the program starts up. It basically        *
 *    handles the command line parsing and setting up library systems.                         *
 *                                                                                             *
 * INPUT:   argc  -- Number of command line arguments.                                         *
 *                                                                                             *
 *          argv  -- Pointer to array of command line argument strings.                        *
 *                                                                                             *
 * OUTPUT:  Returns with execution failure code (if any).                                      *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/20/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int CALLBACK WinMain ( HINSTANCE instance , HINSTANCE , char * command_line , int command_show )
{
	int		argc;       //Command line argument count
	char *	argv[20];   //Pointers to command line arguments
	char	path_to_exe[132];
	char	buffer[512];

#ifdef STEVES_NEW_CATCHER
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF|_CRTDBG_LEAK_CHECK_DF);
#endif

	// First, so that everything after it is covered, including the rest of this function.
	Install_Exception_Handler();

	ProgramInstance = instance;

	Debug_Init();

	// Handed over now because the exception path may not ask the logger for anything: the
	// thread that crashed may be the one holding the logger's lock.
	Exception_Register_Log_File(Debug_Log_File_Name());

	/*
	 * Create a mutex with a unique name to TibSun in order to determine if
	 * our app is already running.
	 *
	 * WARNING: DO NOT use this number for any other application except TibSun
	 */
	AppMutex = ::CreateMutex (NULL, FALSE, APP_GUID);

	//
	// Is there already an instance of this app somewhere?
	//
	if (::GetLastError () == ERROR_ALREADY_EXISTS) {
		//
		// Find the previous instance
		//
		HWND main_wnd = ::FindWindow (APP_GUID, NULL);
		if (main_wnd != NULL) {
			::SetForegroundWindow (main_wnd);
			::ShowWindow (main_wnd, SW_RESTORE);
		}
		if (AppMutex != NULL) {
			CloseHandle(AppMutex);
			AppMutex = NULL;
		}
		DebugString("TibSun is already running...Bail!\n");
		return(EXIT_SUCCESS);
	} else {

		DebugString("Create AppMutex okay.\n");

		//
		// Obtain the mutex unique to the Renegade AutoPlay application.
		//
		// WARNING: DO NOT use this number for any other application except Renegade AutoPlay
		//
		do
		{
			//
			// Attempt to open the mutex
			//
			AutoPlayMutex = ::OpenMutex (MUTEX_ALL_ACCESS, FALSE, AUTOPLAY_GUID);
			if (AutoPlayMutex != NULL) {
				DebugString( "Waiting for Autoplay to quit!\n");
				if (::WaitForSingleObject (AutoPlayMutex, 30000) == WAIT_FAILED) {
					DebugString ("Failed waiting for AutoPlayMutex\n");
					::CloseHandle (AutoPlayMutex);
					AutoPlayMutex = NULL;
				}
			}

			/*
			 * Create a mutex with a name unique to the TibSun AutoPlay application.
			 * This prevents the autoplay from running since it cannot get the mutex.
			 * TibSun needs both of these mutexs before it is allowed to run.
			 */
			if (AutoPlayMutex == NULL) {
				AutoPlayMutex = CreateMutex (NULL, FALSE, AUTOPLAY_GUID);
				if (GetLastError () == ERROR_ALREADY_EXISTS) {
					CloseHandle (AutoPlayMutex);
					AutoPlayMutex = NULL;
					Sleep (2500);
				} else {
					DebugString("Create AutoPlayMutex.\n");
				}
			}
		} while (AutoPlayMutex == NULL);

		DebugString ("Got AutoPlayMutex okay.\n");
	}

	atexit(Prog_End);

	if (!Init_Language_Resources(true)) {
		return(EXIT_SUCCESS);
	}

	if (GetDllVersion("comctl32.dll") < PACKVERSION(4, 70)) {
		sprintf(buffer, Fetch_String(TXT_DLL_INVALID), "comctl32.dll", 4, 70, "comctl32.dll");
		MessageBox(NULL, buffer, Fetch_String(TXT_SHORT_TITLE), MB_ICONERROR);
		exit(EXIT_FAILURE);
	}

	OleInitialize(NULL);

	if (RegisterClasses()) {
		exit(EXIT_FAILURE);
	}

	/*
	**	Get the full path to the .EXE
	*/
	GetModuleFileName (instance, &path_to_exe[0], sizeof(path_to_exe));

	/*
	**	First argument is supposed to be a pointer to the .EXE that is running
	**
	*/
	argc=1;                     //Set argument count to 1
	argv[0]=&path_to_exe[0];    //Set 1st command line argument to point to full path

	/*
	**	Get pointers to command line arguments just like if we were in DOS
	**
	*/
	char *token = strtok(command_line, " ");
	while (argc < ARRAY_SIZE(argv) && token != NULL) {
		argv[argc++] = strtrim(token);
		token = strtok(NULL, " ");
	}

	/*
	**	Change directory to the where the executable is located. Handle the
	**	case where there is no path attached to argv[0].
	*/
	char drive[_MAX_DRIVE];
	char path[_MAX_PATH];
	char dir[_MAX_DIR];
	_splitpath(argv[0], drive, dir, NULL, NULL);
	_makepath(path, drive, dir, NULL, NULL);
	SetCurrentDirectory(path);

	int error_code = EXIT_FAILURE;

	if (Parse_Command_Line(argc, argv)) {

		Exception_Run_Immediate_Test();

		RawFileClass *cfile = new RawFileClass(CONFIG_FILE_NAME);

		ConfigINI.Load(*cfile, false);
		Options.ScreenWidth = ConfigINI.Get_Int("Video", "ScreenWidth", Options.ScreenWidth);
		Options.ScreenHeight = ConfigINI.Get_Int("Video", "ScreenHeight", Options.ScreenHeight);

		/*
		 * These are wanted before the window and the renderer exist, which is well
		 * before the rest of the settings are read.
		 */
		Options.Fullscreen = ConfigINI.Get_Bool("Video", "Fullscreen", Options.Fullscreen);
		Options.WindowWidth = ConfigINI.Get_Int("Video", "WindowWidth", Options.WindowWidth);
		Options.WindowHeight = ConfigINI.Get_Int("Video", "WindowHeight", Options.WindowHeight);
		Options.VSync = ConfigINI.Get_Bool("Video", "VSync", Options.VSync);
		Options.Renderer = ConfigINI.Get_Int("Video", "Renderer", Options.Renderer);

		/*
		 * The command line asks for a window regardless of what the settings say.
		 */
		if (!WindowedMode) {
			WindowedMode = !Options.Fullscreen;
		}

		Keyboard = new KeyboardClass();

		/*
		**	If there is not enough disk space free, don't allow the product to run.
		*/
		if (Disk_Space_Available() < INIT_FREE_DISK_SPACE) {
			wsprintf (buffer, Fetch_String(TXT_CRITICALLY_LOW), (INIT_FREE_DISK_SPACE) / (1024 * 1024));
			int reply = MessageBox(NULL, buffer, Fetch_String(TXT_SHORT_TITLE), MB_ICONQUESTION|MB_YESNO);
			if (reply == IDNO) {
				OleUninitialize();
				return(EXIT_FAILURE);
			}
		}

		if (Session.ShowInternetDebug) {
			Options.ScreenWidth = 640;
			Options.ScreenHeight = 400;
		}

		if (Options.ScreenWidth == -1 || Options.ScreenHeight == -1) {
			Options.ScreenWidth = 640;
			Options.ScreenHeight = 480;
		}

		VisibleRect = Rect(0, 0, Options.ScreenWidth, Options.ScreenHeight);
		VideoModeWidth = Options.ScreenWidth;
		VideoModeHeight = Options.ScreenHeight;

		Create_Main_Window(instance, command_show, Options.ScreenWidth, Options.ScreenHeight);

		Exception_Run_Post_Window_Test();

		Audio.Init(MainWindow, 16, 0, 22050);

		if (!Video_Init(MainWindow)) {
			MessageBox(MainWindow, Fetch_String(TXT_VIDEO_ERROR), Fetch_String(TXT_SHORT_TITLE), MB_ICONWARNING);
			exit(EXIT_FAILURE);
		}

		VisibleSurface = DSurface::Create_Primary();
		if (VisibleSurface == NULL) {
			MessageBox(MainWindow, Fetch_String(TXT_VIDEO_ERROR), Fetch_String(TXT_SHORT_TITLE), MB_ICONWARNING);
			exit(EXIT_FAILURE);
		}

		do {
			Windows_Message_Handler();
		}
		while (!GameInFocus);

		VisibleSurface->Fill(0);

		Rect sidebar_rect(0,0,SidebarClass::SIDE_WIDTH,VisibleRect.Height);
		Rect tile_rect(0,0,VisibleRect.Width-sidebar_rect.Width, sidebar_rect.Height);
		Rect composite_rect(0,0,VisibleRect.Width-sidebar_rect.Width, sidebar_rect.Height);

		Allocate_Surfaces(VisibleRect, composite_rect, tile_rect, sidebar_rect, false);
		LogicalSurface = HiddenSurface;
		Update_Visible_Surface(HiddenSurface);

		DepthBuffer = new ZBuffer(Rect(TacticalRect.X, TacticalRect.Y, 480, 480 - TacticalRect.Y));
		DepthBuffer->Set_Scroll(ZBUFFER_MAX);

		AlphaBuffer = new ABuffer(Rect(TacticalRect.X, TacticalRect.Y, 480, 480 - TacticalRect.Y));

		MouseCursor = new WWMouseClass(MainWindow);
		MouseCursor->Capture_Mouse();

		/*
		**	Check for forced intro movie run disabling. If the conquer
		**	configuration file says "no", then don't run the intro.
		*/
		// A client-launched game plays no intro, and the settings file it manages is left
		// exactly as the client wrote it.
		if (!Special.IsFromInstall && !Spawner_Is_Requested()) {
			Special.IsFromInstall = ConfigINI.Get_Bool("Intro", "PlayIntro", true);
		}

		/*
		**	Regardless of whether we should run it or not, here we're
		**	gonna change it to say "no" in the future.
		*/
		if (Special.IsFromInstall == true && !Spawner_Is_Requested()) {
			ConfigINI.Put_Bool("Intro", "PlayIntro", false);
			cfile->Close();
			cfile->Open();
			ConfigINI.Save(*cfile, false);
		}

		cfile->Close();
		delete cfile;

		DebugString("Main_Game\n");

		Main_Game(argc, argv);

		HiddenSurface->Fill(0);
		Update_Visible_Surface(HiddenSurface);

		/*
		**	Flag that this is a clean shutdown (not killed with Ctrl-Alt-Del)
		*/
		ReadyToQuit = 1;

		Audio.End();

		/*
		**	Post a message to our message handler to tell it to clean up.
		*/
		PostMessage(MainWindow, WM_DESTROY, 0, 0);

		/*
		**	Wait until the message handler has dealt with the message
		*/
		do
		{
			Windows_Message_Handler();
		}while (ReadyToQuit == 1);

		error_code = EXIT_SUCCESS;

	} else {

		// The help and the invalid option message are of no use if the console closes with
		// the process a moment later.
		Debug_Console_Hold();
	}

	OleUninitialize();

	return(error_code);
}

/***********************************************************************************************
 * Prog_End -- Cleans up library systems in prep for game exit.                                *
 *                                                                                             *
 *    This routine should be called before the game terminates. It handles cleaning up         *
 *    library systems so that a graceful return to the host operating system is achieved.      *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/20/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void __cdecl Prog_End(void)
{
	int i;

	GameActive = false;

	Session.Free_Scenario_Descriptions();

	while (ColorSchemes.Count()) {
		delete ColorSchemes[0];
	}
	ColorSchemes.Clear();

	for (i = 0; i < TutorialText.Count(); i++) {
		free((void *)TutorialText.Fetch_By_Position(i));
	}
	TutorialText.Clear();

	Delete_All_Objects();

	while (Movies.Count()) {
		free((void *)Movies[0]);
		Movies.Delete_Index(0);
	}
	Movies.Clear();

	for (i = 0; i < AllCommands.Count(); i++) {
		delete AllCommands[i];
	}
	AllCommands.Clear();

	Theme.Free_Themes();

	if (RuleINI != NULL) {
		delete RuleINI;
		RuleINI = NULL;
	}
	AIINI.Clear();
	ArtINI.Clear();
	FSRuleINI.Clear();
	FSAIINI.Clear();
	EditorINI.Clear();
	ConfigINI.Clear();
	ConfigINI.Clear();

	SpotLightClass::Clear_All();
	IonBlastClass::Clear_All();

	if (MouseCursor) {
		delete MouseCursor;
		MouseCursor = NULL;
	}

	Map.Free_Cells();
	Map.Clear_Radar();

	Reset_Surfaces();

	if (DepthBuffer != NULL) {
		delete DepthBuffer;
		DepthBuffer = NULL;
	}
	if (AlphaBuffer != NULL) {
		delete AlphaBuffer;
		AlphaBuffer = NULL;
	}

	if (Metal12FontPtr != NULL) {
		delete Metal12FontPtr;
		Metal12FontPtr = NULL;
	}
	if (MapFontPtr != NULL) {
		delete MapFontPtr;
		MapFontPtr = NULL;
	}
	if (Font6Ptr != NULL) {
		delete Font6Ptr;
		Font6Ptr = NULL;
	}
	if (EditorFont != NULL) {
		delete EditorFont;
		EditorFont = NULL;
	}
	if (Font8Ptr != NULL) {
		delete Font8Ptr;
		Font8Ptr = NULL;
	}
	if (GradFont6Ptr != NULL) {
		delete GradFont6Ptr;
		GradFont6Ptr = NULL;
	}

	if (TerrainDrawer != NULL) {
		delete TerrainDrawer;
		TerrainDrawer = NULL;
	}
	if (AnimDrawer != NULL) {
		delete AnimDrawer;
		AnimDrawer = NULL;
	}
	if (NormalDrawer != NULL) {
		delete NormalDrawer;
		NormalDrawer = NULL;
	}
	if (VoxelDrawer != NULL) {
		delete VoxelDrawer;
		VoxelDrawer = NULL;
	}
	if (MouseDrawer != NULL) {
		delete MouseDrawer;
		MouseDrawer = NULL;
	}
	if (SidebarDrawer != NULL) {
		delete SidebarDrawer;
		SidebarDrawer = NULL;
	}
	if (CameoDrawer != NULL) {
		delete CameoDrawer;
		CameoDrawer = NULL;
	}
	if (EightBitDrawer != NULL) {
		delete EightBitDrawer;
		EightBitDrawer = NULL;
	}
	if (EightBitSurface != NULL) {
		delete EightBitSurface;
		EightBitSurface = NULL;
	}

	if (CloakingSurface != NULL) {
		delete (Surface *)CloakingSurface;
		CloakingSurface = NULL;
	}

	if (BuildingTypeClass::BuildingZShape != NULL) {
		delete (void *)BuildingTypeClass::BuildingZShape;
		BuildingTypeClass::BuildingZShape = NULL;
	}

	Map.Shutdown();

	for (i = 0; i < TileDrawers.Count(); i++) {
		delete TileDrawers[i];
	}
	TileDrawers.Clear();

	if (TheaterData) {
		delete TheaterData;
		TheaterData = NULL;
	}
	if (TheaterDat) {
		delete TheaterDat;
		TheaterDat = NULL;
	}
	if (IsometricTheaterData) {
		delete IsometricTheaterData;
		IsometricTheaterData = NULL;
	}

	if (IsometricTileTypeClass::CellShadowShapes != NULL) {
		delete IsometricTileTypeClass::CellShadowShapes;
		IsometricTileTypeClass::CellShadowShapes = NULL;
	}

	if (SlopeZShapes[0] != NULL) {
		delete (ShapeSet *)SlopeZShapes[0];
		SlopeZShapes[0] = NULL;
	}
	if (SlopeZShapes[1] != NULL) {
		delete (ShapeSet *)SlopeZShapes[1];
		SlopeZShapes[1] = NULL;
	}
	if (SlopeZShapes[2] != NULL) {
		delete (ShapeSet *)SlopeZShapes[2];
		SlopeZShapes[2] = NULL;
	}
	if (SlopeZShapes[3] != NULL) {
		delete (ShapeSet *)SlopeZShapes[3];
		SlopeZShapes[3] = NULL;
	}

	if (PreviewSurface != NULL) {
		delete PreviewSurface;
		PreviewSurface = NULL;
	}

	if (MoviesMix != NULL) {
		delete MoviesMix;
		MoviesMix = NULL;
	}
	while (MoviesMixLocal.Count() > 0) {
		delete MoviesMixLocal[0];
		MoviesMixLocal.Delete_Index(0);
	}
	if (ScoresMix != NULL) {
		delete ScoresMix;
		ScoresMix = NULL;
	}
	if (Scores01Mix != NULL) {
		delete Scores01Mix;
		Scores01Mix = NULL;
	}
	if (MainMix != NULL) {
		delete MainMix;
		MainMix = NULL;
	}
	if (ConquerMix != NULL) {
		delete ConquerMix;
		ConquerMix = NULL;
	}

	while (ExpandSideMix.Count() > 0) {
		delete ExpandSideMix[0];
		ExpandSideMix.Delete_Index(0);
	}
	while (ExpandSpeechMix.Count() > 0) {
		delete ExpandSpeechMix[0];
		ExpandSpeechMix.Delete_Index(0);
	}
	while (ExpandMix.Count() > 0) {
		delete ExpandMix[0];
		ExpandMix.Delete_Index(0);
	}
	if (CacheMix != NULL) {
		delete CacheMix;
		CacheMix = NULL;
	}
	if (LocalMix != NULL) {
		delete LocalMix;
		LocalMix = NULL;
	}
	if (SpeechMix != NULL) {
		delete SpeechMix;
		SpeechMix = NULL;
	}
	if (SoundsMix != NULL) {
		delete SoundsMix;
		SoundsMix = NULL;
	}
	if (Sounds01Mix != NULL) {
		delete Sounds01Mix;
		Sounds01Mix = NULL;
	}
	if (MapsMix != NULL) {
		delete MapsMix;
		MapsMix = NULL;
	}
	while (MapsMixLocal.Count() > 0) {
		delete MapsMixLocal[0];
		MapsMixLocal.Delete_Index(0);
	}
	if (MultiMix != NULL) {
		delete MultiMix;
		MultiMix = NULL;
	}
	if (SideCMix != NULL) {
		delete SideCMix;
		SideCMix = NULL;
	}
	if (SideNCMix != NULL) {
		delete SideNCMix;
		SideNCMix = NULL;
	}
	if (SideCDMix != NULL) {
		delete SideCDMix;
		SideCDMix = NULL;
	}
	if (GameMix != NULL) {
		delete GameMix;
		GameMix = NULL;
	}

	if (TacticalMap != NULL) {
		delete TacticalMap;
		TacticalMap = NULL;
	}

	delete SpeechBuffer[0];
	SpeechBuffer[0] = NULL;

	Free_Vocs();

	CDFileClass::Clear_Search_Drives();

	if (Keyboard != NULL) {
		delete Keyboard;
		Keyboard = NULL;
	}

	Shutdown_Network();

	if (UnkBuffer != NULL) {
		delete UnkBuffer;
		UnkBuffer = NULL;
	}

	if (Rule != NULL) {
		delete Rule;
		Rule = NULL;
	}

	if (Scen != NULL) {
		delete Scen;
		Scen = NULL;
	}

	for (i = 0; i < RegisteredClasses.Count(); i++) {
		CoRevokeClassObject((DWORD)RegisteredClasses[i]);
	}
	RegisteredClasses.Clear();

	if (LanguageResources) {
		FreeLibrary(LanguageResources);
	}

	if (AutoPlayMutex != NULL) {
		CloseHandle(AutoPlayMutex);
		AutoPlayMutex = NULL;
	}
	if (AppMutex != NULL) {
		CloseHandle(AppMutex);
		AppMutex = NULL;
	}

#ifdef STEVES_NEW_CATCHER
	_CrtDumpMemoryLeaks();
#endif
}

/***********************************************************************************************
 * Emergency_Exit -- Function to call when we want to exit unexpectedly.                       *
 *                   Use this function instead of exit(n) so everything is properly cleaned up.*
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Code to return to the OS                                                          *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *    3/13/97 1:32AM ST : Created                                                              *
 *=============================================================================================*/
void Emergency_Exit(void)
{
	Audio.End();

	ReadyToQuit = 1;

	/*
	**	Post a message to our message handler to tell it to clean up.
	*/
	PostMessage(MainWindow, WM_DESTROY, 0, 0);

	while (MainWindow) {
		Windows_Message_Handler();
		if (ReadyToQuit != 1) {
			break;
		}
	}

	OleUninitialize();

	if (MouseCursor) {
		MouseCursor->Release_Mouse();
		delete MouseCursor;
	}
	MouseCursor = NULL;

	PostQuitMessage(EXIT_SUCCESS);

	Shutdown_Network();

	Prog_End();
}
