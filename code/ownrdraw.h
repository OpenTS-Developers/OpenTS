/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include "arraylist.h"
#include "keyboard.h"
#include "surface.h"
#include "win.h"
#include "winfix.h"
#include "wstring.h"


namespace OwnerDraw {

	struct CellData {
		CellData(void);
		CellData(CellData const & that) : type(that.type), string(that.string), hint(that.hint), color(that.color), surf(that.surf), pingtime(that.pingtime)  {}
		CellData const & operator=(CellData & that)
		{
			type = that.type;
			string = that.string;
			hint = that.hint;
			color = that.color;
			surf = that.surf;
			pingtime = that.pingtime;
			return(*this);
		}

		enum DataType {
			INVALID,
			TEXT,
			SURFACE,
			PING,
			PRIMARY,
		};

		/*
		 * This specifies what kind of content the cell holds, and hence how it is drawn.
		 * A cell constructs as INVALID, and an INVALID cell contributes nothing to the row.
		 */
		DataType type;

		/*
		 * This is the text drawn in a TEXT cell. A PRIMARY cell ignores it and takes its
		 * text from the list box item itself instead.
		 */
		Wstring string;

		/*
		 * This is the tooltip text for the cell, shown when the mouse rests over this
		 * column of the row. If empty, then the cell has no tooltip.
		 */
		Wstring hint;

		/*
		 * This is the color the cell's text is drawn in. If -1, then the default list text
		 * color is used instead.
		 */
		int color;

		/*
		 * Pointer to the image drawn in a SURFACE cell, centered vertically in the row. The
		 * cell does not own the surface.
		 */
		Surface * surf;

		/*
		 * This is the round trip time in milliseconds, drawn as a bar in a PING cell. The
		 * bar fills in proportion to one second, and turns yellow at 300 and red at 500.
		 */
		int pingtime;
	};

	struct ColumnData {
		ColumnData(void) : xPos(0), width(0), cells() {}
		ColumnData(ColumnData & that) : xPos(that.xPos), width(that.width), cells(that.cells) {}

		/*
		 * This is the offset of the column from the left edge of a row, expressed in pixels.
		 */
		int xPos;

		/*
		 * This is the width available to the column, expressed in pixels. Text wider than
		 * this is truncated with an ellipsis. If zero, then the width is unlimited.
		 */
		int width;

		/*
		 * These are the column's cells, one for each row of the list box and indexed by row
		 * number. A row with no cell here draws nothing in this column.
		 */
		ArrayList<CellData> cells;
	};

	struct Tooltip {

		/*
		 * This is the area of the screen the tooltip covers.
		 */
		Rect bounds;

		/*
		 * Pointer to a saved copy of the screen underneath the tooltip. Hiding the tooltip
		 * blits this back, so the dialog beneath never has to repaint itself.
		 */
		Surface * background;

		/*
		 * This is the text displayed in the tooltip. If the owning control supplies none,
		 * then the placeholder "Tool Tip" is used.
		 */
		char text[128];

		/*
		 * If a tooltip currently exists, then this flag will be true. It stays true while
		 * the tooltip is hidden, and is only cleared when the tooltip is taken down.
		 */
		int isActive;

		/*
		 * If the tooltip is active but has been erased from the screen, then this flag will
		 * be true. A hidden tooltip can be put back without saving the screen again.
		 */
		int isHidden;

		/*
		 * This is the control the tooltip belongs to. The tooltip is taken down when that
		 * control is destroyed, hidden, or loses the keyboard focus.
		 */
		HWND window;

		Tooltip(void);
	};

	/*
	 * Key of the dictionary CtrlProc keeps of the messages it is already in the
	 * middle of handling, so that a control cannot re-enter its own handler.
	 */
	struct CtrlMsgData {
		/*
		 * These two together identify one message in flight -- the message code and the
		 * control it was sent to.
		 */
		UINT message;
		HWND window;

		bool operator ==(CtrlMsgData const & that) const;
	};

	/*
	 * Measurements of one of the remap fonts, cached by ODGetFontMetrics.
	 */
	struct FontMetrics {
		int charWidths[256];	/// inked width of each character, indexed by character code
		int glyphWidth;			/// width of the inked part of a glyph cell
		int glyphHeight;		/// height of the inked part of a glyph cell
		int topMargin;			/// blank rows above each row of glyphs
		int leftMargin;			/// blank columns before each glyph
	};

	/*
	 * Per-control data block stored by value in ODWinData, keyed by the control's HWND.
	 * A common header and footer are shared by the framework (CtrlProc) and every control;
	 * the middle is a union overlaid differently by each control type.
	 */
	struct WinData {

		/*
		 * These fields precede the per-control union and are common to every control type.
		 * They are maintained by the framework rather than by any one control's handler.
		 */
		LPARAM userData;		/// value handed in when the control was subclassed; nothing reads it back
		int scrollBarWidth;		/// width of the attached scrollbar, or zero if there is none
		HWND ownerWindow;		/// in a scrollbar's own record, the control that created it
		HWND attachedWindow;	/// attached scrollbar or dropdown; 1 while one is being created
		Surface *cachedSurface;	/// cached copy of the control's backdrop, rebuilt when it resizes
		Surface *image;			/// image the control draws (OD_SETIMAGE)
		Surface *altImage;		/// image drawn instead while the control is pressed (OD_SETALTIMAGE)
		int itemHeightSet;		/// combo box: the item height has already been set up once
		int paintDisabled;		/// suppress painting, for this control and its attached window (OD_DISABLEPAINT)
		int toolTipsEnabled;	/// show a tooltip while the mouse rests over the control (OD_TOOLTIPS)

		/*
		 * Per-control state. The same bytes mean different things per control type.
		 */
		union {
			struct {							/// list box -- columns, colors and selection
				ArrayList<int> *rowColors;		/// per-row background color, -1 for the default (OD_SETCOLOR)
				ArrayList<int> *selStates;		/// per-row selected flag, indexed alongside rowColors
				int topIndex;					/// the row drawn at the top of the visible area
				int curSel;						/// the currently selected row, or -1 if there is none
				ArrayList<ColumnData> *columns;	/// the columns the rows are divided into (OD_ADDCOLUMN)
			} ListBox;

			struct {							/// scroll bar -- range, thumb position and arrows
				int result;						/// the left button is being held down on the scrollbar
				int dragging;					/// the grip is being dragged with the mouse
				int range;						/// number of scroll positions, defaulting to 100
				int position;					/// the current scroll position within the range
				int upPressed;					/// the up arrow is held, and is drawn depressed
				int downPressed;				/// the down arrow is held, and is drawn depressed
				int keepCapture;				/// hand the mouse back to the owner on release (OD_SETKEEPCAPTURE)
			} ScrollBar;

			struct {							/// track bar (slider) -- range, value and thumb
				int result;						/// the left button is being held down on the track bar
				int dragging;					/// the thumb is being dragged with the mouse
				int range;						/// span of the value, the maximum less the minimum
				int value;						/// the current value, measured up from the minimum
				int minimum;					/// the low end of the value range
				int thumbPos;					/// how far along the slider the thumb is drawn, in pixels
				int step;						/// the increment a click applies; reported values snap to it
				int showNumbers;				/// draw the value as a number beside the slider (OD_TRACKNUMBERS)
				int clickSuppress;				/// stay silent rather than click when the value changes (OD_TRACKSILENT)
			} TrackBar;

			struct {							/// combo box -- the closed control; see ComboDrop
				int selIndex;					/// Unused
				int reserved2C;					/// Unused
				int reserved30;					/// Unused
				HWND dropdown;					/// the open dropdown window, or NULL while the list is closed
				int reserved38[6];				/// Unused
				COLORREF itemColors[50];		/// per-item text color, -1 for the default (OD_SETCOLOR)
			} ComboBox;

			struct {							/// static text -- the owner-drawn caption
				char *text;						/// the control's own copy of its caption, freed when it is destroyed
				COLORREF textColor;				/// the color the caption is drawn in (OD_SETCOLOR)
			} Static;

			struct {							/// owner-draw button
				int state;						/// item state from WM_DRAWITEM; ODS_SELECTED draws altImage
			} Button;

			struct {							/// auto-checkbox -- toggles itself when clicked
				int checkState;					/// whether the box is checked (BM_GETCHECK / BM_SETCHECK)
			} CheckBox;

			struct {							/// edit box -- focus / tab-stop state during dialog reveal
				int reserved28;					/// Unused
				int reserved2C;					/// Unused
				int focusPending;				/// focus arrived before the reveal finished, so OD_ACTIVATE must apply it
				int focusEnabled;				/// clear until OD_ACTIVATE; while clear, focus is deflected to MainWindow
				int hadTabStop;					/// WS_TABSTOP was stripped when subclassing, and OD_ACTIVATE restores it
			} Edit;

			struct {							/// combo box dropdown window
				int selection;					/// the highlighted item, seeded from the owning combo box
				int reserved2C;					/// Unused
				int scrollTop;					/// the first item visible in the dropped-down list
			} ComboDrop;

			struct {							/// the item state any owner-draw control was last asked to draw
				int itemState;					/// item state from WM_DRAWITEM; the same field as Button::state
			} DrawItem;

			struct {							/// progress bar -- the filled proportion of the bar
				int minimum;					/// the value at which the bar reads as empty
				int maximum;					/// the value at which the bar reads as full, 100 by default
				int position;					/// the current value, clamped to the range
			} ProgressBar;

			unsigned char _size[0x100];			/// pads the union out so that the footer lands at the right offset
		};

		/*
		 * These fields follow the per-control union and are common to every control type.
		 * They hold the device context state CtrlProc saves and restores around a paint.
		 */
		HFONT font;				/// the control's font, or the object OD_SAVEDC displaced out of the device context
		int bkMode;				/// the background mode saved by OD_SAVEDC
		COLORREF bkColor;		/// the background color saved by OD_SAVEDC
		COLORREF textColor;		/// the text color saved by OD_SAVEDC
		int field_138;			/// Unused
		int animState;			/// reveal state of the owning dialog -- 0 hidden, 1 awaiting the animated reveal, 2 shown
	};

	void Initialize(void);
	void Prepare_Resources(HWND window);
	void Register_Control_Classes(void);
	bool Subclass_Dialog(HWND window, LPARAM lParam);

	void Draw_Item(LPDRAWITEMSTRUCT drawit);
	void Draw_Dialog_Back(HWND window);

	int Capture_Mouse(void);
	int Release_Mouse(void);

	HWND Begin_Dialog(int id, DLGPROC proc);
	void Display_Dialog(HWND window);
	int Move_Dialog(HWND window, int x, int y);
	void End_Dialog(HWND window);

	int Default_Dialog_Proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam);
	bool Dialog_Message_Handler(void);

	bool Start_Tooltip(Rect const & rect, char const * text, HWND window);
	bool Show_Tooltip(bool save_background);
	bool Hide_Tooltip(void);
	bool End_Tooltip(void);

	HWND Custom_Message_Box(const char * btn1txt, const char * btn2txt = NULL, bool * cancelled = NULL);
	void Set_Custom_Message_Box_Text(HWND window, LPCSTR text);
};


#define OD_TEXT_ALIGN_MIN 1
#define OD_TEXT_ALIGN_CENTER 2
#define OD_TEXT_ALIGN_MAX 3

int OD_Draw_Text_Remap(Surface & surface, const char * string, Rect const & rect, char const * name, COLORREF color, int flags, int char_spacing);
int OD_Draw_Text(COLORREF color, HFONT font, Rect const & rect, const char * text, int len, int x_alignment, int y_alignment, Surface * surface);
void OD_Draw_Rect(Surface & surf, Rect const & rect, int offset, int color);

void On_WM_NCDESTROY(HWND window);

int Build_Hotkey_String(KeyNumType key, char * buffer);

extern COLORREF ODColorText;
extern COLORREF ODColorTextDim;
extern COLORREF ODColorDisabled;
extern COLORREF ODColorFrame;
extern COLORREF ODListBoxColor;
extern COLORREF ODColorUnused1;


/// Flags for ODDrawCharRemap.
#define OD_DRAW_CHAR_FLAG_HORIZONTAL_CENTER 1
#define OD_DRAW_CHAR_ALIGN_FLAG_RIGHT 2
#define OD_DRAW_CHAR_FLAG_VERTICAL_CENTER 4


extern unsigned short ODRComponentMask;
extern unsigned short ODGComponentMask;
extern unsigned short ODBComponentMask;

inline unsigned short OD_Blend_Color(unsigned short pixel, unsigned short color, unsigned char alpha)
{
	static unsigned blend_color_alpha;
	static unsigned blend_pixel_alpha;

	blend_color_alpha = alpha;
	blend_pixel_alpha = 255 - alpha;

	unsigned short r = ((((pixel & ODRComponentMask) * blend_pixel_alpha) + ((color & ODRComponentMask) * blend_color_alpha)) >> 8) & ODRComponentMask;
	unsigned short g = ((((pixel & ODGComponentMask) * blend_pixel_alpha) + ((color & ODGComponentMask) * blend_color_alpha)) >> 8) & ODGComponentMask;
	unsigned short b = (((pixel & ODBComponentMask) * blend_pixel_alpha) + ((color & ODBComponentMask) * blend_color_alpha)) >> 8;
	return((unsigned short)(r | g | b));
}


/*
 * Owner-draw control messages, handled by CtrlProc and the per-control
 * *CtrlProc functions in ownrdraw.cpp. Each is WM_USER + offset. The comment
 * above each says what it does, which control(s) handle it, and its
 * wParam / lParam / return contract.
 */

/*
 * Initialize a freshly subclassed control's WinData. Per-control: listbox sets item height/font,
 * combobox sets item height and clears item colors, static captures its caption, progress bar sets
 * max=100, edit strips WS_TABSTOP and deflects focus.
 * Used by: all controls.  Input: none.  Output: none.
 */
#define OD_SUBCLASSED				(WM_USER + 151)

/*
 * Set a per-element color. listbox = per-row background (wParam = row); combobox = per-item text
 * color (wParam = item 0..50); static = text color (wParam ignored, lParam == -1 resets to default).
 * Used by: listbox, combobox, static.  Input: wParam = element index, lParam = COLORREF.  Output: none.
 */
#define OD_SETCOLOR					(WM_USER + 152)

/*
 * Enable or disable hover tooltips for the control.
 * Used by: all controls.  Input: lParam = BOOL.  Output: previous enabled state.
 */
#define OD_TOOLTIPS					(WM_USER + 154)

/*
 * Ask the parent dialog for a control's tooltip text; the dialog copies it into the lParam buffer.
 * Used by: sent by the framework to the parent dialog.  Input: wParam = control ID, lParam = char[].  Output: none.
 */
#define OD_GETTIPTEXT				(WM_USER + 155)

/*
 * Set the control's primary user image (WinData::Image).
 * Used by: all controls.  Input: lParam = Surface*.  Output: previous Image.
 */
#define OD_SETIMAGE					(WM_USER + 156)

/*
 * Suppress painting for the control and its attached child window.
 * Used by: all controls.  Input: lParam = BOOL.  Output: previous state.
 */
#define OD_DISABLEPAINT				(WM_USER + 157)

/*
 * Restore the font / bk-mode / bk-color / text-color into the HDC that were saved by OD_SAVEDC.
 * Used by: all controls.  Input: lParam = HDC.  Output: 1.
 */
#define OD_RESTOREDC				(WM_USER + 158)

/*
 * Save the HDC's current font / bk-mode / bk-color / text-color into WinData for OD_RESTOREDC.
 * Used by: all controls.  Input: lParam = HDC.  Output: 1.
 */
#define OD_SAVEDC					(WM_USER + 159)

/*
 * Query whether the control has an attached child window (scrollbar / dropdown).
 * Used by: all controls.  Input: none.  Output: BOOL.
 */
#define OD_HASATTACHED				(WM_USER + 160)

/*
 * Non-painting refresh tick: sent in place of WM_PAINT while painting is disabled, so the control can
 * update grip/scroll state without drawing.
 * Used by: scrollbar, listbox (sent by CtrlProc).  Input: forwarded wParam/lParam.  Output: none.
 */
#define OD_REFRESHNOPAINT			(WM_USER + 165)

/*
 * Add a column to the multi-column listbox.
 * Used by: listbox.  Input: wParam = column width, lParam = column x-position (also the id).  Output: the x-position (existing one if a column is already there).
 */
#define OD_ADDCOLUMN				(WM_USER + 166)

/*
 * Remove the column whose x-position matches lParam.
 * Used by: listbox.  Input: lParam = column x-position/id.  Output: the id, or -1 if not found.
 */
#define OD_REMOVECOLUMN				(WM_USER + 167)

/*
 * Set the contents of one listbox cell.
 * Used by: listbox.  Input: wParam = MAKEWPARAM(columnId, row), lParam = CellData*.  Output: column id, or -1 on failure.
 */
#define OD_SETCELL					(WM_USER + 168)

/*
 * Push a window onto the modal z-order stack and pin it above its siblings.
 * Used by: all controls (CtrlProc).  Input: wParam = target HWND (0 = self), lParam = BOOL (1 = add and raise, 0 = remove).  Output: previous top window.
 */
#define OD_SETTOP					(WM_USER + 169)

/*
 * Set the control's alternate image (WinData::AltImage, e.g. the pressed/hover button surface).
 * Used by: all controls.  Input: lParam = Surface*.  Output: previous AltImage.
 */
#define OD_SETALTIMAGE				(WM_USER + 170)

/*
 * Set the trackbar step (the increment applied per click).
 * Used by: trackbar.  Input: lParam = INT step.  Output: none.
 */
#define OD_SETTRACKSTEP				(WM_USER + 171)

/*
 * Show or hide the numeric value drawn beside the trackbar.
 * Used by: trackbar.  Input: lParam = BOOL.  Output: none.
 */
#define OD_TRACKNUMBERS				(WM_USER + 172)

/*
 * Hit-test a listbox cell at a client point and copy its text into the buffer (for the per-cell tooltip).
 * Used by: listbox.  Input: wParam = MAKELPARAM(x, y), lParam = char[].  Output: non-zero when the cell text is empty.
 */
#define OD_GETCELLTIP				(WM_USER + 173)

/*
 * Suppress the click sound on trackbar value changes.
 * Used by: trackbar.  Input: wParam = BOOL (0 = silent).  Output: none.
 */
#define OD_TRACKSILENT				(WM_USER + 174)

/*
 * Sent to a dialog's children once the animated reveal finishes; edit controls re-enable focus/tab-stop
 * and apply any focus that arrived during the animation.
 * Used by: edit controls.  Input: none.  Output: none.
 */
#define OD_ACTIVATE					(WM_USER + 175)

/*
 * No dedicated handler: re-enters the control's WndProc so the edit-box focus-deflection at function
 * entry runs again (used after focus changes).
 * Used by: edit controls.  Input: none.  Output: none.
 */
#define OD_REFOCUS					(WM_USER + 176)

/*
 * Set the scrollbar's "keep parent capture" flag.
 * Used by: scrollbar.  Input: lParam = BOOL.  Output: none.
 */
#define OD_SETKEEPCAPTURE			(WM_USER + 177)

/*
 * Initialize a combo-box dropdown window; seeds its highlighted selection from the owner combo.
 * Used by: combo-box dropdown (ComboDropWinCtrlProc).  Input: none.  Output: none.
 */
#define OD_DROPSUBCLASSED			(WM_USER + 1000)
