/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "always.h"

#include "gamedirs.h"

#include "cdfile.h"
#include "dbgprint.h"
#include "ini.h"
#include "rawfile.h"

#include <algorithm>

/*
 * The directories the command line named. Empty means the game's own directory, so an
 * installation that names neither resolves every file exactly as it always did.
 */
static std::string DataDirectory;
static std::string UserDirectory;

/*
 * The folders a deployment's files are looked for in when no configuration names any. A
 * configuration's list replaces this rather than adding to it.
 */
static char const * const DefaultSearchFolders = "INI,MIX,Maps";

static char const * const ConfigName = "OPENTS.INI";

/*
 * The folder saved games are kept in, under whichever directory the player's own files go.
 */
static char const * const SavedGamesFolder = "Saved Games";

/*
 * The folders the configuration itself is looked for in, relative to the data directory.
 */
static char const * const ConfigProbes[] = {"", "INI\\", "MIX\\"};


static std::string Trim_Path(std::string const & path)
{
	std::string::size_type first = path.find_first_not_of(" \t");
	if (first == std::string::npos) {
		return(std::string());
	}

	std::string::size_type last = path.find_last_not_of(" \t");
	return(path.substr(first, last - first + 1));
}


/// <summary>
/// Puts a directory in the form a file name can simply be appended to, which is what the
/// search chain has always expected of one.
/// </summary>
/// <param name="path">The directory to terminate.</param>
/// <returns>The directory, ending in a separator.</returns>
static std::string Terminate_Path(std::string const & path)
{
	if (path.empty()) {
		return(path);
	}

	switch (path[path.length() - 1]) {
		case '\\':
		case '/':
		case ':':
			return(path);

		default:
			return(path + '\\');
	}
}


static bool Is_Same_Path(std::string const & left, std::string const & right)
{
	return(_stricmp(left.c_str(), right.c_str()) == 0);
}


static bool Is_Registered(std::string const & path)
{
	for (int index = 0; ; index++) {
		char const * registered = CDFileClass::Search_Path(index);
		if (registered == NULL) {
			return(false);
		}

		if (Is_Same_Path(registered, path)) {
			return(true);
		}
	}
}


/// <summary>
/// Reports where a deployment's files are, which is the data directory when one is named
/// and the game's own directory otherwise. Everything a configuration names is relative
/// to it.
/// </summary>
/// <returns>The directory a deployment's files are kept in.</returns>
static std::string Data_Home(void)
{
	return(DataDirectory);
}


/*
 * What went wrong with the directories, kept for whoever is in a position to tell the
 * player. Reporting it is not this module's business, since it has no window to report in.
 */
static std::string DirectoryError;


static void Report_Directory_Error(char const * what, std::string const & path)
{
	char message[MAX_PATH + 128];

	sprintf(message, "The %s directory cannot be used:\n\n%s", what, path.c_str());
	DirectoryError = message;

	DebugString("[GameDirs] %s directory unusable: %s.\n", what, path.c_str());
	printf("The %s directory cannot be used: %s\n", what, path.c_str());
}


char const * Game_Directory_Error(void)
{
	return(DirectoryError.c_str());
}


static bool Is_Directory(std::string const & path)
{
	DWORD attributes = GetFileAttributes(path.c_str());

	return(attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0);
}


void Set_Data_Directory(char const * path)
{
	DataDirectory = Terminate_Path(Trim_Path(path != NULL ? path : ""));
}


void Set_User_Directory(char const * path)
{
	UserDirectory = Terminate_Path(Trim_Path(path != NULL ? path : ""));

	// The file layer places and finds the player's own files; this is the only thing that
	// tells it where they go.
	CDFileClass::Set_User_Path(UserDirectory.c_str());
}


/// <summary>
/// Splits a configured folder list into the folders it names.
/// Folders are separated by commas, since a semicolon opens a comment in the file the list
/// is written in. They are returned in the order written, with the whitespace around them
/// dropped and a trailing separator supplied, and a folder named twice is kept once.
/// </summary>
/// <param name="list">The comma separated list of folders.</param>
/// <returns>The folders named, in the order they were written.</returns>
std::vector<std::string> Parse_Search_Folders(char const * list)
{
	std::vector<std::string> folders;

	if (list == NULL) {
		return(folders);
	}

	std::string const text = list;
	std::string::size_type start = 0;

	while (start <= text.length()) {
		std::string::size_type end = text.find(',', start);
		if (end == std::string::npos) {
			end = text.length();
		}

		/*
		 * Comparing the folders only once they are in the form they will be searched in
		 * keeps the same folder written two ways from being searched twice.
		 */
		std::string const folder = Terminate_Path(Trim_Path(text.substr(start, end - start)));

		/*
		 * The game's own directory is examined before any of these, so naming it adds
		 * nothing. Naming only it is how a deployment asks for no other folder, an entry
		 * with nothing after the equals sign being one an INI file cannot carry.
		 */
		if (folder == ".\\" || folder == "./") {
			if (end == text.length()) {
				break;
			}
			start = end + 1;
			continue;
		}

		if (!folder.empty()) {
			bool present = false;
			for (std::string const & existing : folders) {
				if (Is_Same_Path(existing, folder)) {
					present = true;
					break;
				}
			}

			if (!present) {
				folders.push_back(folder);
			}
		}

		if (end == text.length()) {
			break;
		}
		start = end + 1;
	}

	return(folders);
}


/// <summary>
/// Makes the directories the command line named usable.
/// The user directory is created when it is not there yet, because it is the game's own to
/// write. A named data directory must already exist, a missing one being reported here
/// rather than as the missing files it would become later.
/// </summary>
/// <returns>bool; Can the game run with the directories it was given?</returns>
bool Apply_Game_Directories(void)
{
	if (!UserDirectory.empty()) {
		if (!Is_Directory(UserDirectory) && !CreateDirectory(UserDirectory.c_str(), NULL)) {
			Report_Directory_Error("user", UserDirectory);
			return(false);
		}

		DebugString("[GameDirs] User directory is %s.\n", UserDirectory.c_str());
	}

	if (!DataDirectory.empty()) {
		if (!Is_Directory(DataDirectory)) {
			Report_Directory_Error("data", DataDirectory);
			return(false);
		}

		CDFileClass::Add_Search_Drive(DataDirectory.c_str());
		DebugString("[GameDirs] Data directory is %s.\n", DataDirectory.c_str());
	}

	return(true);
}


/// <summary>
/// Reads the deployment's configuration and installs the folders it searches.
/// The file is read from the disk rather than through the game's file system, so a
/// deployment cannot hide the description of its own layout inside an archive.
/// </summary>
void Init_Search_Folders(void)
{
	std::string const home = Data_Home();
	std::string list = DefaultSearchFolders;

	for (char const * probe : ConfigProbes) {
		std::string const name = home + probe + ConfigName;
		RawFileClass file(name.c_str());

		if (!file.Is_Available()) {
			continue;
		}

		INIClass ini;
		ini.Load(file);

		if (ini.Is_Present("Paths", "SearchPaths")) {
			char buffer[2048];

			int length = ini.Get_String("Paths", "SearchPaths", "", buffer, sizeof(buffer));
			if (length >= (int)sizeof(buffer) - 1) {
				DebugString("[GameDirs] %s names more folders than %s can hold.\n", name.c_str(), "SearchPaths");
			}

			list = buffer;
		}

		DebugString("[GameDirs] Read %s.\n", name.c_str());
		break;
	}

	for (std::string const & folder : Parse_Search_Folders(list.c_str())) {
		std::string const path = home + folder;

		if (!Is_Registered(path)) {
			CDFileClass::Add_Search_Drive(path.c_str());
			DebugString("[GameDirs] Searching %s.\n", path.c_str());
		}
	}
}


std::string User_File_Write_Name(char const * filename)
{
	if (UserDirectory.empty()) {
		return(filename);
	}

	return(UserDirectory + filename);
}


/// <summary>
/// Names a saved game inside the folder they are kept in. The folder is not one of the
/// searched ones and is created here, so a launcher can browse it before the first save is
/// written.
/// </summary>
/// <returns>The name to open, delete or scan for.</returns>
std::string Saved_Game_Name(char const * filename)
{
	std::string const folder = UserDirectory + SavedGamesFolder;

	CreateDirectory(folder.c_str(), NULL);

	return(folder + '\\' + filename);
}


static void Scan_Folder(char const * prefix, char const * pattern, std::vector<std::string> & names)
{
	std::string const search = std::string(prefix) + pattern;

	WIN32_FIND_DATA block;
	HANDLE handle = FindFirstFile(search.c_str(), &block);
	if (handle == INVALID_HANDLE_VALUE) {
		return;
	}

	do {
		if ((block.dwFileAttributes & (FILE_ATTRIBUTE_DIRECTORY|FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM|FILE_ATTRIBUTE_TEMPORARY)) != 0) {
			continue;
		}

		bool present = false;
		for (std::string const & existing : names) {
			if (Is_Same_Path(existing, block.cFileName)) {
				present = true;
				break;
			}
		}

		if (!present) {
			names.push_back(block.cFileName);
		}
	} while (FindNextFile(handle, &block));

	FindClose(handle);
}


/// <summary>
/// Finds the files matching a pattern in every directory the game reads from.
/// A name held by more than one directory is reported once, and opening that name afterwards
/// lands on the same file this scan saw, because both walk the directories in the same order.
/// The names come back sorted, so what the game makes of them does not depend on the order
/// a file system happened to hand them over in.
/// </summary>
/// <param name="pattern">The wildcard pattern to match, with no directory attached.</param>
/// <returns>The matching file names, without the directory they were found in.</returns>
std::vector<std::string> Search_Files(char const * pattern)
{
	std::vector<std::string> names;

	/*
	 * Asked of the file layer rather than kept here, so that a scan and an open are reading
	 * the very same directory.
	 */
	char const * user = CDFileClass::User_Path();
	if (user != NULL) {
		Scan_Folder(user, pattern, names);
	}

	Scan_Folder("", pattern, names);

	for (int index = 0; ; index++) {
		char const * path = CDFileClass::Search_Path(index);
		if (path == NULL) {
			break;
		}

		Scan_Folder(path, pattern, names);
	}

	std::sort(names.begin(), names.end(), [](std::string const & left, std::string const & right) {
		return(_stricmp(left.c_str(), right.c_str()) < 0);
	});

	return(names);
}
