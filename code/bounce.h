/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include "coord.h"
#include "matrix3d.h"
#include "quat.h"
#include "vector3.h"


enum BounceResultType {
	BOUNCE_MOVING,
	BOUNCE_IMPACT,
	BOUNCE_SETTLED,
};


class BounceClass
{
	public:
		BounceClass(void) :
			Elasticity(0),
			Gravity(0),
			MaxVelocity(0),
			MyCoord(),
			Velocity(),
			Rotation(0.0, 0.0, 0.0, 1.0),
			AngularVelocity(0.0, 0.0, 0.0, 1.0)
		{

		}

		void Init(Coord const & coord, double elasticity, double, double min_speed, double max_speed);
		void Init(Coord const & coord, double elasticity, double gravity, double max_velocity, Vector3 const & velocity, double rotation);

		Coord Get_Bounce_Coord(void) const;
		Matrix3D Get_Matrix(void) const;
		double Get_Remaining_Motion(void) const;
		BounceResultType AI(void);

		// Carries the physics state to or from a save game.
		template<typename S>
		void Serialize(S & stream)
		{
			stream.Serialize(Elasticity);
			stream.Serialize(Gravity);
			stream.Serialize(MaxVelocity);
			stream.Serialize(MyCoord);
			stream.Serialize(Velocity);
			stream.Serialize(Rotation);
			stream.Serialize(AngularVelocity);
		}

	public:
		/*
		 * This is the fraction of its speed that the object keeps when it strikes a surface.
		 */
		double Elasticity;

		/*
		 * This is the amount subtracted from the vertical velocity every game frame.
		 */
		double Gravity;

		/*
		 * This is the fastest the object is allowed to travel. If zero, then the speed of
		 * the object is not limited.
		 */
		double MaxVelocity;

		/*
		 * This is the current position of the object, expressed in leptons.
		 */
		Vector3 MyCoord;

		/*
		 * This is the distance the object travels each game frame. An impact reflects it off
		 * the slope of the cell landed on and scales it by the Elasticity.
		 */
		Vector3 Velocity;

		/*
		 * This is the orientation the object has tumbled to. The draw code turns it into a
		 * transformation matrix so that debris is rendered at whatever angle it has reached.
		 */
		Quaternion Rotation;

		/*
		 * This is the tumble applied to the Rotation every game frame. The spin axis is picked
		 * at random when the object is launched, so two pieces thrown alike still spin
		 * differently.
		 */
		Quaternion AngularVelocity;
};
