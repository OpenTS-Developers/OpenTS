/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "always.h"

#include "light.h"

#include "_map.h"
#include "_milsectmr.h"
#include "cell.h"
#include "crc.h"
#include "globals.h"
#include "isotype.h"
#include "lightcon.h"
#include "map.h"
#include "milsectmr.h"
#include "savestream.h"
#include "sun.h"
#include "tracker.h"
#include "vector.h"

#include <algorithm>


DynamicVectorClass<LightSourceClass *> LightSources;
DynamicVectorClass<LightSourceClass::PendingCellClass *> LightSourceClass::PendingCells;

bool LightSourceClass::Recalc = true;


/// <summary>
/// Creates a light source at the location specified.
/// The light is listed with the other light sources but starts out switched off; call
/// Enable on it once it is ready to contribute to the scene.
/// </summary>
/// <param name="visibility">The distance, in leptons, that the light reaches.</param>
/// <param name="intensity">The strength of the light at its center.</param>
LightSourceClass::LightSourceClass(Coord coord, int visibility, int intensity, int red, int green, int blue) :
	BASECLASS(),
	Intensity(intensity),
	RedTint(red),
	GreenTint(green),
	BlueTint(blue),
	Position(coord),
	Visibility(visibility),
	IsEnabled(false)
{
	LightSources.Add(this);
}


/// <summary>
/// Creates a blank light source.
/// The light has no position, no tint and is switched off, but it is already listed
/// with the other light sources so that it will be processed along with them.
/// </summary>
LightSourceClass::LightSourceClass(void) :
	BASECLASS(),
	Intensity(0),
	RedTint(0),
	GreenTint(0),
	BlueTint(0),
	Position(COORD_NONE),
	Visibility(0),
	IsEnabled(false)
{
	LightSources.Add(this);
}


/// <summary>
/// Removes this light source from the game.
/// The light is switched off so that the cells it was tinting revert to normal, it
/// leaves the global light list, and anything still pointing at it is notified.
/// </summary>
LightSourceClass::~LightSourceClass(void)
{
	Detach_This_From_All(this);
	LightSources.Delete(this);

	Disable();
}


/// <summary>
/// Destroys every light source in the game.
/// This routine is used when tearing a scenario down. Cell recalculation is held off
/// while the lights go away, so the map is not rebuilt once for every light removed.
/// </summary>
void LightSourceClass::Reset(void)
{
	Recalc = false;

	while (LightSources.Count() != 0) {

		LightSourceClass * light = LightSources[0];
		delete light;
		LightSources.Delete_Index(0);
	}

	Recalc = true;
}


/// <summary>
/// Turns this light source on.
/// The cells within reach are rebuilt so that the light's tint shows up in the scene.
/// Enabling a light that is already on does nothing.
/// </summary>
/// <param name="defer">Should the affected cells be queued rather than updated here?</param>
void LightSourceClass::Enable(bool defer)
{
	if (!IsEnabled) {
		IsEnabled = true;
		Recalculate_Affected_Cells(defer);
	}
}


/// <summary>
/// Turns this light source off.
/// The cells the light was reaching are rebuilt so that its contribution disappears
/// from the scene. Disabling a light that is already off does nothing.
/// </summary>
/// <param name="defer">Should the affected cells be queued rather than updated here?</param>
void LightSourceClass::Disable(bool defer)
{
	if (IsEnabled) {
		IsEnabled = false;
		Recalculate_Affected_Cells(defer);
	}
}


/// <summary>
/// Rebuilds the drawers of the cells this light reaches.
/// Every cell within the light's visibility is given a fresh drawer so that this
/// source's tint is either taken into account or dropped, and the tactical map is
/// flagged for a redraw.
/// </summary>
/// <param name="defer">Should the affected cells be queued rather than updated here?</param>
void LightSourceClass::Recalculate_Affected_Cells(bool defer)
{
	if (Recalc) {
		if (defer && PendingCells.Count() != 0) {
			LightSourceClass::Process_Lighting(0, true);
		}

		Cell center = Position.As_Cell();
		int radius = Visibility / CELL_LEPTON + 1;

		for (int y = -radius; y <= radius; y++) {
			for (int x = -radius; x <= radius; x++) {
				Cell test_cell = center + Cell(x, y);
				if (test_cell.X >= 0 && test_cell.X < MAP_CELL_W && test_cell.Y >= 0 && test_cell.Y < MAP_CELL_H && Map.Is_Valid(test_cell)) {
					Coord cell_coord = test_cell.As_Coord();
					int dx = cell_coord.X - Position.X;
					int dy = cell_coord.Y - Position.Y;
					int dist = dx * dx + dy * dy;

					if (int(std::sqrt(dist)) <= Visibility) {
						if (defer) {
							PendingCells.Add(new PendingCellClass(test_cell));
						} else {
							Map[test_cell].Init_Drawer();
						}
					}
				}
			}
		}

		Map.Flag_To_Redraw(GS_REDRAW_TACTICAL);
	}
}


/// <summary>
/// Handles the lighting work that has been put off until later.
/// Cells queued by a light source have their drawers picked here and are then committed
/// to the map in one pass. The work is spread across calls so that a sweeping lighting
/// change does not stall the game loop.
/// </summary>
/// <param name="time_budget_ms">The milliseconds this routine may spend before yielding.</param>
/// <param name="force">Should the whole queue be finished regardless of the budget?</param>
void LightSourceClass::Process_Lighting(int time_budget_ms, bool force)
{
	static unsigned _update_frames = 0;

	double update_start_time = MillisecondTimer;

	static unsigned _update_interval = 50;
	static unsigned _last_update_frame = 0;
	static int _pending_index = 0;
	static int _pending_remaining = 0;
	static bool _commit_ready = false;

	_update_frames++;

	if (PendingCells.Count() && !_commit_ready) {
		if (_pending_index <= 0) {
			_pending_remaining = PendingCells.Count();
			_pending_index = _pending_remaining - 1;
		}
		for (; _pending_index >= 0; _pending_index--) {
			if (!force && ((unsigned)_pending_index % 16) == 15) {
				if (MillisecondTimer - update_start_time >= time_budget_ms) {
					break;
				}
			}

			if (PendingCells[_pending_index]->Converter != NULL) {
				_pending_remaining--;
			} else {
				LightConvertClass * drawer;
				int intensity, ambient, brightness, tile_brightness, alt_brightness;
				Map[PendingCells[_pending_index]->CellID].Pick_Drawer(drawer, intensity, ambient, brightness, tile_brightness, alt_brightness);
				PendingCells[_pending_index]->Converter = drawer;
				PendingCells[_pending_index]->Intensity = intensity;
				PendingCells[_pending_index]->Ambient = ambient;
				PendingCells[_pending_index]->Brightness = brightness;
				PendingCells[_pending_index]->TileBrightness = tile_brightness;
				PendingCells[_pending_index]->AltBrightness = alt_brightness;
				_pending_remaining--;
			}
		}
		if (_pending_remaining <= 0) {
			_commit_ready = true;
		}
	}

	double time_left = time_budget_ms - (MillisecondTimer - update_start_time);

	if (_commit_ready) {
		if (force || time_left >= time_budget_ms - 1) {
			for (int i = 0; i < PendingCells.Count(); i++) {
				PendingCellClass * pending = PendingCells[i];
				Map[pending->CellID].Init_Drawer(pending->Converter, pending->Intensity, pending->Ambient, pending->Brightness, pending->TileBrightness, pending->AltBrightness);
				delete pending;
			}
			PendingCells.Clear();
			_commit_ready = false;
			Map.Flag_To_Redraw(GS_REDRAW_TACTICAL);
		}
	}

	time_left = time_budget_ms - (MillisecondTimer - update_start_time);

	if (time_left > 0.0 && _update_frames > (_last_update_frame + _update_interval)) {
		bool more_to_free = IsometricTileTypeClass::Free_Unused_Drawers(std::min(time_left * 0.5, 1.0), false);
		_last_update_frame = _update_frames;
		_update_interval = more_to_free ? 1 : 50;
	}
}


/// <summary>
/// Submits this light source to the game state checksum.
/// This routine is used by the network sync check so that every machine can prove it
/// agrees about the lighting in the scene.
/// </summary>
/// <param name="crc">The checksum engine to feed this object's state into.</param>
void LightSourceClass::Compute_CRC(CRCEngine & crc) const
{
	BASECLASS::Compute_CRC(crc);
	crc(Intensity);
	crc(RedTint);
	crc(GreenTint);
	crc(BlueTint);
	crc(Visibility);
	crc(IsEnabled);
}


ClassID LightSourceClass::Class_ID(void) const
{
	return(ClassID_LightSource);
}


/// <summary>
/// Lists the members this light source carries.
/// </summary>
/// <param name="stream">The stream carrying the members.</param>
void LightSourceClass::Serialize(SaveStreamClass & stream)
{
	BASECLASS::Serialize(stream);

	stream.Serialize(Intensity);
	stream.Serialize(RedTint);
	stream.Serialize(GreenTint);
	stream.Serialize(BlueTint);
	stream.Serialize(Position);
	stream.Serialize(Visibility);
	stream.Serialize(IsEnabled);
	// PendingCells -- the relighting queue and its gate, shared by every light.
	// Recalc
}


/// <summary>
/// Fetches the RTTI type identifier of this object.
/// </summary>
/// <returns>Returns with RTTI_LIGHTSOURCE.</returns>
RTTIType LightSourceClass::Fetch_RTTI(void) const
{
	return(RTTI_LIGHTSOURCE);
}
