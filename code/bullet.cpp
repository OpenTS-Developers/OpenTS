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

/* $Header: /CounterStrike/BULLET.CPP 1     3/03/97 10:24a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : BULLET.CPP                                                   *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : April 23, 1994                                               *
 *                                                                                             *
 *                  Last Update : October 10, 1996 [JLB]                                       *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   BulletClass::AI -- Logic processing for bullet.                                           *
 *   BulletClass::BulletClass -- Bullet constructor.                                           *
 *   BulletClass::Bullet_Explodes -- Performs bullet explosion logic.                          *
 *   BulletClass::Detach -- Removes specified target from this bullet's targeting system.      *
 *   BulletClass::Draw_It -- Displays the bullet at location specified.                        *
 *   BulletClass::In_Which_Layer -- Fetches the layer that the bullet resides in.              *
 *   BulletClass::Init -- Clears the bullets array for scenario preparation.                   *
 *   BulletClass::Is_Forced_To_Explode -- Checks if bullet should explode NOW.                 *
 *   BulletClass::Mark -- Performs related map refreshing under bullet.                        *
 *   BulletClass::Occupy_List -- Determines the bullet occupation list.                        *
 *   BulletClass::Shape_Number -- Fetches the shape number for the bullet object.              *
 *   BulletClass::Sort_Y -- Sort coordinate for bullet rendering.                              *
 *   BulletClass::Target_Coord -- Fetches coordinate to use when firing on this object.        *
 *   BulletClass::Unlimbo -- Transitions a bullet object into the game render/logic system.    *
 *   BulletClass::delete -- Bullet memory delete.                                              *
 *   BulletClass::new -- Allocates memory for bullet object.                                   *
 *   BulletClass::~BulletClass -- Destructor for bullet objects.                               *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "bullet.h"

#include "_convert.h"
#include "_map.h"
#include "_rules.h"
#include "_surface.h"
#include "_tactica.h"
#include "aircraft.h"
#include "anim.h"
#include "building.h"
#include "bullettype.h"
#include "ccrand.h"
#include "cell.h"
#include "combat.h"
#include "convert.h"
#include "draw.h"
#include "empulse.h"
#include "fly.h"
#include "house.h"
#include "houstype.h"
#include "infantry.h"
#include "inline.h"
#include "lightcon.h"
#include "map.h"
#include "missile.h"
#include "overtype.h"
#include "partsys.h"
#include "rules.h"
#include "savestream.h"
#include "scheme.h"
#include "stimer.h"
#include "sun.h"
#include "syncrechook.h"
#include "tactical.h"
#include "techno.h"
#include "tracker.h"
#include "unit.h"
#include "voxdrsys.h"
#include "warhead.h"
#include "weapon.h"

#include <algorithm>
#include <intrin.h>




/***********************************************************************************************
 * BulletClass::BulletClass -- Bullet constructor.                                             *
 *                                                                                             *
 *    This is the constructor for the bullet class. It handles all                             *
 *    initialization of the bullet and starting it in motion toward its                        *
 *    target.                                                                                  *
 *                                                                                             *
 * INPUT:   id       -- The type of bullet this is (could be missile).                         *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/02/1994 JLB : Created.                                                                 *
 *   06/20/1994 JLB : Firer is a base class pointer.                                           *
 *   12/10/1994 JLB : Auto calculate range optional.                                           *
 *   12/12/1994 JLB : Handles small arms as an instantaneous effect.                           *
 *   12/23/1994 JLB : Fixed scatter algorithm for non-homing projectiles.                      *
 *   12/31/1994 JLB : Removed range parameter (not needed).                                    *
 *=============================================================================================*/
BulletClass::BulletClass(void) :
	BASECLASS(),
	Class(NULL),
	Payback(NULL),
	IsInaccurate(false),
	Fuse(),
	IsBright(false),
	SmoothedClosure(0),
	field_A4(true),
	IsLaunching(true),
	BounceCount(0),
	TarCom(NULL),
	MaxSpeed(0),
	ClosureSamples(0),
	Warhead(NULL),
	AnimFrame(0),
	AnimRate(0)
{
	Create_ID();
	Bullets.Add(this);
}


/// <summary>
/// Fills in the firing data for a newly created projectile.
/// This routine is used by Create_Bullet to tell the projectile everything it must know
/// about the shot -- what it is, what it was aimed at, and who is to be credited for the
/// damage it causes.
/// </summary>
/// <param name="payback">The object that fired the shot. It receives credit for any kill.</param>
/// <param name="strength">The damage the projectile will inflict when it detonates.</param>
/// <param name="bright">Should the impact throw off a lighting flash?</param>
void BulletClass::Set_Bullet_Data(BulletTypeClass const *type, AbstractClass *target, TechnoClass *payback, int strength, WarheadTypeClass const *warhead, int max_speed, int range, bool bright)
{
	Class = (BulletTypeClass *)type;
	Payback = payback;
	TarCom = target;
	MaxSpeed = max_speed;
	Warhead = (WarheadTypeClass *)warhead;
	IsBright = bright;
	Strength = strength;
	Range = range;
	AnimFrame = 0;
	AnimRate = Class->AnimRate;
}


/***********************************************************************************************
 * BulletClass::~BulletClass -- Destructor for bullet objects.                                 *
 *                                                                                             *
 *    The bullet destructor must detect if a dog has been attached to this bullet. If so,      *
 *    then the attached dog must be unlimboed back onto the map. This operation is necessary   *
 *    because, unlike other objects, the dog flies with the bullet it fires.                   *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/06/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
BulletClass::~BulletClass(void)
{
	Detach_This_From_All(this, true);

	if (GameActive) {
#if NEVER
		/*
		**	SPECIAL CASE:
		**	The dog is attached to the dog bullet in a limbo state. When the bullet is
		**	destroyed, the dog must come back out of limbo at the closest location possible to
		**	the bullet.
		*/
		if (Payback != NULL && Payback->What_Am_I() == RTTI_INFANTRY && ((InfantryClass *)Payback)->Class->IsDog) {

			InfantryClass * dog = (InfantryClass *)Payback;
			if (dog) {
				bool unlimbo = false;
				Dir256 dogface = dog->PrimaryFacing;
				COORDINATE newcoord = Coord;

				/*
				**	Ensure that the coordinate, that the dog is to appear at, is legal. If not,
				**	then find a nearby legal location.
				*/
				if (Can_Enter_Cell(newcoord) != MOVE_OK) {
					newcoord = Map.Nearby_Location(newcoord.As_Cell(), dog->Class->Speed);
				}

				/*
				**	Try to put the dog down where the target impacted.  If we can't
				**	put it in that cell, then scan through the adjacent cells,
				**	starting with our current heading, until we find a place where
				**	we can put him down.  If all 8 adjacent cell checks fail, then
				**	just delete the dog.
				*/
				for (int i = FACING_NONE; i < FACING_COUNT; i++) {
					if (i != FACING_NONE) {
						newcoord = Adjacent_Cell(Coord, FacingType(i));
					}
					ScenarioInit++;
					if (dog->Unlimbo(newcoord, dog->PrimaryFacing)) {
						dog->Mark(MARK_DOWN);
						dog->Do_Action(DO_DOG_MAUL, true);
						if (dog->WasSelected) {
							dog->Select();
						}
						ScenarioInit--;
						unlimbo = true;
						break;
					}
					ScenarioInit--;
				}

				Payback = 0;

				if (!unlimbo) {
					delete dog;
				}
			}
		}
	#endif
		BulletClass::Limbo();
	}

	Class=0;
	Payback=0;
	Bullets.Delete(this);
}


/***********************************************************************************************
 * BulletClass::Occupy_List -- Determines the bullet occupation list.                          *
 *                                                                                             *
 *    This function will determine the cell occupation list and return a pointer to it. Most   *
 *    bullets are small and the list is usually short, but on occasion, it can be a list that  *
 *    rivals the size of regular vehicles.                                                     *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with a pointer to the cell offset list that covers all the cells a bullet  *
 *          is over.                                                                           *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/20/1994 JLB : Created.                                                                 *
 *   01/05/1995 JLB : Handles projectiles with altitude.                                       *
 *=============================================================================================*/
Cell const * BulletClass::Occupy_List(bool) const
{
	static Cell const _list[] = { REFRESH_EOL };

	return(_list);
}


/***********************************************************************************************
 * BulletClass::Mark -- Performs related map refreshing under bullet.                          *
 *                                                                                             *
 *    This routine marks the objects under the bullet so that they will                        *
 *    be redrawn. This is necessary as the bullet moves -- objects under                       *
 *    its path need to be restored.                                                            *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/02/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
bool BulletClass::Mark(MarkType mark)
{
	if (BASECLASS::Mark(mark)) {
		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * BulletClass::AI -- Logic processing for bullet.                                             *
 *                                                                                             *
 *    This routine will perform all logic (flight) logic on the bullet.                        *
 *    Primarily this is motion, fuse tracking, and detonation logic. Call                      *
 *    this routine no more than once per bullet per game tick.                                 *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/02/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void BulletClass::AI(void)
{
	/*
	 * This is how many frames the closure detector spends warming up, and also
	 * the closure value beneath which a homing projectile is judged to have
	 * stopped gaining on its target.
	 */
	static const int _limit = 4 * TICKS_PER_SECOND;

	Coord	coord;
	int		distance;

	BASECLASS::AI();

	int is_aircraft = false;

	if (!IsActive) return;

	/*
	**	Ballistic objects are handled here.
	*/
	bool collided = false;
	bool forced = false;			// Forced explosion.

	/*
	 * A dropping projectile that has finished its fall has arrived and must
	 * detonate.
	 */
	if (Class->IsDropping && !IsFalling) {
		forced = true;
	}

	/*
	 * Projectiles that carry an animation cycle through their frames here,
	 * looping back to the start of the sequence when the end is passed.
	 */
	if (Class->AnimLow || Class->AnimHigh) {
		AnimRate--;
		if (AnimRate == 0) {
			AnimRate = Class->AnimRate;
			AnimFrame++;
			if (AnimFrame > Class->AnimHigh) {
				AnimFrame = Class->AnimLow;
			}
		}
	}

	/*
	**	Move the projectile forward according to its speed
	**	and direction.
	*/
	coord = PositionCoord;

	/*
	 * Projectiles that leave a trail behind them drop one puff of it every
	 * third frame.
	 */
	if (Class->Trailer != NULL) {
		if (!(Frame % 3)) {
			new AnimClass(Class->Trailer, coord, 1, 1);
		}
	}

	ImpactType impact = IMPACT_NONE;

	/*
	 * There are two flight models. A projectile that has a rate of turn is
	 * flown by the missile autopilot, which steers it toward its target and
	 * over any terrain in the way. Everything else merely follows the arc it
	 * was launched along and bounces off whatever it lands on.
	 */
	if (Class->ROT > 0) {
		double speed = Velocity.Speed();
		int max_speed = MaxSpeed;

		/*
		 * The projectile stays in its launch phase until it has worked up to
		 * cruising speed. A fast projectile is given no launch phase at all.
		 */
		if (MaxSpeed >= 40 || speed + 0.5 >= max_speed) {
			IsLaunching = false;
		}

		int acceleration = Class->Acceleration;

		/*
		**	Homing projectiles constantly change facing to face toward the target but
		**	they only do so every other game frame (improves game speed and makes
		**	missiles not so deadly).
		*/
		if (IsLaunching) {
			acceleration = (Frame % 2 == 0) ? 1 : 0;
		}

		/*
		 * Work the projectile's speed toward its maximum, accelerating while it
		 * is flying too slowly and coasting down while it is flying too fast.
		 */
		if (speed < max_speed) {
			speed += acceleration;
			if (speed >= max_speed) {
				speed = max_speed;
			}
			Velocity.Set_Speed(speed);
		} else if (speed > max_speed) {
			speed -= acceleration / 2;
			if (speed <= 0) {
				speed = 0;
			}
			Velocity.Set_Speed(speed);
		}

		TVelocity3D<double> velocity = Velocity;

		/*
		 * Fetch the point to home on. An object is aimed at through its target
		 * coordinate so that the projectile strikes the body of the victim
		 * rather than the ground it happens to be standing on.
		 */
		Coord target_coord = TarCom != NULL ? TarCom->As_Coord() : COORD_NONE;
		ObjectClass * target = dynamic_cast<ObjectClass *>(TarCom);
		if (target != NULL) {
			target_coord = target->Target_Coord();
		}

		/*
		 * Swing the rate of turn above and below its nominal value over a
		 * fifteen frame cycle. This is what gives a missile its weaving flight
		 * path. The projectile's ID is mixed into the cycle so that missiles
		 * fired together do not all weave in unison.
		 */
		double & rotvar = Rule->MissileROTVar;
		double phase = ((Frame + Fetch_ID()) % TICKS_PER_SECOND) * (1.0 / TICKS_PER_SECOND);
		int rot = (std::sin(phase * DEG_TO_RAD(360)) * rotvar + (rotvar + 1)) * Class->ROT;

		/*
		 * Allow a sharper turn over the last cell of the approach so that the
		 * projectile can still catch a target that dodges at the last moment.
		 */
		if (Center_Coord().Distance_To(target_coord) < CELL_LEPTON) {
			rot *= 1.5;
		}

		Coord old_coord = coord;

		/*
		 * A projectile chasing an aircraft flies straight at its quarry rather
		 * than hugging the terrain on the way, since there is no ground between
		 * the two of them that needs clearing.
		 */
		if (TarCom != NULL && TarCom->RTTI == RTTI_AIRCRAFT) {
			is_aircraft = true;
		}

		distance = Projectile_Motion(coord, velocity, target_coord, DirType((Dir256)(IsLaunching == false ? rot : 0)), (bool &)is_aircraft, Class->IsAirburst, Class->IsVeryHigh);
		CellClass * cellptr = &Map[coord];
		TVelocity3D<double> new_velocity = velocity;
		Velocity = new_velocity;

		/*
		 * The projectile has arrived once it is within half a frame's travel of
		 * its target or it has flown into the ground. One that arrives while
		 * still airborne is snapped onto the target first so that the explosion
		 * is seen on the victim. An airburst projectile is left where it is,
		 * since it is meant to go off overhead.
		 */
		double half_speed = Velocity.Speed() * 0.5;
		if (distance <= half_speed || HeightAGL <= 0) {
			forced = true;
			impact = IMPACT_NORMAL;
			if (HeightAGL > 0 && !Class->IsAirburst) {
				coord = target_coord;
			}
		}

		/*
		 * Keep a smoothed measure of how quickly the projectile is gaining on
		 * its target. The first sixty frames are merely accumulated; after that
		 * the measure decays toward the closure of the moment. Should it fall
		 * away to nearly nothing, then the projectile is circling its target
		 * instead of closing on it and is forced to detonate rather than fly
		 * forever. Airburst and very high projectiles are exempt, since they
		 * come down on their targets from above.
		 */
		int delta = ::Distance(old_coord, target_coord) - ::Distance(coord, target_coord);

		if (!IsLaunching) {
			if (ClosureSamples < _limit) {
				ClosureSamples++;
				SmoothedClosure += delta;
			} else {
				SmoothedClosure = SmoothedClosure * ((4 * TICKS_PER_SECOND - 1.0) / (4 * TICKS_PER_SECOND)) + delta;
				if (SmoothedClosure >= 0 && SmoothedClosure < _limit) {
					if (!Class->IsAirburst && !Class->IsVeryHigh) {
						forced = true;
						impact = IMPACT_NORMAL;
					}
				}
			}
		}

		/*
		 * A projectile that crossed the plane of a bridge deck during this step,
		 * from either side, strikes the bridge instead of passing through it.
		 */
		if (impact == IMPACT_NONE && (cellptr->IsUnderBridge || Map[old_coord].IsUnderBridge)) {
			int bridge_height = Map.Get_Height_GL(coord) + BRIDGE_LEPTON_HEIGHT;
			if (coord.Z > bridge_height && old_coord.Z < bridge_height || coord.Z < bridge_height && old_coord.Z > bridge_height) {
				impact = IMPACT_NORMAL;
				coord.Z = bridge_height;
				forced = true;
			}
		}
	} else {
		/*
		 * The position local briefly holds the current velocity: it seeds the
		 * two velocity locals and the slow-speed check before being reused as
		 * the floating point position for the flight step below.
		 */
		TVelocity3D<double> position = Velocity;
		TVelocity3D<double> new_velocity = position;
		TVelocity3D<double> velocity(position.X, position.Y, position.Z);

		/*
		 * A projectile that is barely moving at all has nothing left to do but
		 * land.
		 */
		if (position.Speed() < 8) {
			impact = IMPACT_NORMAL;
		}
		position.X = coord.X;
		position.Y = coord.Y;
		position.Z = coord.Z;

		/*
		 * Floating projectiles fall at half gravity, which is what gives them
		 * their lazy drifting arc.
		 */
		double gravity = Rule->Gravity;
		if (Class->IsFloater) {
			gravity = Get_Floater_Gravity();
		}

		TechnoClass * payback = Payback;
		double elasticity = Class->Elasticity;

		/*
		 * Pull the projectile down by gravity and then step it forward along
		 * its arc.
		 */
		velocity.Z -= gravity;

		Coord old_coord(position.X, position.Y, position.Z);
		position += velocity;

		Coord new_coord(position.X, position.Y, position.Z);
		int height = Map.Get_Height_GL(new_coord);
		int bridge_height = height + BRIDGE_LEPTON_HEIGHT;

		CellClass * cellptr = &Map[new_coord];

		/*
		 * See if the arc carried the projectile through the deck of a bridge,
		 * either dropping onto it from above or rising into it from below.
		 */
		bool fell_through_bridge = false;
		bool rose_through_bridge = false;
		if (cellptr->IsUnderBridge || Map[old_coord].IsUnderBridge) {
			if (new_coord.Z >= bridge_height) {
				if (old_coord.Z < bridge_height) {
					rose_through_bridge = true;
				}
			} else if (old_coord.Z >= bridge_height) {
				fell_through_bridge = true;
			}
		}

		/*
		 * Otherwise see if the projectile ran into a wall or a building on the
		 * way. The firer's own building, a dormant laser fence, a vehicle that
		 * happens to be deployed, and anything belonging to an ally are all
		 * passed through harmlessly.
		 */
		bool hit_obstacle = false;
		if (!fell_through_bridge && !rose_through_bridge) {
			if (position.Z >= height && position.Z - 150 < height) {
				BuildingClass * building = cellptr->Cell_Building();
				if (building != NULL || cellptr->Has_Wall_Or_Gate()) {
					hit_obstacle = true;
					if (building != NULL) {
						if (building == payback || building->Class->IsLaserFence && building->LaserFenceFrame >= 8) {
							hit_obstacle = false;
						}
						if (building->Considered_Vehicle()) {
							hit_obstacle = false;
						}
						if (payback != NULL && payback->House->Is_Ally(building)) {
							hit_obstacle = false;
						}
					}
				}
			}
		}

		/*
		 * The projectile bounces unless it is still clear of the ground and has
		 * struck nothing along the way.
		 */
		bool do_bounce = true;
		if (position.Z >= height) {
			if (!fell_through_bridge && !rose_through_bridge && !hit_obstacle) {
				do_bounce = false;
			}
		}

		if (do_bounce) {
			/*
			 * Bring the projectile back to the surface it struck -- the deck it
			 * dropped onto, the underside of the deck it rose into, or the
			 * ground itself.
			 */
			if (fell_through_bridge) {
				position.Z = bridge_height;
			} else if (rose_through_bridge) {
				bridge_height -= 20;
				position.Z = bridge_height;
			} else {
				bridge_height = height - 100;
				if (bridge_height < position.Z) {
					position.Z = height;
				}
			}

			/*
			 * Bounce the projectile off that surface. Its velocity is rotated
			 * into the frame of reference of the ground slope, damped by the
			 * projectile's elasticity, reflected in Z, and then rotated back
			 * out again.
			 */
			Matrix3D slope_matrix = Get_Slope_Matrix(TacticalMap->Get_Cell_Ramp(new_coord));
			Matrix3D slope_matrix_inv = Matrix3D::Orthogonal_Inverse(slope_matrix);
			Vector3 vec(velocity.X, -velocity.Y, velocity.Z);
			vec = slope_matrix_inv.Rotate_Vector(vec);
			vec *= elasticity;
			vec.Z = -vec.Z;
			vec = slope_matrix.Rotate_Vector(vec);
			velocity.X = vec.X;
			velocity.Y = -vec.Y;
			velocity.Z = vec.Z;

			/*
			 * See who is standing in the cell the projectile is bouncing out
			 * of, taking care to look on top of a bridge rather than beneath it
			 * when the projectile is riding above the deck. Neither the firer
			 * nor an ally of the firer counts as a victim.
			 */
			bool on_bridge = Map[coord].IsUnderBridge && Map.Get_Height_GL(coord) + BRIDGE_LEPTON_HEIGHT <= position.Z;
			TechnoClass * techno = Map[coord].Cell_Techno(Point2D(0, 0), on_bridge);
			if (Payback != NULL && coord.As_Cell() == Payback->Center_Coord().As_Cell() || techno != NULL && Payback != NULL && Payback->House->Is_Ally(techno)) {
				techno = NULL;
			}

			/*
			 * Only a bouncy projectile survives the impact, and even then only
			 * if it did not come down on top of somebody.
			 */
			if (!Class->IsBouncy || techno != NULL && (Payback == NULL || techno != Payback)) {
				impact = IMPACT_NORMAL;
				forced = true;
				collided = true;
			}

			/// This test can never pass -- the check above already forces a
			/// non-bouncy projectile to explode.

			/*
			 * A projectile aimed at a cell rather than at an object was to
			 * detonate as soon as it landed in that cell.
			 */
			if (!Class->IsBouncy && !forced) {
				if (Is_Target_Cell(TarCom)) {
					if (TarCom->Center_Coord().As_Cell() == coord.As_Cell()) {
						impact = IMPACT_NORMAL;
						forced = true;
						collided = true;
					}
				}
			}

			/*
			 * A projectile is allowed only so many bounces before it is forced
			 * to detonate wherever it happens to have landed.
			 */
			BounceCount++;
			if (!forced && BounceCount >= 3) {
				impact = IMPACT_NORMAL;
				forced = true;
			}
		}

		coord.X = (int)position.X;
		coord.Y = (int)position.Y;
		coord.Z = (int)position.Z;

		/*
		 * A projectile that is still flying detonates anyway if it passed close
		 * enough to somebody to count as a hit. The firer and its allies are
		 * ignored.
		 */
		if (!forced) {
			TechnoClass * techno = Map[coord].Cell_Techno();
			if (Payback == NULL || techno != Payback) {
				if (techno != NULL && (Payback == NULL || !Payback->House->Is_Ally(techno))) {
					if (coord.Distance_To(techno->PositionCoord) < CELL_LEPTON / 2) {
						forced = true;
						impact = IMPACT_NORMAL;
						coord = techno->PositionCoord;
					}
				}
			}
		}

		/*
		 * A projectile that has flown off the edge of the world is left at its
		 * last known position and vanishes.
		 */
		if (!Map.In_Radar(coord)) {
			coord = PositionCoord;
			forced = true;
			impact = IMPACT_EDGE;
		}

		new_velocity.X = velocity.X;
		new_velocity.Y = velocity.Y;
		Velocity = TVelocity3D<double>(new_velocity.X, new_velocity.Y, velocity.Z);

		/*
		 * A projectile that is barely moving and lying close to the ground has
		 * finished bouncing and settles where it is.
		 */
		if (Velocity.Speed() < 10 && HeightAGL < 10) {
			forced = true;
			impact = IMPACT_NORMAL;
		}
	}

	switch (impact) {
		/*
		**	When a projectile reaches the edge of the world, it
		**	vanishes from existence -- presumed to explode off
		**	map.
		*/
		case IMPACT_EDGE:
			Mark();
			Delete_Me();
			break;

		default:
		case IMPACT_NONE:

		/*
		**	The projectile has moved. Check its fuse. If detonation
		**	is signaled, then do so. Otherwise, just move.
		*/
		case IMPACT_NORMAL: {
			Mark();

			/*
			 * A fueled projectile carries only so much range with it. Once the
			 * fuel is spent, it detonates wherever it happens to be.
			 */
			if (Class->IsFueled) {
				Range -= coord.Distance_To(PositionCoord);
				if (Range <= 0) {
					forced = true;
				}
			}
			PositionCoord = coord;

			/*
			 * A projectile that flies into an active firestorm wall is consumed
			 * by it. The wall lets its owner's own fire through, as it does any
			 * projectile built to ignore firestorms.
			 */
			BuildingClass * building = Map[coord].Cell_Building();
			if (building != NULL && building->Class->IsFirestormWall && building->House->FirestormDefenseActivated) {
				if (Payback == NULL || building->House != Payback->House) {
					if (Class_Of() != NULL && !Class_Of()->IsIgnoresFirestorm) {
						building->Crossing_Firestorm(this, false);
						Delete_Me();
						return;
					}
				}
			}

			/*
			**	See if the bullet should be forced to explode now in spite of what
			**	the fuse would otherwise indicate. Maybe the bullet hit a wall?
			*/
			if (!forced) {
				Coord cur_coord = PositionCoord;
				forced = Is_Forced_To_Explode(cur_coord);
				PositionCoord = cur_coord;
			}

			/*
			**	If the bullet is not to explode, then perform normal flight
			**	maintenance (usually nothing). Otherwise, explode and then
			**	delete the bullet.
			*/
			FuseResultType fuse = Is_Homing() ? Fuse.Fuse_Checkup(coord) : FUSE_WAIT;
			if (!forced && (Class->IsDropping || fuse == FUSE_WAIT)) {
				/*
				**	Certain projectiles lose strength when they travel.
				*/
				if (Class->IsDegenerate && Strength > 5) {
					Strength--;
				}

			} else {
				/*
				 * Nudge the detonation onto the target itself when the
				 * projectile went off close by, so that the explosion is seen
				 * to strike the victim rather than the air alongside it. A
				 * collision is judged three times as generously as a mere fuse
				 * expiry, and an airburst projectile is always left where it
				 * went off.
				 */
				if (TarCom != NULL) {
					if ((fuse == FUSE_EXPLODE_CLOSE || collided) && !Class->IsAirburst) {
						Coord midpoint = coord;
						Coord target_coord = TarCom->As_Coord();
						midpoint.Z = (midpoint.Z + target_coord.Z) / 2;
						int target_distance = midpoint.Distance_To(target_coord);
						if (collided) {
							target_distance /= 3;
						}

						if (fuse == FUSE_EXPLODE_CLOSE || target_distance <= std::max(CELL_LEPTON / 2.0, Velocity.Speed() * 2)) {
							PositionCoord = TarCom->Center_Coord();
						}
					}
				}
				Bullet_Explodes(forced);
				Delete_Me();
			}
			break;
		}
	}

}


/***********************************************************************************************
 * BulletClass::Shape_Number -- Fetches the shape number for the bullet object.                *
 *                                                                                             *
 *    Use this routine to fetch a shape number to use for this bullet object.                  *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the shape number to use when drawing this bullet.                     *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/06/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
int BulletClass::Shape_Number(void) const
{
	int shapenum = 0;

	if (!Class->IsFaceless) {
		shapenum = UnitClass::BodyShape[DirType(std::atan2(-Velocity.Y, Velocity.X)).As_Dir32()];
	}

	if (Class->AnimLow || Class->AnimHigh) {
		shapenum = AnimFrame;
	}

	return(shapenum);
}


/***********************************************************************************************
 * BulletClass::Draw_It -- Displays the bullet at location specified.                          *
 *                                                                                             *
 *    This routine displays the bullet visual at the location specified.                       *
 *                                                                                             *
 * INPUT:   x,y   -- The center coordinate to render the bullet at.                            *
 *                                                                                             *
 *          window   -- The window to clip to.                                                 *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/20/1994 JLB : Created.                                                                 *
 *   06/27/1994 JLB : Takes a window clipping parameter.                                       *
 *   01/08/1995 JLB : Handles translucent colors if necessary.                                 *
 *=============================================================================================*/
void BulletClass::Draw_It(Point2D const & point, Rect const & cliprect) const
{
	if (Scen->Special.IsFogOfWar && Map.Is_Fogged(PositionCoord)) return;

	/*
	**	Certain projectiles aren't visible. This includes small bullets (which are actually
	**	invisible) and flame thrower flames (which are rendered as an animation instead of a projectile).
	*/
	if (Class->IsInvisible) return;

	if (Class->IsVoxel) {
		Matrix3D matrix(true);
		matrix.Rotate_Z(Velocity.Get_Yaw().As_Radian32());
		matrix.Rotate_Y(-Velocity.Get_Pitch().As_Radian32());
		Draw_Voxel(Class->Voxel, Get_Isometric_View_Matrix() * matrix, point, cliprect, 0, SHAPE_ALPHA, NORMAL_LIGHT);

	} else {

		/*
		**	If there is no shape loaded for this object, then
		**	it obviously can't be rendered -- just bail.
		*/
		ShapeSet const * shapeptr = (ShapeSet const *)Get_Image_Data();
		if (shapeptr == NULL) return;

		/*
		**	Get the basic shape number for this projectile.
		*/
		int shapenum = Shape_Number();

		int height_agl = HeightAGL;
		int height_gl = Map.Get_Height_GL(PositionCoord);

		if (!IsOnBridge && Map[(Coord const &)PositionCoord].IsUnderBridge && height_agl >= BRIDGE_LEPTON_HEIGHT) {
			height_agl -= BRIDGE_LEPTON_HEIGHT;
			height_gl += BRIDGE_LEPTON_HEIGHT;
		}

		/*
		**	For flying projectiles, draw the shadow and adjust the actual projectile body
		**	render position.
		*/
		if (height_agl > 0) {
			Point2D shadowpoint = point + Point2D(0, TacticalMap->Z_Lepton_To_Pixel(height_agl));
			Draw_Shape(*LogicalSurface, *NormalDrawer, shapeptr, shapenum, shadowpoint, cliprect, ShapeFlags_Type(SHAPE_DARKEN|SHAPE_CENTER|SHAPE_WIN_REL|SHAPE_ZGRAD), 0, -10 - TacticalMap->Z_Lepton_To_Pixel(height_gl));
		}

		/*
		**	Draw the main body of the projectile.
		*/
		ConvertClass * drawer = NormalDrawer;
		if (Class->IsAnimPalette) {
			drawer = AnimDrawer;
		}
		Draw_Shape(*LogicalSurface, *drawer, shapeptr, shapenum, point, cliprect, ShapeFlags_Type(SHAPE_CENTER|SHAPE_WIN_REL|SHAPE_ALPHA|SHAPE_ZGRAD), 0, -30 - TacticalMap->Z_Lepton_To_Pixel(Height));
	}
}


/***********************************************************************************************
 * BulletClass::Detach -- Removes specified target from this bullet's targeting system.        *
 *                                                                                             *
 *    When an object is removed from the game system, it must be removed all targeting and     *
 *    tracking systems as well. This routine is used to remove the specified object from the   *
 *    bullet. If the object isn't part of this bullet's tracking system, then no action is     *
 *    performed.                                                                               *
 *                                                                                             *
 * INPUT:   target   -- The target to remove from this tracking system.                        *
 *                                                                                             *
 *          all      -- Is the target going away for good as opposed to just cloaking/hiding?  *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/24/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void BulletClass::Detach(AbstractClass const * target, bool all)
{
	BASECLASS::Detach(target, all);

#if 0
	ObjectClass * obj = As_Object(target);
	if (Payback != NULL && obj == Payback) {

		/*
		**	If we're being called as a result of the dog that fired us being put
		**	in limbo, then don't detach.  If for any other reason, detach.
		*/
		if (Payback->What_Am_I() != RTTI_INFANTRY || !((InfantryClass *)Payback)->Class->IsDog) {
			Payback = 0;
		}
	}

	if (all && target == TarCom) {
		TarCom = NULL;
	}
#endif

	if (Payback == target) {
		Payback = NULL;
	}
	if (TarCom == target) {
		TarCom = NULL;
	}
	if (Class == target) {
		Class = NULL;
	}
}


/***********************************************************************************************
 * BulletClass::Unlimbo -- Transitions a bullet object into the game render/logic system.      *
 *                                                                                             *
 *    This routine is used to take a bullet object that is in limbo and transition it to the   *
 *    game system. A bullet object so transitioned, will be drawn and logic processing         *
 *    performed. In effect, it comes into existence.                                           *
 *                                                                                             *
 * INPUT:   coord -- The location where the bullet object is to appear.                        *
 *                                                                                             *
 *          dir   -- The initial facing for the bullet object.                                 *
 *                                                                                             *
 * OUTPUT:  bool; Was the unlimbo successful?                                                  *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/10/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
bool BulletClass::Unlimbo(Coord const & coord, TVelocity3D<double> const & velocity)
{
	/*
	**	Try to unlimbo the bullet as far as the base class is concerned. Use the already
	**	set direction and strength if the "punt" values were passed in. This allows a bullet
	**	to be setup prior to being launched.
	*/
	if (BASECLASS::Unlimbo(coord)) {
		Velocity = velocity;
		Map.Remove(this);

		Coord tcoord = TarCom->As_Coord();

		/*
		**	Possibly adjust the target if this projectile is inaccurate. This occurs whenever
		**	certain weapons are trained upon targets they were never designed to attack. Example: when
		**	turrets or anti-tank missiles are fired at infantry. Indirect
		**	fire is inherently inaccurate.
		*/
		if (IsInaccurate || Class->IsInaccurate ||
			(dynamic_cast<CellClass *>(TarCom) != NULL || dynamic_cast<InfantryClass *>(TarCom) != NULL) /*&& (Warhead == WARHEAD_AP || Class->IsFueled))*/) {
#if 0
			/*
			**	Inaccuracy for low velocity or homing projectiles manifests itself as a standard
			**	Circular Error of Probability (CEP) algorithm. High speed projectiles usually
			**	just overshoot the target by extending the straight line flight.
			*/
			if (/*Class->ROT != 0 ||*/ Class->IsArcing) {
				int scatterdist = (::Distance(coord, tcoord)/16)-CELL_LEPTON / 4;
				scatterdist = std::min(scatterdist, Rule->HomingScatter);
				scatterdist = std::max(scatterdist, 0);

				dir = (Dir256)((dir + (Random_Pick(0, 10)-5)) & DIR_MAX);
				tcoord = Coord_Scatter(tcoord, Random_Pick(0, scatterdist));
			} else {
				int scatterdist = (::Distance(coord, tcoord)/16)-CELL_LEPTON / 4;
				scatterdist = std::min(scatterdist, Rule->BallisticScatter);
				scatterdist = std::max(scatterdist, 0);
				tcoord = Move_Coord(tcoord, dir, Random_Pick(0, scatterdist));
			}
#endif
		}

		/*
		**	For very fast and invisible projectiles, just make the projectile exist at the target
		**	location and dispense with the actual flight.
		*/
		if (Class->IsInvisible) {
			Coord firestorm_coord = Map.Firestorm_On_Path(coord, tcoord, Payback != NULL ? Payback->House : NULL);
			if (firestorm_coord == COORD_NONE) {
				PositionCoord = tcoord;
				MaxSpeed = 0;
				Velocity.Set_Speed(0);
			} else {
				firestorm_coord.Z = Map.Get_Height_GL(firestorm_coord);
				PositionCoord = firestorm_coord;
				BuildingClass * bptr = Map[firestorm_coord].Cell_Building();
				bptr->Crossing_Firestorm(this, false);
				Delete_Me();
			}
		}

		/*
		**	Arm the fuse.
		*/
		Fuse.Arm_Fuse(PositionCoord, tcoord, dynamic_cast<AircraftClass *>(TarCom) != NULL ? 0 : Class->Arming);

		if (Is_Homing()) {
			Velocity.Set_Speed(1);
		}

		if (IsActive) {
			Map.Submit(this);
		}
		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * BulletClass::In_Which_Layer -- Fetches the layer that the bullet resides in.                *
 *                                                                                             *
 *    This examines the bullet to determine what rendering layer it should be in. The          *
 *    normal logic applies unless this is a torpedo. A torpedo is always in the surface        *
 *    layer.                                                                                   *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the render layer that this bullet should reside in.                   *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/10/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
LayerType BulletClass::In_Which_Layer(void) const
{
	return(LAYER_AIR);
}


/***********************************************************************************************
 * BulletClass::Is_Forced_To_Explode -- Checks if bullet should explode NOW.                   *
 *                                                                                             *
 *    This routine will examine the bullet and where it is travelling in order to determine    *
 *    if it should prematurely explode. Typical of this would be when a bullet hits a wall     *
 *    or a torpedo hits a ship -- regardless of where the projectile was originally aimed.     *
 *                                                                                             *
 * INPUT:   coord -- The new coordinate to place the bullet at presuming it is forced to       *
 *                   explode and a modification of the bullet's coordinate is needed.          *
 *                   Otherwise, the coordinate is not modified.                                *
 *                                                                                             *
 * OUTPUT:  bool; Should the bullet explode now?                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/10/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
bool BulletClass::Is_Forced_To_Explode(Coord & coord) const
{
	coord = PositionCoord;
	CellClass const * cellptr = &Map[coord];
	int height = HeightAGL;

	/*
	**	Check for impact on a wall or other high obstacle.
	*/
	if (!Class->IsHigh && cellptr->Overlay != OVERLAY_NONE && OverlayTypes[cellptr->Overlay]->IsHigh && height < 100) {
		return(true);
	}

	if (height < 0) {
		return(true);
	}

	/*
	**	Bullets are generally more effective when they are fired at aircraft.
	*/
	if (Class->IsAntiAircraft && TarCom != NULL &&
		(TarCom->RTTI == RTTI_AIRCRAFT || (TarCom->RTTI == RTTI_INFANTRY && ((InfantryClass *)TarCom)->Is_JumpJet()) && ((InfantryClass *)TarCom)->HeightAGL > 0) &&
		Distance(TarCom) < CELL_LEPTON / 2) {

		return(true);
	}

	/*
	**	No reason for forced explosion was detected, so return 'false' to
	**	indicate that no forced explosion is required.
	*/
	return(false);
}


/***********************************************************************************************
 * BulletClass::Bullet_Explodes -- Performs bullet explosion logic.                            *
 *                                                                                             *
 *    This handles the exploding bullet action. It will generate the animation and the         *
 *    damage as necessary.                                                                     *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   The bullet should be deleted after this routine is called.                      *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/10/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void BulletClass::Bullet_Explodes(bool forced)
{
	Coord coord = PositionCoord;
	TechnoClass *target = NULL;
	if (TarCom != NULL && TarCom->In_Air()) {
		target = (TechnoClass *)TarCom;
	}

	if (TarCom != NULL) {
		//if (::Distance(coord, TarCom->Center_Coord()) < 32) {
		Coord c = coord - TarCom->Center_Coord();
		if (c.Length() < 32) {
			if (!Class->IsAirburst && !Class->IsInaccurate) {
				coord = TarCom->Center_Coord();
			}
		}
	}

	if (!Warhead->IsEMEffect) {
		if (!Class->IsSplits) {
			if (!forced && !Class->IsArcing && !Is_Homing() && Fuse.Fuse_Target() != COORD_NONE) {
				coord = Fuse.Fuse_Target();
			}

			if (target != NULL && target->In_Which_Layer() != LAYER_GROUND) {
				if (Distance(TarCom) < CELL_LEPTON / 2) {
					coord = target->Target_Coord();
				}
			} else {
				if (TarCom != NULL && Distance(TarCom) < 42) {
					coord = TarCom->As_Coord();
				}
			}
		}
	}

	if (!Class->IsSplits) {
		Coord newcoord = coord;
		for (int i = 0; i < Class->Cluster; i++) {
			Detonate(coord);
			if (!IsActive) {
				break;
			}
			coord = Coord_Scatter(newcoord, Random_Pick(CELL_LEPTON, CELL_LEPTON * 2));
		}
	} else {
		Detonate(coord);
	}
}


/// <summary>
/// Performs the projectile's damage effect at the location specified.
/// This routine applies whatever the warhead calls for -- an electromagnetic pulse, a
/// tiberium web, or ordinary blast damage -- and then spawns the impact animation and any
/// lighting flash. Cluster projectiles use this moment to choose their sub-targets and
/// launch their bomblets.
/// </summary>
/// <param name="coord">The location that the projectile detonates at.</param>
void BulletClass::Detonate(Coord const & coord)
{
	WarheadTypeClass * warhead = Warhead;

	if (warhead->IsEMEffect) {
		new EMPulseClass(coord.As_Cell(), Warhead->SpreadFactor, Strength, Payback);
	}

	else if (warhead->IsWebby) {
		int radius = warhead->WebRadius;
		int radius_squared = radius * radius;
		Cell center = Center_Coord().As_Cell();
		int max_height = 3 * LEVEL_LEPTON_H;

		for (int y = -radius; y <= radius; y++) {
			for (int x = -radius; x <= radius; x++) {
				if (y * y + x * x <= radius_squared) {
					CellClass * cellptr = &Map[center + Cell(x, y)];
					if (cellptr != NULL) {
						Coord particle_coord = cellptr->Cell_Coord();
						ObjectClass * occupier = NULL;
						if (abs(particle_coord.Z - coord.Z) < max_height) {
							occupier = cellptr->Cell_Occupier();
						} else {
							particle_coord = cellptr->As_Coord();
							occupier = cellptr->Cell_Occupier(true);
						}
						ParticleSystemClass * psys = new ParticleSystemClass(Warhead->Particle, particle_coord);
						psys->Spawn_Held_Particle(particle_coord, particle_coord);
						while (occupier != NULL) {
							ObjectClass * next = occupier->Next;
							if (occupier->IsActive && occupier->IsDown && occupier->Strength > 0) {
								int damage = 0;
								occupier->Take_Damage(damage, 0, Warhead);
							}
							occupier = next;
						}
					}
				}
			}
		}
	}

	/*
	**	Non-aircraft targets apply damage to the ground.
	*/
	else {
		Explosion_Damage(coord, Strength, Payback, warhead, true);
		if (!IsActive) return;
	}

	Coord blast_coord = coord;

	/*
	**	For projectiles that are invisible while travelling toward the target,
	**	allow scatter effect for the impact animation.
	*/
	if (Class->IsInvisible) {
		blast_coord = Coord_Scatter(blast_coord, CELL_LEPTON / 8);
	}

	/*
	**	Fetch the land type that the explosion will be upon. Special case for
	**	flying aircraft targets, their land type will be LAND_NONE.
	*/
	LandType land = LAND_NONE;
	if (blast_coord.Z - Map.Get_Height_GL(blast_coord) < 2 * LEVEL_LEPTON_H) {
		land = Map[(Coord const &)PositionCoord].Land_Type();
	}

	const AnimTypeClass * anim = Combat_Anim(Strength, Warhead, land, PositionCoord);

	if (IsBright) {
		Combat_Lighting(blast_coord, Strength, Warhead, true);
	}

	if (anim != NULL) {
		AnimClass * aptr = new AnimClass(anim, blast_coord, 0, 1, ShapeFlags_Type(SHAPE_ZGRAD|SHAPE_WIN_REL|SHAPE_CENTER), Get_Explosion_Z(blast_coord));
		/*
		**	Special case trap: if they're making the nuclear explosion,
		**	and no anim is available, force the nuclear damage anyway
		**	because nuke damage is done in the middle of the animation
		**	and if there's no animation, there won't be any damage.
		*/
		if (!aptr && Warhead == Rule->NukeWarhead) {
			HousesType house = HOUSE_NONE;
			if (Payback) {
				house = Payback->House->Class->House;
			}
			AnimClass::Do_Atom_Damage(house, blast_coord);
		}
	}

	if (Class->IsSplits) {
		DynamicVectorClass<AbstractClass *> targets;

		Coord split_coord = PositionCoord;
		Cell split_cell = Center_Coord().As_Cell();

		if (TarCom != NULL) {
			ObjectClass * object = TarCom->As_ObjectClass();
			if (object != NULL) {
				split_coord = object->PositionCoord;
				split_cell = object->Center_Coord().As_Cell();
			}
		}

		int i;
		for (i = 0; i < Technos.Count(); i++) {
			TechnoClass * tptr = Technos[i];
			if (tptr->IsLocked && tptr->IsDown && tptr->Strength > 0) {
				if (split_coord.Distance_To(tptr->PositionCoord) < CELL_LEPTON * 5) {
					targets.Add(tptr);
				}
			}
		}

		while (targets.Count() < Class->Cluster) {
			Cell newcell = split_cell;
			newcell.X += Random_Pick(-3, 3);
			newcell.Y += Random_Pick(-3, 3);
			targets.Add(&Map[newcell]);
		}

		for (i = 0; i < Class->Cluster; i++) {
			AbstractClass * target = TarCom;
			if (target == NULL || Random_Double(0.0, 1.0) > Class->RetargetAccuracy) {
				int number = Random_Pick(0, targets.Count() - 1);
				target = targets[number];
				if (target == Payback && Random_Double(0.0, 1.0) > 0.5) {
					number = Random_Pick(0, targets.Count() - 1);
					target = targets[number];
				}
				targets.Delete_Index(number);
			}

			if (target != NULL) {
				WeaponTypeClass * weapon = Class->AirburstWeapon;
				const BulletTypeClass * btype = weapon->Bullet;
				BulletClass * bullet = Create_Bullet(btype, target, Payback, weapon->Attack * 10, weapon->WarheadPtr, Class->IsSplits ? 50 : Velocity.Speed(), weapon->Range, false);
				if (bullet != NULL) {
					/*
					 * Send the bomblet straight down (pitch DIR_S) with a dash of
					 * random scatter in the heading.
					 */
					TVelocity3D<double> velocity(DirType((Dir256)Random_Pick(DIR_MIN, DIR_NE)), DirType(DIR_S), weapon->MaxSpeed);
					bullet->Unlimbo(PositionCoord, velocity);
				}
			}
		}
	}
}


/// <summary>
/// Lists the members this projectile carries.
/// </summary>
/// <param name="stream">The stream carrying the members.</param>
void BulletClass::Serialize(SaveStreamClass & stream)
{
	BASECLASS::Serialize(stream);

	stream.Serialize(Class);
	stream.Serialize(Payback);
	stream.Serialize(IsInaccurate);
	stream.Serialize(Fuse);
	stream.Serialize(IsBright);
	stream.Serialize(Velocity);
	stream.Serialize(BounceCount);
	stream.Serialize(field_A4);
	stream.Serialize(IsLaunching);
	stream.Serialize(TarCom);
	stream.Serialize(MaxSpeed);
	stream.Serialize(ClosureSamples);
	stream.Serialize(SmoothedClosure);
	stream.Serialize(Warhead);
	stream.Serialize(AnimFrame);
	stream.Serialize(AnimRate);
	stream.Serialize(Range);
}


/// <summary>
/// Can this projectile steer toward its target?
/// The flight logic calls this routine to decide whether the projectile should be turned
/// toward its target each game frame or simply left to follow its launch trajectory.
/// </summary>
/// <returns>bool; Is this projectile a homing type?</returns>
bool BulletClass::Is_Homing(void) const
{
	if (Class->ROT > 0) {
		return(true);
	}
	return(false);
}


/// <summary>
/// Creates a projectile and fills in the data for the shot.
/// The projectile is inert until it is unlimboed with a starting position and velocity.
/// </summary>
/// <param name="payback">The object that fired the shot. It receives credit for any kill.</param>
/// <param name="strength">The damage the projectile will inflict when it detonates.</param>
/// <param name="bright">Should the impact throw off a lighting flash?</param>
/// <returns>Returns with a pointer to the projectile created, or NULL if one could not be
/// made.</returns>
BulletClass * Create_Bullet(BulletTypeClass const *type, AbstractClass *target, TechnoClass *payback, int strength, WarheadTypeClass const *warhead, int max_speed, int range, bool bright)
{
	BulletClass * bullet = new BulletClass;
	bullet->Set_Bullet_Data(type, target, payback, strength, warhead, max_speed, range, bright);
	return(bullet);
}


/// <summary>
/// Draws the voxel model for this projectile.
/// This routine poses every layer of the model, renders it through the voxel draw system,
/// and then blits the result to the logical surface in the projectile's own color scheme.
/// </summary>
/// <param name="voxeldata">The voxel and motion libraries to draw the projectile with.</param>
/// <param name="cliprect">The clipping rectangle to draw within.</param>
/// <param name="frame">The motion library frame to pose the model with.</param>
void BulletClass::Draw_Voxel(VoxelDataStruct const & voxeldata, Matrix3D const & transform, Point2D const & drawpoint, Rect const & cliprect, int frame, ShapeFlags_Type flags, int brightness) const
{
	VoxelLibrary * voxlib = voxeldata.VoxLib;
	MotionLibrary * motlib = voxeldata.MotLib;

	VoxelDrawSystem::Precalculate_Light(voxlib, 0, 0, transform, VoxelLightSource);
	VoxelDrawSystem::Reset();

	for (unsigned int i = 0; i < voxlib->Get_Layer_Count(); i++) {
		Matrix3D matrix;
		/// Without a motion library the matrix is left uninitialized and used as it stands.
		if (motlib != NULL) {
			Matrix3D hva_transform = motlib->Get_Layer_Matrix(i, frame);
			matrix = transform * hva_transform;
		}
		VoxelDrawSystem::Prep_For_Object(voxlib, i, 0, VoxelCameraMatrix * matrix);
	}

	SurfaceRegion region = VoxelDrawSystem::Render();
	flags = ShapeFlags_Type(flags & ~SHAPE_REMAP);
	Blit_Block(*LogicalSurface, *ColorSchemes[Class->Color]->Converter, *VoxelDrawSystem::Get_Surface(), region.Bounds, region.Point + drawpoint, cliprect, NULL, ColorSchemes[Class->Color]->Converter->Blitter_From_Flags(ShapeFlags_Type(flags)), 0, ZGRAD_90DEG, brightness);
}


/// <summary>
/// Fetches the run time type of this object.
/// </summary>
RTTIType BulletClass::Fetch_RTTI(void) const
{
	return(RTTI_BULLET);
}


ClassID BulletClass::Class_ID(void) const
{
	return(ClassID_BulletClass);
}


/// <summary>
/// Assigns a new target for this projectile.
/// This routine is used when the launcher wants the projectile to chase something other than
/// what it was originally fired at. Only homing projectiles will act on the change.
/// </summary>
void BulletClass::Assign_Target(AbstractClass * target)
{
	Sync_Record_Target(*this, target, (unsigned)(uintptr_t)_ReturnAddress());
	TarCom = target;
}


/// <summary>
/// Fetches the type class object for this projectile.
/// </summary>
/// <returns>Returns with a pointer to the bullet type this projectile was created from.</returns>
ObjectTypeClass const * BulletClass::Class_Of(void) const
{
	return((ObjectTypeClass const *)Class);
}
