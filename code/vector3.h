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

/* $Header: /Commando/Code/wwmath/vector3.h 40    5/11/01 7:11p Jani_p $ */
/***********************************************************************************************
 ***                  Confidential - Westwood Studios                                        ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Westwood 3D                                                  *
 *                                                                                             *
 *                    File Name : VECTOR3.H                                                    *
 *                                                                                             *
 *                   Programmer : Greg Hjelstrom                                               *
 *                                                                                             *
 *                   Start Date : 02/24/97                                                     *
 *                                                                                             *
 *                  Last Update : February 24, 1997 [GH]                                       *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   Scalar Division Operator -- Divide a vector by a scalar                                   *
 *   Scalar Multiply Operator -- Multiply a vector by a scalar                                 *
 *   Vector Addition Operator -- Add two vectors                                               *
 *   Vector Subtraction Operator -- Subract two vectors                                        *
 *   Vector Inner Product Operator -- Compute the inner or dot product                         *
 *   Vector Equality Operator -- Determine if two vectors are identical                        *
 *   Vector Inequality Operator -- Determine if two vectors are identical                      *
 *   Equal_Within_Epsilon -- Determine if two vectors are identical within                     *
 *   Cross_Product -- compute the cross product of two vectors                                 *
 *   Vector3::Normalize -- Normalizes the vector.                                              *
 *   Vector3::Length -- Returns the length of the vector                                       *
 *   Vector3::Length2 -- Returns the square of the length of the vector                        *
 *   Vector3::Quick_Length -- returns a quick approximation of the length                      *
 *   Swap -- swap two Vector3's                                                                *
 *   Lerp -- linearly interpolate two Vector3's by an interpolation factor.                    *
 *   Lerp -- linearly interpolate two Vector3's without return-by-value                        *
 *   Vector3::Add -- Add two vector3's without return-by-value                                 *
 *   Vector3::Subtract -- Subtract two vector3's without return-by-value                       *
 *   Vector3::Update_Min -- sets each component of the vector to the min of this and a         *
 *   Vector3::Update_Max -- Sets each component of the vector to the max of this and a         *
 *   Vector3::Scale -- scale this vector by 3 independent scale factors                        *
 *   Vector3::Rotate_X -- rotates this vector around the X axis                                *
 *   Vector3::Rotate_X -- Rotates this vector around the x axis                                *
 *   Vector3::Rotate_Y -- Rotates this vector around the y axis                                *
 *   Vector3::Rotate_Y -- Rotates this vector around the Y axis                                *
 *   Vector3::Rotate_Z -- Rotates this vector around the Z axis                                *
 *   Vector3::Rotate_Z -- Rotates this vector around the Z axis                                *
 *   Vector3::Is_Valid -- Verifies that each component of this vector is a valid float         *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "visualc.h"

#include <cassert>
#include <cmath>

/*
**	Vector3 - 3-Dimensional Vectors
*/
class Vector3
{
public:
	float X;
	float Y;
	float Z;

	// Constructors
	inline Vector3(void) {};
	inline Vector3(float x, float y, float z) { X = x; Y = y; Z = z; }

	// Assignment
	inline void	Set(float x, float y, float z) { X = x; Y = y; Z = z; }
	inline void	Set(const Vector3 & that) { X = that.X; Y = that.Y; Z = that.Z; }

	//Vector3 & operator = (const Vector3 v);

	// Array access
	float & operator [](int i) { return((&X)[i]); }
	const float operator [](int i) const { return((&X)[i]); }

	// normalize, compute length
	void Normalize(void);
	inline float Length(void) const;
	inline float Length2(void) const;

	// unary operators
	inline Vector3 operator-() const { return(Vector3(-X,-Y,-Z)); }
	inline Vector3 operator+() const { return(*this); }

	inline Vector3 & operator += (const Vector3 & v) { X += v.X; Y += v.Y; Z += v.Z; return(*this); }
	inline Vector3 & operator -= (const Vector3 & v) { X -= v.X; Y -= v.Y; Z -= v.Z; return(*this); }
	inline Vector3 & operator *= (float k) { X=X*k; Y=Y*k; Z=Z*k; return(*this); }
	inline Vector3 & operator /= (float k) { X=X/k; Y=Y/k; Z=Z/k; return(*this); }

	// scalar multiplication, division
	inline friend Vector3 operator * (const Vector3 &a,double k);
	inline friend Vector3 operator * (float k,const Vector3 &a);
	inline friend Vector3 operator / (const Vector3 &a,double k);

	// vector addition,subtraction
	inline friend Vector3 operator + (const Vector3 &a,const Vector3 &b);
	inline friend Vector3 operator - (const Vector3 &a,const Vector3 &b);

	// Carries the vector to or from a save game.
	template<typename S>
	void Serialize(S & stream)
	{
		stream.Serialize(X);
		stream.Serialize(Y);
		stream.Serialize(Z);
	}

	// cross product / outer product
	static inline Vector3 Cross_Product(const Vector3 &a,const Vector3 &b);
	static inline void Cross_Product(const Vector3 &a,const Vector3 &b,Vector3 * result);
	static inline float Cross_Product_X(const Vector3 &a,const Vector3 &b);
	static inline float Cross_Product_Y(const Vector3 &a,const Vector3 &b);
	static inline float Cross_Product_Z(const Vector3 &a,const Vector3 &b);


};

/**************************************************************************
 * Scalar Multiply Operator -- Multiply a vector by a scalar              *
 *                                                                        *
 * INPUT:                                                                 *
 *                                                                        *
 * OUTPUT:                                                                *
 *                                                                        *
 * WARNINGS:                                                              *
 *                                                                        *
 * HISTORY:                                                               *
 *   02/24/1997 GH  : Created.                                            *
 *========================================================================*/
inline Vector3 operator * (const Vector3 &a,double k)
{
	return(Vector3((a.X * k),(a.Y * k),(a.Z * k)));
}

inline Vector3 operator * (float k, const Vector3 &a)
{
	return(Vector3((a.X * k),(a.Y * k),(a.Z * k)));
}

/**************************************************************************
 * Scalar Division Operator -- Divide a vector by a scalar                *
 *                                                                        *
 * INPUT:                                                                 *
 *                                                                        *
 * OUTPUT:                                                                *
 *                                                                        *
 * WARNINGS:                                                              *
 *                                                                        *
 * HISTORY:                                                               *
 *========================================================================*/
inline Vector3 operator / (const Vector3 &a,double k)
{
	return(Vector3((a.X / k),(a.Y / k),(a.Z / k)));
}

/**************************************************************************
 * Vector Addition Operator -- Add two vectors                            *
 *                                                                        *
 * INPUT:                                                                 *
 *                                                                        *
 * OUTPUT:                                                                *
 *                                                                        *
 * WARNINGS:                                                              *
 *                                                                        *
 * HISTORY:                                                               *
 *   02/24/1997 GH  : Created.                                            *
 *========================================================================*/
inline Vector3 operator + (const Vector3 &a,const Vector3 &b)
{
	return(Vector3(
		a.X+b.X,
		a.Y+b.Y,
		a.Z+b.Z)
	);
}

/**************************************************************************
 * Vector Subtraction Operator -- Subract two vectors                     *
 *                                                                        *
 * INPUT:                                                                 *
 *                                                                        *
 * OUTPUT:                                                                *
 *                                                                        *
 * WARNINGS:                                                              *
 *                                                                        *
 * HISTORY:                                                               *
 *   02/24/1997 GH  : Created.                                            *
 *========================================================================*/
inline Vector3 operator - (const Vector3 &a,const Vector3 &b)
{
	return(Vector3(
		a.X-b.X,
		a.Y-b.Y,
		a.Z-b.Z)
	);
}

/**************************************************************************
 * Vector Inner Product -- Compute the inner or dot product of two vector *
 *                                                                        *
 * INPUT:                                                                 *
 *                                                                        *
 * OUTPUT:                                                                *
 *                                                                        *
 * WARNINGS:                                                              *
 *                                                                        *
 * HISTORY:                                                               *
 *========================================================================*/
inline float operator * (const Vector3 &a,const Vector3 &b)
{
	return	(float)((double)a.X*(double)b.X +
			 (double)a.Y*(double)b.Y +
			 (double)a.Z*(double)b.Z);
}

/**************************************************************************
 * Cross_Product -- compute the cross product of two vectors              *
 *                                                                        *
 * INPUT:                                                                 *
 *                                                                        *
 * OUTPUT:                                                                *
 *                                                                        *
 * WARNINGS:                                                              *
 *                                                                        *
 * HISTORY:                                                               *
 *========================================================================*/
inline Vector3 Vector3::Cross_Product(const Vector3 &a,const Vector3 &b)
{
	return(Vector3(
		(a.Y * b.Z - a.Z * b.Y),
		(a.Z * b.X - a.X * b.Z),
		(a.X * b.Y - a.Y * b.X))
	);
}

inline void Vector3::Cross_Product(const Vector3 &a,const Vector3 &b,Vector3 * set_result)
{
	assert(set_result != &a);
	set_result->X = (a.Y * b.Z - a.Z * b.Y);
	set_result->Y = (a.Z * b.X - a.X * b.Z);
	set_result->Z = (a.X * b.Y - a.Y * b.X);
}

inline float Vector3::Cross_Product_X(const Vector3 &a,const Vector3 &b)
{
	return(a.Y * b.Z - a.Z * b.Y);
}

inline float Vector3::Cross_Product_Y(const Vector3 &a,const Vector3 &b)
{
	return(a.Z * b.X - a.X * b.Z);
}

inline float Vector3::Cross_Product_Z(const Vector3 &a,const Vector3 &b)
{
	return(a.X * b.Y - a.Y * b.X);
}

/**************************************************************************
 * Vector3::Normalize -- Normalizes the vector.                           *
 *                                                                        *
 * INPUT:                                                                 *
 *                                                                        *
 * OUTPUT:                                                                *
 *                                                                        *
 * WARNINGS:                                                              *
 *                                                                        *
 * HISTORY:                                                               *
 *========================================================================*/
inline void Vector3::Normalize(void)
{
	float len = Length();
	if (len != 0.0) {
		X /= len;
		Y /= len;
		Z /= len;
	}
}

inline Vector3 Normalize(const Vector3 & vec)
{
	float len = vec.Length();
	if (len != 0.0) {
		return(vec / len);
	}
	return(vec);
}

/**************************************************************************
 * Vector3::Length -- Returns the length of the vector                    *
 *                                                                        *
 * INPUT:                                                                 *
 *                                                                        *
 * OUTPUT:                                                                *
 *                                                                        *
 * WARNINGS:                                                              *
 *                                                                        *
 * HISTORY:                                                               *
 *========================================================================*/
inline float Vector3::Length(void) const
{
	return((float)std::sqrt((double)Length2()));
}

/**************************************************************************
 * Vector3::Length2 -- Returns the square of the length of the vector     *
 *                                                                        *
 * INPUT:                                                                 *
 *                                                                        *
 * OUTPUT:                                                                *
 *                                                                        *
 * WARNINGS:                                                              *
 *                                                                        *
 * HISTORY:                                                               *
 *========================================================================*/
inline float Vector3::Length2(void) const
{
	return((float)((double)X*(double)X + (double)Y*(double)Y + (double)Z*(double)Z));
}
