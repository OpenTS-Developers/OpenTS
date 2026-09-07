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

/* $Header: /CounterStrike/EDIT.CPP 1     3/03/97 10:24a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : EDIT.CPP                                                     *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic, Maria del Mar McCready Legg                   *
 *                                                                                             *
 *                   Start Date : 01/15/95                                                     *
 *                                                                                             *
 *                  Last Update : June 25, 1995 [JLB]                                          *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   EditClass::Action -- Handles input events.                                                *
 *   EditClass::Draw_Background -- Draw the background to the edit gadget.                     *
 *   EditClass::Draw_Me -- Draws the edit box and embedded text.                               *
 *   EditClass::Draw_Text -- Draws the edit gadget text.                                       *
 *   EditClass::EditClass -- Normal constructor for edit class object.                         *
 *   EditClass::Handle_Key -- Handles keyboard input to edit gadget.                           *
 *   EditClass::Set_Text -- Sets the text to the edit gadget.                                  *
 *   EditClass::~EditClass -- Default destructor for the edit gadget.                          *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "edit.h"

#include "_keyboar.h"
#include "_surface.h"
#include "dialog.h"
#include "dsurface.h"
#include "font.h"
#include "globals.h"
#include "keyboard.h"
#include "scheme.h"
#include "utf8.h"
#include "vector.h"

#include "color.hh"


/***********************************************************************************************
 * EditClass::EditClass -- Normal constructor for edit class object.                           *
 *                                                                                             *
 *    This is the normal constructor used to create an edit object.                            *
 *                                                                                             *
 * INPUT:   id    -- The ID number for this edit object. This is the ID number that will be    *
 *                   returned by the Input() function when the <RETURN> key is pressed if this *
 *                   gadget has the keyboard input focus.                                      *
 *                                                                                             *
 *          text  -- Reference to the text buffer that the edit gadget will modify as keyboard *
 *                   input is processed. The value that this buffer contains is the default    *
 *                   text displayed.                                                           *
 *                                                                                             *
 *          maxlen-- The maximum size of the text buffer specified. This length INCLUDES the   *
 *                   trailing null character so a simple sizeof() function call can be used.   *
 *                                                                                             *
 *          flags -- These are the text print control flags. It is used to control how the     *
 *                   text looks in the edit box. Use the normal TPF_??? flags.                 *
 *                                                                                             *
 *          x,y   -- The pixel coordinates of the upper left corner of the edit gadget area.   *
 *                                                                                             *
 *          w,h   -- The pixel dimensions of the edit box. If either of these are no provided, *
 *                   or set to -1, then the dimension is determined from the string itself.    *
 *                                                                                             *
 *          sytle -- This style flag parameter control what kind of characters are allowed in  *
 *                   the edit box. The initial string in the text buffer may contain illegal   *
 *                   characters, but they are NOT removed regardless of this parameter.        *
 *                                                                                             *
 * OUTPUT:     none                                                                            *
 * WARNINGS:   none                                                                            *
 * HISTORY:                                                                                    *
 *   01/05/1995 MML : Created.                                                                 *
 *   01/21/1995 JLB : Modified.                                                                *
 *=============================================================================================*/
EditClass::EditClass(int id, char * text, int max_len, TextPrintType flags, int x, int y, int w, int h, EditStyle style) :
	BASECLASS (id, x, y, w, h, LEFTPRESS), String(text)
{
	TextFlags = (TextPrintType)(flags & ~(TPF_CENTER));
	EditFlags = style;
	Set_Text(text, max_len);
	Color = GadgetClass::Get_Color_Scheme();

	if (w == -1 || h == -1) {
		FontClass *font = Font_From_TPF(flags);

		if (h == -1) {
			Height = font->Get_Height()+1;
		}
		if (w == -1) {
			if (strlen(String) > 0) {
				Width = font->String_Pixel_Width(String) + 6;
			} else {
				Width = (font->String_Pixel_Width("X") * (MaxLength+1)) + 2;
			}
    	}
	}

	IsReadOnly = 0;
}


/***********************************************************************************************
 * EditClass::~EditClass -- Default destructor for the edit gadget.                            *
 *                                                                                             *
 *    This default destructor removes the focus setting if it currently has it.                *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/24/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
EditClass::~EditClass(void)
{
	if (Has_Focus()) {
		Clear_Focus();
	}
}


/***********************************************************************************************
 * EditClass::Set_Text -- Sets the text to the edit gadget.                                    *
 *                                                                                             *
 *    Use this routine to change the text that this edit gadget refers to.                     *
 *                                                                                             *
 * INPUT:   text     -- Reference to the character array that this edit gadget will be         *
 *                      modifying.                                                             *
 *          max_len  -- The maximum size of the buffer that will be modified.                  *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 * WARNINGS:   none                                                                            *
 * HISTORY:                                                                                    *
 *   01/21/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void EditClass::Set_Text(char * text, int max_len)
{
	String = text;
	MaxLength = max_len-1;
	Length = strlen(String);
	Flag_To_Redraw();
}


/***********************************************************************************************
 * EditClass::Draw_Me -- Draws the edit box and embedded text.                                 *
 *                                                                                             *
 *    This routine will render the edit box. This will show the box outline as well as any     *
 *    text it may contain.                                                                     *
 *                                                                                             *
 * INPUT:   forced   -- Should the edit box be drawn even if it thinks it doesn't have to?     *
 *                                                                                             *
 * OUTPUT:  Was the edit box drawn?                                                            *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/25/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int EditClass::Draw_Me(int forced)
{
	if (BASECLASS::Draw_Me(forced)) {
		/*
		**	Hide the mouse.
		*/
		Conditional_Hide_Mouse(Rect(X, Y, Width, Height));

		/*
		**	Draw the body & set text color.
		*/
		Draw_Background();

		/*
		**	Display the text.
		*/
		Draw_Text(String);

		/*
		**	Display the mouse.
		*/
		Conditional_Show_Mouse();

		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * EditClass::Action -- Handles input events.                                                  *
 *                                                                                             *
 *    This routine will handle all mouse and keyboard events directed at this edit box         *
 *    gadget. For keyboard events, this will insert the characters into the edit box.          *
 *                                                                                             *
 * INPUT:   flags -- The event flag that triggered this function call.                         *
 *                                                                                             *
 *          key   -- Reference to the keyboard/mouse event that triggered this function call.  *
 *                                                                                             *
 * OUTPUT:  Should the list be processed further?                                              *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/25/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int EditClass::Action(unsigned flags, KeyNumType & key)
{
	/*
	**	If this is a read-only edit box, it's a display-only device
	*/
	if (IsReadOnly) {
		return(false);
	}

	/*
	**	If the left mouse button is pressed over this gadget, then set the focus to
	**	this gadget. The event flag is cleared so that no button ID number is returned.
	*/
	if ((flags & LEFTPRESS)) {
		flags &= ~LEFTPRESS;
		Set_Focus();
		Flag_To_Redraw();		// force to draw cursor
	}

	/*
	**	Handle keyboard events here. Normally, the key is added to the string, but if the
	**	RETURN key is pressed, then the button ID number is returned from the Input()
	**	function.
	*/
	if ((flags & KEYBOARD) && Has_Focus()) {

		/*
		**	Process the keyboard character. If indicated, consume this keyboard event
		**	so that the edit gadget ID number is not returned.
		*/
		if (key == KN_ESC) {

			Clear_Focus();
			flags = 0;

		} else {

			KeyASCIIType ascii = (KeyASCIIType)Keyboard->To_ASCII(key);

			/*
			**	Allow numeric keypad presses to map to ascii numbers
			*/
			if ((key & WWKEY_VK_BIT) && ascii >='0' && ascii <= '9') {

				key = (KeyNumType)(key & ~WWKEY_VK_BIT);
				if ( (!(flags & LEFTRELEASE)) && (!(flags & RIGHTRELEASE))) {
					if (Handle_Key (ascii) ) {
						flags &= ~KEYBOARD;
						key = KN_NONE;
					}
				}
			} else {
				/*
				**	Filter out all special keys except return and backspace
				*/  	if ((!(key & WWKEY_VK_BIT) && UTF8::Is_Printable((char32_t)ascii))
					|| key == KN_RETURN || key == KN_BACKSPACE) {



					if ((!(flags & LEFTRELEASE)) && (!(flags & RIGHTRELEASE))) {
						if (Handle_Key(ascii)) {
							flags &= ~KEYBOARD;
							key = KN_NONE;
						}
					}
				} else {
					flags &= ~KEYBOARD;
					key = KN_NONE;
				}
			}
		}
	}

	return(BASECLASS::Action(flags, key));
}


/***********************************************************************************************
 * EditClass::Draw_Background -- Draw the background to the edit gadget.                       *
 *                                                                                             *
 *    This routine will redraw the edit gadget background. The overlaying text is handled by   *
 *    a different routine. The mouse is guaranteed to be hidden when this routine is called.   *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/21/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void EditClass::Draw_Background(void)
{
	Draw_Box (Rect(X, Y, Width, Height), BOXSTYLE_BOX, true);
}


/***********************************************************************************************
 * EditClass::Draw_Text -- Draws the edit gadget text.                                         *
 *                                                                                             *
 *    This routine is called when the edit gadget text needs to be drawn. The background has   *
 *    already been drawn by the time this function is called. The mouse is guaranteed to be    *
 *    hidden as well.                                                                          *
 *                                                                                             *
 * INPUT:   text  -- The text to draw in the edit gadget.                                      *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/21/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void EditClass::Draw_Text(char const * text)
{
	TextPrintType flags;

	if (Has_Focus()) {
		flags = TPF_BRIGHT_COLOR;
	} else {
		flags = (TextPrintType)0;
	}

	Conquer_Clip_Text_Print(text, *LogicalSurface, LogicalSurface->Get_Rect(), Point2D(X+1, Y+1), ColorSchemes[Color], TBLACK, (TextPrintType)(TextFlags | flags), Width-2);
	FontClass *font = Font_From_TPF(TextFlags);
	if (Has_Focus() && strlen(text) < (unsigned)MaxLength &&
		(font->String_Pixel_Width(text) + font->String_Pixel_Width ("_") < Width-2) ) {
		Conquer_Clip_Text_Print( "_", *LogicalSurface, LogicalSurface->Get_Rect(), Point2D(X+1+font->String_Pixel_Width(text), Y+1), ColorSchemes[Color], TBLACK, (TextPrintType)(TextFlags | flags), -1);
	}

}


/***********************************************************************************************
 * EditClass::Handle_Key -- Handles keyboard input to edit gadget.                             *
 *                                                                                             *
 *    This is the gruntwork routine that processes keyboard input to the edit gadget. This     *
 *    routine will be called when keyboard input has been detected and this gadget has the     *
 *    current focus.                                                                           *
 *                                                                                             *
 * INPUT:   ascii -- The ASCII key code that was fetched from the keyboard buffer.             *
 *                                                                                             *
 * OUTPUT:  bool; Should this keyboard input NOT cause the gadget ID number to be returned     *
 *                from the controlling Input() routine? Typically, the return value would be   *
 *                true unless the focus is lost due to the <RETURN> key being pressed.         *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 * HISTORY:                                                                                    *
 *   01/21/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
bool EditClass::Handle_Key(KeyASCIIType ascii)
{
	FontClass *font = Font_From_TPF(TextFlags);

	switch (ascii) {
		/*
		**	Handle the special case of a non-keyboard event. It is possible that this
		**	key code might be passed to this routine if this routine has been overridden
		**	and the key event was consumed.
		*/
		case 0:
			break;

		/*
		**	If the return key is pressed, then remove the focus from this edit
		**	gadget but otherwise let the normal gadget processing proceed. This
		**	causes the gadget ID number to be returned from the Input() function
		**	so that the controlling program will know that the text can be
		**	processed.
		*/
		case KA_RETURN:
			Clear_Focus();
			return(false);

		/*
		**	When the BACKSPACE key is pressed, remove the last character in the edit string.
		*/
		case KA_BACKSPACE:
			if (Length) {
				Length = (int)(UTF8::Previous(String, String + Length) - String);
				String[Length] = '\0';
				Flag_To_Redraw();
			}
			break;

		/*
		**	If the keyboard event was not a recognized special key, then examine to see
		**	if it can legally be added to the edit string and do so if possible.
		*/
		default: {
			char32_t code = (char32_t)ascii;

			/*
			**	Don't add a character if the length is greater than edit width.
			*/
			if ((font->String_Pixel_Width(String) + font->Char_Pixel_Width(code) ) >= (Width-2)) {
				break;
			}

			/*
			**	Invisible characters are never added to the string. This is
			**	especially true for spaces at the beginning of the string.
			*/
			bool graphic = (code < 0x80) ? (isgraph((int)code) != 0) : UTF8::Is_Printable(code);
			if (!graphic && code != ' ') break;
			if (code == ' ' && Length == 0) break;

			/*
			**	If this is an upper case only edit gadget, then force the alphabetic
			**	character to upper case.
			*/
			if ((EditFlags & UPPERCASE) && code < 0x80 && isalpha((int)code)) {
				code = (char32_t)toupper((int)code);
			}

			bool digit = code < 0x80 && isdigit((int)code);
			bool alpha = code >= 0x80 || isalpha((int)code);
			bool alnum = code >= 0x80 || isalnum((int)code);
			if ((!(EditFlags & NUMERIC) || !digit) &&
				(!(EditFlags & ALPHA) || !alpha) &&
				(!(EditFlags & MISC) || alnum) &&
				code != ' ') {
					break;
			}

			/*
			**	Don't add a character if the length is already at maximum.
			*/
			char encoded[UTF8::MAX_SEQUENCE];
			int length = UTF8::Encode(code, encoded);
			if ((unsigned)(Length + length) > (unsigned)MaxLength) break;

			/*
			**	The character passed all legality checks, so add it to the edit string
			**	and flag this gadget to be redrawn. The manual flag to redraw is needed
			**	because the event flag has been cleared. This prevents the gadget's ID
			**	number from being returned just because the gadget has been edited.
			*/
			memcpy(String + Length, encoded, length);
			Length += length;
			String[Length] = '\0';
			Flag_To_Redraw();
			break;
		}
	}
	return(true);
}


/// <summary>
/// Gives the keyboard focus to this edit gadget.
/// This routine will resynchronize the edit length with whatever text the gadget was
/// handed before letting the normal focus processing proceed, so that typing continues
/// from the end of the existing text rather than overwriting it.
/// </summary>
void EditClass::Set_Focus(void)
{
	Length = 0;
	if (String) {
		Length = strlen(String);
	}
	BASECLASS::Set_Focus();
}
