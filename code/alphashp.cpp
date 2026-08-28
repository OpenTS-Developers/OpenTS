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

#include "alphashp.h"

#include "_alpha.h"
#include "_tactica.h"
#include "crc.h"
#include "globals.h"
#include "object.h"
#include "objtype.h"
#include "savestream.h"
#include "shapeset.h"
#include "sun.h"
#include "tactical.h"
#include "tracker.h"
#include "vector.h"

#include <algorithm>

DynamicVectorClass<AlphaShapeClass *> AlphaShapes;

unsigned char AlphaShapeClass::BrightnessTable[256][256];
static bool BrightnessCalculated = false;


/// <summary>
/// Creates an alpha shape for the object specified.
/// This routine attaches the light casting shape declared by the owner's type at the
/// screen position given. The shape adds itself to the global list, so the tactical map
/// will blend it into the alpha buffer from here on.
/// </summary>
/// <param name="owner">The object that this alpha shape belongs to.</param>
/// <param name="x">The horizontal pixel position to place the shape at.</param>
/// <param name="y">The vertical pixel position to place the shape at.</param>
AlphaShapeClass::AlphaShapeClass(ObjectClass * owner, int x, int y) :
	BASECLASS(),
	Owner(owner),
	ImageData(NULL),
	IsToDelete(false)
{
	ShapeSet const * image = (ShapeSet const *)owner->Class_Of()->AlphaImageData;
	ImageData = image;
	DrawRect = Rect(x, y, image->Get_Width(), image->Get_Height());
	AlphaShapes.Add(this);
	ObjectPtrTracker.Add(this);

	if (!BrightnessCalculated) {
		Calculate_Brightness_Table();
	}
}


/// <summary>
/// Creates a blank alpha shape.
/// This routine is used by the save/load system, which needs an empty object to read the
/// saved state into.
/// </summary>
AlphaShapeClass::AlphaShapeClass(void) :
	BASECLASS(),
	Owner(NULL),
	DrawRect(0, 0, 0, 0),
	ImageData(NULL),
	IsToDelete(false)
{
	AlphaShapes.Add(this);
	ObjectPtrTracker.Add(this);

	if (!BrightnessCalculated) {
		Calculate_Brightness_Table();
	}
}


/// <summary>
/// Destroys the alpha shape.
/// This routine takes the shape out of the global list so that it stops contributing
/// light to the tactical map.
/// </summary>
AlphaShapeClass::~AlphaShapeClass(void)
{
	AlphaShapes.Delete(this);
	ObjectPtrTracker.Delete(this);
}


/// <summary>
/// Fetches the class identifier used to persist this object.
/// The save system writes this identifier ahead of the object data so that the loader
/// knows what kind of object to reconstruct.
/// </summary>
/// <param name="retval">Pointer to the buffer that will receive the class identifier.</param>
/// <returns>Returns with S_OK, or E_POINTER if no buffer was supplied.</returns>
HRESULT STDMETHODCALLTYPE AlphaShapeClass::GetClassID(CLSID * retval)
{
	if (retval == NULL) return(E_POINTER);
	*retval = CLSID_AlphaShapeClass;
	return(S_OK);
}


/// <summary>
/// Fetches the run time type identifier of this object.
/// </summary>
/// <returns>Returns with RTTI_ALPHASHAPE.</returns>
RTTIType AlphaShapeClass::Fetch_RTTI(void) const
{
	return(RTTI_ALPHASHAPE);
}


/// <summary>
/// Adds the state of this alpha shape to the running game checksum.
/// This routine is used by the multiplayer sync check to prove that every machine holds
/// an identical copy of this object.
/// </summary>
/// <param name="crc">The checksum engine to submit the object state to.</param>
void AlphaShapeClass::Compute_CRC(CRCEngine & crc) const
{
	BASECLASS::Compute_CRC(crc);
	crc(DrawRect.X);
	crc(DrawRect.Y);
	crc(DrawRect.Width);
	crc(DrawRect.Height);
}


/// <summary>
/// Lists the members this alpha shape carries.
/// </summary>
/// <param name="stream">The stream carrying the members.</param>
void AlphaShapeClass::Serialize(SaveStreamClass & stream)
{
	BASECLASS::Serialize(stream);

	stream.Serialize(Owner);
	stream.Serialize(DrawRect);
	// ImageData -- fetched again from the owner's type the next time the shape is drawn.
	stream.Serialize(IsToDelete);
	// BrightnessTable -- a blending table built once and shared by every shape.
}


/// <summary>
/// Removes any reference this shape has to the object specified.
/// When the object going away is this shape's owner, the shape flags itself for deletion
/// rather than lingering with a dangling owner pointer.
/// </summary>
/// <param name="target">The object that is about to be removed from the game.</param>
void AlphaShapeClass::Detach(AbstractClass const * target, bool all)
{
	if (target == Owner) {
		IsToDelete = true;
	}
}


/// <summary>
/// Purges the alpha shapes that have been flagged for deletion.
/// This routine is used by the game logic to clean up the shapes whose owning object has
/// gone away, and to ensure the brightness table exists before any drawing is attempted.
/// </summary>
void AlphaShapeClass::Update_All(void)
{
	if (!BrightnessCalculated) {
		Calculate_Brightness_Table();
	}

	for (int i = AlphaShapes.Count() - 1; i >= 0; i--) {
		if (AlphaShapes[i]->IsToDelete) {
			delete AlphaShapes[i];
		}
	}
}


/// <summary>
/// Draws the alpha shapes that fall across a single map tile.
/// This routine is the per cell counterpart to Draw_All. Each shape's light is blended
/// into the alpha buffer as usual, but only where it lands inside the diamond of the
/// tile being drawn.
/// </summary>
/// <param name="point">The pixel position of the tile being drawn.</param>
/// <param name="cliprect">The clipping rectangle to draw within.</param>
void AlphaShapeClass::Draw_In_Area(Point2D const & point, Rect const & cliprect)
{
	/// The table is spelled with literal 0x20 (space) and 0xDB (solid block) characters,
	/// written as escapes here so that it survives text encoding conversions.
	#define __ "\x20"
	#define XX "\xDB"

	static const unsigned char _tilemask[] = {
		__ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __
		__ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ XX XX XX XX __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __
		__ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ XX XX XX XX XX XX XX XX __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __
		__ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ XX XX XX XX XX XX XX XX XX XX XX XX __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __
		__ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __
		__ __ __ __ __ __ __ __ __ __ __ __ __ __ XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX __ __ __ __ __ __ __ __ __ __ __ __ __ __
		__ __ __ __ __ __ __ __ __ __ __ __ XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX __ __ __ __ __ __ __ __ __ __ __ __
		__ __ __ __ __ __ __ __ __ __ XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX __ __ __ __ __ __ __ __ __ __
		__ __ __ __ __ __ __ __ XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX __ __ __ __ __ __ __ __
		__ __ __ __ __ __ XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX __ __ __ __ __ __
		__ __ __ __ XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX __ __ __ __
		__ __ XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX __ __
		XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX
		__ __ XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX __ __
		__ __ __ __ XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX __ __ __ __
		__ __ __ __ __ __ XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX __ __ __ __ __ __
		__ __ __ __ __ __ __ __ XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX __ __ __ __ __ __ __ __
		__ __ __ __ __ __ __ __ __ __ XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX __ __ __ __ __ __ __ __ __ __
		__ __ __ __ __ __ __ __ __ __ __ __ XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX __ __ __ __ __ __ __ __ __ __ __ __
		__ __ __ __ __ __ __ __ __ __ __ __ __ __ XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX __ __ __ __ __ __ __ __ __ __ __ __ __ __
		__ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __
		__ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ XX XX XX XX XX XX XX XX XX XX XX XX __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __
		__ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ XX XX XX XX XX XX XX XX __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __
		__ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ XX XX XX XX __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __
	};

	#undef __
	#undef XX

	for (int i = 0; i < AlphaShapes.Count(); i++) {
		if (!AlphaShapes[i]->IsToDelete) {
			Rect r1 = AlphaShapes[i]->DrawRect;
			r1 -= Point2D(TacticalMap->TacPixelX, TacticalMap->TacPixelY);
			r1 += TacticalRect.Top_Left();

			Rect r2 = Intersect(r1, cliprect);
			Rect r3(point.X, point.Y, ISO_TILE_PIXEL_W, ISO_TILE_PIXEL_H);
			Rect r4 = Intersect(r2, r3);

			if (r4.Is_Valid()) {
				int dx = r4.X - r3.X;
				int dy = r4.Y - r3.Y;

				ShapeSet const * shape = AlphaShapes[i]->ImageData;
				Rect shape_rect = shape->Get_Rect(0);

				int top = std::max(r4.Y, shape_rect.Y + r1.Y);
				int src_y = top - shape_rect.Y - r1.Y;
				int bottom = std::min(r4.Y + r4.Height, shape_rect.Y + shape_rect.Height + r1.Y);
				int left = std::max(r4.X, shape_rect.X + r1.X);
				int src_x = left - shape_rect.X - r1.X;
				int right = std::min(r4.X + r4.Width, shape_rect.X + shape_rect.Width + r1.X);

				int shape_skip = shape_rect.Width - right + shape_rect.X + src_x + r1.X;
				int alpha_skip = AlphaBuffer->Get_Buffer_Width() - right + left;
				int mask_skip = ISO_TILE_PIXEL_W - r4.Width;

				const unsigned char * maskptr = &_tilemask[ISO_TILE_PIXEL_W * dy + dx];
				unsigned char * shapedata = (unsigned char *)shape->Get_Data(0);

				unsigned short * alphaptr = (unsigned short *)AlphaBuffer->Get_Buffer_Offset(Point2D(left, top - TacticalRect.Y));
				unsigned char * shapeptr = (unsigned char *)&shapedata[src_x + src_y * shape_rect.Width];
				if (&alphaptr[right - left + (bottom - top) * AlphaBuffer->Get_Buffer_Width() + 2] >= (unsigned short *)AlphaBuffer->Get_Buffer_End()) {
					for (int i = top; i < bottom; i++) {
						for (int j = left; j < right; j++) {
							unsigned char pixel = *shapeptr++;
							if (*maskptr++ != '\x20') {
								*alphaptr = BrightnessTable[pixel][*alphaptr];
							}
							alphaptr++;
							alphaptr = (unsigned short *)AlphaBuffer->Wrap_Overflow((uintptr_t)alphaptr);
						}
						shapeptr += shape_skip;
						maskptr += mask_skip;
						alphaptr += alpha_skip;
						alphaptr = (unsigned short *)AlphaBuffer->Wrap_Overflow((uintptr_t)alphaptr);
					}
				} else {
					for (int i = top; i < bottom; i++) {
						for (int j = left; j < right; j++) {
							unsigned char pixel = *shapeptr++;
							if (*maskptr++ != '\x20') {
								*alphaptr = BrightnessTable[pixel][*alphaptr];
							}
							++alphaptr;
						}
						shapeptr += shape_skip;
						alphaptr += alpha_skip;
						maskptr += mask_skip;
					}
				}
			}
		}
	}
}


/// <summary>
/// Draws every alpha shape into the alpha buffer.
/// This routine is used by the tactical map to lay down the light contribution of all
/// registered shapes in one pass, blending each shape against whatever light has already
/// been recorded for that spot.
/// </summary>
/// <param name="cliprect">The clipping rectangle to draw within.</param>
void AlphaShapeClass::Draw_All(Rect const & cliprect)
{
	for (int i = 0; i < AlphaShapes.Count(); i++) {
		{
			Rect r1 = AlphaShapes[i]->DrawRect;
			r1 -= Point2D(TacticalMap->TacPixelX, TacticalMap->TacPixelY);
			r1 += TacticalRect.Top_Left();

			Rect clip = cliprect;
			Rect r2 = Intersect(r1, clip);

			if (r2.Is_Valid()) {
				int x1 = r1.X;
				int y1 = r1.Y;

				ShapeSet const * shape = AlphaShapes[i]->ImageData;
				if (shape == NULL) {
					AlphaShapes[i]->ImageData = (ShapeSet const *)AlphaShapes[i]->Owner->Class_Of()->AlphaImageData;
					shape = AlphaShapes[i]->ImageData;
				}
				Rect shape_rect = shape->Get_Rect(0);

				int top = std::max(r2.Y, shape_rect.Y + y1);
				int src_y = top - shape_rect.Y - y1;
				int bottom = std::min(clip.Y + clip.Height, shape_rect.Y + shape_rect.Height + y1);
				int left = std::max(r2.X, shape_rect.X + x1);
				int src_x = left - shape_rect.X - x1;
				int right = std::min(clip.X + clip.Width, shape_rect.X + shape_rect.Width + x1);

				int shape_skip = shape_rect.Width - right + shape_rect.X + x1 + src_x;
				int alpha_skip = AlphaBuffer->Get_Buffer_Width() - right + left;

				unsigned char * shapedata = (unsigned char *)shape->Get_Data(0);

				unsigned short * alphaptr = (unsigned short *)AlphaBuffer->Get_Buffer_Offset(Point2D(left - TacticalRect.X, top - TacticalRect.Y));

				unsigned char * shapeptr = (unsigned char *)&shapedata[src_x + src_y * shape_rect.Width];
				if (&alphaptr[right - left + (bottom - top) * AlphaBuffer->Get_Buffer_Width() + 2] >= (unsigned short *)AlphaBuffer->Get_Buffer_End()) {
					for (int i = top; i < bottom; i++) {
						for (int j = left; j < right; j++) {
							unsigned char pixel = *shapeptr++;
							*alphaptr = BrightnessTable[pixel][*alphaptr];
							alphaptr++;
							alphaptr = (unsigned short *)AlphaBuffer->Wrap_Overflow((uintptr_t)alphaptr);
						}
						shapeptr += shape_skip;
						alphaptr += alpha_skip;
						alphaptr = (unsigned short *)AlphaBuffer->Wrap_Overflow((uintptr_t)alphaptr);
					}
				} else {
					for (int i = top; i < bottom; i++) {
						for (int j = left; j < right; j++) {
							unsigned char pixel = *shapeptr++;
							*alphaptr = BrightnessTable[pixel][*alphaptr];
							++alphaptr;
						}
						shapeptr += shape_skip;
						alphaptr += alpha_skip;
					}
				}
			}
		}
	}
}


/// <summary>
/// Builds the alpha blending brightness table.
/// The table pairs a shape pixel with the light level already recorded in the alpha
/// buffer and yields the resulting brightness, so that the draw routines can blend with
/// a single lookup. It is built once, the first time an alpha shape is needed.
/// </summary>
void AlphaShapeClass::Calculate_Brightness_Table(void)
{
	BrightnessCalculated = true;
	for (int i = 0; i < 256*256; i++) {
		int low = i % 256;
		int high = i / 256;
		int brightness = std::max(0, std::min(255, low * high / 127));
		BrightnessTable[0][i] = brightness;
	}
}
