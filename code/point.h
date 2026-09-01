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

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name: Command & Conquer                                             *
 *                                                                                             *
 *                      Archive: /Sun/Point.h                                                  *
 *                                                                                             *
 *                       Author: Joe_b                                                         *
 *                                                                                             *
 *                      Modtime: 2/02/98 10:09a                                                *
 *                                                                                             *
 *                     Revision: 24                                                            *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include <cmath>


template<class T> class TRect;

/***********************************************************************************************
**	This class describes a point in 2 dimensional space using arbitrary
**	components. The interpretation of which is outside the scope
**	of this class. This class is the successor to the old style COORDINATE
**	and CELL types but also serves anywhere an X and Y value are treated
**	as a logical object (e.g., pixel location).
*/
template<class T>
class TPoint2D {
	public:
		TPoint2D(void) = default;
		constexpr TPoint2D(T x, T y) : X(x), Y(y) {}

		// Equality comparison operators.
		[[nodiscard]] constexpr bool operator == (TPoint2D<T> const & rvalue) const = default;

		// Addition and subtraction operators.
		constexpr TPoint2D<T> & operator += (TPoint2D<T> const & rvalue) {X += rvalue.X;Y += rvalue.Y;return(*this);}
		constexpr TPoint2D<T> & operator -= (TPoint2D<T> const & rvalue) {X -= rvalue.X;Y -= rvalue.Y;return(*this);}
		[[nodiscard]] constexpr TPoint2D<T> const operator - (TPoint2D<T> const & rvalue) const {return(TPoint2D<T>(T(X - rvalue.X), T(Y - rvalue.Y)));}
		[[nodiscard]] constexpr TPoint2D<T> const operator + (TPoint2D<T> const & rvalue) const {return(TPoint2D<T>(T(X + rvalue.X), T(Y + rvalue.Y)));}

		// Scalar multiplication and division.
		[[nodiscard]] constexpr TPoint2D<T> const operator * (T rvalue) const {return(TPoint2D<T>(T(X * rvalue), T(Y * rvalue)));}
		constexpr TPoint2D<T> & operator *= (T rvalue) {X *= rvalue; Y *= rvalue;return(*this);}
		[[nodiscard]] constexpr TPoint2D<T> const operator / (T rvalue) const {if (rvalue == T(0)) return(TPoint2D<T>(0,0));return(TPoint2D<T>(T(X / rvalue), T(Y / rvalue)));}
		constexpr TPoint2D<T> & operator /= (T rvalue) {if (rvalue != T(0)) {X /= rvalue;Y /= rvalue;}return(*this);}

		// Component-wise product and dot product.
		[[nodiscard]] constexpr TPoint2D<T> const operator * (TPoint2D<T> const & rvalue) const {return(TPoint2D<T>(T(X * rvalue.X), T(Y * rvalue.Y)));}
		[[nodiscard]] constexpr T Dot_Product(TPoint2D<T> const & rvalue) const {return((T(X * rvalue.X + Y * rvalue.Y)));}

		// Negation operator -- simple and effective
		[[nodiscard]] constexpr TPoint2D<T> const operator - (void) const {return(TPoint2D<T>(T(-X), T(-Y)));}

		// Vector support functions.
		//T Length(void) const {return(T(sqrt(double(X*X + Y*Y))));}
		[[nodiscard]] T Length(void) const {return(T(std::sqrt((double)X*(double)X + (double)Y*(double)Y)));}
		[[nodiscard]] TPoint2D<T> const Normalize(void) const {
			double len = std::sqrt((double)(X*X + Y*Y));
			if (len != 0.0) {
				return(TPoint2D<T>((T)(X / len), (T)(Y / len)));
			} else {
				return(*this);
			}
		}

		// Find distance between points.
		T Distance_To(TPoint2D<T> const & point) const {return((*this - point).Length());}

		// Carries the point to or from a save game.
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


/***********************************************************************************************
**	This typedef provides an uncluttered type name for use by simple integer points.
*/
class Point2D : public TPoint2D<int>
{
	public:
		Point2D(void) = default;
		constexpr Point2D(int x, int y) : TPoint2D<int>(x, y) {}
		constexpr Point2D(TPoint2D<int> const & rvalue) : TPoint2D<int>(rvalue) {}

		constexpr Point2D & operator +=(Point2D const & rvalue) {X += rvalue.X;Y += rvalue.Y;return(*this);}
		constexpr Point2D & operator -= (Point2D const & rvalue) {X -= rvalue.X;Y -= rvalue.Y;return(*this);}
		[[nodiscard]] constexpr Point2D const operator - (Point2D const & rvalue) const {return(Point2D(int(X - rvalue.X), int(Y - rvalue.Y)));}
		[[nodiscard]] constexpr Point2D const operator + (Point2D const & rvalue) const {return(Point2D(int(X + rvalue.X), int(Y + rvalue.Y)));}
};


template<class T>
T Distance(TPoint2D<T> const & point1, TPoint2D<T> const & point2)
{
	return((point1 - point2).Length());
}


/***********************************************************************************************
**	This describes a point in 3 dimensional space using arbitrary
**	components. This is the successor to the COORDINATE type for those
**	times when height (Z axis) needs to be tracked.
**
**	Notice that it is NOT implemented as a virtually derived class. This
**	is for efficiency reasons. This class chooses to be smaller and faster at the
**	expense of polymorphism. However, since it is publicly derived, inheritance is
**	the next best thing.
*/
template<class T>
class TPoint3D : public TPoint2D<T> {
		typedef TPoint2D<T> BASECLASS;

	public:
		using BASECLASS::X;
		using BASECLASS::Y;

		TPoint3D(void) = default;
		constexpr TPoint3D(T x, T y, T z) : BASECLASS(x, y), Z(z) {}
		constexpr TPoint3D(BASECLASS const & rvalue, T z /*= 0*/) : BASECLASS(rvalue), Z(z) {}

		// Equality comparison operators.
		[[nodiscard]] constexpr bool operator == (TPoint3D<T> const & rvalue) const = default;

		// Addition and subtraction operators.
		constexpr TPoint3D<T> & operator += (TPoint3D<T> const & rvalue) {X += rvalue.X;Y += rvalue.Y;Z += rvalue.Z;return(*this);}
		constexpr TPoint3D<T> & operator += (TPoint2D<T> const & rvalue) {BASECLASS::operator += (rvalue);return(*this);}
		constexpr TPoint3D<T> & operator -= (TPoint3D<T> const & rvalue) {X -= rvalue.X;Y -= rvalue.Y;Z -= rvalue.Z;return(*this);}
		constexpr TPoint3D<T> & operator -= (TPoint2D<T> const & rvalue) {BASECLASS::operator -= (rvalue);return(*this);}
		[[nodiscard]] constexpr TPoint3D<T> const operator - (TPoint3D<T> const & rvalue) const {return(TPoint3D<T>(X - rvalue.X, Y - rvalue.Y, Z - rvalue.Z));}
		[[nodiscard]] constexpr TPoint3D<T> const operator - (TPoint2D<T> const & rvalue) const {return(TPoint3D<T>(X - rvalue.X, Y - rvalue.Y, Z));}
		[[nodiscard]] constexpr TPoint3D<T> const operator + (TPoint3D<T> const & rvalue) const {return(TPoint3D<T>(X + rvalue.X, Y + rvalue.Y, Z + rvalue.Z));}
		[[nodiscard]] constexpr TPoint3D<T> const operator + (TPoint2D<T> const & rvalue) const {return(TPoint3D<T>(X + rvalue.X, Y + rvalue.Y, Z));}

		// Scalar multiplication and division.
		[[nodiscard]] constexpr TPoint3D<T> const operator * (T rvalue) const {return(TPoint3D<T>(X * rvalue, Y * rvalue, Z * rvalue));}
		constexpr TPoint3D<T> & operator *= (T rvalue) {X *= rvalue;Y *= rvalue;Z *= rvalue;return(*this);}
		[[nodiscard]] constexpr TPoint3D<T> const operator / (T rvalue) const {if (rvalue == T(0)) return(TPoint3D<T>(0,0,0));return(TPoint3D<T>(X / rvalue, Y / rvalue, Z / rvalue));}
		constexpr TPoint3D<T> & operator /= (T rvalue) {if (rvalue != T(0)) {X /= rvalue;Y /= rvalue;Z /= rvalue;}return(*this);}

		// Dot and cross product.
		[[nodiscard]] constexpr TPoint3D<T> const operator * (TPoint3D<T> const & rvalue) const {return(TPoint3D<T>(X * rvalue.X, Y * rvalue.Y, Z * rvalue.Z));}
		[[nodiscard]] constexpr T Dot_Product(TPoint3D<T> const & rvalue) const {return(T(X * rvalue.X + Y * rvalue.Y + Z * rvalue.Z));}
		[[nodiscard]] constexpr TPoint3D<T> const Cross_Product(TPoint3D<T> const & rvalue) const {return(TPoint3D<T>(Y * rvalue.Z - Z * rvalue.Y, Z * rvalue.X - X * rvalue.Z, X * rvalue.Y - Y * rvalue.X));}

		// Negation operator -- simple and effective
		[[nodiscard]] constexpr TPoint3D<T> const operator - (void) const {return(TPoint3D<T>(-X, -Y, -Z));}

		// Vector support functions.
		//T Length(void) const {return(T(sqrt(double(X*X + Y*Y + Z*Z))));}
		[[nodiscard]] T Length(void) const {return(T(std::sqrt((double)X*(double)X + (double)Y*(double)Y + (double)Z*(double)Z)));}
		[[nodiscard]] TPoint3D<T> const Normalize(void) const {
			double len = std::sqrt((double)(X*X + Y*Y + Z*Z));
			if (len != 0.0) {
				return(TPoint3D<T>((T)(X / len), (T)(Y / len), (T)(Z / len)));
			} else {
				return(*this);
			}
		}

		T Distance_To(TPoint3D<T> const & point) const {return((*this - point).Length());}
		T Distance_To(TPoint2D<T> const & point) const {return(BASECLASS::Distance_To(point));}

	public:

		// Carries the point to or from a save game.
		template<typename S>
		void Serialize(S & stream)
		{
			BASECLASS::Serialize(stream);
			stream.Serialize(Z);
		}

		/*
		**	The Z component of this point.
		*/
		T Z = T(0);
};


/***********************************************************************************************
**	This typedef provides a simple uncluttered type name for use by
**	integer 3D points.
*/
typedef TPoint3D<int> Point3D;

template<class T>
TPoint3D<T> const Cross_Product(TPoint3D<T> const & lvalue, TPoint3D<T> const & rvalue)
{
	return(lvalue.Cross_Product(rvalue));
}
