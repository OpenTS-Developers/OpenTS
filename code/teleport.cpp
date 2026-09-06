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

#include "teleport.h"

#include "foot.h"
#include "globals.h"
#include "savestream.h"


/// <summary>
/// Creates a teleport locomotor.
/// The locomotor starts out idle, with no destination waiting to be jumped to.
/// </summary>
TeleportLocomotionClass::TeleportLocomotionClass(void) :
	BASECLASS(),
	DestinationCoord(COORD_NONE)
{
}


/// <summary>
/// Is a teleport pending?
/// The object counts as moving from the moment a destination is handed to this
/// locomotor until the jump has actually been made.
/// </summary>
bool TeleportLocomotionClass::Is_Moving(void)
{
	if (DestinationCoord != COORD_NONE) {
		return(true);
	}
	return(false);
}


/// <summary>
/// Is the object standing still?
/// This is the plain opposite of Is_Moving. An object with a teleport ordered counts as
/// being on the move even though it has not gone anywhere yet.
/// </summary>
bool TeleportLocomotionClass::Is_Stationary(void)
{
	if (Is_Moving() == false) {
		return(true);
	}
	return(false);
}


/// <summary>
/// Fetches the location this locomotor is bound for.
/// </summary>
/// <returns>Returns with the pending teleport destination, or with the object's current
/// position if no teleport has been ordered.</returns>
Coord TeleportLocomotionClass::Destination(void)
{
	if (Is_Moving()) {
		return(DestinationCoord);
	}
	return(LinkedTo->PositionCoord);
}


/// <summary>
/// Orders the object to teleport to the location specified.
/// The jump is not made here. It happens the next time this locomotor is processed.
/// </summary>
/// <param name="to">The coordinate to teleport the object to.</param>
void TeleportLocomotionClass::Move_To(Coord to)
{
	DestinationCoord = to;
}


/// <summary>
/// Cancels any teleport that has been ordered.
/// The pending destination is forgotten, so the object stays where it is rather than
/// making the jump.
/// </summary>
void TeleportLocomotionClass::Stop_Moving(void)
{
	DestinationCoord = COORD_NONE;
}


/// <summary>
/// Performs any pending teleport.
/// This routine is called by the owning object's movement processing. The object is
/// lifted off the map, set down at its destination, and made to look around from where
/// it now stands. The whole journey is over by the time this routine returns.
/// </summary>
/// <returns>bool; Is there more movement still to process? A teleport never leaves any.</returns>
bool TeleportLocomotionClass::Process(void)
{
	if (Is_Moving()) {
		LinkedTo->Mark(MARK_UP);
		LinkedTo->PositionCoord = DestinationCoord;
		LinkedTo->Mark(MARK_DOWN);
		Stop_Moving();
		LinkedTo->Per_Cell_Process(PCP_END);
		LinkedTo->Look();
	}
	return(false);
}


/// <summary>
/// Fetches the class identifier of this locomotor.
/// This routine is used by the persistence system to record which locomotor was
/// written, so that the right one can be created when the save game is loaded.
/// </summary>
/// <param name="retval">Pointer to the class identifier to fill in.</param>
/// <returns>Returns with S_OK, or E_POINTER if no destination was supplied.</returns>
HRESULT TeleportLocomotionClass::GetClassID(CLSID * retval)
{
	if (retval == NULL) return(E_POINTER);
	*retval = CLSID_TeleportLocomotion;
	return(S_OK);
}


/// <summary>
/// Lists the members this teleport locomotor carries.
/// </summary>
/// <param name="stream">The stream carrying the members.</param>
void TeleportLocomotionClass::Serialize(SaveStreamClass & stream)
{
	BASECLASS::Serialize(stream);

	stream.Serialize(DestinationCoord);
}


/// <summary>
/// Fetches the display layer that the owning object belongs in.
/// A teleporting object is always on the ground. It never travels through the air on
/// the way to its destination, so it never rises out of the ground layer.
/// </summary>
/// <returns>Returns with the layer the object should be rendered in.</returns>
LayerType TeleportLocomotionClass::In_Which_Layer(void)
{
	return(LAYER_GROUND);
}
