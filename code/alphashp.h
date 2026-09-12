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

#include "abstract.h"
#include "rect.h"


class ShapeSet;
template<class T> class DynamicVectorClass;


class AlphaShapeClass : public AbstractClass
{
		typedef AbstractClass BASECLASS;

	public:

		/*---------------------------------------------------------------------
		**	Constructors, Destructors, and overloaded operators.
		*/
		AlphaShapeClass(ObjectClass * owner, int x, int y);
		AlphaShapeClass(void);
		~AlphaShapeClass(void);

		virtual ClassID Class_ID(void) const override;

		virtual void Serialize(SaveStreamClass & stream) override;

		virtual RTTIType Fetch_RTTI(void) const override;
		virtual void Compute_CRC(CRCEngine &) const override;
		virtual void Detach(AbstractClass const * target, bool all = true) override;

		static void Update_All(void);
		static void Draw_In_Area(Point2D const & point, Rect const & cliprect);
		static void Draw_All(Rect const & rect);
		static void Calculate_Brightness_Table(void);

	public:
		/*
		 * Pointer to the object that this alpha shape belongs to. Should that object leave the
		 * game, the shape marks itself for deletion rather than linger with a dangling owner.
		 */
		ObjectClass * Owner;

		/*
		 * This is the screen rectangle that the shape's light is blended into. Its position is
		 * where the owner was last drawn and its dimensions come from the shape itself.
		 */
		Rect DrawRect;

		/*
		 * Pointer to the shape that describes the light this object casts, taken from the
		 * owner's type. Its pixels index the BrightnessTable rather than a palette.
		 */
		ShapeSet const * ImageData;

		/*
		 * If this shape has outlived the object it belonged to, then this flag will be true.
		 * The shape is purged on a later logic pass rather than deleted on the spot.
		 */
		bool IsToDelete;

	private:
		/*
		 * This table pairs a shape pixel with the light level already recorded in the alpha
		 * buffer and yields the brightness that results, so that blending costs a single
		 * lookup. It is built once, the first time an alpha shape is needed.
		 */
		static unsigned char BrightnessTable[256][256];
};

extern DynamicVectorClass<AlphaShapeClass *> AlphaShapes;
