/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "always.h"

#include "revent.h"

#include "_map.h"
#include "_rules.h"
#include "coord.h"
#include "dsurface.h"
#include "globals.h"
#include "matrix3d.h"
#include "mouse.h"
#include "rules.h"
#include "savestream.h"
#include "vector.h"

#include <algorithm>

DynamicVectorClass<RadarEventClass *> RadarEventClass::RadarEvents;

Cell LastRadarEventCell(0,0);

const char * RadarEventNames[RADAREVENT_COUNT] = {
	"Combat Event",
	"Noncombat Event",
	"DropZone Event",
	"Base Attacked Event",
	"Harvester Attacked Event",
	"Enemy Sensed Event"
};


/// <summary>
/// Raises a radar event at the specified cell.
/// This is the routine the game logic calls to draw the player's eye to something
/// worth looking at. An event of a suppressible kind is quietly dropped when one of
/// the same kind is already showing nearby.
/// </summary>
/// <param name="event">The kind of event to raise.</param>
/// <param name="cell">The cell that the event should draw attention to.</param>
/// <returns>bool; Was an event actually created?</returns>
bool Submit_Radar_Event(RadarEventType event, Cell cell)
{
	if (Can_Suppress_Radar_Event(event)) {
		for (int index = 0; index < RadarEventClass::RadarEvents.Count(); index++) {
			if (RadarEventClass::RadarEvents[index]->Type == event) {
				if (cell.Distance_To(RadarEventClass::RadarEvents[index]->Location) < RadarEventClass::RadarEvents[index]->Get_Suppression_Distance()) {
					return(false);
				}
			}
		}
	}
	new RadarEventClass(event, cell);
	return(true);
}


/// <summary>
/// Plots a single radar event pixel.
/// This routine is handed to the line plotter as its callback, so that the event box
/// is stroked through the radar's own pixel routine.
/// </summary>
/// <param name="point">The radar pixel to light up.</param>
void Plot_Radar_Event_Point(Point2D const & point)
{
	Map.Radar_Pixel(point);
}


/// <summary>
/// Can this kind of radar event be suppressed?
/// The noisier kinds are allowed to swallow their own nearby duplicates so that a running
/// battle does not litter the radar. Anything the player must not miss is always shown.
/// </summary>
/// <returns>bool; May a nearby event of this kind be discarded?</returns>
bool Can_Suppress_Radar_Event(RadarEventType event)
{
	switch (event) {
		case RADAREVENT_COMBAT:
		case RADAREVENT_HARVESTER_ATTACKED:
		case RADAREVENT_ENEMY_SENSED:
			return(true);
	}
	return(false);
}


/// <summary>
/// Creates a radar event at the specified cell.
/// The event starts out as a wide spinning box that will close in on the cell it is
/// flagging. It adds itself to the master list, so the caller has no need to keep
/// hold of the pointer.
/// </summary>
/// <param name="type">The kind of event being flagged.</param>
/// <param name="cell">The cell that the event should draw attention to.</param>
RadarEventClass::RadarEventClass(RadarEventType type, Cell cell):
	Type(type),
	RotationAngle((float)DEG_TO_RAD(45)),
	RotationSpeed(Rule->RadarEventRotationSpeed),
	ColorFactor(0),
	ColorSpeed(Rule->RadarEventColorSpeed),
	Location(cell),
	DurationTimer(0),
	VisibilityTimer(0),
	IsRotating(true),
	IsVisible(true)
{
	Offset = Map.Cell_To_Radar_Pixel(cell).TopLeft - Map.RadarRect.TopLeft;
	Radius = std::max(Offset.X, std::max(Offset.Y, std::max(Map.RadarRect.Width - Offset.X, Map.RadarRect.Height - Offset.Y)));
	LastRadarEventCell = cell;
	RadarEvents.Add(this);
}


/// <summary>
/// Destroys this radar event.
/// The event takes itself out of the master list, so deleting one is all that is
/// needed to retire it from the radar.
/// </summary>
RadarEventClass::~RadarEventClass(void)
{
	RadarEvents.Delete(this);
}


/// <summary>
/// Advances every radar event in play.
/// This routine is called by the main game logic loop so that the radar pings keep
/// closing in and pulsing.
/// </summary>
void Process_Radar_Events(void)
{
	for (int index = 0; index < RadarEventClass::RadarEvents.Count(); index++) {
		RadarEventClass::RadarEvents[index]->Process();
	}
}


/// <summary>
/// Advances this radar event by one game frame.
/// The event closes in on the spot it is flagging, spinning as it goes, and once it
/// has settled square it merely pulses in place until its time runs out. The event
/// plots itself on the radar as part of this update.
/// </summary>
void RadarEventClass::Process(void)
{
	if (IsVisible) {
		if (!IsRotating && VisibilityTimer == 0) {
			IsVisible = false;
		}

		Plot();
		Radius = std::max<float>(Radius - Rule->RadarEventSpeed, Rule->RadarEventMinRadius);
		float normalized_angle = RotationAngle + (M_PI / 4) - (int)((RotationAngle + (M_PI / 4)) * (2 / M_PI)) * M_PI / 2;
		if (IsRotating) {
			if (fabs(Radius - Rule->RadarEventMinRadius) < 0.01) {
				if (normalized_angle < RotationSpeed) {
					RotationAngle = normalized_angle + RotationAngle;
					IsRotating = false;
					RotationAngle = RotationAngle;
					VisibilityTimer = Get_Visibility_Duration();
					DurationTimer = Get_Duration();
				} else {
					RotationAngle += RotationSpeed;
					RotationSpeed = std::max(Rule->RadarEventRotationSpeed * (1.0f / 3.0f), RotationSpeed - Rule->RadarEventRotationSpeed * 0.02f);
				}
			} else {
				RotationAngle = RotationSpeed + RotationAngle;
			}
		}

		if (RotationAngle > DEG_TO_RAD(360)) {
			RotationAngle -= DEG_TO_RAD(360);
		}

		ColorFactor += ColorSpeed;
		if (ColorFactor < 0.0f && ColorSpeed < 0.0f) {
			ColorSpeed = -ColorSpeed;
			ColorFactor = 0.0f;
		} else if (ColorFactor > 1.0f && ColorSpeed > 0.0f) {
			ColorSpeed = -ColorSpeed;
			ColorFactor = 1.0f;
		}
	}
}


/// <summary>
/// Draws every radar event that still has something to show.
/// This routine is called by the radar rendering code. Events that have gone quiet
/// are passed over rather than drawn.
/// </summary>
void RadarEventClass::Draw_Events(void)
{
	for (int index = 0; index < RadarEvents.Count(); index++) {
		RadarEventClass * event = RadarEvents[index];
		if (event->VisibilityTimer > 0 || event->IsRotating) {
			event->Draw();
		}
	}
}


/// <summary>
/// Draws this radar event onto the radar surface.
/// The event box is stroked as a gradient line that cycles between the event's two
/// colors, which is what gives the ping its pulse. The radar's dirty rectangle is
/// grown to cover whatever was drawn.
/// </summary>
void RadarEventClass::Draw(void)
{
	Point2D event_rect[4];
	int i;

	Get_Event_Rect(event_rect);

	for (i = 0; i < ARRAY_SIZE(event_rect); i++) {
		event_rect[i] += Offset;
	}

	float color_factor = ColorFactor;

	int dy = abs(event_rect[0].Y - event_rect[1].Y);
	int dx = abs(event_rect[0].X - event_rect[1].X);
	int maxdist = std::max(dx, dy);

	float rate = (Radius * 2) * M_SQRT_2 / (float)maxdist * ColorSpeed;

	RGBClass color1 = Get_Max_Color();
	RGBClass color2 = Get_Min_Color();

	for (i = 0; i < ARRAY_SIZE(event_rect); i++) {
		Map.RadarSurface->Draw_Ping_Pong_Gradient_Line(Map.RadarSurface->Get_Rect(), event_rect[i], event_rect[(i + 1) % 4], color1, color2, rate, color_factor);
	}

	for (i = 0; i < ARRAY_SIZE(event_rect); i++) {
		Clip_Line_To_Rect(event_rect[i], event_rect[(i + 1) % 4], Map.RadarSurface->Get_Rect());
	}

	if (Map.RadarMode == RadarClass::RMODE_TACTICAL && Map.RadarState == RadarClass::RSTATE_ACTIVE) {
		for (i = 0; i < ARRAY_SIZE(event_rect); i++) {
			Map.LastDrawRect = Intersect(Union(Map.LastDrawRect, Rect(event_rect[i].X, event_rect[i].Y, 1, 1) + Map.RadarRect.TopLeft), Map.RadarRect);
		}
	}
}


/// <summary>
/// Deletes the radar events that have run their course.
/// This routine is called once the events have been processed for the frame, and
/// disposes of any that have finished shrinking and outlived their duration.
/// </summary>
void RadarEventClass::Remove_Finished(void)
{
	for (int index = RadarEvents.Count() - 1; index >= 0; index--) {
		if (RadarEventClass::RadarEvents[index]->DurationTimer == 0 && !RadarEvents[index]->IsRotating) {
			delete RadarEventClass::RadarEvents[index];
		}
	}
}


/// <summary>
/// Should a proposed radar event be suppressed?
/// Some event kinds are raised in floods -- a firefight will submit one every time a
/// shot lands. This routine spots a proposed event that is close enough to one already
/// showing that the player would learn nothing from a second ping.
/// </summary>
/// <returns>bool; Should the proposed event be thrown away?</returns>
bool Try_Suppress_Radar_Event(RadarEventType event, int, Cell cell)
{
	if (Can_Suppress_Radar_Event(event)) {
		for (int index = 0; index < RadarEventClass::RadarEvents.Count(); index++) {
			if (RadarEventClass::RadarEvents[index]->Type == event) {
				if (cell.Distance_To(RadarEventClass::RadarEvents[index]->Location) < RadarEventClass::RadarEvents[index]->Get_Suppression_Distance()) {
					return(true);
				}
			}
		}
	}
	return(false);
}


/// <summary>
/// Are there no radar events in play?
/// </summary>
/// <returns>bool; Is the radar free of events?</returns>
bool No_Radar_Events_Submitted(void)
{
	if (RadarEventClass::RadarEvents.Count() == 0) {
		return(true);
	}
	return(false);
}


/// <summary>
/// Plots the event box onto the radar map.
/// This routine stamps the box as plain radar pixels rather than a shaded gradient,
/// and grows the radar's dirty rectangle so the pixels touched will be blitted.
/// </summary>
void RadarEventClass::Plot(void)
{
	Point2D event_rect[4];
	int i;

	Get_Event_Rect(event_rect);

	for (i = 0; i < ARRAY_SIZE(event_rect); i++) {
		event_rect[i] += Offset;
	}

	for (i = 0; i < ARRAY_SIZE(event_rect); i++) {
		Map.RadarSurface->Plot_Line(Map.RadarSurface->Get_Rect(), event_rect[i], event_rect[(i + 1) % 4], Plot_Radar_Event_Point);
	}

	for (i = 0; i < ARRAY_SIZE(event_rect); i++) {
		Clip_Line_To_Rect(event_rect[i], event_rect[(i + 1) % 4], Map.RadarSurface->Get_Rect());
	}

	if (Map.RadarMode == RadarClass::RMODE_TACTICAL && Map.RadarState == RadarClass::RSTATE_ACTIVE) {
		for (i = 0; i < ARRAY_SIZE(event_rect); i++) {
			Map.LastDrawRect = Intersect(Union(Map.LastDrawRect, Rect(event_rect[i].X, event_rect[i].Y, 1, 1) + Map.RadarRect.TopLeft), Map.RadarRect);
		}
	}
}


/// <summary>
/// Builds the four corners of the event box.
/// This routine hands back the box at its current radius and rotation, centered about
/// the origin. Callers offset the corners to the event's place on the radar before
/// they plot or draw them.
/// </summary>
void RadarEventClass::Get_Event_Rect(Point2D (& event_rect)[4]) const
{
	Matrix3D matrix;
	matrix.Make_Identity();
	matrix.Rotate_Z(RotationAngle);

	Vector3 vector(Radius, 0, 0);
	vector = matrix * vector;
	int x_radius = vector.X;
	int y_radius = vector.Y;

	event_rect[0] = Point2D(x_radius, y_radius);
	event_rect[1] = Point2D(-y_radius, x_radius);
	event_rect[2] = Point2D(-x_radius, -y_radius);
	event_rect[3] = Point2D(y_radius, -x_radius);
}


/// <summary>
/// Writes the radar events out to a saved game.
/// This routine is called by the save/load system. Every event still in play is
/// written, along with the cell of the most recent one.
/// </summary>
/// <param name="stream">The stream to write the radar events to.</param>
/// <returns>bool; Were the events written successfully?</returns>
bool RadarEventClass::Save(IStream * stream)
{
	SaveStreamClass savestream(stream, SaveStreamClass::MODE_SAVE);

	int count = RadarEvents.Count();
	savestream.Serialize(count);

	for (int index = 0; index < count; index++) {
		RadarEvents[index]->Serialize(savestream);
	}

	savestream.Serialize(LastRadarEventCell);

	return(SUCCEEDED(savestream.Result()));
}


/// <summary>
/// Reads the radar events back from a saved game.
/// This routine is called by the save/load system. Any events currently in play are
/// discarded first, so the stream's events entirely replace them.
/// </summary>
/// <param name="stream">The stream to read the radar events from.</param>
/// <returns>bool; Were the events read successfully?</returns>
bool RadarEventClass::Load(IStream * stream)
{
	for (int i = RadarEvents.Count() - 1; i >= 0; i--) {
		delete RadarEvents[i];
		RadarEvents.Delete_Index(i);
	}

	SaveStreamClass savestream(stream, SaveStreamClass::MODE_LOAD);
	savestream.Set_Context("RadarEventClass");

	int count = 0;
	savestream.Serialize(count);

	for (int index = 0; index < count; index++) {
		RadarEventClass * event = new RadarEventClass(RADAREVENT_NONE, Cell(0, 0));
		event->Serialize(savestream);
	}

	savestream.Serialize(LastRadarEventCell);

	return(SUCCEEDED(savestream.Result()));
}


/// <summary>
/// Lists the members this radar event carries.
/// </summary>
/// <param name="stream">The stream carrying the members.</param>
void RadarEventClass::Serialize(SaveStreamClass & stream)
{
	stream.Serialize(Type);
	stream.Serialize(Offset);
	stream.Serialize(Radius);
	stream.Serialize(RotationAngle);
	stream.Serialize(RotationSpeed);
	stream.Serialize(ColorFactor);
	stream.Serialize(ColorSpeed);
	stream.Serialize(Location);
	stream.Serialize(DurationTimer);
	stream.Serialize(VisibilityTimer);
	stream.Serialize(IsRotating);
	stream.Serialize(IsVisible);

	// RadarEvents -- the master list, rebuilt as the events are constructed.
}


/// <summary>
/// Fetches the bright end of this event's color cycle.
/// The event pulses between this color and the one Get_Min_Color returns, so that
/// hostile events read as orange and friendly ones as green.
/// </summary>
/// <returns>Returns with the brightest color this event will be drawn in.</returns>
RGBClass RadarEventClass::Get_Max_Color(void) const
{
	switch (Type) {
		case RADAREVENT_COMBAT:
		case RADAREVENT_BASE_ATTACKED:
		case RADAREVENT_HARVESTER_ATTACKED:
			return(RGBClass(255,128,0));

		case RADAREVENT_NONCOMBAT:
		case RADAREVENT_DROPZONE:
			return(RGBClass(0,255,0));

		default:
			return(RGBClass(255,255,0));
	}
}


/// <summary>
/// Fetches the dark end of this event's color cycle.
/// The event pulses between this color and the one Get_Max_Color returns, so that
/// hostile events read as red and friendly ones as green.
/// </summary>
/// <returns>Returns with the darkest color this event will be drawn in.</returns>
RGBClass RadarEventClass::Get_Min_Color(void) const
{
	switch (Type) {
		case RADAREVENT_COMBAT:
		case RADAREVENT_BASE_ATTACKED:
		case RADAREVENT_HARVESTER_ATTACKED:
			return(RGBClass(128,0,0));

		case RADAREVENT_NONCOMBAT:
		case RADAREVENT_DROPZONE:
			return(RGBClass(0,128,0));

		default:
			return(RGBClass(128,128,0));
	}
}


/// <summary>
/// Fetches how long this event stays visible.
/// Once the shrinking box has settled, the event holds still on the radar for this
/// long before it stops being drawn.
/// </summary>
/// <returns>Returns with the number of game frames the settled event remains drawn.</returns>
int RadarEventClass::Get_Visibility_Duration(void)
{
	return(Rule->RadarEventVisibilityDurations[Type]);
}


/// <summary>
/// Fetches the lifetime of this radar event.
/// The rules control this per event kind, so a combat ping can linger longer than a
/// dropzone marker.
/// </summary>
/// <returns>Returns with the number of game frames this event should survive.</returns>
int RadarEventClass::Get_Duration(void)
{
	return(Rule->RadarEventDurations[Type]);
}


/// <summary>
/// Fetches the suppression distance for this event.
/// A fresh event of the same kind raised within this distance of this one will be
/// swallowed rather than added to the radar.
/// </summary>
/// <returns>Returns with the suppression distance, in cells.</returns>
int RadarEventClass::Get_Suppression_Distance(void)
{
	return(Rule->RadarEventSuppressionDistances[Type]);
}


/// <summary>
/// Removes every radar event currently in play.
/// This routine is used when the radar events must be discarded wholesale, such as
/// when a scenario is torn down or a saved game is about to be read in.
/// </summary>
void RadarEventClass::Clear(void)
{
	for (int i = RadarEvents.Count() - 1; i >= 0; i--) {
		delete RadarEvents[i];
	}
}
