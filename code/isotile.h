/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/


#pragma once

#include "object.h"
#include "sun.h"

#include "isotype.hh"


class IsometricTileTypeClass;

class IsometricTileClass : public ObjectClass
{
		typedef ObjectClass BASECLASS;
	public:
		IsometricTileClass(IsometricTileType type, Cell const &cell);
		virtual ~IsometricTileClass() override;

		virtual HRESULT GetClassID(CLSID * retval) override;

		virtual void Serialize(SaveStreamClass & stream) override;

		virtual RTTIType Fetch_RTTI(void) const override;

		virtual ObjectTypeClass const * Class_Of(void) const override;
		virtual bool Limbo() override;
		virtual bool Unlimbo(Coord const &, Dir256 facing = DIR_N) override;
		virtual void Draw_It(Point2D const &point, Rect const &bounds) const override;
		virtual bool Mark(MarkType mark = MARK_CHANGE) override;

	public:
		/*
		 * This is a pointer to the isometric tile object's class, which supplies the
		 * dimensions and the sub-tile layout used when the tile is marked down on the map.
		 */
		IsometricTileTypeClass * Class;
};

inline IsometricTileClass * AbstractClass::As_IsometricTileClass(void) { return(dynamic_cast<IsometricTileClass *>(this)); }
inline IsometricTileClass const * AbstractClass::As_IsometricTileClass(void) const { return(dynamic_cast<IsometricTileClass const *>(this)); }
