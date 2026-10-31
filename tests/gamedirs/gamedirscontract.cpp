/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// Exercises the game directories without the engine or any game data: the folder list it
// is handed, the scan that covers every folder, and where a player's own files are read
// from and written to. Every file this uses is one the harness makes itself.

#include <windows.h>

#include <cstdio>
#include <string>
#include <vector>

#include "cdfile.h"
#include "deploymentconfig.h"
#include "gamedirs.h"
#include "rawfile.h"

#include "doctest/doctest.h"

namespace {

std::string Root;
char OriginalDirectory[MAX_PATH];

void Check_List(std::vector<std::string> const & actual, std::vector<std::string> const & expected, char const * what)
{
	bool same = actual.size() == expected.size();

	for (unsigned int index = 0; same && index < actual.size(); index++) {
		same = actual[index] == expected[index];
	}

	CHECK_MESSAGE(same, what);

	if (!same) {
		std::printf("    got:");
		for (std::string const & entry : actual) {
			std::printf(" [%s]", entry.c_str());
		}
		std::printf("\n    expected:");
		for (std::string const & entry : expected) {
			std::printf(" [%s]", entry.c_str());
		}
		std::printf("\n");
	}
}


// The file object keeps the name pointer it is handed rather than copying it, so the string
// it points into has to outlive it.
void Write_File(std::string const & path, char const * contents)
{
	RawFileClass file(path.c_str());

	file.Open(FileClass::WRITE);
	file.Write(contents, (int)strlen(contents));
	file.Close();
}


std::string Read_File(std::string const & path)
{
	RawFileClass file(path.c_str());

	if (!file.Is_Available()) {
		return(std::string());
	}

	int const size = file.Size();
	std::string contents(size, '\0');

	file.Open(FileClass::READ);
	file.Read(contents.data(), size);
	file.Close();

	return(contents);
}


bool File_Exists(std::string const & path)
{
	return(GetFileAttributes(path.c_str()) != INVALID_FILE_ATTRIBUTES);
}


void Make_Directory(std::string const & path)
{
	CreateDirectory(path.c_str(), NULL);
}


void Remove_Directory(std::string const & path)
{
	RemoveDirectory(path.c_str());
}


/*
 * Every case starts from the same empty tree, with no folders configured and the current
 * directory back at the root, so that one case cannot decide another's outcome.
 */
void Reset(void)
{
	CDFileClass::Clear_Search_Drives();
	Set_Data_Directory("");
	Set_User_Directory("");
	SetCurrentDirectory(Root.c_str());
}


TEST_CASE("a macOS application path is recognized")
{
	CHECK_MESSAGE(Is_MacOS_App_Executable("/Users/me/siliconsun.app/Contents/MacOS/Game"), "the release bundle executable is recognized");
	CHECK_MESSAGE(!Is_MacOS_App_Executable("/Users/me/build/macos/bin/GameD"), "a loose binary is not an application bundle");
	CHECK_MESSAGE(!Is_MacOS_App_Executable("/Users/me/Contents/MacOS/Game"), "Contents/MacOS without an .app directory is not a bundle");
	CHECK_MESSAGE(!Is_MacOS_App_Executable("/Users/me/Game.app/Contents/MacOS/Helpers/tool"), "a nested helper is not the bundle executable");
	CHECK_MESSAGE(!Is_MacOS_App_Executable(nullptr), "a missing path is not a bundle");
	CHECK_MESSAGE(!Is_MacOS_App_Executable(""), "an empty path is not a bundle");
}


TEST_CASE("bundle directories are under the home directory")
{
	std::string data;
	std::string user;

	CHECK_MESSAGE(MacOS_Bundle_Directories("/Users/me", data, user), "a home directory names both folders");
	CHECK_MESSAGE(data == "/Users/me/Games/OpenTS", "game data is Games/OpenTS");
	CHECK_MESSAGE(user == "/Users/me/Library/Application Support/siliconsun", "player files are in Application Support");

	data = "keep";
	user = "keep";
	CHECK_MESSAGE(!MacOS_Bundle_Directories(nullptr, data, user), "a missing home directory names nothing");
	CHECK_MESSAGE(data == "keep" && user == "keep", "a missing home directory leaves the outputs alone");
	CHECK_MESSAGE(!MacOS_Bundle_Directories("", data, user), "an empty home directory names nothing");
	CHECK_MESSAGE(!MacOS_Bundle_Directories("///", data, user), "a home directory of only slashes names nothing");
	CHECK_MESSAGE(data == "keep" && user == "keep", "a rejected home directory leaves the outputs alone");

	CHECK_MESSAGE(MacOS_Bundle_Directories("/Users/me/", data, user), "a trailing slash is dropped");
	CHECK_MESSAGE(data == "/Users/me/Games/OpenTS", "a trailing slash is dropped");
}


bool Make_Root(void);
void Remove_Root(void);


// Each case works in a tree of its own, made fresh before it and removed after it.
struct RootFixture
{
	RootFixture(void)
	{
		GetCurrentDirectory(sizeof(OriginalDirectory), OriginalDirectory);
		REQUIRE_MESSAGE(Make_Root(), "the working directory is created");
	}

	~RootFixture(void)
	{
		Remove_Root();
	}
};


TEST_CASE_FIXTURE(RootFixture, "unnamed directories take the bundle defaults and named ones stay")
{
	Reset();
	Apply_Unnamed_Directories("D:\\Data", "D:\\User");
	CHECK_MESSAGE(Data_Directory() == "D:\\Data\\", "an unnamed data directory takes the default");
	CHECK_MESSAGE(CDFileClass::User_Path() != nullptr, "an unnamed user directory takes the default");
	CHECK_MESSAGE(std::string(CDFileClass::User_Path()) == "D:\\User\\", "an unnamed user directory takes the default");

	Reset();
	Set_Data_Directory("D:\\Keep");
	Apply_Unnamed_Directories("D:\\Data", "D:\\User");
	CHECK_MESSAGE(Data_Directory() == "D:\\Keep\\", "a data directory named on the command line stays");
	CHECK_MESSAGE(std::string(CDFileClass::User_Path()) == "D:\\User\\", "an unnamed user directory still takes the default");

	Reset();
	Set_User_Directory("D:\\Keep");
	Apply_Unnamed_Directories("D:\\Data", "D:\\User");
	CHECK_MESSAGE(Data_Directory() == "D:\\Data\\", "an unnamed data directory still takes the default");
	CHECK_MESSAGE(std::string(CDFileClass::User_Path()) == "D:\\Keep\\", "a user directory named on the command line stays");
}


TEST_CASE_FIXTURE(RootFixture, "a folder list keeps its order, drops whitespace and repeats, and passes over the game's own directory")
{
	Check_List(Parse_Search_Folders("INI,MIX"), {"INI\\", "MIX\\"}, "a plain list keeps its order");
	Check_List(Parse_Search_Folders("  INI  ,\tMIX "), {"INI\\", "MIX\\"}, "surrounding whitespace is dropped");
	Check_List(Parse_Search_Folders("INI\\,MIX/"), {"INI\\", "MIX/"}, "a separator already written is kept");
	Check_List(Parse_Search_Folders("INI,ini\\,INI"), {"INI\\"}, "the same folder written differently is one folder");
	Check_List(Parse_Search_Folders("INI,,MIX"), {"INI\\", "MIX\\"}, "an empty entry is passed over");
	Check_List(Parse_Search_Folders(""), {}, "an empty list names no folders");
	Check_List(Parse_Search_Folders("   "), {}, "a list of whitespace names no folders");
	Check_List(Parse_Search_Folders(NULL), {}, "no list at all names no folders");
	Check_List(Parse_Search_Folders("D:"), {"D:"}, "a bare drive is left as it is");
	Check_List(Parse_Search_Folders("."), {}, "naming only the game's own directory adds no folder");
	Check_List(Parse_Search_Folders(".,Extra"), {"Extra\\"}, "the game's own directory is passed over in a longer list");
}


// The list the deployment's file supplies when it names no folders.
std::string Default_List(void)
{
	return(DeploymentConfigClass().SearchPaths);
}


TEST_CASE_FIXTURE(RootFixture, "with nothing configured the default folders are searched")
{
	Reset();
	Init_Search_Folders(Default_List().c_str());

	CHECK_MESSAGE(CDFileClass::Search_Path(0) != nullptr, "with no configuration the INI folder is searched");
	CHECK_MESSAGE(std::string(CDFileClass::Search_Path(0)) == "INI\\", "with no configuration the INI folder is searched");
	CHECK_MESSAGE(CDFileClass::Search_Path(1) != nullptr, "with no configuration the MIX folder is searched");
	CHECK_MESSAGE(std::string(CDFileClass::Search_Path(1)) == "MIX\\", "with no configuration the MIX folder is searched");
	CHECK_MESSAGE(CDFileClass::Search_Path(2) != nullptr, "with no configuration the Maps folder is searched");
	CHECK_MESSAGE(std::string(CDFileClass::Search_Path(2)) == "Maps\\", "with no configuration the Maps folder is searched");
	CHECK_MESSAGE(CDFileClass::Search_Path(3) == nullptr, "nothing else is searched");
}


TEST_CASE_FIXTURE(RootFixture, "a configured list replaces the default folders")
{
	Reset();
	Init_Search_Folders("Data,More");

	CHECK_MESSAGE(CDFileClass::Search_Path(0) != nullptr, "a configured folder is searched");
	CHECK_MESSAGE(std::string(CDFileClass::Search_Path(0)) == "Data\\", "a configured folder is searched");
	CHECK_MESSAGE(CDFileClass::Search_Path(1) != nullptr, "configured folders keep the order they are written in");
	CHECK_MESSAGE(std::string(CDFileClass::Search_Path(1)) == "More\\", "configured folders keep the order they are written in");
	CHECK_MESSAGE(CDFileClass::Search_Path(2) == nullptr, "a configured list replaces the default folders");

	// Naming only the game's own directory is how a deployment asks for no other folder.
	Reset();
	Init_Search_Folders(".");

	CHECK_MESSAGE(CDFileClass::Search_Path(0) == nullptr, "naming only the game's own directory turns the default folders off");
}


TEST_CASE_FIXTURE(RootFixture, "a data directory that exists is accepted")
{
	Reset();
	Set_Data_Directory((Root + "\\Data").c_str());

	CHECK_MESSAGE(Apply_Game_Directories(), "a data directory that exists is accepted");
	Init_Search_Folders("Sorted");

	std::string const expected_data = Root + "\\Data\\";
	std::string const expected_sorted = expected_data + "Sorted\\";

	CHECK_MESSAGE(CDFileClass::Search_Path(0) != nullptr, "the data directory itself is searched");
	CHECK_MESSAGE(std::string(CDFileClass::Search_Path(0)) == expected_data, "the data directory itself is searched");
	CHECK_MESSAGE(CDFileClass::Search_Path(1) != nullptr, "a folder it configures is searched inside it");
	CHECK_MESSAGE(std::string(CDFileClass::Search_Path(1)) == expected_sorted, "a folder it configures is searched inside it");

	Reset();
	Set_Data_Directory((Root + "\\Missing").c_str());
	CHECK_MESSAGE(!Apply_Game_Directories(), "a data directory that is not there is refused");
}


TEST_CASE_FIXTURE(RootFixture, "a user directory is created when it is not there yet")
{
	Reset();
	Set_User_Directory((Root + "\\User\\Fresh").c_str());

	CHECK_MESSAGE(Apply_Game_Directories(), "a user directory is created when it is not there yet");
	CHECK_MESSAGE(GetFileAttributes((Root + "\\User\\Fresh").c_str()) != INVALID_FILE_ATTRIBUTES, "the created user directory is on the disk");

	std::string const expected_user = Root + "\\User\\Fresh\\";
	CHECK_MESSAGE(CDFileClass::User_Path() != nullptr, "the file layer is told where the player's own files go");
	CHECK_MESSAGE(std::string(CDFileClass::User_Path()) == expected_user, "the file layer is told where the player's own files go");
	CHECK_MESSAGE(CDFileClass::Search_Path(0) == nullptr, "the user directory is not one of the searched folders");

	CHECK_MESSAGE(User_File_Write_Name("SUN.INI") == expected_user + "SUN.INI", "a file the game writes goes to the user directory");

	Reset();
	CHECK_MESSAGE(CDFileClass::User_Path() == nullptr, "with no user directory the file layer has none either");
	CHECK_MESSAGE(User_File_Write_Name("SUN.INI") == "SUN.INI", "with no user directory a written file keeps its plain name");
}


TEST_CASE_FIXTURE(RootFixture, "a file is found in the folders searched, in their order")
{
	Reset();

	Write_File(Root + "\\ALPHA.MPR", "");
	Write_File(Root + "\\INI\\BRAVO.MPR", "");
	Write_File(Root + "\\INI\\ALPHA.MPR", "");
	Write_File(Root + "\\MIX\\CHARLIE.MPR", "");

	Init_Search_Folders(Default_List().c_str());

	Check_List(Search_Files("*.MPR"), {"ALPHA.MPR", "BRAVO.MPR", "CHARLIE.MPR"},
		"a scan covers every folder, reports a name once, and sorts it");

	/*
	 * The scan and an ordinary open have to agree, or the game would list one file and load
	 * another. Both walk the folders in the same order, so the game's own copy wins.
	 */
	CDFileClass found("ALPHA.MPR");
	CHECK_MESSAGE(std::string(found.File_Name()) == "ALPHA.MPR", "opening a name the scan reported lands on the copy the scan saw");

	CDFileClass sorted("CHARLIE.MPR");
	CHECK_MESSAGE(std::string(sorted.File_Name()) == "MIX\\CHARLIE.MPR", "a name held only by a searched folder opens from that folder");
}


TEST_CASE_FIXTURE(RootFixture, "a read-write open does not settle on a searched folder")
{
	Reset();
	Write_File(Root + "\\MIX\\WRITTEN.DAT", "shipped");
	Init_Search_Folders(Default_List().c_str());

	/*
	 * A file opened for writing must never be looked for anywhere but the current directory:
	 * a deployment's folders are read from, not written to.
	 */
	CDFileClass file;
	file.Open("WRITTEN.DAT", FileClass::READ|FileClass::WRITE);
	std::string const written = file.File_Name();
	file.Close();

	CHECK_MESSAGE(written == "WRITTEN.DAT", "a read-write open does not settle on a searched folder");

	CHECK_MESSAGE(GetFileAttributes((Root + "\\WRITTEN.DAT").c_str()) != INVALID_FILE_ATTRIBUTES, "the written file is in the current directory");

	WIN32_FILE_ATTRIBUTE_DATA shipped;
	GetFileAttributesEx((Root + "\\MIX\\WRITTEN.DAT").c_str(), GetFileExInfoStandard, &shipped);
	CHECK_MESSAGE(shipped.nFileSizeLow == 7, "the copy in the searched folder is untouched");
}


/*
 * A file named with a directory of its own, looked up while folders are searched, makes the
 * search build a pathname out of two full paths. That pair does not have to fit in one, and
 * once it does not the search has to pass it over rather than build it anyway.
 */
TEST_CASE_FIXTURE(RootFixture, "a scan passes over a folder it cannot build a name in")
{
	Reset();

	std::string long_folder = Root + "\\";
	while (long_folder.length() < 150) {
		long_folder += "x";
	}
	long_folder += "\\";

	CDFileClass::Add_Search_Drive(long_folder.c_str());

	std::string const absolute = Root + "\\ALPHA.MPR";
	Write_File(absolute, "");

	CDFileClass file(absolute.c_str());
	CHECK_MESSAGE(std::string(file.File_Name()) == absolute, "a file named with its own directory is found while long folders are searched");

	CDFileClass missing((Root + "\\NOTHERE.MPR").c_str());
	CHECK_MESSAGE(std::string(missing.File_Name()) == Root + "\\NOTHERE.MPR", "a name that no folder holds comes back as it was given");

	Check_List(Search_Files("*.MPR"), {"ALPHA.MPR"}, "a scan passes over a folder it cannot build a name in");
}


/*
 * The file layer places what the game writes and finds what it reads. These are the rules a
 * caller never states, so they are checked here rather than at any one of them.
 */
TEST_CASE_FIXTURE(RootFixture, "the file layer places written files")
{
	Reset();
	Set_User_Directory((Root + "\\User\\Own").c_str());
	Apply_Game_Directories();
	Init_Search_Folders(Default_List().c_str());

	std::string const own = Root + "\\User\\Own\\";

	CDFileClass written("OWN.DAT");
	written.Open(FileClass::WRITE);
	written.Write("mine", 4);
	written.Close();

	CHECK_MESSAGE(std::string(written.File_Name()) == own + "OWN.DAT", "a written file is named in the user directory");
	CHECK_MESSAGE(File_Exists(own + "OWN.DAT"), "a written file is in the user directory");
	CHECK_MESSAGE(!File_Exists(Root + "\\OWN.DAT"), "a written file is not beside the game");

	/*
	 * A second object, made once both copies exist, so that what answers is the search and
	 * not the object that did the writing.
	 */
	Write_File(Root + "\\MIX\\SHARED.DAT", "shipped");
	Write_File(own + "SHARED.DAT", "own");

	CDFileClass shared("SHARED.DAT");
	CHECK_MESSAGE(Read_File(shared.File_Name()) == "own", "a read prefers the player's own copy");

	// A created file is read back from where it was created, which is what a game storing
	// its progress does every time it starts.
	CDFileClass progress("PROGRESS.INI");
	CHECK_MESSAGE(!progress.Is_Available(), "a file the player has never had is not there yet");
	progress.Create();
	progress.Close();

	CDFileClass reopened("PROGRESS.INI");
	CHECK_MESSAGE(reopened.Is_Available(), "a created file is found again");
	CHECK_MESSAGE(std::string(reopened.File_Name()) == own + "PROGRESS.INI", "a created file is found in the user directory");
}


TEST_CASE_FIXTURE(RootFixture, "the file layer deletes only the player copy")
{
	Reset();
	Set_User_Directory((Root + "\\User\\Own").c_str());
	Apply_Game_Directories();
	Init_Search_Folders(Default_List().c_str());

	std::string const own = Root + "\\User\\Own\\";

	Write_File(Root + "\\MIX\\GONE.DAT", "shipped");
	Write_File(own + "GONE.DAT", "own");

	CDFileClass discard("GONE.DAT");
	discard.Delete();

	CHECK_MESSAGE(!File_Exists(own + "GONE.DAT"), "the player's own copy is thrown away");
	CHECK_MESSAGE(File_Exists(Root + "\\MIX\\GONE.DAT"), "the copy a deployment ships is left alone");

	CDFileClass again("GONE.DAT");
	CHECK_MESSAGE(Read_File(again.File_Name()) == "shipped", "what a deployment ships answers once the player's copy is gone");
}


/*
 * A deployment ships default hotkeys in a folder it searches, and the player saves their own
 * over the top. Throwing the player's away has to leave the deployment's alone, or the reset
 * takes with it the very defaults it is meant to fall back on.
 */
TEST_CASE_FIXTURE(RootFixture, "resetting keeps the shipped default")
{
	Reset();
	Set_User_Directory((Root + "\\User\\Own").c_str());
	Apply_Game_Directories();
	Init_Search_Folders(Default_List().c_str());

	Write_File(Root + "\\INI\\KEYBOARD.INI", "shipped");

	// A player who has never saved their own asks for the defaults back.
	CDFileClass untouched("KEYBOARD.INI");
	CHECK_MESSAGE(std::string(untouched.File_Name()) == "INI\\KEYBOARD.INI", "a player with none of their own reads the shipped default");
	untouched.Delete();
	CHECK_MESSAGE(File_Exists(Root + "\\INI\\KEYBOARD.INI"), "a reset with nothing of the player's own leaves the shipped default");

	// And now one who has.
	Write_File(Root + "\\User\\Own\\KEYBOARD.INI", "mine");

	CDFileClass owned("KEYBOARD.INI");
	CHECK_MESSAGE(Read_File(owned.File_Name()) == "mine", "the player's own hotkeys are the ones read");
	owned.Delete();

	CHECK_MESSAGE(!File_Exists(Root + "\\User\\Own\\KEYBOARD.INI"), "a reset throws the player's own away");
	CHECK_MESSAGE(File_Exists(Root + "\\INI\\KEYBOARD.INI"), "a reset leaves the shipped default");

	CDFileClass fallback("KEYBOARD.INI");
	CHECK_MESSAGE(Read_File(fallback.File_Name()) == "shipped", "the shipped default answers again after a reset");

	DeleteFile((Root + "\\INI\\KEYBOARD.INI").c_str());
}


TEST_CASE_FIXTURE(RootFixture, "a name with a directory is left alone")
{
	Reset();
	Set_User_Directory((Root + "\\User\\Own").c_str());
	Apply_Game_Directories();

	CDFileClass rooted("MIX\\ROOTED.DAT");
	rooted.Open(FileClass::WRITE);
	rooted.Write("here", 4);
	rooted.Close();

	CHECK_MESSAGE(std::string(rooted.File_Name()) == "MIX\\ROOTED.DAT", "a name with a directory keeps it");
	CHECK_MESSAGE(File_Exists(Root + "\\MIX\\ROOTED.DAT"), "a name with a directory is written where it says");
	CHECK_MESSAGE(!File_Exists(Root + "\\User\\Own\\ROOTED.DAT"), "a name with a directory is not moved");
}


TEST_CASE_FIXTURE(RootFixture, "placing a file is repeatable")
{
	Reset();
	Set_User_Directory((Root + "\\User\\Own").c_str());
	Apply_Game_Directories();

	CDFileClass file("AGAIN.DAT");
	file.Open(FileClass::WRITE);
	file.Close();
	std::string const once = file.File_Name();

	file.Open(FileClass::WRITE);
	file.Close();

	CHECK_MESSAGE(std::string(file.File_Name()) == once, "opening a file for writing twice names it the same place");

	// The buffered path opens the file a second time itself, with read access added.
	CDFileClass buffered("BUFFERED.DAT");
	buffered.Cache(1024);
	buffered.Open(FileClass::WRITE);
	buffered.Write("cached", 6);
	buffered.Close();

	CHECK_MESSAGE(File_Exists(Root + "\\User\\Own\\BUFFERED.DAT"), "a buffered write lands in the user directory");
}


TEST_CASE_FIXTURE(RootFixture, "without a user directory nothing moves")
{
	Reset();
	Init_Search_Folders(Default_List().c_str());

	Write_File(Root + "\\MIX\\STILL.DAT", "shipped");

	CDFileClass written("STILL.DAT");
	written.Open(FileClass::WRITE);
	written.Write("here", 4);
	written.Close();

	CHECK_MESSAGE(std::string(written.File_Name()) == "STILL.DAT", "a written file keeps its plain name");
	CHECK_MESSAGE(File_Exists(Root + "\\STILL.DAT"), "a written file lands beside the game");
	CHECK_MESSAGE(Read_File(Root + "\\MIX\\STILL.DAT") == "shipped", "a searched folder's copy is untouched");

	CDFileClass discard("STILL.DAT");
	discard.Delete();

	CHECK_MESSAGE(!File_Exists(Root + "\\STILL.DAT"), "a delete takes the copy beside the game");
	CHECK_MESSAGE(File_Exists(Root + "\\MIX\\STILL.DAT"), "a delete leaves the searched folder's copy");

	CDFileClass shipped("STILL.DAT");
	CHECK_MESSAGE(std::string(shipped.File_Name()) == "MIX\\STILL.DAT", "a read still falls through to the searched folders");
}


/*
 * Saved games are the one thing the game both writes and browses, so they keep to a folder of
 * their own that is named outright rather than searched for.
 */
TEST_CASE_FIXTURE(RootFixture, "saved games go to their own folder in the user directory")
{
	Reset();
	Init_Search_Folders(Default_List().c_str());

	CHECK_MESSAGE(Saved_Game_Name("SAVE0001.SAV") == "Saved Games\\SAVE0001.SAV", "a saved game is named inside the folder saved games are kept in");
	CHECK_MESSAGE(File_Exists(Root + "\\Saved Games"), "asking for a saved game makes the folder to keep it in");

	for (int index = 0; ; index++) {
		char const * path = CDFileClass::Search_Path(index);
		if (path == NULL) {
			break;
		}

		CHECK_MESSAGE(std::string(path).find("Saved Games") == std::string::npos, "the folder saved games are kept in is not one of the searched folders");
	}

	Reset();
	Set_User_Directory((Root + "\\User\\Saves").c_str());
	Apply_Game_Directories();

	std::string const expected = Root + "\\User\\Saves\\Saved Games";
	CHECK_MESSAGE(Saved_Game_Name("SAVE0002.SAV") == expected + "\\SAVE0002.SAV", "a user directory takes the saved games with it");
	CHECK_MESSAGE(File_Exists(expected), "the folder is made inside the user directory");

	/*
	 * A pattern is named the same way a file is, since the listing scans the one folder rather
	 * than every folder the game reads from.
	 */
	CHECK_MESSAGE(Saved_Game_Name("*.SAV") == expected + "\\*.SAV", "a pattern is named in the same folder the saved games are");
}


/*
 * Screen captures keep to a folder of their own on the same terms, except that the folder is
 * made on every request rather than once, so removing it mid-session costs nothing.
 */
TEST_CASE_FIXTURE(RootFixture, "screenshots go to their own folder in the user directory")
{
	Reset();
	Init_Search_Folders(Default_List().c_str());

	CHECK_MESSAGE(Screenshot_Name("SCRN0000.pcx") == "Screenshots\\SCRN0000.pcx", "a capture is named inside the folder screen captures are kept in");
	CHECK_MESSAGE(File_Exists(Root + "\\Screenshots"), "asking for a capture makes the folder to keep it in");

	for (int index = 0; ; index++) {
		char const * path = CDFileClass::Search_Path(index);
		if (path == NULL) {
			break;
		}

		CHECK_MESSAGE(std::string(path).find("Screenshots") == std::string::npos, "the folder screen captures are kept in is not one of the searched folders");
	}

	Reset();
	Set_User_Directory((Root + "\\User\\Shots").c_str());
	Apply_Game_Directories();

	std::string const expected = Root + "\\User\\Shots\\Screenshots";
	CHECK_MESSAGE(Screenshot_Name("SCRN0001.png") == expected + "\\SCRN0001.png", "a user directory takes the captures with it");
	CHECK_MESSAGE(File_Exists(expected), "the folder is made inside the user directory");

	Remove_Directory(expected);
	CHECK_MESSAGE(!File_Exists(expected), "the folder can be taken away while the game runs");
	CHECK_MESSAGE(Screenshot_Name("SCRN0002.png") == expected + "\\SCRN0002.png", "and naming the next capture still answers");
	CHECK_MESSAGE(File_Exists(expected), "which makes the folder again");
}


bool Make_Root(void)
{
	char temp[MAX_PATH];
	if (GetTempPath(sizeof(temp), temp) == 0) {
		return(false);
	}

	char name[MAX_PATH];
	std::snprintf(name, sizeof(name), "%sopents-gamedirs-%lu", temp, GetCurrentProcessId());
	Root = name;

	Make_Directory(Root);
	Make_Directory(Root + "\\INI");
	Make_Directory(Root + "\\MIX");
	Make_Directory(Root + "\\Data");
	Make_Directory(Root + "\\User");

	return(SetCurrentDirectory(Root.c_str()) != 0);
}


void Remove_Root(void)
{
	SetCurrentDirectory(OriginalDirectory);

	// The tree is shallow and entirely this harness's own, so it is removed by name.
	char command[MAX_PATH + 32];
	std::snprintf(command, sizeof(command), "cmd /c rd /s /q \"%s\"", Root.c_str());
	system(command);
}

}
