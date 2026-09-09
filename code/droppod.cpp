/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "always.h"

#include "droppod.h"

#include "_map.h"
#include "_rand.h"
#include "_rules.h"
#include "anim.h"
#include "cell.h"
#include "combat.h"
#include "coord.h"
#include "foot.h"
#include "globals.h"
#include "house.h"
#include "map.h"
#include "rules.h"
#include "saveload.h"
#include "savestream.h"
#include "sun.h"
#include "weapon.h"

#include "layer.hh"


/// <summary>
/// Constructs a drop pod locomotor.
/// The pod has no landing site until Move_To assigns one.
/// </summary>
DropPodLocomotionClass::DropPodLocomotionClass(void) :
	BASECLASS(),
	Direction(DPOD_DIR_NE),
	DestinationCoord(COORD_NONE)
{
}


/// <summary>
/// Destroys the drop pod locomotor.
/// </summary>
DropPodLocomotionClass::~DropPodLocomotionClass(void)
{
}


/// <summary>
/// Is the drop pod in motion?
/// A pod exists only for the duration of its fall, so it always reports movement.
/// </summary>
bool DropPodLocomotionClass::Is_Moving(void)
{
	return(true);
}


/// <summary>
/// Fetches the coordinate the pod is falling toward.
/// </summary>
/// <returns>Returns with the landing coordinate, or COORD_NONE if no destination has
/// been assigned yet.</returns>
Coord DropPodLocomotionClass::Destination(void)
{
	return(DestinationCoord);
}


/// <summary>
/// Handles the drop pod's descent for one game logic frame.
/// The pod slides in along its fixed approach angle, trailing smoke and shelling the
/// landing zone as it comes. On touchdown the carried locomotor is handed back and the
/// passenger is unlimboed, or destroyed along with its surroundings if there is nowhere
/// for it to stand.
/// </summary>
bool DropPodLocomotionClass::Process(void)
{
	// Handing the carried locomotor back leaves this pod unowned, so it holds itself for
	// the rest of the routine. The slot is declared here rather than beside the hand-back
	// so that the pod outlives every member this routine still reads.
	std::unique_ptr<ILocomotion> self;

	Coord coord = LinkedTo->PositionCoord;
	Coord smoke_coord = coord;

	int speed = LinkedTo->HeightAGL / 10 + 2;
	if (Rule->DropPodSpeed > speed) {
		speed = Rule->DropPodSpeed;
	}

	double horspeed = std::cos(Rule->DropPodAngle) * speed;

	switch (Direction) {
		case DPOD_DIR_NE:
		default:
			coord.X -= horspeed;
			break;
		case DPOD_DIR_NW:
			coord.X += horspeed;
			break;
		case DPOD_DIR_SE:
			coord.Y -= horspeed;
			break;
		case DPOD_DIR_SW:
			coord.Y += horspeed;
			break;
	}

	coord.Z -= std::sin(Rule->DropPodAngle) * speed;

	int height = LinkedTo->HeightAGL;
	FootClass * linked = LinkedTo;

	if (height <= 0) {
		linked->HeightAGL = 0;
		coord = linked->PositionCoord;
		linked->Limbo();

		// A pod that carries nothing stays the object's locomotor.
		std::unique_ptr<ILocomotion> carried = End_Piggyback();
		if (carried != nullptr) {
			self = std::move(LinkedTo->Locomotion);
			LinkedTo->Locomotion = std::move(carried);
		}

		if (!linked->Unlimbo(coord, DIR_N)) {
			Explosion_Damage(coord, 100, LinkedTo, Rule->C4Warhead);
			new AnimClass(Combat_Anim(100, Rule->C4Warhead, LAND_CLEAR, coord), coord, 0, 1, ShapeFlags_Type(SHAPE_CENTER | SHAPE_WIN_REL | SHAPE_ZGRAD), Get_Explosion_Z(coord));
			linked->Delete_Me();
		} else {
			new AnimClass(Rule->DropPod[Direction % Rule->DropPod.Count()], coord);
			linked->Mark(MARK_DOWN);
			linked->HeightAGL = 0;
			linked->Enter_Idle_Mode();
			linked->Commence();
			linked->Scatter(COORD_NONE);
		}
	} else {
		LinkedTo->PositionCoord = coord;
		WeaponTypeClass const * weapon = Rule->DropPodWeapon;

		if (weapon != NULL) {
			if (Frame % 6 == 0) {
				new AnimClass(AnimTypes[AnimTypeClass::From_Name("SMOKEY")], smoke_coord);
			}
			if (Frame % 3 == 0) {
				if (!LinkedTo->House->Is_Ally(Map[DestinationCoord].Cell_Techno())) {
					Coord scattered_coord = Coord_Scatter(DestinationCoord, CELL_LEPTON / 3, false);
					Sound_Effect((VocType)weapon->Sound.Pick(NonCriticalRandomNumber), coord);
					Explosion_Damage(scattered_coord, 2 * weapon->Attack, LinkedTo, weapon->WarheadPtr);
					new AnimClass(Combat_Anim(2 * weapon->Attack, weapon->WarheadPtr, LAND_CLEAR, scattered_coord), scattered_coord, 0, 1, ShapeFlags_Type(SHAPE_CENTER | SHAPE_WIN_REL | SHAPE_ZGRAD), Get_Explosion_Z(scattered_coord));
				}
			}
		}
	}
	return(true);
}


/// <summary>
/// Sets the ground position that the pod is to land on.
/// This routine picks the compass direction the pod will fall in, preferring one whose
/// approach begins on screen, then lifts the object to the drop height above that
/// approach and unlimbos it there with the atmosphere entry effect. A pod that already
/// has a destination ignores any later request.
/// </summary>
/// <param name="to">The coordinate the pod should land on.</param>
void DropPodLocomotionClass::Move_To(Coord to)
{
	if (DestinationCoord == COORD_NONE) {

		DestinationCoord = to;
		DestinationCoord.Z = Map.Get_Height_GL(DestinationCoord);

		Coord dropcoord = DestinationCoord;
		dropcoord.Z += Rule->DropPodHeight;
		double dropradius = (double)Rule->DropPodHeight / std::tan(Rule->DropPodAngle);
		dropcoord.X += dropradius;

		CellClass * cptr = &Map[dropcoord];

		if (Map.In_Local_Radar(dropcoord)) {
			Direction = DPOD_DIR_NE;
		} else {
			dropcoord.X = DestinationCoord.X - dropradius;
			cptr = &Map[dropcoord];

			if (Map.In_Local_Radar(dropcoord)) {
				Direction = DPOD_DIR_NW;
			} else {
				dropcoord.X = DestinationCoord.X;
				dropcoord.Y = DestinationCoord.Y + dropradius;
				cptr = &Map[dropcoord];

				if (Map.In_Local_Radar(dropcoord)) {
					Direction = DPOD_DIR_SE;
				} else {
					dropcoord.Y = DestinationCoord.Y - dropradius;
					cptr = &Map[dropcoord];
					Direction = DPOD_DIR_SW;
				}
			}
		}

		LinkedTo->PositionCoord = dropcoord;

		if (LinkedTo->Unlimbo(dropcoord, DIR_S)) {
			LinkedTo->PositionCoord = dropcoord;
			LinkedTo->PrimaryFacing.Set(DIR_S << 8);
			new AnimClass(Rule->AtmosphereEntry, dropcoord);
		} else {
			LinkedTo->Unlimbo(dropcoord, DIR_S);
		}
	}
}


ClassID DropPodLocomotionClass::Class_ID(void) const
{
	return(ClassID_BallisticLocomotion);
}


/// <summary>
/// Lists the members this drop pod locomotor carries.
/// The locomotor set aside while the pod descends is a separate persistent object rather
/// than a member, so it travels as a record of its own and is recreated as the class it was
/// saved as.
/// </summary>
/// <param name="stream">The stream carrying the members.</param>
void DropPodLocomotionClass::Serialize(SaveStreamClass & stream)
{
	BASECLASS::Serialize(stream);

	stream.Serialize(Direction);
	stream.Serialize(DestinationCoord);

	bool haspiggy = (Piggybacker != NULL);
	stream.Serialize(haspiggy);

	if (haspiggy) {
		if (stream.Is_Saving()) {
			Save_Object(stream, Piggybacker.get());
		} else {
			Piggybacker = Load_Locomotor(stream);
		}
	}
}


/// <summary>
/// Stops the pod's descent.
/// A pod cannot be halted in mid air, so this request is quietly ignored.
/// </summary>
void DropPodLocomotionClass::Stop_Moving(void)
{
	// empty
}


/// <summary>
/// Takes charge of the object's existing locomotor.
/// The drop pod holds on to the locomotor it displaces so that the object can be given
/// it back when the pod touches down.
/// </summary>
/// <param name="carried">The locomotor that is to take over the unit.</param>
/// <returns>bool; Was the locomotor taken on? One already carrying a locomotor refuses.</returns>
bool DropPodLocomotionClass::Begin_Piggyback(std::unique_ptr<ILocomotion> carried)
{
	if (carried == nullptr || Piggybacker != nullptr) {
		return(false);
	}
	Piggybacker = std::move(carried);
	return(true);
}


/// <summary>
/// Hands the carried locomotor back to the caller.
/// The pod gives up its hold without destroying the locomotor, so the object can resume
/// using it once the pod has landed.
/// </summary>
/// <returns>Returns with the locomotor that was riding, or nothing when none was.</returns>
std::unique_ptr<ILocomotion> DropPodLocomotionClass::End_Piggyback(void)
{
	return(std::move(Piggybacker));
}


/// <summary>
/// Can the piggyback be ended now?
/// The carried locomotor may only be given control back once the pod has come to rest.
/// </summary>
/// <returns>bool; May the carried locomotor take over again?</returns>
bool DropPodLocomotionClass::Is_Ok_To_End(void)
{
	if (!Is_Moving() && Piggybacker != NULL) {
		return(true);
	}
	return(false);
}


/// <summary>
/// Determines which display layer the pod belongs in.
/// A pod is always falling, so it draws along with the other airborne objects right up
/// until it lands and gives its object back.
/// </summary>
LayerType DropPodLocomotionClass::In_Which_Layer(void)
{
	return(LAYER_AIR);
}


/// <summary>
/// Fetches the drawing code for the drop pod.
/// The renderer uses this to choose the artwork that suits the pod's approach.
/// </summary>
int DropPodLocomotionClass::Drawing_Code(void)
{
	return((unsigned)Direction % 2);
}
