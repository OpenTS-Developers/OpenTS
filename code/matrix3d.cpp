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

/* $Header: /Commando/Code/Tools/max2w3d/matrix3d.cpp 39    2/03/00 4:55p Jason_a $ */
/***********************************************************************************************
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Voxel Technology                                             *
 *                                                                                             *
 *                    File Name : MATRIX3D.CPP                                                 *
 *                                                                                             *
 *                   Programmer : Greg Hjelstrom                                               *
 *                                                                                             *
 *                   Start Date : 02/24/97                                                     *
 *                                                                                             *
 *                  Last Update : February 28, 1997 [GH]                                       *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   Matrix3D::Set_Rotation -- Sets the rotation part of the matrix                            *
 *   Matrix3D::Set_Rotation -- Sets the rotation part of the matrix                            *
 *   Matrix3D::Set -- Init a matrix3D from a matrix3 and a position                            *
 *   Matrix3D::Set -- Init a matrix3D from a quaternion and a position                         *
 *   Matrix3D::Get_X_Rotation -- approximates the rotation about the X axis                    *
 *   Matrix3D::Get_Y_Rotation -- approximates the rotation about the Y axis                    *
 *   Matrix3D::Get_Z_Rotation -- approximates the rotation about the Z axis                    *
 *   Matrix3D::Multiply -- matrix multiplication without temporaries.                          *
 *   Matrix3D::Inverse_Rotate_Vector -- rotates a vector by the inverse of the 3x3 sub-matrix  *
 *   Matrix3D::Transform_Min_Max_AABox -- compute transformed axis-aligned box                 *
 *   Matrix3D::Transform_Center_Extent_AABox -- compute transformed axis-aligned box           *
 *   Matrix3D::Get_Inverse -- calculate the inverse of this matrix                             *
 *   Matrix3D::Get_Orthogonal_Inverse -- Returns the inverse of the matrix                     *
 *   Matrix3D::Re_Orthogonalize -- makes this matrix orthogonal.                               *
 *   Matrix3D::Is_Orthogonal -- checks whether this matrix is orthogonal                       *
 *   Lerp - linearly interpolate matrices (orientation is slerped)                             *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "matrix3d.h"


#include <cstring>


/***********************************************************************************************
 * M3DC::Matrix3D -- Constructors for Matrix3D                                                 *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/24/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
Matrix3D::Matrix3D(float m[12])
{
	for (int i = 0; i < 12; i++) {
		((float *)this)[i] = m[i];
	}
}



/// <summary>
/// Constructs a matrix from its twelve elements.
/// The elements are given a row at a time, with the fourth element of each row carrying that
/// row's translation term.
/// </summary>
Matrix3D::Matrix3D
(
	float m11,float m12,float m13,float m14,
	float m21,float m22,float m23,float m24,
	float m31,float m32,float m33,float m34
)
{
	Row[0].Set(m11,m12,m13,m14);
	Row[1].Set(m21,m22,m23,m24);
	Row[2].Set(m31,m32,m33,m34);
}


/// <summary>
/// Constructs a matrix from a set of axes and a position.
/// The three vectors become the transform's axes and the fourth its origin. They are taken
/// as given, so a caller handing over axes that are not unit length or not perpendicular
/// will get a matrix that scales or shears.
/// </summary>
/// <param name="x">The X axis of the transform.</param>
/// <param name="y">The Y axis of the transform.</param>
/// <param name="z">The Z axis of the transform.</param>
/// <param name="pos">The position of the transform's origin.</param>
Matrix3D::Matrix3D
(
	const Vector3	&x,		// x-axis unit vector
	const Vector3	&y,		// y-axis unit vector
	const Vector3	&z,		// z-axis unit vector
	const Vector3	&pos	// position
)
{
	Row[0].X = x.X;
	Row[0].Y = x.Y;
	Row[0].Z = x.Z;
	Row[0].W = pos.X;

	Row[1].X = y.X;
	Row[1].Y = y.Y;
	Row[1].Z = y.Z;
	Row[1].W = pos.Y;

	Row[2].X = z.X;
	Row[2].Y = z.Y;
	Row[2].Z = z.Z;
	Row[2].W = pos.Z;
}


/// <summary>
/// Constructs a tilt about an axis lying in the XY plane.
/// The Z angle picks which horizontal axis to tilt about and the X angle says how far, so
/// this is the constructor to use for pitching something that already has a heading.
/// </summary>
/// <param name="zrot">The heading of the tilt axis, in radians.</param>
/// <param name="xrot">The angle to tilt by, in radians.</param>
Matrix3D::Matrix3D(float zrot, float xrot)
{
	float(&m)[3][4] = (float(&)[3][4])*this;

	m[0][0] = 1.0f;
	m[0][1] = 0.0f;
	m[0][2] = 0.0f;
	m[0][3] = 0.0f;

	m[1][0] = 0.0f;
	m[1][1] = 1.0f;
	m[1][2] = 0.0f;
	m[1][3] = 0.0f;

	m[2][0] = 0.0f;
	m[2][1] = 0.0f;
	m[2][2] = 1.0f;
	m[2][3] = 0.0f;

	Rotate_Z(zrot);
	Rotate_X(xrot);
	Rotate_Z(-zrot);
}


/// <summary>
/// Constructs a rotation about an arbitrary axis.
/// </summary>
/// <param name="axis">The unit vector to rotate about.</param>
/// <param name="angle">The angle to rotate by, in radians.</param>
Matrix3D::Matrix3D(const Vector3 & axis, float angle)
{
	Set(axis, angle);
}


/***********************************************************************************************
 * M3DC::Make_Identity -- Initializes the matrix to be the identity matrix                     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/24/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
void Matrix3D::Make_Identity(void)
{
	Row[0].Set(1,0,0,0);
	Row[1].Set(0,1,0,0);
	Row[2].Set(0,0,1,0);
}


/***********************************************************************************************
 * M3DC::Translate -- Post-Multiplies by a Translation Matrix                                  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/24/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
void Matrix3D::Translate(float x, float y, float z)
{
	double dx = x, dy = y, dz = z;

	Row[0].W += (float)(Row[0].X*dx + Row[0].Y*dy + Row[0].Z*dz);
	Row[1].W += (float)(Row[1].X*dx + Row[1].Y*dy + Row[1].Z*dz);
	Row[2].W += (float)(Row[2].X*dx + Row[2].Y*dy + Row[2].Z*dz);
}


/***********************************************************************************************
 * M3DC::Translate -- Post-Multiplies the matrix by a translation matrix                       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/24/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
void Matrix3D::Translate(Vector3 const & t)
{
	Translate_X(t[0]);
	Translate_Y(t[1]);
	Translate_Z(t[2]);
}


/***********************************************************************************************
 * M3DC::Translate_X -- Post-Multiplies the matrix by a translation matrix with X only         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/06/1998 NH  : Created.                                                                 *
 *=============================================================================================*/
void Matrix3D::Translate_X(float x)
{
	Row[0].W += (float)(Row[0].X*x);
	Row[1].W += (float)(Row[1].X*x);
	Row[2].W += (float)(Row[2].X*x);
}


/***********************************************************************************************
 * M3DC::Translate_Y -- Post-Multiplies the matrix by a translation matrix with Y only         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/06/1998 NH  : Created.                                                                 *
 *=============================================================================================*/
void Matrix3D::Translate_Y(float y)
{
	Row[0].W += (float)(Row[0].Y*y);
	Row[1].W += (float)(Row[1].Y*y);
	Row[2].W += (float)(Row[2].Y*y);
}


/***********************************************************************************************
 * M3DC::Translate_Z -- Post-Multiplies the matrix by a translation matrix with Z only         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/06/1998 NH  : Created.                                                                 *
 *=============================================================================================*/
void Matrix3D::Translate_Z(float z)
{
	Row[0].W += (float)(Row[0].Z*z);
	Row[1].W += (float)(Row[1].Z*z);
	Row[2].W += (float)(Row[2].Z*z);
}


/***********************************************************************************************
 * Matrix3D::Scale -- Scales each Axis                                                         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/07/2000 jga  : Created.                                                                *
 *=============================================================================================*/
// !!
// !! Use Scale methods with Extreme Caution
// !! The Matrix Inverse function, only works
// !! with Orthogonal Matrices, for optimization purposes
// !!
void Matrix3D::Scale(float scale)
{	// uniform scale all 3 axis
	// X
	Row[0].X *= scale;
	Row[1].X *= scale;
	Row[2].X *= scale;
	// Y
	Row[0].Y *= scale;
	Row[1].Y *= scale;
	Row[2].Y *= scale;
	// Z
	Row[0].Z *= scale;
	Row[1].Z *= scale;
	Row[2].Z *= scale;
}


/***********************************************************************************************
 * Matrix3D::Scale -- Scales each Axis                                                         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/07/2000 jga  : Created.                                                                *
 *=============================================================================================*/
// !!
// !! Use Scale methods with Extreme Caution
// !! The Matrix Inverse function, only works
// !! with Orthogonal Matrices, for optimization purposes
// !!
void Matrix3D::Scale(float x, float y, float z)
{ // separate input for each axis
	// X
	Row[0].X *= x;
	Row[1].X *= x;
	Row[2].X *= x;
	// Y
	Row[0].Y *= y;
	Row[1].Y *= y;
	Row[2].Y *= y;
	// Z
	Row[0].Z *= z;
	Row[1].Z *= z;
	Row[2].Z *= z;
}


/// <summary>
/// Scales the matrix along its X axis only.
/// </summary>
/// <param name="scale">The factor to scale the X axis by.</param>
/// <remarks>A scaled matrix is no longer orthogonal, so Orthogonal_Inverse will not invert
/// it.</remarks>
void Matrix3D::Scale_X(float scale)
{
	Row[0].X *= scale;
	Row[1].X *= scale;
	Row[2].X *= scale;
}


/// <summary>
/// Scales the matrix along its Y axis only.
/// </summary>
/// <param name="scale">The factor to scale the Y axis by.</param>
/// <remarks>A scaled matrix is no longer orthogonal, so Orthogonal_Inverse will not invert
/// it.</remarks>
void Matrix3D::Scale_Y(float scale)
{
	Row[0].Y *= scale;
	Row[1].Y *= scale;
	Row[2].Y *= scale;
}


/// <summary>
/// Scales the matrix along its Z axis only.
/// </summary>
/// <param name="scale">The factor to scale the Z axis by.</param>
/// <remarks>A scaled matrix is no longer orthogonal, so Orthogonal_Inverse will not invert
/// it.</remarks>
void Matrix3D::Scale_Z(float scale)
{
	Row[0].Z *= scale;
	Row[1].Z *= scale;
	Row[2].Z *= scale;
}


/// <summary>
/// Post-multiplies the matrix by a shear of the X axis.
/// The X axis is leaned toward the Y and Z axes by the amounts given, and the other two axes
/// are left where they are.
/// </summary>
/// <param name="y">How far to lean the X axis toward Y.</param>
/// <param name="z">How far to lean the X axis toward Z.</param>
/// <remarks>A sheared matrix is no longer orthogonal, so Orthogonal_Inverse will not invert
/// it.</remarks>
void Matrix3D::Shear_YZ(float y, float z)
{
	Row[0].X += y * Row[0].Y + z * Row[0].Z;
	Row[1].X += y * Row[1].Y + z * Row[1].Z;
	Row[2].X += y * Row[2].Y + z * Row[2].Z;
}


/// <summary>
/// Post-multiplies the matrix by a shear of the Z axis.
/// The Z axis is leaned toward the X and Y axes by the amounts given, and the other two axes
/// are left where they are.
/// </summary>
/// <param name="x">How far to lean the Z axis toward X.</param>
/// <param name="y">How far to lean the Z axis toward Y.</param>
/// <remarks>A sheared matrix is no longer orthogonal, so Orthogonal_Inverse will not invert
/// it.</remarks>
void Matrix3D::Shear_XY(float x, float y)
{
	Row[0].Z += x * Row[0].X + y * Row[0].Y;
	Row[1].Z += x * Row[1].X + y * Row[1].Y;
	Row[2].Z += x * Row[2].X + y * Row[2].Y;
}


/// <summary>
/// Post-multiplies the matrix by a shear of the Y axis.
/// The Y axis is leaned toward the X and Z axes by the amounts given, and the other two axes
/// are left where they are.
/// </summary>
/// <param name="x">How far to lean the Y axis toward X.</param>
/// <param name="z">How far to lean the Y axis toward Z.</param>
/// <remarks>A sheared matrix is no longer orthogonal, so Orthogonal_Inverse will not invert
/// it.</remarks>
void Matrix3D::Shear_XZ(float x, float z)
{
	Row[0].Y += x * Row[0].X + z * Row[0].Z;
	Row[1].Y += x * Row[1].X + z * Row[1].Z;
	Row[2].Y += x * Row[2].X + z * Row[2].Z;
}


/***********************************************************************************************
 * M3DC::Pre_Rotate_X -- Pre-multiplies the matrix by a rotation about X                       *
 *                                                                                             *
 * INPUT:                                                                                      *
 * theta - angle (in radians) to rotate                                                        *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/1/1999 NH  : Created.                                                                  *
 *=============================================================================================*/
void Matrix3D::Pre_Rotate_X(float theta)
{
	float s = std::sin(theta);
	float c = std::cos(theta);
	float ns = -s;

	float m10 = Row[1].X;
	float m11 = Row[1].Y;
	float m12 = Row[1].Z;
	float m13 = Row[1].W;

	float m20 = Row[2].X;
	float m21 = Row[2].Y;
	float m22 = Row[2].Z;
	float m23 = Row[2].W;

	Row[1].X = m20 * ns + m10 * c;
	Row[1].Y = m21 * ns + m11 * c;
	Row[1].Z = m22 * ns + m12 * c;
	Row[1].W = m23 * ns + m13 * c;

	Row[2].X = m20 * c + m10 * s;
	Row[2].Y = m21 * c + m11 * s;
	Row[2].Z = m22 * c + m12 * s;
	Row[2].W = m23 * c + m13 * s;
}


/***********************************************************************************************
 * M3DC::Pre_Rotate_Y -- Pre-multiplies the matrix by a rotation about Y                       *
 *                                                                                             *
 * INPUT:                                                                                      *
 * theta - angle (in radians) to rotate                                                        *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/1/1999 NH  : Created.                                                                  *
 *=============================================================================================*/
void Matrix3D::Pre_Rotate_Y(float theta)
{
	float s = std::sin(theta);
	float c = std::cos(theta);
	float ns = -s;

	float m00 = Row[0].X;
	float m01 = Row[0].Y;
	float m02 = Row[0].Z;
	float m03 = Row[0].W;

	float m20 = Row[2].X;
	float m21 = Row[2].Y;
	float m22 = Row[2].Z;
	float m23 = Row[2].W;

	Row[0].X = m20 * s + m00 * c;
	Row[0].Y = m21 * s + m01 * c;
	Row[0].Z = m22 * s + m02 * c;
	Row[0].W = m23 * s + m03 * c;

	Row[2].X = m00 * ns + m20 * c;
	Row[2].Y = m01 * ns + m21 * c;
	Row[2].Z = m02 * ns + m22 * c;
	Row[2].W = m03 * ns + m23 * c;
}


/***********************************************************************************************
 * M3DC::Pre_Rotate_Z -- Pre-multiplies the matrix by a rotation about Z                       *
 *                                                                                             *
 * INPUT:                                                                                      *
 * theta - angle (in radians) to rotate                                                        *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/1/1999 NH  : Created.                                                                  *
 *=============================================================================================*/
void Matrix3D::Pre_Rotate_Z(float theta)
{
	float s = std::sin(theta);
	float c = std::cos(theta);
	float ns = -s;

	float m00 = Row[0].X;
	float m01 = Row[0].Y;
	float m02 = Row[0].Z;
	float m03 = Row[0].W;

	float m10 = Row[1].X;
	float m11 = Row[1].Y;
	float m12 = Row[1].Z;
	float m13 = Row[1].W;

	Row[0].X = m10 * ns + m00 * c;
	Row[0].Y = m11 * ns + m01 * c;
	Row[0].Z = m12 * ns + m02 * c;
	Row[0].W = m13 * ns + m03 * c;

	Row[1].X = m10 * c + m00 * s;
	Row[1].Y = m11 * c + m01 * s;
	Row[1].Z = m12 * c + m02 * s;
	Row[1].W = m13 * c + m03 * s;
}


/// <summary>
/// Post-multiplies the matrix by a rotation about X.
/// </summary>
/// <param name="theta">The angle to rotate by, in radians.</param>
void Matrix3D::Rotate_X(float theta)
{
	double tmp1,tmp2;
	float s,c;

	s = (float)std::sin(theta);
	c = (float)std::cos(theta);

	tmp1 = Row[0].Y; tmp2 = Row[0].Z;
	Row[0].Y = (float)( c*tmp1 + s*tmp2);
	Row[0].Z = (float)(-s*tmp1 + c*tmp2);

	tmp1 = Row[1].Y; tmp2 = Row[1].Z;
	Row[1].Y = (float)( c*tmp1 + s*tmp2);
	Row[1].Z = (float)(-s*tmp1 + c*tmp2);

	tmp1 = Row[2].Y; tmp2 = Row[2].Z;
	Row[2].Y = (float)( c*tmp1 + s*tmp2);
	Row[2].Z = (float)(-s*tmp1 + c*tmp2);

}


/// <summary>
/// Post-multiplies the matrix by a rotation about X.
/// Use this routine when the sine and cosine of the angle are already in hand, as they are
/// when several matrices are turned by the same amount.
/// </summary>
/// <param name="s">The sine of the angle to rotate by.</param>
/// <param name="c">The cosine of the angle to rotate by.</param>
void Matrix3D::Rotate_X(float s,float c)
{
	double tmp1,tmp2;

	tmp1 = Row[0].Y; tmp2 = Row[0].Z;
	Row[0].Y = (float)( c*tmp1 + s*tmp2);
	Row[0].Z = (float)(-s*tmp1 + c*tmp2);

	tmp1 = Row[1].Y; tmp2 = Row[1].Z;
	Row[1].Y = (float)( c*tmp1 + s*tmp2);
	Row[1].Z = (float)(-s*tmp1 + c*tmp2);

	tmp1 = Row[2].Y; tmp2 = Row[2].Z;
	Row[2].Y = (float)( c*tmp1 + s*tmp2);
	Row[2].Z = (float)(-s*tmp1 + c*tmp2);
}


/// <summary>
/// Post-multiplies the matrix by a rotation about Y.
/// </summary>
/// <param name="theta">The angle to rotate by, in radians.</param>
void Matrix3D::Rotate_Y(float theta)
{
	double tmp1,tmp2;
	float s,c;

	s = (float)std::sin(theta);
	c = (float)std::cos(theta);

	tmp1 = Row[0].X; tmp2 = Row[0].Z;
	Row[0].X = (float)(c*tmp1 - s*tmp2);
	Row[0].Z = (float)(s*tmp1 + c*tmp2);

	tmp1 = Row[1].X; tmp2 = Row[1].Z;
	Row[1].X = (float)(c*tmp1 - s*tmp2);
	Row[1].Z = (float)(s*tmp1 + c*tmp2);

	tmp1 = Row[2].X; tmp2 = Row[2].Z;
	Row[2].X = (float)(c*tmp1 - s*tmp2);
	Row[2].Z = (float)(s*tmp1 + c*tmp2);
}


/// <summary>
/// Post-multiplies the matrix by a rotation about Y.
/// Use this routine when the sine and cosine of the angle are already in hand, as they are
/// when several matrices are turned by the same amount.
/// </summary>
/// <param name="s">The sine of the angle to rotate by.</param>
/// <param name="c">The cosine of the angle to rotate by.</param>
void Matrix3D::Rotate_Y(float s,float c)
{
	double tmp1,tmp2;

	tmp1 = Row[0].X; tmp2 = Row[0].Z;
	Row[0].X = (float)(c*tmp1 - s*tmp2);
	Row[0].Z = (float)(s*tmp1 + c*tmp2);

	tmp1 = Row[1].X; tmp2 = Row[1].Z;
	Row[1].X = (float)(c*tmp1 - s*tmp2);
	Row[1].Z = (float)(s*tmp1 + c*tmp2);

	tmp1 = Row[2].X; tmp2 = Row[2].Z;
	Row[2].X = (float)(c*tmp1 - s*tmp2);
	Row[2].Z = (float)(s*tmp1 + c*tmp2);
}


/// <summary>
/// Post-multiplies the matrix by a rotation about Z.
/// </summary>
/// <param name="theta">The angle to rotate by, in radians.</param>
void Matrix3D::Rotate_Z(float theta)
{
	double tmp1,tmp2;
	float s, c;

	c = (float)std::cos(theta);
	s = (float)std::sin(theta);

	tmp1 = Row[0].X; tmp2 = Row[0].Y;
	Row[0].X = (float)( c*tmp1 + s*tmp2);
	Row[0].Y = (float)(-s*tmp1 + c*tmp2);

	tmp1 = Row[1].X; tmp2 = Row[1].Y;
	Row[1].X = (float)( c*tmp1 + s*tmp2);
	Row[1].Y = (float)(-s*tmp1 + c*tmp2);

	tmp1 = Row[2].X; tmp2 = Row[2].Y;
	Row[2].X = (float)( c*tmp1 + s*tmp2);
	Row[2].Y = (float)(-s*tmp1 + c*tmp2);
}


/// <summary>
/// Post-multiplies the matrix by a rotation about Z.
/// Use this routine when the sine and cosine of the angle are already in hand, as they are
/// when several matrices are turned by the same amount.
/// </summary>
/// <param name="s">The sine of the angle to rotate by.</param>
/// <param name="c">The cosine of the angle to rotate by.</param>
void Matrix3D::Rotate_Z(float s,float c)
{
	double tmp1,tmp2;

	tmp1 = Row[0].X; tmp2 = Row[0].Y;
	Row[0].X = (float)( c*tmp1 + s*tmp2);
	Row[0].Y = (float)(-s*tmp1 + c*tmp2);

	tmp1 = Row[1].X; tmp2 = Row[1].Y;
	Row[1].X = (float)( c*tmp1 + s*tmp2);
	Row[1].Y = (float)(-s*tmp1 + c*tmp2);

	tmp1 = Row[2].X; tmp2 = Row[2].Y;
	Row[2].X = (float)( c*tmp1 + s*tmp2);
	Row[2].Y = (float)(-s*tmp1 + c*tmp2);
}


/// <summary>
/// Fetches the X translation of the matrix.
/// This routine reports where the transform places a point sitting at the origin.
/// </summary>
/// <returns>Returns with the X coordinate of the transformed origin.</returns>
float Matrix3D::Get_X_Val(void)
{
	Vector3 v = (*this) * Vector3(0, 0, 0);
	return(v.X);
}


/// <summary>
/// Fetches the Y translation of the matrix.
/// This routine reports where the transform places a point sitting at the origin.
/// </summary>
/// <returns>Returns with the Y coordinate of the transformed origin.</returns>
float Matrix3D::Get_Y_Val(void)
{
	Vector3 v = (*this) * Vector3(0, 0, 0);
	return(v.Y);
}


/// <summary>
/// Fetches the Z translation of the matrix.
/// This routine reports where the transform places a point sitting at the origin.
/// </summary>
/// <returns>Returns with the Z coordinate of the transformed origin.</returns>
float Matrix3D::Get_Z_Val(void)
{
	Vector3 v = (*this) * Vector3(0, 0, 0);
	return(v.Z);
}


/// <summary>
/// Approximates the rotation of the matrix about the X axis.
/// This routine is an approximation only. It is meaningful when the matrix carries a
/// rotation about a single axis, and misleading when it carries more than one.
/// </summary>
/// <returns>Returns with the X rotation angle, in radians.</returns>
float Matrix3D::Get_X_Rotation(void)
{
	Vector3 v = (*this) * Vector3(0, 1, 0);
	return(std::atan2((double)v.Z, (double)v.Y));
}


/// <summary>
/// Approximates the rotation of the matrix about the Y axis.
/// This routine is an approximation only. It is meaningful when the matrix carries a
/// rotation about a single axis, and misleading when it carries more than one.
/// </summary>
/// <returns>Returns with the Y rotation angle, in radians.</returns>
float Matrix3D::Get_Y_Rotation(void)
{
	Vector3 v = (*this) * Vector3(0, 0, 1);
	return(std::atan2(v.X, v.Z));
}


/// <summary>
/// Approximates the rotation of the matrix about the Z axis.
/// This routine is an approximation only. It is meaningful when the matrix carries a
/// rotation about a single axis, and misleading when it carries more than one.
/// </summary>
/// <returns>Returns with the Z rotation angle, in radians.</returns>
float Matrix3D::Get_Z_Rotation(void)
{
	Vector3 v = (*this) * Vector3(1, 0, 0);
	return(std::atan2(v.Y, v.X));
}


/// <summary>
/// Rotates a vector by the matrix, leaving the translation out.
/// Use this routine on directions and offsets, where the transform's position must not be
/// applied. Points belong in the multiply operator instead.
/// </summary>
/// <returns>Returns with the rotated vector.</returns>
Vector3 Matrix3D::Rotate_Vector(Vector3 const & vect)
{
	return(Vector3(
		(Row[0].X * vect.X + Row[0].Y * vect.Y + Row[0].Z * vect.Z),
		(Row[1].X * vect.X + Row[1].Y * vect.Y + Row[1].Z * vect.Z),
		(Row[2].X * vect.X + Row[2].Y * vect.Y + Row[2].Z * vect.Z))
	);
}


/// <summary>
/// Builds a viewing transform that looks from one point toward another.
/// This routine places the matrix at the eye point and orients it onto the target, with the
/// roll applied last. An eye point sitting on the target leaves the orientation unrotated
/// rather than dividing by zero.
/// </summary>
/// <param name="p">The point to look from.</param>
/// <param name="t">The point to look at.</param>
/// <param name="roll">The roll angle, in radians, about the line of sight.</param>
void Matrix3D::Look_At(const Vector3 &p, const Vector3 &t, float roll)
{
	float sinp, cosp;
	float siny, cosy;

	float dx = t.X - p.X;
	float dy = t.Y - p.Y;
	float dz = t.Z - p.Z;

	float len1 = std::sqrt(dx * dx + dy * dy + dz * dz);
	float len2 = std::sqrt(dx * dx + dz * dz);

	if ( len1 != 0.0f ) {
		sinp = dy / len1;
		cosp = len2 / len1;

	}
	else {
		sinp = 0.0f;
		cosp = 1.0f;
	}

	if ( len2 != 0.0f ) {
		siny = dx / len2;
		cosy = dz / len2;
	}
	else {
		siny = 0.0f;
		cosy = 1.0f;
	}

	Make_Identity();
	Translate(p);

	Rotate_Y(siny, cosy);
	Rotate_X(-sinp, cosp);
	Rotate_Z(roll);
}


/// <summary>
/// Builds a transform that places an object and aims it at a target.
/// This routine is the object counterpart to Look_At. The object's own axes are oriented
/// along the line of sight to the target, and the roll spins it about that line.
/// </summary>
/// <param name="p">The position to place the object at.</param>
/// <param name="t">The point the object is to face.</param>
/// <param name="roll">The roll angle, in radians, about the line of sight.</param>
void Matrix3D::Obj_Look_At(const Vector3 &p, const Vector3 &t, float roll)
{
	float sinp,cosp;
	float siny,cosy;
	float sinr,cosr;
	float yaw;

	Vector3 s;
	Vector3 r;
	Vector3 d = (t - p);

	if (d.Y == 0.0f && d.X == 0.0f) {
		yaw = 0.0;
	}
	else {
		yaw = std::atan2(d.Y, d.X);
	}

	float len = std::sqrt(d[1] * d[1] + d[0] * d[0]);

	float pitch = std::atan2(-d.Z, len);

	cosy = std::cos(yaw);
	siny = std::sin(yaw);

	cosp = std::cos(pitch);
	sinp = std::sin(pitch);

	cosr = std::cos(roll);
	sinr = std::sin(roll);

	s.X = cosr * sinp * cosy + sinr * siny;
	s.Y = cosr * sinp * siny - sinr * cosy;
	s.Z = cosr * cosp;

	d = Normalize(d);

	Vector3::Cross_Product(d, s, &r);

	Row[0].X = r.X;
	Row[0].Y = s.X;
	Row[0].Z = d.X;
	Row[0].W = p.X;

	Row[1].X = r.Y;
	Row[1].Y = s.Y;
	Row[1].Z = d.Y;
	Row[1].W = p.Y;

	Row[2].X = r.Z;
	Row[2].Y = s.Z;
	Row[2].Z = d.Z;
	Row[2].W = p.Z;
}


/***********************************************************************************************
* Matrix3D::Multiply -- matrix multiplication without temporaries.                            *
*                                                                                             *
* INPUT:                                                                                      *
*                                                                                             *
* OUTPUT:                                                                                     *
*                                                                                             *
* WARNINGS:                                                                                   *
*                                                                                             *
* HISTORY:                                                                                    *
*   4/22/98    GTH : Created.                                                                 *
*=============================================================================================*/
inline void Matrix3D::Multiply(const Matrix3D & A, const Matrix3D & B, Matrix3D * set_result)
{
	// Widened so the sums below accumulate in double and narrow once, on store.
	double a00 = A.Row[0].X;
	double a01 = A.Row[0].Y;
	double a02 = A.Row[0].Z;
	double a10 = A.Row[1].X;
	double a11 = A.Row[1].Y;
	double a12 = A.Row[1].Z;
	double a20 = A.Row[2].X;
	double a21 = A.Row[2].Y;
	double a22 = A.Row[2].Z;

	double b00 = B.Row[0].X;
	double b01 = B.Row[0].Y;
	double b02 = B.Row[0].Z;
	double b03 = B.Row[0].W;
	double b10 = B.Row[1].X;
	double b11 = B.Row[1].Y;
	double b12 = B.Row[1].Z;
	double b13 = B.Row[1].W;
	double b20 = B.Row[2].X;
	double b21 = B.Row[2].Y;
	double b22 = B.Row[2].Z;
	double b23 = B.Row[2].W;

	set_result->Row[0].X = a00 * b00 + a01 * b10 + a02 * b20;
	set_result->Row[1].X = a10 * b00 + a11 * b10 + a12 * b20;
	set_result->Row[2].X = a20 * b00 + a21 * b10 + a22 * b20;

	set_result->Row[0].Y = a00 * b01 + a01 * b11 + a02 * b21;
	set_result->Row[1].Y = a10 * b01 + a11 * b11 + a12 * b21;
	set_result->Row[2].Y = a20 * b01 + a21 * b11 + a22 * b21;

	set_result->Row[0].Z = a00 * b02 + a01 * b12 + a02 * b22;
	set_result->Row[1].Z = a10 * b02 + a11 * b12 + a12 * b22;
	set_result->Row[2].Z = a20 * b02 + a21 * b12 + a22 * b22;

	set_result->Row[0].W = a00 * b03 + a01 * b13 + a02 * b23 + A.Row[0].W;
	set_result->Row[1].W = a10 * b03 + a11 * b13 + a12 * b23 + A.Row[1].W;
	set_result->Row[2].W = a20 * b03 + a21 * b13 + a22 * b23 + A.Row[2].W;
}


/// <summary>
/// Concatenates two transform matrices.
/// The resulting transform applies the right hand matrix first and the left hand matrix
/// second, which is the usual convention for building a transform out of parts.
/// </summary>
/// <returns>Returns with the product of the two matrices.</returns>
Matrix3D operator * (const Matrix3D & a, const Matrix3D & b)
{
	Matrix3D c;
	Matrix3D::Multiply(a, b, &c);
	return(c);
}


/// <summary>
/// Transforms a point by the matrix.
/// This routine applies the whole transform, translation included, so it is the one to use
/// on positions. Use Rotate_Vector instead when the vector is a direction and the
/// translation must be left out of it.
/// </summary>
/// <returns>Returns with the transformed point.</returns>
Vector3 operator * (const Matrix3D & m, const Vector3 & vect)
{
	// Accumulated in double; the drawer truncates the result into fixed point.
	double x = (double)vect.X;
	double y = (double)vect.Y;
	double z = (double)vect.Z;

	Vector3 vec(
		(float)((double)m.Row[0].X * x + (double)m.Row[0].Y * y + (double)m.Row[0].Z * z + (double)m.Row[0].W),
		(float)((double)m.Row[1].X * x + (double)m.Row[1].Y * y + (double)m.Row[1].Z * z + (double)m.Row[1].W),
		(float)((double)m.Row[2].X * x + (double)m.Row[2].Y * y + (double)m.Row[2].Z * z + (double)m.Row[2].W)
	);
	return(vec);
}



/// <summary>
/// Fetches the inverse of an orthogonal transform.
/// This routine takes the cheap route rather than inverting the matrix in general, which is
/// all the engine ever needs -- every transform it builds out of rotations and translations
/// qualifies.
/// </summary>
/// <returns>Returns with the inverse of the supplied matrix.</returns>
/// <remarks>The source matrix must be orthogonal. A matrix that has been through any of the
/// Scale or Shear routines is not, and the result will be nonsense.</remarks>
Matrix3D Matrix3D::Orthogonal_Inverse(Matrix3D const & src)
{
	Matrix3D inv;

	// Transposing the rotation submatrix

	inv.Row[0].X = src.Row[0].X;
	inv.Row[0].Y = src.Row[1].X;
	inv.Row[0].Z = src.Row[2].X;

	inv.Row[1].X = src.Row[0].Y;
	inv.Row[1].Y = src.Row[1].Y;
	inv.Row[1].Z = src.Row[2].Y;

	inv.Row[2].X = src.Row[0].Z;
	inv.Row[2].Y = src.Row[1].Z;
	inv.Row[2].Z = src.Row[2].Z;

	// Now, calculate translation portion of matrix:
	// T' = -R'T

	inv.Row[0].W = -(inv.Row[0].X * src.Row[0].W + inv.Row[0].Z * src.Row[2].W + inv.Row[0].Y * src.Row[1].W);
	inv.Row[1].W = -(inv.Row[1].X * src.Row[0].W + inv.Row[1].Z * src.Row[2].W + inv.Row[1].Y * src.Row[1].W);
	inv.Row[2].W = -(inv.Row[2].X * src.Row[0].W + inv.Row[2].Z * src.Row[2].W + inv.Row[2].Y * src.Row[1].W);

	return(inv);
}
