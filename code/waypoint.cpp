/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "always.h"

#include "waypoint.h"

#include "crc.h"
#include "savestream.h"
#include "sun.h"
#include "tracker.h"
#include "vector.h"


DynamicVectorClass<WaypointPathClass *> WaypointPaths;


/// <summary>
/// Converts a waypoint number into its display name.
/// This routine builds the short letter label that the scenario data and the map editor
/// use to identify a waypoint -- "A" through "Z", and then two letter labels beyond that.
/// </summary>
/// <param name="wp">The waypoint number to convert, or -1 for no waypoint.</param>
/// <returns>Returns with a pointer to the name, or an empty string if there is no
/// waypoint.</returns>
/// <remarks>The name is built in a shared buffer, so copy it before calling this routine
/// again.</remarks>
const char *Waypoint_To_Name(WAYPOINT wp)
{
	static char _string[4];

	if (wp == -1) {
		_string[0] = '\0';
		return(_string);
	}

	const int num_chars = ('Z' - 'A') + 1;

	if (wp < num_chars) {

		wsprintf(_string, "%c", wp + 'A');
		return(_string);
	}

	wsprintf(_string, "%c%c", (wp / num_chars) + ('A' - 1), (wp % num_chars) + 'A');
	return(_string);
}


/// <summary>
/// Converts a waypoint name back into a waypoint number.
/// This is the counterpart of Waypoint_To_Name and is used when reading waypoint labels
/// out of the scenario data.
/// </summary>
/// <param name="string">The waypoint name to convert.</param>
/// <returns>Returns with the waypoint number. Otherwise, -1 is returned if the text is not
/// a waypoint label at all.</returns>
WAYPOINT Waypoint_From_Name(const char *string)
{
	WAYPOINT wp = -1;

	const int num_chars = ('Z' - 'A') + 1;

	if (isalpha((unsigned char)string[0])) {

		wp = toupper((unsigned char)string[0]) - 'A';

		if (isalpha((unsigned char)string[1])) {
			wp = toupper((unsigned char)string[1]) + (wp * num_chars) - ('A' - num_chars);
		}
	}

	return(wp);
}


/// <summary>
/// Constructs a waypoint at the map origin.
/// The location is expected to be supplied later, either by the editing routines or by the
/// load process.
/// </summary>
WaypointClass::WaypointClass(void) :
	Location(0,0,0)
{

}


/// <summary>
/// Constructs a waypoint at the location specified.
/// </summary>
WaypointClass::WaypointClass(Coord const & coord) :
	Location(coord)
{

}


/// <summary>
/// Destroys the waypoint.
/// </summary>
WaypointClass::~WaypointClass(void)
{

}


/// <summary>
/// Constructs an empty waypoint path.
/// This routine will register the new path with the global waypoint path list so that the
/// rest of the game can find it.
/// </summary>
WaypointPathClass::WaypointPathClass(void) :
	BASECLASS(),
	CurrentWaypoint(-1),
	Waypoints()
{
	WaypointPaths.Add(this);
}


/// <summary>
/// Constructs an empty waypoint path for the path number specified.
/// The path only joins the global waypoint path list when the number falls within the
/// range of paths the game supports.
/// </summary>
/// <param name="index">The path number that this path will serve.</param>
WaypointPathClass::WaypointPathClass(int index) :
	BASECLASS(),
	CurrentWaypoint(-1),
	Waypoints()
{
	if (index >= 0 && index < 12) {
		WaypointPaths.Add(this);
	}
}


/// <summary>
/// Destroys the waypoint path.
/// This routine will break any references the game still holds to this path and then drop
/// it from the global waypoint path list.
/// </summary>
WaypointPathClass::~WaypointPathClass(void)
{
	Detach_This_From_All(this);
	WaypointPaths.Delete(this);
}


/// <summary>
/// Fetches a waypoint from this path.
/// </summary>
/// <param name="index">The waypoint on this path to fetch.</param>
/// <returns>Returns with a pointer to the waypoint. Otherwise, NULL is returned if the
/// index names no waypoint on this path.</returns>
WaypointClass * WaypointPathClass::Get_Waypoint(int index)
{
	if (index >= 0 && index < Waypoints.Count()) {
		return(&Waypoints[index]);
	}
	return(NULL);
}


/// <summary>
/// Adds a waypoint to the end of this path.
/// The location is snapped to the center of its cell, but the original height is kept. A
/// path with a waypoint selected is being edited rather than extended, so nothing is
/// appended in that case.
/// </summary>
bool WaypointPathClass::Add_Waypoint(Coord const & coord)
{
	Coord snapped = coord.As_Cell().As_Coord();
	snapped.Z = coord.Z;
	if (CurrentWaypoint == -1) {
		Waypoints.Add(WaypointClass(snapped));
	}
	return(true);
}


/// <summary>
/// Selects the waypoint that lies in the cell specified.
/// This routine is used when the player clicks on the map in order to pick up a waypoint
/// that is already part of the path. The match is made by cell, not by exact coordinate.
/// </summary>
/// <returns>bool; Was a waypoint found and selected?</returns>
bool WaypointPathClass::Select_Waypoint(Coord const & coord)
{
	for (int i = 0; i < Waypoints.Count(); i++) {
		if (Waypoints[i].Location.As_Cell() == coord.As_Cell()) {
			CurrentWaypoint = i;
			return(true);
		}
	}
	return(false);
}


/// <summary>
/// Moves an existing waypoint to a new location.
/// Nothing happens if the index names no waypoint on this path.
/// </summary>
/// <param name="index">The waypoint on this path to move.</param>
void WaypointPathClass::Replace_Waypoint(int index, Coord const & coord)
{
	WaypointClass * wp = Get_Waypoint(index);
	if (wp != NULL) {
		wp->Location = coord;
	}
}


/// <summary>
/// Removes a waypoint from this path.
/// The loop's return point stays on its waypoint, or moves to the one after it when that is
/// the waypoint removed; removing the last waypoint opens the loop. Nothing happens if the
/// index names no waypoint.
/// </summary>
/// <param name="index">The waypoint on this path to remove.</param>
void WaypointPathClass::Delete_Waypoint(int index)
{
	if (index >= 0 && index < Waypoints.Count()) {
		if (index == Waypoints.Count() - 1) {
			CurrentWaypoint = -1;
		} else if (index < CurrentWaypoint) {
			CurrentWaypoint--;
		}
		Waypoints.Delete_Index(index);
	}
}


/// <summary>
/// Fetches the waypoint that follows the one specified.
/// This routine is used to walk the path in order. A path that has a waypoint selected is
/// treated as a circuit, so the walk loops back to the selection rather than stopping.
/// </summary>
/// <param name="wp">The waypoint to step forward from.</param>
/// <returns>Returns with a pointer to the following waypoint. Otherwise, NULL is returned
/// if the path ends here.</returns>
WaypointClass * WaypointPathClass::Get_Next_Waypoint(WaypointClass * wp)
{
	int index = Waypoints.ID(wp) + 1;
	if (CurrentWaypoint != -1 && index == Waypoints.Count()) {
		index = CurrentWaypoint;
	}
	return(Get_Waypoint(index));
}


/// <summary>
/// Removes every waypoint from this path.
/// The path object itself survives; it is simply left empty and with nothing selected.
/// </summary>
void WaypointPathClass::Clear(void)
{
	Waypoints.Clear();
	CurrentWaypoint = -1;
}


/// <summary>
/// Adds the state of this waypoint path to the game checksum.
/// This routine is used by the multiplayer sync check to prove that every machine agrees
/// about this path.
/// </summary>
void WaypointPathClass::Compute_CRC(CRCEngine & crc) const
{
	BASECLASS::Compute_CRC(crc);
	crc(CurrentWaypoint);
	crc(Waypoints.Count());
}


ClassID WaypointPathClass::Class_ID(void) const
{
	return(ClassID_WaypointPath);
}


/// <summary>
/// Lists the members this waypoint path carries.
/// </summary>
/// <param name="stream">The stream carrying the members.</param>
void WaypointPathClass::Serialize(SaveStreamClass & stream)
{
	BASECLASS::Serialize(stream);

	stream.Serialize(CurrentWaypoint);
	stream.Serialize(Waypoints);
}


/// <summary>
/// Fetches the run time type of this object.
/// </summary>
/// <returns>Returns with RTTI_WAYPOINT.</returns>
RTTIType WaypointPathClass::Fetch_RTTI(void) const
{
	return(RTTI_WAYPOINT);
}
