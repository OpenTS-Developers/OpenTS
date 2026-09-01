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

#include "always.h"

#include "quat.h"

#include "matrix3d.h"

#include <algorithm>

#define SLERP_EPSILON 1e-5

static float project_to_sphere(float r, float x, float y);


/// <summary>
/// Constructs a quaternion from the components given.
/// </summary>
Quaternion::Quaternion(float x, float y, float z, float w)
{
	X=x;
	Y=y;
	Z=z;
	W=w;
}


/// <summary>
/// Sets the quaternion to the components given.
/// </summary>
void Quaternion::Set(float x, float y, float z, float w)
{
	X=x;
	Y=y;
	Z=z;
	W=w;
}


/// <summary>
/// Normalizes the quaternion.
/// The components are divided by the squared length rather than by the length. A quaternion
/// of zero length is left exactly as it was rather than being divided out of existence.
/// </summary>
void Quaternion::Normalize(void)
{
	// len2 stays a float: it is the divisor below, so widening it would change
	// the quotient rather than just the way the sum is reached.
	float len2 = (float)((double)X * (double)X + (double)Y * (double)Y +
						 (double)Z * (double)Z + (double)W * (double)W);

	if (0.0f != len2) {
		X /= len2;
		Y /= len2;
		Z /= len2;
		W /= len2;
	}
}


/// <summary>
/// Scales every component of the quaternion.
/// </summary>
/// <param name="s">The factor to scale the components by.</param>
void Quaternion::Scale(float s)
{
	X = (float)(s * X);
	Y = (float)(s * Y);
	Z = (float)(s * Z);
	W = (float)(s * W);
}


/// <summary>
/// Fetches a reference to one component of the quaternion by index.
/// The components are laid out in X, Y, Z, W order. Use this routine to walk the components
/// when building a quaternion up a piece at a time.
/// </summary>
/// <param name="i">The index of the component wanted, 0 through 3.</param>
/// <returns>Returns with a reference to the component requested.</returns>
float & Quaternion::operator [](int i)
{
	return (&X)[i];
}


/// <summary>
/// Fetches one component of the quaternion by index.
/// The components are laid out in X, Y, Z, W order.
/// </summary>
/// <param name="i">The index of the component wanted, 0 through 3.</param>
/// <returns>Returns with the value of the component requested.</returns>
const float Quaternion::operator [](int i) const
{
	return (&X)[i];
}


/// <summary>
/// Assigns one quaternion to another.
/// </summary>
/// <returns>Returns with a copy of the quaternion that was assigned.</returns>
Quaternion Quaternion::operator=(const Quaternion & q)
{
	X = q[0];
	Y = q[1];
	Z = q[2];
	W = q[3];

	return(*this);
}


/***********************************************************************************************
 * Q::Make_Closest -- Use nearest representation to the given quaternion.                      *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/28/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
Quaternion & Quaternion::Make_Closest(const Quaternion & qto)
{
	Quaternion t = *this;
	Quaternion rel1 = t * Inverse(qto);

	Quaternion negthis = Quaternion(-X, -Y, -Z, -W);
	Quaternion rel2 = negthis * Inverse(qto);

	float dot1 = rel1.Length2();
	float dot2 = rel2.Length2();

	// if we are on opposite hemisphere from qto, negate ourselves
	if (dot2 < dot1) {
		X = -X;
		Y = -Y;
		Z = -Z;
		W = -W;
	}

	return(*this);
}


/// <summary>
/// Multiplies two quaternions together.
/// The product is the two rotations composed into one. The result is normalized before it
/// is handed back, so a chain of multiplies will not wander off unit length.
/// </summary>
/// <returns>Returns with the normalized product quaternion.</returns>
Quaternion operator * (const Quaternion & a,const Quaternion & b)
{
	Quaternion t1;
	Quaternion t2;
	Quaternion t3;

	t1 = a;
	t2 = b;

	t1.Scale(b.W);
	t2.Scale(a.W);

	t3.X = a.Y * b.Z - a.Z * b.Y;
	t3.Y = a.Z * b.X - a.X * b.Z;
	t3.Z = a.X * b.Y - a.Y * b.X;

	Quaternion q;

	q.X = t1.X + t2.X + t3.X;
	q.Y = t1.Y + t2.Y + t3.Y;
	q.Z = t1.Z + t2.Z + t3.Z;
	q.W = a.W * b.W - (a.X * b.X + a.Y * b.Y + a.Z * b.Z);

	q.Normalize();
	return(q);
}


/// <summary>
/// Divides one quaternion by another.
/// The quotient is the first rotation composed with the inverse of the second.
/// </summary>
/// <returns>Returns with the quotient quaternion.</returns>
Quaternion operator / (const Quaternion & a,const Quaternion & b)
{
	return(a * Inverse(b));
}


/// <summary>
/// Fetches the inverse of the quaternion given.
/// This routine is used by the division operator and by Make_Closest whenever a rotation
/// has to be undone.
/// </summary>
/// <returns>Returns with the inverse quaternion.</returns>
Quaternion Inverse(const Quaternion & q)
{
	return(Quaternion(-q[0],-q[1],-q[2],q[3]));
}


/// <summary>
/// Fetches the conjugate of the quaternion given.
/// The vector part is negated and the scalar part is left alone. For a unit quaternion the
/// conjugate is also its inverse.
/// </summary>
/// <returns>Returns with the conjugate quaternion.</returns>
Quaternion Conjugate(const Quaternion & q)
{
	return(Quaternion(-q[0],-q[1],-q[2],q[3]));
}

/// https://android.googlesource.com/platform/external/tinyobjloader/+/HEAD/examples/viewer/trackball.cc#194

/// <summary>
/// Determines the rotation implied by a drag across a virtual trackball.
/// This routine projects the two points onto a deformed sphere of the size given and works
/// out the rotation that carries the first onto the second. Use this routine to let the
/// mouse spin an object about an arbitrary axis.
/// </summary>
/// <param name="x0">The horizontal coordinate the drag started from.</param>
/// <param name="y0">The vertical coordinate the drag started from.</param>
/// <param name="x1">The horizontal coordinate the drag finished at.</param>
/// <param name="y1">The vertical coordinate the drag finished at.</param>
/// <param name="size">The radius of the virtual trackball.</param>
/// <returns>Returns with the rotation quaternion. A drag that went nowhere yields the
/// identity quaternion.</returns>
Quaternion Trackball(float x0, float y0, float x1, float y1, float size)
{
	Vector3 a; /// axis of rotation
	float phi; /// how much to rotate about axis
	Vector3	p1;
	Vector3	p2;
	Vector3 d;
	float t;

	if (x0 == x1 && y0 == y1) {
		/// Zero rotation.
		return(Quaternion(0.0, 0.0, 0.0, 1.0));
	}

	/// First, figure out z-coordinates for projection of P1 and P2 to
	/// deformed sphere
	p1[0] = x0;
	p1[1] = y0;
	p1[2] = project_to_sphere(size, x0, y0);

	p2[0] = x1;
	p2[1] = y1;
	p2[2] = project_to_sphere(size, x1, y1);

	/// Now, we want the cross product of P1 and P2
	Vector3::Cross_Product(p2, p1, &a);

	/// Figure out how much to rotate around that axis
	d = p1 - p2;
	t = d.Length() / (2.0 * size);

	/// Avoid problems with out of control values...
	if (t > 1.0f) {
		t = 1.0f;
	} else if (t < -1.0f) {
		t = -1.0f;
	}
	phi = 2.0 * std::asin((double)t);

	return(Axis_To_Quat(a, phi));
}


/// <summary>
/// Converts an axis and an angle into a rotation quaternion.
/// The axis given is normalized on the way through, so the caller need not bother. This
/// routine is used by the trackball code to turn a computed rotation axis into a quaternion
/// the rest of the math library can use.
/// </summary>
/// <param name="a">The axis to rotate about. It need not be normalized.</param>
/// <param name="phi">The angle to rotate by, expressed in radians.</param>
/// <returns>Returns with the quaternion that performs the rotation described.</returns>
Quaternion Axis_To_Quat(const Vector3 &a, float phi)
{
	Quaternion q;
	Vector3 tmp = a;

	tmp = Normalize(tmp);
	q.X = tmp[0];
	q.Y = tmp[1];
	q.Z = tmp[2];

	q.Scale(std::sin(phi / 2.0f));
	q.W = std::cos(phi / 2.0f);

	return(q);
}

/// https://android.googlesource.com/platform/external/tinyobjloader/+/HEAD/examples/viewer/trackball.cc#194

/// <summary>
/// Projects a screen point onto the trackball's deformed sphere.
/// This routine gives a two dimensional point the depth it would have on the trackball.
/// Points near the middle land on a sphere of the radius given; points further out land on
/// a hyperbolic sheet instead, so the mapping stays smooth right out to the edge.
/// </summary>
/// <param name="r">The radius of the virtual trackball.</param>
/// <returns>Returns with the depth coordinate of the projected point.</returns>
static float project_to_sphere(float r, float x, float y)
{
	const float SQRT2 = (float)M_SQRT2;

	float d, t, z;

	d = std::sqrt(x * x + y * y);
	if (d < r * (SQRT2 / 2.0f)) {
		z = (float)std::sqrt(r * r - d * d);
	} else {
		t = r * (SQRT2 / 2.0f);
		z = t * t / d;
	}

	return(z);
}


/***********************************************************************************************
 * Slerp -- Spherical Linear interpolation!                                                    *
 *                                                                                             *
 * INPUT:                                                                                      *
 *  p - start quaternion                                                                       *
 *  q - end quaternion                                                                         *
 *  alpha - interpolating parameter                                                            *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/28/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
Quaternion Slerp(const Quaternion &a, const Quaternion &b, float t)
{
	Quaternion res;
	double alpha;				/// interpolation parameter (0 to 1)
	double beta;				// complementary interploation parameter
	double theta;				// angle between p and q
	double sin_t, cos_t; 		// sine, cosine of theta

	// cos theta = dot product of p and q
	// Accumulated in double; the two epsilon tests below turn it into a branch.
	cos_t = (double)a.X * (double)b.X + (double)a.Y * (double)b.Y +
			(double)a.Z * (double)b.Z + (double)a.W * (double)b.W;

	// if q is on opposite hemisphere from A, use -B instead
	if ((cos_t + 1.0) > SLERP_EPSILON) {
		/*
		 * if B is (within precision limits) the same as A,
		 * just linear interpolate between A and B.
		 * Can't do spins, since we don't know what direction to spin.
		 */
		if ((1.0 - cos_t) > SLERP_EPSILON) {
			// normal slerp!
			// The epsilon test bounds cos_t away from one but not from just
			// outside the domain, where acos would answer with a NaN.
			theta = std::acos(std::clamp(cos_t, -1.0, 1.0));
			sin_t = std::sin(theta);
			beta = std::sin((1.0 - (double)t) * theta) / sin_t;
			alpha = std::sin((double)t * theta) / sin_t;
		} else {
			// if q is very close to p, just linearly interpolate
			// between the two.
			beta = 1.0 - (double)t;
			alpha = t;

		}
		/// Interpolate.
		for (int i = 0; i < 4; i++) {
			res[i] = (beta * a[i]) + (alpha * b[i]);
		}
	} else {
		beta = std::sin((1.0 - (double)t) * M_PI_2);
		alpha = std::sin((double)t * M_PI_2);
		/// Interpolate.
		for (int i = 0; i < 4; i++) {
			res[i] = (beta * a[i]) + (alpha * b[i]);
		}
	}
	return(res);
}


/***********************************************************************************************
 * Build_Quaternion -- Creates a quaternion from a Matrix                                      *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *   Matrix MUST NOT have scaling!                                                             *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/28/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
Quaternion Build_Quaternion(const Matrix3D& mat)
{
	const float * m = (float const *)&mat;

	double s, tr;
	int i, j, k;
	Quaternion q;

	int tab1[3] = { (4 * 0) + 0, (4 * 1) + 1, (4 * 2) + 2 };

	int tab2[3][4] = {
		{ (4 * 0) + 0, (4 * 0) + 1, (4 * 0) + 2, (4 * 0) + 3 },
		{ (4 * 1) + 0, (4 * 1) + 1, (4 * 1) + 2, (4 * 1) + 3 },
		{ (4 * 2) + 0, (4 * 2) + 1, (4 * 2) + 2, (4 * 2) + 3 },
	};


	// sum the diagonal of the rotation matrix
	tr = m[tab1[0]] + m[tab1[1]] + m[tab1[2]];

	if (tr == 0.0) {
		return(q);
	}

	if (tr > 0.0) {

		s = std::sqrt(tr + 1.0);
		q[3] = (s * 0.5);
		s = 0.5 / s;

		q[0] = (m[tab2[2][1]] - m[tab2[1][2]]) * s;
		q[1] = (m[tab2[0][2]] - m[tab2[2][0]]) * s;
		q[2] = (m[tab2[1][0]] - m[tab2[0][1]]) * s;

	} else {

		i = 0;
		if ((double)m[tab1[1]] > (double)m[tab1[i]]) i = 1;
		if ((double)m[tab1[2]] > (double)m[tab1[i]]) i = 2;

		switch (i) {
			case 1:
				j = 2;
				k = 0;
				break;
			case 2:
				j = 0;
				k = 1;
				break;
			default:
				j = 1;
				k = 2;
				break;
		}

		double di = m[tab1[i]];
		double dj = m[tab1[j]];
		double dk = m[tab1[k]];

		s = std::sqrt(di - (dj + dk) + 1.0);

		q[i] = (s * 0.5);
		s = 0.5 / s;

		q[3] = (m[tab2[k][j]] - m[tab2[j][k]]) * s;
		q[j] = (m[tab2[j][i]] + m[tab2[i][j]]) * s;
		q[k] = (m[tab2[k][i]] + m[tab2[i][k]]) * s;
	}

	return(q);
}


/// <summary>
/// Converts a quaternion into an equivalent transform matrix.
/// This routine builds the rotation sub-matrix out of the quaternion and leaves the
/// translation column zeroed. Use this routine to hand a quaternion rotation to code that
/// works in Matrix3D terms.
/// </summary>
/// <returns>Returns with the transform that performs the rotation described.</returns>
Matrix3D Build_Matrix3D(const Quaternion &q)
{
	Matrix3D m;

	// initialize the rotation sub-matrix
	m[0].X = 1.0f - 2.0f * (q[1] * q[1] + q[2] * q[2]);
	m[0].Y = 2.0f * (q[0] * q[1] - q[2] * q[3]);
	m[0].Z = 2.0f * (q[2] * q[0] + q[1] * q[3]);

	m[1].X = 2.0f * (q[0] * q[1] + q[2] * q[3]);
	m[1].Y = 1.0f - 2.0f * (q[2] * q[2] + q[0] * q[0]);
	m[1].Z = 2.0f * (q[1] * q[2] - q[0] * q[3]);

	m[2].X = 2.0f * (q[2] * q[0] - q[1] * q[3]);
	m[2].Y = 2.0f * (q[1] * q[2] + q[0] * q[3]);
	m[2].Z = 1.0f - 2.0f * (q[1] * q[1] + q[0] * q[0]);

	// no translation
	m[0].W = m[1].W = m[2].W = 0.0f;

	return(m);
}
