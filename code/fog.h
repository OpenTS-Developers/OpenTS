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
#include "index.h"
#include "rect.h"
#include "vector.h"

#include "overlay.hh"
#include "smudge.hh"

class ObjectTypeClass;
class Cell;
class Coord;
class TerrainClass;
class BuildingTypeClass;
template<class T> class DynamicVectorClass;

class FoggedObjectClass : public AbstractClass
{
		typedef AbstractClass BASECLASS;

		friend class CellClass;
		friend void Update_Fogged_Objects(void);

	public:
		FoggedObjectClass(void);
		FoggedObjectClass(Coord const & coord, OverlayType type, int data);
		FoggedObjectClass(Coord const & coord, SmudgeType type, int data);
		FoggedObjectClass(BuildingClass * object, bool fade);
		FoggedObjectClass(TerrainClass * object);
		virtual ~FoggedObjectClass(void) override;

		virtual HRESULT STDMETHODCALLTYPE GetClassID(CLSID * retval) override;

		virtual void Serialize(SaveStreamClass & stream) override;
		virtual void Post_Load(void) override;

		virtual RTTIType Fetch_RTTI(void) const override;

		virtual void Compute_CRC(CRCEngine &) const override;

		Cell const *Get_Head_Record_Occupy_List(void);
		ObjectTypeClass *Get_Head_Record_Object_Type(void);
		virtual Cell Get_Cell(void);

		/*
		 * This is the master list of every fogged object in the game. An object adds itself
		 * when it is created and removes itself when it is destroyed, so that the whole set
		 * can be walked or thrown away at once.
		 */
		static DynamicVectorClass<FoggedObjectClass *> FoggyObjects;

		/*
		 * This is an index of the fogged objects keyed by the cell they lie in and the kind
		 * of object they remember. The fog is drawn by walking this index, so the records
		 * come out in map order rather than in the order they happened to be created.
		 */
		static IndexClass<int, FoggedObjectClass *> FoggedObjectIndex;

	private:

		struct DrawRecord {
			/*
			 * This is the object type whose artwork this record draws -- the building type
			 * for the structure itself, or the animation type for one of its animations.
			 */
			ObjectTypeClass *TypeClass;

			/*
			 * This is the frame of the type's shape to draw, captured when the object was
			 * fogged so that it keeps the pose it was last seen in.
			 */
			int FrameNumber;

			/*
			 * If the recorded shape must be drawn flat against the ground rather than
			 * standing upright against a depth shape, then this flag will be true. Laser
			 * fence posts and firestorm wall sections are drawn that way.
			 */
			unsigned char HeightAdjust;

			/*
			 * This is the depth bias the recorded shape is drawn with, copied from the
			 * animation it came from so that it stays layered against its building.
			 */
			int ZAdjust;

			DrawRecord(ObjectTypeClass *type = NULL, int framenum = 0, int hadjust = 0, int zadjust = 0) :
				TypeClass(type),
				FrameNumber(framenum),
				HeightAdjust(hadjust),
				ZAdjust(zadjust)
			{
			}

			DrawRecord(const DrawRecord &that) :
				TypeClass(that.TypeClass),
				FrameNumber(that.FrameNumber),
				HeightAdjust(that.HeightAdjust),
				ZAdjust(that.ZAdjust)
			{
			}

			bool operator == (DrawRecord const & that) { return(TypeClass == that.TypeClass && FrameNumber == that.FrameNumber); }
			bool operator != (DrawRecord const & that) { return(TypeClass != that.TypeClass || FrameNumber != that.FrameNumber); }

			// Carries the recorded shape to or from a save game.
			template<typename S>
			void Serialize(S & stream)
			{
				stream.Serialize(TypeClass);
				stream.Serialize(FrameNumber);
				stream.Serialize(HeightAdjust);
				stream.Serialize(ZAdjust);
			}
		};

	public:
		/*
		 * This is the overlay the player last saw in this cell. It is remembered so that
		 * a wall or a tiberium patch goes on being drawn, and goes on being targetable,
		 * after the cell has fallen back under the fog.
		 */
		OverlayType Overlay;

		/*
		 * This is the house that owned the building this record remembers. It supplies the
		 * color scheme the fogged structure is drawn with and decides whether the player is
		 * still entitled to target it. It is NULL for anything but a building.
		 */
		HouseClass *House;

		/*
		 * This is the data value of the remembered overlay, which fixes the frame a wall or
		 * a tiberium patch is drawn with.
		 */
		int OverlayData;

		/*
		 * This is the kind of object this record remembers -- an overlay, a smudge, a
		 * terrain object, or a building -- which decides how it is drawn. The type of the
		 * fog record itself is a separate matter, reported by Fetch_RTTI.
		 */
		RTTIType RTTI;

		/*
		 * This is the coordinate the remembered object stood at. It fixes where the record
		 * is drawn and which cell holds it fogged.
		 */
		Coord Position;

		/*
		 * This is the screen area this record covers, biased by the tactical map's pixel
		 * origin so that it stays correct as the view scrolls. It is what the record is
		 * clipped against when drawing, and what is flagged for redraw when it is destroyed.
		 */
		Rect BoundingRect;

		/// Unused
		int CellHeight;

		/*
		 * This is the smudge the player last saw in this cell, so that a crater or a scorch
		 * mark goes on showing after the cell has fallen back under the fog.
		 */
		SmudgeType Smudge;

		/*
		 * This is the frame of the remembered smudge to draw, which is how far the crater
		 * or scorch mark had spread when it was last seen.
		 */
		int SmudgeData;

		/*
		 * These are the shapes that make up the remembered image. The first record is the
		 * object itself and any that follow are the animations that were attached to it, so
		 * that a fogged building keeps its lit windows and its smoke.
		 */
		DynamicVectorClass<DrawRecord> Records;

		/*
		 * If this record may be drawn, then this flag will be true. A building that counted
		 * as a vehicle, or that was fully translucent when the fog closed over it, leaves a
		 * record that still holds its cells fogged but is never drawn and never targeted.
		 */
		bool CanDraw;
};

void Draw_Fogged_Objects(Rect const & rect);
void Update_Fogged_Objects(void);
