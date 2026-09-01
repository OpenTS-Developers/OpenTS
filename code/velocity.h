/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include "face.h"

#include <cmath>


template <typename T>
class TVelocity2D
{
	public:
		TVelocity2D(void) = default;
		TVelocity2D(T x, T y)
		{
			X = x;
			Y = y;
		}

		T Speed(void) const
		{
			return(std::sqrt(X * X + Y * Y));
		}

		/*
		 * Repoints the velocity to the given heading while preserving its speed.
		 * A zero vector is nudged eastward first so the result is well defined.
		 */
		void Set_Yaw(DirType const & yaw)
		{
			Nonzero_Check();
			T horizontal_speed = Speed();
			X = horizontal_speed * std::cos(yaw.As_Radian());
			Y = -horizontal_speed * std::sin(yaw.As_Radian());
		}

		/*
		 * Turns the heading toward that of another velocity, but by no more than
		 * the given rate.
		 */
		void Yaw(TVelocity2D const & towards, DirType const & rate)
		{
			DirType current_yaw = Get_Yaw();
			DirType desired_yaw = towards.Get_Yaw();
			current_yaw.Turn(desired_yaw, rate);
			Set_Yaw(current_yaw);
		}

		DirType Get_Yaw(void) const
		{
			return(std::atan2((double)-Y, (double)X));
		}

		void Nonzero_Check(void)
		{
			if (X == 0 && Y == 0) {
				X = 100;
			}
		}

		// Carries the velocity to or from a save game.
		template<typename S>
		void Serialize(S & stream)
		{
			stream.Serialize(X);
			stream.Serialize(Y);
		}

	public:
		T X = T(0);
		T Y = T(0);
};


/*
 * A velocity vector in world space. X and Y lie in the map plane and Z points
 * up. Yaw follows the Get_Yaw/Set_Yaw convention -- yaw zero points toward +X
 * and a positive yaw swings toward -Y. Pitch reuses the compass dial as an
 * elevation: DIR_E is level flight, DIR_N is straight up and DIR_S is straight
 * down (a consequence of DirType::As_Radian mapping E to 0 and N to +pi/2).
 */
template <typename T>
class TVelocity3D : public TVelocity2D<T>
{
		typedef TVelocity2D<T> BASECLASS;

	public:
		using BASECLASS::X;
		using BASECLASS::Y;

		TVelocity3D(void) = default;
		TVelocity3D(const TVelocity3D & velocity) : BASECLASS(velocity), Z(velocity.Z) {}
		TVelocity3D(Dir256 pitch, Dir256 yaw, T magnitude);
		TVelocity3D(DirType const & yaw, DirType const & pitch, T magnitude);
		TVelocity3D(double angle, T magnitude);
		TVelocity3D(T x, T y, T z) : BASECLASS(TVelocity2D<T>(x, y)), Z(z) {}

		void Set(T x, T y, T z)
		{
			X = x;
			Y = y;
			Z = z;
		}

		T Horizontal_Speed(void) const { return(std::sqrt(X * X + Y * Y)); }

		T Speed(void) const { return(std::sqrt(X * X + Y * Y + Z * Z)); }

		/*
		 * Repoints the velocity to the given pitch, preserving the total speed
		 * and the heading: the horizontal components are rescaled from the old
		 * pitch cosine to the new one and Z is rebuilt from the new pitch sine.
		 */
		void Set_Pitch(DirType const & pitch)
		{
			double current_pitch = Get_Pitch().As_Radian();
			T speed = Speed();
			if (current_pitch != 0) {
				X /= std::cos(current_pitch);
				Y /= std::cos(current_pitch);
			}
			X *= std::cos(pitch.As_Radian());
			Y *= std::cos(pitch.As_Radian());
			Z = speed * std::sin(pitch.As_Radian());
		}

		/*
		 * Turns the pitch toward the desired angle, but by no more than the
		 * given rate. The missile autopilot uses this to limit how sharply a
		 * projectile may climb or dive in one frame (see Projectile_Motion).
		 */
		void Pitch(DirType const & towards, DirType const & rate)
		{
			DirType current_pitch = Get_Pitch();
			current_pitch.Turn(towards, rate);
			Set_Pitch(current_pitch);
		}

		/*
		 * Rescales the velocity to the given speed, keeping its direction. A
		 * zero vector is nudged eastward first so that the result is well
		 * defined.
		 */
		void Set_Speed(T speed)
		{
			Nonzero_Check();
			T speed_ratio = speed / Speed();
			X *= speed_ratio;
			Y *= speed_ratio;
			Z *= speed_ratio;
		}

		DirType Get_Pitch(void) const
		{
			return(std::atan2((double)Z, Horizontal_Speed()));
		}

		void Nonzero_Check(void)
		{
			if (X == 0 && Y == 0 && Z == 0) {
				X = 100;
			}
		}

		TVelocity3D & operator += (const TVelocity3D & velocity)
		{
			X += velocity.X;
			Y += velocity.Y;
			Z += velocity.Z;
			return(*this);
		}

		TVelocity3D & operator *= (double scale)
		{
			X *= scale;
			Y *= scale;
			Z *= scale;
			return(*this);
		}

		// Carries the velocity to or from a save game.
		template<typename S>
		void Serialize(S & stream)
		{
			BASECLASS::Serialize(stream);
			stream.Serialize(Z);
		}

	public:
		T Z = T(0);
};


template <typename T>
TVelocity3D<T>::TVelocity3D(Dir256 pitch, Dir256 yaw, T magnitude)
	: BASECLASS(
		TVelocity2D<T>(
			std::cos(DirType(pitch).As_Radian()) * std::cos(DirType(yaw).As_Radian()) * magnitude,
			std::cos(DirType(pitch).As_Radian()) * std::sin(DirType(yaw).As_Radian()) * magnitude)),
		Z(std::sin(DirType(pitch).As_Radian()) * magnitude)
{
}


/*
 * Builds a velocity from a heading and a pitch (yaw FIRST in this form -- see
 * the note on the Dir256 constructor above).
 */
template <typename T>
TVelocity3D<T>::TVelocity3D(DirType const & yaw, DirType const & pitch, T magnitude)
	: BASECLASS(
		TVelocity2D<T>(
			std::cos(yaw.As_Radian()) * std::cos(pitch.As_Radian()) * magnitude,
			std::sin(yaw.As_Radian()) * std::cos(pitch.As_Radian()) * magnitude)),
		Z(std::sin(pitch.As_Radian()) * magnitude)
{
}


/*
 * Builds a level velocity from a math angle in radians. Note that this form
 * negates the sine, matching the Get_Yaw/Set_Yaw convention (unlike the
 * direction constructors above).
 */
template <typename T>
TVelocity3D<T>::TVelocity3D(double angle, T magnitude)
{
	X = magnitude * std::cos(angle);
	Y = -(magnitude * std::sin(angle));
	Z = 0;
}
