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
 *                     $Archive:: /Renegade Setup/Autorun/WinFix.CPP                          $*
 *                                                                                             *
 *                      $Author:: Maria_l                                                     $*
 *                                                                                             *
 *                     $Modtime:: 11/15/01 10:44a                                             $*
 *                                                                                             *
 *                    $Revision:: 6                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   Make_Identifier -- Creates a temporary string identifer.                                  *
 *   WindowsVersionInfo::WindowsVersionInfo -- Windows Version Info constructor.               *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "winfix.h"

#include "ini.h"
#include "misc.h"
#include "ownrdraw.h"
#include "trim.h"

#include <algorithm>
#include <commctrl.h>

char *TreeView_Text;
HWND TreeView_LastHandle;
static bool s_dragging;
HTREEITEM TreeView_DraggedItem;
HIMAGELIST TreeView_ImageList;

struct item_data
{
	HWND hwnd1;
	HWND hwnd2;
	HWND hwnd3;
	HTREEITEM item;
	POINT point1;
	POINT point2;
	POINT point3;
};

item_data TreeView_get_item_data(DWORD xy);
bool TreeView_On_Mouse_Move(LPARAM lParam);


/// <summary>
/// Starts dragging an item out of a tree view.
/// This routine asks the dialog that owns the tree for permission with a TVDRAG_BEGIN
/// message, and if it is granted the drag image is built, the mouse is captured, and the
/// edge scroll timer is started.
/// </summary>
/// <param name="window">The dialog that owns the tree view.</param>
/// <param name="hwndTV">The tree view the item is being dragged out of.</param>
/// <param name="lpnmtv">The begin drag notification describing the item.</param>
/// <returns>bool; Is a drag now under way?</returns>
bool TreeView_Begin_Drag(HWND window, HWND hwndTV, LPNMTREEVIEW lpnmtv)
{
	if (lpnmtv->itemNew.lParam != 0) {
		if (SendMessage(window, TVDRAG_BEGIN, (WPARAM)hwndTV, (LPARAM)lpnmtv->itemNew.hItem) == 0) {
			return(false);
		}
		TreeView_DraggedItem = lpnmtv->itemNew.hItem;

		TreeView_SelectItem(hwndTV, 0);

		/*
		 * Tell the tree-view control to create an image to use
		 * for dragging.
		 */
		TreeView_ImageList = TreeView_CreateDragImage(hwndTV, lpnmtv->itemNew.hItem);

		/*
		 * bounding rectangle of item
		 */
		RECT rcItem;
		/// Get the bounding rectangle of the item being dragged.
		TreeView_GetItemRect(hwndTV, lpnmtv->itemNew.hItem, &rcItem, false);

		/*
		 * amount that child items are indented
		 */
		int indent = TreeView_GetIndent(hwndTV);

		int hotspotx = lpnmtv->ptDrag.x - indent - rcItem.left;
		int hotspoty = lpnmtv->ptDrag.y - rcItem.top;

		/*
		 * Capture the mouse.
		 */
		SetCapture(window);

		/*
		 * Start the drag operation.
		 */
		ImageList_BeginDrag(TreeView_ImageList, 0, hotspotx, hotspoty);
		ImageList_DragEnter(window, 0, 0);

		SetTimer(window, 1, 1, NULL);

		/*
		 * Set a global flag that tells whether dragging is occurring.
		 */
		s_dragging = true;
		TreeView_LastHandle = hwndTV;
	}
	return(s_dragging);
}


/// <summary>
/// Finishes a tree view drag operation.
/// This routine tears down the drag image and gives the mouse back, then tells the
/// dialog that owns the tree to take the item with a TVDRAG_DROP message -- but only if
/// the cursor came to rest somewhere that will have it.
/// </summary>
/// <param name="lParam">The cursor position, packed as it arrives with the message.</param>
/// <returns>bool; Was a drag actually under way?</returns>
bool TreeView_End_Drag(LPARAM lParam)
{
	if (s_dragging) {

		bool ok = TreeView_On_Mouse_Move(lParam);
		ImageList_EndDrag();

		ReleaseCapture();

		ImageList_Destroy(TreeView_ImageList);

		TreeView_ImageList = NULL;
		s_dragging = false;

		item_data data;
		data = TreeView_get_item_data(lParam);

		if (ok) {
			SendMessage(data.hwnd2, TVDRAG_DROP, (WPARAM)&data, lParam);
		}

		KillTimer(data.hwnd2, 1);

		TreeView_SelectDropTarget(TreeView_LastHandle, 0);

		return(true);
	}

	return(false);
}


/// <summary>
/// Fetches the drop context under the cursor during a tree view drag.
/// This routine works out which window the cursor is over and bundles it up with the
/// item being dragged and the cursor position in each of the coordinate spaces the
/// receiver might want. The bundle is what rides along with the TVDRAG messages.
/// </summary>
/// <param name="xy">The cursor position, packed as it arrives with the message.</param>
/// <returns>Returns with the drop context describing what lies under the cursor.</returns>
item_data TreeView_get_item_data(DWORD xy)
{
	item_data data;

	POINT point;
	point.x = LOWORD(xy);
	point.y = HIWORD(xy);

	HWND hwnd1 = ChildWindowFromPoint((HWND)GetWindowLong(TreeView_LastHandle, GWL_HWNDPARENT), point);
	HWND hwnd2 = (HWND)GetWindowLong(hwnd1, GWL_HWNDPARENT);
	if ((LONG)hwnd1 == GetWindowLong(TreeView_LastHandle, GWL_HWNDPARENT)) {
		hwnd2 = hwnd1;
	}

	RECT rect;
	POINT screen;
	screen.x = point.x;
	screen.y = point.y;

	ClientToScreen(hwnd2, &screen);
	data.point1 = screen;

	GetWindowRect(hwnd1, &rect);
	screen.x -= rect.left;
	screen.y -= rect.top;

	data.hwnd1 = hwnd1;
	data.hwnd2 = hwnd2;
	data.hwnd3 = TreeView_LastHandle;
	data.item = TreeView_DraggedItem;
	data.point2 = point;
	data.point3 = screen;
	return(data);
}


/// <summary>
/// Scrolls a tree view that an item is being dragged off the end of.
/// This routine is called from the drag timer. Whenever the cursor has strayed above or
/// below the tree, the view is scrolled toward it and the timer is retuned so that the
/// further out the cursor has wandered, the faster the tree scrolls.
/// </summary>
/// <param name="timer_id">The timer identifier reported with the message.</param>
void TreeView_handle_item_drag(int timer_id)
{
	if (timer_id == 1 && s_dragging) {
		POINT cursor;
		GetCursorPos(&cursor);

		RECT rect;
		GetWindowRect(TreeView_LastHandle, &rect);

		LONG i1 = rect.top - cursor.y;
		LONG i2 = cursor.y - rect.bottom;

		i2 = std::max(rect.top - cursor.y, cursor.y - rect.bottom);

		int i3 = std::max(i2, 0L);

		if (i3 > 0) {
			int time = 500 - 40 * i3;
			time = std::max(time, 5);
			SetTimer((HWND)GetWindowLong(TreeView_LastHandle, GWL_HWNDPARENT), 1, time, NULL);
			HTREEITEM item;
			HTREEITEM visible;

			item = TreeView_GetFirstVisible(TreeView_LastHandle);

			if (i1 > 0) {
				visible = TreeView_GetPrevVisible(TreeView_LastHandle, item);
			} else {
				visible = TreeView_GetNextVisible(TreeView_LastHandle, item);
			}

			ImageList_DragShowNolock(FALSE);
			TreeView_SelectSetFirstVisible(TreeView_LastHandle, visible);
			ImageList_DragShowNolock(TRUE);
		}
	}
}


/// <summary>
/// Handles mouse movement during a tree view drag.
/// This routine asks the dialog that owns the tree, by way of a TVDRAG_OVER message,
/// whether the item may be dropped where the cursor now rests. The cursor shape is set
/// to suit the answer and the drag image is dragged along behind it.
/// </summary>
/// <param name="lParam">The cursor position, packed as it arrives with the message.</param>
/// <returns>bool; Is the cursor over somewhere the item may be dropped?</returns>
bool TreeView_On_Mouse_Move(LPARAM lParam)
{
	bool ok;
	if (s_dragging) {

		ImageList_DragShowNolock(FALSE);

		item_data data;
		data = TreeView_get_item_data(lParam);

		if (SendMessage(data.hwnd2, TVDRAG_OVER, (WPARAM)&data, lParam) != 0) {
			SetCursor(LoadCursor(NULL, IDC_ARROW));
			ok = true;
		} else {
			SetCursor(LoadCursor(NULL, IDC_NO));
			ok = false;
		}

		ImageList_DragShowNolock(TRUE);

		RECT rect;
		GetWindowRect(data.hwnd2, &rect);

		ImageList_DragMove(data.point1.x - rect.left, data.point1.y - rect.top);

		return(ok);
	}
	return(false);
}


/// <summary>
/// Sets the icon of a tree view item that is opening or closing.
/// This routine is called from the item expand notification so that a folder style item
/// can show the appropriate icon for its new state.
/// </summary>
/// <param name="nmtv">The expand notification naming the item and what is happening to
/// it.</param>
/// <param name="collapse_image">The image to show when the item closes.</param>
/// <param name="expand_image">The image to show when the item opens.</param>
/// <returns>bool; Is the item now open?</returns>
bool TreeView_set_item_image(HWND window, NMTREEVIEW *nmtv, int collapse_image, int expand_image)
{
	bool ret;
	TVITEMA item;

	ret = false;
	item.hItem = nmtv->itemNew.hItem;
	/// item, image, handle
	item.mask = TVIF_HANDLE|TVIF_IMAGE|TVIF_SELECTEDIMAGE;

	switch (nmtv->action) {

		case TVE_EXPAND:
			item.iImage = expand_image;
			item.iSelectedImage = expand_image;
			ret = true;
			break;

		case TVE_COLLAPSE:
			item.iImage = collapse_image;
			item.iSelectedImage = collapse_image;
			ret = false;
			break;
	}

	TreeView_SetItem(window, &item);
	return(ret);
}


/// <summary>
/// Prepares a tree view label for editing.
/// This routine is called when the label editor appears. It caps how much the player may
/// type and remembers the buffer that TreeView_set_item_text should write the finished
/// label back into.
/// </summary>
/// <param name="text">The buffer that will receive the edited label.</param>
/// <param name="limit">The most characters the player may type.</param>
void TreeView_set_edit_text(HWND window, char *text, WPARAM limit)
{
	HWND ctrl = TreeView_GetEditControl(window);
	if (ctrl != NULL) {
		SendMessage(ctrl, EM_SETLIMITTEXT, limit, 0);
		TreeView_Text = text;
		TreeView_LastHandle = window;
	}
}


/// <summary>
/// Commits an edited tree view label.
/// This routine is called when the label editor closes. The new text is trimmed of the
/// white space the player left around it, copied back into the buffer that
/// TreeView_set_edit_text was given, and put back into the item.
/// </summary>
/// <param name="info">The label edit notification carrying the new text.</param>
/// <returns>Returns with the length of the text accepted, or zero if the edit was
/// abandoned.</returns>
int TreeView_set_item_text(NMTVDISPINFO *info)
{
	char * text = info->item.pszText;
	if (text != NULL) {
		strtrim(text);
		strcpy(TreeView_Text, text);
		TVITEMA item;
		item.hItem = info->item.hItem;
		item.mask = TVIF_TEXT;
		item.pszText = TreeView_Text;
		TreeView_SetItem(TreeView_LastHandle, &item);
		return(strlen(TreeView_Text));
	}
	return(0);
}


/// <summary>
/// Handles the WM_HELP message for a dialog.
/// This routine pops up the context help for whichever control the player asked about,
/// drawing the text from SUN.HLP.
/// </summary>
/// <param name="help_info">The help information that arrived with the message.</param>
/// <returns>bool; Was the help popup displayed?</returns>
BOOL On_WM_HELP(LPARAM help_info)
{
	HELPINFO *info = (HELPINFO *)help_info;
	HWND window = (HWND)info->hItemHandle;
	return(WinHelp(window, "SUN.HLP", HELP_CONTEXTPOPUP, GetWindowContextHelpId(window)));
}


/// <summary>
/// Handles the WM_CONTEXTMENU message for a dialog control.
/// This routine puts up the "What's This?" help menu for the control that was right
/// clicked, drawing the text from SUN.HLP.
/// </summary>
/// <param name="window">Handle of the control that was clicked.</param>
/// <returns>bool; Was the help menu put up?</returns>
BOOL On_WM_CONTEXTMENU(WPARAM window)
{
	DWORD identifiers[4];

	identifiers[3] = identifiers[2] = identifiers[1] = identifiers[0] = 0;

	identifiers[0] = GetWindowLong((HWND)window, GWL_ID);
	identifiers[1] = GetWindowContextHelpId((HWND)window);
	return(WinHelp((HWND)window, "SUN.HLP", HELP_CONTEXTMENU, (ULONG)identifiers));
}


char const *last_view_ini_section_name;

/// <summary>
/// Reads one child view's state back from the INI database.
/// This is the child window callback that read_views_from_ini hands to Windows. A tree
/// view has its root items expanded or collapsed as recorded, a list view has its column
/// widths restored, and anything else is left alone.
/// </summary>
/// <returns>Always true, so the enumeration carries on to the next child.</returns>
BOOL CALLBACK read_view_from_ini(HWND window, INIClass const &ini)
{
	char buffer[64];
	char const *section;

	GetClassName(window, buffer, sizeof(buffer));

	if (strcmp(buffer, WC_TREEVIEW) == 0) {
		//if (window != NULL) {
			int i = 0;
			section = last_view_ini_section_name;
			HTREEITEM item = TreeView_GetRoot(window);
			while (item != NULL) {
				i++;
				wsprintf(buffer, "TV%d", i);

				if (ini.Get_Bool(section, buffer, false)) {
					TreeView_Expand(window, item, TVE_EXPAND);
				} else {
					TreeView_Expand(window, item, TVE_COLLAPSE);
				}

				item = TreeView_GetNextSibling(window, item);
			}
		//}
		//return(true);
	}

	GetClassName(window, buffer, sizeof(buffer));

	if (strcmp(buffer, WC_LISTVIEW) == 0) {
		section = last_view_ini_section_name;
		if (window != NULL) {
			for (int i = 0; i < 10; i++) {
				wsprintf(buffer, "LV%d", i);
				unsigned int width = ListView_GetColumnWidth(window, i);
				width = ini.Get_Int(section, buffer, width);
				if (width < 1000) {
					ListView_SetColumnWidth(window, i, width);
				}
			}
		}
	}

	return(true);
}


/// <summary>
/// Reads a window's position and view layout back from the INI database.
/// This routine restores the position that write_views_to_ini recorded, nudging the
/// window back onto the display if the screen has shrunk since, then visits every child
/// control so each tree view and list view can restore its own state.
/// </summary>
/// <param name="section">The INI section holding the window's state.</param>
void read_views_from_ini(char const *section, HWND window, INIClass const &ini)
{
	LONG x;
	LONG y;
	int screen_x;
	int screen_y;

	if (window != NULL)
	{
		RECT rect;
		GetWindowRect(window, &rect);

		x = ini.Get_Int(section, "X", rect.left);
		y = ini.Get_Int(section, "Y", rect.top);

		screen_x = GetSystemMetrics(SM_CXSCREEN);
		if (x + rect.right - rect.left > screen_x) {
			x = rect.left + screen_x - rect.right;
		}
		screen_y = GetSystemMetrics(SM_CYSCREEN);
		if (y + rect.bottom - rect.top > screen_y) {
			y = rect.top + screen_y - rect.bottom;
		}

		if (x < 0) {
			x = 0;
		}
		if (y < 0) {
			y = 0;
		}

		SetWindowPos(window, 0, x, y, 0, 0, SWP_NOSIZE|SWP_NOZORDER);
		last_view_ini_section_name = section;
		EnumChildWindows(window, (WNDENUMPROC)read_view_from_ini, (LPARAM)&ini);
	}
}


/// <summary>
/// Writes one child view's state to the INI database.
/// This is the child window callback that write_views_to_ini hands to Windows. A tree
/// view records which of its root items are expanded, a list view records its column
/// widths, and anything else is left alone.
/// </summary>
/// <returns>Always true, so the enumeration carries on to the next child.</returns>
BOOL CALLBACK write_view_to_ini(HWND window, INIClass &ini)
{
	char buf[32];
	char buffer[64];
	char const *section;

	GetClassName(window, buffer, sizeof(buffer));

	if (strcmp(buffer, WC_TREEVIEW) == 0) {
		//if (window != NULL) {
			int i = 0;

			section = last_view_ini_section_name;
			HTREEITEM item = TreeView_GetRoot(window);

			while (item != NULL) {
				i++;
				wsprintf(buf, "TV%d", i);

				TVITEM *tmp = (TVITEM *)buffer;
				tmp->mask = TVIF_HANDLE|TVIF_STATE;
				tmp->stateMask = TVIS_EXPANDED;
				tmp->hItem = item;

				if (TreeView_GetItem(window, tmp)) {

					bool expanded = false;
					if (tmp->state & TVIS_EXPANDED) {
						expanded = true;
					}
					ini.Put_Bool(section, buf, expanded);
				}
				item = TreeView_GetNextSibling(window, item);
			}
		//}
		//return(true);
	}

	GetClassName(window, buffer, sizeof(buffer));

	if (strcmp(buffer, WC_LISTVIEW) == 0) {
		section = last_view_ini_section_name;
		if (window != NULL) {
			for (int i = 0; i < 10; i++) {
				wsprintf(buf, "LV%d", i);
				unsigned int width = ListView_GetColumnWidth(window, i);
				if (width < 1000) {
					ini.Put_Int(section, buf, width);
				}
			}
		}
	}

	return(true);
}


/// <summary>
/// Writes a window's position and view layout to the INI database.
/// This routine records where the window sits, then visits every child control so that
/// each tree view and list view can store its own state. Read it back in with
/// read_views_from_ini.
/// </summary>
/// <param name="section">The INI section to store the window's state under.</param>
void write_views_to_ini(char const *section, HWND window, INIClass &ini)
{
	if (window != NULL) {
		RECT rcl;
		GetWindowRect(window, &rcl);
		ini.Put_Int(section, "X", rcl.left);
		ini.Put_Int(section, "Y", rcl.top);
		last_view_ini_section_name = section;
		EnumChildWindows(window, (WNDENUMPROC)write_view_to_ini, (LPARAM)&ini);
	}
}


/// <summary>
/// Creates a temporary string identifier.
/// This routine is used to build the numbered entry names that the view state is stored
/// under. A NULL prefix yields just the number.
/// </summary>
/// <param name="str">The text to put in front of the number, or NULL for none.</param>
/// <param name="num">The number to tack on the end.</param>
/// <returns>Returns with a pointer to the assembled identifier.</returns>
/// <remarks>The identifier is built in a static buffer, so use it before calling this
/// routine again.</remarks>
const char *Make_Identifier(char *str, int num)
{
	static char _buffer[32];

	if ( str )
	{
		wsprintf(_buffer, "%s%d", str, num);
	}
	else
	{
		wsprintf(_buffer, "%d", num);
	}
	return(_buffer);
}


/// <summary>
/// Handles the WM_MOVING message for a window.
/// This routine keeps a window from being dragged off the edge of the display. The
/// dialog handlers call it from their WM_MOVING case and let Windows carry out the move
/// with the corrected rectangle.
/// </summary>
/// <param name="lparam">The proposed window rectangle, corrected in place.</param>
/// <returns>bool; Was the rectangle pulled back onto the screen?</returns>
bool On_WM_MOVING(HWND window, WPARAM wparam, LPARAM lparam)
{
	RECT *rcl = (RECT *)lparam;

	bool res = false;

	if (rcl->left < 0) {
		rcl->right -= rcl->left;
		rcl->left = 0;
		res = true;
	}

	if (rcl->top < 0) {
		rcl->bottom -= rcl->top;
		rcl->top = 0;
		res = true;
	}

	if (rcl->right > GetSystemMetrics(SM_CXFULLSCREEN))
	{
		rcl->left += GetSystemMetrics(SM_CXFULLSCREEN) - rcl->right;
		rcl->right = GetSystemMetrics(SM_CXFULLSCREEN);
		res = true;
	}
	if (rcl->bottom > GetSystemMetrics(SM_CYFULLSCREEN))
	{
		rcl->top += GetSystemMetrics(SM_CYFULLSCREEN) - rcl->bottom;
		rcl->bottom = GetSystemMetrics(SM_CYFULLSCREEN);
		res = true;
	}

	return(res);
}


/// <summary>
/// Finds the combo box entry that carries a particular item data value.
/// This routine stands in for the Windows macro of the same purpose, walking the entries
/// from the starting index onward and stopping at the first one whose attached data
/// matches.
/// </summary>
/// <param name="indexStart">The entry to begin the search at. Pass -1 to search from the
/// top of the list.</param>
/// <param name="data">The item data value being looked for.</param>
/// <returns>Returns with the index of the matching entry, or -1 if no entry carries that
/// data.</returns>
int ComboBox_Find_Item_Data(HWND hwndCtl, WPARAM indexStart, LPARAM data)
{
	int count = SendMessage(hwndCtl, CB_GETCOUNT, 0, 0);

	for (int index = indexStart; index < count; index++) {
		LRESULT res = SendMessage(hwndCtl, CB_GETITEMDATA, index, 0);
		if (res == data) {
			return(index);
		}
	}

	return(-1);
}


/// <summary>
/// Takes the mouse for the game, or hands it back to Windows.
/// This routine is used when a window has to be given the mouse for a while. Handing the
/// mouse back also forces a full repaint of the window and everything inside it, since
/// the game cursor will have scribbled over the frame while it had control.
/// </summary>
/// <param name="window">The window to repaint when the mouse is handed back.</param>
/// <param name="release">Zero to take the mouse, otherwise hand it back.</param>
void Capture_Or_Release_Mouse(HWND window, short release)
{
	if (MouseCursor != NULL) {
		if (release == 0) {
			MouseCursor->Capture_Mouse();
		} else {
			MouseCursor->Release_Mouse();
			UINT flags = RDW_INVALIDATE|RDW_INTERNALPAINT|RDW_ERASE|RDW_ALLCHILDREN|RDW_UPDATENOW|RDW_FRAME;
			RedrawWindow(window, NULL, NULL, flags);
		}
	}
}


void noop(void)
{

}


/// <summary>
/// Centers a window over its owner.
/// This is the convenient form for dialog handlers that have no parent handle at hand.
/// A window with no owner is left where it is.
/// </summary>
/// <param name="window">The window to be moved.</param>
void Center_Window_Within_Window(HWND window)
{
	HWND parent = (HWND)GetWindowLong(window, GWL_HWNDPARENT);
	if (parent != NULL) {
		Center_Window_Within_Window(window, parent);
	}
}


/// <summary>
/// Centers a window over another window.
/// This routine is used by the dialog handlers to place a dialog over the window that
/// spawned it. The main game window is measured by the current video mode rather than by
/// its client area, so a dialog lands centered on what the player can actually see.
/// </summary>
/// <param name="window">The window to be moved.</param>
/// <param name="parent">The window to center over.</param>
void Center_Window_Within_Window(HWND window, HWND parent)
{
	RECT rcl;
	GetClientRect(parent, &rcl);

	if (parent == MainWindow) {
		rcl.right = VideoModeWidth;
		rcl.bottom = VideoModeHeight;
	}

	ClientToScreen(parent, (LPPOINT)&rcl);
	ClientToScreen(parent, (LPPOINT)&rcl.right);
	rcl.right -= rcl.left;
	rcl.bottom -= rcl.top;

	RECT rect;
	GetClientRect(window, &rect);
	ClientToScreen(window, (LPPOINT)&rect);
	ClientToScreen(window, (LPPOINT)&rect.right);
	rect.right -= rect.left;
	rect.bottom -= rect.top;
	int x = (rcl.right - rect.right + 1) / 2;
	int y = (rcl.bottom - rect.bottom + 1) / 2;

	if (x < 0) {
		x = 0;
	}
	if (y < 0) {
		y = 0;
	}

	SetWindowPos(window, 0, x, y, -1, -1, SWP_NOSIZE|SWP_NOZORDER);
}


/// <summary>
/// Walks an audio buffer without disturbing it.
/// This routine is called by the sound stream service just before the play buffer is
/// locked and refilled. It reads through the pending sample data and throws the result
/// away, so the buffer is paged in and warm by the time the lock is taken.
/// </summary>
/// <param name="src">Pointer to the sample data to walk over.</param>
/// <param name="size">Number of bytes of sample data on hand.</param>
#define AUD_UNCOMP_CHUNK_SIZE 2048
#define AUD_FILL_SIZE ((unsigned)AUD_UNCOMP_CHUNK_SIZE * sizeof(unsigned short))
void Prefetch_Audio_Buffer(char *src, int size)
{

	int n1 = (unsigned short)src % AUD_FILL_SIZE;
	int n2 = size - n1;
	char *ptr = src + n1;

	if (n2 >= 1) {
		n2 -= n2 % AUD_FILL_SIZE;

		if (n2 >= 1) {
			int n3 = (16 * AUD_FILL_SIZE);
			int n4 = 0;

			while (n3 < n2) {
				int n5 = ptr[n3 - (16 * AUD_FILL_SIZE)];
				int n6 = ptr[n3] + n4;
				n4 = n5 + n6;
				n3 += AUD_FILL_SIZE;
			}
		}
	}
}
