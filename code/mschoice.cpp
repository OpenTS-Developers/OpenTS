/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "always.h"

#include "mschoice.h"

#include "_mixfile.h"
#include "addon.h"
#include "ccfile.h"
#include "ccini.h"
#include "data.h"
#include "dbgprint.h"
#include "dsaudio.h"
#include "globals.h"
#include "goptions.h"
#include "mixfile.h"

/// <summary>
/// Creates an empty map choice database.
/// The stage, animation and sound lists start out empty. Call Initialize to fill them
/// in from the map selection control file.
/// </summary>
MapChoice::MapChoice(void)
{
	Stages.Clear();
	AnimPaletteName = NULL;
	AnimEntries.Clear();
}


/// <summary>
/// Destroys the map choice database.
/// Everything the database loaded is released by way of Deinit.
/// </summary>
MapChoice::~MapChoice(void)
{
	Deinit();
}


/// <summary>
/// Loads the map selection data for the specified house.
/// This routine reads the map selection control file belonging to whichever addon is
/// required and builds the stages, background animations and sound effects that the
/// map selection screen presents. Any previously loaded data is discarded first.
/// </summary>
/// <param name="house_name">Name of the house whose campaign is being presented.</param>
/// <returns>bool; Was the map selection data loaded?</returns>
bool MapChoice::Initialize(char const * house_name)
{
	char buffer[256];
	char key[64];
	char section[64];
	int index;

	CCFileClass file;
	INIClass ini;

	Deinit();

	if (Get_Required_Addon() > ADDON_BASE_GAME) {
		sprintf(buffer, "MAPSEL%02d.INI", Get_Required_Addon());
		file.Open(buffer);
	} else {
		file.Open("MAPSEL.INI");
	}

	if (!ini.Load(file)) {
		DebugString("Failed to load MAPSEL.INI\n");
		return(false);
	}

	for (index = 1; index <= 100; index++) {

		sprintf(key, "%d", index);

		if (!ini.Get_String(house_name, key, NULL, buffer, sizeof(buffer))) {
			break;
		}

		MapStage * stage = new MapStage(ini, buffer);
		if (stage == NULL) {
			DebugString("MapSelect: Failed to create stage %s\n", buffer);
			return(false);
		}

		Stages.Add(stage);
	}

	if (Stages.Count() == 0) {
		DebugString("MapSelect: There isn't any stages!\n");
		return(false);
	}

	if (ini.Get_String(house_name, "Anims", NULL, section, sizeof(section)) > 0) {

		if (ini.Get_String(section, "TextRect", NULL, buffer, sizeof(buffer)) > 0) {

			char * token = strtok(buffer, ",");
			if (token) {
				TextRect.X = atoi(token);
			}

			token = strtok(NULL, ",");
			if (token) {
				TextRect.Y = atoi(token);
			}

			token = strtok(NULL, ",");
			if (token) {
				TextRect.Width = atoi(token);
			}

			token = strtok(NULL, ",");
			if (token) {
				TextRect.Height = atoi(token);
			}
		}

		if (ini.Get_String(section, "Palette", NULL, buffer, sizeof(buffer)) > 0) {
			AnimPaletteName = strdup(buffer);
		}

		for (index = 1; index <= 100; index++) {

			sprintf(key, "%d", index);

			if (!ini.Get_String(section, key, NULL, buffer, sizeof(buffer))) {
				break;
			}

			MSAnimEntry * anim = new MSAnimEntry(buffer);
			if (anim == NULL) {
				DebugString("Failed to create anim %s\n", buffer);
				return(false);
			}

			AnimEntries.Add(anim);
		}
	}

	if (AudioEngine.Is_Available()) {
		if (ini.Get_String(house_name, "Sounds", NULL, section, sizeof(section)) > 0) {
			for (index = 0; index < ini.Entry_Count(section); index++) {
				char const * entry = ini.Get_Entry(section, index);

				if (entry != NULL && ini.Get_String(section, entry, NULL, buffer, sizeof(buffer)) > 0) {

					MSSfxEntry * sfx = new MSSfxEntry(entry, buffer);

					if (sfx->Get_Name() == NULL) {
						delete sfx;
					} else {
						Add_Sound(sfx);
					}
				}
			}
		}
	}

	return(true);
}


/// <summary>
/// Discards the loaded map selection data.
/// This routine releases the stages, animations and sound effects and puts the
/// database back into the empty state it was constructed in. Initialize calls it
/// first, so it is safe to call more than once.
/// </summary>
void MapChoice::Deinit(void)
{
	int i;

	for (i = 0; i < SoundEntries.Count(); i++) {
		MSSfxEntry * sfx = SoundEntries[i];
		if (sfx != NULL) {
			delete sfx;
		}
	}
	SoundEntries.Clear();
	if (AnimPaletteName != NULL) {
		free((void *)AnimPaletteName);
	}
	AnimPaletteName = NULL;
	TextRect.Set(0,0,640,400);
	for (i = 0; i < AnimEntries.Count(); i++) {
		MSAnimEntry * anim = AnimEntries[i];
		if (anim != NULL) {
			delete anim;
		}
	}
	AnimEntries.Clear();
	for (i = 0; i < Stages.Count(); i++) {
		MapStage * stage = Stages[i];
		if (stage != NULL) {
			delete stage;
		}
	}
	Stages.Clear();
}


/// <summary>
/// Fetches the stage with the specified label.
/// The label is the control file section name, so the comparison ignores case and a
/// selection may spell its destination however it likes.
/// </summary>
/// <param name="name">Label of the stage to search for.</param>
/// <returns>Returns with a pointer to the stage. Otherwise, NULL is returned.</returns>
MapStage * MapChoice::Find_Stage_By_Name(char const * name)
{
	if (name != NULL) {
		for (int index = 0; index < Stages.Count(); index++) {
			if (stricmp(name, Stages[index]->Get_Stage_Label()) == 0) {
				return(Stages[index]);
			}
		}
	}
	return(NULL);
}


/// <summary>
/// Fetches the stage with the specified identifier.
/// This is the counterpart of Get_Stage_ID and is how the scenario record's remembered
/// stage is turned back into something the map selection screen can present.
/// </summary>
/// <returns>Returns with a pointer to the stage. Otherwise, NULL is returned.</returns>
MapStage * MapChoice::Find_Stage_By_ID(unsigned short index)
{
	if (Stages.Count() <= index) {
		return(NULL);
	}
	return(Stages[index]);
}


/// <summary>
/// Fetches the identifier of the specified stage.
/// The scenario record remembers which stage the player reached as a number rather
/// than as a pointer, so this is how a stage is turned into something storable.
/// </summary>
/// <returns>Returns with the stage identifier, or -1 if the stage is not one of ours.</returns>
short MapChoice::Get_Stage_ID(MapStage * stage)
{
	return(Stages.ID(stage));
}


/// <summary>
/// Fetches the sound effect registered under the specified name.
/// The map selection screen asks for its sounds by name rather than by file, so that
/// the control file may re-point them without the screen caring.
/// </summary>
/// <param name="name">Name of the sound effect as the control file spells it.</param>
/// <returns>Returns with a pointer to the sound effect. Otherwise, NULL is returned.</returns>
MSSfxEntry * MapChoice::Find_Sound(char const * name)
{
	if (!AudioEngine.Is_Available()) return(NULL);
	if (name == NULL) return(NULL);
	for (int i = 0; i < SoundEntries.Count(); i++) {
		MSSfxEntry * sfx = SoundEntries[i];
		if (stricmp(name, sfx->Get_Name()) == 0) {
			return(sfx);
		}
	}
	return(NULL);
}


/// <summary>
/// Creates a map selection stage from its control file section.
/// This routine gathers everything the map selection screen needs in order to present
/// one stage -- the scenario it launches, its description and voice over, the movie
/// and overlay artwork, the target markers, and the click map regions the player may
/// pick from.
/// </summary>
/// <param name="ini">The map selection control file to read the stage from.</param>
/// <param name="label">Name of the section that describes this stage.</param>
MapStage::MapStage(INIClass const & ini, char const * label) :
	Targets(),
	TextEntries(),
	Selections()
{
	char buffer[256];
	char key[64];
	int index;

	StageLabel = strdup(label);

	ScenarioName = NULL;
	if (ini.Get_String(label, "Scenario", NULL, buffer, sizeof(buffer)) > 0) {
		ScenarioName = strdup(buffer);
	}

	Description = NULL;
	if (ini.Get_String(label, "Description", NULL, buffer, sizeof(buffer)) > 0) {
		int txt = atol(buffer);
		if (txt == TXT_NONE && isalpha(buffer[0])) {
			if (strlen(buffer) != 0) {
				Description = (char *)malloc(1024);
				if (Description != NULL) {
					ini.Get_TextBlock(buffer, Description, 1024);
				}
			}
		} else {
			char const * desc = Fetch_String(txt);
			Description = strdup(desc);
		}
	}

	VoiceOver = NULL;
	if (ini.Get_String(label, "VoiceOver", NULL, buffer, sizeof(buffer)) > 0) {
		VoiceOver = strdup(buffer);
	}

	MapVQName = NULL;
	if (ini.Get_String(label, "MapVQ", NULL, buffer, sizeof(buffer)) > 0) {
		MapVQName = strdup(buffer);
	}

	OverlayCount = 0;
	memset(OverlayNames, 0, sizeof(OverlayNames));

	if (ini.Get_String(label, "Overlays", NULL, buffer, sizeof(buffer)) > 0) {
		index = 0;
		char * token = strtok(buffer, ",");
		while (token && index < ARRAY_SIZE(OverlayNames)) {
			OverlayNames[index] = strdup(token);
			token = strtok(NULL, ",");
			index++;
		}

		OverlayCount = index;
	}

	ClickMapName = NULL;
	if (ini.Get_String(label, "ClickMap", NULL, buffer, sizeof(buffer)) > 0) {
		ClickMapName = strdup(buffer);
	}

	for (index = 1; index < 8; index++) {
		sprintf(key, "Text%d", index);
		if (ini.Get_String(label, key, NULL, buffer, sizeof(buffer)) > 0) {
			MSTextEntry * text = new MSTextEntry(buffer);
			if (text) {
				TextEntries.Add(text);
			}
		}
	}

	if (ini.Get_String(label, "Targets", 0, buffer, sizeof(buffer)) > 0) {

		int count = 0;
		char * token = strtok(buffer, ",");
		if (token) {
			count = atoi(token);
		}

		for (index = 0; index < count; index++) {
			int x = -1;
			int y = -1;

			token = strtok(NULL, ",");
			if (token) {
				x = atoi(token);
			}

			token = strtok(NULL, ",");
			if (token) {
				y = atoi(token);
			}

			Targets.Add(Point2D(x, y));
		}
	}

	for (index = 0; index < 256; ++index) {
		sprintf(key, "%d", index);

		if (ini.Get_String(label, key, 0, buffer, sizeof(buffer)) > 0) {

			MapSelection* selection = new MapSelection();
			if (selection) {
				selection->Set_Index(index);
				selection->Set_Stage_Label(buffer);
				Selections.Add(selection);
			}
		}
	}
}


/// <summary>
/// Destroys the map selection stage.
/// The strings this stage copied out of the control file, along with its text entries
/// and its selections, are all released here.
/// </summary>
MapStage::~MapStage(void)
{
	int index;

	if (StageLabel != NULL) {
		free(StageLabel);
	}
	if (ScenarioName != NULL) {
		free(ScenarioName);
	}
	if (Description != NULL) {
		free(Description);
	}
	if (VoiceOver != NULL) {
		free(VoiceOver);
	}
	if (MapVQName != NULL) {
		free(MapVQName);
	}
	for (index = 0; index < 2; index++) {
		if (OverlayNames[index] != NULL) {
			free(OverlayNames[index]);
		}
	}
	if (ClickMapName != NULL) {
		free(ClickMapName);
	}
	for (index = 0; index < TextEntries.Count(); index++) {
		MSTextEntry * entry = TextEntries[index];
		if (entry != NULL) {
			delete entry;
		}
	}
	for (index = 0; index < Selections.Count(); index++) {
		MapSelection * selection = Selections[index];
		if (selection != NULL) {
			if (selection->Get_Stage_Label() != NULL) {
				free((char *)selection->Get_Stage_Label());
			}
			delete selection;
		}
	}
}


/// <summary>
/// Fetches the selection that leads to the named stage.
/// This routine is used to get at the target animation of a click map region once
/// Find_Selection has named the stage that region selects.
/// </summary>
/// <param name="name">Label of the stage the selection leads to.</param>
/// <returns>Returns with a pointer to the selection. Otherwise, NULL is returned.</returns>
MapSelection * MapStage::Find_Selection_By_Name(char const * name) const
{
	if (name == NULL) return(NULL);
	for (int i = 0; i < Selections.Count(); i++) {
		const char * selection = Selections[i]->Get_Stage_Label();
		if (selection != NULL) {
			if (stricmp(selection, name) == 0) {
				return(Selections[i]);
			}
		}
	}
	return(NULL);
}


/// <summary>
/// Determines which stage the specified click map region leads to.
/// The map selection screen reads a pixel out of the stage's click map artwork and
/// hands the color here to learn whether the player is pointing at something, and if
/// so, at what.
/// </summary>
/// <param name="index">Click map color of the region the player is pointing at.</param>
/// <returns>Returns with the label of the stage that region selects, or NULL if the region
/// is not selectable.</returns>
char * MapStage::Find_Selection_By_Index(int index) const
{
	for (int i = 0; i < Selections.Count(); i++) {
		if ((char)index == Selections[i]->Get_Index()) {
			return(Selections[i]->Get_Stage_Label());
		}
	}
	return(NULL);
}


/// <summary>
/// Creates a stage text entry from its control file line.
/// The line gives the screen position, the moment the text is due to appear, and the
/// text itself.
/// </summary>
/// <param name="string">The control file line to parse; it is chopped up in place.</param>
MSTextEntry::MSTextEntry(char * string) :
	XPos(0),
	YPos(0),
	StartTime(0),
	String(NULL)
{
	char * token;

	token = strtok(string, ",");
	if (token != NULL) {
		XPos = atoi(token);
	}
	token = strtok(NULL, ",");
	if (token != NULL) {
		YPos = atoi(token);
	}
	token = strtok(NULL, ",");
	if (token != NULL) {
		StartTime = atol(token);
	}
	token = strtok(NULL, ",");
	if (token != NULL) {
		String = strdup(token);
	}
}


/// <summary>
/// Creates a map selection animation entry from its control file line.
/// The line names the animation to play and gives the screen position and the rate to
/// play it at.
/// </summary>
/// <param name="string">The control file line to parse; it is chopped up in place.</param>
MSAnimEntry::MSAnimEntry(char * string) :
	Filename(NULL),
	XPos(0),
	YPos(0),
	Rate(0)
{
	char * token;

	token = strtok(string, ",");
	if (token != NULL) {
		Filename = strdup(token);
	}
	token = strtok(NULL, ",");
	if (token != NULL) {
		XPos = atoi(token);
	}
	token = strtok(NULL, ",");
	if (token != NULL) {
		YPos = atoi(token);
	}
	token = strtok(NULL, ",");
	if (token != NULL) {
		Rate = atoi(token);
	}
}


/// <summary>
/// Creates a map selection sound effect from its control file line.
/// The line names the sample file and the volume to play it at. The sample comes from
/// the mixfiles, or from a loose file if the mixfiles do not carry it. Nothing is
/// loaded when the audio system is unavailable or the volume works out to silence.
/// </summary>
/// <param name="name">Name the map selection screen will ask for this sound by.</param>
/// <param name="string">The control file line to parse; it is chopped up in place.</param>
/// <remarks>Check Get_Name for NULL to tell whether the sound effect is usable.</remarks>
MSSfxEntry::MSSfxEntry(char const * name, char * string) :
	Name(NULL),
	AllocLoaded(false),
	Sample(NULL),
	Volume(255)
{
	if (name != NULL && AudioEngine.Is_Available()) {

		char * file_name = strtok(string, ",");
		char * volume_str = strtok(NULL, ",");

		if (volume_str != NULL) {
			Volume = atoi(volume_str);
			if (Volume > 100) {
				Volume = 100;
			} else if (Volume < 0) {
				Volume = 0;
			}
			Volume = 255 * Volume / 100;
		}

		if (Volume > 0 && file_name != NULL) {

			Name = strdup(name);
			Sample = (void *)MFCD::Retrieve(file_name);

			if (Sample == NULL) {
				CCFileClass file;
				file.Open(file_name);
				Sample = Load_Alloc_Data(file);
				AllocLoaded = true;
				DebugString("MSSfxEntry: AllocLoaded %s\n", file_name);
			}
		}
	}
}


/// <summary>
/// Destroys the sound effect.
/// Any playback in progress is stopped first, and the sample data is released only if
/// this entry is the one that loaded it.
/// </summary>
MSSfxEntry::~MSSfxEntry(void)
{
	if (Sample != NULL) {
		AudioEngine.Stop_Sample_Playing(Sample);
		AudioEngine.Release_Sample(Sample);
		if (AllocLoaded == true) {
			delete Sample;
		}
	}

	if (Name != NULL) {
		free(Name);
	}
}


/// <summary>
/// Plays the sound effect.
/// The volume this entry was configured with is scaled by the player's sound volume
/// option before the sample is handed to the audio system.
/// </summary>
void MSSfxEntry::Play(void)
{
	if (Sample != NULL) {
		AudioEngine.Play_Sample(Sample, AUDIO_GROUP_SFX, (float)Volume / 255.0f, 255);
	}
}
