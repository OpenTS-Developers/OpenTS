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

#include "fog.h"

#include "_convert.h"
#include "_map.h"
#include "_rect.h"
#include "_surface.h"
#include "_tactica.h"
#include "anim.h"
#include "animtype.h"
#include "building.h"
#include "builtype.h"
#include "cell.h"
#include "convert.h"
#include "coord.h"
#include "crc.h"
#include "draw.h"
#include "globals.h"
#include "goptions.h"
#include "house.h"
#include "inline.h"
#include "lightcon.h"
#include "objtype.h"
#include "rect.h"
#include "savestream.h"
#include "scheme.h"
#include "shapeset.h"
#include "smudtype.h"
#include "sun.h"
#include "tactical.h"
#include "terrain.h"
#include "vector.h"


DynamicVectorClass<FoggedObjectClass *> FoggedObjectClass::FoggyObjects;
IndexClass<int, FoggedObjectClass *> FoggedObjectClass::FoggedObjectIndex;


/// <summary>
/// Creates an empty fogged object.
/// The object joins the master list of fogged objects, but it remembers nothing until
/// something fills it in.
/// </summary>
FoggedObjectClass::FoggedObjectClass(void) :
	BASECLASS(),
	Overlay(OVERLAY_NONE),
	House(NULL),
	OverlayData(0),
	RTTI(RTTI_NONE),
	Position(0,0,0),
	BoundingRect(0,0,0,0),
	CellHeight(0),
	Smudge(SMUDGE_NONE),
	SmudgeData(0),
	Records(),
	CanDraw(true)
{
	FoggyObjects.Add(this);
}


/// <summary>
/// Creates a fogged record of a cell overlay.
/// This is what the player goes on seeing of a wall or a tiberium patch after the
/// cell it lies in has fallen back under the fog.
/// </summary>
/// <param name="coord">The coordinate of the cell the overlay lies in.</param>
/// <param name="type">The overlay type to remember.</param>
/// <param name="data">The overlay data value to remember.</param>
FoggedObjectClass::FoggedObjectClass(Coord const & coord, OverlayType type, int data) :
	BASECLASS(),
	Overlay(type),
	House(NULL),
	OverlayData(data),
	RTTI(RTTI_OVERLAY),
	Position(coord),
	CellHeight(Map[coord].Height),
	Smudge(SMUDGE_NONE),
	SmudgeData(0),
	Records(),
	CanDraw(true)
{
	CellClass * cellptr = &Map[coord];
	BoundingRect = Union(cellptr->Overlay_Render_Rect(), cellptr->Overlay_Shadow_Render_Rect());
	BoundingRect.X += TacticalMap->TacPixelX;
	BoundingRect.Y += TacticalMap->TacPixelY;
	BoundingRect -= TacticalRect.Top_Left();

	FoggyObjects.Add(this);

	Cell cell = Get_Cell();
	int id = cell.As_Int() - RTTI + INT_MAX;
	FoggedObjectIndex.Add_Index(id, this);
}


/// <summary>
/// Creates a fogged record of a smudge.
/// This is what the player goes on seeing of a crater or a scorch mark after the cell
/// it lies in has fallen back under the fog.
/// </summary>
/// <param name="coord">The coordinate of the cell the smudge lies in.</param>
/// <param name="type">The smudge type to remember.</param>
/// <param name="data">The smudge frame to remember.</param>
FoggedObjectClass::FoggedObjectClass(Coord const & coord, SmudgeType type, int data) :
	BASECLASS(),
	Overlay(OVERLAY_NONE),
	House(NULL),
	OverlayData(0),
	RTTI(RTTI_SMUDGE),
//	Position(coord),
	CellHeight(Map[coord].Height),
	Smudge(type),
	SmudgeData(data),
	Records(),
	CanDraw(true)
{
	Coord crd;
	Point2D point;
	CellClass * cellptr = &Map[coord];
	crd = Coord(coord);
	crd.Z = LEVEL_LEPTON_H * cellptr->Height;
	Position = crd;

	TacticalMap->Coord_To_Pixel(crd, point);
	point += Point2D(ISO_TILE_PIXEL_W / -2, ISO_TILE_PIXEL_H / -2);
	BoundingRect = Rect(point.X, point.Y, ISO_TILE_PIXEL_W, ISO_TILE_PIXEL_H);
	BoundingRect.X += TacticalMap->TacPixelX;
	BoundingRect.Y += TacticalMap->TacPixelY;

	FoggyObjects.Add(this);

	Cell cell = Get_Cell();
	int id = cell.As_Int() - RTTI + INT_MAX;
	FoggedObjectIndex.Add_Index(id, this);
}


/// <summary>
/// Creates a fogged record of a building.
/// The building's shape and each of its attached animations are recorded, so the
/// structure keeps its lit windows and its smoke under the fog. The building and its
/// animations are marked as fogged, which stops them from drawing themselves.
/// </summary>
/// <param name="object">The building to remember.</param>
/// <param name="fade">Should the remembered building be drawn?</param>
FoggedObjectClass::FoggedObjectClass(BuildingClass * object, bool fade) :
	BASECLASS(),
	Overlay(OVERLAY_NONE),
	House(object->House),
	OverlayData(0),
	RTTI(RTTI_BUILDING),
	Position(object->PositionCoord),
	CellHeight(Map[(Coord const &)object->PositionCoord].Height),
	Smudge(SMUDGE_NONE),
	SmudgeData(0),
	Records(),
	CanDraw(fade)
{
	BoundingRect = object->Get_Render_Rect();

	FoggyObjects.Add(this);

	Cell cell = Get_Cell();
	int id = cell.As_Int() - RTTI + INT_MAX;
	FoggedObjectIndex.Add_Index(id, this);

	object->IsFogged = true;
	int shapenum = object->Shape_Number();

	DrawRecord record(object->Class, shapenum, 0, 0);
	if (object->Class->IsLaserFence && (object->LaserFenceFrame == 12 || object->LaserFenceFrame == 8)) {
		record.HeightAdjust = 1;
	} else {
		record.HeightAdjust = 0;
		if (object->Class->IsFirestormWall) {
			record.HeightAdjust = 1;
		}
	}
	Records.Add(record);

	for (int i = 0; i < BANIM_COUNT; i++) {
		if (object->Anims[i] != NULL) {
			object->Anims[i]->IsFogged = true;
			AnimTypeClass *atype = object->Anims[i]->Class;
			BoundingRect = Union(BoundingRect, object->Anims[i]->Get_Visual_Rect());
			Records.Add(DrawRecord(atype, atype->Start + object->Anims[i]->Fetch_Stage(), 0, object->Anims[i]->ZAdjust));
		}
	}

	TacticalMap->Register_Dirty_Area(BoundingRect);
	BoundingRect.X += TacticalMap->TacPixelX;
	BoundingRect.Y += TacticalMap->TacPixelY;
}


/// <summary>
/// Creates a fogged record of a terrain object.
/// This is what the player goes on seeing of a tree or a rock after the cell it
/// stands in has fallen back under the fog.
/// </summary>
/// <param name="object">The terrain object to remember.</param>
FoggedObjectClass::FoggedObjectClass(TerrainClass * object) :
	BASECLASS(),
	Overlay(OVERLAY_NONE),
	House(NULL),
	OverlayData(0),
	RTTI(RTTI_TERRAIN),
	Position(object->PositionCoord),
	CellHeight(Map[(Coord const &)object->PositionCoord].Height),
	Smudge(SMUDGE_NONE),
	SmudgeData(0),
	Records(),
	CanDraw(true)
{
	BoundingRect = object->Get_Render_Rect();
	BoundingRect.X += TacticalMap->TacPixelX;
	BoundingRect.Y += TacticalMap->TacPixelY;

	FoggyObjects.Add(this);

	Records.Add(DrawRecord(object->Class, 0, 0, 0));

	Cell cell = Get_Cell();
	int id = cell.As_Int() - RTTI + INT_MAX;
	FoggedObjectIndex.Add_Index(id, this);
}


/// <summary>
/// Destroys the fogged object.
/// The record is unhooked from the fogged object list and index, any building it was
/// remembering is released so that it draws itself again, and the area it covered is
/// flagged for redraw.
/// </summary>
FoggedObjectClass::~FoggedObjectClass(void)
{
	FoggyObjects.Delete(this);

	Cell cell = Get_Cell();
	int id = cell.As_Int() - RTTI + INT_MAX;
	FoggedObjectIndex.Remove_Index(id);

	if (RTTI == RTTI_BUILDING) {
		BuildingClass *bptr = Map[cell].Cell_Building();
		if (bptr != NULL) {
			bptr->IsFogged = false;
			for (int i = 0; i < BANIM_COUNT; i++) {
				if (bptr->Anims[i] != NULL) {
					bptr->Anims[i]->IsFogged = false;
				}
			}
		}
	}

	if (TacticalMap != NULL) {
		Rect rect = BoundingRect;
		rect -= Point2D(TacticalMap->TacPixelX, TacticalMap->TacPixelY);
		TacticalMap->Register_Dirty_Area(rect);
	}
}


/// <summary>
/// Draws the remembered objects that lie under the fog of war.
/// This routine is called by the tactical map as it redraws a dirty region. Each
/// fogged object is drawn as it appeared when the player last saw it -- the overlay,
/// terrain object, smudge, or building with all of its attached animations. Nothing
/// is drawn at all when the shroud is being debugged away.
/// </summary>
/// <param name="rect">The dirty rectangle currently being redrawn.</param>
void Draw_Fogged_Objects(Rect const & rect)
{
	if (MainWindow == NULL) {
		return;
	}

	Rect cliprect = Intersect(rect, TacticalRect);
	if (!cliprect.Is_Valid()) {
		return;
	}

	for (int index = 0; index < FoggedObjectClass::FoggedObjectIndex.Count(); index++) {

		FoggedObjectClass * data = FoggedObjectClass::FoggedObjectIndex.Fetch_By_Position(index);

		if (!data->CanDraw) {
			continue;
		}

		/*
		 * Convert the stored bounding rectangle back into a screen rectangle and
		 * clip it against the dirty rectangle being redrawn.
		 */
		Rect rectangle = data->BoundingRect;
		rectangle.X -= TacticalMap->TacPixelX;
		rectangle.Y = rectangle.Y - TacticalMap->TacPixelY + TacticalRect.Y;
		rectangle.X += TacticalRect.X;

		Rect clipped = Intersect(cliprect, rectangle);
		if (!clipped.Is_Valid()) {
			continue;
		}

		switch (data->RTTI) {

			/*
			 * Draw a fogged overlay by temporarily restoring the cell's overlay to the
			 * fogged version and using the normal overlay draw routines.
			 */
			case RTTI_OVERLAY: {
				CellClass * cellptr = &Map[(Coord)data->Position];

				OverlayType saveoverlay = cellptr->Overlay;
				int saveoverlaydata = cellptr->OverlayData;
				cellptr->Overlay = data->Overlay;
				cellptr->OverlayData = data->OverlayData;

				Coord coord = Coord_Whole(Coord(cellptr->CellID));
				Point2D point;
				TacticalMap->Coord_To_Pixel(coord, point);
				point.X += ISO_TILE_PIXEL_W / -2;

				cellptr->Draw_Overlay(point, cliprect);
				cellptr->Draw_Overlay_Shadow(point, cliprect);

				cellptr->Overlay = saveoverlay;
				cellptr->OverlayData = saveoverlaydata;
				break;
			}

			/*
			 * Draw a fogged terrain object.
			 */
			case RTTI_TERRAIN: {
				Coord * position = &data->Position;
				ObjectTypeClass * type = data->Records[0].TypeClass;
				CellClass * cellptr = &Map[(Coord)*position];
				int shapenum = data->Records[0].FrameNumber;
				ShapeSet const * shape = (ShapeSet const *)type->Get_Image_Data();

				Point2D xy;
				TacticalMap->Coord_To_Pixel((Coord)*position, xy);
				xy += Point2D(TacticalRect.X - cliprect.X, TacticalRect.Y - cliprect.Y);

				int zadjust = -TacticalMap->Z_Lepton_To_Pixel(((Coord)*position).Z);

				if (cellptr->Drawer == NULL) {
					cellptr->Init_Drawer(NULL, 0x10000, 0, NORMAL_LIGHT, NORMAL_LIGHT, NORMAL_LIGHT);
				}

				Draw_Shape(*LogicalSurface, *cellptr->Drawer, shape, shapenum, xy, cliprect, ShapeFlags_Type(SHAPE_CENTER|SHAPE_WIN_REL|SHAPE_ALPHA|SHAPE_ZWRITE), NULL, zadjust - 4, ZGRAD_90DEG, cellptr->TileBrightness);

				if (DrawShapeShadows) {
					Draw_Shape(*LogicalSurface, *cellptr->Drawer, shape, shapenum + shape->Get_Count() / 2, xy, cliprect, ShapeFlags_Type(SHAPE_DARKEN|SHAPE_CENTER|SHAPE_WIN_REL|SHAPE_ZWRITE), NULL, zadjust - 2, ZGRAD_GROUND, NORMAL_LIGHT);
				}
				break;
			}

			/*
			 * Draw a fogged building. The first record is the building itself, the
			 * remaining records are its attached animations.
			 */
			case RTTI_BUILDING: {
				for (int record = 0; record < data->Records.Count(); record++) {

					BuildingTypeClass * type = (BuildingTypeClass *)data->Records[record].TypeClass;
					int shapenum = data->Records[record].FrameNumber;
					ShapeSet const * shape = (ShapeSet const *)type->Get_Image_Data();
					unsigned char heightadjust = data->Records[record].HeightAdjust;
					LightConvertClass * converter = ColorSchemes[data->House->Scheme]->Converter;

					if (record == 0) {

						/*
						 * Draw the building shape itself.
						 */
						Coord * position = &data->Position;
						Point2D drawpoint;
						TacticalMap->Coord_To_Pixel(*position - Coord(CELL_LEPTON_W / 2, CELL_LEPTON_H / 2, 0), drawpoint);
						CellClass * cellptr = &Map[(Coord)*position];
						drawpoint += Point2D(TacticalRect.X - cliprect.X, TacticalRect.Y - cliprect.Y);

						if (type->IsInvisibleInGame) {
							break;
						}

						int zadjust = type->NormalZAdjust;

						Point2D shapepoint = drawpoint;
						int height = drawpoint.Y + shape->Get_Height() / 2;
						Rect shapeclip = cliprect;
						if (shapeclip.Height > height) {
							shapeclip.Height = height;
						}

						Point2D zdrawpoint(144, 172);
						zdrawpoint += type->ZShapePointMove;
						Point2D zsizeoffset(type->Width() * CELL_LEPTON - CELL_LEPTON, type->Height() * CELL_LEPTON - CELL_LEPTON);
						zdrawpoint -= TacticalMap->Coord_To_Pixel_Absolute(zsizeoffset);

						ShapeSet const * zshapefile = (ShapeSet const *)BuildingTypeClass::BuildingZShape;

						int zlepton = -TacticalMap->Z_Lepton_To_Pixel(((Coord)*position).Z);
						int brightness = Map[(Coord)*position].Brightness + type->ExtraLight;

						if (cellptr->Drawer == NULL) {
							cellptr->Init_Drawer(NULL, 0x10000, 0, NORMAL_LIGHT, NORMAL_LIGHT, NORMAL_LIGHT);
						}
						ConvertClass * drawconvert = type->IsTerrainPalette ? cellptr->Drawer : converter;

						if (shapeclip.Height > 0) {
							if (heightadjust) {
								Draw_Shape(*LogicalSurface, *drawconvert, shape, shapenum, shapepoint, shapeclip, ShapeFlags_Type(SHAPE_CENTER|SHAPE_WIN_REL|SHAPE_ALPHA|SHAPE_ZWRITE), NULL, zlepton - 2, ZGRAD_GROUND, brightness);
								Draw_Shape(*LogicalSurface, *drawconvert, shape, shapenum + shape->Get_Count() / 2, shapepoint, shapeclip, ShapeFlags_Type(SHAPE_DARKEN|SHAPE_CENTER|SHAPE_WIN_REL|SHAPE_ALPHA|SHAPE_ZWRITE), NULL, zlepton - 2, ZGRAD_GROUND, NORMAL_LIGHT);
							} else {
								Draw_Shape(*LogicalSurface, *drawconvert, shape, shapenum, shapepoint, shapeclip, ShapeFlags_Type(SHAPE_CENTER|SHAPE_WIN_REL|SHAPE_ALPHA|SHAPE_ZWRITE), NULL, zlepton + zadjust - 2, ZGRAD_90DEG, brightness, zshapefile, 0, zdrawpoint);
								Draw_Shape(*LogicalSurface, *drawconvert, shape, shapenum + shape->Get_Count() / 2, shapepoint, shapeclip, ShapeFlags_Type(SHAPE_DARKEN|SHAPE_CENTER|SHAPE_WIN_REL|SHAPE_ALPHA|SHAPE_ZWRITE), NULL, zlepton - 2, ZGRAD_GROUND, NORMAL_LIGHT);
							}
						}

						/*
						 * Draw the building's bib graphic.
						 */
						if (type->BibShape != NULL) {
							Draw_Shape(*LogicalSurface, *drawconvert, type->BibShape, shapenum, drawpoint, cliprect, ShapeFlags_Type(SHAPE_CENTER|SHAPE_WIN_REL|SHAPE_ALPHA|SHAPE_ZWRITE), NULL, zlepton - 3, ZGRAD_GROUND, brightness);
						}

					} else {

						/*
						 * Draw an attached animation of the building.
						 */
						AnimTypeClass * anim = (AnimTypeClass *)type;

						if (anim->DetailLevel > Options.DetailLevel || shape == NULL) {
							continue;
						}

						Coord * position = &data->Position;
						Point2D drawpoint;
						TacticalMap->Coord_To_Pixel(*position - Coord(CELL_LEPTON_W / 2, CELL_LEPTON_H / 2, 0), drawpoint);
						drawpoint += Point2D(TacticalRect.X - cliprect.X, TacticalRect.Y - cliprect.Y);

						int brightness = NORMAL_LIGHT;
						if (!anim->IsUseNormalLight) {
							brightness = Map[(Coord)*position].Brightness;
						}

						ConvertClass * animconvert;
						if (converter != NULL && anim->IsShouldUseCellDrawer) {
							animconvert = (ConvertClass *)converter;
						} else {
							animconvert = AnimDrawer;
						}

						int zadjust = data->Records[record].ZAdjust;

						if (anim->IsFlat) {
							Draw_Shape(*LogicalSurface, *animconvert, shape, shapenum, drawpoint, cliprect, ShapeFlags_Type(SHAPE_CENTER|SHAPE_WIN_REL|SHAPE_ALPHA|SHAPE_ZGRAD), NULL, zadjust - TacticalMap->Z_Lepton_To_Pixel(((Coord)*position).Z) - 2, ZGRAD_GROUND, brightness);
						} else {
							Draw_Shape(*LogicalSurface, *animconvert, shape, shapenum, drawpoint, cliprect, ShapeFlags_Type(SHAPE_CENTER|SHAPE_WIN_REL|SHAPE_ALPHA|SHAPE_ZGRAD), NULL, zadjust - TacticalMap->Z_Lepton_To_Pixel(((Coord)*position).Z) - 2, ZGRAD_90DEG, brightness);
						}
					}
				}
				break;
			}

			/*
			 * Draw a fogged smudge.
			 */
			case RTTI_SMUDGE: {
				Point2D point;
				point.X = data->BoundingRect.X - TacticalMap->TacPixelX + TacticalRect.X + ISO_TILE_PIXEL_W / 2 - cliprect.X;
				point.Y = data->BoundingRect.Y - TacticalMap->TacPixelY + TacticalRect.Y - cliprect.Y;

				ShapeSet const * shape = (ShapeSet const *)SmudgeTypes[data->Smudge]->Get_Image_Data();
				if (shape != NULL) {
					Coord * position = &data->Position;
					CellClass * cellptr = &Map[(Coord)*position];
					Draw_Shape(*LogicalSurface, *cellptr->Drawer, shape, data->SmudgeData, point, cliprect, ShapeFlags_Type(SHAPE_CENTER|SHAPE_WIN_REL|SHAPE_ALPHA), NULL, -TacticalMap->Z_Lepton_To_Pixel(((Coord)*position).Z), ZGRAD_90DEG, cellptr->TileBrightness);
				}
				break;
			}

			default:
				break;
		}
	}
}


/// <summary>
/// Flags the cells covered by the fogged objects.
/// Use this routine to keep every cell that a remembered object covers marked as
/// fogged, so that the map does not fall back to plain shroud where the player still
/// has something to look at.
/// </summary>
void Update_Fogged_Objects(void)
{
	for (int i = 0; i < FoggedObjectClass::FoggyObjects.Count(); i++) {
		FoggedObjectClass *fptr = FoggedObjectClass::FoggyObjects[i];
		if (fptr->Get_Head_Record_Occupy_List() != NULL) {
			const Cell * list = fptr->Get_Head_Record_Occupy_List();
			Cell cell = fptr->Get_Cell();
			while (*list != REFRESH_EOL) {
				Cell ncell = *list + cell;
				Map[ncell].IsFogged = true;
				list++;
			}
		} else {
			Map[(Coord)fptr->Position].IsFogged = true;
		}
	}
}


/// <summary>
/// Lists the members this fogged object carries.
/// </summary>
/// <param name="stream">The stream carrying the members.</param>
void FoggedObjectClass::Serialize(SaveStreamClass & stream)
{
	BASECLASS::Serialize(stream);

	// FoggyObjects -- the master list, which each record joins as it is constructed.
	// FoggedObjectIndex -- the cell keyed index, re-registered by Post_Load.
	stream.Serialize(Overlay);
	stream.Serialize(House);
	stream.Serialize(OverlayData);
	stream.Serialize(RTTI);
	stream.Serialize(Position);
	stream.Serialize(BoundingRect);
	stream.Serialize(CellHeight);
	stream.Serialize(Smudge);
	stream.Serialize(SmudgeData);
	stream.Serialize(Records);
	stream.Serialize(CanDraw);
}


/// <summary>
/// Re-registers this fogged object with the index that the fog is drawn from.
/// The index is keyed by the cell the record lies in, which is only known once the
/// record's own members are back in place.
/// </summary>
void FoggedObjectClass::Post_Load(void)
{
	BASECLASS::Post_Load();

		Cell cell = Get_Cell();
		int id = cell.As_Int() - RTTI + INT_MAX;
		FoggedObjectIndex.Add_Index(id, this);
}


/// <summary>
/// Fetches the cell occupation list of the remembered object.
/// This routine is used when flagging cells as fogged, so that a remembered building
/// holds its entire footprint under the fog rather than just its anchor cell.
/// </summary>
/// <returns>Returns with a pointer to the occupation list, or NULL if the object
/// covers nothing beyond its own cell.</returns>
Cell const * FoggedObjectClass::Get_Head_Record_Occupy_List(void)
{
	if (RTTI == RTTI_BUILDING) {
		return(Records[0].TypeClass->Occupy_List());
	}
	return(NULL);
}


/// <summary>
/// Fetches the RTTI type of this object.
/// This identifies the fog record itself, not the object it is remembering -- the
/// kind of the remembered object is kept separately.
/// </summary>
/// <returns>Returns with RTTI_FOGGEDOBJECT.</returns>
RTTIType FoggedObjectClass::Fetch_RTTI(void) const
{
	return(RTTI_FOGGEDOBJECT);
}


/// <summary>
/// Fetches the class ID of this object.
/// This routine is part of the persistence interface the save game system uses to
/// recreate objects of the right kind when a game is loaded.
/// </summary>
/// <param name="retval">Pointer to the class ID to fill in.</param>
/// <returns>Returns with S_OK, or E_POINTER if no destination was supplied.</returns>
HRESULT FoggedObjectClass::GetClassID(CLSID * retval)
{
	if (retval == NULL) return(E_POINTER);
	*retval = CLSID_FoggedObjectClass;
	return(S_OK);
}


/// <summary>
/// Adds this fogged object's state to a CRC calculation.
/// This routine is used by the multiplayer sync check to prove that every machine
/// remembers the same things under the fog.
/// </summary>
/// <param name="crc">The CRC engine to submit this object's data to.</param>
void FoggedObjectClass::Compute_CRC(CRCEngine &crc) const
{
	BASECLASS::Compute_CRC(crc);
	crc(Overlay);
	crc(OverlayData);
	crc(RTTI);
	crc(Position.X);
	crc(Position.Y);
	crc(Position.Z);
	crc(BoundingRect.X);
	crc(BoundingRect.Y);
	crc(BoundingRect.Width);
	crc(BoundingRect.Height);
	crc(CellHeight);
	crc(Smudge);
	crc(SmudgeData);
	crc(Records.Count());
}


/// <summary>
/// Fetches the object type of the primary draw record.
/// For a fogged building this is the building type itself, since the records that
/// follow it belong to the animations that were attached to the structure.
/// </summary>
/// <returns>Returns with a pointer to the object type, or NULL if this fogged object
/// has nothing recorded to draw.</returns>
ObjectTypeClass * FoggedObjectClass::Get_Head_Record_Object_Type(void)
{
	if (Records.Count() > 0) {
		return(Records[0].TypeClass);
	}
	return(NULL);
}


/// <summary>
/// Fetches the cell that this fogged object occupies.
/// </summary>
/// <returns>Returns with the cell the object was standing in when it was fogged.</returns>
Cell FoggedObjectClass::Get_Cell(void)
{
	return(Position.As_Cell());
}
