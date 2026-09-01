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
 *                     $Archive:: /G/wwlib/RGB.H                                              $*
 *                                                                                             *
 *                      $Author:: Eric_c                                                      $*
 *                                                                                             *
 *                     $Modtime:: 4/02/99 12:00p                                              $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once


class PaletteClass;
class HSVClass;


/*
 * The three colour guns as the game's data files store them. Voxel palettes and the control
 * section of the isometric tile artwork are read straight onto this layout, so its size and
 * packing are fixed by those formats.
 */
#pragma pack(1)
struct RGBStruct
{
	bool operator==(const RGBStruct &that) const { return(Red == that.Red && Green == that.Green && Blue == that.Blue); }
	bool operator!=(const RGBStruct &that) const { return(Red != that.Red || Green != that.Green || Blue != that.Blue); }

	unsigned char Red;
	unsigned char Green;
	unsigned char Blue;
};
#pragma pack()


/*
**	Each color entry is represented by this class. It holds the values for the color
**	guns. The gun values are recorded in device dependant format, but the interface
**	uses gun values from 0 to 255.
*/
class RGBClass
{
	public:
		RGBClass(void) : Red(0), Green(0), Blue(0) {}
		RGBClass(unsigned char red, unsigned char green, unsigned char blue) : Red(red), Green(green), Blue(blue) {}
		operator HSVClass (void) const;

		bool operator==(const RGBClass &that) const { return(Red == that.Red && Green == that.Green && Blue == that.Blue); }
		bool operator!=(const RGBClass &that) const { return(Red != that.Red || Green != that.Green || Blue != that.Blue); }

		enum {
			MAX_VALUE=255
		};

		RGBClass & Lerp(RGBClass const & a, RGBClass const & b, float t);
		RGBClass & Set(RGBClass const & rgb, float value);
		void Adjust(int ratio, RGBClass const & rgb);
		int Difference(RGBClass const & rgb) const;
		// Carries the color to or from a save game.
		template<typename S>
		void Serialize(S & stream)
		{
			stream.Serialize(Red);
			stream.Serialize(Green);
			stream.Serialize(Blue);
		}

		int Get_Red(void) const {return(Red);}
		int Get_Green(void) const {return(Green);}
		int Get_Blue(void) const {return(Blue);}
		void Set_Red(unsigned char value) {Red = value;}
		void Set_Green(unsigned char value) {Green = value;}
		void Set_Blue(unsigned char value) {Blue = value;}

	private:

		friend class PaletteClass;

		/*
		**	These hold the actual color gun values in machine independant scale. This
		**	means the values range from 0 to 255.
		*/
		unsigned char Red;
		unsigned char Green;
		unsigned char Blue;
};

extern RGBClass const BlackColor;
