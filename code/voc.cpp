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

#include "voc.h"

#include "_map.h"
#include "_rect.h"
#include "_tactica.h"
#include "audio/audioengine.h"
#include "ccini.h"
#include "cell.h"
#include "globals.h"
#include "goptions.h"
#include "map.h"
#include "sndtype.h"
#include "tactical.h"
#include "vector.h"

#include <algorithm>
#include <cstdlib>


DynamicVectorClass<VocClass *> Vocs;

AudioEventTypeClass VocClass::Defaults;

namespace {

int const DEFAULT_CHANNELS = 16;
int const PIXELS_PER_CELL = 48;
float const SILENT_LEVEL = 0.05f;
int const POSITIONAL_MAX = 64;

// Placed sounds still playing, re-aimed each tick as the view moves.
struct PositionalSound {
	AudioHandle Handle;
	Coord Position;
	float Level;
	int Pan;
};

PositionalSound _positional[POSITIONAL_MAX];


float Effect_Level(float volume)
{
	return(std::min(Options.SoundVolume * volume, 1.0f));
}


float Pan_Level(int pan)
{
	return((float)std::clamp(pan, -100, 100) / 100.0f);
}


void Track_Positional(AudioHandle handle, Coord const & coord, float level, int pan)
{
	if (handle.Is_Null()) {
		return;
	}
	int slot = -1;
	for (int i = 0; i < POSITIONAL_MAX; i++) {
		if (_positional[i].Handle == handle) {
			slot = i;
			break;
		}
		if (slot < 0 && !_positional[i].Handle.Is_Valid()) {
			slot = i;
		}
	}
	if (slot >= 0) {
		_positional[slot].Handle = handle;
		_positional[slot].Position = coord;
		_positional[slot].Level = level;
		_positional[slot].Pan = pan;
	}
}

} // namespace


/// <summary>
/// Creates a sound effect for the sample name specified.
/// The new sound adds itself to the master sound list and starts out as a copy of the
/// defaults, with the name as its one sample. Its own section is read when the rules are.
/// </summary>
/// <param name="filename">The root name of the sound sample, without any extension.</param>
VocClass::VocClass(const char *filename)
{
	strcpy(Name, filename);

	Vocs.Add(this);

	Type = Defaults;
	strncpy(Type.Name, Name, sizeof(Type.Name) - 1);
	Type.Name[sizeof(Type.Name) - 1] = '\0';
	strncpy(Type.Sounds[0], Name, sizeof(Type.Sounds[0]) - 1);
	Type.Sounds[0][sizeof(Type.Sounds[0]) - 1] = '\0';
	Type.SoundCount = 1;
}


/// <summary>
/// Removes this sound effect from the master sound list.
/// </summary>
VocClass::~VocClass(void)
{
	Vocs.Delete(this);
}


/// <summary>
/// Fetches this sound effect's settings from the rules.
/// Every key the sound's own section omits comes from the defaults.
/// </summary>
/// <param name="ini">The rules database to fetch the settings from.</param>
/// <returns>bool; Did the sound have a section of its own?</returns>
bool VocClass::Fill_In(CCINIClass const &ini)
{
	return(Sound_Type_Fill_In(ini, Name, Defaults, Type));
}


/// <summary>
/// Plays this sound effect at the volume and pan specified.
/// The volume requested is scaled by the player's sound effect option setting, so the
/// sound falls silent when the effects are turned off.
/// </summary>
/// <param name="vol">The volume to play at, where 1.0 is this sound's own full volume.</param>
/// <param name="pan">The pan, from -100 at the left to 100 at the right.</param>
/// <returns>The handle of the playing sound, or a null handle if none was played.</returns>
AudioHandle VocClass::Play(float vol, int pan)
{
	if (Options.SoundVolume > 0.0 && vol > 0.0 && Can_Play() && AudioEngine.Is_Available()) {
		return(AudioEngine.Play_Event(Type, AUDIO_GROUP_SFX, Effect_Level(vol), Pan_Level(pan)));
	}
	return(AudioHandle());
}


/// <summary>
/// Plays this sound effect as a voice, at the volume specified.
/// The caller has already worked out the final volume and the sound effect option
/// setting does not apply.
/// </summary>
/// <param name="vol">The volume to play at, where 1.0 is this sound's own full volume.</param>
/// <returns>The handle of the playing sound, or a null handle if none was played.</returns>
AudioHandle VocClass::Play_Voice(float vol)
{
	if (vol > 0.0 && Can_Play() && AudioEngine.Is_Available()) {
		return(AudioEngine.Play_Event(Type, AUDIO_GROUP_SYSTEM, std::min(vol, 1.0f), 0.0f));
	}
	return(AudioHandle());
}


AudioHandle Sound_Effect(VocType voc, float volume, int pan, AudioHandle * handle)
{
	if (voc == VOC_NONE || voc >= Vocs.Count()) {
		return(AudioHandle());
	}
	VocClass & sound = *Vocs[voc];

	if (handle != nullptr && handle->Is_Valid()) {
		if (handle->Type() == &sound.Type_Data()) {
			handle->Retarget(Effect_Level(volume), Pan_Level(pan));
			return(*handle);
		}
		handle->Stop();
	}

	AudioHandle played = sound.Play(volume, pan);
	if (handle != nullptr) {
		*handle = played;
	}
	return(played);
}


AudioHandle Voice_Sound_Effect(VocType voc, float volume)
{
	if (voc != VOC_NONE && voc < Vocs.Count()) {
		return(Vocs[voc]->Play_Voice(volume));
	}
	return(AudioHandle());
}


float Calculate_Volume_And_Pan(Coord const & coord, AudioEventTypeClass const & type, int & pan)
{
	pan = SOUND_PAN_CENTER;
	if (TacticalMap == nullptr) {
		return(1.0f);
	}

	if (type.Type & (SOUND_TYPE_SHROUD | SOUND_TYPE_HIDDEN)) {
		Cell cell = coord.As_Cell();
		bool seen = false;
		if (Map.Is_Valid(cell)) {
			CellClass const & place = Map[cell];
			seen = place.IsMapped || place.IsVisible;
		}
		if ((type.Type & SOUND_TYPE_SHROUD) && !seen) {
			return(0.0f);
		}
		if ((type.Type & SOUND_TYPE_HIDDEN) && seen) {
			return(0.0f);
		}
	}

	Point2D pixel;
	TacticalMap->Coord_To_Pixel(coord, pixel);
	int width = TacticalRect.Width;
	int height = TacticalRect.Height;
	if (width <= 0 || height <= 0) {
		return(1.0f);
	}

	int dx;
	int dy;
	if (type.Type & SOUND_TYPE_LOCAL) {
		dx = std::abs(pixel.X - width / 2);
		dy = std::abs(pixel.Y - height / 2);
	} else {
		dx = pixel.X < 0 ? -pixel.X : (pixel.X > width ? pixel.X - width : 0);
		dy = pixel.Y < 0 ? -pixel.Y : (pixel.Y > height ? pixel.Y - height : 0);
	}
	// The view is wider than it is tall, so vertical distance counts double.
	dy *= 2;

	int range = std::max(type.Range, 1) * PIXELS_PER_CELL;
	float volume = 1.0f - (float)std::max(dx, dy) / (float)range;
	if (type.Type & SOUND_TYPE_GLOBAL) {
		volume = std::max(volume, type.MinVolume);
	}
	if (volume < SILENT_LEVEL) {
		return(0.0f);
	}

	pan = std::clamp((pixel.X - width / 2) * 100 / (width / 2), -100, 100);
	return(std::min(volume, 1.0f));
}


AudioHandle Sound_Effect(VocType voc, Coord const & coord, AudioHandle * handle)
{
	if (voc == VOC_NONE || voc >= Vocs.Count()) {
		return(AudioHandle());
	}
	VocClass & sound = *Vocs[voc];

	int pan;
	float volume = Calculate_Volume_And_Pan(coord, sound.Type_Data(), pan);
	if (volume <= 0.0f) {
		if (handle != nullptr && handle->Is_Valid()) {
			handle->Stop();
			handle->Clear();
		}
		return(AudioHandle());
	}

	AudioHandle played = Sound_Effect(voc, volume, pan, handle);
	Track_Positional(played, coord, volume, pan);
	return(played);
}


AudioHandle Play_If_In_Range(VocType voc, Coord const & coord, AudioHandle * handle)
{
	if (handle == nullptr || voc == VOC_NONE || voc >= Vocs.Count()) {
		return(AudioHandle());
	}
	VocClass & sound = *Vocs[voc];

	int pan;
	float volume = Calculate_Volume_And_Pan(coord, sound.Type_Data(), pan);

	if (handle->Is_Valid()) {
		if (volume <= 0.0f) {
			handle->Stop();
			handle->Clear();
			return(AudioHandle());
		}
		handle->Retarget(Effect_Level(volume), Pan_Level(pan));
		return(*handle);
	}

	// A one-shot that has ended stays ended; an endless loop comes back
	// without its attack once its place is in range again.
	handle->Clear();
	if (volume <= 0.0f || !sound.Type_Data().Never_Ends() || Options.SoundVolume <= 0.0 || !sound.Can_Play() || !AudioEngine.Is_Available()) {
		return(AudioHandle());
	}
	*handle = AudioEngine.Play_Event(sound.Type_Data(), AUDIO_GROUP_SFX, Effect_Level(volume), Pan_Level(pan), true);
	return(*handle);
}


void Sound_Effect_AI(void)
{
	for (int i = 0; i < POSITIONAL_MAX; i++) {
		PositionalSound & entry = _positional[i];
		if (entry.Handle.Is_Null()) {
			continue;
		}
		AudioEventTypeClass const * type = entry.Handle.Type();
		if (type == nullptr) {
			entry.Handle.Clear();
			continue;
		}
		int pan;
		float volume = Calculate_Volume_And_Pan(entry.Position, *type, pan);
		if (volume <= 0.0f) {
			entry.Handle.Stop();
			entry.Handle.Clear();
			continue;
		}
		if (std::fabs(volume - entry.Level) > 0.01f || pan != entry.Pan) {
			entry.Handle.Retarget(Effect_Level(volume), Pan_Level(pan));
			entry.Level = volume;
			entry.Pan = pan;
		}
	}
}


void Stop_All_Sound_Effects(void)
{
	for (int i = 0; i < POSITIONAL_MAX; i++) {
		_positional[i].Handle.Clear();
	}
	if (AudioEngine.Is_Available()) {
		AudioEngine.Events().Stop_Group(AUDIO_GROUP_SFX, 0);
	}
}


/// <summary>
/// Creates the master sound effect list from the rules.
/// This routine reads the channel budget and the defaults, then fetches every sound named
/// in the sound list section, creating a sound effect for any that does not exist yet, and
/// lets each one fill itself in from its own section.
/// </summary>
/// <param name="ini">The rules database to fetch the sound list from.</param>
void Init_Vocs(CCINIClass const &ini)
{
	char const * const SECTION = "SoundList";

	AudioEngine.Set_Channels(Sound_Type_Read_Channels(ini, DEFAULT_CHANNELS));
	Sound_Type_Read_Defaults(ini, VocClass::Defaults);

	if (ini.Is_Present(SECTION)) {
		int count = ini.Entry_Count(SECTION);
		for (int i = 0; i < count; i++) {
			char name[32];
			if (ini.Get_String(SECTION, ini.Get_Entry(SECTION, i), "", name, sizeof(name)) != 0) {

				VocClass *voc = NULL;
				VocType type = VocClass::From_Name(name);
				if (type == VOC_NONE) {
					voc = new VocClass(name);
				} else {
					voc = Vocs[type];
				}
				voc->Fill_In(ini);
			}
		}
	}
}


/// <summary>
/// Destroys every sound effect in the master sound list.
/// This routine is used when shutting the game down or before the sound list is rebuilt
/// from a fresh set of rules.
/// </summary>
void Free_Vocs(void)
{
	Stop_All_Sound_Effects();
	while (Vocs.Count() > 0) {
		VocClass *voc = Vocs[0];
		delete voc;
	}
}


/***********************************************************************************************
 * Voc_From_Name -- Fetch VocType from ASCII name specified.                                   *
 *                                                                                             *
 *    This will find the corresponding VocType from the ASCII string specified. It does this   *
 *    by finding a root filename that matches the string.                                      *
 *                                                                                             *
 * INPUT:   name  -- Pointer to the ASCII string that will be converted into a VocType.        *
 *                                                                                             *
 * OUTPUT:  Returns with the VocType that matches the string specified. If no match could be   *
 *          found, then VOC_NONE is returned.                                                  *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/06/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
VocType VocClass::From_Name(char const * name)
{
	if (name == NULL) return(VOC_NONE);

	for (VocType voc = VOC_FIRST; voc < Vocs.Count(); voc = VocType(voc + 1)) {
		if (stricmp(name, Vocs[voc]->Name) == 0) {
			return(voc);
		}
	}

	return(VOC_NONE);
}


/// <summary>
/// Fetches the sound effect that matches the ASCII name specified.
/// This routine is used when reading sound assignments out of the rules, where a sound is
/// named by the root of its filename. The placeholder "none" name is recognized as meaning
/// no sound at all.
/// </summary>
/// <param name="name">Pointer to the ASCII name of the sound effect to find.</param>
/// <returns>Returns with a pointer to the matching sound effect. Otherwise, NULL is
/// returned.</returns>
VocClass * VocClass_From_Name(char const * name)
{
	if (name == NULL) return(NULL);

	if (!strcmpi(name, "<none>")) return(NULL);

	for (VocType voc = VOC_FIRST; voc < Vocs.Count(); voc = VocType(voc + 1)) {
		if (stricmp(name, Vocs[voc]->Name) == 0) {
			return(Vocs[voc]);
		}
	}

	return(NULL);
}


/***********************************************************************************************
 * Voc_Name -- Fetches the name for the sound effect.                                          *
 *                                                                                             *
 *    This routine returns the descriptive name of the sound effect. Currently, this is just   *
 *    the root of the file name.                                                               *
 *                                                                                             *
 * INPUT:   voc   -- The VocType that the corresponding name is requested.                     *
 *                                                                                             *
 * OUTPUT:  Returns with a pointer to the text string the represents the sound effect.         *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/06/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
char const * Voc_Name(VocType voc)
{
	if (voc != VOC_NONE && voc < Vocs.Count()) {
		return(Vocs[voc]->Name);
	}
	return("<none>");
}


/// <summary>
/// Fetches the sound effect number of this sound.
/// This is the inverse of the master sound list lookup -- it recovers the VocType that the
/// rest of the game uses to refer to this sound.
/// </summary>
/// <returns>Returns with the VocType of this sound, or VOC_NONE if it is not
/// registered.</returns>
VocType VocClass::Voc_Type(void)
{
	for (int index = 0; index < Vocs.Count(); index++) {
		if (Vocs[index] == this) {
			return(VocType)(index);
		}
	}
	return(VOC_NONE);
}


/// <summary>
/// Checks whether this sound effect may be played at all.
/// A sound is playable while the game was not started quiet and it names at least one
/// sample; whether that sample exists is found out when it is first played.
/// </summary>
/// <returns>bool; Can this sound be played?</returns>
bool VocClass::Can_Play(void) const
{
	return(!Debug_Quiet && Type.SoundCount > 0);
}
