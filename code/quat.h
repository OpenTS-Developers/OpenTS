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

#include "vector3.h"

#include <cmath>

class Matrix3D;

class Quaternion
{
	public:
		//Quaternion(void) {}
		Quaternion(const Vector3 & axis,float angle);

		Quaternion(float x = 0.0, float y = 0.0, float z = 0.0, float w = 1.0);
		Quaternion operator=(const Quaternion & q);

		void Set(float a = 0.0, float b = 0.0, float c = 0.0, float d = 1.0);
		void Make_Identity(void) { Set(); };

		void Scale(float s);

		float & operator [](int i);
		const float operator [](int i) const;

		Quaternion operator-() const { return(Quaternion(-X,-Y,-Z,-W)); }

		float Length(void) const { return((float)std::sqrt((double)Length2())); }
		float Length2(void) const { return(X * X + Y * Y + Z * Z + W * W); }

		// Every 3D rotation can be expressed by two different quaternions,  This
		// function makes the current quaternion convert itself to the representation
		// which is closer on the 4D unit-hypersphere to the given quaternion.
		Quaternion & Make_Closest(const Quaternion & q);

		void Normalize(void);

		void Invert(void);

		// Carries the quaternion to or from a save game.
		template<typename S>
		void Serialize(S & stream)
		{
			stream.Serialize(X);
			stream.Serialize(Y);
			stream.Serialize(Z);
			stream.Serialize(W);
		}

	public:
		float X;
		float Y;
		float Z;
		float W;
};


inline Quaternion::Quaternion(const Vector3 & axis,float angle)
{
	float s = std::sin((double)(angle/2));
	float c = std::cos((double)(angle/2));
	X = s * axis[0];
	Y = s * axis[1];
	Z = s * axis[2];
	W = c;
}

Quaternion operator * (const Quaternion & a,const Quaternion & b);
Quaternion operator / (const Quaternion & a,const Quaternion & b);

Quaternion Inverse(const Quaternion & q);
Quaternion Conjugate(const Quaternion & q);


// This function computes a quaternion based on an axis
// (defined by the given Vector a) and an angle about
// which to rotate.  The angle is expressed in radians.
Quaternion Axis_To_Quat(const Vector3 &a, float angle);

// Pass the x and y coordinates of the last and current position
// of the mouse, scaled so they are from -1.0 to 1.0
// The quaternion is the computed as the rotation of a trackball
// between the two points projected onto a sphere.  This can
// be used to implement an intuitive viewing control system.
Quaternion Trackball(float x0, float y0, float x1, float y1, float size);

// Spherical Linear interpolation of quaternions
Quaternion Slerp(const Quaternion &a, const Quaternion &b, float alpha);

// Convert a rotation matrix into a quaternion
Quaternion Build_Quaternion(Matrix3D const &);

// Convert a quaternion into a rotation matrix
Matrix3D Build_Matrix3D(Quaternion const &);

inline void Quaternion::Invert(void)
{
	X = -X;
	Y = -Y;
	Z = -Z;
	W = -W;
}
