/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "always.h"

#include "_mixfile.h"
#include "ccfile.h"
#include "ccrand.h"
#include "data.h"
#include "dsaudio.h"
#include "globals.h"
#include "goptions.h"
#include "ini.h"
#include "mixfile.h"
#include "wdtnet.h"

using namespace WorldDominationTour;


bool WDT_Make_Sound_Filename(int side, int index, char * buffer, int bufsize);
bool WDT_Random_Pick_Sound_Filename(int side, VOICEINDEX_LIST & list, char * buffer, int bufsize);
void Parse_Index_List(char * buffer, VOICEINDEX_LIST & list);


/// <summary>
/// Constructs the voice player that speaks for one side.
/// The voice-over categories are given the names they are known by in the INI database
/// so that they can be read; the lists themselves are fetched later by Init.
/// </summary>
/// <param name="side">The side whose voice-overs this player will speak.</param>
Voices::Voices(int side) :
	Side(side),
	IsIdle(true),
	QueuedSampleCount(0)
{
	VoiceCategories[VOICECAT_OLD_CYCLE].Set_Name("OldCycle");
	VoiceCategories[VOICECAT_HISTORY].Set_Name("History");
	VoiceCategories[VOICECAT_STATUS].Set_Name("Status");
	VoiceCategories[VOICECAT_TERRITORY].Set_Name("Territory");
	VoiceCategories[VOICECAT_GAME].Set_Name("Game");
	VoiceCategories[VOICECAT_EVACUATE].Set_Name("Evacuate");
}


/// <summary>
/// Destroys the voice player.
/// Anything still queued or playing is silenced and thrown away.
/// </summary>
Voices::~Voices(void)
{
	Cleanup();
}


/// <summary>
/// Reads every voice-over list this player will need.
/// The World Domination Tour screens call this routine once, as they are being set up,
/// with the INI section that holds the tour's voice-over lists.
/// </summary>
/// <param name="section">The INI section holding the voice-over lists.</param>
void Voices::Init(INIClass const & ini, const char * section)
{
	for (int i = 0; i < VOICECAT_COUNT; i++) {
		VoiceCategories[i].Read(ini, section, Side);
	}
	Init_Standalone(ini, section, VOICE_STARTUP, "Startup");
	Init_Standalone(ini, section, VOICE_TERRITORY_SELECT, "TerritorySelect");
}


/// <summary>
/// Reads the voice-over list for one standalone announcement.
/// The INI entry is named for the side and the announcement, so each side gets its own
/// set of lines for the same moment.
/// </summary>
/// <param name="section">The INI section holding the voice-over lists.</param>
/// <param name="voice_name">The announcement's name as it appears in the INI entry.</param>
void Voices::Init_Standalone(INIClass const & ini, const char * section, StandaloneVoiceType voice, const char * voice_name)
{
	char buffer[32];
	char entry[64];

	strcpy(entry, Side == 3 ? "NOD" : "GDI");
	strncat(entry, voice_name, sizeof(entry));
	ini.Get_String(section, entry, "", buffer, sizeof(buffer));
	Parse_Index_List(buffer, StandaloneVoices[voice]);
}


/// <summary>
/// Picks the filename of an ordinary voice-over for this player's side.
/// </summary>
/// <param name="buffer">Buffer to fill in with the sound filename.</param>
/// <param name="bufsize">Size of the destination buffer.</param>
/// <returns>bool; Was a line available to pick?</returns>
bool Voices::Pick_Standard_Voice(VoiceCategoryType category, Outcome outcome, char *buffer, int bufsize)
{
	return(VoiceCategories[category].Pick_Standard_Sound(Side, outcome, buffer, bufsize));
}


/// <summary>
/// Picks the filename of an emphatic voice-over for this player's side.
/// </summary>
/// <param name="buffer">Buffer to fill in with the sound filename.</param>
/// <param name="bufsize">Size of the destination buffer.</param>
/// <returns>bool; Was an emphatic line available to pick?</returns>
bool Voices::Pick_Emphasis_Voice(VoiceCategoryType category, Outcome outcome, char *buffer, int bufsize)
{
	return(VoiceCategories[category].Pick_Emphasis_Sound(Side, outcome, buffer, bufsize));
}


/// <summary>
/// Picks the filename of a standalone voice-over.
/// </summary>
/// <param name="buffer">Buffer to fill in with the sound filename.</param>
/// <param name="bufsize">Size of the destination buffer.</param>
/// <returns>bool; Was a voice-over available to pick?</returns>
bool Voices::Pick_Standalone_Voice(StandaloneVoiceType voice, char * buffer, int bufsize)
{
	return(WDT_Random_Pick_Sound_Filename(Side, StandaloneVoices[voice], buffer, bufsize));
}


/// <summary>
/// Picks a voice-over at random from a list and builds its filename.
/// This is the low level routine behind every voice-over pick. An empty list is not an
/// error -- it just means that side has nothing to say in that situation.
/// </summary>
/// <param name="side">The side whose voice-over set to draw from.</param>
/// <param name="list">The voice-over numbers to choose between.</param>
/// <param name="buffer">Buffer to fill in with the sound filename. It is emptied when
/// no pick can be made.</param>
/// <param name="bufsize">Size of the destination buffer.</param>
/// <returns>bool; Was a voice-over picked?</returns>
bool WDT_Random_Pick_Sound_Filename(int side, VOICEINDEX_LIST & list, char * buffer, int bufsize)
{
	*buffer = '\0';

	int count = list.Count();
	if (count <= 0) {
		return(false);
	}
	return(WDT_Make_Sound_Filename(side, list[count == 1 ? 0 : Sim_Random_Pick(0, count - 1)], buffer, bufsize));
}


/// <summary>
/// Builds the sound filename for a voice-over number.
/// The two sides keep their voice-overs in separate numbered sets, so the side decides
/// which of the two naming patterns the number is plugged into.
/// </summary>
/// <param name="side">The side whose voice-over set the number belongs to.</param>
/// <param name="index">The voice-over number within that set.</param>
/// <param name="buffer">Buffer to fill in with the sound filename.</param>
/// <returns>bool; Was a filename built?</returns>
/// <remarks>Be sure the destination buffer is big enough to hold a sound filename.</remarks>
bool WDT_Make_Sound_Filename(int side, int index, char * buffer, int bufsize)
{
	sprintf(buffer, side == 3 ? "01-W%03d.v01" : "00-W%03d.v00", index);
	return(true);
}


/// <summary>
/// Converts a comma separated INI value into a list of voice-over numbers.
/// This routine is how every voice-over list in the tour's INI section is turned into
/// something the voice player can pick from.
/// </summary>
/// <param name="buffer">The comma separated text to parse. It is chopped up in place.</param>
/// <param name="list">The list to fill in with the numbers found. Any previous contents
/// are thrown away.</param>
void Parse_Index_List(char * buffer, VOICEINDEX_LIST & list)
{
	list.Clear();
	char *tok = strtok(buffer, ",");
	while (tok != NULL) {
		list.Add((unsigned int)atoi(tok));

		tok = strtok(0, ",");
	}
}


/// <summary>
/// Constructs an empty voice-over category.
/// The category stays nameless and silent until it has been named and read from the
/// INI database.
/// </summary>
Voices::VoiceCategory::VoiceCategory(void)
{
	Name[0] = '\0';
}


/// <summary>
/// Sets the name this category is known by.
/// The name forms part of the INI entry that each of the category's voice-over lists is
/// read from, so it must be set before the category is read.
/// </summary>
/// <param name="name">The category name as it appears in the INI entry.</param>
void Voices::VoiceCategory::Set_Name(const char * name)
{
	strncpy(Name, name, sizeof(Name));
}


/// <summary>
/// Reads this category's voice-over lists from the INI database.
/// Every outcome the category can announce -- win, lose and draw -- is read for the
/// side specified.
/// </summary>
/// <param name="section">The INI section holding the voice-over lists.</param>
/// <param name="side">The side whose voice-overs should be read.</param>
void Voices::VoiceCategory::Read(INIClass const & ini, const char * section, int side)
{
	Read_Voiceover(ini, section, side, OUTCOME_WIN, "Win");
	Read_Voiceover(ini, section, side, OUTCOME_LOSE, "Lose");
	Read_Voiceover(ini, section, side, OUTCOME_DRAW, "Draw");
}


/// <summary>
/// Picks the ordinary voice-over for this category and outcome.
/// One of the lines the side has recorded for the occasion is chosen at random, so the
/// same announcement is not heard word for word every time.
/// </summary>
/// <param name="side">The side whose voice-over set to draw from.</param>
/// <param name="buffer">Buffer to fill in with the sound filename.</param>
/// <param name="bufsize">Size of the destination buffer.</param>
/// <returns>bool; Was there a line to pick?</returns>
bool Voices::VoiceCategory::Pick_Standard_Sound(int side, Outcome outcome, char * buffer, int bufsize)
{
	return(WDT_Random_Pick_Sound_Filename(side, VoiceIndexes[outcome][0], buffer, bufsize));
}


/// <summary>
/// Picks an emphatic voice-over for this category and outcome.
/// This is the stronger line that follows the ordinary announcement when the moment
/// calls for one.
/// </summary>
/// <param name="side">The side whose voice-over set to draw from.</param>
/// <param name="buffer">Buffer to fill in with the sound filename.</param>
/// <param name="bufsize">Size of the destination buffer.</param>
/// <returns>bool; Was there an emphatic line to pick?</returns>
bool Voices::VoiceCategory::Pick_Emphasis_Sound(int side, Outcome outcome, char * buffer, int bufsize)
{
	return(WDT_Random_Pick_Sound_Filename(side, VoiceIndexes[outcome][1], buffer, bufsize));
}


/// <summary>
/// Reads the voice-over lists for one outcome of this category.
/// The INI entry is named for the side, the category and the outcome, and a companion
/// entry with an "Emphasis" suffix supplies the more emphatic lines. A side that has
/// nothing to say about this outcome simply leaves both entries out.
/// </summary>
/// <param name="section">The INI section holding the voice-over lists.</param>
/// <param name="side">The side whose voice-overs are being read.</param>
/// <param name="outcome_string">The outcome's name as it appears in the INI entry.</param>
void Voices::VoiceCategory::Read_Voiceover(INIClass const & ini, const char * section, int side, Outcome outcome, const char * outcome_string)
{
	char entry[64];
	char buffer[32];

	strcpy(entry, side == 3 ? "NOD" : "GDI");
	strncat(entry, Name, sizeof(entry));
	strncat(entry, outcome_string, sizeof(entry));

	ini.Get_String(section, entry, "", buffer, sizeof(buffer));
	Parse_Index_List(buffer, VoiceIndexes[outcome][0]);

	strncat(entry, "Emphasis", sizeof(entry));
	ini.Get_String(section, entry, "", buffer, sizeof(buffer));
	Parse_Index_List(buffer, VoiceIndexes[outcome][1]);
}


/// <summary>
/// Queues the voice-over that announces an outcome.
/// This is how the World Domination Tour screens comment on what just happened -- the
/// category says what is being talked about and the outcome says whether it went well.
/// A second, more emphatic line can be queued behind the first for the occasions that
/// deserve one.
/// </summary>
/// <param name="emphasis">Should a follow-up emphasis line be queued as well?</param>
/// <param name="count_queued">Should these voice-overs be safe from being discarded?</param>
/// <param name="delay">How long to wait before the queue starts speaking.</param>
void Voices::Queue(VoiceCategoryType category, Outcome outcome, bool emphasis, bool count_queued, int delay)
{
	Discard();
	if (AudioEngine.Is_Available()) {
		char buffer[64];
		if (Pick_Standard_Voice(category, outcome, buffer, sizeof(buffer))) {
			Sample * sample = new Sample(buffer);
			if (!Playing()) {
				Timer = delay;
			}
			QueuedSamples.Add(sample);
			if (count_queued) {
				QueuedSampleCount++;
			}
			if (emphasis) {
				if (Pick_Emphasis_Voice(category, outcome, buffer, sizeof(buffer))) {
					sample = new Sample(buffer);
					QueuedSamples.Add(sample);
					if (count_queued) {
						QueuedSampleCount++;
					}
				}
			}
		}
	}
}


/// <summary>
/// Queues one of the standalone voice-overs for playback.
/// These are the one-off announcements -- the startup greeting and the territory
/// selection prompt -- which are not tied to a win, lose or draw outcome. Nothing is
/// queued when the side has no line for the occasion or when there is no audio at all.
/// </summary>
/// <param name="count_queued">Should this voice-over be safe from being discarded?</param>
/// <param name="delay">How long to wait before the queue starts speaking.</param>
void Voices::Queue(StandaloneVoiceType voice, bool count_queued, int delay)
{
	Discard();
	if (AudioEngine.Is_Available()) {
		char buffer[64];
		if (Pick_Standalone_Voice(voice, buffer, sizeof(buffer))) {
			Sample * sample = new Sample(buffer);
			if (!Playing()) {
				Timer = delay;
			}
			QueuedSamples.Add(sample);
			if (count_queued) {
				QueuedSampleCount++;
			}
		}
	}
}


/// <summary>
/// Services the voice-over queue.
/// This routine retires the voice-over that has finished and starts the next one, so
/// that a run of queued lines is heard one after another instead of all at once. The
/// queue is left alone until the delay set when it was filled has expired.
/// </summary>
/// <remarks>Call this routine regularly, or the queue will never move along.</remarks>
void Voices::Advance(void)
{
	if (Timer == 0 && QueuedSamples.Count() > 0) {
		Sample * sample = QueuedSamples[0];
		if (!sample->Playing()) {
			if (!IsIdle) {
				QueuedSamples.Delete_Index(0);
				delete sample;
				if (QueuedSampleCount > 0) {
					QueuedSampleCount--;
				}

			}
			if (QueuedSamples.Count() > 0) {
				QueuedSamples[0]->Start();
				IsIdle = false;
			} else {
				QueuedSampleCount = 0;
				IsIdle = true;
			}
		}
	}
}


/// <summary>
/// Discards the queued voice-overs that were never spoken for.
/// A voice-over may be queued either as something that must be heard or as something
/// that may be dropped if events overtake it. This routine drops the latter kind, and
/// the queue routines call it before adding anything new.
/// </summary>
void Voices::Discard(void)
{
	int last = QueuedSamples.Count();
	while (last-- > QueuedSampleCount) {
		delete QueuedSamples[last];
		QueuedSamples.Delete_Index(last);
	}
	if (QueuedSampleCount == 0) {
		IsIdle = true;
	}
}


void Voices_Noop1(void)
{

}


void Voices_Noop2(void)
{

}


/// <summary>
/// Silences the voice player and throws away everything it had queued.
/// Use this routine when the screen is going away, or whenever the voice-overs must be
/// abandoned outright rather than allowed to play themselves out.
/// </summary>
void Voices::Cleanup(void)
{
	int last = QueuedSamples.Count();
	if (last > 0) {
		QueuedSamples[0]->Stop();
		while (last-- != 0) {
			delete QueuedSamples[last];
		}
		QueuedSamples.Clear();
	}

	Timer = 0;
}


/// <summary>
/// Is the voice player still busy?
/// Use this routine to hold a presentation step open until everything queued for it has
/// been heard. A voice-over waiting its turn counts as busy just as much as one that is
/// actually sounding.
/// </summary>
/// <returns>bool; Are there voice-overs still queued or playing?</returns>
bool Voices::Playing(void)
{
	return(QueuedSamples.Count() > 0 && (IsIdle || (QueuedSamples[0] != NULL && QueuedSamples[0]->Playing())));
}


/// <summary>
/// Creates an animation that drives the supplied voice player.
/// The World Domination Tour screens add the animation returned here to their animation
/// list, which is how the voice player gets a chance to run at all.
/// </summary>
/// <returns>Returns with a pointer to the newly created animation.</returns>
Voices::Anim * WorldDominationTour::WDT_New_Voiced_Animation(Voices & voices)
{
	return(new Voices::Anim(&voices));
}


/// <summary>
/// Constructs a voice sample from the named sound file.
/// The sound data is fetched from the loaded mixfiles when it can be, and only read
/// off the disk when no mixfile has it. A sample whose sound was found nowhere is
/// harmless -- it simply never makes any noise.
/// </summary>
/// <param name="name">Name of the sound file to fetch.</param>
/// <param name="volume">The volume this sample should be played at.</param>
Voices::Sample::Sample(const char * name, int volume) :
	Volume(volume),
	SoundHandle(),
	File(NULL),
	Allocated(false)
{
	File = (void *)MFCD::Retrieve(name);

	if (File == NULL) {
		CCFileClass file;
		file.Open(name);
		if (file.Is_Available()) {
			File = Load_Alloc_Data(file);
			Allocated = true;
		}
	}
}


/// <summary>
/// Destroys the sample.
/// Playback is cut short if it is still going, and any sound data that was read from
/// disk is freed. Data that came out of a mixfile belongs to the mixfile and is left
/// where it is.
/// </summary>
Voices::Sample::~Sample(void)
{
	if (Playing()) {
		SoundHandle.Stop();
	}
	AudioEngine.Release_Sample(File);
	if (Allocated) {
		delete File;
	}
}


/// <summary>
/// Is this sample still being heard?
/// The audio engine is asked directly, so a sample that has run to its end reports
/// itself finished without anyone having to stop it.
/// </summary>
/// <returns>bool; Is the audio engine still playing this sample?</returns>
bool Voices::Sample::Playing(void) const
{
	return(SoundHandle.Is_Playing());
}


/// <summary>
/// Starts this sample playing.
/// The sample is handed to the audio engine at its own volume scaled by the player's
/// sound volume setting. A sample that is already started, or whose sound data could
/// not be found, is quietly ignored.
/// </summary>
void Voices::Sample::Start(void)
{
	if (!SoundHandle.Is_Valid() && File != NULL) {
		SoundHandle = AudioEngine.Play_Sample(File, AUDIO_GROUP_SFX, (float)Volume / 255.0f, 255);
	}
}


/// <summary>
/// Stops this sample from playing.
/// The sample forgets its sound handle, so it can be started over again afterwards.
/// </summary>
void Voices::Sample::Stop(void)
{
	if (SoundHandle.Is_Valid()) {
		SoundHandle.Stop();
	}
	SoundHandle.Clear();
}


/// <summary>
/// Constructs a voice-over animation for the supplied voice player.
/// This animation carries no artwork at all. It exists so that a voice player can be
/// added to a screen's animation list and be pumped along with everything else.
/// </summary>
/// <param name="voices">The voice player this animation will drive.</param>
Voices::Anim::Anim(Voices *voices) :
	MSAnim(0,0,false),
	VoicePlayer(voices)
{
	//nothing
}


/// <summary>
/// Destroys the voice-over animation.
/// The voice player is owned by the screen rather than by the animation, so it is left
/// alone here.
/// </summary>
Voices::Anim::~Anim(void)
{
	//nothing
}


/// <summary>
/// Activates or deactivates the voice-over animation.
/// Switching the animation off silences the voice player outright, so the screen goes
/// quiet the moment it stops being shown.
/// </summary>
/// <param name="active">Should this animation be active?</param>
void Voices::Anim::Set_Active(bool active)
{
	Active=active;
	if (!Active) {
		VoicePlayer->Cleanup();
	}
}


/// <summary>
/// Pauses the voice-over animation.
/// The animation timer is held still until the animation is resumed. Anything already
/// playing is left to finish -- this is not a way to silence the voice-overs.
/// </summary>
void Voices::Anim::Pause(void)
{
	Timer.Stop();
}


/// <summary>
/// Resumes the voice-over animation after a pause.
/// The animation timer starts running again from where it was stopped.
/// </summary>
void Voices::Anim::Resume(void)
{
	Timer.Start();
}


/// <summary>
/// Advances the voice-over animation by one frame.
/// This routine is how the voice queue gets serviced -- the presentation engine ticks
/// its animation list every frame, and this animation spends its tick retiring the
/// sample that finished and starting the next one.
/// </summary>
/// <returns>bool; Should this animation be deleted? It lives as long as its owner does.</returns>
bool Voices::Anim::Advance(Surface * surface, Rect & rect)
{
	VoicePlayer->Advance();
	return(false);
}


/// <summary>
/// Handles a redraw of this animation.
/// There is nothing visible to draw, so the redraw hook simply gives the voice queue
/// another chance to advance.
/// </summary>
void Voices::Anim::Redraw(Surface * surface, Rect const * rect)
{
	VoicePlayer->Advance();
}


/// <summary>
/// Fetches the screen area this animation occupies.
/// A voice-over animation is heard rather than seen, so it lays claim to nothing.
/// </summary>
/// <returns>Returns with an empty rectangle.</returns>
Rect Voices::Anim::Get_Rect(void) const
{
	return(Rect());
}


/// <summary>
/// Has this animation run its course?
/// The presentation engine uses this routine to decide when the voice-overs are done
/// and the screen may move on to whatever comes next.
/// </summary>
/// <returns>bool; Have all of the queued voice-overs been heard?</returns>
bool Voices::Anim::Has_Finished(void) const
{
	return(VoicePlayer->Playing() == false);
}


/// <summary>
/// Handles a restore request for this animation.
/// A voice-over animation puts nothing on the screen, so there is nothing to restore.
/// The hook is used to give the voice queue another chance to advance.
/// </summary>
void Voices::Anim::Restore(Rect const & rect)
{
	VoicePlayer->Advance();
}
