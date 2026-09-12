/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "always.h"

#include "particle.h"

#include "_alpha.h"
#include "_convert.h"
#include "_map.h"
#include "_rect.h"
#include "_rules.h"
#include "_surface.h"
#include "_tactica.h"
#include "_zbuffer.h"
#include "abuffer.h"
#include "building.h"
#include "cell.h"
#include "convert.h"
#include "draw.h"
#include "dsurface.h"
#include "globals.h"
#include "goptions.h"
#include "house.h"
#include "houstype.h"
#include "inline.h"
#include "mouse.h"
#include "partsys.h"
#include "psystype.h"
#include "ptype.h"
#include "rgb.h"
#include "rules.h"
#include "savestream.h"
#include "scenario.h"
#include "sun.h"
#include "surface.h"
#include "tactical.h"
#include "tracker.h"
#include "unit.h"
#include "wave.h"
#include "zbuffer.h"

#include "draw.hh"

#include <algorithm>


LEPTON ParticleClass::SmokeWindX[FACING_COUNT] = { 0, 2, 2, 1, 0, -2, -2, -2 };
LEPTON ParticleClass::SmokeWindY[FACING_COUNT] = { -2, -2, 0, 2, 2, 2, 0, -2 };
LEPTON ParticleClass::GasWindX[FACING_COUNT] = { 0, 2, 2, 2, 0, -2, -2, -2 };
LEPTON ParticleClass::GasWindY[FACING_COUNT] = { -2, -2, 0, 2, 2, 2, 0, -2 };


/// <summary>
/// Creates a particle traveling from one location toward another.
/// This routine is used by the particle systems to spawn their particles. The
/// particle is aimed at the target, given a lifetime and a starting color drawn from
/// its type, lifted clear of the ground if it was spawned below it, and placed on
/// the map ready for its first logic frame.
/// </summary>
/// <param name="type">The particle type to create.</param>
/// <param name="origin">The location the particle is fired from.</param>
/// <param name="target">The location the particle is aimed at.</param>
/// <param name="partsys">The particle system that owns this particle.</param>
ParticleClass::ParticleClass(ParticleTypeClass const * type, Coord const & origin, Coord const & target, ParticleSystemClass * partsys) :
	Class(type),
	Color(0,0,0),
	ColorAccum(0),
	ColorIndex(0),
	GasDrift(0,0,0),
	GasVelocity(0,0,-1),
	UnusedCoord1(0,0,0),
	Speed(Class->Velocity),
	FireTarget(target),
	FireOrigin(origin),
	FireMoveDelta(COORD_NONE),
	MovementDirection(target.X - origin.X, target.Y - origin.Y, target.Z - origin.Z),
	PrecisePosition(origin.X, origin.Y, origin.Z),
	System(partsys),
	StateAIAdvance(Class->StateAIAdvance),
	IsFireBelowGround(false),
	StateAI(Class->StartStateAI),
	Translucency(Class->Translucency),
	WasSaved(false),
	IsToDie(false)
{
	Create_ID();
	Particles.Add(this);

	if (Class->BehavesLike == BEHAVIOR_SMOKE) {
		Speed += Scen->RandomNumber() % 2;
	}

	RemainingDC = Class->MaxDC;
	if (Class->BehavesLike == BEHAVIOR_RAILGUN) {
		RemainingEC = Class->MaxEC + abs((Scen->RandomNumber()) % 10);
	} else {
		int random = Scen->RandomNumber();
		RemainingEC = Class->MaxEC + abs(random % Class->MaxEC);
	}

	Coord coord = origin;
	if (coord.Z <= Map.Get_Height_GL(origin)) {
		coord.Z = Map.Get_Height_GL(origin);
	}
	PositionCoord = coord;

	float length = MovementDirection.Length();
	Vector3 normalized;
	if (length != 0.0) {
		normalized = MovementDirection / length;
	} else {
		normalized = MovementDirection;
	}
	MovementDirection = normalized;

	if (Class->IsNormalized) {
		Point2D delta(origin.X - target.X, origin.Y - target.Y);

		float abs_vx = abs(MovementDirection.X * Speed);
		float abs_vy = abs(MovementDirection.Y * Speed);

		float min_time = 9999.0f;

		if (abs_vx > 1e-6) {
			min_time = abs(delta.X) / abs_vx;
		}

		if (abs_vy > 1e-6) {
			min_time = std::min(min_time, abs(delta.Y) / abs_vy);
		}

		StateAIAdvance = (int)(min_time / (Class->FinalDamageState + 1) + 1.0);
	}

	if (Class->ColorList.Count() > 0) {
		if (Class->StartColor1 == RGBClass(0,0,0) && Class->StartColor2 == RGBClass(0,0,0)) {
			Color = Class->ColorList[0];
		} else {
			Color = RGBClass().Lerp(Class->StartColor1, Class->StartColor2, Random_Double(0.0,1.0));
		}
	}

	Unlimbo(coord);
}


/// <summary>
/// Creates a blank particle.
/// This is the constructor the save game loader uses. The particle is added to the
/// master list but is otherwise empty until it is read back in.
/// </summary>
ParticleClass::ParticleClass(void) :
	Class(NULL),
	Color(0,0,0),
	ColorAccum(0),
	ColorIndex(0),
	GasDrift(0,0,0),
	GasVelocity(0,0,-1),
	UnusedCoord1(0,0,0),
	Speed(0),
	FireTarget(COORD_NONE),
	FireOrigin(COORD_NONE),
	FireMoveDelta(COORD_NONE),
	MovementDirection(0,0,0),
	PrecisePosition(0,0,0),
	System(NULL),
	StateAIAdvance(0),
	IsFireBelowGround(false),
	StateAI(0),
	Translucency(0),
	WasSaved(false),
	IsToDie(false)
{
	Particles.Add(this);
}


/// <summary>
/// Destroys this particle.
/// The particle is pulled off the map, every object holding a reference to it is
/// told to let go, and it is removed from the master particle list.
/// </summary>
ParticleClass::~ParticleClass(void)
{
	Limbo();
	Detach_This_From_All(this);
	Class = NULL;
	Particles.Delete(this);
}


/// <summary>
/// Deflects a vector along the lie of the ground.
/// This routine is used when a particle glances off the terrain, so that it skims
/// away along the slope of a ramped cell rather than off a flat plane.
/// </summary>
/// <param name="vector">The vector to deflect. It is updated in place.</param>
/// <param name="coord">The location whose ground slope should be used.</param>
/// <param name="length">The length to scale the deflected vector to.</param>
/// <returns>Returns with the deflected vector.</returns>
inline Vector3 Slope_Vector(Vector3 & vector, Coord const & coord, double length)
{
	Matrix3D slope_matrix;
	Matrix3D slope_matrix_inv = Matrix3D::Orthogonal_Inverse(
		slope_matrix = Get_Slope_Matrix(TacticalMap->Get_Cell_Ramp(coord)));
	Vector3 vec(vector.X, -vector.Y, vector.Z);
	vec = slope_matrix_inv.Rotate_Vector(vec);
	vec *= length;
	vec.Z = -vec.Z;
	vec = slope_matrix.Rotate_Vector(vec);
	vector.X = vec.X;
	vector.Y = -vec.Y;
	vector.Z = vec.Z;
	return(vector);
}


/// <summary>
/// Handles the per frame logic for a gas particle.
/// The cloud wanders sideways and sinks under gravity, glancing off the terrain and
/// off any bridge, wall, or building that stands in its way. A gas that carries a
/// warhead also poisons whatever it settles over, and when the scenario allows it,
/// anything killed that way rises again as a visceroid.
/// </summary>
void ParticleClass::Gas_Behavior_AI(void)
{
	if ((Frame & 1) == 0) {
		Coord coord = Get_Coord();
		if ((abs(Scen->RandomNumber()) & 7) == 0) {
			int dx = 0;
			int dy = 0;
			if ((abs(Scen->RandomNumber()) & 1) == 0) {
				dx = (int)abs(Scen->RandomNumber()) % 3 - 1;
			} else {
				dy = (int)abs(Scen->RandomNumber()) % 3 - 1;
			}
			GasDrift += Point2D(dx, dy);
			if (GasDrift.X < -5) {
				GasDrift.X = -5;
			} else if (GasDrift.X > 5) {
				GasDrift.X = 5;
			}
			if (GasDrift.Y < -5) {
				GasDrift.Y = -5;
			} else if (GasDrift.Y > 5) {
				GasDrift.Y = 5;
			}
		}

		Vector3 position(coord.X, coord.Y, coord.Z);
		Vector3 & velocity = GasVelocity;
		velocity = Vector3(0, 0, -2);
		velocity.Z -= Rule->Gravity;

		Coord old_coord(position.X, position.Y, position.Z);
		position += velocity;
		Coord coord2(position.X, position.Y, position.Z);

		int height_gl = Map.Get_Height_GL(coord2);
		int height_bridge = height_gl + BRIDGE_LEPTON_HEIGHT;

		CellClass * cellptr = &Map[coord2];
		bool start_under_bridge = false;
		bool end_under_bridge = false;
		if (cellptr->IsUnderBridge || Map[old_coord].IsUnderBridge) {
			if (coord2.Z >= height_bridge) {
				if (old_coord.Z < height_bridge) {
					end_under_bridge = true;
				}
			} else {
				if (old_coord.Z >= height_bridge) {
					start_under_bridge = true;
				}
			}
		}

		bool blocked = false;
		if (!start_under_bridge && !end_under_bridge) {
			if (position.Z >= (double)height_gl && position.Z - 150.0f < (double)height_gl) {
				BuildingClass * building = cellptr->Cell_Building();
				if (building || cellptr->Has_Wall_Or_Gate()) {
					blocked = true;
					if (building) {
						if (building->Class->IsLaserFence && building->LaserFenceFrame >= 8) {
							blocked = false;
						}
						if (building->Considered_Vehicle()) {
							blocked = false;
						}
					}
				} else {
					blocked = false;
				}
			}
		}

		if (height_gl > position.Z || start_under_bridge || end_under_bridge || blocked) {
			Slope_Vector(velocity, coord2, 1);
		}

		if (HeightAGL > 5 && (Frame % 2) == 0) {
			GasDrift.Z = std::max(-5, GasDrift.Z - 1);
		} else {
			GasDrift.Z = std::max(GasDrift.Z, 0);
		}
	}

	RemainingDC--;

	if (Class->BehavesLike != BEHAVIOR_WEAKGAS && RemainingDC == 0) {
		if (Class->Damage) {
			RemainingDC = Class->MaxDC;
			ObjectClass * occupier = Map[Get_Coord()].Cell_Occupier();
			while (occupier) {
				Coord delta = Get_Coord() - occupier->Get_Coord();
				ObjectClass * next = occupier->Next;
				Coord ocoord = occupier->Get_Coord();
				ResultType result = RESULT_NONE;
				if (occupier->IsActive) {
					if (occupier->Strength > 0) {
						int damage = Class->Damage;
						result = occupier->Take_Damage(damage, TacticalMap->Z_Lepton_To_Pixel(abs(delta.X) + abs(delta.Y)), Class->Warhead, 0, 0, 0);
					}
				}
				occupier = next;

				if (result == RESULT_DESTROYED) {
					if (Scen->IsTiberiumDeathToVisceroid) {
						UnitClass * visc = new UnitClass(Rule->SmallVisceroid, House_From_HousesType(HouseTypeClass::From_Name("Neutral")));
						if (visc) {
							Coord coord = Map[ocoord].Cell_Coord();
							if (!Map[coord].Flag.Occupy.Vehicle) {
								ScenarioInit++;
								visc->Unlimbo(coord, DIR_N);
								ScenarioInit--;
							}
						}
					}
				}
			}
		}
	}
	if (((Class->MaxEC - RemainingEC + Fetch_ID()) % (Class->StateAIAdvance + (Fetch_ID() & 1))) == 0) {
		StateAI++;
	}
	if (StateAI == Class->EndStateAI) {
		if (Class->DeleteOnStateLimit) {
			IsToDie = true;
		} else {
			StateAI = 0;
		}
	}
}


/// <summary>
/// Handles the per frame logic for a railgun particle.
/// The particle rides its firing direction at a jittering speed, which is what gives
/// the railgun trail its coiled look, and blends its color a step further along the
/// type's color list.
/// </summary>
void ParticleClass::Railgun_Behavior_AI(void)
{
	Vector3 direction(MovementDirection.X, MovementDirection.Y, MovementDirection.Z);
	float speed = Speed;
	Vector3 velocity = speed * direction;
	Speed += Random_Double(-0.5, 0.5) * 0.1;
	PrecisePosition += velocity;
	Coord pos(PrecisePosition.X, PrecisePosition.Y, PrecisePosition.Z);
	PositionCoord = pos;
	ColorAccum += Class->ColorSpeed + Random_Double(0.0, 0.05);
	if (ColorAccum > 1.0) {
		if (ColorIndex < Class->ColorList.Count() - 2) {
			ColorIndex++;
			ColorAccum = 0;
		} else {
			ColorAccum = 1.0;
		}
	}
}


/// <summary>
/// Handles the per frame logic for a smoke particle.
/// The puff wanders a little to either side so that a plume does not rise in a
/// straight column, slows as it climbs, and advances its animation state until it
/// reaches the end of the smoke sequence.
/// </summary>
void ParticleClass::Smoke_Behavior_AI(void)
{
	if (Frame & 1) {
		if ((abs(Scen->RandomNumber) & 3) == 0) {
			int x = 0;
			int y = 0;
			if ((abs(Scen->RandomNumber) & 1) == 0) {
				x = (int)abs(Scen->RandomNumber) % 3 - 1;
			} else {
				y = (int)abs(Scen->RandomNumber) % 3 - 1;
			}
			GasDrift += Point2D(x,y);
			if (GasDrift.X < -5) {
				GasDrift.X = -5;
			} else if (GasDrift.X > 5) {
				GasDrift.X = 5;
			}
			if (GasDrift.Y < -5) {
				GasDrift.Y = -5;
			} else if (GasDrift.Y > 5) {
				GasDrift.Y = 5;
			}
		}
		GasDrift.Z = 0;
	}

	if (StateAI < Class->EndStateAI) {
		if (((Class->MaxEC - RemainingEC + Fetch_ID()) % (Class->StateAIAdvance + (Fetch_ID() & 1))) == 0) {
			StateAI++;
		}
		if (StateAI == Class->EndStateAI && Class->DeleteOnStateLimit) {
			IsToDie = true;
		}
	}
	if (Speed > 3.0) {
		Speed = Speed - Class->Deacc;
	}
}


/// <summary>
/// Handles the per frame logic for a spark particle.
/// The spark falls under gravity until it meets the ground, a bridge deck, or a
/// wall, at which point it glances off along the slope and is flagged for death.
/// Its color is blended a step further along the type's color list each frame.
/// </summary>
void ParticleClass::Spark_Behavior_AI(void)
{
	Coord coord = PositionCoord;
	MovementDirection.Z -= Rule->Gravity;
	Vector3 velocity(MovementDirection.X, MovementDirection.Y, MovementDirection.Z);
	Vector3 position(coord.X, coord.Y, coord.Z);
	velocity.Z -= Rule->Gravity;

	Coord old_coord(position.X, position.Y, position.Z);
	position += velocity;
	Coord coord2(position.X, position.Y, position.Z);

	int height_gl = Map.Get_Height_GL(coord2);
	int height_bridge = height_gl + BRIDGE_LEPTON_HEIGHT;

	CellClass * cellptr = &Map[coord2];
	bool start_under_bridge = false;
	bool end_under_bridge = false;
	if (cellptr->IsUnderBridge || Map[old_coord].IsUnderBridge) {
		if (coord2.Z >= height_bridge) {
			if (old_coord.Z < height_bridge) {
				end_under_bridge = true;
			}
		} else {
			if (old_coord.Z >= height_bridge) {
				start_under_bridge = true;
			}
		}
	}

	bool blocked = false;
	if (!start_under_bridge && !end_under_bridge) {
		if (position.Z >= (double)height_gl && position.Z - 150.0f < (double)height_gl) {
			BuildingClass * building = cellptr->Cell_Building();
			if (building || cellptr->Has_Wall_Or_Gate()) {
				blocked = true;
				if (building) {
					if (building->Class->IsLaserFence && building->LaserFenceFrame >= 8) {
						blocked = false;
					}
					if (building->Considered_Vehicle()) {
						blocked = false;
					}
				}
			} else {
				blocked = false;
			}
		}
	}

	bool sloped = true;
	if (position.Z >= (float)height_gl && !start_under_bridge && !end_under_bridge && !blocked) {
		sloped = false;
	} else if (start_under_bridge) {
		position.Z = height_bridge;
	} else if (end_under_bridge) {
		position.Z = height_bridge - 20;
	} else if ((float)(height_gl - 100) < position.Z) {
		position.Z = (float)height_gl;
	}

	if (sloped) {
		Slope_Vector(velocity, coord2, 1);
		IsToDie = true;
	}

	Point3D new_position(position.X, position.Y, position.Z);
	Set_Coord((Coord const &)new_position);
	ColorAccum += Class->ColorSpeed + Random_Double(0.0, 0.05);
	if (ColorAccum > 1.0) {
		if (ColorIndex < Class->ColorList.Count() - 2) {
			ColorIndex++;
			ColorAccum = 0;
		} else {
			ColorAccum = 1.0;
		}
	}
}


/// <summary>
/// Handles the per frame logic for a flame particle.
/// The flame works out how far it will travel this frame, fades toward translucency
/// as its animation advances, and scorches whatever shares its cell -- everything
/// except the object that spawned it. A flame that has slowed to a halt is flagged
/// for death.
/// </summary>
void ParticleClass::Fire_Behavior_AI(void)
{
	if (Speed > 0.0) {
		double rndval = (double)(((Scen->RandomNumber % 10) - 5) / 50.0) + 1.0;

		Vector3 motion = rndval * MovementDirection;
		FireMoveDelta = Coord(motion.X * Speed, motion.Y * Speed, motion.Z * Speed);
		if (StateAI < Class->EndStateAI) {
			if (((Class->MaxEC - RemainingEC + Fetch_ID()) % (StateAIAdvance + (Fetch_ID() & 1))) == 0) {
				StateAI++;
				if (StateAI == Class->Translucent25State) {
					Translucency = 25;
				}
				if (StateAI == Class->Translucent50State) {
					Translucency = 50;
				}
			}
			if (StateAI == Class->EndStateAI) {
				if (Class->DeleteOnStateLimit) {
					IsToDie = true;
					return;
				}
			}
		}
		Speed -= Class->Deacc;
		RemainingDC--;
		if (RemainingDC == 0 && Class->Damage && StateAI <= (char)Class->FinalDamageState) {
			RemainingDC = Class->MaxDC;
			CellClass *cellptr = &Map[(Coord const &)PositionCoord];
			bool onbridge = cellptr->IsUnderBridge && PositionCoord.Z >= LEVEL_LEPTON_H * (cellptr->Height + BRIDGE_CELL_HEIGHT);
			ObjectClass *optr = cellptr->Cell_Occupier(onbridge);
			while (optr != NULL) {
				optr->PositionCoord;
				PositionCoord;
				int dist = Distance(optr->PositionCoord);
				const ParticleTypeClass *ptype = Class;
				int damage = ptype->Damage;
				if ( optr->Strength > 0 && optr->IsActive && optr != System->Source_Object() )
				{
					optr->Take_Damage(damage, dist / 10, ptype->Warhead, 0, 0, 0);
				}
				optr = optr->Next;
			}
		}
	} else {
		Speed = 0.0;
		IsToDie = true;
	}
}


/// <summary>
/// Handles the per frame logic for a web particle.
/// The web applies its warhead to everything sharing its cell, which is how the
/// tangling effect is delivered, and then advances its animation state.
/// </summary>
void ParticleClass::Web_Behavior_AI(void)
{
	CellClass * cptr = &Map[Get_Coord()];

	if (Class->Warhead != NULL) {
		ObjectClass * occupier = cptr->Cell_Occupier();
		while (occupier) {
			ObjectClass * next = occupier->Next;
			int damage = 0;
			occupier->Take_Damage(damage, 0, Class->Warhead);
			occupier = next;
		}
	}

	if (((Class->MaxEC - RemainingEC + Fetch_ID()) % int(Class->StateAIAdvance + ((unsigned)Fetch_ID() % 2))) == 0) {
		StateAI++;
	}

	if (StateAI == Class->EndStateAI) {
		if (Class->DeleteOnStateLimit) {
			IsToDie = true;
		} else {
			StateAI = 0;
		}
	}
}


/// <summary>
/// Handles the per frame logic for this particle.
/// This routine dispatches to the handler that suits the particle's behavior and
/// then ages the particle, flagging it for death once its lifetime runs out.
/// </summary>
void ParticleClass::Behavior_AI(void)
{
	switch (Class->BehavesLike) {
		case BEHAVIOR_GAS:
		case BEHAVIOR_WEAKGAS:
			Gas_Behavior_AI();
			break;
		case BEHAVIOR_SMOKE:
			Smoke_Behavior_AI();
			break;
		case BEHAVIOR_FIRE:
			Fire_Behavior_AI();
			break;
		case BEHAVIOR_SPARK:
			Spark_Behavior_AI();
			break;
		case BEHAVIOR_RAILGUN:
			Railgun_Behavior_AI();
			break;
		case BEHAVIOR_WEB:
			Web_Behavior_AI();
			break;
		default:
			break;
	}

	RemainingEC--;
	if (RemainingEC == 0) {
		IsToDie = true;
	}
}


/// <summary>
/// Draws this particle to the logical surface.
/// Most particles are drawn as a translucent shape from their artwork. Sparks and
/// railgun traces have no artwork -- they are plotted as a single lit pixel whose
/// color is blended along the particle's color list and tested against the alpha
/// and depth buffers. Particles hidden by the fog of war are skipped, and the low
/// detail setting drops smoke and sparks entirely.
/// </summary>
/// <param name="point">The screen position to draw the particle at.</param>
/// <param name="cliprect">The clipping rectangle to draw within.</param>
void ParticleClass::Draw_It(Point2D const & point, Rect const & cliprect) const
{
	if (Options.DetailLevel != 0 || Class->BehavesLike != BEHAVIOR_SMOKE && Class->BehavesLike != BEHAVIOR_SPARK) {
		if (Debug_Map || MainWindow == 0 || !Scen->Special.IsFogOfWar || !Map.Is_Fogged((Coord const &)PositionCoord)) {

			if (Class->BehavesLike != BEHAVIOR_SPARK && Class->BehavesLike != BEHAVIOR_RAILGUN) {
				int height_offset = -15 - TacticalMap->Z_Lepton_To_Pixel(Height);
				ShapeSet const * shape = (ShapeSet const *)Get_Image_Data();
				if (shape != NULL) {
					ShapeFlags_Type flags = ShapeFlags_Type(SHAPE_ALPHA|SHAPE_ZGRAD);
					int shapenum = Shape_Number();
					if (Options.DetailLevel == 2) {
						if (Translucency == 25) {
							flags = ShapeFlags_Type(SHAPE_TRANSLUCENT25|SHAPE_ALPHA|SHAPE_ZGRAD);
						} else if (Translucency == 50) {
							flags = ShapeFlags_Type(SHAPE_TRANSLUCENT50|SHAPE_ALPHA|SHAPE_ZGRAD);
						} else if (Translucency >= 75) {
							flags = ShapeFlags_Type(SHAPE_TRANSLUCENT75|SHAPE_ALPHA|SHAPE_ZGRAD);
						}
					}
					Draw_Shape(*LogicalSurface, *AnimDrawer, shape, shapenum, point, cliprect, (ShapeFlags_Type)(flags|SHAPE_CENTER|SHAPE_WIN_REL|SHAPE_ALPHA), 0, height_offset, ZGRAD_90DEG);
				}
			} else {
				Point2D pixel;
				TacticalMap->Coord_To_Pixel(PositionCoord, pixel);
				pixel.Y += TacticalRect.Y;
				if (cliprect.Is_Point_Within(pixel)) {
					Point2D alpha_point = pixel - Point2D(0, AlphaBuffer->Get_Bounds().Y);
					int alpha = *AlphaBuffer->Get_Buffer_Offset(alpha_point);
					if (alpha != 0) {
						Point2D depth_point = pixel - Point2D(0, DepthBuffer->Get_Bounds().Y);
						int zdepth = (unsigned short)(DepthBuffer->Get_Bounds().Y + DepthBuffer->Get_Scroll_Delta(pixel.Y)) - TacticalMap->Z_Lepton_To_Pixel(PositionCoord.Z) - 50;
						int depth = *DepthBuffer->Get_Buffer_Offset(depth_point);
						if (zdepth < depth) {
							RGBClass color1 = ColorIndex == 0 ? Color : Class->ColorList[ColorIndex];
							RGBClass color2 = Class->ColorList[ColorIndex + 1];
							int red = color1.Get_Red() * (1.0 - ColorAccum) + color2.Get_Red() * ColorAccum;
							int green = color1.Get_Green() * (1.0 - ColorAccum) + color2.Get_Green() * ColorAccum;
							int blue = color1.Get_Blue() * (1.0 - ColorAccum) + color2.Get_Blue() * ColorAccum;
							if (alpha < 127) {
								red = (int)(red * alpha) >> 7;
								green = (int)(green * alpha) >> 7;
								blue = (int)(blue * alpha) >> 7;
							}
							LogicalSurface->Put_Pixel(pixel, (unsigned short)DSurface::Build_Hicolor_Pixel(red, green, blue));
						}
					}
				}

			}

		}
	}
}


/// <summary>
/// Handles drifting a smoke particle for this game frame.
/// The prevailing wind carries the smoke sideways while it settles toward the
/// ground, and it is never allowed to sink into the terrain.
/// </summary>
void ParticleClass::Smoke_Motion_AI(void)
{
	Coord coord = Get_Coord();

	if (Class->WindEffect > 0 && (Frame % (10 / Class->WindEffect)) == 0) {
		coord += Coord(SmokeWindX[Rule->WindDirection], SmokeWindY[Rule->WindDirection]);
	}

	if ((unsigned)Frame % 2 != 0) {

		coord.X += GasVelocity.X;
		coord.Y += GasVelocity.Y;

		int height = Map.Get_Height_GL(coord);
		if (coord.Z > height + 5) {
			int descend = std::min(coord.Z - (height + 5), 2);
			coord.Z -= descend;
		}

		coord += GasDrift;

		if (coord.Z < Map.Get_Height_GL(coord) + 5) {
			coord.Z = Map.Get_Height_GL(coord) + 5;
		}
	}

	Set_Coord(coord);
}


/// <summary>
/// Handles drifting a gas particle for this game frame.
/// The cloud rises at its own speed while the prevailing wind and its accumulated
/// drift push it sideways. A cloud that would rise up through a bridge deck is
/// flagged for death instead of passing through it.
/// </summary>
void ParticleClass::Gas_Motion_AI(void)
{
	static const double _level_scale = 2.5;

	Coord coord = PositionCoord;
	Coord previous_coord = coord;

	int wind_effect = Class->WindEffect;

	Point2D wind;
	wind.X = GasWindX[Rule->WindDirection] * wind_effect;
	wind.Y = GasWindY[Rule->WindDirection] * wind_effect;

	coord.X += wind.X;
	coord.Y += wind.Y;
	coord.Z += (int)Speed;
	coord += GasDrift;

	if (Map[previous_coord].IsUnderBridge) {
		int height = Map.Get_Height_GL(previous_coord) + BRIDGE_LEPTON_HEIGHT;
		if (previous_coord.Z < height && coord.Z >= height - LEVEL_LEPTON_H * _level_scale) {
			IsToDie = true;
			return;
		}
	}

	PositionCoord = coord;
}


/// <summary>
/// Handles moving a flame particle along its firing path.
/// The flame coasts along the travel delta its behavior logic worked out. Should it
/// run into rising ground it is flagged for death, since a flame cannot burn its way
/// into a hillside.
/// </summary>
void ParticleClass::Fire_Motion_AI(void)
{
	Coord coord = PositionCoord;
	Coord previous_coord = coord;
	if (Speed > 0.0) {
		coord += FireMoveDelta;
	}

	if (Speed > 0.0) {
		int height = Map.Get_Height_GL(previous_coord);
		if (Map.Get_Height_GL(coord) > height) {
			IsFireBelowGround = true;
			IsToDie = true;
		}
	}

	PositionCoord = coord;
}


/// <summary>
/// Handles moving the particle for this game frame.
/// This routine dispatches to the movement handler that suits the particle's
/// behavior. Behaviors with no movement of their own, such as sparks and railgun
/// traces, are left where their behavior logic put them.
/// </summary>
void ParticleClass::Motion_AI(void)
{
	switch (Class->BehavesLike)
	{
		case PSYS_BEHAVIOR_GAS:
			Gas_Motion_AI();
			break;
		case PSYS_BEHAVIOR_SMOKE:
			Smoke_Motion_AI();
			break;
		case PSYS_BEHAVIOR_WEAKGAS:
			Smoke_Motion_AI();
			break;
		case PSYS_BEHAVIOR_FIRE:
			Fire_Motion_AI();
			break;
		default:
			break;
	}
}


/// <summary>
/// Handles adding or removing this particle from the map.
/// </summary>
/// <param name="mark">The marking operation to perform.</param>
/// <returns>bool; Was the particle's presence on the map changed?</returns>
bool ParticleClass::Mark(MarkType mark)
{
	if (BASECLASS::Mark(mark)) {
		return(true);
	}
	return(false);
}


/// <summary>
/// Fetches the list of cells this particle occupies.
/// A particle never claims any ground, so the list it hands back is empty.
/// </summary>
/// <returns>Returns with a pointer to an empty occupation list.</returns>
Cell const * ParticleClass::Occupy_List(bool placement) const
{
	static Cell const _list[] = {REFRESH_EOL};
	return(_list);
}


/// <summary>
/// Determines which render layer this particle belongs to.
/// Particles always float above the ground clutter, so they are drawn with the
/// airborne objects.
/// </summary>
/// <returns>Returns with LAYER_AIR.</returns>
LayerType ParticleClass::In_Which_Layer(void) const
{
	return(LAYER_AIR);
}


/// <summary>
/// Determines the approximate length of a two dimensional offset.
/// Use this routine where a rough magnitude will do and the cost of a square root
/// is not worth paying.
/// </summary>
/// <param name="pt">The offset to measure.</param>
/// <returns>Returns with the approximate length of the offset.</returns>
int Approximate_Distance(Point2D & pt)
{
	return((int)(abs(pt.X) + abs(pt.Y)) >> 1);
}


/// <summary>
/// Lists the members this particle carries.
/// </summary>
/// <param name="stream">The stream carrying the members.</param>
void ParticleClass::Serialize(SaveStreamClass & stream)
{
	BASECLASS::Serialize(stream);

	stream.Serialize(Class);
	stream.Serialize(Color);
	stream.Serialize(ColorIndex);
	stream.Serialize(ColorAccum);
	stream.Serialize(GasDrift);
	stream.Serialize(GasVelocity);
	stream.Serialize(UnusedCoord1);
	stream.Serialize(Speed);
	stream.Serialize(FireTarget);
	stream.Serialize(FireOrigin);
	stream.Serialize(FireMoveDelta);
	stream.Serialize(MovementDirection);
	stream.Serialize(PrecisePosition);
	stream.Serialize(System);
	stream.Serialize(RemainingEC);
	stream.Serialize(RemainingDC);
	stream.Serialize(StateAIAdvance);
	stream.Serialize(IsFireBelowGround);
	stream.Serialize(StateAI);
	stream.Serialize(Translucency);
	stream.Serialize(WasSaved);
	stream.Serialize(IsToDie);
}


/// <summary>
/// Writes this particle out to the save game stream.
/// </summary>
/// <param name="stream">The stream to write this particle to.</param>
/// <param name="cleardirty">Should the modified flag be cleared once written?</param>
/// <returns>bool; Was the record written whole?</returns>
bool ParticleClass::Save(SaveStreamClass & stream, bool cleardirty)
{
	bool result = BASECLASS::Save(stream, cleardirty);
	WasSaved = true;
	return(result);
}


/// <summary>
/// Fetches the shape frame to draw this particle with.
/// The frame follows the particle's animation state. Fire particles also pick an
/// artwork bank according to the direction they were fired in, so that the flame
/// leans the right way on screen.
/// </summary>
/// <returns>Returns with the shape frame number, or zero if the particle has no artwork.</returns>
int ParticleClass::Shape_Number(void) const
{
	Point2D fire_start;
	Point2D fire_end;
	FacingType face;

	switch (Class->BehavesLike) {
		case BEHAVIOR_GAS:
			return(StateAI);

		case BEHAVIOR_SMOKE:
			return(StateAI);

		case BEHAVIOR_FIRE:
			TacticalMap->Coord_To_Pixel(FireOrigin, fire_start);
			TacticalMap->Coord_To_Pixel(FireTarget, fire_end);
			face = Facing_Between_Points(fire_start, fire_end);
			if (face == FACING_E || face == FACING_W) {
				return(StateAI + 2 * Class->EndStateAI);
			}
			if (face == FACING_S || face == FACING_N) {
				return(StateAI);
			}
			if (face == FACING_NE || face == FACING_SW) {
				return(StateAI + Class->EndStateAI);
			}
			if (face == FACING_SE || face == FACING_NW) {
				return(StateAI + 3 * Class->EndStateAI);
			}
			break;

		case BEHAVIOR_SPARK:
			break;

		case BEHAVIOR_RAILGUN:
			break;

		case BEHAVIOR_WEB:
			return(StateAI);

		case BEHAVIOR_WEAKGAS:
			return(StateAI);
	}

	return(0);
}


ClassID ParticleClass::Class_ID(void) const
{
	return(ClassID_ParticleClass);
}


/// <summary>
/// Fetches the run time type identifier for this object.
/// </summary>
/// <returns>Returns with RTTI_PARTICLE.</returns>
RTTIType ParticleClass::Fetch_RTTI(void) const
{
	return(RTTI_PARTICLE);
}


/// <summary>
/// Fetches the type class object for this particle.
/// </summary>
/// <returns>Returns with a pointer to the particle type this particle was created from.</returns>
ObjectTypeClass const * ParticleClass::Class_Of(void) const
{
	return(Class);
}
