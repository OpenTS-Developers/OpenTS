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

/* $Header: /CounterStrike/DRIVE.CPP 1     3/03/97 10:24a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : DRIVE.CPP                                                    *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : April 22, 1994                                               *
 *                                                                                             *
 *                  Last Update : October 31, 1996 [JLB]                                       *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   DriveClass::AI -- Processes unit movement and rotation.                                   *
 *   DriveClass::Approach_Target -- Handles approaching the target in order to attack it.      *
 *   DriveClass::Assign_Destination -- Set the unit's NavCom.                                  *
 *   DriveClass::Class_Of -- Fetches a reference to the class type for this object.            *
 *   DriveClass::Debug_Dump -- Displays status information to monochrome screen.               *
 *   DriveClass::Do_Turn -- Tries to turn the vehicle to the specified direction.              *
 *   DriveClass::DriveClass -- Constructor for drive class object.                             *
 *   DriveClass::Fixup_Path -- Adds smooth start path to normal movement path.                 *
 *   DriveClass::Force_Track -- Forces the unit to use the indicated track.                    *
 *   DriveClass::Lay_Track -- Handles track laying logic for the unit.                         *
 *   DriveClass::Limbo -- Prepares vehicle and then limbos it.                                 *
 *   DriveClass::Mark_Track -- Marks the midpoint of the track as occupied.                    *
 *   DriveClass::Ok_To_Move -- Checks to see if this object can begin moving.                  *
 *   DriveClass::Per_Cell_Process -- Handles when unit finishes movement into a cell.          *
 *   DriveClass::Response_Attack -- Voice feedback when ordering the unit to attack a target.  *
 *   DriveClass::Response_Move -- Voice feedback when ordering the unit to move.               *
 *   DriveClass::Response_Select -- Voice feedback when selecting the unit.                    *
 *   DriveClass::Scatter -- Causes the unit to travel to a nearby safe cell.                   *
 *   DriveClass::Smooth_Turn -- Handles the low level coord calc for smooth turn logic.        *
 *   DriveClass::Start_Of_Move -- Tries to get a unit to advance toward cell.                  *
 *   DriveClass::Stop_Driver -- Handles removing occupation bits when driving stops.           *
 *   DriveClass::Teleport_To -- Teleport object to specified location.                         *
 *   DriveClass::While_Moving -- Processes unit movement.                                      *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "drive.h"

#include "_map.h"
#include "_rules.h"
#include "anim.h"
#include "cell.h"
#include "foot.h"
#include "globals.h"
#include "house.h"
#include "infantry.h"
#include "inline.h"
#include "overtype.h"
#include "rules.h"
#include "savestream.h"
#include "tube.h"
#include "unit.h"
#include "unittype.h"
#include "wave.h"

#include "layer.hh"
#include "tube.hh"

#include <algorithm>
#include <cassert>

static const double _deaccel = 0.3f;
static const double _sinking = 0.1f;
static const double _sinking_scale = 0.0015f;


/***********************************************************************************************
 * DriveClass::DriveClass -- Constructor for drive class object.                               *
 *                                                                                             *
 *    This will initialize the drive class to its default state. It is called as a result      *
 *    of creating a unit.                                                                      *
 *                                                                                             *
 * INPUT:   classid  -- The unit's ID class. It is passed on to the foot class constructor.    *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/13/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
DriveLocomotionClass::DriveLocomotionClass(void) :
	BASECLASS(),
	CurrentRamp(0),
	PreviousRamp(0),
	RampTimer(),
	DestinationCoord(COORD_NONE),
	HeadToCoord(COORD_NONE),
	IsOnShortTrack(false),
	IsTurretLockedDown(false),
	IsRotating(false),
	IsDriving(false),
	IsRocking(false),
	IsLocomotorUnlocked(true),
	SpeedAccum(0),
	TargetSpeed(0),
	TrackNumber(-1),
	TrackIndex(-1),
	Piggybacker(NULL)
{
}


/// <summary>
/// Destroys the drive locomotor.
/// </summary>
DriveLocomotionClass::~DriveLocomotionClass(void)
{
	//nothing
}


/// <summary>
/// Fetches the class identifier of whichever locomotor is driving the unit.
/// That is the identifier of the locomotor riding along on this driver when there is one,
/// and the driver's own otherwise.
/// </summary>
/// <param name="classid">Pointer to the identifier to fill in.</param>
/// <returns>Returns with S_OK if the identifier was supplied, E_FAIL if the locomotor
/// could not be asked, or E_POINTER if no destination was supplied.</returns>
HRESULT DriveLocomotionClass::Piggyback_CLSID(CLSID * classid)
{
	if (classid == NULL) {
		return(E_POINTER);
	}

	if (Piggybacker != NULL) {
		IPersistPtr ptr(Piggybacker);
		if (ptr == NULL) {
			return(E_FAIL);
		}
		return(ptr->GetClassID(classid));
	}

	IPersistPtr ptr(this);
	if (ptr == NULL) {
		return(E_FAIL);
	}
	return(ptr->GetClassID(classid));
}


/// <summary>
/// Fetches an interface supported by this locomotor.
/// The driver answers for the piggyback interface on top of whatever the base locomotor
/// already supports.
/// </summary>
/// <param name="riid">The identifier of the interface asked for.</param>
/// <param name="ppvObject">Pointer to the interface pointer to fill in.</param>
/// <returns>Returns with S_OK if the interface was supplied, otherwise
/// E_NOINTERFACE.</returns>
HRESULT STDMETHODCALLTYPE DriveLocomotionClass::QueryInterface(REFIID riid, LPVOID * ppvObject)
{
	HRESULT result = BASECLASS::QueryInterface(riid, ppvObject);

	if (result == E_NOINTERFACE) {
		if (riid == IID_IPiggyback) {
			*ppvObject = (IPiggyback*)this;
		}
		if (*ppvObject == NULL) {
			result = E_NOINTERFACE;
		} else {
			AddRef();
			result = S_OK;
		}
	}
	return(result);
}


/// <summary>
/// Lists the members this driver carries.
/// A locomotor riding along on this one is a separate persistent object rather than a
/// member, so it still travels framed by OLE and is recreated as the class it was saved as.
/// </summary>
/// <param name="stream">The stream carrying the members.</param>
void DriveLocomotionClass::Serialize(SaveStreamClass & stream)
{
	BASECLASS::Serialize(stream);

	stream.Serialize(CurrentRamp);
	stream.Serialize(PreviousRamp);
	stream.Serialize(RampTimer);
	stream.Serialize(DestinationCoord);
	stream.Serialize(HeadToCoord);
	stream.Serialize(SpeedAccum);
	stream.Serialize(TargetSpeed);
	stream.Serialize(TrackNumber);
	stream.Serialize(TrackIndex);
	stream.Serialize(IsOnShortTrack);
	stream.Serialize(IsTurretLockedDown);
	stream.Serialize(IsRotating);
	stream.Serialize(IsDriving);
	stream.Serialize(IsRocking);
	stream.Serialize(IsLocomotorUnlocked);

	bool haspiggy = (Piggybacker != NULL);
	stream.Serialize(haspiggy);

	if (haspiggy) {
		if (stream.Is_Saving()) {
			IPersistStreamPtr persist(Piggybacker);
			OleSaveToStream(persist, stream.Get_Stream());
		} else {
			OleLoadFromStream(stream.Get_Stream(), IID_ILocomotion, (LPVOID *)&Piggybacker);
		}
	}
	// TrackControl -- constant tables shared by every driver.
	// RawTracks
	// Track1 - Track13
}


/// <summary>
/// Takes on a locomotor to ride along on top of this driver.
/// A unit that must travel in some special manner -- through a tunnel, or aboard a
/// carrier -- keeps its driver but lets the special locomotor move it for the duration.
/// </summary>
/// <param name="pointer">The locomotor that is to take over the unit.</param>
/// <returns>Returns with S_OK if the locomotor was taken on, E_FAIL if one is already
/// riding, or E_POINTER if none was supplied.</returns>
HRESULT STDMETHODCALLTYPE DriveLocomotionClass::Begin_Piggyback(ILocomotion *pointer)
{
	if (pointer == NULL) {
		return(E_POINTER);
	}
	if (Piggybacker == NULL) {
		Piggybacker = pointer;
		return(S_OK);
	}
	return(E_FAIL);
}


/// <summary>
/// Hands the piggybacking locomotor back to the caller.
/// The riding locomotor is detached and given up, leaving this driver in sole charge of
/// the unit once more.
/// </summary>
/// <param name="pointer">Pointer to the locomotor pointer to fill in.</param>
/// <returns>Returns with S_OK if a locomotor was handed back, S_FALSE if there was none
/// riding, or E_POINTER if no destination was supplied.</returns>
HRESULT DriveLocomotionClass::End_Piggyback(ILocomotion **pointer)
{
	if (pointer == NULL) {
		return(E_POINTER);
	}
	if (Piggybacker != NULL) {
		*pointer = Piggybacker;
		Piggybacker.Detach();
		return(S_OK);
	}
	return(S_FALSE);
}


/// <summary>
/// May the piggybacking locomotor hand control back now?
/// A temporary locomotor riding on this driver asks before it lets go. The driver refuses
/// while the vehicle is still under way or while it has been locked, so that control comes
/// back only once the unit has settled.
/// </summary>
/// <returns>bool; Is it safe to end the piggyback?</returns>
boolean DriveLocomotionClass::Is_Ok_To_End(void)
{
	if (!Is_Moving() && (Piggybacker != NULL && IsLocomotorUnlocked)) {
		return(true);
	}
	return(false);
}


/// <summary>
/// Fetches the tilt matrix for the slope the unit is on.
/// While the vehicle is still rocking from one slope to the next, the matrix is blended
/// between the two so that the body swings over the crest rather than jumping.
/// </summary>
/// <returns>Returns with the matrix that tilts the unit onto its slope. This is the
/// identity matrix for a unit standing on level ground.</returns>
Matrix3D DriveLocomotionClass::Get_Slope_Matrix(void) const
{
	Matrix3D mtx(true);

	if (!RampTimer.Has_Completed2()) {
		mtx = ::Get_Slope_Transition_Matrix(PreviousRamp, CurrentRamp, Get_Slope_Ratio());
	} else if (CurrentRamp) {
		mtx = ::Get_Slope_Matrix(CurrentRamp);
	}

	return(mtx);
}


/// <summary>
/// Fetches how far the unit has tilted toward its new slope.
/// </summary>
/// <returns>Returns with the progress of the slope transition, from 0 to 1.</returns>
double DriveLocomotionClass::Get_Slope_Ratio(void) const
{
	return(RampTimer.Progress());
}


/// <summary>
/// Sets the slope that the unit is driving onto.
/// The vehicle tilts from the slope it was on to the new one over a short span of time,
/// so that a tank cresting a ramp rocks over it instead of snapping to the new angle.
/// </summary>
/// <param name="ramp">The ramp the unit is moving onto.</param>
void DriveLocomotionClass::Set_Slope(int ramp)
{
	int cur = CurrentRamp;
	if (ramp != cur) {
		PreviousRamp = cur;
		CurrentRamp = ramp;
		RampTimer = 3;
	}
}


/// <summary>
/// Sets the unit's slope with no transition at all.
/// Use this routine when the vehicle should simply already be on the new slope, such as
/// when it is first placed on the map.
/// </summary>
/// <param name="ramp">The ramp the unit is to be sitting on.</param>
void STDMETHODCALLTYPE DriveLocomotionClass::Force_New_Slope(int ramp)
{
	PreviousRamp = ramp;
	CurrentRamp = ramp;
	RampTimer = 0;
}


/// <summary>
/// Does the unit still have somewhere to be?
/// A unit counts as moving while it owes a destination, and also while it is part way
/// between cells, even if it is not making any headway at this moment.
/// </summary>
/// <returns>bool; Is the unit under way or owing a move?</returns>
boolean STDMETHODCALLTYPE DriveLocomotionClass::Is_Moving(void)
{
	if (DestinationCoord != COORD_NONE) {
		return(true);
	}
	if (HeadToCoord != COORD_NONE && (HeadToCoord.X != LinkedTo->PositionCoord.X || HeadToCoord.Y != LinkedTo->PositionCoord.Y)) {
		return(true);
	}
	return(false);
}


/// <summary>
/// Is the unit visibly in motion at this instant?
/// A vehicle turning on the spot counts, as does one under way at some speed. One that
/// has been given a destination but has not gotten rolling yet does not.
/// </summary>
/// <returns>bool; Is the unit moving right now?</returns>
boolean STDMETHODCALLTYPE DriveLocomotionClass::Is_Moving_Now(void)
{
	if (LinkedTo->PrimaryFacing.Is_Rotating()) {
		return(true);
	}
	if (Is_Moving() && HeadToCoord != COORD_NONE && LinkedTo->Current_Speed() > 0) {
		return(true);
	}
	return(false);
}


/// <summary>
/// Fetches the final destination of the driving unit.
/// </summary>
/// <returns>Returns with the destination coordinate, or COORD_NONE if the unit has
/// nowhere it must be.</returns>
Coord STDMETHODCALLTYPE DriveLocomotionClass::Destination(void)
{
	return(DestinationCoord);
}


/// <summary>
/// Fetches the location the unit is immediately headed for.
/// </summary>
/// <returns>Returns with the coordinate being driven toward. A unit that is not under
/// way returns its current position instead.</returns>
Coord STDMETHODCALLTYPE DriveLocomotionClass::Head_To_Coord(void)
{
	if (HeadToCoord != COORD_NONE) {
		return(HeadToCoord);
	}
	return(LinkedTo->PositionCoord);
}


/// <summary>
/// Sets the location the unit is to drive to.
/// A stunned unit ignores the order entirely. A destination that lies beneath a bridge is
/// raised to the deck, since that is where the vehicle will actually end up driving.
/// </summary>
/// <param name="to">The location to drive to.</param>
void STDMETHODCALLTYPE DriveLocomotionClass::Move_To(Coord to)
{
	if (LinkedTo->StunDuration <= 0) {
		DestinationCoord = to;
		if (Coord(to) != COORD_NONE) {
			if (Map[to].IsUnderBridge) {
				DestinationCoord.Z += BRIDGE_LEPTON_HEIGHT;
			}
		}
	}
}


/// <summary>
/// Handles the order for the unit to stop.
/// The destination is given up and the driver begins slowing down. A train engine passes
/// the order back along the line so that every car it is pulling stops with it.
/// </summary>
void STDMETHODCALLTYPE DriveLocomotionClass::Stop_Moving(void)
{
	if (HeadToCoord != COORD_NONE) {
		if (LinkedTo->TClass->IsTrain) {
			UnitClass *unit = (UnitClass *)LinkedTo;

			if (!unit->IsFollowing) {
				UnitClass *follower = unit->FollowingMe;
				while (follower != NULL) {
					follower->Locomotion->Stop_Moving();
					follower = follower->FollowingMe;
					if (follower == NULL || follower == follower->FollowingMe) {
						break;
					}
				}
			}
		}
	}

	TargetSpeed = TargetSpeed < _deaccel ? TargetSpeed : _deaccel;
	DestinationCoord = COORD_NONE;
}


/// <summary>
/// Is the unit sitting square upon the ground?
/// The drawing code asks this to find out whether the vehicle has settled onto its slope
/// and is neither pitched nor rolled, in which case it can take the cheaper of the two
/// ways of building the unit's draw matrix.
/// </summary>
/// <returns>bool; Is the unit level and settled?</returns>
BOOL DriveLocomotionClass::Is_Angled(void) const
{
	float percent = Get_Slope_Ratio();
	if (percent == 1.0) {
		if ((float)fabs(LinkedTo->AngleRotatedSideways) < 0.005 && (float)fabs(LinkedTo->AngleRotatedForwards) < 0.005) {
			return(true);
		}
	}
	return(false);
}


/// <summary>
/// Fetches the transformation matrix to draw the unit through.
/// This routine lays the vehicle's slope tilt over the matrix the base locomotor supplies.
/// A unit that is pitched or rolled is also lifted and shoved sideways, so that the voxel
/// appears to pivot about the ground it rests on rather than about its own center.
/// </summary>
/// <param name="key">Pointer to the voxel cache key to be updated. May be NULL.</param>
/// <returns>Returns with the matrix the unit is to be rendered through.</returns>
Matrix3D STDMETHODCALLTYPE DriveLocomotionClass::Draw_Matrix(int *key)
{
	Matrix3D m;

	if (!Is_Angled()) {
		Matrix3D m1, m2;
		m1.Make_Identity();
		m2.Make_Identity();

		float val = LinkedTo->TClass->VoxelCenterY;
		float val2 = LinkedTo->TClass->VoxelCenterX;

		float fcos = std::cos(LinkedTo->AngleRotatedForwards);
		float fsin = std::sin(LinkedTo->AngleRotatedForwards);

		float scos = std::cos(LinkedTo->AngleRotatedSideways);
		float ssin = std::sin(LinkedTo->AngleRotatedSideways);

		int num = fabs(ssin) * val + fabs(fsin) * val2;

		int x_shift = (int)(val2 - (float)(int)(fcos * val2));
		int x_offset = x_shift;
		int y_shift = (int)(val - (float)(int)(scos * val));
		int y_offset = y_shift;
		if (LinkedTo->AngleRotatedForwards < 0.0f) {
			x_offset = -x_shift;
		}
		if (LinkedTo->AngleRotatedSideways > 0.0f) {
			y_offset = -y_shift;
		}

		m1.Translate_Z(num);
		m2.Translate_X(x_offset);
		m2.Translate_Y(y_offset);
		m2.Rotate_X(LinkedTo->AngleRotatedSideways);
		m2.Rotate_Y(LinkedTo->AngleRotatedForwards);

		if (key) {
			*key = -1;
		}

		Matrix3D basemtx(BASECLASS::Draw_Matrix(key));
		Matrix3D slopemtx = Get_Slope_Matrix();
		m = (((m1 * slopemtx) * basemtx) * m2);
		return(m);
	}

	if (key && *key != -1) {
		int keybase = *key << 6;
		*key = keybase;
		*key = keybase + CurrentRamp;
	}

	Matrix3D basemtx(BASECLASS::Draw_Matrix(key));
	Matrix3D slopemtx = Get_Slope_Matrix();
	m = (slopemtx * basemtx);
	return(m);
}


/// <summary>
/// Handles the unit being placed onto the map.
/// The driver adopts the slope of the cell the vehicle appears on straight away, so that
/// a unit unlimboed onto a ramp is never seen tilting itself into place.
/// </summary>
void STDMETHODCALLTYPE DriveLocomotionClass::Unlimbo(void)
{
	Force_New_Slope(LinkedTo->Get_Cell_Ptr()->Ramp);
}


/***********************************************************************************************
 * DriveClass::AI -- Processes unit movement and rotation.                                     *
 *                                                                                             *
 *    This routine is used to process unit movement and rotation. It                           *
 *    functions autonomously from the script system. Thus, once a unit                         *
 *    is give rotation command or movement path, it will follow this                           *
 *    until specifically instructed to stop. The advantage of this                             *
 *    method is that it allows smooth movement of units, faster game                           *
 *    execution, and reduced script complexity (since actual movement                          *
 *    dynamics need not be controlled directly by the scripts).                                *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   This routine relies on the process control bits for the                         *
 *             specified unit (for speed reasons). Thus, only setting                          *
 *             movement, rotation, or path list will the unit perform                          *
 *             any physics.                                                                    *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/26/1993 JLB : Created.                                                                 *
 *   04/15/1994 JLB : Converted to member function.                                            *
 *=============================================================================================*/
boolean STDMETHODCALLTYPE DriveLocomotionClass::Process(void)
{
	Set_Slope(LinkedTo->Get_Cell_Ptr()->Ramp);

	/*
	**	If the unit is following a track, then continue
	**	to do so -- mindlessly.
	*/
	if (TrackNumber != -1 && IsDriving) {

		/*
		**	Perform the movement accumulation.
		*/
		if (While_Moving()) return(false);
		if (!LinkedTo->IsActive) return(false);
		if (TrackNumber == -1 && (Is_Moving() || LinkedTo->Path[0] != FACING_NONE) && (LinkedTo->RTTI != RTTI_UNIT || !((UnitClass*)LinkedTo)->IsDumping)) {
			bool res = false;
			Start_Of_Move(res);
			if (res) return(false);
			if (!LinkedTo->IsActive) return(false);
			While_Moving(true);
			if (!LinkedTo->IsActive) return(false);
		}
	} else {

		if (LinkedTo->NavCom != NULL && LinkedTo->NavCom->RTTI == RTTI_CELL && LinkedTo->PositionCell == ((CellClass *)LinkedTo->NavCom)->CellID) {
			Abandon_Navigation();
			return(false);
		} else {

			if (LinkedTo->CurrentMission == MISSION_GUARD && !IsDriving && DestinationCoord != COORD_NONE && LinkedTo->PositionCoord == DestinationCoord) {
				Abandon_Navigation();
				return(false);
			}

			/*
			**	For tracked units that are rotating in place, perform the rotation now.
			*/
			if (LinkedTo->PrimaryFacing.Is_Rotating()) {
				IsRotating = true;
			} else {

				if (IsRotating) {
					IsRotating = false;
					LinkedTo->Per_Cell_Process(PCP_ROTATION);
					if (LinkedTo == NULL || !LinkedTo->IsActive || LinkedTo->IsInLimbo || LinkedTo->IsFalling) return(false);
				}

				/*
				**	The unit has no track to follow, but if there
				**	is a navigation target or a remaining path,
				**	then start on a new track.
				*/
				if ((LinkedTo->Mission != MISSION_GUARD || Is_Moving()) && LinkedTo->Mission != MISSION_UNLOAD) {
					if (Is_Moving() || LinkedTo->Path[0] != FACING_NONE) {

						/*
						**	Double check to make sure that the movement destination is
						**	in a zone that this unit can travel to. If not, then abort
						**	the navigation target.
						*/
						if (LinkedTo->IsLocked && LinkedTo->Mission != MISSION_ENTER && Is_Moving() && !LinkedTo->Is_In_Same_Zone(DestinationCoord)) {
							Stop_Driver();
							if (Abandon_Navigation()) {
								return(false);
							}
						} else {
							bool res = false;
							Start_Of_Move(res);
							if (res) return(false);
							if (!LinkedTo->IsActive) return(false);
							While_Moving();
							if (!LinkedTo->IsActive) return(false);
						}
					} else if (LinkedTo->IsSinking) {
						Stop_Driver();
						TargetSpeed = 0;
					} else {
						//Stop_Driver();
						if (LinkedTo->NavCom != NULL) {
							Move_To(LinkedTo->NavCom->Destination_Coord());
						}
					}
				}
			}
		}
	}

	if (Is_Moving_Now() && (Frame % 10) == 0) {
		if (!LinkedTo->IsOnBridge && LinkedTo->Get_Cell_Ptr()->Land_Type() == LAND_WATER) {
			if (Rule->Wake != NULL) {
				new AnimClass(Rule->Wake, LinkedTo->PositionCoord);
			}
		}
	}

	if (DestinationCoord == COORD_NONE && HeadToCoord == COORD_NONE) {
		if (LinkedTo->Path[0] == FACING_NONE && LinkedTo->Speed > 0) {
			LinkedTo->Set_Speed(0);
		}
	}

	return(Is_Moving());
}


/***********************************************************************************************
 * DriveClass::Mark_Track -- Marks the midpoint of the track as occupied.                      *
 *                                                                                             *
 *    This routine will ensure that the midpoint (if any) of the track that the unit is        *
 *    following, will be marked according to the mark type specified.                          *
 *                                                                                             *
 * INPUT:   headto   -- The head to coordinate.                                                *
 *                                                                                             *
 *          type     -- The type of marking to perform.                                        *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/30/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void DriveLocomotionClass::Mark_Track(Coord const & headto, MarkType type)
{
	assert(LinkedTo->IsActive);

	if (headto != COORD_NONE) {
		if (!IsOnShortTrack && TrackNumber != -1) {

			/*
			**	If we have not passed the per cell process point we need
			**	to deal with it.
			*/
			int tracknum = TrackControl[TrackNumber].Track;
			if (tracknum) {
				TrackType const * ptr = RawTracks[tracknum - 1].Track;
				int cellidx = RawTracks[tracknum - 1].Cell;
				if (cellidx > -1) {
					Dir256 dir = ptr[cellidx].Facing;

					if (TrackIndex < cellidx && cellidx != -1) {
						Point2D offset = Smooth_Turn(ptr[cellidx].Offset, dir);
						if (type == MARK_UP) {
							Coord coord(offset.X, offset.Y, LinkedTo->Height);
							LinkedTo->Clear_Occupy_Bit(coord);
						} else {
							if (type == MARK_DOWN || type == MARK_DOWN_FORCED) {
								Coord coord(offset.X, offset.Y, LinkedTo->Height);
								LinkedTo->Set_Occupy_Bit(coord);
							}
						}
					}
				}
			}
		}

		if (type == MARK_UP) {
			LinkedTo->Clear_Occupy_Bit(headto);
		} else {
			if (type == MARK_DOWN || type == MARK_DOWN_FORCED) {
				LinkedTo->Set_Occupy_Bit(headto);
			}
		}
	}
}


/***********************************************************************************************
 * DriveClass::Force_Track -- Forces the unit to use the indicated track.                      *
 *                                                                                             *
 *    This override (nuclear bomb) style routine is to be used when a unit needs to start      *
 *    on a movement track but is outside the normal movement system. This occurs when a        *
 *    harvester starts driving off of a refinery.                                              *
 *                                                                                             *
 * INPUT:   track -- The track number to start on.                                             *
 *                                                                                             *
 *          coord -- The coordinate that the unit will end up at when the movement track       *
 *                   is completed.                                                             *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/17/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void STDMETHODCALLTYPE DriveLocomotionClass::Force_Track(int track, Coord coord)
{
	assert(LinkedTo->IsActive);

	TrackNumber = track;
	TrackIndex = 0;
	if ((Coord)coord != COORD_NONE) {
		if (Start_Driver(coord)) {
			DestinationCoord = coord;
			TargetSpeed = 1.0;
		}
	}
}


/***********************************************************************************************
 * DriveClass::Stop_Driver -- Handles removing occupation bits when driving stops.             *
 *                                                                                             *
 *    This routine will remove the "reservation" flag (if present) when the vehicle is         *
 *    required to stop movement.                                                               *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  bool; Was the vehicle stopped?                                                     *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/22/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
bool DriveLocomotionClass::Stop_Driver(void)
{
	assert(LinkedTo->IsActive);

	/*
	**	We only need to do something if the vehicle is actually going
	**	somewhere.
	*/
	if (HeadToCoord != COORD_NONE) {
		HeadToCoord = COORD_NONE;
		IsDriving = false;
		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * FootClass::Start_Driver -- This starts the driver heading to the destination desired.       *
 *                                                                                             *
 *    Before a unit can move it must be started by this routine. This routine handles          *
 *    reserving the cell and setting the driving flag.                                         *
 *                                                                                             *
 * INPUT:   headto   -- The coordinate of the immediate drive destination. This is one cell    *
 *                      away from the unit's current location.                                 *
 *                                                                                             *
 * OUTPUT:  bool; Was driving initiated?                                                       *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/17/1994 JLB : Created.                                                                 *
 *   12/12/1994 JLB : Uses simple spot index finder.                                           *
 *=============================================================================================*/
bool DriveLocomotionClass::Start_Driver(Coord const & headto)
{
	assert(LinkedTo->IsActive);

	Stop_Driver();
	if (headto != COORD_NONE) {
		HeadToCoord = headto;
		IsDriving = true;

		/*
		**	Check for crate goodie finder here.
		*/
		if (Map[headto].Goodie_Check(LinkedTo) && !LinkedTo->IsInLimbo) {
			Mark_Track(headto, MARK_DOWN);
			return(true);
		}
		if (!LinkedTo->IsActive) return(false);

		HeadToCoord = COORD_NONE;
		IsDriving = false;
	}
	return(false);
}


/***********************************************************************************************
 * DriveClass::Do_Turn -- Tries to turn the vehicle to the specified direction.                *
 *                                                                                             *
 *    This routine will set the vehicle to rotate to the direction specified. For tracked      *
 *    vehicles, it is just a simple rotation. For wheeled vehicles, it performs a series       *
 *    of short drives (three point turn) to face the desired direction.                        *
 *                                                                                             *
 * INPUT:   dir   -- The direction that this vehicle should face.                              *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/29/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void DriveLocomotionClass::Do_Turn(DirType coord)
{
	assert(LinkedTo->IsActive);

	DirType dir = coord;

	//if (dir != PrimaryFacing) {

#ifdef TOFIX
		/*
		**	Special rotation track is needed for units that
		**	cannot rotate in place.
		*/
		if (Special.IsThreePoint && TrackNumber == -1 && TClass->Speed == SPEED_WHEEL) {
			int			facediff;   // Signed difference between current and desired facing.
			FacingType	face;       // Current facing (ordinal value).

			facediff = PrimaryFacing.Difference(dir) >> 5;
			facediff = Bound(facediff, -2, 2);
			if (facediff) {
				face = Dir_Facing(PrimaryFacing);

				IsOnShortTrack = true;
				Force_Track(face*FACING_COUNT + (face + facediff), Coord);

				Path[0] = FACING_NONE;
				Set_Speed(MPH_LIGHT_SPEED);		// Full speed.
			}
		} else {
			PrimaryFacing.Set_Desired(dir);
		}
#else
			LinkedTo->PrimaryFacing.Set_Desired(dir);
//			IsRotating = true;
#endif
	//}
}


/***********************************************************************************************
 * DriveClass::While_Moving -- Processes unit movement.                                        *
 *                                                                                             *
 *    This routine is used to process movement for the units as they move.                     *
 *    It is called many times for each cell's worth of movement.   This                        *
 *    routine only applies after the next cell HeadTo has been determined.                     *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  true/false; Should this routine be called again?                                   *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/02/1992 JLB : Created.                                                                 *
 *   04/15/1994 JLB : Converted to member function.                                            *
 *=============================================================================================*/
bool DriveLocomotionClass::While_Moving(bool just_started)
{
	assert(LinkedTo->IsActive);

	/*
	**	Perform quick legality checks.
	*/
	if ((!IsDriving || TrackNumber == -1) && LinkedTo->Path[0] != FACING_COUNT || (IsRotating && !LinkedTo->TClass->IsTurretEquipped)) {
		SpeedAccum = 0;		// Kludge?  No speed should accumulate if movement is on hold.
		return(false);
	}
	if (LinkedTo->TClass->IsAccelerates) {
		bool not_passive = LinkedTo->RTTI != RTTI_UNIT || !((UnitClass *)LinkedTo)->Class->IsPassive;
		if (TrackNumber < BACKUP_INTO_REFINERY && not_passive) {
			Coord dest_coord = DestinationCoord;
			dest_coord.Z = Map.Get_Height_GL(dest_coord) + (Map[dest_coord].IsUnderBridge ? BRIDGE_LEPTON_HEIGHT : 0);
			int distance = (LinkedTo->PositionCoord - dest_coord).Length();//LinkedTo->PositionCoord.Distance_To(dest_coord);

			bool forced_speed = false;
			double speed = LinkedTo->Speed;

			TechnoTypeClass const * tclass = LinkedTo->TClass;
			int maxspeed = LinkedTo->Get_Max_Speed();

			if (distance < tclass->SlowdownDistance) {
				forced_speed = true;
				speed = std::max(_deaccel, speed - maxspeed * tclass->DeaccelerationFactor);
			} else if (LinkedTo->IsSinking) {
				speed = std::max(_sinking, speed - maxspeed * _sinking_scale);
				forced_speed = true;
			}

			if (LinkedTo->IsCrushing) {
				TargetSpeed = std::min(TargetSpeed, 0.2);
				LinkedTo->Set_Speed(TargetSpeed);
			} else if (forced_speed) {
				LinkedTo->Set_Speed(speed);
			} else if (LinkedTo->Speed < TargetSpeed) {
				LinkedTo->Set_Speed(std::min(TargetSpeed, LinkedTo->Speed + tclass->AccelerationFactor));
			} else if (LinkedTo->Speed > TargetSpeed) {
				LinkedTo->Set_Speed(std::max(TargetSpeed, LinkedTo->Speed - maxspeed * tclass->DeaccelerationFactor));
			}

			if (LinkedTo->RTTI == RTTI_UNIT) {
				UnitClass * following = ((UnitClass *)LinkedTo)->FollowingMe;
				if (following != NULL) {
					do {
						following->Set_Speed(LinkedTo->Speed);
						following = following->FollowingMe;
					} while (following && following != following->FollowingMe);
				}
			}
		}
	} else {
		LinkedTo->Set_Speed(TargetSpeed);
	}

	/*
	**	If enough movement has accumulated so that the unit can
	**	visibly move on the map, then process accordingly.
	**	Slow the unit down if he's carrying a flag.
	*/
	int maxspeed = LinkedTo->Current_Speed();
	//if (IsFormationMove) maxspeed = FormationMaxSpeed;

	FacingType		nextface;		// Next facing queued in path.
	nextface = LinkedTo->Path[0];

	int actual;			// Working movement addition value.
	actual = SpeedAccum + (just_started ? 0 : maxspeed);

	if (nextface == FACING_COUNT && TrackNumber == -1) {
		LinkedTo->Mark(MARK_UP);
		Stop_Driver();

		TubeType tubenum = (TubeType)LinkedTo->Get_Cell_Ptr()->Tube;
		if (tubenum >= TUBE_FIRST && tubenum < Tubes.Count()) {
			TubeClass * tube = Tubes[tubenum];
			Cell c = tube->Exit;
			HeadToCoord = c.As_Coord();
			LinkedTo->Advance_Path(1);
			LinkedTo->CurrentTube = tubenum;
			LinkedTo->CurrentTubeDir = FACING_FIRST;
			LinkedTo->LastTubeCoord = Map[Adjacent_Cell((Cell)tube->Enter, tube->Dirs[0])].Cell_Coord();
			int current_height = Map.Get_Height_GL(LinkedTo->PositionCoord);
			int count = tube->Count;
			c = tube->Exit;
			LinkedTo->LastTubeCoord.Z = current_height + (Map.Get_Height_GL(c) - current_height) / count;
			IsDriving = true;
			TrackNumber = -1;
		} else {
			LinkedTo->Path[0] = FACING_NONE;
			TrackNumber = -1;
			HeadToCoord = COORD_NONE;
			Stop_Driver();
		}
		return(false);
	}

	if (actual > ((PIXEL_LEPTON_W + PIXEL_LEPTON_H) / 2)) {
		TurnTrackType	const * track;  // Track control pointer.
		TrackType		const	* ptr;  // Pointer to coord offset values.
		int				tracknum;       // The track number being processed.
		bool				adj;        // Is a turn coming up?

		track = &TrackControl[TrackNumber];
		if (IsOnShortTrack) {
			tracknum = track->StartTrack;
		} else {
			tracknum = track->Track;
		}
		ptr = RawTracks[tracknum-1].Track;
		//nextface = LinkedTo->Path[0];

		if (nextface < FACING_NONE || nextface > FACING_COUNT) {
			LinkedTo->Path[0] = FACING_NONE;
			return(false);
		}

		/*
		**	Determine if there is a turn coming up. If there is
		**	a turn, then track jumping might occur.
		*/
		adj = false;
		if (nextface != FACING_COUNT && nextface != FACING_NONE && Dir_Facing(track->Facing) != nextface) {
			adj = true;
		}

		/*
		**	Skip ahead the number of track steps required (limited only
		**	by track length). Set the unit to the new position and
		**	flag the unit accordingly.
		*/
		//LinkedTo->Mark(MARK_UP);
		while (actual > (PIXEL_LEPTON_W + PIXEL_LEPTON_H) / 2) {
			Point2D	offset;
			Dir256	dir;

			actual -= (PIXEL_LEPTON_W + PIXEL_LEPTON_H) / 2;

			offset = ptr[TrackIndex].Offset;
			if (offset != Point2D(0, 0) || !TrackIndex) {
				if (LinkedTo->Occupies_Cells()) {
					LinkedTo->Clear_Occupy_Bit(LinkedTo->PositionCoord);
					LinkedTo->IsOccupyingCell = false;
					LinkedTo->IsToPathAroundBlockage = false;
				}

				Cell oldcell;
				if (!TrackIndex) {
					oldcell = LinkedTo->PositionCell;
				} else {
					dir = ptr[TrackIndex - 1].Facing;
					Point2D prevoffset = ptr[TrackIndex - 1].Offset;
					oldcell = Coord(Smooth_Turn(prevoffset, dir), 0).As_Cell();
				}

				dir = ptr[TrackIndex].Facing;
				Coord newcoord = Coord(Smooth_Turn(offset, dir), 0);
				Cell newcell = newcoord.As_Cell();

				if (LinkedTo->PositionCoord.As_Cell() != newcoord.As_Cell()) {
					LinkedTo->Mark(MARK_UP);
					LinkedTo->PositionCoord = newcoord;
					CellClass * oldcellptr = &Map[oldcell];
					CellClass * newcellptr = &Map[newcell];
					if (newcellptr->Height == oldcellptr->Height - BRIDGE_CELL_HEIGHT && newcellptr->IsUnderBridge) {
						LinkedTo->IsOnBridge = true;
					}
					if (!newcellptr->IsUnderBridge && oldcellptr->IsUnderBridge) {
						LinkedTo->IsOnBridge = false;
					}
					if (LinkedTo->TClass->IsTrain && !((UnitClass *)LinkedTo)->IsFollowing) {
						bool is_bridge = LinkedTo->IsOnBridge || (LinkedTo->PositionCoord.Z >= (Map.Get_Height_GL(LinkedTo->PositionCoord) + BRIDGE_LEPTON_HEIGHT));
						ObjectClass * occupier;
						if (is_bridge) {
							occupier = Map[newcell].Cell_Occupier(true);
						} else {
							occupier = Map[newcell].Cell_Occupier(false);
						}
						while (occupier != NULL) {
							ObjectClass * next = occupier->Next;
							if (!occupier->Class_Of()->IsCrushable) {
								int strength = 10000;
								occupier->Take_Damage(strength, 0, Rule->C4Warhead, NULL, true, true);
								strength = 20;
								LinkedTo->Take_Damage(strength, 0, Rule->C4Warhead, NULL, true, false);
							}
							occupier = next;
						}
					}
					if (!LinkedTo->IsActive) return(false);
					LinkedTo->Mark(MARK_DOWN);

					if (newcellptr->Overlay != OVERLAY_NONE) {
						OverlayTypeClass * otype = OverlayTypes[newcellptr->Overlay];
						if (IsRocking && (LinkedTo->TClass->IsCrusher || LinkedTo->Has_Ability(ABILITY_CRUSHER)) && otype->HeapID == OVERLAY_SANDBAG_WALL) {
							LinkedTo->IsCrushing = true;
							if (LinkedTo->TClass->IsTiltsWhenCrushes) {
								LinkedTo->RockingForwardsPerFrame = -0.02f;
							}
						}
					}
				} else {
					bool down = LinkedTo->IsDown;
					LinkedTo->IsDown = false;
					LinkedTo->PositionCoord = newcoord;
					LinkedTo->IsDown = down;
				}

				if (!LinkedTo->IsActive) {
					return(false);
				}

				bool down = LinkedTo->IsDown;
				LinkedTo->IsDown = false;
				LinkedTo->HeightAGL = 0;
				LinkedTo->IsDown = down;

				LinkedTo->PrimaryFacing.Set(dir);

				/*
				**	See if "per cell" processing is necessary.
				*/
				if (TrackIndex && RawTracks[tracknum-1].Cell == TrackIndex) {
					LinkedTo->Clear_Occupy_Bit(LinkedTo->PositionCoord);
				}

				/*
				**	The unit could "jump tracks". Check to see if the unit should
				**	do so.
				*/
				if (nextface != FACING_COUNT && nextface != FACING_NONE && adj && RawTracks[tracknum-1].Jump == TrackIndex && TrackIndex) {
					TurnTrackType const * newtrack;		// Proposed jump-to track.
					int	tnum;

					tnum = (int)(Dir_Facing(track->Facing) * FACING_COUNT) + (int)nextface;
					newtrack = &TrackControl[tnum];
					if (newtrack->Track && RawTracks[newtrack->Track-1].Entry) {
						double oldspeed = LinkedTo->Speed;

						Coord c = Adjacent_Cell(HeadToCoord, nextface);

						switch (LinkedTo->Can_Enter_Cell(&Map[c.As_Cell()], nextface, LinkedTo->Get_Cell_Height())) {
							case MOVE_OK:
							case MOVE_MOVING_BLOCK:
								if (LinkedTo->RTTI == RTTI_UNIT && !((UnitClass *)LinkedTo)->Class->IsPassive) break;
								IsOnShortTrack = false;		// Shouldn't be necessary, but...
								TrackNumber = tnum;
								track = newtrack;

								tracknum = track->Track;
								TrackIndex = RawTracks[tracknum-1].Entry-1;	// Anticipate increment.
								ptr = RawTracks[tracknum-1].Track;
								adj = false;

								Stop_Driver();
								IsDriving = true;
								LinkedTo->Per_Cell_Process(PCP_END);
								IsDriving = false;
								if (LinkedTo == NULL || !LinkedTo->IsActive || LinkedTo->IsInLimbo || LinkedTo->IsFalling) return(false);
								if (Start_Driver(c)) {
									LinkedTo->Set_Speed(oldspeed);
									LinkedTo->Advance_Path(1);
								}
								break;

							case MOVE_CLOAK:
								Map[c].Shimmer();
								break;

							case MOVE_CLOSED_GATE:
								Map.Try_Open_Gate(LinkedTo, c.As_Cell());
								break;

							case MOVE_TEMP: {
									bool bridge;
									if (!Map[c].IsUnderBridge || abs(LinkedTo->PositionCoord.Z / LEVEL_LEPTON_H - Map[c].Height) <= 2) {
										bridge = false;
									} else {
										bridge = true;
									}
									Map[HeadToCoord].Incoming(COORD_NONE, true, true, bridge);
								}
								break;
						}
					}
				}
				TrackIndex++;

			} else {
				Coord dc = HeadToCoord - LinkedTo->PositionCoord;
				int d = abs(dc.X) + abs(dc.Y);
				actual += (int)((1.0 - (double)d / 11.0) * 7.0);
				LinkedTo->IsOccupyingCell = true;
				LinkedTo->IsToPathAroundBlockage = false;

				if (HeadToCoord.As_Cell() != LinkedTo->PositionCoord.As_Cell()) {
					LinkedTo->Mark(MARK_UP);
					LinkedTo->PositionCoord = HeadToCoord;
					LinkedTo->HeightAGL = 0;
					LinkedTo->Mark(MARK_DOWN);
				} else {
					bool down = LinkedTo->IsDown;
					LinkedTo->IsDown = false;
					LinkedTo->PositionCoord = HeadToCoord;
					LinkedTo->HeightAGL = 0;
					LinkedTo->IsDown = down;
				}

				//actual = 0;
				//DestinationCoord = Head_To_Coord();
				Stop_Driver();
				TrackNumber = -1;
				TrackIndex = NULL;

				bool arrived = false;
				if (LinkedTo->NavCom != NULL && LinkedTo->PositionCell == LinkedTo->NavCom->Destination_Coord().As_Cell()) {
					if (abs(LinkedTo->Destination_Coord().Z - DestinationCoord.Z) < 2 * LEVEL_LEPTON_H) {
						arrived = true;
						DestinationCoord = COORD_NONE;
						Stop_Driver();
						IsDriving = false;
					}
				}

				/*
				**	Perform "per cell" activities.
				*/
				//LinkedTo->Mark(MARK_DOWN);
				LinkedTo->Per_Cell_Process(PCP_END);
				if (LinkedTo == NULL || !LinkedTo->IsActive || LinkedTo->IsInLimbo || LinkedTo->IsFalling) return(true);
				//LinkedTo->Mark(MARK_UP);

				if (arrived) {
					LinkedTo->NavCom = NULL;
					LinkedTo->Path[0] = FACING_NONE;
					if (LinkedTo->Mission == MISSION_MOVE && LinkedTo->Enter_Idle_Mode()) {
						return(true);
					}
				}

				if (LinkedTo->Is_Docked_For_Repair()) {
					return(true);
				}

				if (!LinkedTo->IsActive) return(false);

				break;
			}
		}
		// if (LinkedTo->IsActive) {
		// 	LinkedTo->Mark(MARK_DOWN);
		// }
	}

	/*
	**	Replace any remainder back into the unit's movement
	**	accumulator to be processed next pass.
	*/
	SpeedAccum = actual;

	if (actual > 0 && TrackNumber > -1) {
		TurnTrackType	const * track;  // Track control pointer.
		TrackType		const	* ptr;  // Pointer to coord offset values.
		int				tracknum;       // The track number being processed.

		track = &TrackControl[TrackNumber];
		if (IsOnShortTrack) {
			tracknum = track->StartTrack;
		} else {
			tracknum = track->Track;
		}
		ptr = RawTracks[tracknum-1].Track;

		Point2D	offset;
		Dir256	dir;

		offset = ptr[TrackIndex].Offset;
		if (offset != Point2D(0, 0) || !TrackIndex) {

			Coord oldcoord = LinkedTo->PositionCoord;

			dir = ptr[TrackIndex].Facing;
			Coord movement = Coord(Smooth_Turn(offset, dir) - LinkedTo->PositionCoord, 0);
			Coord stepcoord = movement + LinkedTo->PositionCoord;
			CellClass * stepcellptr = &Map[stepcoord];

			Coord partial(0, 0, 0);
			partial = Lerp(partial, movement, SpeedAccum / 7.0); /// 7 == (CELL_LEPTON / ((CELL_PIXEL_W + CELL_PIXEL_H) / 2))
			Coord partialcoord = oldcoord + partial;

			CellClass * oldcellptr = &Map[oldcoord];
			CellClass * partialcellptr = &Map[partialcoord];

			Coord poscoord = LinkedTo->PositionCoord;
			Coord coord = poscoord;

			if (partialcellptr != stepcellptr && partialcellptr != oldcellptr) {
				if (SpeedAccum > 3) {
					coord = stepcoord;
					coord.Z = poscoord.Z;
				}
			} else {
				coord = partialcoord;
				coord.Z = poscoord.Z;
			}

			CellClass * newcellptr = &Map[coord];

			if (coord.As_Cell() != LinkedTo->PositionCoord.As_Cell()) {
				LinkedTo->Mark(MARK_UP);
				LinkedTo->PositionCoord = coord;
				if (newcellptr->Height == oldcellptr->Height - BRIDGE_CELL_HEIGHT && newcellptr->IsUnderBridge) {
					LinkedTo->IsOnBridge = true;
				}
				if (!newcellptr->IsUnderBridge && oldcellptr->IsUnderBridge) {
					LinkedTo->IsOnBridge = false;
				}
				LinkedTo->Mark(MARK_DOWN);
			} else {
				bool down = LinkedTo->IsDown;
				LinkedTo->IsDown = false;
				LinkedTo->PositionCoord = coord;
				LinkedTo->IsDown = down;
			}
		}
	}

	return(false);
}


/***********************************************************************************************
 * DriveClass::Start_Of_Move -- Tries to get a unit to advance toward cell.                    *
 *                                                                                             *
 *    This will try to start a unit advancing toward the cell it is                            *
 *    facing. It will check for and handle legality and reserving of the                       *
 *    necessary cell.                                                                          *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  true/false; Should this routine be called again because                            *
 *                      initial start operation is temporarily delayed?                        *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/02/1992 JLB : Created.                                                                 *
 *   10/18/1993 JLB : This should be called repeatedly until HeadTo is not NULL.               *
 *   03/16/1994 JLB : Revamped for track logic.                                                *
 *   04/15/1994 JLB : Converted to member function.                                            *
 *   06/19/1995 JLB : Fixed so that it won't fire on ground unnecessarily.                     *
 *   07/13/1995 JLB : Handles bumping into cloaked objects.                                    *
 *   09/22/1995 JLB : Breaks out of hopeless hunt mode.                                        *
 *   07/10/1996 JLB : Sets scan limit if necessary.                                            *
 *=============================================================================================*/
bool DriveLocomotionClass::Start_Of_Move(bool & stop_processing, bool retry, bool force_straight)
{
	assert(LinkedTo->IsActive);

	FacingType		facing;				// Direction movement will commence.

	facing = LinkedTo->Path[0];

	if (!Is_Moving() && facing == FACING_NONE) {
		IsTurretLockedDown = false;
		Stop_Driver();
		if (LinkedTo->Mission == MISSION_MOVE) {
			stop_processing = LinkedTo->Enter_Idle_Mode();
		}
		return(false);		// Why is it calling this routine!?!
	}

	if (DestinationCoord == COORD_NONE) return(false);

	if (LinkedTo->StunDuration > 0) return(true);

	/*
	**	Reduce the path length if the target is a unit and the
	**	range to the unit is less than the precalculated path steps.
	*/
	if (facing != FACING_NONE) {
		if (dynamic_cast<UnitClass *>(LinkedTo->NavCom) || dynamic_cast<InfantryClass *>(LinkedTo->NavCom)) {
			int dist = Lepton_To_Cell((LEPTON)LinkedTo->Distance(DestinationCoord));

			if (dist < ARRAY_SIZE(LinkedTo->Path)) {
				LinkedTo->Path[dist] = FACING_NONE;
				facing = LinkedTo->Path[0];		// Maybe needed.
			}
		}
	}

	/*
	**	If the path is invalid at this point, then generate one. If
	**	generating a new path fails, then abort NavCom.
	*/
	if (facing == FACING_NONE) {

		/*
		**	If after a path search, there is still no valid path, then set the
		**	NavCom to null and let the script take care of assigning a new
		**	navigation target.
		*/
		if (LinkedTo->PathDelay != 0) {
			return(false);
		}
		LinkedTo->PathDelay = Rule->PathDelay * TICKS_PER_MINUTE;

		if (!LinkedTo->Basic_Path(DestinationCoord.As_Cell())) {
			if (LinkedTo == NULL) {
				stop_processing = true;
				return(false);
			}
			if (LinkedTo->Is_In_Same_Zone(DestinationCoord)) {
				if (DestinationCoord == COORD_NONE) return(false);

				/*
				**	If the unit is close enough to the target then just stop
				**	driving now. This prevents the fidgeting that would occur
				**	if they mindlessly kept trying to get to the exact location
				**	desired. This is quite necessary since it is typical to move
				**	several units with the same mouse click.
				*/
				if (!LinkedTo->Is_On_Priority_Mission() && LinkedTo->Distance(DestinationCoord) < Rule->CloseEnoughDistance && (LinkedTo->Mission == MISSION_MOVE || LinkedTo->Mission == MISSION_GUARD_AREA)) {
					Stop_Driver();
					if (Abandon_Navigation()) return(true);
					if (!LinkedTo->IsActive) return(false);
				} else {
					/*
					**	If a basic path could not be found, but the immediate move destination is
					**	blocked by a friendly temporary blockage, then cause that blockage
					**	to scatter.
					*/
					Cell cell = Adjacent_Cell(LinkedTo->Center_Coord().As_Cell(), LinkedTo->PrimaryFacing.Current().As_Dir8());
					if (Map.In_Local_Radar(cell)) {
						MoveType ok = LinkedTo->Can_Enter_Cell(&Map[cell], LinkedTo->PrimaryFacing.Current().As_Dir8(), LinkedTo->Get_Cell_Height());
						if (ok == MOVE_CLOSED_GATE) {
							Incoming(cell);
						} else if (ok == MOVE_TEMP) {
							CellClass * cellptr = &Map[cell];
							TechnoClass * blockage = cellptr->Cell_Techno(Point2D(0,0), LinkedTo->PositionCoord.Z > (Map.Get_Height_GL(cell) + (2 * LEVEL_LEPTON_H)));
							if (blockage != NULL && LinkedTo->House->Is_Ally(blockage) && !LinkedTo->TClass->IsTrain) {

								/*
								**	If the target can be told to get out of the way, only bother
								**	to do so if we aren't very close to the target and this
								**	object can just say "good enough" and stop here.
								*/
								if (LinkedTo->Distance(DestinationCoord) < Rule->CloseEnoughDistance && !LinkedTo->In_Radio_Contact()) {
									if (abs(DestinationCoord.Z - LinkedTo->PositionCoord.Z) < 2 * LEVEL_LEPTON_H) {
										if (Map[(Coord const &)LinkedTo->PositionCoord].Land_Type() != LAND_TUNNEL) {
											Stop_Driver();
											return(Abandon_Navigation());
										}
									}
								}

								bool is_bridge;
								if (!cellptr->IsUnderBridge || abs(LinkedTo->PositionCoord.Z / LEVEL_LEPTON_H - cellptr->Height) <= 2) {
									is_bridge = false;
								} else {
									is_bridge = true;
								}
								cellptr->Incoming(COORD_NONE, true, true, is_bridge);
							}
						}
					}

					if (LinkedTo->TryTryAgain > 0) {
						LinkedTo->TryTryAgain--;
					} else {
						Stop_Driver();
						if (Abandon_Navigation()) return(true);
						if (!LinkedTo->IsActive) return(false);
						if (LinkedTo->IsNewNavCom) Sound_Effect(Rule->ScoldSound);
						LinkedTo->IsNewNavCom = false;
					}
				}

				/*
				**	Since the path was blocked, check to make sure that it was completely
				**	blocked. If so and it has a valid TarCom and it is out of range of the
				**	TarCom, then give this unit a range limit so that it might not pick
				**	a "can't reach" target again.
				*/
				if (!Is_Moving() && LinkedTo->TarCom != NULL && !LinkedTo->In_Range(LinkedTo->TarCom)) {
					LinkedTo->IsScanLimited = true;
					if (LinkedTo->Team != NULL) LinkedTo->Team->Scan_Limit();
					LinkedTo->Assign_Target(NULL);
				}

				/*
				**	Stop the movement, for now, and let the subsequent logic in later game
				**	frames resume movement as appropriate.
				*/
				Stop_Driver();
				TrackNumber = -1;
				IsTurretLockedDown = false;
				return(false);
			} else {
				LinkedTo->Assign_Destination(NULL);
				return(false);
			}
		}

		if (LinkedTo->Path[0] == FACING_COUNT) {
			return(false);
		}

		/*
		**	If a basic path could be found, but the immediate move destination is
		**	blocked by a friendly temporary blockage, then cause that blockage
		**	to scatter.
		*/
		Cell cell = Adjacent_Cell(LinkedTo->Center_Coord().As_Cell(), LinkedTo->Path[0]);
		if (Map.In_Local_Radar(cell)) {
			MoveType ok = LinkedTo->Can_Enter_Cell(&Map[cell], LinkedTo->Path[0], LinkedTo->Get_Cell_Height());

			if (ok == MOVE_CLOSED_GATE) {
				Incoming(cell);
			} else if (ok == MOVE_TEMP) {
				CellClass * cellptr = &Map[cell];
				TechnoClass * blockage = cellptr->Cell_Techno(Point2D(0,0), LinkedTo->PositionCoord.Z > (Map.Get_Height_GL(cell) + (2 * LEVEL_LEPTON_H)));
				if (blockage != NULL && LinkedTo->House->Is_Ally(blockage) && !LinkedTo->TClass->IsTrain) {

					/*
					**	If the target can be told to get out of the way, only bother
					**	to do so if we aren't very close to the target and this
					**	object can just say "good enough" and stop here.
					*/
					if (LinkedTo->Distance(DestinationCoord) < Rule->CloseEnoughDistance && !LinkedTo->In_Radio_Contact()) {
						if (abs(DestinationCoord.Z - LinkedTo->PositionCoord.Z) < 2 * LEVEL_LEPTON_H) {
							if (Map[(Coord const &)LinkedTo->PositionCoord].Land_Type() != LAND_TUNNEL) {
								Stop_Driver();
								return(Abandon_Navigation());
							}
						}
					}

					bool is_bridge;
					if (!cellptr->IsUnderBridge || abs(LinkedTo->PositionCoord.Z / LEVEL_LEPTON_H - cellptr->Height) <= 2) {
						is_bridge = false;
					} else {
						is_bridge = true;
					}
					cellptr->Incoming(COORD_NONE, true, true, is_bridge);
				}
			}
		}

		LinkedTo->TryTryAgain = FootClass::PATH_RETRY;
		facing = LinkedTo->Path[0];
	}

	/*
	 * Beyond this point, actual track assignment can begin. This is reached either
	 * because the path was already valid (facing != FACING_NONE on entry) or because
	 * a new path was just successfully generated.
	 */
	if (facing == FACING_COUNT) {
		return(false);
	}

	/*
	**	Determine the coordinate of the next cell to move into.
	*/
	Coord dest = Adjacent_Cell(LinkedTo->PositionCoord, facing);

	int cell_height = Map[(Coord const &)LinkedTo->PositionCoord].Height + (LinkedTo->IsOnBridge ? BRIDGE_CELL_HEIGHT : 0);

	/*
	 * If this move crosses a bridge transition, force a full scan next process.
	 */
	if (LinkedTo->IsOnBridge ^ Map[Adjacent_Cell(LinkedTo->PositionCoord, facing)].IsUnderBridge) {
		LinkedTo->IsPlanningToLook = true;
	}
	if (!LinkedTo->Is_Ready_To_Move()) {
		return(true);
	}
	if (!Incoming(dest.As_Cell())) {
		return(true);
	}

	/*
	**	Set the facing correctly if it isn't already correct. This
	**	means starting a rotation track if necessary.
	*/
	if (DIR_MIN < LinkedTo->PrimaryFacing.Current() - DirType(facing)) {

		/*
		**	Request a change of facing.
		*/
		Do_Turn(DirType(facing));
		return(true);

	} else {

		/* NOTE:  Beyond this point, actual track assignment can begin.
		**
		**	If the cell to move into is impassable (probably for some unexpected
		**	reason), then abort the path list and set the speed to zero. The
		**	next time this routine is called, a new path will be generated.
		*/
		Cell destcell = dest.As_Cell();
		CellClass * destptr = &Map[destcell];
		bool crushable_dest = false;
		LinkedTo->Mark(MARK_UP);
		MoveType cando = LinkedTo->Can_Enter_Cell(destptr, facing, cell_height);
		LinkedTo->Mark(MARK_DOWN);

		/*
		 * A train ignores all blockages, and a crusher rolls straight over sandbags.
		 */
		if ((cando < MOVE_NO && LinkedTo->TClass->IsTrain)
			|| ((cando == MOVE_DESTROYABLE || cando == MOVE_FRIENDLY_DESTROYABLE) && LinkedTo->TClass->IsCrusher && destptr->Overlay == OVERLAY_SANDBAG_WALL)) {
			cando = MOVE_OK;
		}


		/*
		 * If the destination cell holds a crushable overlay, then remember to drive
		 * straight through it rather than curving around.
		 */
		if (destptr->Overlay != OVERLAY_NONE && cando == MOVE_OK && OverlayTypes[destptr->Overlay]->IsCrushable) {
			crushable_dest = true;
		}

		if (cando != MOVE_OK) {
			/*
			 * The destination cell cannot be entered. Handle each blockage type.
			 */
			if (cando == MOVE_CLOSED_GATE) {
				Incoming(destcell);
			}

			/*
			**	If a temporary friendly object is blocking the path, then cause it to
			**	get out of the way.
			*/
			else if (cando == MOVE_TEMP) {
				if (!LinkedTo->TClass->IsTrain) {
					if (retry) {
						LinkedTo->Path[0] = FACING_NONE;
						LinkedTo->PathDelay = 0;
						return(Start_Of_Move(stop_processing, false, false));
					}
					Coord diff = LinkedTo->Center_Coord() - DestinationCoord;
					int dist = diff.Length();
					if (dist >= Rule->CloseEnoughDistance
						|| abs(DestinationCoord.Z - LinkedTo->PositionCoord.Z) >= 2 * LEVEL_LEPTON_H
						|| Map[(Coord const &)LinkedTo->PositionCoord].Land_Type() == LAND_TUNNEL) {
						bool is_bridge;
						if (!Map[destcell].IsUnderBridge || abs(LinkedTo->PositionCoord.Z / LEVEL_LEPTON_H - Map[destcell].Height) <= 2) {
							is_bridge = false;
						} else {
							is_bridge = true;
						}
						Map[destcell].Incoming(COORD_NONE, true, true, is_bridge);
					} else {
						Stop_Driver();
						if (Abandon_Navigation()) {
							return(true);
						}
					}
				}

			/*
			**	If a cloaked object is blocking, then shimmer the cell.
			*/
			} else if (cando == MOVE_CLOAK) {
				Map[destcell].Shimmer();
				if (retry) {
					LinkedTo->Path[0] = FACING_NONE;
					return(Start_Of_Move(stop_processing, false, false));
				}

				Stop_Driver();
				return(Abandon_Navigation());
			}

			Stop_Driver();
			if (cando == MOVE_MOVING_BLOCK) {

				/*
				 * If this is a temporary blockage, try once more to path around it.
				 */
				if (!LinkedTo->IsToPathAroundBlockage) {
					LinkedTo->IsToPathAroundBlockage = true;
					LinkedTo->BlockagePathDelay = Rule->BlockagePathDelay;
				}
				if (LinkedTo->PathDelay == 0) {
					bool around_blockage = LinkedTo->IsToPathAroundBlockage && LinkedTo->BlockagePathDelay == 0;
					bool ok = LinkedTo->Basic_Path(DestinationCoord.As_Cell(), 0, (around_blockage != 0) + 1);
					if (LinkedTo == NULL) {
						stop_processing = true;
						return(false);
					}
					if (ok || LinkedTo->Is_In_Same_Zone(DestinationCoord)) {
						LinkedTo->PathDelay = Rule->PathDelay * TICKS_PER_MINUTE;
						return(true);
					}
					LinkedTo->Assign_Destination(NULL);
					return(false);
				}
			}
			/*
			** If blocked by a moving block then just exit start of move and
			** try again next tick.
			*/
			if (cando == MOVE_DESTROYABLE || cando == MOVE_FRIENDLY_DESTROYABLE) {
				if (retry) {
					LinkedTo->Path[0] = FACING_NONE;		// Path is blocked!
					LinkedTo->PathDelay = 0;
					return(Start_Of_Move(stop_processing, false, false));
				}

				/*
				 * An enemy unit, building, or wall is blocking. Pick a fight with it.
				 */
				if (Map[destcell].Cell_Object()) {
					if (!LinkedTo->House->Is_Ally(Map[destcell].Cell_Object())) {
						LinkedTo->Override_Mission(MISSION_ATTACK, Map[destcell].Cell_Object(), NULL);
					}
				} else {
					if (Map[destcell].Overlay != OVERLAY_NONE && OverlayTypes[Map[destcell].Overlay]->IsWall) {
						LinkedTo->Override_Mission(MISSION_ATTACK, &Map[destcell], NULL);
					}
				}
			} else {
				if (LinkedTo->IsNewNavCom) Sound_Effect(Rule->ScoldSound);
			}


			if (cando == MOVE_NO && retry) {
				LinkedTo->Path[0] = FACING_NONE;
				LinkedTo->PathDelay = 0;
				return(Start_Of_Move(stop_processing, false, false));
			} else if (cando == MOVE_NO && !retry) {
				Stop_Driver();
				return(Abandon_Navigation());
			}

			LinkedTo->IsNewNavCom = false;
			TrackNumber = -1;
			return(true);
		}

		/*
		 * Determine the speed that the unit can travel to the desired square.
		 * The terrain cost is a fractional multiplier (1.0 == full speed). It
		 * is clamped to a maximum of full speed and scaled according to whether
		 * the unit is travelling uphill or downhill.
		 */
		int height;
		LandType ground;
		if (abs(cell_height - Map[destcell].Height) < 2) {
			height = Map[destcell].Height;
			ground = Map[destcell].Land_Type();
		} else {
			height = cell_height;
			ground = LAND_ROAD;
		}

		double speed = Ground[ground].Cost[LinkedTo->TClass->Speed];
		if (speed > 1.0) speed = 1.0;

		int destheight = Map.Get_Height_GL(Map[destcell].Cell_Coord());
		int unitheight = Map.Get_Height_GL(LinkedTo->PositionCoord);
		if (destheight > unitheight) {
			if (LinkedTo->RTTI == RTTI_UNIT) {
				if (LinkedTo->TClass->Speed == SPEED_TRACK) {
					speed *= Rule->TrackedUphill;
				} else {
					speed *= Rule->WheeledUphill;
				}
			}
		} else if (destheight < unitheight) {
			if (LinkedTo->RTTI == RTTI_UNIT) {
				if (LinkedTo->TClass->Speed == SPEED_TRACK) {
					speed *= Rule->TrackedDownhill;
				} else {
					speed *= Rule->WheeledDownhill;
				}
			}
		}
		if (speed == 0.0) speed = 0.5;

		/*
		**	A damaged unit has a reduced speed.
		*/
		if (LinkedTo->HealthRatio <= Rule->ConditionYellow) {
			speed *= 0.75;	// Three quarters speed.
		}
		if (TrackNumber < BACKUP_INTO_REFINERY) {
			TargetSpeed = speed;
		} else {
			if (speed != LinkedTo->Speed) {
				LinkedTo->Set_Speed(speed);		// Full speed.
			}
		}

		/*
		**	Reserve the destination cell so that it won't become
		**	occupied AS this unit is moving into it.
		*/
		LinkedTo->Overrun_Square(dest.As_Cell(), true);

		/*
		**	Determine which track to use (based on recorded path).
		*/
		FacingType nextface = LinkedTo->Path[1];
		if (nextface == FACING_NONE) {
			if (LinkedTo->Distance(DestinationCoord) > 2 * CELL_LEPTON) {

				/*
				 * The end of the path was reached but the destination is still far away.
				 * Regenerate the path so that movement can continue.
				 */
				int patharg = LinkedTo->TClass->IsTrain ? 1 : 0;
				if (!LinkedTo->Basic_Path(DestinationCoord.As_Cell(), patharg)) {
					if (LinkedTo == NULL) {
						stop_processing = true;
						return(false);
					}
					if (!LinkedTo->Is_In_Same_Zone(DestinationCoord)) {
						LinkedTo->Assign_Destination(NULL);
					}
				}
				nextface = LinkedTo->Path[1];
			} else {
				nextface = facing;
			}
		}
		if (nextface == FACING_COUNT) nextface = facing;
		if (nextface == FACING_NONE || force_straight) {
			nextface = facing;
		}

		/*
		 * If the cell that this unit would curve into contains a crushable
		 * overlay (such as a fence), then don't curve. Instead, drive straight
		 * through it and flag the unit as rocking from the crush.
		 */
		CellClass const * adjcell;
		if (nextface != FACING_NONE && (adjcell = &destptr->Adjacent_Cell(nextface)) != NULL && adjcell->Overlay != OVERLAY_NONE && OverlayTypes[adjcell->Overlay]->IsCrushable || crushable_dest) {
			nextface = facing;
			IsRocking = true;
		} else {
			IsRocking = false;
		}

		IsOnShortTrack = false;
		TrackNumber = facing * FACING_COUNT + (int)nextface;
		if (TrackControl[TrackNumber].Track == 0) {
			TrackNumber = facing * FACING_COUNT + (int)facing;
		}

		if (TrackControl[TrackNumber].Flag & F_D) {
			/*
			**	If the middle cell of a two cell track contains a crate,
			**	the check for goodies before movement starts.
			*/
			MoveType nextcando;
			if (!Map[destcell].Goodie_Check(LinkedTo) && !LinkedTo->IsInLimbo) {
				nextcando = MOVE_NO;
				if (!LinkedTo->IsActive) return(false);
			} else {
				if (!LinkedTo->IsActive) return(false);
				dest = Adjacent_Cell(dest, nextface);
				destcell = dest.As_Cell();
				nextcando = LinkedTo->Can_Enter_Cell(&Map[destcell], nextface, height);
				if (nextcando < MOVE_NO && LinkedTo->TClass->IsTrain) {
					nextcando = MOVE_OK;
				} else if (((nextcando == MOVE_FRIENDLY_DESTROYABLE || nextcando == MOVE_DESTROYABLE) && LinkedTo->TClass->IsCrusher && Map[destcell].Overlay == OVERLAY_SANDBAG_WALL)) {
					nextcando = MOVE_OK;
				}
			}
			if (!LinkedTo->IsActive) return(false);

			if (nextcando != MOVE_OK) {

				if (nextcando == MOVE_CLOSED_GATE) {
					Incoming(destcell);
				}
				else if (nextcando == MOVE_MOVING_BLOCK) {
					return(Start_Of_Move(stop_processing, retry, true));
				}

				/*
				**	If a temporary friendly object is blocking the path, then cause it to
				**	get out of the way.
				*/
				else if (nextcando == MOVE_TEMP) {
					if (!LinkedTo->TClass->IsTrain) {
						if (retry) {
							LinkedTo->Path[0] = FACING_NONE;
							LinkedTo->PathDelay = 0;
							return(Start_Of_Move(stop_processing, false, false));
						}
						if (LinkedTo->Distance(DestinationCoord) >= Rule->CloseEnoughDistance
							|| abs(DestinationCoord.Z - LinkedTo->PositionCoord.Z) >= 2 * LEVEL_LEPTON_H
							|| Map[(Coord const &)LinkedTo->PositionCoord].Land_Type() == LAND_TUNNEL) {
							CellClass * cellptr = &Map[destcell];
							bool is_bridge;
							if (!cellptr->IsUnderBridge || abs(LinkedTo->PositionCoord.Z / LEVEL_LEPTON_H - cellptr->Height) <= 2) {
								is_bridge = false;
							} else {
								is_bridge = true;
							}
							cellptr->Incoming(COORD_NONE, true, true, is_bridge);
						} else {
							Stop_Driver();
							if (Abandon_Navigation()) {
								return(true);
							}
						}
					}
				}

				/*
				**	If a cloaked object is blocking, then shimmer the cell.
				*/
				else if (nextcando == MOVE_CLOAK) {
					Map[destcell].Shimmer();
					if (retry) {
						LinkedTo->Path[0] = FACING_NONE;
						return(Start_Of_Move(stop_processing, false, false));
					}
					Stop_Driver();
					return(Abandon_Navigation());
				}

				else if (nextcando == MOVE_NO) {
					if (retry) {
						LinkedTo->Path[0] = FACING_NONE;		// Path is blocked!
						LinkedTo->PathDelay = 0;
						return(Start_Of_Move(stop_processing, false, false));
					}
					Stop_Driver();
					return(Abandon_Navigation());
				}

				/*
				 * A closed gate, a train-ignored or scattered temporary blockage,
				 * and an unhandled destroyable obstacle all fall through to here.
				 * Blow away the path and the current track. A destroyable obstacle
				 * gets one more pass (which picks the fight); everything else falls
				 * into the track-start logic below, which stops the unit.
				 */
				LinkedTo->Path[0] = FACING_NONE;		// Path is blocked!
				TrackNumber = -1;
				dest = COORD_NONE;
				if (nextcando == MOVE_DESTROYABLE || nextcando == MOVE_FRIENDLY_DESTROYABLE) {
					return(Start_Of_Move(stop_processing, retry, true));
				}

			} else {
				LinkedTo->Advance_Path(2);
				LinkedTo->IsPlanningToLook = true;
			}
		} else {
			LinkedTo->Advance_Path(1);
		}
		LinkedTo->LastPathingCell = dest.As_Cell();
		LinkedTo->IsNewNavCom = false;
		TrackIndex = 0;
		if (!Start_Driver(dest)) {
			TrackNumber = -1;
			LinkedTo->Path[0] = FACING_NONE;
			LinkedTo->Set_Speed(0);
		}
		return(false);
	}
	return(false);
}


/***********************************************************************************************
 * DriveClass::Lay_Track -- Handles track laying logic for the unit.                           *
 *                                                                                             *
 *    This routine handles the track laying for the unit. This entails examining the unit's    *
 *    current location as well as the direction and whether this unit is allowed to lay        *
 *    tracks in the first place.                                                               *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/28/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void DriveLocomotionClass::Lay_Track(void)
{
	assert(LinkedTo->IsActive);

#ifdef NEVER
	static IconCommandType * _trackdirs[FACING_COUNT] = {
		TrackN_S,
		TrackNE_SW,
		TrackE_W,
		TrackNW_SE,
		TrackN_S,
		TrackNE_SW,
		TrackE_W,
		TrackNW_SE
	};

	if (!(ClassF & CLASSF_TRACKS)) return;

	Icon_Install(Coord_Cell(Coord), _trackdirs[Facing_To_8(BodyFacing)]);
#endif
}


/***********************************************************************************************
 * DriveClass::Smooth_Turn -- Handles the low level coord calc for smooth turn logic.          *
 *                                                                                             *
 *    This routine calculates the new coordinate value needed for the                          *
 *    smooth turn logic. The adjustment and flag values must be                                *
 *    determined prior to entering this routine.                                               *
 *                                                                                             *
 * INPUT:   adj      -- The adjustment coordinate as lifted from the                           *
 *                      correct smooth turn table.                                             *
 *                                                                                             *
 *          dir      -- Pointer to dir for possible modification                               *
 *                      according to the flag bits.                                            *
 *                                                                                             *
 * OUTPUT:  Returns with the coordinate the unit should positioned to.                         *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/14/1994 JLB : Created.                                                                 *
 *   07/13/1994 JLB : Converted to member function.                                            *
 *=============================================================================================*/
Point2D DriveLocomotionClass::Smooth_Turn(Point2D const & adj, Dir256 & dir)
{
	assert(LinkedTo->IsActive);

	Dir256 workdir = dir;
	int x,y;
	int temp;
	TrackControlType flags = TrackControl[TrackNumber].Flag;

	x = adj.X;
	y = adj.Y;

	if (flags & F_T) {
		temp	= x;
		x		= y;
		y 		= temp;
		workdir = (Dir256)((DIR_W - workdir) & DIR_MAX);
	}

	if (flags & F_X) {
		x 		 = -x;
		workdir = (Dir256)(-workdir & DIR_MAX);
	}

	if (flags & F_Y) {
		y = -y;
		workdir = (Dir256)((DIR_S - workdir) & DIR_MAX);
	}

	dir = workdir;

	return(Point2D(HeadToCoord.X + x, HeadToCoord.Y + y));
}


/// <summary>
/// Warns the map that the unit is about to enter a cell.
/// This gives a friendly gate standing in that cell the chance to open before the
/// vehicle arrives at it.
/// </summary>
/// <param name="cell">The cell the unit is about to drive into.</param>
/// <returns>bool; Is the way into that cell clear?</returns>
bool DriveLocomotionClass::Incoming(Cell cell)
{
	return(Map.Try_Open_Gate(LinkedTo, cell));
}


/// <summary>
/// Fetches the display layer the driving unit belongs to.
/// </summary>
/// <returns>Returns with LAYER_GROUND, since a driving unit travels on the ground.</returns>
LayerType STDMETHODCALLTYPE DriveLocomotionClass::In_Which_Layer(void)
{
	return(LAYER_GROUND);
}


/// <summary>
/// Fetches the class identifier of this locomotor.
/// The persistence system uses this to know which locomotor to create when the unit is
/// loaded back in.
/// </summary>
/// <param name="retval">Pointer to the identifier to fill in.</param>
/// <returns>Returns with S_OK, or E_POINTER if no destination was supplied.</returns>
HRESULT STDMETHODCALLTYPE DriveLocomotionClass::GetClassID(CLSID * retval)
{
	if (retval == NULL) return(E_POINTER);
	*retval = CLSID_DriveLocomotion;
	return(S_OK);
}


/// <summary>
/// Fetches the depth adjustment for the driving unit.
/// A driving vehicle sits at the depth of the ground it is standing on.
/// </summary>
/// <returns>Returns with the adjustment to apply to the unit's draw depth.</returns>
int STDMETHODCALLTYPE DriveLocomotionClass::Z_Adjust(void)
{
	return(0);
}


/// <summary>
/// Fetches the depth gradient the unit is to be drawn with.
/// </summary>
/// <returns>Returns with the gradient the base locomotor asks for.</returns>
ZGradientType STDMETHODCALLTYPE DriveLocomotionClass::Z_Gradient(void)
{
	return(BASECLASS::Z_Gradient());
}


/// <summary>
/// Abandons the unit's current navigation order.
/// If the unit still has legs of a route queued up, it is dropped into its idle mode so
/// that it can take up the next one. Otherwise it is simply left with nowhere to go.
/// </summary>
/// <returns>bool; Did the unit take up a new mission?</returns>
bool DriveLocomotionClass::Abandon_Navigation(void)
{
	if (LinkedTo->RouteQueue.Count() == 0) {
		LinkedTo->Assign_Destination(NULL);
		return(false);
	}

	LinkedTo->NavCom = NULL;
	return(LinkedTo->Enter_Idle_Mode());
}


/// <summary>
/// Marks or removes the unit's occupation of the cells it is driving through.
/// A vehicle between cells lays claim to more than the one it stands in, so the map must
/// be told about every cell of the track it is committed to.
/// </summary>
/// <param name="mark">The MarkType to apply to the cells occupied.</param>
void STDMETHODCALLTYPE DriveLocomotionClass::Mark_All_Occupation_Bits(int mark)
{
	if (HeadToCoord != COORD_NONE) {
		Mark_Track(HeadToCoord, (MarkType)mark);
	}
}


/// <summary>
/// Is the driver headed for the specified location?
/// The cell the unit is bound for counts, and so does the cell it is about to reach at
/// its next per cell process point, so that a vehicle part way around a turn is still
/// recognized as moving into the cell it is committed to.
/// </summary>
/// <param name="to">The location to test against.</param>
/// <returns>bool; Is the unit moving there?</returns>
boolean STDMETHODCALLTYPE DriveLocomotionClass::Is_Moving_Here(Coord to)
{
	Coord coord = Head_To_Coord();

	if (coord != COORD_NONE) {
		if (!IsOnShortTrack && TrackNumber != -1) {

			/*
			**	If we have not passed the per cell process point we need
			**	to deal with it.
			*/
			int tracknum = TrackControl[TrackNumber].Track;
			if (tracknum) {
				TrackType const * ptr = RawTracks[tracknum - 1].Track;
				int cellidx = RawTracks[tracknum - 1].Cell;
				if (cellidx > -1) {
					Dir256 dir = ptr[cellidx].Facing;

					if (TrackIndex < cellidx && cellidx != -1) {
						Point2D pt = Smooth_Turn(ptr[cellidx].Offset, dir);
						Coord coord = Coord(pt.X, pt.Y);
						coord.Z += LinkedTo->PositionCoord.Z;
						if (coord.As_Cell() == to.As_Cell() && abs(coord.Z - to.Z) <= LEVEL_LEPTON_H) {
							return(true);
						}
					}
				}
			}
		}

		if (coord.As_Cell() == to.As_Cell() && abs(coord.Z - to.Z) <= LEVEL_LEPTON_H) {
			return(true);
		}
	}

	return(false);
}


/// <summary>
/// Is the unit about to jump onto another track?
/// A turn queued up in the path can be taken smoothly by hopping from the track the unit
/// is on to one that curves toward the new facing. This routine tells the caller that
/// such a hop is due, but performs none of it.
/// </summary>
/// <returns>bool; Will the driver jump tracks?</returns>
boolean STDMETHODCALLTYPE DriveLocomotionClass::Will_Jump_Tracks(void)
{
	/// This repeats the track jump test that While_Moving performs.
	assert(LinkedTo->IsActive);

	TurnTrackType	const * track;	// Track control pointer.
	int				tracknum;		// The track number being processed.
	FacingType		nextface;		// Next facing queued in path.
	bool				adj;		// Is a turn coming up?

	nextface = LinkedTo->Path[0];

	if (nextface >= FACING_NONE && nextface <= FACING_COUNT) {

		track = &TrackControl[TrackNumber];
		if (IsOnShortTrack) {
			tracknum = track->StartTrack;
		} else {
			tracknum = track->Track;
		}

		/*
		**	Determine if there is a turn coming up. If there is
		**	a turn, then track jumping might occur.
		*/
		adj = false;
		if (nextface != FACING_COUNT && nextface != FACING_NONE && Dir_Facing(track->Facing) != nextface) {
			adj = true;
		}

		/*
		**	The unit could "jump tracks". Check to see if the unit should
		**	do so.
		*/
		if (nextface != FACING_COUNT && nextface != FACING_NONE && adj && RawTracks[tracknum-1].Jump == TrackIndex && TrackIndex) {
			TurnTrackType const * newtrack;		// Proposed jump-to track.
			int	tnum;

			tnum = (int)(Dir_Facing(track->Facing) * FACING_COUNT) + (int)nextface;
			newtrack = &TrackControl[tnum];
			if (newtrack->Track && RawTracks[newtrack->Track-1].Entry) {
				return(true);
			}
		}
	}
	return(false);
}


/// <summary>
/// Locks the driver against being handed back.
/// While locked, this driver will not report itself ready to end a piggyback, so a
/// temporary locomotor riding on top of it keeps control of the unit.
/// </summary>
void STDMETHODCALLTYPE DriveLocomotionClass::Lock(void)
{
	IsLocomotorUnlocked = false;
}


/// <summary>
/// Unlocks the driver so that it may be handed back.
/// This is the counterpart to Lock. The driver may once again report itself ready to
/// end a piggyback.
/// </summary>
void STDMETHODCALLTYPE DriveLocomotionClass::Unlock(void)
{
	IsLocomotorUnlocked = true;
}


/// <summary>
/// Fetches the turn track the unit is currently following.
/// </summary>
/// <returns>Returns with the track control number, or -1 if the unit is not on a
/// track.</returns>
int STDMETHODCALLTYPE DriveLocomotionClass::Get_Track_Number(void)
{
	return(TrackNumber);
}


/// <summary>
/// Fetches how far along its track the unit has traveled.
/// </summary>
/// <returns>Returns with the index into the track the unit has reached, or -1 if the
/// unit is not following one.</returns>
int STDMETHODCALLTYPE DriveLocomotionClass::Get_Track_Index(void)
{
	return(TrackIndex);
}


/// <summary>
/// Fetches the movement the driver has banked up along its track.
/// </summary>
/// <returns>Returns with the accumulated movement not yet spent advancing the unit.</returns>
int STDMETHODCALLTYPE DriveLocomotionClass::Get_Speed_Accum(void)
{
	return(SpeedAccum);
}


/// <summary>
/// Adds a reference to this locomotor.
/// </summary>
/// <returns>Returns with the reference count once the new reference is counted.</returns>
ULONG STDMETHODCALLTYPE DriveLocomotionClass::AddRef(void)
{
	return(BASECLASS::AddRef());
}


/// <summary>
/// Releases a reference to this locomotor.
/// </summary>
/// <returns>Returns with the reference count remaining after the release.</returns>
ULONG STDMETHODCALLTYPE DriveLocomotionClass::Release(void)
{
	return(BASECLASS::Release());
}


/***************************************************************************
**	Smooth turn track tables. These are coordinate offsets from the center
**	of the destination cell. These are the raw tracks that are modified
**	by negating the X and Y portions as necessary. Also for reverse travelling
**	direction, the track list can be processed backward.
**
**	Track 1 = N
**	Track 2 = NE
**	Track 3 = N->NE 45 deg (double path consumption)
**	Track 4 = N->E 90 deg (double path consumption)
**	Track 5 = NE->SE 90 deg (double path consumption)
**	Track 6 = NE->N 45 deg (double path consumption)
**	Track 7 = N->NE (facing change only)
**	Track 8 = NE->E (facing change only)
**	Track 9 = N->E (facing change only)
**	Track 10= NE->SE (facing change only)
**	Track 11= back up into refinery
**	Track 12= drive out of refinery
*/
DriveLocomotionClass::TrackType const DriveLocomotionClass::Track1[24] = {
	{{0,245},(Dir256)0},
	{{0,234},(Dir256)0},
	{{0,223},(Dir256)0},
	{{0,212},(Dir256)0},
	{{0,201},(Dir256)0},
	{{0,190},(Dir256)0},
	{{0,179},(Dir256)0},
	{{0,168},(Dir256)0},
	{{0,157},(Dir256)0},
	{{0,146},(Dir256)0},
	{{0,135},(Dir256)0},
	{{0,124},(Dir256)0},		// Track jump check here.
	{{0,113},(Dir256)0},
	{{0,102},(Dir256)0},
	{{0,91},(Dir256)0},
	{{0,80},(Dir256)0},
	{{0,69},(Dir256)0},
	{{0,58},(Dir256)0},
	{{0,47},(Dir256)0},
	{{0,36},(Dir256)0},
	{{0,25},(Dir256)0},
	{{0,14},(Dir256)0},
	{{0,3},(Dir256)0},
	{{0,0},(Dir256)0}
};

DriveLocomotionClass::TrackType const DriveLocomotionClass::Track2[] = {
	{{-248,248},(Dir256)32},
	{{-240,240},(Dir256)32},
	{{-232,232},(Dir256)32},
	{{-224,224},(Dir256)32},
	{{-216,216},(Dir256)32},
	{{-208,208},(Dir256)32},
	{{-200,200},(Dir256)32},
	{{-192,192},(Dir256)32},
	{{-184,184},(Dir256)32},
	{{-176,176},(Dir256)32},
	{{-168,168},(Dir256)32},
	{{-160,160},(Dir256)32},
	{{-152,152},(Dir256)32},
	{{-144,144},(Dir256)32},
	{{-136,136},(Dir256)32},
	{{-129,129},(Dir256)32},		// Track jump check here.
	{{-120,120},(Dir256)32},
	{{-112,112},(Dir256)32},
	{{-104,104},(Dir256)32},
	{{-96,96},(Dir256)32},
	{{-88,88},(Dir256)32},
	{{-80,80},(Dir256)32},
	{{-72,72},(Dir256)32},
	{{-64,64},(Dir256)32},
	{{-56,56},(Dir256)32},
	{{-48,48},(Dir256)32},
	{{-40,40},(Dir256)32},
	{{-32,32},(Dir256)32},
	{{-24,24},(Dir256)32},
	{{-16,16},(Dir256)32},
	{{-8,8},(Dir256)32},
	{{0,0},(Dir256)32}
};

DriveLocomotionClass::TrackType const DriveLocomotionClass::Track3[] = {
	{{-256,501},(Dir256)0},
	{{-256,490},(Dir256)0},
	{{-256,479},(Dir256)0},
	{{-256,468},(Dir256)0},
	{{-256,457},(Dir256)0},
	{{-256,446},(Dir256)0},
	{{-256,435},(Dir256)0},
	{{-256,424},(Dir256)0},
	{{-256,413},(Dir256)0},
	{{-256,402},(Dir256)0},
	{{-256,391},(Dir256)0},
	{{-256,383},(Dir256)0},
	{{-256,373},(Dir256)0},		// Jump entry point here.
	{{-256,363},(Dir256)0},
	{{-254,352},(Dir256)1},
	{{-252,341},(Dir256)3},
	{{-250,332},(Dir256)4},
	{{-248,321},(Dir256)5},
	{{-245,311},(Dir256)7},
	{{-241,302},(Dir256)8},
	{{-237,292},(Dir256)9},
	{{-233,282},(Dir256)11},
	{{-229,272},(Dir256)12},
	{{-225,263},(Dir256)13},		// Center cell processing here.
	{{-220,252},(Dir256)15},
	{{-216,243},(Dir256)16},
	{{-212,236},(Dir256)17},
	{{-206,224},(Dir256)19},
	{{-202,215},(Dir256)20},
	{{-195,207},(Dir256)21},
	{{-190,198},(Dir256)23},
	{{-183,186},(Dir256)24},
	{{-179,176},(Dir256)25},
	{{-168,168},(Dir256)27},
	{{-160,160},(Dir256)28},
	{{-152,152},(Dir256)29},
	{{-144,144},(Dir256)31},
	{{-136,136},(Dir256)32},
	{{-129,129},(Dir256)32},		// Track jump check here.
	{{-120,120},(Dir256)32},
	{{-112,112},(Dir256)32},
	{{-104,104},(Dir256)32},
	{{-96,96},(Dir256)32},
	{{-88,88},(Dir256)32},
	{{-80,80},(Dir256)32},
	{{-72,72},(Dir256)32},
	{{-64,64},(Dir256)32},
	{{-56,56},(Dir256)32},
	{{-48,48},(Dir256)32},
	{{-40,40},(Dir256)32},
	{{-32,32},(Dir256)32},
	{{-24,24},(Dir256)32},
	{{-16,16},(Dir256)32},
	{{-8,8},(Dir256)32},
	{{0,0},(Dir256)32}
};

DriveLocomotionClass::TrackType const DriveLocomotionClass::Track4[] = {
	{{-256,245},(Dir256)0},
	{{-256,235},(Dir256)0},
	{{-256,224},(Dir256)0},
	{{-256,213},(Dir256)0},
	{{-255,203},(Dir256)0},
	{{-253,192},(Dir256)0},
	{{-251,181},(Dir256)1},
	{{-249,171},(Dir256)1},
	{{-246,160},(Dir256)2},
	{{-243,149},(Dir256)3},
	{{-240,139},(Dir256)4},
	{{-236,127},(Dir256)5},		// Track entry here.
	{{-232,117},(Dir256)8},
	{{-228,109},(Dir256)12},
	{{-222,99},(Dir256)16},
	{{-219,90},(Dir256)20},
	{{-213,82},(Dir256)23},
	{{-206,72},(Dir256)27},
	{{-201,64},(Dir256)32},
	{{-195,56},(Dir256)36},
	{{-186,48},(Dir256)39},
	{{-177,43},(Dir256)43},
	{{-168,36},(Dir256)47},
	{{-160,32},(Dir256)51},
	{{-147,27},(Dir256)54},
	{{-135,23},(Dir256)57},
	{{-126,20},(Dir256)60},		// Track jump here.
	{{-113,17},(Dir256)62},
	{{-104,13},(Dir256)63},
	{{-94,9},(Dir256)64},
	{{-84,6},(Dir256)64},
	{{-75,4},(Dir256)66},
	{{-64,3},(Dir256)64},
	{{-53,2},(Dir256)64},
	{{-43,1},(Dir256)64},
	{{-32,0},(Dir256)64},
	{{-21,0},(Dir256)64},
	{{-11,0},(Dir256)64},
	{{0,0},(Dir256)64}
};

DriveLocomotionClass::TrackType const DriveLocomotionClass::Track5[] = {
	{{-504,-8},(Dir256)32},
	{{-496,-16},(Dir256)32},
	{{-488,-24},(Dir256)32},
	{{-480,-32},(Dir256)32},
	{{-472,-40},(Dir256)32},
	{{-464,-48},(Dir256)32},
	{{-456,-56},(Dir256)32},
	{{-448,-64},(Dir256)32},
	{{-440,-72},(Dir256)32},
	{{-432,-80},(Dir256)32},
	{{-424,-88},(Dir256)32},
	{{-416,-96},(Dir256)32},
	{{-408,-104},(Dir256)32},
	{{-400,-112},(Dir256)32},
	{{-392,-120},(Dir256)32},
	{{-385,-127},(Dir256)32},		// Track entry here.
	{{-376,-136},(Dir256)32},
	{{-368,-143},(Dir256)32},
	{{-361,-150},(Dir256)32},
	{{-353,-158},(Dir256)32},
	{{-344,-166},(Dir256)32},
	{{-336,-173},(Dir256)35},
	{{-329,-181},(Dir256)38},
	{{-322,-188},(Dir256)41},
	{{-316,-194},(Dir256)44},
	{{-306,-199},(Dir256)47},
	{{-296,-204},(Dir256)50},
	{{-288,-208},(Dir256)53},
	{{-277,-211},(Dir256)56},
	{{-267,-212},(Dir256)59},
	{{-256,-213},(Dir256)62},
	{{-245,-212},(Dir256)66},
	{{-235,-211},(Dir256)69},
	{{-225,-208},(Dir256)72},
	{{-216,-204},(Dir256)75},
	{{-208,-199},(Dir256)78},
	{{-198,-194},(Dir256)81},
	{{-188,-188},(Dir256)84},
	{{-181,-181},(Dir256)87},
	{{-176,-173},(Dir256)90},
	{{-168,-166},(Dir256)93},
	{{-160,-158},(Dir256)96},
	{{-152,-150},(Dir256)96},
	{{-144,-143},(Dir256)96},
	{{-136,-136},(Dir256)96},
	{{-129,-129},(Dir256)96},		// Track jump check here.
	{{-120,-120},(Dir256)96},
	{{-112,-112},(Dir256)96},
	{{-104,-104},(Dir256)96},
	{{-96,-96},(Dir256)96},
	{{-88,-88},(Dir256)96},
	{{-80,-80},(Dir256)96},
	{{-72,-72},(Dir256)96},
	{{-64,-64},(Dir256)96},
	{{-56,-56},(Dir256)96},
	{{-48,-48},(Dir256)96},
	{{-40,-40},(Dir256)96},
	{{-32,-32},(Dir256)96},
	{{-24,-24},(Dir256)96},
	{{-16,-16},(Dir256)96},
	{{-8,-8},(Dir256)96},
	{{0,0},(Dir256)96}
};

DriveLocomotionClass::TrackType const DriveLocomotionClass::Track6[] = {
	{{-512,256},(Dir256)32},
	{{-504,248},(Dir256)32},
	{{-496,240},(Dir256)32},
	{{-488,232},(Dir256)32},
	{{-480,224},(Dir256)32},
	{{-472,216},(Dir256)32},
	{{-464,208},(Dir256)32},
	{{-456,200},(Dir256)32},
	{{-448,192},(Dir256)32},
	{{-440,184},(Dir256)32},
	{{-432,176},(Dir256)32},
	{{-424,168},(Dir256)32},
	{{-416,160},(Dir256)32},
	{{-408,152},(Dir256)32},
	{{-400,144},(Dir256)32},
	{{-392,136},(Dir256)32},
	{{-385,129},(Dir256)32},		// Jump entry point here.
	{{-376,120},(Dir256)32},
	{{-368,112},(Dir256)32},
	{{-360,104},(Dir256)32},
	{{-352,96},(Dir256)32},
	{{-344,88},(Dir256)32},
	{{-338,85},(Dir256)32},
	{{-328,78},(Dir256)35},
	{{-320,72},(Dir256)37},
	{{-311,66},(Dir256)40},
	{{-302,59},(Dir256)43},
	{{-294,55},(Dir256)45},
	{{-285,50},(Dir256)48},
	{{-277,43},(Dir256)51},
	{{-267,38},(Dir256)53},
	{{-258,34},(Dir256)56},
	{{-248,28},(Dir256)59},
	{{-238,25},(Dir256)61},
	{{-229,21},(Dir256)64},
	{{-218,17},(Dir256)64},
	{{-208,14},(Dir256)64},
	{{-199,11},(Dir256)64},
	{{-189,9},(Dir256)64},
	{{-178,7},(Dir256)64},
	{{-169,5},(Dir256)64},
	{{-158,3},(Dir256)64},
	{{-147,1},(Dir256)64},
	{{-137,0},(Dir256)64},
	{{-129,0},(Dir256)64},		// Track jump check here.
	{{-117,0},(Dir256)64},
	{{-107,0},(Dir256)64},
	{{-96,0},(Dir256)64},
	{{-85,0},(Dir256)64},
	{{-75,0},(Dir256)64},
	{{-64,0},(Dir256)64},
	{{-53,0},(Dir256)64},
	{{-43,0},(Dir256)64},
	{{-32,0},(Dir256)64},
	{{-21,0},(Dir256)64},
	{{-11,0},(Dir256)64},
	{{0,0},(Dir256)64}
};

DriveLocomotionClass::TrackType const DriveLocomotionClass::Track7[] = {
	{{-1,6},(Dir256)0},
	{{-2,12},(Dir256)4},
	{{-4,17},(Dir256)8},
	{{-6,24},(Dir256)12},
	{{-10,31},(Dir256)16},
	{{-13,36},(Dir256)19},
	{{-16,43},(Dir256)22},
	{{-3,48},(Dir256)23},
	{{-21,53},(Dir256)24},
	{{-24,56},(Dir256)25},
	{{-26,60},(Dir256)26},
	{{-29,64},(Dir256)27},
	{{-32,67},(Dir256)28},
	{{-35,70},(Dir256)29},
	{{-33,67},(Dir256)30},
	{{-31,64},(Dir256)30},
	{{-29,60},(Dir256)30},
	{{-27,56},(Dir256)30},
	{{-25,53},(Dir256)31},
	{{-23,48},(Dir256)31},
	{{-21,43},(Dir256)31},
	{{-19,36},(Dir256)31},
	{{-15,31},(Dir256)31},
	{{-12,24},(Dir256)32},
	{{-9,17},(Dir256)32},
	{{-6,12},(Dir256)32},
	{{-3,6},(Dir256)32},
	{{0,0},(Dir256)32}
};

DriveLocomotionClass::TrackType const DriveLocomotionClass::Track8[] = {
	{{-4,3},(Dir256)32},
	{{-9,6},(Dir256)36},
	{{-15,10},(Dir256)40},
	{{-21,12},(Dir256)44},
	{{-28,13},(Dir256)46},
	{{-36,14},(Dir256)48},
	{{-43,15},(Dir256)50},
	{{-48,16},(Dir256)52},
	{{-55,17},(Dir256)54},
	{{-62,18},(Dir256)56},
	{{-64,17},(Dir256)58},
	{{-62,16},(Dir256)60},
	{{-55,14},(Dir256)62},
	{{-49,12},(Dir256)64},
	{{-43,10},(Dir256)64},
	{{-38,8},(Dir256)64},
	{{-30,6},(Dir256)64},
	{{-23,4},(Dir256)64},
	{{-17,2},(Dir256)64},
	{{-11,1},(Dir256)64},
	{{-7,0},(Dir256)64},
	{{0,0},(Dir256)64}
};

DriveLocomotionClass::TrackType const DriveLocomotionClass::Track9[] = {
	{{2,-11},(Dir256)0},
	{{4,-21},(Dir256)2},
	{{6,-32},(Dir256)4},
	{{9,-43},(Dir256)6},
	{{12,-50},(Dir256)9},
	{{15,-56},(Dir256)11},
	{{18,-64},(Dir256)13},
	{{21,-72},(Dir256)16},
	{{18,-64},(Dir256)18},
	{{14,-56},(Dir256)20},
	{{10,-50},(Dir256)22},
	{{4,-43},(Dir256)24},
	{{0,-34},(Dir256)26},
	{{-8,-23},(Dir256)28},
	{{-14,-18},(Dir256)30},
	{{-21,-11},(Dir256)32},
	{{-31,-3},(Dir256)34},
	{{-40,2},(Dir256)36},
	{{-46,7},(Dir256)39},
	{{-53,11},(Dir256)41},
	{{-59,16},(Dir256)43},
	{{-66,19},(Dir256)45},
	{{-73,21},(Dir256)48},
	{{-66,19},(Dir256)50},
	{{-59,17},(Dir256)52},
	{{-52,11},(Dir256)54},
	{{-44,8},(Dir256)56},
	{{-33,5},(Dir256)58},
	{{-21,3},(Dir256)62},
	{{-11,1},(Dir256)64},
	{{0,0},(Dir256)64}
};

DriveLocomotionClass::TrackType const DriveLocomotionClass::Track10[] = {
	{{11,-10},(Dir256)32},
	{{21,-16},(Dir256)37},
	{{32,-21},(Dir256)42},
	{{43,-23},(Dir256)47},
	{{50,-27},(Dir256)52},
	{{56,-29},(Dir256)57},
	{{64,-32},(Dir256)60},
	{{56,-30},(Dir256)62},
	{{50,-28},(Dir256)64},
	{{42,-27},(Dir256)68},
	{{30,-26},(Dir256)70},
	{{21,-25},(Dir256)72},
	{{11,-24},(Dir256)74},
	{{0,-23},(Dir256)76},
	{{-11,-24},(Dir256)78},
	{{-21,-25},(Dir256)80},
	{{-32,-26},(Dir256)82},
	{{-43,-27},(Dir256)84},
	{{-50,-28},(Dir256)86},
	{{-59,-30},(Dir256)88},
	{{-64,-32},(Dir256)90},
	{{-59,-29},(Dir256)92},
	{{-50,-27},(Dir256)94},
	{{-43,-23},(Dir256)95},
	{{-32,-21},(Dir256)96},
	{{-21,-16},(Dir256)96},
	{{-11,-10},(Dir256)96},
	{{0,0},(Dir256)96}
};

DriveLocomotionClass::TrackType const DriveLocomotionClass::Track11[] = {
	{{0,256},DIR_SW},
	{{8,243},DIR_SW},
	{{16,229},DIR_SW},
	{{24,214},DIR_SW},
	{{32,200},DIR_SW},
	{{40,185},DIR_SW},
	{{48,171},DIR_SW},
	{{56,156},DIR_SW},
	{{64,141},DIR_SW},
	{{72,127},DIR_SW},
	{{80,113},DIR_SW},
	{{88,100},DIR_SW},
	{{96,85},DIR_SW},

	{{0,0},DIR_SW}
};

DriveLocomotionClass::TrackType const DriveLocomotionClass::Track12[] = {
	{{96,-171},DIR_SW},
	{{88,-156},DIR_SW},
	{{80,-143},DIR_SW},
	{{72,-129},DIR_SW},
	{{64,-115},DIR_SW},
	{{56,-100},DIR_SW},
	{{48,-85},DIR_SW},
	{{40,-71},DIR_SW},
	{{32,-56},DIR_SW},
	{{24,-42},DIR_SW},
	{{16,-27},DIR_SW},
	{{8,-13},DIR_SW},

	{{0,0},DIR_SW}
};

/*
**	Drive out of weapon's factory.
*/
DriveLocomotionClass::TrackType const DriveLocomotionClass::Track13[] = {
	{{-670,-68},DIR_E},
	{{-660,-67},DIR_E},
	{{-650,-66},DIR_E},
	{{-639,-65},DIR_E},
	{{-630,-64},DIR_E},
	{{-620,-63},DIR_E},
	{{-610,-62},DIR_E},
	{{-600,-61},DIR_E},
	{{-590,-60},DIR_E},
	{{-580,-59},DIR_E},
	{{-570,-58},DIR_E},
	{{-560,-57},DIR_E},
	{{-550,-56},DIR_E},
	{{-540,-55},DIR_E},
	{{-530,-54},DIR_E},
	{{-520,-53},DIR_E},
	{{-510,-52},DIR_E},
	{{-500,-51},DIR_E},
	{{-490,-50},DIR_E},
	{{-480,-49},DIR_E},
	{{-470,-48},DIR_E},
	{{-460,-47},DIR_E},
	{{-450,-46},DIR_E},
	{{-440,-45},DIR_E},
	{{-430,-44},DIR_E},
	{{-420,-43},DIR_E},
	{{-410,-42},DIR_E},
	{{-400,-41},DIR_E},
	{{-390,-40},DIR_E},
	{{-380,-39},DIR_E},
	{{-370,-38},DIR_E},
	{{-360,-37},DIR_E},
	{{-350,-36},DIR_E},
	{{-340,-35},DIR_E},
	{{-330,-34},DIR_E},
	{{-320,-33},DIR_E},
	{{-310,-32},DIR_E},
	{{-300,-31},DIR_E},
	{{-290,-30},DIR_E},
	{{-280,-29},DIR_E},
	{{-270,-28},DIR_E},
	{{-260,-27},DIR_E},
	{{-250,-26},DIR_E},
	{{-240,-25},DIR_E},
	{{-230,-24},DIR_E},
	{{-220,-23},DIR_E},
	{{-210,-22},DIR_E},
	{{-200,-21},DIR_E},
	{{-190,-20},DIR_E},
	{{-180,-19},DIR_E},
	{{-170,-18},DIR_E},
	{{-160,-17},DIR_E},
	{{-150,-16},DIR_E},
	{{-140,-15},DIR_E},
	{{-130,-14},DIR_E},
	{{-120,-13},DIR_E},
	{{-110,-12},DIR_E},
	{{-100,-11},DIR_E},
	{{-90,-10},DIR_E},
	{{-80,-9},DIR_E},
	{{-70,-8},DIR_E},
	{{-60,-7},DIR_E},
	{{-50,-6},DIR_E},
	{{-40,-5},DIR_E},
	{{-30,-4},DIR_E},
	{{-20,-3},DIR_E},
	{{-10,-2},DIR_E},
	{{0,-1},DIR_E},

	{{0,0},DIR_E}
};


/*
**	There are a limited basic number of tracks that a vehicle can follow. These
**	are they. Each track can be interpreted differently but this is controlled
**	by the TrackControl structure elaborated elsewhere.
*/
DriveLocomotionClass::RawTrackType const DriveLocomotionClass::RawTracks[13] = {
	{Track1, -1, 0, -1},
	{Track2, -1, 0, -1},
	{Track3, 37, 12, 22},
	{Track4, 26, 11, 19},
	{Track5, 45, 15, 31},
	{Track6, 44, 16, 27},
	{Track7, -1, 0, -1},
	{Track8, -1, 0, -1},
	{Track9, -1, 0, -1},
	{Track10, -1, 0, -1},
	{Track11, -1, 0, -1},
	{Track12, -1, 0, -1},
	{Track13, -1, 0, -1}
};


/***************************************************************************
**	Smooth turning control table. Given two directions in a path list, this
**	table determines which track to use and what modifying operations need
**	be performed on the track data.
*/
DriveLocomotionClass::TurnTrackType const DriveLocomotionClass::TrackControl[67] = {
	{1,	0,		DIR_N,	F_},                                                                // 0-0
	{3,	7,		DIR_NE,	F_D},                                                               //	0-1 (raw chart)
	{4,	9,		DIR_E,	F_D},                                                               //	0-2 (raw chart)
	{0,	0,		DIR_SE,	F_},                                                                // 0-3 !
	{0,	0,		DIR_S,	F_},                                                                // 0-4 !
	{0,	0,		DIR_SW,	F_},                                                                // 0-5 !
	{4,	9,		DIR_W,	(DriveLocomotionClass::TrackControlType)(F_X|F_D)},                 // 0-6
	{3,	7,		DIR_NW,	(DriveLocomotionClass::TrackControlType)(F_X|F_D)},                 // 0-7
	{6,	8,		DIR_N,	(DriveLocomotionClass::TrackControlType)(F_T|F_X|F_Y|F_D)},         // 1-0
	{2,	0,		DIR_NE,	F_},                                                                //	1-1 (raw chart)
	{6,	8,		DIR_E,	F_D},                                                               //	1-2 (raw chart)
	{5,	10,	DIR_SE,	F_D},                                                                   //	1-3 (raw chart)
	{0,	0,		DIR_S,	F_},                                                                // 1-4 !
	{0,	0,		DIR_SW,	F_},                                                                // 1-5 !
	{0,	0,		DIR_W,	F_},                                                                // 1-6 !
	{5,	10,	DIR_NW,	(DriveLocomotionClass::TrackControlType)(F_T|F_X|F_Y|F_D)},             // 1-7
	{4,	9,		DIR_N,	(DriveLocomotionClass::TrackControlType)(F_T|F_X|F_Y|F_D)},         // 2-0
	{3,	7,		DIR_NE,	(DriveLocomotionClass::TrackControlType)(F_T|F_X|F_Y|F_D)},         // 2-1
	{1,	0,		DIR_E,	(DriveLocomotionClass::TrackControlType)(F_T|F_X)},                 // 2-2
	{3,	7,		DIR_SE,	(DriveLocomotionClass::TrackControlType)(F_T|F_X|F_D)},             // 2-3
	{4,	9,		DIR_S,	(DriveLocomotionClass::TrackControlType)(F_T|F_X|F_D)},             // 2-4
	{0,	0,		DIR_SW,	F_},                                                                // 2-5 !
	{0,	0,		DIR_W,	F_},                                                                // 2-6 !
	{0,	0,		DIR_NW,	F_},                                                                // 2-7 !
	{0,	0,		DIR_N,	F_},                                                                // 3-0 !
	{5,	10,	DIR_NE,	(DriveLocomotionClass::TrackControlType)(F_Y|F_D)},                     // 3-1
	{6,	8,		DIR_E,	(DriveLocomotionClass::TrackControlType)(F_Y|F_D)},                 // 3-2
	{2,	0,		DIR_SE,	F_Y},                                                               // 3-3
	{6,	8,		DIR_S,	(DriveLocomotionClass::TrackControlType)(F_T|F_X|F_D)},             // 3-4
	{5,	10,	DIR_SW,	(DriveLocomotionClass::TrackControlType)(F_T|F_X|F_D)},                 // 3-5
	{0,	0,		DIR_W,	F_},                                                                // 3-6 !
	{0,	0,		DIR_NW,	F_},                                                                // 3-7 !
	{0,	0,		DIR_N,	F_},                                                                // 4-0 !
	{0,	0,		DIR_NE,	F_},                                                                // 4-1 !
	{4,	9,		DIR_E,	(DriveLocomotionClass::TrackControlType)(F_Y|F_D)},                 // 4-2
	{3,	7,		DIR_SE,	(DriveLocomotionClass::TrackControlType)(F_Y|F_D)},                 // 4-3
	{1,	0,		DIR_S,	F_Y},                                                               // 4-4
	{3,	7,		DIR_SW,	(DriveLocomotionClass::TrackControlType)(F_X|F_Y|F_D)},             // 4-5
	{4,	9,		DIR_W,	(DriveLocomotionClass::TrackControlType)(F_X|F_Y|F_D)},             // 4-6
	{0,	0,		DIR_NW,	F_},                                                                // 4-7 !
	{0,	0,		DIR_N,	F_},                                                                // 5-0 !
	{0,	0,		DIR_NE,	F_},                                                                // 5-1 !
	{0,	0,		DIR_E,	F_},                                                                // 5-2 !
	{5,	10,	DIR_SE,	(DriveLocomotionClass::TrackControlType)(F_T|F_D)},                     // 5-3
	{6,	8,		DIR_S,	(DriveLocomotionClass::TrackControlType)(F_T|F_D)},                 // 5-4
	{2,	0,		DIR_SW,	F_T},                                                               // 5-5
	{6,	8,		DIR_W,	(DriveLocomotionClass::TrackControlType)(F_X|F_Y|F_D)},             // 5-6
	{5,	10,	DIR_NW,	(DriveLocomotionClass::TrackControlType)(F_X|F_Y|F_D)},                 // 5-7
	{4,	9,		DIR_N,	(DriveLocomotionClass::TrackControlType)(F_T|F_Y|F_D)},             // 6-0
	{0,	0,		DIR_NE,	F_},                                                                // 6-1 !
	{0,	0,		DIR_E,	F_},                                                                // 6-2 !
	{0,	0,		DIR_SE,	F_},                                                                // 6-3 !
	{4,	9,		DIR_S,	(DriveLocomotionClass::TrackControlType)(F_T|F_D)},                 // 6-4
	{3,	7,		DIR_SW,	(DriveLocomotionClass::TrackControlType)(F_T|F_D)},                 // 6-5
	{1,	0,		DIR_W,	F_T},                                                               // 6-6
	{3,	7,		DIR_NW,	(DriveLocomotionClass::TrackControlType)(F_T|F_Y|F_D)},             // 6-7
	{6,	8,		DIR_N,	(DriveLocomotionClass::TrackControlType)(F_T|F_Y|F_D)},             // 7-0
	{5,	10,	DIR_NE,	(DriveLocomotionClass::TrackControlType)(F_T|F_Y|F_D)},                 // 7-1
	{0,	0,		DIR_E,	F_},                                                                // 7-2 !
	{0,	0,		DIR_SE,	F_},                                                                // 7-3 !
	{0,	0,		DIR_S,	F_},                                                                // 7-4 !
	{5,	10,	DIR_SW,	(DriveLocomotionClass::TrackControlType)(F_X|F_D)},                     // 7-5
	{6,	8,		DIR_W,	(DriveLocomotionClass::TrackControlType)(F_X|F_D)},                 // 7-6
	{2,	0,		DIR_NW,	F_X},                                                               // 7-7

	{11,	11,	DIR_SW,	F_},                                                            // Backup harvester into refinery.
	{12,	12,	DIR_SW,	F_},                                                            // Drive back into refinery.
	{13,	13,	DIR_SW,	F_}                                                             // Drive out of weapons factory.
};
