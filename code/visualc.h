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
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwlib/visualc.h                              $*
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 5/16/01 4:11p                                               $*
 *                                                                                             *
 *                    $Revision:: 9                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#if defined(_MSC_VER)


/*
**	Make the inline depth 255
*/


/*
**	Turn off some unneeded warnings.
*/

// "unreferenced inline function has been removed" Yea, so what?
#pragma warning(disable : 4514)

// "conversion from 'double' to 'float', possible loss of data" Yea, so what?
#pragma warning(disable : 4244)

// "overflow in floating-point constant arithmetic" This warning occurs even if the
// loss of precision is insignificant.
#pragma warning(disable : 4056)

// "function not inlined" This warning is typically useless. The inline keyword
// only serves as a suggestion to the compiler and it may or may not inline a
// function on a case by case basis. No need to be told of this.
#pragma warning(disable : 4710)

// "'this' used in base member initializer list" Using "this" in a base member
// initializer is valid -- no need for this warning.
#pragma warning(disable : 4355)

// "typedef-name used as a synonym for class-name". This is by design and should
// not be a warning.
#pragma warning(disable : 4097)

// Unreferenced local function removed.
#pragma warning(disable : 4505)

// 'function selected for automatic inlining'
#pragma warning(disable : 4711)

// 'copy constructor could not be generated'
#pragma warning(disable : 4511)

// 'assignment operator could not be generated'
#pragma warning(disable : 4512)

// 'unreferenced formal parameter'
#pragma warning(disable : 4100)

// HIDE WARNING 4786 "identifier was truncated to '255' characters in the browser information"
// Tempates create LLLOOONNNGGG identifiers!
#pragma warning(disable : 4786)

// 'function selected for automatic inline expansion'.  Cool, but since we're treating
// warnings as errors, don't warn me about this!
#pragma warning(disable : 4711)


/*
 * Make some warnings errors.
 */

/// deletion of pointer to incomplete type 'X'; no destructor called
#pragma warning(error : 4150)


#define M_E         2.71828182845904523536
#define M_LOG2E     1.44269504088896340736
#define M_LOG10E    0.434294481903251827651
#define M_LN2       0.693147180559945309417
#define M_LN10      2.30258509299404568402
#define M_PI        3.14159265358979323846
#define M_PI_2      1.57079632679489661923
#define M_PI_4      0.785398163397448309616
#define M_1_PI      0.318309886183790671538
#define M_2_PI      0.636619772367581343076
#define M_1_SQRTPI  0.564189583547756286948
#define M_2_SQRTPI  1.12837916709551257390
#define M_SQRT2     1.41421356237309504880
#define M_SQRT_2    0.707106781186547524401

// Single precision pi, for the float paths that would otherwise round M_PI at every use.
#define M_FPI 3.141592654f

#endif


/*
**	Macros to convert between degrees and radians
*/
#ifndef RAD_TO_DEG
#define RAD_TO_DEG(x)	(((double)x)*180.0/M_PI)
#endif

#ifndef DEG_TO_RAD
#define DEG_TO_RAD(x)	(((double)x)*M_PI/180.0)
#endif

#ifndef RAD_TO_DEGF
#define RAD_TO_DEGF(x)	(((float)x)*180.0f/M_PI)
#endif

#ifndef DEG_TO_RADF
#define DEG_TO_RADF(x)	(((float)x)*M_PI/180.0f)
#endif
