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
 *                     $Archive:: /G/wwlib/KEYBOARD.CPP                                       $*
 *                                                                                             *
 *                      $Author:: Eric_c                                                      $*
 *                                                                                             *
 *                     $Modtime:: 4/15/99 10:15a                                              $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   WWKeyboardClass::Buff_Get -- Lowlevel function to get a key from key buffer               *
 *   WWKeyboardClass::Check -- Checks to see if a key is in the buffer                         *
 *   WWKeyboardClass::Clear -- Clears the keyboard buffer.                                     *
 *   WWKeyboardClass::Down -- Checks to see if the specified key is being held down.           *
 *   WWKeyboardClass::Fetch_Element -- Extract the next element in the keyboard buffer.        *
 *   WWKeyboardClass::Fill_Buffer_From_Syste -- Extract and process any queued windows messages*
 *   WWKeyboardClass::Get -- Logic to get a metakey from the buffer                            *
 *   WWKeyboardClass::Get_Mouse_X -- Returns the mouses current x position in pixels           *
 *   WWKeyboardClass::Get_Mouse_XY -- Returns the mouses x,y position via reference vars       *
 *   WWKeyboardClass::Get_Mouse_Y -- returns the mouses current y position in pixels           *
 *   WWKeyboardClass::Is_Buffer_Empty -- Checks to see if the keyboard buffer is empty.        *
 *   WWKeyboardClass::Is_Buffer_Full -- Determines if the keyboard buffer is full.             *
 *   WWKeyboardClass::Is_Mouse_Key -- Checks to see if specified key refers to the mouse.      *
 *   WWKeyboardClass::Message_Handler -- Process a windows message as it relates to the keyboar*
 *   WWKeyboardClass::Peek_Element -- Fetches the next element in the keyboard buffer.         *
 *   WWKeyboardClass::Put -- Logic to insert a key into the keybuffer]                         *
 *   WWKeyboardClass::Put_Element -- Put a keyboard data element into the buffer.              *
 *   WWKeyboardClass::Put_Key_Message -- Translates and inserts wParam into Keyboard Buffer    *
 *   WWKeyboardClass::To_ASCII -- Convert the key value into an ASCII representation.          *
 *   WWKeyboardClass::Available_Buffer_Room -- Fetch the quantity of free elements in the keybo*
 *   WWKeyboardClass::Put_Mouse_Message -- Stores a mouse type message into the keyboard buffer*
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "keyboard.h"

#include "_xmouse.h"
#include "msgloop.h"
#include "platform/platform.h"
#include "platform/windowevent.hh"
#include "vidscale.h"

#include <cmath>


#define	ARRAY_SIZE(x)		int(sizeof(x)/sizeof(x[0]))


/// <summary>
/// Halts the game for a waiting programmer.
/// This routine is what the Scroll Lock key calls. It does nothing on its own -- the
/// point of it is to be somewhere convenient to hang a breakpoint, so that the game can
/// be stopped from the keyboard at an interesting moment.
/// </summary>
void Stop_Execution (void)
{
	//	__asm nop			// Is this line needed?
}


// SDL sends no text for these keys, so the queue supplies the character each one types.
static int Control_Character(unsigned short key)
{
	switch (key & 0xFF) {
		case VK_ESCAPE:	return(0x1B);
		case VK_RETURN:	return(0x0D);
		case VK_BACK:	return(0x08);
		case VK_TAB:	return(0x09);
		default:		return(0);
	}
}


/***********************************************************************************************
 * WWKeyboardClass::WWKeyBoardClass -- Construction for Westwood Keyboard Class                *
 *                                                                                             *
 * INPUT:      none                                                                            *
 *                                                                                             *
 * OUTPUT:     none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/16/1995 PWG : Created.                                                                 *
 *=============================================================================================*/
WWKeyboardClass::WWKeyboardClass(void) :
	MouseQX(0),
	MouseQY(0),
	MousePos(0,0),
	TextSlot(-1),
	DropText(false),
	FetchedKey(0),
	FetchedText(0),
	Head(0),
	Tail(0)
{
	memset(Text, '\0', sizeof(Text));
}


/***********************************************************************************************
 * WWKeyboardClass::Buff_Get -- Lowlevel function to get a key from key buffer                 *
 *                                                                                             *
 * INPUT:      none                                                                            *
 *                                                                                             *
 * OUTPUT:     int      - the key value that was pulled from buffer (includes bits)            *
 *                                                                                             *
 * WARNINGS:   If the key was a mouse event MouseQX and MouseQY will be updated                *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/17/1995 PWG : Created.                                                                 *
 *=============================================================================================*/
unsigned short WWKeyboardClass::Buff_Get(void)
{
	while (!Check()) {}					// wait for key in buffer

	int const slot = Head;
	unsigned short temp = Fetch_Element();
	FetchedKey = temp;
	FetchedText = Text[slot];
	if (Is_Mouse_Key(temp)) {
		MouseQX = Fetch_Element();
		MouseQY = Fetch_Element();
		MousePos = Point2D(MouseQX, MouseQY);
	}
	return(temp);
}


/***********************************************************************************************
 * WWKeyboardClass::Is_Mouse_Key -- Checks to see if specified key refers to the mouse.        *
 *                                                                                             *
 *    This checks the specified key code to see if it refers to the mouse buttons.             *
 *                                                                                             *
 * INPUT:   key   -- The key to check.                                                         *
 *                                                                                             *
 * OUTPUT:  bool; Is the key a mouse button key?                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/30/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
bool WWKeyboardClass::Is_Mouse_Key(unsigned short key)
{
	key &= 0xFF;
	return(key == VK_LBUTTON || key == VK_MBUTTON || key == VK_RBUTTON);
}


/***********************************************************************************************
 * WWKeyboardClass::Check -- Checks to see if a key is in the buffer                           *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/16/1995 PWG : Created.                                                                 *
 *   09/24/1996 JLB : Converted to new style keyboard system.                                  *
 *=============================================================================================*/
unsigned short WWKeyboardClass::Check(void) const
{
	((WWKeyboardClass *)this)->Fill_Buffer_From_System();
	if (Is_Buffer_Empty()) return(false);
	return(Peek_Element());
}


/***********************************************************************************************
 * WWKeyboardClass::Get -- Logic to get a metakey from the buffer                              *
 *                                                                                             *
 * INPUT:      none                                                                            *
 *                                                                                             *
 * OUTPUT:     int      - the meta key taken from the buffer.                                  *
 *                                                                                             *
 * WARNINGS:   This routine will not return until a keypress is received                       *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/16/1995 PWG : Created.                                                                 *
 *=============================================================================================*/
unsigned short WWKeyboardClass::Get(void)
{
	while (!Check()) {}								// wait for key in buffer
	return(Buff_Get());
}


/***********************************************************************************************
 * WWKeyboardClass::Put -- Logic to insert a key into the keybuffer]                           *
 *                                                                                             *
 * INPUT:      int       - the key to insert into the buffer                                   *
 *                                                                                             *
 * OUTPUT:     bool      - true if key is sucessfuly inserted.                                 *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/16/1995 PWG : Created.                                                                 *
 *=============================================================================================*/
bool WWKeyboardClass::Put(unsigned short key)
{
	if (!Is_Buffer_Full()) {
		Put_Element(key);
		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * WWKeyboardClass::Put_Key_Message -- Translates and inserts wParam into Keyboard Buffer      *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/16/1995 PWG : Created.                                                                 *
 *=============================================================================================*/
bool WWKeyboardClass::Put_Key_Message(unsigned short vk_key, bool release, int modifiers)
{
	/*
	**	Get the status of all of the different keyboard modifiers.  Note that we do not
	**	want to set the shift, ctrl and alt bits for Mouse keypresses as this would be
	**	incompatible with the dos version.
	*/
	if (!Is_Mouse_Key(vk_key)) {
		if ((modifiers & WINDOW_MOD_SHIFT) != 0) {
			vk_key |= WWKEY_SHIFT_BIT;
		}
		if ((modifiers & WINDOW_MOD_CTRL) != 0) {
			vk_key |= WWKEY_CTRL_BIT;
		}
		if ((modifiers & WINDOW_MOD_ALT) != 0) {
			vk_key |= WWKEY_ALT_BIT;
		}
	}

	if (release) {
		vk_key |= WWKEY_RLS_BIT;
	}

	/*
	**	Finally use the put command to enter the key into the keyboard
	**	system.
	*/
	int const slot = Tail;
	bool const queued = Put(vk_key);

	// The character a key press types arrives after it and belongs to it.
	if (!release && !Is_Mouse_Key(vk_key)) {
		TextSlot = (queued && Control_Character(vk_key) == 0) ? slot : -1;
		DropText = false;
	}
	return(queued);
}


/// <summary>
/// Queues a typed character with the key press that typed it, or as a KN_TEXT entry of its
/// own when no queued press can take it: the press already has one, has left the queue, or
/// is Esc, Enter, Backspace or Tab. A character typed by a repeat of a held key is dropped,
/// as the repeat itself is.
/// </summary>
/// <returns>False when the character was dropped or the queue is full.</returns>
bool WWKeyboardClass::Put_Text(char32_t code)
{
	if (DropText) {
		return(false);
	}

	if (TextSlot >= 0 && Text[TextSlot] == 0) {
		Text[TextSlot] = code;
		TextSlot = -1;
		return(true);
	}

	int const slot = Tail;
	if (!Put(KN_TEXT)) {
		return(false);
	}
	Text[slot] = code;
	return(true);
}


/***********************************************************************************************
 * WWKeyboardClass::Put_Mouse_Message -- Stores a mouse type message into the keyboard buffer. *
 *                                                                                             *
 *    This routine will store the mouse type event into the keyboard buffer. It also checks    *
 *    to ensure that there is enough room in the buffer so that partial mouse events won't     *
 *    be recorded.                                                                             *
 *                                                                                             *
 * INPUT:   vk_key   -- The mouse key message itself.                                          *
 *                                                                                             *
 *          x,y      -- The mouse coordinates at the time of the event.                        *
 *                                                                                             *
 *          release  -- Is this a mouse button release?                                        *
 *                                                                                             *
 * OUTPUT:  bool; Was the event stored sucessfully into the keyboard buffer?                   *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/02/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
bool WWKeyboardClass::Put_Mouse_Message(unsigned short vk_key, int x, int y, bool release)
{
	if (Available_Buffer_Room() >= 3 && Is_Mouse_Key(vk_key)) {
		Put_Key_Message(vk_key, release);
		Put((unsigned short)x);
		Put((unsigned short)y);
		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * WWKeyboardClass::To_ASCII -- Convert the key value into the character it typed.             *
 *                                                                                             *
 *    This routine will convert the key code specified into the character it typed. This       *
 *    takes into consideration the language and keyboard mapping of the host system. Only      *
 *    the key the last Get returned carries its character; Esc, Enter, Backspace and Tab       *
 *    give their control characters whichever entry they come from.                            *
 *                                                                                             *
 * INPUT:   key   -- The key code to convert; the key the last Get returned.                   *
 *                                                                                             *
 * OUTPUT:  Returns with the key converted into its Unicode character. If the key is a         *
 *          release or typed no character, then '\0' is returned.                              *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/30/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
int WWKeyboardClass::To_ASCII(unsigned short key)
{
	/*
	**	Released keys never translate into a character.
	*/
	if (key & WWKEY_RLS_BIT) {
		return(0);
	}

	if (key == FetchedKey && FetchedText != 0) {
		return((int)FetchedText);
	}

	return(Control_Character(key));
}


/***********************************************************************************************
 * WWKeyboardClass::Down -- Checks to see if the specified key is being held down.             *
 *                                                                                             *
 *    This routine will examine the key specified to see if it is currently being held down.   *
 *                                                                                             *
 * INPUT:   key   -- The key to check.                                                         *
 *                                                                                             *
 * OUTPUT:  bool; Is the specified key currently being held down?                              *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/30/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
bool WWKeyboardClass::Down(unsigned short key)
{
	return(Platform_Key_Down(key & 0xFF));
}


/***********************************************************************************************
 * WWKeyboardClass::Fetch_Element -- Extract the next element in the keyboard buffer.          *
 *                                                                                             *
 *    This routine will extract the next pending element in the keyboard queue. If there is    *
 *    no element available, then NULL is returned.                                             *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the element extracted from the queue. An empty queue is signified     *
 *          by a 0 return value.                                                               *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/30/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
unsigned short WWKeyboardClass::Fetch_Element(void)
{
	unsigned short val = 0;
	if (Head != Tail) {
		if (Head == TextSlot) {
			TextSlot = -1;
		}
		val = Buffer[Head];

		Head = (Head + 1) % ARRAY_SIZE(Buffer);
	}
	return(val);
}


/***********************************************************************************************
 * WWKeyboardClass::Peek_Element -- Fetches the next element in the keyboard buffer.           *
 *                                                                                             *
 *    This routine will examine and return with the next element in the keyboard buffer but    *
 *    it will not alter or remove that element. Use this routine to see what is pending in     *
 *    the keyboard queue.                                                                      *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the next element in the keyboard queue. If the keyboard buffer is     *
 *          empty, then 0 is returned.                                                         *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/30/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
unsigned short WWKeyboardClass::Peek_Element(void) const
{
	if (!Is_Buffer_Empty()) {
		return(Buffer[Head]);
	}
	return(0);
}


/***********************************************************************************************
 * WWKeyboardClass::Put_Element -- Put a keyboard data element into the buffer.                *
 *                                                                                             *
 *    This will put one keyboard data element into the keyboard buffer. Typically, this data   *
 *    is a key code, but it might be mouse coordinates.                                        *
 *                                                                                             *
 * INPUT:   val   -- The data element to add to the keyboard buffer.                           *
 *                                                                                             *
 * OUTPUT:  bool; Was the keyboard element added successfully? A failure would indicate that   *
 *                the keyboard buffer is full.                                                 *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/30/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
bool WWKeyboardClass::Put_Element(unsigned short val)
{
	if (!Is_Buffer_Full()) {
		int temp = (Tail+1) % ARRAY_SIZE(Buffer);
		Buffer[Tail] = val;
		Text[Tail] = 0;
		Tail = temp;
		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * WWKeyboardClass::Is_Buffer_Full -- Determines if the keyboard buffer is full.               *
 *                                                                                             *
 *    This routine will examine the keyboard buffer to determine if it is completely           *
 *    full of queued keyboard events.                                                          *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  bool; Is the keyboard buffer completely full?                                      *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/30/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
bool WWKeyboardClass::Is_Buffer_Full(void) const
{
	if ((Tail + 1) % ARRAY_SIZE(Buffer) == Head) {
		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * WWKeyboardClass::Is_Buffer_Empty -- Checks to see if the keyboard buffer is empty.          *
 *                                                                                             *
 *    This routine will examine the keyboard buffer to see if it contains no events at all.    *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  bool; Is the keyboard buffer currently without any pending events queued?          *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/30/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
bool WWKeyboardClass::Is_Buffer_Empty(void) const
{
	if (Head == Tail) {
		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * WWKeyboardClass::Fill_Buffer_From_Syste -- Extract and process any queued windows messages. *
 *                                                                                             *
 *    This routine will extract and process any windows messages in the windows message        *
 *    queue. It is presumed that the normal message handler will call the keyboard             *
 *    message processing function.                                                             *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/30/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void WWKeyboardClass::Fill_Buffer_From_System(void)
{
	if (!Is_Buffer_Full()) {
		Windows_Message_Handler();
//		MSG	msg;
//		while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) {
//		  	if (!GetMessage( &msg, NULL, 0, 0 )) {
//				return;
//			}
//			TranslateMessage(&msg);
//			DispatchMessage(&msg);
//		}
	}
}


/***********************************************************************************************
 * WWKeyboardClass::Clear -- Clears the keyboard buffer.                                       *
 *                                                                                             *
 *    This routine will clear the keyboard buffer of all pending keyboard events.              *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/30/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void WWKeyboardClass::Clear(void)
{
	/*
	**	Extract any windows pending keyboard message events and then clear out the keyboard
	**	buffer.
	*/
	Fill_Buffer_From_System();
	Head = Tail;
	TextSlot = -1;

	/*
	**	Perform a second clear to handle the rare case of the keyboard buffer being full and there
	**	still remains keyboard related events in the windows message queue.
	*/
	Fill_Buffer_From_System();
	Head = Tail;
	TextSlot = -1;
}


/***********************************************************************************************
 * WWKeyboardClass::Handle_Window_Event -- Process a window event as it relates to the keyboard*
 *                                                                                             *
 *    This routine will examine the window event specified. If the event relates to one that   *
 *    the keyboard input system needs to process, then it will be processed accordingly. Key   *
 *    presses, releases and typed characters are queued, and so are mouse button presses and   *
 *    releases, whose positions are held inside the frame.                                     *
 *                                                                                             *
 * INPUT:   event -- The window event to examine. Mouse positions must already be in frame     *
 *                   coordinates.                                                              *
 *                                                                                             *
 * OUTPUT:  bool; Was this keyboard event recognized and processed?                            *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/30/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
bool WWKeyboardClass::Handle_Window_Event(WindowEvent const & event)
{
	Point2D point(event.X, event.Y);
	Clamp_To_Game(point);

	/*
	**	Examine the event to see if it is one that should be processed. Only keyboard and
	**	pertinent mouse events are processed.
	*/
	switch (event.Type) {

		/*
		**	A key has been pressed. This is the normal keyboard event.
		*/
		case WINDOW_EVENT_KEY_DOWN:
			if (event.VirtualKey == VK_SCROLL) {
				Stop_Execution();
			} else if (!event.Repeat) {
				Put_Key_Message((unsigned short)event.VirtualKey, false, event.Modifiers);
			} else {
				DropText = true;
				TextSlot = -1;
			}
			return(true);

		/*
		**	The key has been released. This is the normal key release event.
		*/
		case WINDOW_EVENT_KEY_UP:
			Put_Key_Message((unsigned short)event.VirtualKey, true, event.Modifiers);
			return(true);

		case WINDOW_EVENT_TEXT:
			Put_Text(event.Text);
			return(true);

		case WINDOW_EVENT_MOUSE_DOWN:
		case WINDOW_EVENT_MOUSE_UP:
			break;

		/*
		**	If the event is not pertinent to the keyboard system,
		**	then do nothing.
		*/
		default:
			return(false);
	}

	unsigned short button;
	switch (event.Button) {
		case WINDOW_BUTTON_LEFT:
			button = VK_LBUTTON;
			break;

		case WINDOW_BUTTON_RIGHT:
			button = VK_RBUTTON;
			break;

		case WINDOW_BUTTON_MIDDLE:
			button = VK_MBUTTON;
			break;

		default:
			return(false);
	}

	if (event.Type == WINDOW_EVENT_MOUSE_UP) {
		Put_Mouse_Message(button, point.X, point.Y, true);
	} else {
		Put_Mouse_Message(button, point.X, point.Y);

		/*
		**	Double click of a mouse button. Fake this into being
		**	just a rapid click of the button twice.
		*/
		if (event.Clicks >= 2) {
			Put_Mouse_Message(button, point.X, point.Y, true);
		}
	}
	return(true);
}


/***********************************************************************************************
 * WWKeyboardClass::Available_Buffer_Room -- Fetch the quantity of free elements in the keyboa *
 *                                                                                             *
 *    This examines the keyboard buffer queue and determine how many elements are available    *
 *    for use before the buffer becomes full. Typical use of this would be when inserting      *
 *    mouse events that require more than one element. Such an event must detect when there    *
 *    would be insufficient room in the buffer and bail accordingly.                           *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the number of elements that may be stored in to the keyboard buffer   *
 *          before it becomes full and cannot accept any more elements.                        *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/02/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
int WWKeyboardClass::Available_Buffer_Room(void) const
{
#if 0
	int avail = 0;
	if (Head == Tail) {
		avail = ARRAY_SIZE(Buffer);
	}
	if (Head < Tail) {
		avail = Tail - Head;
	}
	if (Head > Tail) {
		avail = (Tail + ARRAY_SIZE(Buffer)) - Head;
	}
	return(avail);
#endif
	return(ARRAY_SIZE(Buffer) - abs(Tail - Head));
}


/// Appeared in TS 2.00
int WWKeyboardClass::Noop(void) const
{
	return(0);
}
