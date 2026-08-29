/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// Exercises the game directories without the engine or any game data: the folder list a
// deployment configures, the scan that covers every folder, and where a player's own files
// are read from and written to. Every file this uses is one the harness makes itself.

#include <windows.h>

#include <cstdio>
#include <string>
#include <vector>

#include "cdfile.h"
#include "gamedirs.h"
#include "rawfile.h"

namespace {

int Failures = 0;

std::string Root;
char OriginalDirectory[MAX_PATH];


void Check(bool condition, char const * what)
{
	std::printf("%-62s %s\n", what, condition ? "ok" : "FAILED");

	if (!condition) {
		Failures++;
	}
}


void Check_List(std::vector<std::string> const & actual, std::vector<std::string> const & expected, char const * what)
{
	bool same = actual.size() == expected.size();

	for (unsigned int index = 0; same && index < actual.size(); index++) {
		same = actual[index] == expected[index];
	}

	Check(same, what);

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


void Test_Parsing(void)
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


void Test_Defaults(void)
{
	Reset();
	Init_Search_Folders();

	Check(CDFileClass::Search_Path(0) != NULL && std::string(CDFileClass::Search_Path(0)) == "INI\\",
		"with no configuration the INI folder is searched");
	Check(CDFileClass::Search_Path(1) != NULL && std::string(CDFileClass::Search_Path(1)) == "MIX\\",
		"with no configuration the MIX folder is searched");
	Check(CDFileClass::Search_Path(2) == NULL, "nothing else is searched");
}


void Test_Configured_Folders(void)
{
	Reset();
	Write_File(Root + "\\OPENTS.INI", "[Paths]\nSearchPaths=Data,More\n");
	Init_Search_Folders();

	Check(CDFileClass::Search_Path(0) != NULL && std::string(CDFileClass::Search_Path(0)) == "Data\\",
		"a configured folder is searched");
	Check(CDFileClass::Search_Path(1) != NULL && std::string(CDFileClass::Search_Path(1)) == "More\\",
		"configured folders keep the order they are written in");
	Check(CDFileClass::Search_Path(2) == NULL, "a configured list replaces the default folders");

	/*
	 * A file cannot carry an entry with nothing after the equals sign -- the reader passes
	 * such a line over -- so naming the game's own directory is how a deployment asks for
	 * no other folder.
	 */
	Reset();
	Write_File(Root + "\\OPENTS.INI", "[Paths]\nSearchPaths=.\n");
	Init_Search_Folders();

	Check(CDFileClass::Search_Path(0) == NULL, "naming only the game's own directory turns the default folders off");

	DeleteFile((Root + "\\OPENTS.INI").c_str());
}


void Test_Configuration_In_A_Folder(void)
{
	Reset();
	Write_File(Root + "\\INI\\OPENTS.INI", "[Paths]\nSearchPaths=FromIni\n");
	Init_Search_Folders();

	Check(CDFileClass::Search_Path(0) != NULL && std::string(CDFileClass::Search_Path(0)) == "FromIni\\",
		"the configuration is found in a sorted deployment's own INI folder");

	DeleteFile((Root + "\\INI\\OPENTS.INI").c_str());
}


void Test_Data_Directory(void)
{
	Reset();
	Set_Data_Directory((Root + "\\Data").c_str());
	Write_File(Root + "\\Data\\OPENTS.INI", "[Paths]\nSearchPaths=Sorted\n");


	Check(Apply_Game_Directories(), "a data directory that exists is accepted");
	Init_Search_Folders();

	std::string const expected_data = Root + "\\Data\\";
	std::string const expected_sorted = expected_data + "Sorted\\";

	Check(CDFileClass::Search_Path(0) != NULL && std::string(CDFileClass::Search_Path(0)) == expected_data,
		"the data directory itself is searched");
	Check(CDFileClass::Search_Path(1) != NULL && std::string(CDFileClass::Search_Path(1)) == expected_sorted,
		"a folder it configures is searched inside it");

	DeleteFile((Root + "\\Data\\OPENTS.INI").c_str());

	Reset();
	Set_Data_Directory((Root + "\\Missing").c_str());
	Check(!Apply_Game_Directories(), "a data directory that is not there is refused");
}


void Test_User_Directory(void)
{
	Reset();
	Set_User_Directory((Root + "\\User\\Fresh").c_str());

	Check(Apply_Game_Directories(), "a user directory is created when it is not there yet");
	Check(GetFileAttributes((Root + "\\User\\Fresh").c_str()) != INVALID_FILE_ATTRIBUTES,
		"the created user directory is on the disk");

	std::string const expected_user = Root + "\\User\\Fresh\\";
	Check(CDFileClass::User_Path() != NULL && std::string(CDFileClass::User_Path()) == expected_user,
		"the file layer is told where the player's own files go");
	Check(CDFileClass::Search_Path(0) == NULL,
		"the user directory is not one of the searched folders");

	Check(User_File_Write_Name("SUN.INI") == expected_user + "SUN.INI",
		"a file the game writes goes to the user directory");

	Reset();
	Check(CDFileClass::User_Path() == NULL,
		"with no user directory the file layer has none either");
	Check(User_File_Write_Name("SUN.INI") == "SUN.INI",
		"with no user directory a written file keeps its plain name");
}


void Test_Search_Files(void)
{
	Reset();

	Write_File(Root + "\\ALPHA.MPR", "");
	Write_File(Root + "\\INI\\BRAVO.MPR", "");
	Write_File(Root + "\\INI\\ALPHA.MPR", "");
	Write_File(Root + "\\MIX\\CHARLIE.MPR", "");

	Init_Search_Folders();

	Check_List(Search_Files("*.MPR"), {"ALPHA.MPR", "BRAVO.MPR", "CHARLIE.MPR"},
		"a scan covers every folder, reports a name once, and sorts it");

	/*
	 * The scan and an ordinary open have to agree, or the game would list one file and load
	 * another. Both walk the folders in the same order, so the game's own copy wins.
	 */
	CDFileClass found("ALPHA.MPR");
	Check(std::string(found.File_Name()) == "ALPHA.MPR",
		"opening a name the scan reported lands on the copy the scan saw");

	CDFileClass sorted("CHARLIE.MPR");
	Check(std::string(sorted.File_Name()) == "MIX\\CHARLIE.MPR",
		"a name held only by a searched folder opens from that folder");
}


void Test_Writes_Do_Not_Search(void)
{
	Reset();
	Write_File(Root + "\\MIX\\WRITTEN.DAT", "shipped");
	Init_Search_Folders();

	/*
	 * A file opened for writing must never be looked for anywhere but the current directory:
	 * a deployment's folders are read from, not written to.
	 */
	CDFileClass file;
	file.Open("WRITTEN.DAT", FileClass::READ|FileClass::WRITE);
	std::string const written = file.File_Name();
	file.Close();

	Check(written == "WRITTEN.DAT", "a read-write open does not settle on a searched folder");

	Check(GetFileAttributes((Root + "\\WRITTEN.DAT").c_str()) != INVALID_FILE_ATTRIBUTES,
		"the written file is in the current directory");

	WIN32_FILE_ATTRIBUTE_DATA shipped;
	GetFileAttributesEx((Root + "\\MIX\\WRITTEN.DAT").c_str(), GetFileExInfoStandard, &shipped);
	Check(shipped.nFileSizeLow == 7, "the copy in the searched folder is untouched");
}


/*
 * A file named with a directory of its own, looked up while folders are searched, makes the
 * search build a pathname out of two full paths. That pair does not have to fit in one, and
 * once it does not the search has to pass it over rather than build it anyway.
 */
void Test_Long_Names(void)
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
	Check(std::string(file.File_Name()) == absolute,
		"a file named with its own directory is found while long folders are searched");

	CDFileClass missing((Root + "\\NOTHERE.MPR").c_str());
	Check(std::string(missing.File_Name()) == Root + "\\NOTHERE.MPR",
		"a name that no folder holds comes back as it was given");

	Check_List(Search_Files("*.MPR"), {"ALPHA.MPR"}, "a scan passes over a folder it cannot build a name in");
}


/*
 * The file layer places what the game writes and finds what it reads. These are the rules a
 * caller never states, so they are checked here rather than at any one of them.
 */
void Test_The_File_Layer_Places_Written_Files(void)
{
	Reset();
	Set_User_Directory((Root + "\\User\\Own").c_str());
	Apply_Game_Directories();
	Init_Search_Folders();

	std::string const own = Root + "\\User\\Own\\";

	CDFileClass written("OWN.DAT");
	written.Open(FileClass::WRITE);
	written.Write("mine", 4);
	written.Close();

	Check(std::string(written.File_Name()) == own + "OWN.DAT", "a written file is named in the user directory");
	Check(File_Exists(own + "OWN.DAT"), "a written file is in the user directory");
	Check(!File_Exists(Root + "\\OWN.DAT"), "a written file is not beside the game");

	/*
	 * A second object, made once both copies exist, so that what answers is the search and
	 * not the object that did the writing.
	 */
	Write_File(Root + "\\MIX\\SHARED.DAT", "shipped");
	Write_File(own + "SHARED.DAT", "own");

	CDFileClass shared("SHARED.DAT");
	Check(Read_File(shared.File_Name()) == "own", "a read prefers the player's own copy");

	// A created file is read back from where it was created, which is what a game storing
	// its progress does every time it starts.
	CDFileClass progress("PROGRESS.INI");
	Check(!progress.Is_Available(), "a file the player has never had is not there yet");
	progress.Create();
	progress.Close();

	CDFileClass reopened("PROGRESS.INI");
	Check(reopened.Is_Available(), "a created file is found again");
	Check(std::string(reopened.File_Name()) == own + "PROGRESS.INI", "a created file is found in the user directory");
}


void Test_The_File_Layer_Deletes_Only_The_Player_Copy(void)
{
	Reset();
	Set_User_Directory((Root + "\\User\\Own").c_str());
	Apply_Game_Directories();
	Init_Search_Folders();

	std::string const own = Root + "\\User\\Own\\";

	Write_File(Root + "\\MIX\\GONE.DAT", "shipped");
	Write_File(own + "GONE.DAT", "own");

	CDFileClass discard("GONE.DAT");
	discard.Delete();

	Check(!File_Exists(own + "GONE.DAT"), "the player's own copy is thrown away");
	Check(File_Exists(Root + "\\MIX\\GONE.DAT"), "the copy a deployment ships is left alone");

	CDFileClass again("GONE.DAT");
	Check(Read_File(again.File_Name()) == "shipped", "what a deployment ships answers once the player's copy is gone");
}


void Test_A_Name_With_A_Directory_Is_Left_Alone(void)
{
	Reset();
	Set_User_Directory((Root + "\\User\\Own").c_str());
	Apply_Game_Directories();

	CDFileClass rooted("MIX\\ROOTED.DAT");
	rooted.Open(FileClass::WRITE);
	rooted.Write("here", 4);
	rooted.Close();

	Check(std::string(rooted.File_Name()) == "MIX\\ROOTED.DAT", "a name with a directory keeps it");
	Check(File_Exists(Root + "\\MIX\\ROOTED.DAT"), "a name with a directory is written where it says");
	Check(!File_Exists(Root + "\\User\\Own\\ROOTED.DAT"), "a name with a directory is not moved");
}


void Test_Placing_A_File_Is_Repeatable(void)
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

	Check(std::string(file.File_Name()) == once, "opening a file for writing twice names it the same place");

	// The buffered path opens the file a second time itself, with read access added.
	CDFileClass buffered("BUFFERED.DAT");
	buffered.Cache(1024);
	buffered.Open(FileClass::WRITE);
	buffered.Write("cached", 6);
	buffered.Close();

	Check(File_Exists(Root + "\\User\\Own\\BUFFERED.DAT"), "a buffered write lands in the user directory");
}


void Test_Without_A_User_Directory_Nothing_Moves(void)
{
	Reset();
	Init_Search_Folders();

	Write_File(Root + "\\MIX\\STILL.DAT", "shipped");

	CDFileClass written("STILL.DAT");
	written.Open(FileClass::WRITE);
	written.Write("here", 4);
	written.Close();

	Check(std::string(written.File_Name()) == "STILL.DAT", "a written file keeps its plain name");
	Check(File_Exists(Root + "\\STILL.DAT"), "a written file lands beside the game");
	Check(Read_File(Root + "\\MIX\\STILL.DAT") == "shipped", "a searched folder's copy is untouched");

	CDFileClass discard("STILL.DAT");
	discard.Delete();

	Check(!File_Exists(Root + "\\STILL.DAT"), "a delete takes the copy beside the game");
	Check(File_Exists(Root + "\\MIX\\STILL.DAT"), "a delete leaves the searched folder's copy");

	CDFileClass shipped("STILL.DAT");
	Check(std::string(shipped.File_Name()) == "MIX\\STILL.DAT", "a read still falls through to the searched folders");
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


int main(void)
{
	GetCurrentDirectory(sizeof(OriginalDirectory), OriginalDirectory);

	if (!Make_Root()) {
		std::printf("could not create the working directory\n");
		return(1);
	}

	std::printf("Working in %s\n\n", Root.c_str());

	Test_Parsing();
	Test_Defaults();
	Test_Configured_Folders();
	Test_Configuration_In_A_Folder();
	Test_Data_Directory();
	Test_User_Directory();
	Test_Search_Files();
	Test_Writes_Do_Not_Search();
	Test_Long_Names();
	Test_The_File_Layer_Places_Written_Files();
	Test_The_File_Layer_Deletes_Only_The_Player_Copy();
	Test_A_Name_With_A_Directory_Is_Left_Alone();
	Test_Placing_A_File_Is_Repeatable();
	Test_Without_A_User_Directory_Nothing_Moves();

	Reset();
	Remove_Root();

	std::printf("\n%s\n", Failures == 0 ? "All checks passed." : "There were failures.");
	return(Failures == 0 ? 0 : 1);
}
