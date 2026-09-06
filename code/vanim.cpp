/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#define INCLUDE_COM
#include "always.h"

#include "vanim.h"

#include "_convert.h"
#include "_rect.h"
#include "_rules.h"
#include "_surface.h"
#include "_tactica.h"
#include "_vanim.h"
#include "anim.h"
#include "cell.h"
#include "combat.h"
#include "crc.h"
#include "draw.h"
#include "house.h"
#include "inline.h"
#include "lightcon.h"
#include "overlay.h"
#include "overtype.h"
#include "partsys.h"
#include "rect.h"
#include "rules.h"
#include "savestream.h"
#include "scheme.h"
#include "sun.h"
#include "tactical.h"
#include "techno.h"
#include "tiberium.h"
#include "tracker.h"
#include "vanimtype.h"
#include "voc.h"
#include "voxdrsys.h"

/// <summary>
/// Creates a voxel animation at the coordinate specified.
/// This routine sets the animation tumbling with a randomized velocity and spin drawn
/// from the limits its type declares, plays the start sound, and attaches the type's
/// particle system to it. A meteor is instead placed back along its flight path so that
/// it appears to come in from off in the distance.
/// </summary>
/// <param name="type">The voxel animation type to create an instance of.</param>
/// <param name="coord">The coordinate to create the animation at.</param>
/// <param name="house">The house that owns the animation, or NULL if it belongs to nobody.</param>
VoxelAnimClass::VoxelAnimClass(VoxelAnimTypeClass const * type, Coord const & coord, HouseClass * house) :
	BASECLASS(),
	Class((VoxelAnimTypeClass *)type),
	Unused1(-1),
	AttachedParticleSys(NULL),
	House(house),
	IsToDie(false),
	IsInvisible(false),
	ECCounter(0)
{
	Create_ID();
	ECCounter = Class->Duration;
	VoxelAnims.Add(this);

	if (!Class->IsMeteor) {
		Unlimbo(coord);
		Coord center = Center_Coord() + Coord(0, 0, 10);
		int r1 = Scen->RandomNumber;
		int r2 = Scen->RandomNumber;
		int r3 = Scen->RandomNumber;
		int r4 = Scen->RandomNumber;
		float y = (abs(r3) % int(Class->MaxXYVel + Class->MaxXYVel)) - Class->MaxXYVel;
		float z = (abs(r2) % int(Class->MaxZVel - Class->MinZVel + 1.0)) + Class->MinZVel;
		float x = (abs(r4) % int(Class->MaxXYVel + Class->MaxXYVel)) - Class->MaxXYVel;

		Vector3 velocity(x, y, z);
		double rotation = (abs(r1) % int(Class->MaxAngularVelocity - Class->MinAngularVelocity + 1.0)) + Class->MinAngularVelocity;
		BounceClass::Init(center, Class->Elasticity, 1.4f, 0.0, velocity, rotation);

	} else {
		Vector3 velocity((abs(Scen->RandomNumber) % int(Class->MaxXYVel + Class->MaxXYVel)) - Class->MaxXYVel,
						 (abs(Scen->RandomNumber) % int(Class->MaxXYVel + Class->MaxXYVel)) - Class->MaxXYVel, Class->MinZVel);

		if (velocity.X < -velocity.Y) {
			velocity.X = -velocity.X;
			velocity.Y = -velocity.Y;
		}

		ECCounter -= abs(Scen->RandomNumber % 20);
		int x = coord.X - ECCounter * velocity.X;
		int y = coord.Y - ECCounter * velocity.Y;
		int z = coord.Z - ECCounter * velocity.Z;
		Coord ucoord(x, y, z);
		Unlimbo(ucoord);
		int rnd = Scen->RandomNumber;
		BounceClass::Init(Center_Coord(), Class->Elasticity, 1.4f, 0.0, velocity, (abs(rnd) % int(Class->MaxAngularVelocity - Class->MinAngularVelocity + 1.0)) + Class->MinAngularVelocity);
	}

	if (Class->StartSound != VOC_NONE) {
		Sound_Effect(Class->StartSound, coord);
	}

	if (Class->AttachedSystem) {
		AttachedParticleSys = new ParticleSystemClass(Class->AttachedSystem, coord, &Map[coord], this);
	}
}


/// <summary>
/// Creates a blank voxel animation.
/// This routine is used by the save/load system, which needs an empty object to read
/// the saved state into.
/// </summary>
VoxelAnimClass::VoxelAnimClass(void) :
	BASECLASS(),
	Class(NULL),
	Unused1(-1),
	AttachedParticleSys(NULL),
	House(NULL),
	IsToDie(false),
	IsInvisible(false),
	ECCounter(0)
{
	Create_ID();
	VoxelAnims.Add(this);
}


/// <summary>
/// Destroys the voxel animation.
/// This routine detaches the animation from everything that refers to it, disposes of
/// any particle system riding along with it, and takes it out of the game object lists.
/// </summary>
VoxelAnimClass::~VoxelAnimClass(void)
{
	Detach_This_From_All(this);
	VoxelAnims.Delete(this);
	if (AttachedParticleSys != NULL) {
		AttachedParticleSys->Delete_Me();
		AttachedParticleSys = NULL;
	}
	if (GameActive) {
		Limbo();
	}
	AbstractTypePtrTracker.Delete(this);
	Class = NULL;
}


/// <summary>
/// Submits this animation to the display for rendering.
/// This routine marks the animation as displayable whenever the cell it occupies is
/// currently visible, then lets the base class queue the actual redraw.
/// </summary>
/// <param name="cliprect">The clipping rectangle to render within.</param>
/// <param name="forced">Is this redraw forced by outside circumstances?</param>
/// <returns>bool; Was the animation queued for rendering?</returns>
bool VoxelAnimClass::Render(Rect &cliprect, bool forced, bool extras_only) const
{
	if (Map[Center_Coord()].IsVisible) {
		IsToDisplay = true;
	}
	return(BASECLASS::Render(cliprect, forced, false));
}


/// <summary>
/// Draws this voxel animation to the tactical map.
/// This routine hands the voxel to the voxel draw system -- first its shadow, then the
/// lit object itself, tinted by the owning house's color scheme or by the tiberium
/// drawer as appropriate. An animation hidden by fog of war is not drawn at all.
/// </summary>
/// <param name="xpoint">The pixel position to draw the animation at.</param>
/// <param name="cliprect">The clipping rectangle to draw within.</param>
void VoxelAnimClass::Draw_It(Point2D const & xpoint, Rect const & cliprect) const
{
	if (!Scen->Special.IsFogOfWar || !Map.Is_Fogged(Coord(Position))) {
		Point2D point = xpoint;
		Coord pos = Position;
		pos.Z = Map.Get_Height_GL(pos);

		Point2D pixel;
		int z;

		/*
		 * If the animation is hovering above a bridge, then draw it relative
		 * to the bridge deck rather than the ground beneath the bridge.
		 */
		{
			Coord coord;
			if (Map[pos].IsUnderBridge && (coord = Position).Z >= pos.Z + BRIDGE_LEPTON_HEIGHT) {
				z = pos.Z + BRIDGE_LEPTON_HEIGHT;
				coord = pos + Coord(0, 0, BRIDGE_LEPTON_HEIGHT + 1);
				TacticalMap->Coord_To_Pixel(coord, pixel);
			} else {
				z = pos.Z;
				TacticalMap->Coord_To_Pixel(pos, pixel);
			}
		}

		if (!IsInvisible) {
			VoxelLibrary * voxlib = Class->Voxel.VoxLib;
			if (voxlib != NULL) {

				Matrix3D matrix;
				Rect fromrect;
				int x, y;

				matrix = BounceClass::Get_Matrix();
				VoxelDrawSystem::Reset();
				VoxelDrawSystem::Prep_For_Shadow(voxlib, Class->VoxelIndex, 0, VoxelCameraMatrix, matrix, VoxelShadowLightVector);

				VoxelDrawSystem::Render(fromrect, x, y);

				{
					Point2D drawpoint;
					drawpoint.X = pixel.X + x;
					drawpoint.Y = y + pixel.Y;

					Blit_Block(*LogicalSurface, *VoxelDrawer, *VoxelDrawSystem::Get_Surface(), fromrect, drawpoint, cliprect, 0, VoxelDrawer->Blitter_From_Flags(ShapeFlags_Type(SHAPE_DARKEN|SHAPE_ZGRAD)), -4 - TacticalMap->Z_Lepton_To_Pixel(z));
				}

				matrix = BounceClass::Get_Matrix();
				VoxelDrawSystem::Precalculate_Light(voxlib, 0, 0, matrix, VoxelLightSource);
				VoxelDrawSystem::Reset();
				VoxelDrawSystem::Prep_For_Object(voxlib, Class->VoxelIndex, 0, VoxelCameraMatrix * matrix);

				VoxelDrawSystem::Render(fromrect, x, y);

				ShapeFlags_Type flags = ShapeFlags_Type(SHAPE_ALPHA|SHAPE_ZGRAD);
				if (Class->IsTranslucent) {
					flags = ShapeFlags_Type(SHAPE_TRANSLUCENT50|SHAPE_ALPHA|SHAPE_ZGRAD);
				}

				int brightness = Map[Coord(Position)].Brightness;

				ConvertClass * drawer;
				if (House != NULL) {
					drawer = ColorSchemes[House->Scheme]->Converter;
				} else if (Class->IsTiberium) {
					drawer = TiberiumDrawer;
				} else {
					drawer = VoxelDrawer;
					brightness = NORMAL_LIGHT;
				}

				{
					Point2D drawpoint;
					drawpoint.X = x + point.X;
					drawpoint.Y = y + point.Y;

					Blit_Block(*LogicalSurface, *drawer, *VoxelDrawSystem::Get_Surface(), fromrect, drawpoint, cliprect, 0, drawer->Blitter_From_Flags(flags), -2 - TacticalMap->Z_Lepton_To_Pixel(Height), ZGRAD_GROUND, brightness);
				}
			}
		}
	}
}


/// <summary>
/// Removes every voxel animation from the game.
/// This routine is used when the scenario is being torn down so that no animations
/// survive into the next one.
/// </summary>
void VoxelAnimClass::Init_Clear(void)
{
	while (VoxelAnims.Count()) {
		delete VoxelAnims[0];
	}
}


/// <summary>
/// Handles the per frame logic for this voxel animation.
/// This routine ages the animation, emits its trailer effect, and hands the object over
/// to the bounce physics. When the animation strikes something it plays its bounce
/// effects and damages whatever it landed on; when its life runs out it performs the
/// impact -- expiration animation, blast damage, splash, crater, and any tiberium it was
/// carrying -- and then removes itself.
/// </summary>
/// <remarks>Only call this routine once per animation per game logic loop.</remarks>
void VoxelAnimClass::AI(void)
{
	/*
	 * An anim flagged for death is removed immediately.
	 */
	if (IsToDie) {
		Delete_Me();
		return;
	}

	/*
	 * Count down the lifetime timer.
	 */
	if (ECCounter) {
		ECCounter--;
	}

	/*
	 * When the timer expires, perform the impact effects and remove the anim.
	 */
	if (ECCounter <= 0) {
		Coord coord = Position;
		bool is_water = (Map[coord].Land_Type() == LAND_WATER);
		coord = Position;
		bool is_above_bridge = (Position.Z >= Map.Get_Height_GL(coord) + BRIDGE_LEPTON_HEIGHT);

		/*
		 * On solid ground (or on top of a bridge), play the expiration animation,
		 * deal the blast damage, and play the expiration sound.
		 */
		if (!is_water || is_above_bridge) {
			if (Class->ExpireAnim != NULL) {
				new AnimClass(Class->ExpireAnim, Coord(MyCoord.X, MyCoord.Y, MyCoord.Z), 0, 1, ShapeFlags_Type(SHAPE_ZGRAD|SHAPE_WIN_REL|SHAPE_CENTER), -30);
				Explosion_Damage(Get_Bounce_Coord(), Class->Damage, 0, Class->Warhead, 1);
				Combat_Lighting(Get_Bounce_Coord(), Class->Damage, Class->Warhead, 0);
			}
			if (Class->ExpireSound != VOC_NONE) {
				Sound_Effect(Class->ExpireSound, Coord(Position.X, Position.Y, Position.Z));
			}

		/*
		 * Splashing into water: a meteor makes the biggest splash, anything else
		 * makes a wake plus a small splash.
		 */
		} else if (Class->IsMeteor) {
			new AnimClass(*(&Rule->SplashList[0] + Rule->SplashList.Count() - 1), Coord(Position) + Coord(0, 0, 5), 0, 1, ShapeFlags_Type(SHAPE_WIN_REL|SHAPE_CENTER), 0);
		} else {
			new AnimClass(Rule->Wake, Coord(Position), 0, 1, ShapeFlags_Type(SHAPE_WIN_REL|SHAPE_CENTER), 0);
			new AnimClass(Rule->SplashList[0], Position + Coord(0, 0, 10), 0, 1, ShapeFlags_Type(SHAPE_WIN_REL|SHAPE_CENTER), 0);
		}

		if (!is_water || is_above_bridge) {

			/*
			 * Running bounding rectangle of the cells modified below.
			 */
			Rect dirty(0, 0, 0, 0);

			if (Class->IsMeteor) {
				Coord impact = Get_Bounce_Coord();

				/*
				 * A meteor may spawn a random number of child voxel anims on impact.
				 */
				if (Class->Spawns != NULL) {
					int maxspawn = Class->SpawnCount;
					if (maxspawn > 0) {
						int count = Scen->RandomNumber(0, maxspawn) + Scen->RandomNumber(0, maxspawn);
						while (count > 0) {
							new VoxelAnimClass(Class->Spawns, impact, NULL);
							count--;
						}
					}
				}

				/*
				 * Gouge a crater into the terrain around the impact cell.
				 */
				if (Rule->CraterLevel) {
					if (!is_above_bridge) {
						Map.Deform_Terrain(impact.As_Cell(), false);
						if (Rule->CraterLevel > 1) {
							for (int i = 0; i < FACING_COUNT; i++) {
								if (i % (FACING_COUNT / 4) || Rule->CraterLevel > 2) {
									Map.Deform_Terrain(Adjacent_Cell(impact.As_Cell(), (FacingType)i), false);
								}
							}
							if (Rule->CraterLevel > 3) {
								Map.Deform_Terrain(impact.As_Cell(), false);
							}
						}
					}
				}

				/*
				 * A tiberium meteor seeds tiberium into the ring of cells around the
				 * impact, accumulating each affected cell into the dirty rectangle.
				 */
				if (Class->IsTiberium && !is_above_bridge) {
					int i = FACING_FIRST;
					do {
						Cell cell = Adjacent_Cell(impact.As_Cell(), (FacingType)i);
						CellClass * cellptr = &Map[cell];
						if (cellptr->Can_Tiberium_Germinate(NULL)) {
							TiberiumClass * tiberium = Tiberiums[Which_Tiberium_Type(OVERLAY_TIBERIUM2_01)];
							if (cellptr->Ramp) {
								new OverlayClass(Scen->RandomNumber(0, 1) + *(&OverlayTypes[2 * cellptr->Ramp - 2] + tiberium->Variety + tiberium->Overlay->HeapID), cellptr->CellID, HOUSE_NONE);
							} else {
								new OverlayClass(*(&OverlayTypes[Scen->RandomNumber(0, 11)] + tiberium->Overlay->HeapID), cellptr->CellID, HOUSE_NONE);
							}
							tiberium->Queue_Growth(cellptr->CellID);
							cellptr->OverlayData = 0;

							/*
							 * Grow the running dirty rectangle to enclose this cell.
							 */
							Rect render = cellptr->Overlay_Render_Rect();
							render.Y -= TacticalRect.Y;
							dirty = Union(dirty, render);
							Map.Radar_Background(cellptr->CellID);
						}
						i++;
					} while (i < FACING_COUNT);

					TacticalMap->Register_Dirty_Area(dirty, false);
					Delete_Me();
					return;
				}

			/*
			 * A non-meteor tiberium anim seeds a single cell of tiberium.
			 */
			} else if (Class->IsTiberium && !is_above_bridge) {
				CellClass * cellptr = &Map[Get_Bounce_Coord()];
				if (cellptr->Can_Tiberium_Germinate(NULL)) {
					TiberiumClass * tiberium = Tiberiums[Which_Tiberium_Type(OVERLAY_TIBERIUM2_01)];
					if (cellptr->Ramp) {
						new OverlayClass(Scen->RandomNumber(0, 1) + *(&OverlayTypes[2 * cellptr->Ramp - 2] + tiberium->Variety + tiberium->Overlay->HeapID), cellptr->CellID, HOUSE_NONE);
					} else {
						new OverlayClass(*(&OverlayTypes[Scen->RandomNumber(0, 11)] + tiberium->Overlay->HeapID), cellptr->CellID, HOUSE_NONE);
					}
					tiberium->Queue_Growth(cellptr->CellID);
					cellptr->OverlayData = 0;
					cellptr->Overlay_Render_Rect();
					Map.Radar_Background(cellptr->CellID);
				}
			}
		}
		Delete_Me();
		return;
	}

	/*
	 * Emit the trailer animation every other frame.
	 */
	if (Class->TrailerAnim != NULL) {
		if ((Frame % 2) == 0) {
			new AnimClass(Class->TrailerAnim, Coord(MyCoord.X, MyCoord.Y, (MyCoord.Z * CELL_LEPTON_DIAG / (ISO_TILE_PIXEL_W * 0.8660254037844) + 0.5)), 1, 1, ShapeFlags_Type(SHAPE_WIN_REL|SHAPE_CENTER), 0);
		}
	}

	/*
	 * Advance the physics. BOUNCE_IMPACT means the anim just struck the ground
	 * this frame; BOUNCE_SETTLED means it has come to rest.
	 */
	BounceResultType bounce_result = BounceClass::AI();
	if (Class->IsMeteor) {
		Velocity.Z = Velocity.Z + Gravity;
	}

	switch (bounce_result) {
		case BOUNCE_IMPACT: {

			/*
			 * Struck the ground. If it landed in water it simply expires; otherwise it
			 * plays the bounce effects and damages whatever it landed on.
			 */
			Coord coord = Position;
			if (Map[coord].Land_Type() == LAND_WATER) {
				ECCounter = 0;
			} else {
				if (Class->BounceAnim != NULL) {
					new AnimClass(Class->BounceAnim, Coord(Position), 0, 1, ShapeFlags_Type(SHAPE_WIN_REL|SHAPE_CENTER), 0);
				}
				if ((unsigned char)Class->BounceSound != (unsigned char)VOC_NONE) {
					Sound_Effect(Class->BounceSound, Coord(Position.X, Position.Y, Position.Z));
				}
				if (Class->Warhead != NULL) {
					for (ObjectClass * occupier = Map[Get_Bounce_Coord()].Cell_Occupier(); occupier != NULL; occupier = occupier->Next) {
						Coord occoord = occupier->PositionCoord;
						Coord mycoord = Get_Bounce_Coord();
						int lepton = abs(mycoord.X - occoord.X) + abs(mycoord.Y - occoord.Y);
						if (lepton <= Class->DamageRadius) {
							int damage = Class->Damage;
							occupier->Take_Damage(damage, Tactical::Z_Lepton_To_Pixel(lepton), Class->Warhead, 0, 0, 0);
						}
					}
				}
			}
			break;
		}

		case BOUNCE_SETTLED:
			ECCounter = 0;
			break;
	}

	PositionCoord = Get_Bounce_Coord();
}


/// <summary>
/// Fetches the display layer this animation belongs to.
/// Voxel animations are thrown about by the bounce logic, so they render in the air
/// layer no matter how close to the ground they happen to be.
/// </summary>
/// <returns>Returns with LAYER_AIR.</returns>
LayerType VoxelAnimClass::In_Which_Layer(void) const
{
	return(LAYER_AIR);
}


/// <summary>
/// Lists the members this animation carries.
/// </summary>
/// <param name="stream">The stream carrying the members.</param>
void VoxelAnimClass::Serialize(SaveStreamClass & stream)
{
	BASECLASS::Serialize(stream);
	BounceClass::Serialize(stream);

	stream.Serialize(Unused1);
	stream.Serialize(Class);
	stream.Serialize(AttachedParticleSys);
	stream.Serialize(House);
	stream.Serialize(IsToDie);
	stream.Serialize(IsInvisible);
	stream.Serialize(ECCounter);
}


/// <summary>
/// Adds the state of this animation to the running game checksum.
/// This routine is used by the multiplayer sync check to prove that every machine holds
/// an identical copy of this object.
/// </summary>
/// <param name="crc">The checksum engine to submit the object state to.</param>
void VoxelAnimClass::Compute_CRC(CRCEngine & crc) const
{
	BASECLASS::Compute_CRC(crc);
	crc(Unused1);
	crc(Class->Fetch_ID());
	if (AttachedParticleSys != NULL) {
		crc(AttachedParticleSys->Fetch_ID());
	}
	crc(House->Fetch_ID());
	crc(IsToDie);
	crc(IsInvisible);
	crc(ECCounter);
}


/// <summary>
/// Fetches the class identifier used to persist this object.
/// The save system writes this identifier ahead of the object data so that the loader
/// knows what kind of object to reconstruct.
/// </summary>
/// <param name="retval">Pointer to the buffer that will receive the class identifier.</param>
/// <returns>Returns with S_OK, or E_POINTER if no buffer was supplied.</returns>
HRESULT VoxelAnimClass::GetClassID(CLSID * retval)
{
	if (retval == NULL) return(E_POINTER);
	*retval = CLSID_VoxelAnimClass;
	return(S_OK);
}


/// <summary>
/// Fetches the run time type identifier of this object.
/// </summary>
/// <returns>Returns with RTTI_VOXELANIM.</returns>
RTTIType VoxelAnimClass::Fetch_RTTI(void) const
{
	return(RTTI_VOXELANIM);
}


/// <summary>
/// Fetches the type class of this animation.
/// </summary>
/// <returns>Returns with a pointer to the voxel animation type this object was made from.</returns>
ObjectTypeClass const * VoxelAnimClass::Class_Of(void) const
{
	return(Class);
}


/// <summary>
/// Fetches the cell occupation list for this animation.
/// A voxel animation is purely decorative and never reserves any part of the map, so it
/// presents an empty occupation list to the placement code.
/// </summary>
/// <returns>Returns with the occupation list, which is always NULL.</returns>
Cell const * VoxelAnimClass::Occupy_List(bool placement) const
{
	return(NULL);
}
