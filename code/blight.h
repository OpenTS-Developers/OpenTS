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

#include "object.h"

#include "blight.hh"

class BuildingLightClass : public ObjectClass
{
		typedef ObjectClass BASECLASS;

	public:
		BuildingLightClass(TechnoClass * owner = NULL);
		virtual ~BuildingLightClass(void) override;

		virtual HRESULT STDMETHODCALLTYPE GetClassID(CLSID * retval) override;

		virtual void Serialize(SaveStreamClass & stream) override;

		/*---------------------------------------------------------------------
		**	Member function prototypes.
		*/
		virtual ObjectTypeClass const * Class_Of(void) const override;
		virtual void Compute_CRC(CRCEngine & crc) const override;
		virtual void Detach(AbstractClass const * target, bool all = true) override;
		virtual RTTIType Fetch_RTTI(void) const override {return(RTTI_LIGHT);}
		virtual LayerType In_Which_Layer(void) const override;

		/*
		**	Object entry and exit from the game system.
		*/
		virtual bool Limbo(void) override;
		virtual bool Unlimbo(Coord const & , Dir256 facing=DIR_N) override;

		/*
		**	Display and rendering support functionality. Supports imagery and how
		**	object interacts with the map and thus indirectly controls rendering.
		*/
		virtual void Draw_It(Point2D const & point, Rect const & cliprect) const override;

		/*
		**	AI.
		*/
		virtual void AI(void) override;

		void Init_Rotation_Arc(TechnoClass * obj);
		void Set_Behavior_Type(LightBehaviorType type);
		int Detection_Radius(void) const;
		int Sweep_Stage(void) const;

	public:
		/*
		 * This is how far the beam has been rotated, expressed in radians, from the arc the
		 * light was laid out with. A sweeping light swings it back and forth under
		 * Acceleration, while a circling light simply winds it on and wraps it at a full turn.
		 */
		double Speed;

		/*
		 * This is the point the beam is swung about, placed behind the owner when the light's
		 * arc is laid out. Pivoting from behind is what makes the beam sweep across the ground
		 * in front of the building rather than spin about on the spot.
		 */
		Coord RotationPivot;

		/*
		 * This is where the beam rests when it has not been rotated at all, placed out in
		 * front of the owner. Both the sweep and the circle turn the vector that runs to it,
		 * so it fixes how far the light reaches as well as which way it points.
		 */
		Coord RotationTarget;

		/*
		 * This is the rate that the beam's rotation is being changed by. It winds up to the
		 * rule book's spotlight speed while the beam travels and then falls away through zero
		 * as the far end of the arc is neared, which gives the sweep its lazy reversal.
		 */
		double Acceleration;

		/*
		 * Used to ensure that Follow behavior is only applied to the
		 * spotlight that actually detected the enemy, rather than to
		 * other spotlights sharing the same trigger/tag.
		 */
		bool IsDetectionSource;

		/*
		 * If the beam is sweeping back the other way along its arc, then this flag will be
		 * true. Alternate lights are created with it already set, so that a row of spotlights
		 * does not swing in unison.
		 */
		bool IsOppositeDirection;

		/*
		 * This specifies the manner in which the beam travels -- sweeping across its arc,
		 * circling its owner, or chasing a target. A light left at LIGHT_BEHAVIOR_NONE is not
		 * drawn at all.
		 */
		int Behavior;

		/*
		 * This points to the object the beam is chasing while the light is set to follow. It
		 * is chosen as the nearest enemy in the neighborhood when that behavior is taken up,
		 * and the beam gives up on it once it strays beyond the spotlight movement radius.
		 */
		TechnoClass * Target;

		/*
		 * This points to the building the light is mounted upon. Everything about the light
		 * is measured from it -- the arc swept, the beam drawn back up to the caster, and the
		 * triggers sprung by an intruder -- so a light outliving its owner deletes itself.
		 */
		TechnoClass * Owner;
};
