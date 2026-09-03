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

/* $Header: /CounterStrike/LIST.CPP 1     3/03/97 10:25a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : LIST.CPP                                                     *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : 01/15/95                                                     *
 *                                                                                             *
 *                  Last Update : January 23, 1996 [JLB]                                       *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   ListClass::Add -- This object adds itself to the given list                               *
 *   ListClass::Add_Head -- This gadget makes itself the head of the given list.               *
 *   ListClass::Add_Item -- Adds a text item (as number) to the list box.                      *
 *   ListClass::Add_Item -- Adds an item to the list box.                                      *
 *   ListClass::Add_Scroll_Bar -- Adds a scroll bar to the list box.                           *
 *   ListClass::Add_Tail -- Add myself to the end of the given list.                           *
 *   ListClass::Bump -- Bumps the list box up/down one "page".                                 *
 *   ListClass::Current_Index -- Fetches the current selected index.                           *
 *   ListClass::Current_Item -- Fetches pointer to current item string.                        *
 *   ListClass::Draw_Entry -- Draws a list box text line as indicated.                         *
 *   ListClass::Draw_Me -- Draws the listbox.                                                  *
 *   ListClass::Get_Item -- Fetches an arbitrary item string.                                  *
 *   ListClass::Peer_To_Peer -- A peer gadget was touched -- make adjustments.                 *
 *   ListClass::Remove -- Removes the specified object from the list.                          *
 *   ListClass::Remove_Item -- Remove specified text from list box.                            *
 *   ListClass::Remove_Scroll_Bar -- Removes the scroll bar if present                         *
 *   ListClass::Set_Selected_Index -- Set the top of the listbox to index specified.           *
 *   ListClass::Set_Tabs -- Sets the tab stop list to be used for text printing.               *
 *   ListClass::Set_View_Index -- Sets the top line for the current list view.                 *
 *   ListClass::Step -- Moves the list view one line in direction specified.                   *
 *   ListClass::Step_Selected_Index -- Change the listbox top line in direction specified.     *
 *   ListClass::~ListClass -- Destructor for list class objects.                               *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "list.h"

#include "_convert.h"
#include "_surface.h"
#include "convert.h"
#include "data.h"
#include "dialog.h"
#include "dsurface.h"
#include "font.h"
#include "language/language.h"
#include "scheme.h"
#include "vector.h"
#include "wwmouse.h"

#include <algorithm>


/***************************************************************************
 * ListClass::ListClass -- class constructor                               *
 *                                                                         *
 * INPUT:            id               button ID                            *
 *                                                                         *
 *                     x,y            upper-left corner, in pixels         *
 *                                                                         *
 *                     w,h            width, height, in pixels             *
 *                                                                         *
 *                     list            ptr to array of char strings to list*
 *                                                                         *
 *                   flags, style   flags for mouse, style of listbox      *
 *                                                                         *
 * OUTPUT:           none.                                                 *
 *                                                                         *
 * WARNINGS:         none.                                                 *
 *                                                                         *
 * HISTORY:          01/05/1995 MML : Created.                             *
 *=========================================================================*/
ListClass::ListClass (int id, int x, int y, int w, int h, TextPrintType flags, ShapeSet const * up, ShapeSet const * down) :
	BASECLASS(id, x, y, w, h, LEFTPRESS | LEFTRELEASE | KEYBOARD, false),
	UpGadget(0, up, x+w, y),
	DownGadget(0, down, x+w, y+h),
	ScrollGadget(0, x+w, y, 0, h, true)
{
	/*
	**	Set preliminary values for the slider related gadgets. They don't automatically
	**	appear at this time, but there are some values that can be pre-filled in.
	*/
	UpGadget.X -= UpGadget.Width;
	DownGadget.X -= DownGadget.Width;
	DownGadget.Y -= DownGadget.Height;
	ScrollGadget.X -= std::max(UpGadget.Width, DownGadget.Width);
	ScrollGadget.Y = Y+UpGadget.Height;
	ScrollGadget.Height -= UpGadget.Height + DownGadget.Height;
	ScrollGadget.Width = std::max(UpGadget.Width, DownGadget.Width);

	/*
	**	Set the list box to a default state.
	*/
	TextFlags = flags;
	IsScrollActive = false;
	Tabs = 0;
	SelectedIndex = 0;
	CurrentTopIndex = 0;
	FontClass *font = Font_From_TPF(TextFlags);
	LineHeight = font->Get_Height()-1;
	LineCount = (h-1) / LineHeight;
}


/// <summary>
/// Copy constructor for list box objects.
/// The copy takes over the source list's entries and scroll bar settings, but the scroll bar
/// gadgets are made peers of the new list box so that they report to it rather than to the
/// list they were copied from.
/// </summary>
/// <param name="list">The list box to copy.</param>
ListClass::ListClass(ListClass const & list) :
	BASECLASS(list),
	TextFlags(list.TextFlags),
	Tabs(list.Tabs),
	List(list.List),
	LineHeight(list.LineHeight),
	LineCount(list.LineCount),
	IsScrollActive(list.IsScrollActive),
	UpGadget(list.UpGadget),
	DownGadget(list.DownGadget),
	ScrollGadget(list.ScrollGadget),
	SelectedIndex(list.SelectedIndex),
	CurrentTopIndex(list.CurrentTopIndex)
{
	UpGadget.Make_Peer(*this);
	DownGadget.Make_Peer(*this);
	ScrollGadget.Make_Peer(*this);
}


/// <summary>
/// Sets the display position of the list box.
/// This routine moves the list box and drags its scroll bar gadgets along with it, so that
/// the arrow buttons and the slider stay pinned to the right hand edge of the list.
/// </summary>
/// <param name="x">The new X pixel coordinate of the upper left corner.</param>
/// <param name="y">The new Y pixel coordinate of the upper left corner.</param>
void ListClass::Set_Position(int x, int y)
{
	UpGadget.X = x + Width - UpGadget.Width;
	UpGadget.Y = y;
	DownGadget.X = x + Width - DownGadget.Width;
	DownGadget.Y = y + Height - DownGadget.Height;
	ScrollGadget.X = x + Width - std::max(UpGadget.Width, DownGadget.Width);
	ScrollGadget.Y = y + UpGadget.Height;
	ScrollGadget.Height = Height - (UpGadget.Height + DownGadget.Height);
	ScrollGadget.Width = std::max(UpGadget.Width, DownGadget.Width);
}


/***********************************************************************************************
 * ListClass::~ListClass -- Destructor for list class objects.                                 *
 *                                                                                             *
 *    This is the destructor for list objects. It handles removing anything it might have      *
 *    allocated. This is typically the scroll bar.                                             *
 *                                                                                             *
 * INPUT:      none                                                                            *
 * OUTPUT:     none                                                                            *
 * WARNINGS:   none                                                                            *
 * HISTORY:    01/16/1995 JLB : Created.                                                       *
 *=============================================================================================*/
ListClass::~ListClass(void)
{
	Remove_Scroll_Bar();
}


/***********************************************************************************************
 * ListClass::Add_Item -- Adds an item to the list box.                                        *
 *                                                                                             *
 *    This will add the specified string to the list box. The string is added to the end       *
 *    of the list.                                                                             *
 *                                                                                             *
 * INPUT:      text  -- Pointer to the string to add to the list box.                          *
 * OUTPUT:     none                                                                            *
 * WARNINGS:   none                                                                            *
 * HISTORY:    01/15/1995 JLB : Created.                                                       *
 *=============================================================================================*/
int ListClass::Add_Item(char const * text)
{
	if (text) {
		List.Add(text);
		Flag_To_Redraw();

		/*
		**	Add scroll gadget if the list gets too large to display all of the items
		**	at the same time.
		*/
		if (List.Count() > LineCount) {
			Add_Scroll_Bar();
		}

		/*
		**	Tell the slider that there is one more entry in the list.
		*/
		if (IsScrollActive) {
			ScrollGadget.Set_Maximum(List.Count());
		}
	}
	return(List.Count() - 1);
}


/***********************************************************************************************
 * ListClass::Add_Item -- Adds a text item (as number) to the list box.                        *
 *                                                                                             *
 *    This will add the text as specified by the text number provided, to the list box.        *
 *    The string is added to the end of the list.                                              *
 *                                                                                             *
 * INPUT:      text  -- The text number for the string to add to the list box.                 *
 * OUTPUT:     none                                                                            *
 * WARNINGS:   Once a string is added to the list box in this fashion, there is no method of   *
 *             retrieving the text number as it relates to any particular index in the list.   *
 * HISTORY:                                                                                    *
 *   01/15/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int ListClass::Add_Item(int text)
{
	if (text != TXT_NONE) {
		Add_Item(Fetch_String(text));
	}
	return(List.Count() - 1);
}


/// <summary>
/// Removes the entry at the index specified.
/// This routine will drop the entry from the list box and then bring the scroll bar and the
/// selection back into agreement with the shortened list. An index that is not in the list
/// is quietly ignored.
/// </summary>
/// <param name="index">The index of the entry to remove.</param>
void ListClass::Remove_Item(int index)
{
	if ((unsigned)index < (unsigned)List.Count()) {
		List.Delete_Index(index);

		/*
		**	If the list is now small enough to display completely within the list box region,
		**	then delete the slider gadget (if they are present).
		*/
		if (List.Count() <= LineCount) {
			Remove_Scroll_Bar();
		}

		/*
		**	Tell the slider that there is one less entry in the list.
		*/
		if (IsScrollActive) {
			ScrollGadget.Set_Maximum(List.Count());
		}

		/*
		**	If we just removed the selected entry, select the previous one
		*/
		if (SelectedIndex >= List.Count()) {
			SelectedIndex--;
			if (SelectedIndex < 0) {
				SelectedIndex = 0;
			}
		}

		/*
		**	If we just removed the top-displayed entry, step up one item
		*/
		if (CurrentTopIndex >= List.Count()) {
			CurrentTopIndex--;
			if (CurrentTopIndex < 0)
				CurrentTopIndex = 0;
			if (IsScrollActive)
				ScrollGadget.Step(1);
		}
	}
}


/***********************************************************************************************
 * ListClass::Remove_Item -- Remove specified text from list box.                              *
 *                                                                                             *
 *    This routine will remove the specified text string from the list box.                    *
 *                                                                                             *
 * INPUT:      text  -- Pointer to the string to remove.                                       *
 *                                                                                             *
 * OUTPUT:     none                                                                            *
 *                                                                                             *
 * WARNINGS:   The text pointer passed into this routine MUST be the same text pointer that    *
 *             was used to add the string to the list.                                         *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/15/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void ListClass::Remove_Item(char const * text)
{
	if (text) {
		Remove_Item(List.ID(text));
	}
}


/***************************************************************************
 * ListClass::Action -- If clicked on, do this!                            *
 *                                                                         *
 * INPUT:            int flags -- combination of mouse flags indicating    *
 *                                  what action to take.                   *
 *                                                                         *
 * OUTPUT:           bool result.                                          *
 *                                                                         *
 * WARNINGS:         none.                                                 *
 *                                                                         *
 * HISTORY:          01/05/1995 MML : Created.                             *
 *=========================================================================*/
int ListClass::Action(unsigned flags, KeyNumType & key)
{
	if (flags & LEFTRELEASE) {
		key = KN_NONE;
		flags &= (~LEFTRELEASE);
		BASECLASS::Action(flags, key);
		return(true);
	} else {

		/*
		**	Handle keyboard events here.
		*/
		if (flags & KEYBOARD) {

			/*
			**	Process the keyboard character. If indicated, consume this keyboard event
			**	so that the edit gadget ID number is not returned.
			*/
			if (key == KN_UP) {
				Step_Selected_Index(-1);
				key = KN_NONE;
			} else if (key == KN_DOWN) {
				Step_Selected_Index(1);
				key = KN_NONE;
			} else {
				flags &= ~KEYBOARD;
			}

		} else {

			int index = Get_Mouse_Y() - (Y+1);
			index = index / LineHeight;
			SelectedIndex = CurrentTopIndex + index;
			SelectedIndex = std::min(SelectedIndex, List.Count()-1);
			if (SelectedIndex == -1) SelectedIndex = 0;
		}
	}
	return(BASECLASS::Action(flags, key));
}


/***********************************************************************************************
 * ListClass::Draw_Me -- Draws the listbox.                                                    *
 *                                                                                             *
 *    This routine will render the listbox.                                                    *
 *                                                                                             *
 * INPUT:   forced   -- Should the listbox be redrawn even if it already thinks it doesn't     *
 *                      need to be? This is true when something outside of the gadget system   *
 *                      has trashed the screen.                                                *
 *                                                                                             *
 * OUTPUT:  Was the listbox redrawn?                                                           *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/25/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int ListClass::Draw_Me(int forced)
{
	if (GadgetClass::Draw_Me(forced)) {

		/*
		**	Turn off the mouse.
		*/
		Conditional_Hide_Mouse(Rect(X, Y, Width, Height));

		Draw_Box (Rect(X, Y, Width, Height), BOXSTYLE_BOX, true);

		/*
		**	Draw List.
		*/
		if (List.Count()) {
			for (int index = 0; index < LineCount; index++)  {
				int line = CurrentTopIndex + index;

				if (List.Count() > line) {

					/*
					**	Prints the text and handles right edge clipping and tabs.
					*/
					Draw_Entry(line, X+1, Y+(LineHeight*index)+1, Width-2, (line == SelectedIndex));
				}
			}
		}

		/*
		**	Turn on the mouse.
		*/
			Conditional_Show_Mouse();
		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * ListClass::Bump -- Bumps the list box up/down one "page".                                   *
 *                                                                                             *
 *    Use this routine to adjust the "page" that is being viewed in the list box. The view     *
 *    will move up or down (as specified) one page (screen full) of text strings.              *
 *                                                                                             *
 * INPUT:   up -- Should the adjustment be up?                                                 *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/15/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void ListClass::Bump(int up)
{
	if (IsScrollActive) {
		if (ScrollGadget.Step(up)) {
			CurrentTopIndex = ScrollGadget.Get_Value();
			Flag_To_Redraw();
		}
	}
}


/***********************************************************************************************
 * ListClass::Step -- Moves the list view one line in direction specified.                     *
 *                                                                                             *
 *    This routine will move the current view "page" one line in the direction specified.      *
 *                                                                                             *
 * INPUT:   up -- Should the view be moved upward?                                             *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/15/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void ListClass::Step(int up)
{
	if (IsScrollActive) {
		if (ScrollGadget.Step(up)) {
			CurrentTopIndex = ScrollGadget.Get_Value();
			Flag_To_Redraw();
		}
	}
}


/***********************************************************************************************
 * ListClass::Get_Item -- Fetches an arbitrary item string.                                    *
 *                                                                                             *
 *    This routine will fetch an item string from the list box. The item fetched can be any    *
 *    one of the ones in the list.                                                             *
 *                                                                                             *
 * INPUT:   index -- The index to examine and return the text pointer from.                    *
 *                                                                                             *
 * OUTPUT:  Returns with the text pointer to the string at the index position specified.       *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/16/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
char const * ListClass::Get_Item(int index) const
{
	if (List.Count() == 0) {
		return(NULL);
	}
	index = std::min(index, List.Count()-1);
	return(List[index]);
}


/***********************************************************************************************
 * ListClass::Current_Item -- Fetches pointer to current item string.                          *
 *                                                                                             *
 *    This routine will fetch a pointer to the currently selected item's text.                 *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Return with pointer to currently selected text.                                    *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/16/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
char const * ListClass::Current_Item(void) const
{
	if (List.Count() <= SelectedIndex) {
		return(0);
	}
	return(List[SelectedIndex]);
}


/***********************************************************************************************
 * ListClass::Current_Index -- Fetches the current selected index.                             *
 *                                                                                             *
 *    This routine will fetch the index number for the currently selected line.                *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the index of the currently selected line. This ranges from zero to    *
 *          the number of items in the list minus one.                                         *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/16/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int ListClass::Current_Index(void) const
{
	return(SelectedIndex);
}


/***********************************************************************************************
 * ListClass::Peer_To_Peer -- A peer gadget was touched -- make adjustments.                   *
 *                                                                                             *
 *    This routine is called when one of the peer gadgets (the scroll arrows or the slider)    *
 *    was touched in some fashion. This routine will sort out whom and why and then make       *
 *    any necessary adjustments to the list box.                                               *
 *                                                                                             *
 * INPUT:   flags    -- The event flags that affected the peer gadget.                         *
 *                                                                                             *
 *          key      -- The key value at the time of the event.                                *
 *                                                                                             *
 *          whom     -- Which gadget is being touched.                                         *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/16/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void ListClass::Peer_To_Peer(unsigned flags, KeyNumType &, BASECLASS & whom)
{
	if (flags & LEFTRELEASE) {
		if (&whom == &UpGadget) {
			Step(true);
		}
		if (&whom == &DownGadget) {
			Step(false);
		}
	}

	/*
	**	The slider has changed, so reflect the current list position
	**	according to the slider setting.
	*/
	if (&whom == &ScrollGadget) {
		Set_View_Index(ScrollGadget.Get_Value());
	}
}


/***********************************************************************************************
 * ListClass::Set_View_Index -- Sets the top line for the current list view.                   *
 *                                                                                             *
 *    This routine is used to set the line that will be at the top of the list view. This is   *
 *    how the view can be scrolled up and down. This does not affect the currently selected    *
 *    item.                                                                                    *
 *                                                                                             *
 * INPUT:   index -- The line (index) to move to the top of the list view.                     *
 *                                                                                             *
 * OUTPUT:  bool; Was the view actually changed?                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/16/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int ListClass::Set_View_Index(int index)
{
	index = std::clamp(index, 0, std::max(0, List.Count() - LineCount));
	if (index != CurrentTopIndex) {
		CurrentTopIndex = index;
		Flag_To_Redraw();
		if (IsScrollActive) {
			ScrollGadget.Set_Value(CurrentTopIndex);
		}
		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * ListClass::Add_Scroll_Bar -- Adds a scroll bar to the list box.                             *
 *                                                                                             *
 *    This routine will add a scroll bar (with matching arrows) to the list box. They are      *
 *    added to the right edge and cause the interior of the list box to become narrower.       *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  bool; Was the scroll bar added?                                                    *
 *                                                                                             *
 * WARNINGS:   The list box becomes narrower when the scroll bar is added.                     *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/16/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int ListClass::Add_Scroll_Bar(void)
{
	if (!IsScrollActive) {
		IsScrollActive = true;

		/*
		**	Everything has been created successfully. Flag the list box to be
		**	redrawn because it now must be made narrower to accomodate the new
		**	slider gadgets.
		*/
		Flag_To_Redraw();
		Width -= ScrollGadget.Width;

		/*
		**	Tell the newly created gadgets that they should inform this list box
		**	whenever they get touched. In this way, the list box will automatically
		**	be updated under control of the slider buttons.
		*/
		UpGadget.Make_Peer(*this);
		DownGadget.Make_Peer(*this);
		ScrollGadget.Make_Peer(*this);

		/*
		**	Add these newly created gadgets to the same gadget list that the
		**	list box is part of.
		*/
		UpGadget.Add(*this);
		DownGadget.Add(*this);
		ScrollGadget.Add(*this);

		/*
		**	Make sure these added gadgets get redrawn at the next opportunity.
		*/
		UpGadget.Flag_To_Redraw();
		DownGadget.Flag_To_Redraw();
		ScrollGadget.Flag_To_Redraw();

		/*
		**	Inform the slider of the size of the window and the current view position.
		*/
		ScrollGadget.Set_Maximum(List.Count());
		ScrollGadget.Set_Thumb_Size(LineCount);
		ScrollGadget.Set_Value(CurrentTopIndex);

		/*
		**	Return with success flag.
		*/
		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * ListClass::Remove_Scroll_Bar -- Removes the scroll bar if present                           *
 *                                                                                             *
 *    Use this routine to remove any attached scroll bar to this list box. If the scroll bar   *
 *    is not present, then no action occurs.                                                   *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  bool; Was the scroll bar removed?                                                  *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/16/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int ListClass::Remove_Scroll_Bar(void)
{
	if (IsScrollActive) {
		IsScrollActive = false;
		Width += ScrollGadget.Width;
		ScrollGadget.Remove();
		UpGadget.Remove();
		DownGadget.Remove();
		Flag_To_Redraw();
		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * ListClass::Set_Tabs -- Sets the tab stop list to be used for text printing.                 *
 *                                                                                             *
 *    This sets the tab stop list to be used for text printing. It specifies a series of       *
 *    pixel offsets for each tab stop. The offsets are from the starting pixel position that   *
 *    the text begins at.                                                                      *
 *                                                                                             *
 * INPUT:   tabs  -- Pointer to a list of tab pixel offsets.                                   *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   Only a pointer to the tabs is recorded by the ListClass object. Make sure that  *
 *             the list remains intact for the duration of the existence of this object.       *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/16/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void ListClass::Set_Tabs(int const * tabs)
{
	Tabs = tabs;
}


/***********************************************************************************************
 * ListClass::Draw_Entry -- Draws a list box text line as indicated.                           *
 *                                                                                             *
 *    This routine is called by the Draw_Me function when it desired to redraw a particular    *
 *    text line in the list box.                                                               *
 *                                                                                             *
 * INPUT:   index    -- The index of the list entry to draw. This index is based on the        *
 *                      total list and NOT the current visible view page.                      *
 *                                                                                             *
 *          x,y      -- Pixel coordinates for the upper left corner of the text entry.         *
 *                                                                                             *
 *          width    -- The maximum width that the text may draw over. It is expected that     *
 *                      this drawing routine entirely fills this length.                       *
 *                                                                                             *
 *          selected -- bool; Is this a selected (highlighted) listbox entry?                  *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 * WARNINGS:   none                                                                            *
 * HISTORY:                                                                                    *
 *   01/16/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void ListClass::Draw_Entry(int index, int x, int y, int width, int selected)
{
	TextPrintType flags = TextFlags;
	int scheme = GadgetClass::Get_Color_Scheme();

	if (selected) {
		flags = TextPrintType(flags | TPF_BRIGHT_COLOR);
		int shadow = ColorSchemes[scheme]->Shadow;
		LogicalSurface->Fill_Rect (Rect(x, y, width, LineHeight), NormalDrawer->Convert_Pixel(shadow));
	} else {
		if (!(flags & TPF_USE_GRAD_PAL)) {
			flags = TextPrintType(flags | TPF_MEDIUM_COLOR);
		}
	}

	Conquer_Clip_Text_Print(List[index], *LogicalSurface, LogicalSurface->Get_Rect(), Point2D(x, y), ColorSchemes[scheme], TBLACK, flags, width, Tabs);

}


/***********************************************************************************************
 * ListClass::Add -- Adds myself to list immediately after given object                        *
 *                                                                                             *
 * Adds the list box to the chain, immediately after the given object.  The order will be:     *
 * - Listbox                                                                                   *
 * - Up arrow (if active)                                                                      *
 * - Down arrow (if active)                                                                    *
 * - Scroll gadget (if active)                                                                 *
 *                                                                                             *
 * INPUT:   object   -- Pointer to the object to be added right after this one.                *
 *                                                                                             *
 * OUTPUT:  Returns with a pointer to the head of the list.                                    *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/19/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
LinkClass & ListClass::Add(LinkClass & list)
{
	/*
	**	Add the scroll bar gadgets if they're active.
	*/
	if (IsScrollActive) {
		ScrollGadget.Add(list);
		DownGadget.Add(list);
		UpGadget.Add(list);
	}

	/*
	**	Add myself to the list, then return.
	*/
	return(BASECLASS::Add(list));
}


/***********************************************************************************************
 * ListClass::Add_Head -- Adds myself to head of the given list                                *
 *                                                                                             *
 * INPUT:   list -- list to add myself to                                                      *
 *                                                                                             *
 * OUTPUT:  Returns with a reference to the object at the head of the list. This should be     *
 *          the same object that is passed in.                                                 *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/19/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
LinkClass & ListClass::Add_Head(LinkClass & list)
{
	/*
	**	Add the scroll bar gadgets if they're active.
	*/
	if (IsScrollActive) {
		ScrollGadget.Add_Head(list);
		DownGadget.Add_Head(list);
		UpGadget.Add_Head(list);
	}

	/*
	**	Add myself to the list, then return.
	*/
	return(BASECLASS::Add_Head(list));
}


/***********************************************************************************************
 * ListClass::Add_Tail -- Adds myself to tail of given list                                    *
 *                                                                                             *
 * Adds the list box to the tail of the give chain.  The order will be:                        *
 * - Listbox                                                                                   *
 * - Up arrow (if active)                                                                      *
 * - Down arrow (if active)                                                                    *
 * - Scroll gadget (if active)                                                                 *
 *                                                                                             *
 * INPUT:   list -- list to add myself to                                                      *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   The previous and next pointers for the added object MUST have been properly     *
 *             initialized for this routine to work correctly.                                 *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/15/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
LinkClass & ListClass::Add_Tail(LinkClass & list)
{
	/*
	**	Add myself to the list.
	*/
	BASECLASS::Add_Tail(list);

	/*
	**	Add the scroll bar gadgets if they're active.
	*/
	if (IsScrollActive) {
		UpGadget.Add_Tail(list);
		DownGadget.Add_Tail(list);
		ScrollGadget.Add_Tail(list);
	}

	return(Head_Of_List());
}


/***********************************************************************************************
 * ListClass::Remove -- Removes the specified object from the list.                            *
 *                                                                                             *
 *    This routine will remove the specified object from the list of objects. Because of the   *
 *    previous and next pointers, it is possible to remove an object from the list without     *
 *    knowing the head of the list. To do this, just call Remove() with the parameter of       *
 *    "this".                                                                                  *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the new head of list.                                                 *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/15/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
LinkClass * ListClass::Remove(void)
{
	/*
	**	Remove the scroll bar if it's active
	*/
	if (IsScrollActive) {
		ScrollGadget.Remove();
		DownGadget.Remove();
		UpGadget.Remove();
	}

	/*
	**	Remove myself & return
	*/
	return(BASECLASS::Remove());
}


/***********************************************************************************************
 * ListClass::Set_Selected_Index -- Set the top of the listbox to index specified.             *
 *                                                                                             *
 *    This routine will set the top line of the listbox to the index value specified.          *
 *                                                                                             *
 * INPUT:   index -- The index to set the top of the listbox to.                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   The requested index may be adjusted to fit within legal parameters.             *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/25/1995 JLB : Created.                                                                 *
 *   01/23/1996 JLB : Forces selected index to always be zero for a null list.                 *
 *=============================================================================================*/
void ListClass::Set_Selected_Index(int index)
{
   if ((unsigned)index < (unsigned)List.Count()) {
		SelectedIndex = index;
		Flag_To_Redraw();
		if (SelectedIndex < CurrentTopIndex) {
			Set_View_Index(SelectedIndex);
		}
		if (SelectedIndex >= CurrentTopIndex+LineCount) {
			Set_View_Index(SelectedIndex-(LineCount-1));
		}
   } else {
   	SelectedIndex = 0;
   }
}


/***********************************************************************************************
 * ListClass::Step_Selected_Index -- Change the listbox top line in direction specified.       *
 *                                                                                             *
 *    This routine will scroll the top line of the listbox in the direction specified.         *
 *                                                                                             *
 * INPUT:   step  -- The direction (and amount) to adjust the listbox. If negative value, then *
 *                   the top line is scrolled upward.                                          *
 *                                                                                             *
 * OUTPUT:  Returns with the original top line index number.                                   *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/25/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int ListClass::Step_Selected_Index(int step)
{
	int old = SelectedIndex;

	Set_Selected_Index(old + step);
	return(old);
}


/// <summary>
/// Flags the list box to be redrawn.
/// This routine marks the list box, and any scroll bar gadgets it owns, as needing to be
/// redrawn at the next opportunity. The scroll bar pieces are peers rather than children,
/// so they must be flagged along with the list itself.
/// </summary>
void ListClass::Flag_To_Redraw(void)
{
	if (IsScrollActive) {
		UpGadget.Flag_To_Redraw();
		DownGadget.Flag_To_Redraw();
		ScrollGadget.Flag_To_Redraw();
	}
	BASECLASS::Flag_To_Redraw();
}


/// <summary>
/// Sets the selected entry to the one matching the text specified.
/// This routine is used when the caller knows what the entry says but not where it sits in
/// the list. The comparison ignores case. If no entry matches, the selection is left alone.
/// </summary>
/// <param name="text">The text of the entry to select.</param>
void ListClass::Set_Selected_Index(char const * text)
{
	if (text && List.Count() > 0) {
		for (int index = 0; index < List.Count(); index++) {
			if (stricmp(List[index], text) == 0) {
				Set_Selected_Index(index);
				break;
			}
		}
	}
}
