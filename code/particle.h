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
#include "rgb.h"
#include "vector3.h"


class ParticleTypeClass;
class ParticleSystemClass;


class ParticleClass : public ObjectClass
{
		typedef ObjectClass BASECLASS;

	public:
		ParticleClass(void);
		ParticleClass(ParticleTypeClass const * type, Coord const & origin, Coord const & target = COORD_NONE, ParticleSystemClass * partsys = NULL);
		virtual ~ParticleClass(void) override;

		virtual HRESULT GetClassID(CLSID * retval) override;
		virtual HRESULT Save(SaveStreamClass & stream, BOOL cleardirty) override;

		virtual void Serialize(SaveStreamClass & stream) override;

		virtual RTTIType Fetch_RTTI(void) const override;

		virtual LayerType In_Which_Layer(void) const override;
		virtual ObjectTypeClass const * Class_Of(void) const override;
		virtual Cell const * Occupy_List(bool placement=false) const override;
		virtual void Draw_It(Point2D const & point, Rect const & cliprect) const override;
		virtual bool Mark(MarkType=MARK_CHANGE) override;

		virtual int Shape_Number(void) const;

		void Gas_Behavior_AI(void);
		void Railgun_Behavior_AI(void);
		void Smoke_Behavior_AI(void);
		void Spark_Behavior_AI(void);
		void Fire_Behavior_AI(void);
		void Web_Behavior_AI(void);
		void Behavior_AI(void);

		void Smoke_Motion_AI(void);
		void Gas_Motion_AI(void);
		void Fire_Motion_AI(void);
		void Motion_AI(void);

	public:
		/*
		 * This points to the type of particle object this is.
		 */
		ParticleTypeClass const * Class;

		/*
		 * Color used by railgun and spark particles.
		 * Color is the "default" color used when ColorIndex is 0.
		 */
		RGBClass Color;
		int ColorIndex;
		double ColorAccum;

		/*
		 * Gas behavior controls.
		 */
		Coord GasDrift;
		Vector3 GasVelocity;

		/// Unused
		Coord UnusedCoord1;

		/*
		 * The speed at which the particle moves.
		 */
		float Speed;

		/*
		 * Fire behavior controls.
		 */
		Coord FireTarget;
		Coord FireOrigin;
		Coord FireMoveDelta;

		/*
		 * Normalized movement direction vector.
		 */
		Vector3 MovementDirection;

		/*
		**	Particle position as a float vector, used by railgun
		**	for extra precision.
		*/
		Vector3 PrecisePosition;

		/*
		 * The system this particle belongs to.
		 */
		ParticleSystemClass * System;

		/*
		 * Existence state machine
		 */
		unsigned short RemainingEC;
		unsigned short RemainingDC;
		char StateAIAdvance;
		bool IsFireBelowGround;
		char StateAI;

		/*
		 * This specifies how faded this particle is drawn (0, 25, 50 or 75 percent
		 * translucent). It starts out at the particle type's translucency and steps up as
		 * the particle ages, and it is only honored at the highest detail level.
		 */
		signed char Translucency;

		/// Unused
		bool WasSaved;

		/*
		 * If this particle has finished and should be removed from the game, then this flag
		 * will be true. It is set when the particle's lifetime runs out or its behavior
		 * decides it is done, and the system deletes it rather than moving it that frame.
		 */
		bool IsToDie;

	private:
		/*
		 * These are the X and Y offsets, expressed in leptons, that the prevailing wind
		 * drifts a smoke particle by. The wind direction indexes them, and the particle
		 * type's wind effect decides how often the offset is applied.
		 */
		static LEPTON SmokeWindX[FACING_COUNT];
		static LEPTON SmokeWindY[FACING_COUNT];

		/*
		 * These are the X and Y offsets, expressed in leptons, that the prevailing wind
		 * drifts a gas cloud by. Unlike the smoke tables, these are applied every frame
		 * and scaled by the particle type's wind effect.
		 */
		static LEPTON GasWindX[FACING_COUNT];
		static LEPTON GasWindY[FACING_COUNT];
};
