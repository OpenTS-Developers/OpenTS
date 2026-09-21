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

#pragma once

#include "_voxel.h"
#include "abstype.h"
#include "index.h"
#include "rgb.h"

#include "armor.hh"
#include "voc.hh"

template<class T> class DynamicVectorClass;

/***************************************************************************
**	This the the common base class of game objects. Since these values
**	represent the unchanging object TYPES, this data is initialized at game
**	start and not changed during play. It is "const" data.
*/
class ObjectTypeClass : public AbstractTypeClass
{
		typedef AbstractTypeClass BASECLASS;

	public:
		/*
		 * This is the color that this object type's radial range indicator is drawn in. Both
		 * the ellipse and the spokes sweeping around it use it, so a sensor array and a cloak
		 * generator can be told apart at a glance.
		 */
		RGBClass RadialColor;

		/*
		**	The defense of this object is greatly affected by the type of armor
		**	it possesses. This value specifies the type of armor.
		*/
		ArmorType Armor;

		/*
		**	This is the maximum strength of this object type.
		*/
		int MaxStrength;

		/*
		**	These point to the shape imagery for this object type. Since the shape imagery
		**	exists in a separate file, the data is filled in after this object is constructed.
		*/
		void const * ImageData;
		void const * AlphaImageData;

		/*
		 * These are the voxel models and motion data for this object type -- the body, the
		 * turret (or the APC's weapon), and the barrel. They are filled in after this object
		 * is constructed, and the whole set is discarded if any piece of it fails to load.
		 */
		VoxelDataStruct Voxel;
		VoxelDataStruct AuxVoxel;
		VoxelDataStruct AuxVoxel2;

		/*
		 * This is the largest dimension of this object type's artwork in pixels -- the widest
		 * voxel layer, or the shape's width or height -- and never less than eight. It stands
		 * in wherever only a rough size is wanted, such as the dimensions a unit reports.
		 */
		int MaxSize;

		/*
		 * This is the sound effect that plays when this object type is crushed under a
		 * vehicle. If VOC_NONE, then the crushing happens silently.
		 */
		VocType CrushSound;

		/*
		**	This is the base name of the graphic data associated with this object
		**	type. If the graphic name is a null string, then there is no graphic
		**	associated with this object type.
		*/
		TStringID<24> GraphicName;
		TStringID<24> AlphaGraphicName;

		/*
		**	If the artwork for this object (only for generics) is theater specific, then
		**	this flag will be true. Civilian buildings are a good example of this.
		*/
		bool IsTheater;

		/*
		**	Is this object squashable by heavy vehicles?  If it is, then the vehicle
		**	can travel over this object and destroy it in the process.
		*/
		bool IsCrushable;

		/*
		**	Does this object type NOT show up on radar scans?  If true, then in any
		**	radar display, only the underlying ground will be show, not this object.
		**	Most terrain falls into this category, but only a few special real units/buildings
		**	do.
		*/
		bool IsStealthy;

		/*
		**	It is legal to "select" some objects in the game. If it is legal to select this
		**	object type then this flag will be true. Selected game objects typically display
		**	a floating health bar and allows special user I/O control.
		*/
		bool IsSelectable;

		/*
		**	Can this object be the target of an attack or move command?  Typically, only objects
		**	that take damage or can be destroyed are allowed to be a target.
		*/
		bool IsLegalTarget;

		/*
		**	"Insignificant" objects will not be announced when they are destroyed or when they
		**	appear. Terrain elements and some lesser vehicles have this characteristic.
		*/
		bool IsInsignificant;

		/*
		**	Is this object immune to normal combat damage?  Rocks and other inert type terrain
		**	object are typically of this type.
		*/
		bool IsImmune;

		/*
		**	"Sentient" objects are ones that have logic AI processing performed on them. All
		**	vehicles, buildings, infantry, and aircraft are so flagged. Terrain elements also
		**	fall under this category, but only because certain animation effects require this.
		*/
		bool IsSentient;

		/*
		**	If this object type affects the occupation and collision logic associated with
		**	cells, then this flag will be true. Typically, this characteristic is limited
		**	to buildings, units, terrain objects, and landed aircraft.
		*/
		bool IsFootprint;

		/*
		 * If this object type is drawn as a voxel model rather than a flat shape, then this
		 * flag will be true. A voxel type takes its artwork from a .VXL and .HVA pair rather
		 * than a .SHP, and is rendered fresh from whatever angle it is facing.
		 */
		bool IsVoxel;

		/*
		 * If the artwork for this object uses the new theater naming convention, then this flag
		 * will be true. Where IsTheater swaps the file extension, this convention rewrites the
		 * second letter of the graphic name to pick the temperate or snow version of the art.
		 */
		bool IsNewTheater;

		/*
		 * If this object type shows a radial range indicator while it is selected, then this
		 * flag will be true. Sensor arrays and cloak generators use it to show the player how
		 * far their field reaches.
		 */
		bool IsHasRadialIndicator;

		/*
		 * If this object type can pass through an active firestorm wall unharmed, then this
		 * flag will be true. Anything else that touches the wall, or that is caught trying to
		 * move into it, is destroyed outright.
		 */
		bool IsIgnoresFirestorm;

		/*
		 * These are the caches of rendered voxel images -- one per model, plus one for the
		 * body's shadow. A frame already rendered at a given facing is blitted from the cache
		 * rather than drawn again, which is what makes a screen of voxels affordable.
		 */
		VoxelIndexClass VoxelIndex;
		VoxelIndexClass AuxVoxelIndex;
		VoxelIndexClass ShadowVoxelIndex;
		VoxelIndexClass AuxVoxel2Index;

		//--------------------------------------------------------------------
		ObjectTypeClass(char const * ininame = NULL);
		virtual ~ObjectTypeClass() override;

		virtual void Serialize(SaveStreamClass & stream) override;
		virtual void Post_Load(void) override;

		static void One_Time(void);

		char const * Graphic_Name(void) const {return(GraphicName);}
		char const * Alpha_Graphic_Name(void) const {return(AlphaGraphicName);}

		virtual bool Read_INI(CCINIClass const & ini) override;
		virtual Coord const Coord_Fixup(Coord const & coord) const {return(coord);};
		virtual int Get_Ownable(void) const {return(-1);};
		virtual int Max_Pips(void) const;
		virtual Point3D Pixel_Dimensions(void) const;
		virtual Point3D Lepton_Dimensions(void) const;
		virtual bool Create_And_Place(Cell const & , HouseClass * house = NULL) const = 0;
		virtual int Cost_Of(HouseClass * house = NULL) const;
		virtual int Time_To_Build(void) const;
		virtual ObjectClass * Create_One_Of(HouseClass *) const = 0;
		virtual Cell const * Occupy_List(bool placement=false) const;
		virtual BuildingClass * Who_Can_Build_Me(bool intheory, bool needsnopower, bool legal, HouseClass * house) const;
		bool Can_Be_Built_At(BuildingClass const * building, bool needsnopower, bool legal, HouseClass const * house) const;
		virtual void const * Get_Cameo_Data(void) const;
		virtual void const * Get_Image_Data(void) const {return(ImageData);};

		static ObjectTypeClass const * From_Name(char const * name);

		void Fetch_Voxel_Image(void);
		void Fetch_Aux_Voxel_Image(void);
		void Fetch_Normal_Image(void);

		void Theater_Naming_Convention(char * name, TheaterType theater) const;

		static void Clear_Voxel_Indexes(void);

		void Clear_Voxel_Index(void)
		{
			VoxelIndex.Clear();
			AuxVoxelIndex.Clear();
			ShadowVoxelIndex.Clear();
			AuxVoxel2Index.Clear();
		}

		static void const * SelectShapes;
		static void const * PipShapes;

		/*
		 * This points to the pip imagery that everything except a building draws with. Pips
		 * appear beside the health bar to show cargo, ammunition, or a harvester's load.
		 */
		static void const * Pip2Shapes;

		/*
		 * This points to the talk bubble imagery. A trigger or team mission can put one of
		 * these bubbles over a character for a while, so the player can see who is speaking.
		 */
		static void const * TalkBubbleShapes;

		/*
		 * This is the list of every object type in the game, in the order the types were
		 * created. Each object type adds itself here when it is constructed, so that a type
		 * can be looked up by its name and so that every type can be visited at once.
		 */
		static DynamicVectorClass<ObjectTypeClass *> ObjectTypes;
};
