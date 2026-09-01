/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include "abstract.h"
#include "coord.h"
#include "types.h"
#include "vector.h"


class WaypointClass
{
public:
	WaypointClass(void);
	WaypointClass(Coord const & coord);
	~WaypointClass(void);

	bool operator==(const WaypointClass &that) const { return(Location == that.Location); }
	bool operator!=(const WaypointClass &that) const { return(Location != that.Location); }

	// Carries the waypoint to or from a save game.
	template<typename S>
	void Serialize(S & stream)
	{
		stream.Serialize(Location);
	}

	/*
	 * This is the map coordinate this waypoint marks. It is snapped to the center of its
	 * cell as the waypoint is laid down, but it keeps the height it was placed at.
	 */
	Coord Location;
};


class WaypointPathClass : public AbstractClass
{
		typedef AbstractClass BASECLASS;

	public:
		WaypointPathClass(void);
		WaypointPathClass(int index);
		virtual ~WaypointPathClass(void) override;

		virtual HRESULT STDMETHODCALLTYPE GetClassID(CLSID * retval) override;

		virtual void Serialize(SaveStreamClass & stream) override;

		virtual RTTIType Fetch_RTTI(void) const override;
		virtual void Compute_CRC(CRCEngine &) const override;

		int Current_Waypoint(void) const {return(CurrentWaypoint);}
		int Waypoint_Count(void) const {return(Waypoints.Count());}
		WaypointClass * Get_Waypoint(int index);
		bool Add_Waypoint(Coord const & coord);
		bool Select_Waypoint(Coord const & coord);
		void Replace_Waypoint(int index, Coord const & coord);
		void Delete_Waypoint(int index);
		WaypointClass * Get_Next_Waypoint(WaypointClass * wp);
		void Clear(void);

	private:
		/*
		 * This is the waypoint on this path that the player has picked up for editing, or -1
		 * if none is selected. A path with a selection is walked as a circuit rather than a
		 * line, and further clicks move the selected waypoint instead of extending the path.
		 */
		int CurrentWaypoint;

		/*
		 * These are the waypoints that make up this path, in the order they are traveled.
		 */
		DynamicVectorClass<WaypointClass> Waypoints;
};


const char *Waypoint_To_Name(WAYPOINT wp);
WAYPOINT Waypoint_From_Name(const char *string);

extern DynamicVectorClass<WaypointPathClass *> WaypointPaths;
