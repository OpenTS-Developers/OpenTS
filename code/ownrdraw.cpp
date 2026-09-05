/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "always.h"

#include "ownrdraw.h"

#include "_keyboar.h"
#include "_mixfile.h"
#include "_rules.h"
#include "_surface.h"
#include "_xmouse.h"
#include "arraylist.h"
#include "bsurface.h"
#include "conquer.h"
#include "data.h"
#include "dbgprint.h"
#include "dict.h"
#include "dsaudio.h"
#include "dsurface.h"
#include "globals.h"
#include "goptions.h"
#include "hsv.h"
#include "keyboard.h"
#include "language/language.h"
#include "mainloop.h"
#include "misc.h"
#include "msgroute.h"
#include "vidscale.h"
#include "video.h"
#include "mixfile.h"
#include "msgloop.h"
#include "rgb.h"
#include "rules.h"
#include "session.h"
#include "srfcache.h"
#include "theme.h"
#include "voc.h"
#include "vox.h"
#include "windlg.h"

#include <commctrl.h>
#include <ctime>
#include <sys\timeb.h>
#include <utility>



using namespace OwnerDraw;

extern unsigned int Wstring_Hash(Wstring & string);


int _mouse_counter;
int _surface_count;

/*
 * globals
 */
int ODBorderThickness;
int ODColorSteps;
int ODScrollBarAdj;
COLORREF ODColorText;
COLORREF ODColorTextDim;
COLORREF ODColorDisabled;
COLORREF ODColorFrame;
COLORREF ODListBoxColor;
static COLORREF ODTooltipBoxColor;
COLORREF ODColorUnused1;

/*
 * fonts
 */
HFONT ODFontPtr;
HFONT ODListFontPtr;
char const * ODFontName = "MS Sans Serif";
char const * ODListFontName = "MS Sans Serif";
int ODFontSize = 14;
int ODListFontSize = 12;


#define RECT_WIDTH(rc)  ((rc).right  - (rc).left)
#define RECT_HEIGHT(rc) ((rc).bottom - (rc).top)


void ODDrawDimmedBackground(Rect const & rect, HWND hWndc);
void ODDrawGradientRect(Rect const & rect, Surface & surf, int color, int scale);
void ODDrawBevelDarken(Rect const & rect, Surface & surf, int xpos, int ypos);

void ODInitMasks(void);
void ODCacheImages(void);
int ODColorToHiColor(COLORREF color);


/*
 * private forward declarations
 */
LRESULT CALLBACK ComboDropWinCtrlProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK CtrlProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam);
LRESULT CALLBACK DefaultCtrlProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam);
LRESULT CALLBACK ButtonCtrlProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam);
LRESULT CALLBACK TextBoxCtrlProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam);
LRESULT CALLBACK EditBoxCtrlProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam);
LRESULT CALLBACK StaticCtrlProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam);
LRESULT CALLBACK CheckBoxCtrlProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam);
LRESULT CALLBACK ComboBoxCtrlProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam);
LRESULT CALLBACK ListBoxCtrlProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam);
LRESULT CALLBACK ScrollBarCtrlProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam);
LRESULT CALLBACK ProgressBarCtrlProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam);
LRESULT CALLBACK TrackBarCtrlProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam);
LRESULT CALLBACK GroupBoxCtrlProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam);
LRESULT CALLBACK HotkeyCtrlProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam);
LRESULT CALLBACK Custom_Message_Box_Proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam);

BOOL CALLBACK ODRemoveFromDict(HWND window, LPARAM);
int WINAPI ODUpdateWindowRect(HWND window, RECT *rect);
bool ODGetFontMetrics(char const *font_name, FontMetrics *metrics);
int ODDrawTextBG(Surface & surface, LPCSTR string, LPRECT rect, HGDIOBJ font, COLORREF color, UINT format);
void ODFillRectTrans(Rect const & rect, Surface & surf, int color, int trans);
void ODDrawArrowBitmap(Surface & surface, Rect const & rect, BOOL upward, BOOL pressed);
void ODDrawEdgeGlows(Surface & surface, Rect const & rect, BOOL raised, int count, int left_alpha, int top_alpha, int right_alpha, int bottom_alpha);
BOOL CALLBACK ODAddWindowToList(HWND window, ArrayList<HWND> * list);

BOOL CALLBACK SetUserData2(HWND window, LPARAM lparam);
BOOL CALLBACK InitializeCtrl(HWND window, LPARAM lparam);
void ODDrawCharRemap(Surface & dst_surf, const char *text, int max_chars, Rect const & rect, char const *font_name, COLORREF color, char flags, int char_spacing);


///////////////////////////////////

/// <summary>
/// Constructs an empty list box cell.
/// </summary>
OwnerDraw::CellData::CellData(void)
{
	type = CellData::INVALID;
	color = -1;
	pingtime = -1;
	surf = NULL;
}


/// <summary>
/// Fetches the hash value for a window handle.
/// This is the hash routine handed to the dictionaries that key their entries by the
/// window handle of a subclassed control.
/// </summary>
/// <returns>Returns with the handle itself, taken as an unsigned value.</returns>
unsigned int Hash_HWND(HWND &key)
{
	return(*(unsigned int*)&key);
}


/// <summary>
/// Fetches the hash value for a control message key.
/// This is the hash routine handed to the dictionary CtrlProc keeps of the messages it is
/// already in the middle of handling.
/// </summary>
/// <returns>Returns with the hash value formed from the window and the message.</returns>
unsigned int Hash_CtrlMsg(CtrlMsgData &key)
{
	return((unsigned int)((int)key.message * (int)key.window));
}


/// <summary>
/// Determines if two control message keys refer to the same thing.
/// </summary>
/// <returns>bool; Do both keys name the same window and the same message?</returns>
bool OwnerDraw::CtrlMsgData::operator ==(CtrlMsgData const & that) const
{
	if (that.window == window && that.message == message){
		return(true);
	}
	return(false);
}


/// <summary>
/// Sets the owner-draw metrics and colors to their defaults.
/// This routine establishes the border thickness, the blend strength and the family of
/// colors that every owner-draw control paints with. Each control asks for it as it is
/// subclassed, so the values are always current.
/// </summary>
void OwnerDraw::Initialize(void)
{
	ODBorderThickness = 1;
	ODColorSteps = 40;
	ODScrollBarAdj = 127;
	ODColorText = RGB(112,255,0);
	ODColorTextDim = RGB(16,144,16);
	ODColorDisabled = RGB(144,144,144);
	ODColorFrame = RGB(78, 182, 220);
	ODListBoxColor = RGB(34,80,97);
	ODTooltipBoxColor = RGB(11, 27, 34);
	ODColorUnused1 = RGB(22, 55, 68);
}



SurfaceCacheClass SurfaceCache;

/*
 * OriginalWndProcs contains original Win32 procs.
 * CustomWndProcs contains per-control custom procs.
 * The main proc for all controls is CtrlProc, it calls the custom proc,
 * and the custom proc usually calls the Win32 proc.
 */
Dictionary<HWND, WNDPROC> OriginalWndProcs(Hash_HWND);
Dictionary<HWND, WNDPROC> CustomWndProcs(Hash_HWND);
Dictionary<HWND, OwnerDraw::WinData> ODWinData(Hash_HWND);


/// <summary>
/// Registers the window classes the owner-draw system needs.
/// The combo box drop-down is a class of its own rather than a stock control, so it has to
/// be registered with Windows before any dialog is subclassed. Later calls do nothing.
/// </summary>
void OwnerDraw::Register_Control_Classes(void)
{
	static int registered = 0;
	if (registered == 1) {
		return;
	} else {
		registered = 1;
		WNDCLASS wc;
		memset(&wc, 0, sizeof(wc));
		wc.style = CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc = ComboDropWinCtrlProc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = ProgramInstance;
		wc.hIcon = NULL;
		wc.hCursor = NULL;
		wc.hbrBackground = NULL;
		wc.lpszMenuName = "ComboDropWin";
		wc.lpszClassName = "ComboDropWin";
		RegisterClass(&wc);
	}
}

static HWND _dropdown_window = NULL;
static HWND _dropdown_owner = NULL;


/// <summary>
/// Handles the messages for a combo box drop-down window.
/// The dropped list is a window of its own rather than a stock Windows list, so that it
/// can be painted over a dimmed copy of the dialog background. This routine tracks the
/// item under the mouse, attaches or removes a scroll bar as the item count demands, and
/// folds the list back into the owning combo box once a selection is made.
/// </summary>
/// <returns>Returns with zero for the messages handled here; otherwise with the result of
/// the default window procedure.</returns>
static LRESULT CALLBACK ComboDropWinCtrlProc_Internal(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);


/// <summary>
/// Hands the drop-down its messages, in frame coordinates, and presents what it paints.
/// </summary>
LRESULT CALLBACK ComboDropWinCtrlProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	LPARAM translated_lparam;
	if (Route_Mouse_Message(hWnd, Msg, wParam, lParam, &translated_lparam)) {
		return(0);
	}

	LRESULT result = ComboDropWinCtrlProc_Internal(hWnd, Msg, wParam, translated_lparam);

	if (Msg == WM_PAINT) {
		Video_Present_If_Dirty();
	}

	return(result);
}


static LRESULT CALLBACK ComboDropWinCtrlProc_Internal(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	static HBRUSH SolidBrush = CreateSolidBrush(RGB(48,96,48));
	static HWND OwnerComboHandle;
	(void)SolidBrush;

	RECT rect;
	Get_Display_Rect(hWnd, &rect);

	RECT client;
	GetClientRect(hWnd, &client);

	RECT parent_rect;
	memset(&parent_rect, 0, sizeof(parent_rect));

	HWND hWndParent = NULL;
	WinData * parent_data = NULL;

	int scrollbar_width = 2 * ODBorderThickness + 18;
	int need_scrollbar = -1;
	int max_top_index = 0;

	if (OwnerComboHandle) {
		hWndParent = GetParent(OwnerComboHandle);
	}

	if (hWndParent) {
		ODWinData.getPointer(hWndParent, &parent_data);
		Get_Display_Rect(hWndParent, &parent_rect);
	}

	_dropdown_owner = hWndParent;
	_dropdown_window = hWnd;

	WinData * data = NULL;
	WinData * master_data = NULL;

	ODWinData.getPointer(hWnd, &data);
	if (data == NULL) {
		DebugString("ComboBox dropdown windata = NULL\n");
	}

	if (OwnerComboHandle) {
		ODWinData.getPointer(OwnerComboHandle, &master_data);
	}

	if (Msg != CB_GETCOUNT && Msg != CB_GETITEMHEIGHT && Msg != WM_VSCROLL) {
		LRESULT item_count = SendMessage(OwnerComboHandle, CB_GETCOUNT, 0, 0);
		LRESULT item_height = SendMessage(OwnerComboHandle, CB_GETITEMHEIGHT, 0, 0);
		if (item_height <= 1) {
			item_height = 1;
		}

		need_scrollbar = (item_count * item_height > client.bottom - client.top);
		max_top_index = item_count - (client.bottom - client.top) / item_height;

		if (data) {
			if ((UINT_PTR)data->attachedWindow > 1) {
				SCROLLINFO info;
				info.cbSize = sizeof(SCROLLINFO);
				info.fMask = SIF_RANGE | SIF_POS;
				info.nMin = 0;
				info.nMax = max_top_index;
				info.nPos = data->ComboDrop.scrollTop;
				SendMessage(data->attachedWindow, SBM_SETSCROLLINFO, 0, (LPARAM)&info);
			}
			if (data->attachedWindow != NULL) {
				BringWindowToTop(data->attachedWindow);
			}
		}
	}

	switch (Msg) {
		case WM_ERASEBKGND:
			return(0);

		case WM_CREATE:
			SetCapture(hWnd);
			OwnerComboHandle = *(HWND *)lParam;
			return(0);

		case WM_PAINT: {
			LRESULT item_count = SendMessage(OwnerComboHandle, CB_GETCOUNT, 0, 0);
			LRESULT item_height = SendMessage(OwnerComboHandle, CB_GETITEMHEIGHT, 0, 0);
			int client_height = client.bottom - client.top;
			int selected_index = data->ComboDrop.selection;

			Rect dest_rect(rect.left, rect.top, rect.right - rect.left, client_height);
			int source_x = rect.left - parent_rect.left;
			int source_y = rect.top - parent_rect.top;

			if (data->cachedSurface == NULL) {
				Rect dst(0, 0, client.right, client.bottom);
				Rect src(source_x, source_y, client.right, client.bottom);
				BSurface * surface = new BSurface(client.right, client.bottom, 2);
				data->cachedSurface = surface;
				++_surface_count;

				if (parent_data != NULL && parent_data->cachedSurface != NULL) {
					surface->Blit_From(dst, *parent_data->cachedSurface, src, false, true);
				}

				int total = client.right * client.bottom;
				unsigned short * pixels = (unsigned short *)surface->Lock();
				for (int i = 0; i < total; ++i) {
					pixels[i] = OD_Blend_Color(pixels[i], 0, 180);
				}
				surface->Unlock();
			}

			{
				Rect src(0, 0, rect.right - rect.left, client_height);
				AlternateSurface->Blit_From(dest_rect, *data->cachedSurface, src, false, true);
			}

			{
				Rect border(rect.left + 1, rect.top + 1, rect.right - rect.left - 2, client_height - 2);
				OD_Draw_Rect(*AlternateSurface, border, 1, 0xFFFFFFFF);
			}

			FontMetrics font_data;
			int have_font = ODGetFontMetrics("dlgsys", &font_data);

			int index = data->ComboDrop.scrollTop;
			if (index < item_count) {
				int row_base = item_height * index;
				int row = row_base;

				while (1) {
					int left = rect.left + 1;
					int width = rect.right - rect.left - 2;
					int top = row - row_base + rect.top;

					if (item_height + row - row_base > client_height) {
						break;
					}

					char text[128];
					SendMessage(OwnerComboHandle, CB_GETLBTEXT, (WPARAM)index, (LPARAM)text);

					if (index == selected_index) {
						Rect fill(left, top + 1, width, item_height - 2);
						int fill_color = ODListBoxColor;
						if (fill_color != -1) {
							fill_color = ODColorToHiColor(fill_color);
						}
						AlternateSurface->Fill_Rect(fill, fill_color);
					}

					COLORREF text_color = ODColorText;
					if (index < 50 && master_data != NULL && master_data->ComboBox.itemColors[index] != -1) {
						text_color = master_data->ComboBox.itemColors[index];
					}

					if (have_font) {
						int text_width = 0;
						for (int i = 0; i < (int)strlen(text); ++i) {
							text_width += font_data.charWidths[(unsigned char)text[i]];
						}

						int max_width = width - 10;
						int ellipsis_width = 3 * font_data.charWidths['.'];
						int clipped = 0;

						if (text_width > max_width) {
							while (strlen(text) > 0) {
								int last = (int)strlen(text) - 1;
								text_width -= font_data.charWidths[(unsigned char)text[last]];
								text[last] = '\0';
								if (!clipped) {
									text_width += ellipsis_width;
								}
								clipped = 1;
								if (text_width <= max_width) {
									strcat(text, "...");
									break;
								}
							}
						}
					}

					RECT text_rect;
					text_rect.left = left + 3;
					text_rect.top = top;
					text_rect.right = left + width;
					text_rect.bottom = top + item_height;
					OD_Draw_Text_Remap(*AlternateSurface, text, *(Rect *)&text_rect, "dlgsys", text_color, 4, 0);

					++index;
					row += item_height;
					if (index >= item_count) {
						break;
					}
				}
			}

			AlternateSurface->Lock();
			VisibleSurface->Lock();

			RECT src_rect;
			src_rect.left = rect.left;
			src_rect.top = rect.top;
			src_rect.right = rect.right - rect.left;
			src_rect.bottom = client_height;

			RECT window_rect;
			GetWindowRect(hWnd, &window_rect);

			RECT dst_rect = src_rect;

			VisibleSurface->Blit_From(*(Rect *)&dst_rect, *AlternateSurface, *(Rect *)&src_rect, false, true);

			VisibleSurface->Unlock();
			AlternateSurface->Unlock();

			ValidateRect(hWnd, NULL);
			return(0);
		}

		case WM_NCDESTROY:
			_dropdown_window = NULL;
			_dropdown_owner = NULL;
			ReleaseCapture();
			break;

		case WM_VSCROLL: {
			LRESULT top_index = SendMessage(data->attachedWindow, SBM_GETPOS, 0, 0);
			if (top_index != SendMessage(hWnd, CB_GETTOPINDEX, 0, 0)) {
				SendMessage(hWnd, CB_SETTOPINDEX, top_index, 0);
			}
			break;
		}

		case CB_GETTOPINDEX:
			if (data) {
				return(data->ComboDrop.scrollTop);
			}
			break;

		case WM_MOUSEMOVE: {
			int x = (unsigned short)LOWORD(lParam);
			int y = (unsigned short)HIWORD(lParam);

			if (x >= 0 && y >= 0 && x < client.right && y < client.bottom) {
				LRESULT item_height = SendMessage(OwnerComboHandle, CB_GETITEMHEIGHT, 0, 0);
				LRESULT item_count = SendMessage(OwnerComboHandle, CB_GETCOUNT, 0, 0);
				int index = y / item_height;
				if (data) {
					index += data->ComboDrop.scrollTop;
				}

				int clamped = index < 0 ? 0 : index;
				int max_index = (int)item_count - 1;
				if (max_index < clamped) {
					clamped = max_index;
				}

				if (data->ComboDrop.selection != clamped) {
					InvalidateRect(hWnd, NULL, FALSE);
				}
				data->ComboDrop.selection = clamped;
				return(0);
			}
			return(0);
		}

		case CB_SETTOPINDEX: {
			int new_top = (int)wParam;
			LRESULT item_count = SendMessage(OwnerComboHandle, CB_GETCOUNT, 0, 0);
			LRESULT item_height = SendMessage(OwnerComboHandle, CB_GETITEMHEIGHT, 0, 0);
			if (!item_count || !item_height) {
				break;
			}

			int visible = (client.bottom - client.top) / item_height;
			if (new_top < 0) {
				new_top = 0;
			}

			int max_top = (int)item_count - visible;
			if (max_top > 0) {
				if (new_top > max_top) {
					new_top = max_top;
				}
			} else {
				new_top = 0;
			}

			if (new_top != data->ComboDrop.scrollTop) {
				data->ComboDrop.scrollTop = new_top;
				InvalidateRect(hWnd, NULL, FALSE);
			}
			return(0);
		}

		case WM_LBUTTONDOWN:
		case WM_LBUTTONDBLCLK: {
			int x = (unsigned short)LOWORD(lParam);
			int y = (unsigned short)HIWORD(lParam);

			if (x > client.right && x < client.right + data->scrollBarWidth && y > 0 && y < client.bottom) {
				SendMessage(data->attachedWindow, Msg, wParam, lParam);
				break;
			}

			Sound_Effect(Rule->GenericClick);

			if (x >= 0 && y >= 0 && x <= client.right && y <= client.bottom) {
				LRESULT item_height = SendMessage(OwnerComboHandle, CB_GETITEMHEIGHT, 0, 0);
				LRESULT item_count = SendMessage(OwnerComboHandle, CB_GETCOUNT, 0, 0);

				int index = y / item_height;
				if (data) {
					index += data->ComboDrop.scrollTop;
				}

				int clamped = index < 0 ? 0 : index;
				int max_index = (int)item_count - 1;
				if (max_index < clamped) {
					clamped = max_index;
				}

				SendMessage(OwnerComboHandle, CB_SETCURSEL, clamped, 0);

				if (need_scrollbar) {
					DestroyWindow(data->attachedWindow);
				}

				ReleaseCapture();
				SendMessage(OwnerComboHandle, CB_SHOWDROPDOWN, FALSE, 0);

				SendMessage(
					hWndParent,
					WM_COMMAND,
					MAKEWPARAM((UINT)GetWindowLong(OwnerComboHandle, GWL_ID), CBN_SELCHANGE),
					(LPARAM)OwnerComboHandle);
				return(0);
			}

			if (need_scrollbar) {
				DestroyWindow(data->attachedWindow);
			}
			ReleaseCapture();
			SendMessage(OwnerComboHandle, CB_SHOWDROPDOWN, FALSE, 0);
			return(0);
		}

		case OD_DROPSUBCLASSED: {
			WinData * drop_data = NULL;
			ODWinData.getPointer(hWnd, &drop_data);
			data->ComboDrop.selection = SendMessage(OwnerComboHandle, CB_GETCURSEL, 0, 0);
			return(0);
		}

		default:
			break;
	}

	if (need_scrollbar == 1) {
		if (data && !data->attachedWindow) {

			data->attachedWindow = (HWND)1;
			GetWindowLong(hWnd, GWL_ID);

			hWndParent = GetParent(hWnd);

			RECT parent_display_rect;
			Get_Display_Rect(hWndParent, &parent_display_rect);

			RECT drop_display_rect;
			Get_Display_Rect(hWnd, &drop_display_rect);

			int left = drop_display_rect.left - parent_display_rect.left;
			int top = drop_display_rect.top - parent_display_rect.top;

			HWND scroll_wnd = CreateWindowEx(
				0,
				"Scrollbar",
				NULL,
				0x50010001u,
				left + client.right - scrollbar_width,
				top + client.top,
				scrollbar_width,
				drop_display_rect.bottom - drop_display_rect.top,
				hWndParent,
				NULL,
				ProgramInstance,
				NULL);

			data->attachedWindow = scroll_wnd;
			data->scrollBarWidth = scrollbar_width;

			InitializeCtrl(scroll_wnd, 0);

			WinData *scroll_data = NULL;
			ODWinData.getPointer(scroll_wnd, &scroll_data);
			scroll_data->ownerWindow = hWnd;

			SCROLLINFO info;
			info.cbSize = sizeof(SCROLLINFO);
			info.fMask = SIF_RANGE | SIF_POS;
			info.nMin = 0;
			info.nMax = max_top_index;
			info.nPos = data->ComboDrop.scrollTop;
			SendMessage(scroll_wnd, SBM_SETSCROLLINFO, 0, (LPARAM)&info);

			SetWindowPos(
				hWnd,
				NULL,
				0,
				0,
				(drop_display_rect.right - drop_display_rect.left) - scrollbar_width,
				drop_display_rect.bottom - drop_display_rect.top,
				SWP_NOMOVE);

			ShowWindow(scroll_wnd, SW_SHOW);
			BringWindowToTop(scroll_wnd);
			InvalidateRect(scroll_wnd, NULL, FALSE);
			UpdateWindow(scroll_wnd);

			SendMessage(scroll_wnd, OD_SETTOP, (WPARAM)scroll_wnd, 1);
			SendMessage(scroll_wnd, OD_SETKEEPCAPTURE, 0, 1);

			RECT validate_rect;
			validate_rect.left = left;
			validate_rect.top = top;
			validate_rect.right = left + client.right + 1;
			validate_rect.bottom = top + client.bottom + 1;
			ValidateRect(hWndParent, &validate_rect);
		}

		return(DefWindowProc(hWnd, Msg, wParam, lParam));
	}

	if (need_scrollbar == 0 && data && data->attachedWindow && !data->paintDisabled) {
		HWND scroll_wnd = data->attachedWindow;
		DestroyWindow(scroll_wnd);

		OriginalWndProcs.remove(scroll_wnd);
		ODWinData.remove(scroll_wnd);
		CustomWndProcs.remove(scroll_wnd);

		data->attachedWindow = NULL;
		data->scrollBarWidth = 0;

		SetWindowPos(
			hWnd,
			NULL,
			0,
			0,
			ODBorderThickness + client.right - client.left + scrollbar_width + 1,
			client.bottom + 2 * ODBorderThickness - client.top,
			SWP_NOMOVE);

		hWndParent = GetParent(hWnd);
		Get_Display_Rect(hWndParent, &parent_rect);

		RECT validate_rect;
		validate_rect.left = rect.left - parent_rect.left - 1;
		validate_rect.top = rect.top - parent_rect.top - 1;
		validate_rect.right = rect.left - parent_rect.left + client.right + scrollbar_width;
		validate_rect.bottom = rect.top - parent_rect.top + client.bottom + 1;
		ValidateRect(hWndParent, &validate_rect);
	}

	return(DefWindowProc(hWnd, Msg, wParam, lParam));
}


/// <summary>
/// Subclasses a dialog and every one of its controls for owner drawing.
/// This is the routine a dialog procedure calls when it is first created. Painting is
/// suppressed across the whole window while the controls are being hooked, so the dialog
/// never flickers through its stock Windows appearance on the way.
/// </summary>
/// <param name="lparam">Caller supplied value to store with each control for its own use.</param>
bool OwnerDraw::Subclass_Dialog(HWND window, LPARAM lparam)
{
	OwnerDraw::Register_Control_Classes();

	EnumChildWindows(window, SetUserData2, 1);
	SetUserData2(window, 1);

	EnumChildWindows(window, InitializeCtrl, lparam);
	InitializeCtrl(window, lparam);

	EnumChildWindows(window, SetUserData2, 0);
	SetUserData2(window, 0);
	return(true);
}


/// <summary>
/// Sets the paint suppression flag for one window.
/// This is the enumeration callback Subclass_Dialog uses to silence, and later re-enable,
/// painting across a whole dialog. A window with no owner-draw record yet is given one.
/// </summary>
/// <param name="lparam">Should painting be suppressed for this window?</param>
BOOL CALLBACK SetUserData2(HWND window, LPARAM lparam)
{
	WinData * data = NULL;
	WinData temp;

	if (!ODWinData.getPointer(window, &data)) {
		memset(&temp, 0, sizeof(temp));
		ODWinData.add(window, temp);
		ODWinData.getPointer(window, &data);
	}

	data->paintDisabled = lparam;

	return(TRUE);
}


/// <summary>
/// Prepares one control for owner drawing.
/// This is the enumeration callback Subclass_Dialog uses. The control's window class and style
/// pick the custom procedure that will paint it, its original procedure is displaced by
/// CtrlProc and remembered, and the control is then told that it has been subclassed. The
/// shared fonts, color masks and cached artwork are built on the first control to arrive.
/// </summary>
/// <param name="lparam">Caller supplied value to store with the control for its own use.</param>
BOOL CALLBACK InitializeCtrl(HWND window, LPARAM lparam)
{
	char class_name[128];
	GetClassName(window, class_name, sizeof(class_name));

	LONG style = GetWindowLong(window, GWL_STYLE);

	RECT rect1;
	Get_Display_Rect(window, &rect1);
	RECT rect2;
	GetClientRect(window, &rect2);

	Initialize();

	static int _inited = false;
	if (!_inited) {
		ODInitMasks();
		ODCacheImages();
		HDC hdc = GetDC(window);
		ODFontPtr = WS_Get_Font(hdc, ODFontName, 0, ODFontSize, 0);
		ODListFontPtr = WS_Get_Font(hdc, ODListFontName, 0, ODListFontSize, 0);
		ReleaseDC(window, hdc);
		_inited = 1;
	}

	WNDPROC customProc = NULL;

	if (strcmp(class_name, WC_SCROLLBAR) == 0) {
		customProc = ScrollBarCtrlProc;
	} else if (strcmp(class_name, WC_LISTBOX) == 0) {
		customProc = ListBoxCtrlProc;
	} else if (strcmp(class_name, WC_COMBOBOX) == 0) {
		customProc = ComboBoxCtrlProc;
	} else if (strcmp(class_name, TRACKBAR_CLASS) == 0) {
		customProc = TrackBarCtrlProc;
	} else if (strcmp(class_name, PROGRESS_CLASS) == 0) {
		customProc = ProgressBarCtrlProc;
	} else if (strcmp(class_name, WC_EDIT) == 0) {
		customProc = EditBoxCtrlProc;
	} else if (strcmp(class_name, WC_STATIC) == 0) {
		customProc = StaticCtrlProc;
	} else if (strcmp(class_name, WC_TABCONTROL) == 0) {
		customProc = TextBoxCtrlProc;
	} else if (strcmp(class_name, WC_BUTTON) == 0) {
		if ((style & BS_GROUPBOX) == BS_GROUPBOX) {
			customProc = GroupBoxCtrlProc;
		} else if ((style & BS_OWNERDRAW) == BS_OWNERDRAW) {
			customProc = ButtonCtrlProc;
		} else if ((style & BS_AUTOCHECKBOX) == BS_AUTOCHECKBOX) {
			customProc = CheckBoxCtrlProc;
		}
	} else if (strcmp(class_name, HOTKEY_CLASS) == 0) {
		customProc = HotkeyCtrlProc;
	} else {
		customProc = DefaultCtrlProc;
	}

	WNDPROC originalProc = (WNDPROC)SetWindowLong(window, GWL_WNDPROC, (LONG)CtrlProc);

	if (!CustomWndProcs.contains(window)) {
		CustomWndProcs.add(window, customProc);
	}

	if (!OriginalWndProcs.contains(window)) {
		OriginalWndProcs.add(window, originalProc);
	}

	WinData * data = NULL;
	WinData temp;

	if (!ODWinData.getPointer(window, &data)) {
		memset(&temp, 0, sizeof(temp));
		ODWinData.add(window, temp);
		ODWinData.getPointer(window, &data);
	}

	data->userData = lparam;

	SendMessage(window, OD_SUBCLASSED, 0, 0);

	return(TRUE);
}


/// <summary>
/// Removes a dialog and all of its controls from the owner-draw dictionaries.
/// Use this routine as a subclassed dialog is torn down. Windows is free to hand the same
/// handles out again, and a stale entry would misdirect the next dialog to use them.
/// </summary>
BOOL ODCleanupDicts(HWND window)
{
	EnumChildWindows(window, ODRemoveFromDict, 0);
	ODRemoveFromDict(window, 0);

	return(TRUE);
}


/// <summary>
/// Removes one window from the owner-draw dictionaries.
/// This is the enumeration callback ODCleanupDicts uses. It drops the window's original
/// procedure, its custom procedure and its owner-draw record.
/// </summary>
BOOL CALLBACK ODRemoveFromDict(HWND window, LPARAM)
{
	OriginalWndProcs.remove(window);
	ODWinData.remove(window);
	CustomWndProcs.remove(window);

	return(TRUE);
}


Tooltip ODTooltip;
int ODLastTooltipTime;


/// <summary>
/// Constructs an empty tooltip.
/// </summary>
Tooltip::Tooltip(void)
{
	bounds.Set(0,0,0,0);
	background = NULL;
	text[0] = '\0';
	isActive = false;
	isHidden = false;
	window = (HWND)NULL;
}


/// <summary>
/// Starts displaying a tooltip over the given area.
/// Any tooltip already on screen is taken down first, since there is only ever one. The
/// area underneath is saved so the tooltip can be hidden again without the dialog having
/// to repaint itself.
/// </summary>
/// <param name="rect">The area of the screen the tooltip is to occupy.</param>
/// <param name="text">The text to display, or NULL for the placeholder text.</param>
/// <param name="window">The control the tooltip belongs to.</param>
/// <returns>bool; Was the tooltip displayed?</returns>
bool OwnerDraw::Start_Tooltip(Rect const & rect, char const * text, HWND window)
{
	OwnerDraw::End_Tooltip();

	ODLastTooltipTime = time(NULL);

	sprintf(ODTooltip.text, "Tool Tip");

	if (text != NULL) {
		strcpy(ODTooltip.text, text);
	}

	ODTooltip.bounds = rect;
	ODTooltip.window = window;
	ODTooltip.isActive = true;

	return(OwnerDraw::Show_Tooltip(true));
}


/// <summary>
/// Draws the tooltip onto the visible surface.
/// Use this routine to bring the tooltip back after a repaint has forced it into hiding.
/// The background is only captured when the caller asks for it, since redrawing the
/// tooltip must not save the tooltip as its own background.
/// </summary>
/// <param name="save_background">Should the area under the tooltip be captured first?</param>
/// <returns>bool; Was the tooltip drawn?</returns>
bool OwnerDraw::Show_Tooltip(bool save_background)
{
	if (ODTooltip.isActive == false) {
		return(false);
	}

	if (save_background) {
		if (ODTooltip.background != NULL) {
			delete ODTooltip.background;
		}
		ODTooltip.background = NULL;

		Surface * backgd = new BSurface(ODTooltip.bounds.Width, ODTooltip.bounds.Height, 2);
		ODTooltip.background = backgd;

		Rect drect(0, 0, ODTooltip.bounds.Width, ODTooltip.bounds.Height);
		backgd->Blit_From(drect, *VisibleSurface, ODTooltip.bounds);
	}

	ODTooltip.isHidden = false;

	Rect rect = ODTooltip.bounds;

	int color = ODColorToHiColor(ODTooltipBoxColor);

	VisibleSurface->Lock();
	VisibleSurface->Fill_Rect(rect, color);
	VisibleSurface->Unlock();

	ODDrawBevelDarken(rect, *VisibleSurface, 8, 4);

	OD_Draw_Text(ODColorText, ODFontPtr, rect, ODTooltip.text, strlen(ODTooltip.text), 1, 1, VisibleSurface);
	rect.X += 1;
	rect.Y += 1;
	rect.Width -= 2;
	rect.Height -= 2;
	OD_Draw_Rect(*VisibleSurface, rect, 1, -1);

	return(true);
}


/// <summary>
/// Hides the tooltip by restoring the area it covered.
/// The tooltip stays active while hidden, so it can be put back with Show_Tooltip once
/// whatever prompted the hide has finished painting.
/// </summary>
/// <returns>bool; Was the tooltip hidden?</returns>
bool OwnerDraw::Hide_Tooltip(void)
{
	if (ODTooltip.isActive == false) {
		return(false);
	}

	if (ODTooltip.isHidden == (int)true) {
		return(false);
	}

	if (ODTooltip.background == NULL) {
		return(false);
	}

	Rect drect = ODTooltip.bounds;
	Rect srect(0,0, ODTooltip.bounds.Width, ODTooltip.bounds.Height);
	VisibleSurface->Blit_From(drect, *ODTooltip.background, srect);
	ODTooltip.isHidden = true;
	return(true);
}


/// <summary>
/// Takes the tooltip down for good.
/// The area it covered is restored and the saved background released. Use this routine
/// when the mouse leaves the control, or when the control itself is going away.
/// </summary>
/// <returns>bool; Was there a tooltip to take down?</returns>
bool OwnerDraw::End_Tooltip(void)
{
	if (ODTooltip.isActive == false) {
		return(false);
	}

	OwnerDraw::Hide_Tooltip();

	if (ODTooltip.background != NULL) {
		delete ODTooltip.background;
	}
	ODTooltip.background = NULL;

	ODTooltip.isActive = false;
	ODTooltip.isHidden = false;
	return(true);
}


/// <summary>
/// Handles every message sent to a subclassed control.
/// This is the procedure that displaces the stock Windows procedure of each control, and
/// it is the heart of the owner-draw system. It enforces the modal window stack, guards
/// against a message re-entering the same control, drives the tooltip and hover timers,
/// accumulates the area that has to reach the screen, and then hands the message to the
/// control's own custom procedure -- which is usually the one that calls the original
/// Windows procedure.
/// </summary>
/// <returns>Returns with the result of the control's custom procedure, or zero when the
/// message was swallowed here.</returns>
static LRESULT CALLBACK CtrlProc_Internal(HWND window, UINT message, WPARAM wparam, LPARAM lparam);


/// <summary>
/// Hands a control its messages, in frame coordinates, and puts what it paints on screen.
/// The controls draw into the game's own surfaces rather than into their windows, so
/// every repaint has to be followed by a present to be seen.
/// </summary>
LRESULT CALLBACK CtrlProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	LPARAM translated_lparam;
	if (Route_Mouse_Message(window, message, wparam, lparam, &translated_lparam)) {
		return(0);
	}

	LRESULT result = CtrlProc_Internal(window, message, wparam, translated_lparam);

	if (message == WM_PAINT) {
		Video_Present_If_Dirty();
	}

	return(result);
}


static LRESULT CALLBACK CtrlProc_Internal(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	static POINT min_update_rect = {0xFFFFFF, 0xFFFFFF};
	static POINT max_update_rect;
	static int num_rect_updates;

	/*
	 * Tracks whether the game window had focus on the previous call so the
	 * controls can be refreshed when focus is regained.
	 */
	static bool was_in_focus = true;

	/*
	 * Set to 1 around the SetWindowPos() call issued by the OD_SETTOP handler,
	 * and checked by the WM_WINDOWPOSCHANGING handler so the engine does not
	 * fight its own z-order change.
	 */
	static char in_programmatic_reorder;

	if (message == WM_SETCURSOR) {
		return(1);
	}

	WNDPROC specific_proc = NULL;
	CustomWndProcs.getValue(window, specific_proc);

	WNDPROC original_proc = NULL;
	OriginalWndProcs.getValue(window, original_proc);

	if (message == WM_SYSKEYUP && wparam == VK_TAB) {
		SendMessage(MainWindow, WM_SYSKEYUP, VK_TAB, lparam);
	}

	LRESULT result = 0;
	BOOL show_tooltip = FALSE;
	int anim_state = 0;

	static Dictionary<CtrlMsgData, bool> ctrlmessages(Hash_CtrlMsg);

	CtrlMsgData key;
	key.window = window;
	key.message = message;

	RECT display_rect;
	Get_Display_Rect(window, &display_rect);
	RECT window_rect;
	GetWindowRect(window, &window_rect);
	int offset_x = 0;
	int offset_y = 0;

	bool mouse_over = false;

	static ArrayList<HWND> hwndarray;

	HWND owner;
	WinData *ownerdata;
	int state;

	/*
	 * When a window has been pushed to the top (modal) via OD_SETTOP, swallow
	 * mouse messages that are not directed at it or one of its children.
	 */
	if (hwndarray.length() != 0) {
		HWND topwindow;
		hwndarray.getTail(topwindow);

		BOOL allow = FALSE;
		BOOL forward = FALSE;
		if (GetParent(window) == NULL) {
			allow = TRUE;
		}
		if (GetWindowLong(window, GWL_ID) <= 0) {
			allow = TRUE;
		}

		HWND parent = window;
		while (parent != NULL) {
			if (parent == topwindow) {
				allow = TRUE;
				break;
			}
			parent = GetParent(parent);
		}

		if (message < WM_MOUSEMOVE || message > WM_MBUTTONDBLCLK) {
			forward = TRUE;
		}
		if (message >= WM_NCMOUSEMOVE && message <= WM_KEYLAST) {
			forward = FALSE;
		}
		if (message == WM_SYSKEYUP || message == WM_SYSKEYDOWN || message == WM_SYSCOMMAND || message == WM_SYSCHAR) {
			forward = TRUE;
		}
		if (message == OD_GETTIPTEXT || message == WM_TIMER || message == OD_GETCELLTIP) {
			forward = FALSE;
		}
		if (!allow && !forward) {
			return(0);
		}
	}

	/*
	 * Guard against re-entrant processing of the same message for the same
	 * window. A handful of messages are allowed to re-enter.
	 */
	if (ctrlmessages.contains(key)) {
		if (message != WM_COMMAND && message != WM_SYSKEYDOWN && message != WM_SYSKEYUP
			&& message != WM_SYSCOMMAND && message != WM_SYSCHAR) {
			return(0);
		}
	}

	bool processing = true;
	ctrlmessages.remove(key);
	ctrlmessages.add(key, processing);

	RECT client_rect;
	GetClientRect(window, &client_rect);
	RECT disp_rect;
	Get_Display_Rect(window, &disp_rect);

	bool in_focus = GameInFocus;

	bool is_paint = false;
	if (message == WM_PAINT) {
		is_paint = true;
		if (!in_focus) {
			ValidateRect(window, NULL);
			ctrlmessages.remove(key);
			return(0);
		}
	}

	if (in_focus == true && !was_in_focus) {
		InvalidateRect(MainWindow, NULL, FALSE);
		in_focus = GameInFocus;
	}
	was_in_focus = in_focus;

	RECT update_rect;
	if (is_paint) {
		++num_rect_updates;
		GetUpdateRect(window, &update_rect, FALSE);
		update_rect.left += disp_rect.left;
		update_rect.right += disp_rect.left;
		update_rect.top += disp_rect.top;
		update_rect.bottom += disp_rect.top;
	}

	WinData *data = NULL;
	ODWinData.getPointer(window, &data);

	if (message == OD_HASATTACHED) {
		result = (data->attachedWindow != NULL);
		goto cleanup;
	}

	/*
	 * Save the device context's current font / colors so they can be
	 * restored by OD_RESTOREDC.
	 */
	if (message == OD_SAVEDC) {
		HDC hdc = (HDC)lparam;
		HGDIOBJ old = SelectObject(hdc, GetStockObject(SYSTEM_FONT));
		data->font = (HFONT)old;
		SelectObject(hdc, old);
		data->bkMode = GetBkMode(hdc);
		data->bkColor = GetBkColor(hdc);
		data->textColor = GetTextColor(hdc);
		result = 1;
		goto cleanup;
	}

	/*
	 * Restore the device context state saved by OD_SAVEDC.
	 */
	if (message == OD_RESTOREDC) {
		HDC hdc = (HDC)lparam;
		SelectObject(hdc, data->font);
		SetBkMode(hdc, data->bkMode);
		SetBkColor(hdc, data->bkColor);
		SetTextColor(hdc, data->textColor);
		result = 1;
		goto cleanup;
	}

	/*
	 * Toggle paint suppression for this control (and its linked window).
	 */
	if (message == OD_DISABLEPAINT) {
		{
			HWND linked = (HWND)data->attachedWindow;
			result = data->paintDisabled;
			data->paintDisabled = lparam;
			if (linked != NULL) {
				WinData *linkeddata = NULL;
				ODWinData.getPointer((HWND &)data->attachedWindow, &linkeddata);
				if (linkeddata != NULL) {
					linkeddata->paintDisabled = lparam;
				}
			}
		}

	call_custom_proc:
		if (specific_proc != NULL) {
			result = CallWindowProc(specific_proc, window, message, wparam, lparam);
		}

	after_proc:
		if (message == WM_NCDESTROY) {
			HWND destroyed = window;
			WinData * destroydata;
			if (ODWinData.getPointer(destroyed, &destroydata)) {
				if (destroydata != NULL && destroydata->cachedSurface != NULL) {
					delete destroydata->cachedSurface;
					destroydata->cachedSurface = NULL;
					_surface_count--;
				}
			}
			ODRemoveFromDict(window, 0);
		}
		goto cleanup;
	}

	/*
	 * Push a window to the top of the modal stack.
	 */
	if (message == OD_SETTOP) {
		HWND topwindow = NULL;
		hwndarray.getTail(topwindow);
		result = (LRESULT)topwindow;

		HWND target = window;
		if (wparam) {
			target = (HWND)wparam;
		}

		HWND found = NULL;
		int scan = 0;
		for (int index = 0; index < hwndarray.length(); index++) {
			hwndarray.get(found, scan);
			if (found == target) {
				hwndarray.remove(scan);
			} else {
				scan++;
			}
		}

		if (lparam) {
			hwndarray.addTail(target);
			in_programmatic_reorder = 1;
			SetWindowPos(target, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
			in_programmatic_reorder = 0;
		}
		goto cleanup;
	}

	if (hwndarray.length() != 0) {

		/*
		 * Keep the modal window pinned on top when Windows tries to reorder it.
		 */
		if (message == WM_WINDOWPOSCHANGING) {
			if (in_programmatic_reorder != 1) {
				HWND found = NULL;
				for (int index = 0; index < hwndarray.length(); index++) {
					hwndarray.get(found, index);
					if (found == window) {
						WINDOWPOS *wp = (WINDOWPOS *)lparam;
						if (index == hwndarray.length() - 1 && wp->hwndInsertAfter != NULL && (wp->flags & SWP_NOZORDER) == 0) {
							wp->hwndInsertAfter = NULL;
							InvalidateRect(window, NULL, FALSE);
							result = 0;
						} else {
							wp->flags |= SWP_NOOWNERZORDER | SWP_NOZORDER;
							InvalidateRect(window, NULL, FALSE);
							result = 0;
						}
						goto cleanup;
					}
				}
			}
			goto call_custom_proc;
		}

		/*
		 * Drop a destroyed window from the modal stack.
		 */
		if (message == WM_DESTROY) {
			HWND found = NULL;
			int scan = 0;
			for (int index = 0; index < hwndarray.length(); index++) {
				hwndarray.get(found, scan);
				if (found == window) {
					hwndarray.remove(scan);
				} else {
					scan++;
				}
			}
		}
	}

	/*
	 * Tear down the tooltip when its owning window is destroyed, hidden, or
	 * loses focus.
	 */
	if (window == ODTooltip.window
		&& (message == WM_NCDESTROY || message == WM_SHOWWINDOW || message == WM_KILLFOCUS)
		&& ODTooltip.isActive) {
		OwnerDraw::End_Tooltip();
	}

	if (message == WM_ERASEBKGND) {
		result = 1;
		goto cleanup;
	} else if (message == WM_SETFOCUS) {
		if (specific_proc == ButtonCtrlProc || specific_proc == ListBoxCtrlProc) {
			SetFocus((HWND)wparam);
		}
	} else if (message == WM_SHOWWINDOW) {
		if (wparam == 0) {
			data->animState = 0;
		}
	} else if (message == OD_SETIMAGE) {
		result = (LRESULT)data->image;
		data->image = (Surface *)lparam;
		goto cleanup;
	} else if (message == OD_SETALTIMAGE) {
		result = (LRESULT)data->altImage;
		data->altImage = (Surface *)lparam;
		goto cleanup;
	} else if (message == OD_TOOLTIPS) {
		result = data->toolTipsEnabled;
		data->toolTipsEnabled = lparam;
		goto cleanup;
	}

	if (data->toolTipsEnabled) {

		if (message == WM_TIMER || (message >= WM_MOUSEMOVE && message <= WM_MOUSELAST)) {
			Rect corner;
			GetWindowRect(window, (LPRECT)&corner);
			if (WindowFromPoint(*(POINT *)&corner) == window) {
				mouse_over = true;
			}
		}

		if (data->toolTipsEnabled && mouse_over) {

			if (message == WM_MOUSEMOVE) {
				int mx = LOWORD(lparam);
				int my = HIWORD(lparam);

				if (specific_proc == ListBoxCtrlProc) {
					if (OwnerDraw::End_Tooltip()) {
						ReleaseCapture();
					}
					KillTimer(window, 0);
				}

				if (mx >= 0 && my >= 0 && mx < client_rect.right && my < client_rect.bottom) {
					if ((wparam & 0x13) == 0) {
						UINT delay = 1000;
						if (time(NULL) - ODLastTooltipTime <= 1) {
							delay = 300;
						}
						SetTimer(window, 0, delay, NULL);
					}
				} else {
					if (OwnerDraw::End_Tooltip()) {
						ReleaseCapture();
					}
					KillTimer(window, 0);
				}

			} else {
				if (message >= WM_MOUSEMOVE && message <= WM_MBUTTONDBLCLK) {
					if (OwnerDraw::End_Tooltip()) {
						ReleaseCapture();
					}
					KillTimer(window, 0);
				}

				if (message == WM_TIMER) {
					POINT pt;
					Get_Logical_Cursor_Pos(window, pt);
					int mx = pt.x;
					int my = pt.y;

					BOOL inside = FALSE;
					if (pt.x >= 0 && pt.y >= 0 && pt.x < client_rect.right && pt.y < client_rect.bottom) {
						inside = TRUE;
					}

					CHAR buf[128];
					memset(buf, 0, sizeof(buf));
					if (inside) {
						WPARAM ctrl_id = GetWindowLong(window, GWL_ID);
						HWND parent = GetParent(window);
						SendMessage(parent, OD_GETTIPTEXT, ctrl_id, (LPARAM)buf);
						if (strlen(buf) == 0) {
							SendMessage(window, OD_GETCELLTIP, MAKELONG(mx, my), (LPARAM)buf);
						}

						if (strcmp(buf, ODTooltip.text) != 0 && ODTooltip.isActive) {
							if (OwnerDraw::End_Tooltip()) {
								ReleaseCapture();
							}
						}

						if (!OwnerDraw::Show_Tooltip(false)) {
							if (strlen(buf)) {
								HDC hdc = GetDC(window);
								HFONT font = WS_Get_Font(hdc, ODFontName, 0, ODFontSize, 0);
								if (font != NULL) {
									SelectObject(hdc, font);
								}
								SetTextColor(hdc, ODColorText);
								SetBkMode(hdc, TRANSPARENT);
								SIZE size;
								GetTextExtentPoint32(hdc, buf, strlen(buf), &size);
								ReleaseDC(window, hdc);

								POINT cursor;
								Get_Logical_Cursor_Pos(NULL, cursor);
								Rect tooltip_rect;
								tooltip_rect.X = cursor.x;
								tooltip_rect.Y = cursor.y + 16;
								tooltip_rect.Width = size.cx + 8;
								tooltip_rect.Height = size.cy + 6;

								Rect mainclient = VisibleSurface->Get_Rect();
								if (tooltip_rect.Width + tooltip_rect.X >= mainclient.Width) {
									tooltip_rect.X = mainclient.Width - tooltip_rect.Width;
								}
								if (tooltip_rect.Height + tooltip_rect.Y >= mainclient.Height) {
									tooltip_rect.Y = mainclient.Height - tooltip_rect.Height;
								}

								SetCapture(window);
								OwnerDraw::Start_Tooltip(tooltip_rect, buf, window);
							}
						} else {
							SetCapture(window);
						}
					}
				}
			}
		}
	}

	if (is_paint) {

		/*
		 * Paint path: when painting is disabled, just tell the control to repaint
		 * its frame.
		 */
		if (data->paintDisabled) {
			ValidateRect(window, NULL);
			result = CallWindowProc(specific_proc, window, OD_REFRESHNOPAINT, wparam, lparam);
		} else {

			/*
			 * If this paint overlaps the tooltip, hide the tooltip first and re-show it
			 * once painting is finished.
			 */
			{
				RECT tooltip_rect;
				tooltip_rect.left = ODTooltip.bounds.X;
				tooltip_rect.right = ODTooltip.bounds.Width + ODTooltip.bounds.X + 1;
				tooltip_rect.top = ODTooltip.bounds.Y;
				tooltip_rect.bottom = ODTooltip.bounds.Height + ODTooltip.bounds.Y + 1;
				if (num_rect_updates == 1) {
					RECT intersect;
					if (IntersectRect(&intersect, &disp_rect, &tooltip_rect)) {
						if (ODTooltip.isActive && ODTooltip.isHidden != 1 && ODTooltip.background != NULL) {
							Rect drect = ODTooltip.bounds;
							Rect srect(0, 0, ODTooltip.bounds.Width, ODTooltip.bounds.Height);
							VisibleSurface->Blit_From(drect, *ODTooltip.background, srect);
							ODTooltip.isHidden = true;
							show_tooltip = true;
						}
					}
				}
			}

			if (min_update_rect.x >= disp_rect.left) {
				min_update_rect.x = disp_rect.left;
			}
			if (min_update_rect.y >= disp_rect.top) {
				min_update_rect.y = disp_rect.top;
			}
			if (max_update_rect.x <= disp_rect.right) {
				max_update_rect.x = disp_rect.right;
			}
			if (max_update_rect.y <= disp_rect.bottom) {
				max_update_rect.y = disp_rect.bottom;
			}

			/*
			 * Walk up to the owning dialog window (the first ancestor that draws its own
			 * background) and read its animation state.
			 */
			owner = window;
			while (owner != NULL) {
				if (GetWindowLong(owner, DWL_DLGPROC) != 0) {
					break;
				}
				owner = GetParent(owner);
			}

			ownerdata = NULL;
			if (owner != NULL) {
				ODWinData.getPointer(owner, &ownerdata);
			}

			if (owner == window) {
				if (ownerdata->animState < 1) {
					state = 1;
					anim_state = 1;
					goto call_proc_paint;
				}
				state = ownerdata->animState;
			} else {
				if (ownerdata == NULL) {
					state = anim_state;
					ValidateRect(window, NULL);
					result = 1;
					goto finalize_paint;
				}
				state = ownerdata->animState;
			}

			anim_state = state;
			if (state < 1) {
				ValidateRect(window, NULL);
				result = 1;
			} else {

			call_proc_paint:
				result = CallWindowProc(specific_proc, window, message, wparam, lparam);
				if (ownerdata != NULL) {
					ownerdata->animState = state;
				}

				{
					ArrayList<HWND> children;
					EnumChildWindows(window, (WNDENUMPROC)ODAddWindowToList, (LPARAM)&children);

					HWND combo_owner = NULL;
					HWND child = NULL;
					for (int index = 0; index < children.length(); index++) {
						children.get(child, index);

						WNDPROC childproc = NULL;
						OriginalWndProcs.getValue(child, childproc);

						if (childproc != (WNDPROC)ComboDropWinCtrlProc) {
							InvalidateRect(child, NULL, FALSE);
							UpdateWindow(child);
						} else {
							combo_owner = child;
						}
					}

					if (combo_owner != NULL) {
						InvalidateRect(combo_owner, NULL, FALSE);
						UpdateWindow(combo_owner);
					} else if (_dropdown_window != NULL) {
						if (_dropdown_owner == owner) {
							Rect drop_rect;
							Get_Display_Rect(_dropdown_window, (LPRECT)&drop_rect);
							RECT intersect;
							if (IntersectRect(&intersect, &disp_rect, (const RECT *)&drop_rect)) {
								InvalidateRect(_dropdown_window, NULL, FALSE);
								UpdateWindow(_dropdown_window);
							}
						}
					}
				}

				state = anim_state;
			}

		finalize_paint:
			if (state > 0) {
				HWND sibling = owner;
				while (window == owner || num_rect_updates == 1) {
					if (sibling == NULL) {
						break;
					}
					sibling = GetWindow(sibling, GW_HWNDPREV);
					if (sibling == NULL) {
						break;
					}
					if (GetWindowLong(sibling, DWL_DLGPROC)) {
						Rect sibling_rect;
						Get_Display_Rect(sibling, (LPRECT)&sibling_rect);
						RECT intersect;
						if (IntersectRect(&intersect, &disp_rect, (const RECT *)&sibling_rect)) {
							InvalidateRect(sibling, NULL, FALSE);
							UpdateWindow(sibling);
							break;
						}
					}
				}
			}
		}
		goto after_proc;
	} else {
		goto call_custom_proc;
	}

cleanup:
	ctrlmessages.remove(key);

	if (is_paint) {
		if (GetWindowLong(window, DWL_DLGPROC)) {
			if (num_rect_updates > 1) {
				data->animState = 2;
			}
		}

		if (--num_rect_updates == 0) {
			if (!data->paintDisabled && anim_state >= 1) {

				Rect rect2;
				Rect screen_rect;
				rect2.X = min_update_rect.x;
				rect2.Y = min_update_rect.y;
				rect2.Width = max_update_rect.x - min_update_rect.x;
				rect2.Height = max_update_rect.y - min_update_rect.y;
				screen_rect.X = min_update_rect.x;
				screen_rect.Y = min_update_rect.y;
				screen_rect.Width = max_update_rect.x - min_update_rect.x;
				screen_rect.Height = max_update_rect.y - min_update_rect.y;

				if (GetWindowLong(window, DWL_DLGPROC) && data->animState == 1) {

					/*
					 * Animated dialog reveal -- the screen wipes open from the
					 * center outward with sliding "leftbar"/"rightbar" edges.
					 */
					if (Options.SoundVolume > 0.0) {
						if (MixFileClass::Retrieve("EMBLEM.AUD")) {
							int volume = (int)(Options.SoundVolume * 64.0f);
							Audio.Play_Sample(MixFileClass::Retrieve("EMBLEM.AUD"), 255, volume);
						}
					}

					struct _timeb start_time;
					_ftime(&start_time);

					int half = rect2.Width / 2;
					int frame = 0;
					int center = rect2.Width / 2 + rect2.X;

					Surface *leftbar = SurfaceCache.GetSurface("leftbar.pcx", 0);
					Surface *rightbar = SurfaceCache.GetSurface("rightbar.pcx", 0);

					Rect barsrc(0, 0, leftbar->Get_Width(), leftbar->Get_Height());
					Rect bardst = barsrc;

					int bar_width = leftbar->Get_Width();
					int bar_height = leftbar->Get_Height();

					int step = 0;
					int counter = 0;
					int left_x = center - 12;

					while (step < half) {
						{
							int advance = bar_width;
							if (bar_width >= step) {
								advance = step;
							}

							/*
							 * Reveal the next slice on the left of the wipe. The slice
							 * is one stride (12 pixels) wider than the bar so that it
							 * completely covers the bar stamped by the previous frame.
							 */
							int reveal_x = left_x;
							rect2.X = left_x;
							int width_cache = advance + 12;
							rect2.Width = advance + 12;
							int reveal_w = advance + 12;
							if (left_x < min_update_rect.x) {
								reveal_x = min_update_rect.x;
								rect2.X = min_update_rect.x;
								reveal_w = reveal_w + left_x - min_update_rect.x;
								rect2.Width = reveal_w;
							}
							screen_rect.Width = reveal_w;
							screen_rect.Height = rect2.Height;
							screen_rect.X = offset_x + reveal_x;
							screen_rect.Y = offset_y + rect2.Y;
							VisibleSurface->Lock();
							AlternateSurface->Lock();
							VisibleSurface->Blit_From(screen_rect, *AlternateSurface, rect2);
							AlternateSurface->Unlock();
							VisibleSurface->Unlock();

							for (int y = 0; y < rect2.Height; y += bar_height) {
								int bar_x = rect2.X - bar_width;
								if (bar_x < rect2.X) {
									bar_x = rect2.X;
								}
								bardst.Y = y + rect2.Y;
								if (bar_height + y >= rect2.Height) {
									int clip = rect2.Height - bar_height - y;
									barsrc.Height += clip;
									bardst.Height += clip;
								}
								screen_rect = bardst;
								screen_rect.Y += offset_y;
								screen_rect.X = bar_x;
								screen_rect.X += offset_x;
								VisibleSurface->Blit_From(screen_rect, *leftbar, barsrc);
								barsrc.Height = bar_height;
								bardst.Height = bar_height;
							}

							/*
							 * Reveal the matching slice on the right of the wipe.
							 */
							int right_dst = center + step - advance;
							rect2.Width = width_cache;
							rect2.X = right_dst;
							if (width_cache + right_dst >= max_update_rect.x) {
								rect2.Width = max_update_rect.x - right_dst;
							}
							screen_rect.X = offset_x + rect2.X;
							screen_rect.Y = offset_y + rect2.Y;
							screen_rect.Width = rect2.Width;
							screen_rect.Height = rect2.Height;
							VisibleSurface->Lock();
							AlternateSurface->Lock();
							VisibleSurface->Blit_From(screen_rect, *AlternateSurface, rect2);
							VisibleSurface->Unlock();
							AlternateSurface->Unlock();

							for (int ry = 0; ry < rect2.Height; ry += bar_height) {
								int bar_x = rect2.X + rect2.Width + bar_width;
								if (bar_x > rect2.X + rect2.Width - bar_width) {
									bar_x = rect2.X + rect2.Width - bar_width;
								}
								bardst.Y = ry + rect2.Y;
								if (ry + bar_height >= rect2.Height) {
									int clip = rect2.Height - ry - bar_height;
									bardst.Height += clip;
									barsrc.Height += clip;
								}
								screen_rect = bardst;
								screen_rect.Y += offset_y;
								screen_rect.X = bar_x;
								screen_rect.X += offset_x;
								VisibleSurface->Blit_From(screen_rect, *rightbar, barsrc);
								barsrc.Height = bar_height;
								bardst.Height = bar_height;
							}

							struct _timeb now;
							_ftime(&now);
							frame++;
							int wait = start_time.millitm + frame * (40 - counter / half) + 1000 * (start_time.time - now.time) - now.millitm;
							if (wait > 0) {
								Sleep(wait);
							}

							if (Audio_Available() && GameInFocus == true) {
								Audio.Sound_Callback();
								Theme.AI();
								Speak_AI();
							}

							/*
							 * The whole animation runs inside one paint, so each step
							 * has to reach the screen from here.
							 */
							Video_Present_If_Dirty();

							Sleep(0);

							step += 12;
							counter += 240;
							left_x -= 12;
						};
					}

					data->animState = 2;

					Rect whole_rect;
					whole_rect.X = min_update_rect.x;
					whole_rect.Y = min_update_rect.y;
					whole_rect.Width = max_update_rect.x - min_update_rect.x;
					whole_rect.Height = max_update_rect.y - min_update_rect.y;
					screen_rect.X = offset_x + min_update_rect.x;
					screen_rect.Y = offset_y + min_update_rect.y;
					screen_rect.Width = max_update_rect.x - min_update_rect.x;
					screen_rect.Height = max_update_rect.y - min_update_rect.y;
					VisibleSurface->Lock();
					AlternateSurface->Lock();
					VisibleSurface->Blit_From(screen_rect, *AlternateSurface, whole_rect);
					AlternateSurface->Unlock();
					VisibleSurface->Unlock();

					ArrayList<HWND> children;
					EnumChildWindows(window, (WNDENUMPROC)ODAddWindowToList, (LPARAM)&children);
					HWND child = NULL;
					for (int index = 0; index < children.length(); index++) {
						children.get(child, index);
						SendMessage(child, OD_ACTIVATE, 0, 0);
					}

				} else {

					/*
					 * Unanimated update -- copy the dirty rectangle straight from
					 * the back buffer to the screen.
					 */
					data->animState = 2;
					screen_rect.Width = rect2.Width;
					screen_rect.Height = rect2.Height;
					screen_rect.Y = offset_y + rect2.Y;
					screen_rect.X = offset_x + rect2.X;
					VisibleSurface->Lock();
					AlternateSurface->Lock();
					VisibleSurface->Blit_From(screen_rect, *AlternateSurface, rect2);
					AlternateSurface->Unlock();
					VisibleSurface->Unlock();
				}
			}

			min_update_rect.x = 0xFFFFFF;
			min_update_rect.y = 0xFFFFFF;
			max_update_rect.x = 0;
			max_update_rect.y = 0;
		}
	}

	if (show_tooltip) {
		OwnerDraw::Show_Tooltip(true);
	}

	if (message == WM_INITDIALOG) {
		result = 0;
	}
	return(result);
}


/// <summary>
/// Handles the messages for a control with no owner-draw procedure of its own.
/// This is the fallback custom procedure InitializeCtrl hands to any control class the
/// owner-draw system does not paint. Only the edit coloring message is claimed, so that a
/// child edit box picks up the dialog's own font and text color.
/// </summary>
/// <returns>Returns with a null background brush for the edit coloring message; otherwise
/// with the result of the original window procedure.</returns>
LRESULT CALLBACK DefaultCtrlProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	WNDPROC proc = NULL;

	OriginalWndProcs.getValue(window, proc);

	RECT rect1;
	Get_Display_Rect(window, &rect1);

	RECT rect2;
	GetClientRect(window, &rect2);

	WinData *data = NULL;
	ODWinData.getPointer(window, &data);

	if (message == WM_CTLCOLOREDIT) {
		HDC hdc = (HDC)wparam;
		HWND ctrl = (HWND)lparam;
		SetTextColor(hdc, ODColorText);
		SetBkMode(hdc, TRANSPARENT);

		HFONT font = WS_Get_Font(hdc, ODFontName, 0, ODFontSize, 0);
		if (font != NULL) {
			SendMessage(ctrl, WM_SETFONT, (WPARAM)font, 0);
		}

		return((LRESULT)GetStockObject(NULL_BRUSH));
	}

	return(CallWindowProc(proc, window, message, wparam, lparam));
}


/// <summary>
/// Handles the messages for an owner-drawn push button.
/// The button is painted either from an image supplied by the dialog or from the button
/// artwork fitted to the control, with the caption drawn over it and the whole control
/// dimmed while it is disabled. The click sound is played as the button goes down.
/// </summary>
/// <returns>Returns with the result of the original window procedure for anything this
/// routine does not handle itself.</returns>
LRESULT CALLBACK ButtonCtrlProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	WNDPROC proc = NULL;
	OriginalWndProcs.getValue(window, proc);

	RECT drect;
	memset(&drect, 0, sizeof(drect));
	Get_Display_Rect(window, &drect);

	Rect origin(drect.left, drect.top, drect.right - drect.left, drect.bottom - drect.top);

	RECT crect;
	memset(&crect, 0, sizeof(crect));
	GetClientRect(window, &crect);

	WinData * data = NULL;
	ODWinData.getPointer(window, &data);

	int state = data->Button.state;
	LONG style = GetWindowLong(window, GWL_STYLE);

	switch (message) {

		case WM_PAINT: {
			Rect rect = origin;
			COLORREF color = ODColorText;

			/*
			 * Lazily build the render-cache surface that holds the dimmed
			 * background captured behind the button.
			 */
			if (data->cachedSurface == NULL) {
				BSurface * surface = new BSurface(crect.right + 1, crect.bottom + 1, 2);
				data->cachedSurface = surface;
				_surface_count++;

				Rect dest;
				Rect src;
				dest.X = 0;
				dest.Y = 0;
				dest.Width = crect.right + 1;
				dest.Height = crect.bottom + 1;
				src.X = drect.left;
				src.Y = drect.top;
				src.Width = crect.right + 1;
				src.Height = crect.bottom + 1;

				surface->Blit_From(dest, *AlternateSurface, src);
			}

			static char _prev_state = 'u';

			if (data->image != NULL) {

				/*
				 * A user image was supplied -- blit it directly, choosing the
				 * pressed variant when the button is down.
				 */
				Surface * image = data->image;
				if ((state & 1) && data->altImage != NULL) {
					image = data->altImage;
				}

				Rect src = rect;
				src.X = 0;
				src.Y = 0;
				AlternateSurface->Blit_From(rect, *image, src);

			} else {

				/*
				 * No image -- draw the button from its skin pieces and play the
				 * click sound when it first goes down.
				 */
				char updown = 'u';
				if (state & 1) {
					updown = 'd';
				}
				if (style & WS_DISABLED) {
					updown = 'u';
				} else if (updown == 'd' && _prev_state == 'u') {
					Sound_Effect(Rule->GenericClick);
				}

				int widths[2] = {7, 7};
				_prev_state = updown;
				int margins[2] = {10, 10};
				int heights[2] = {24, 30};

				int index = 0;
				for (unsigned int i = 0; i < 2; i++) {
					if (heights[i] > rect.Height && i != 0) {
						break;
					}
					index = i;
				}

				int height = heights[index];
				int w = widths[index];
				int margin = margins[index];

				/*
				 * Restore the cached background before drawing the skin.
				 */
				if (data->cachedSurface != NULL) {
					AlternateSurface->Blit_From(
						Rect(drect.left, drect.top, crect.right + 1, crect.bottom + 1),
						*data->cachedSurface,
						Rect(0, 0, crect.right + 1, crect.bottom + 1));
					InvalidateRect(window, NULL, FALSE);
				}

				origin.Y += (rect.Height - height) / 2;
				if (state & 1) {
					origin.Y += 2;
				}

				Rect destrect;
				Rect sourcerect;
				char buffer[40];

				sprintf(buffer, "b%c%c_li%d.pcx", updown, 'e', height);
				Surface * left = SurfaceCache.GetSurface(buffer);
				origin.Height = left->Get_Height();
				destrect = origin;
				destrect.Width = w;
				sourcerect.Width = w;
				sourcerect.Y = 0;
				sourcerect.X = 0;
				destrect.Height = height;
				sourcerect.Height = height;
				AlternateSurface->Blit_From(destrect, *left, sourcerect);

				sprintf(buffer, "b%c%c_mi%d.pcx", updown, 'e', height);
				Surface * mid = SurfaceCache.GetSurface(buffer);
				sourcerect = origin;
				sourcerect.X += w;
				sourcerect.Width -= margin;
				sourcerect.Height = mid->Get_Height();
				SurfaceCache.Draw(sourcerect, *AlternateSurface, *mid, 0, 0);

				sprintf(buffer, "b%c%c_ri%d.pcx", updown, 'e', height);
				Surface * right = SurfaceCache.GetSurface(buffer);
				destrect = origin;
				destrect.X += origin.Width - margin;
				destrect.Width = margin;
				destrect.Height = right->Get_Height();
				sourcerect.Height = destrect.Height;
				sourcerect.Width = destrect.Width;
				sourcerect.Y = 0;
				sourcerect.X = 0;
				AlternateSurface->Blit_From(destrect, *right, sourcerect);
			}

			/*
			 * Render the caption text (no user image case only).
			 */
			if (data->image == NULL) {
				RECT client;
				GetClientRect(window, &client);
				static char buffer2[256];
				GetWindowText(window, buffer2, 256);

				Rect text_rect(
					origin.X,
					origin.Y + 1,
					origin.Width + origin.X - 2,
					origin.Height + origin.Y - 2);
				if (state & 1) {
					text_rect.X += 2;
					text_rect.Y += 4;
				}
				OD_Draw_Text_Remap(*AlternateSurface, buffer2, text_rect, "dlgsys", color, 5, 0);
			}

			if (style & WS_DISABLED) {
				ODFillRectTrans(rect, *AlternateSurface, 0, 128);
			}
			ValidateRect(window, NULL);
		}
		/// Fall through.

		case WM_ACTIVATE:
		case WM_KILLFOCUS:
		case WM_MOUSEACTIVATE:
			return(0);

		default:
			return(CallWindowProc(proc, window, message, wparam, lparam));
	}
}


/// <summary>
/// Handles the messages for an owner-drawn tab control.
/// The body of the control is painted as dimmed dialog background and each tab is built
/// from its corner and middle artwork, with the caption drawn over it in the remapped
/// font. The active tab gets the brighter text color.
/// </summary>
/// <returns>Returns with the result of the original window procedure for anything this
/// routine does not paint itself.</returns>
LRESULT CALLBACK TextBoxCtrlProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	static char string1[64];

	WinData * data = NULL;
	ODWinData.getPointer(window, &data);

	WNDPROC proc = NULL;
	OriginalWndProcs.getValue(window, proc);

	LRESULT result = 0;

	if (message == WM_ERASEBKGND) {
		return(1);
	}

	if (message == WM_NCPAINT) {
		return(0);
	}

	if (message == OD_SUBCLASSED) {
		RECT rect;
		Get_Display_Rect(window, &rect);

		Surface * surf = SurfaceCache.GetSurface("tab_tlu.pcx");
		SendMessage(window, TCM_SETITEMSIZE, 0, MAKELPARAM(89, surf->Get_Height() - 1));

		if (proc) {
			return(CallWindowProc(proc, window, message, wparam, lparam));
		}
		return(result);
	}

	if (message == WM_PAINT) {
		RECT winrect;
		Get_Display_Rect(window, &winrect);

		TabCtrl_GetItemCount(window);

		RECT tabrect;
		TabCtrl_GetItemRect(window, 0, &tabrect);
		TabCtrl_GetItemRect(window, 0, &tabrect);

		int y = winrect.top + tabrect.bottom - tabrect.top + 3;

		Rect dimrect;
		dimrect.X = winrect.left;
		dimrect.Y = y;
		dimrect.Width = winrect.right - winrect.left;
		dimrect.Height = winrect.bottom - y;

		winrect.top = y;

		ODDrawDimmedBackground(dimrect, window);
		ValidateRect(window, NULL);

		/*
		 * Refilled with the full display rect, although nothing reads it before
		 * the tab loop overwrites it again.
		 */
		dimrect.X = winrect.left;
		dimrect.Y = winrect.top;
		dimrect.Width = winrect.right - winrect.left;
		dimrect.Height = winrect.bottom - winrect.top;

		Rect rect;
		Rect src;

		Surface * image = SurfaceCache.GetSurface("tab_fml.pcx");
		if (image) {
			image->Get_Height();
			rect.Width = image->Get_Width();
			rect.X = winrect.left;
			rect.Y = winrect.top;
			rect.Height = winrect.bottom - winrect.top;
			SurfaceCache.Draw(rect, *AlternateSurface, *image, 0, 0);
		}

		image = SurfaceCache.GetSurface("tab_fmr.pcx");
		if (image) {
			image->Get_Height();
			rect.Width = image->Get_Width();
			rect.X = winrect.right - rect.Width;
			rect.Y = winrect.top;
			rect.Height = winrect.bottom - winrect.top;
			SurfaceCache.Draw(rect, *AlternateSurface, *image, 0, 0);
		}

		image = SurfaceCache.GetSurface("tab_ftm.pcx");
		if (image) {
			int height = image->Get_Height();
			image->Get_Width();
			rect.X = winrect.left;
			rect.Y = winrect.top;
			rect.Width = winrect.right - winrect.left;
			rect.Height = height;
			SurfaceCache.Draw(rect, *AlternateSurface, *image, 0, 0);
		}

		image = SurfaceCache.GetSurface("tab_fbm.pcx");
		if (image) {
			int height = image->Get_Height();
			image->Get_Width();
			rect.X = winrect.left;
			rect.Y = winrect.bottom - height;
			rect.Width = winrect.right - winrect.left;
			rect.Height = height;
			SurfaceCache.Draw(rect, *AlternateSurface, *image, 0, 0);
		}

		image = SurfaceCache.GetSurface("tab_ftl.pcx");
		if (image) {
			int height = image->Get_Height();
			int width = image->Get_Width();
			rect.Y = winrect.top;
			rect.X = winrect.left;
			src.X = 0;
			src.Y = 0;
			src.Width = width;
			src.Height = height;
			rect.Width = width;
			rect.Height = height;
			AlternateSurface->Blit_From(rect, *image, src);
		}

		image = SurfaceCache.GetSurface("tab_ftr.pcx");
		if (image) {
			int height = image->Get_Height();
			int width = image->Get_Width();
			rect.Y = winrect.top;
			rect.X = winrect.right - width;
			src.X = 0;
			src.Y = 0;
			src.Width = width;
			src.Height = height;
			rect.Width = width;
			rect.Height = height;
			AlternateSurface->Blit_From(rect, *image, src);
		}

		image = SurfaceCache.GetSurface("tab_fbl.pcx");
		if (image) {
			int height = image->Get_Height();
			int width = image->Get_Width();
			rect.X = winrect.left;
			rect.Y = winrect.bottom - height;
			src.X = 0;
			src.Y = 0;
			src.Width = width;
			src.Height = height;
			rect.Width = width;
			rect.Height = height;
			AlternateSurface->Blit_From(rect, *image, src);
		}

		image = SurfaceCache.GetSurface("tab_fbr.pcx");
		if (image) {
			int height = image->Get_Height();
			int width = image->Get_Width();
			rect.Y = winrect.bottom - height;
			rect.X = winrect.right - width;
			src.X = 0;
			src.Y = 0;
			src.Width = width;
			src.Height = height;
			rect.Width = width;
			rect.Height = height;
			AlternateSurface->Blit_From(rect, *image, src);
		}

		int current = TabCtrl_GetCurSel(window);
		Get_Display_Rect(window, &winrect);
		int tab = current + 1;
		int itab = tab;

		while (true) {
			RECT itemrect;

			while (!TabCtrl_GetItemRect(window, tab, &itemrect)) {
				itab = 0;
				tab = 0;
			}

			TC_ITEM item;
			memset(&item, 0, sizeof(item));
			item.pszText = string1;
			item.cchTextMax = sizeof(string1);
			strcpy(item.pszText, "Title");
			item.mask = TCIF_TEXT;
			TabCtrl_GetItem(window, tab, &item);

			char state = 'd';
			if (tab == current) {
				state = 'u';
			}

			LONG left = itemrect.left;
			if (itemrect.left >= 6) {
				left = 6;
			}

			Rect tab_rect;
			tab_rect.X = winrect.left + itemrect.left - left;
			tab_rect.Y = itemrect.top + winrect.top;

			LONG right = itemrect.left;
			if (itemrect.left >= 6) {
				right = 6;
			}
			tab_rect.Width = itemrect.right + right - itemrect.left;
			tab_rect.Height = itemrect.bottom - itemrect.top;

			char fname[64];
			Surface * tab_lu = SurfaceCache.GetSurface("tab_tlu.pcx");
			sprintf(fname, "tab_tm%c.pcx", state);
			image = SurfaceCache.GetSurface(fname);
			if (image) {
				int height = image->Get_Height();
				image->Get_Width();
				int width = tab_rect.Width - 2 * tab_lu->Get_Width();

				rect.X = tab_rect.X + tab_lu->Get_Width();
				rect.Y = tab_rect.Y;
				rect.Width = width;
				rect.Height = height;
				SurfaceCache.Draw(rect, *AlternateSurface, *image, 0, 0);
			}

			sprintf(fname, "tab_tl%c.pcx", state);
			image = SurfaceCache.GetSurface(fname);
			if (image) {
				int height = image->Get_Height();
				int width = image->Get_Width();
				rect.X = tab_rect.X;
				rect.Y = tab_rect.Y;
				rect.Width = width;
				rect.Height = height;
				SurfaceCache.DrawTrans(rect, *AlternateSurface, *image, (255 >> DSurface::RedLeft << DSurface::RedRight) | (255u >> DSurface::BlueLeft << DSurface::BlueRight));
			}

			sprintf(fname, "tab_tr%c.pcx", state);
			image = SurfaceCache.GetSurface(fname);
			if (image) {
				int height = image->Get_Height();
				Rect trrect;
				trrect.Width = image->Get_Width();
				trrect.X = tab_rect.X + tab_rect.Width - trrect.Width;
				trrect.Y = tab_rect.Y;
				trrect.Height = height;
				SurfaceCache.DrawTrans(trrect, *AlternateSurface, *image, (255 >> DSurface::RedLeft << DSurface::RedRight) | (255u >> DSurface::BlueLeft << DSurface::BlueRight));
			}

			if (item.pszText) {
				strcpy(string1, item.pszText);
			}

			Rect text_rect;
			text_rect.X = tab_rect.X;
			text_rect.Width = tab_rect.X + tab_rect.Width;
			text_rect.Y = tab_rect.Y + 6;
			text_rect.Height = tab_rect.Y + tab_rect.Height;

			COLORREF color = ODColorTextDim;
			if (itab == current) {
				color = ODColorText;
			}
			OD_Draw_Text_Remap(*AlternateSurface, string1, text_rect, "dlgsys", color, 5, 0);

			ValidateRect(window, &itemrect);
			if (itab == current) {
				break;
			}

			itab++;
			tab = itab;
		}

		ValidateRect(window, NULL);
		return(result);
	}

	if (proc) {
		return(CallWindowProc(proc, window, message, wparam, lparam));
	}
	return(result);
}


/// <summary>
/// Handles the messages for an owner-drawn edit control.
/// The control is inset inside its border and held out of the dialog's tab order until it
/// is deliberately activated, so that a stray keystroke cannot land in it. Return and tab
/// are intercepted -- return notifies the parent dialog, tab moves on to the next control.
/// </summary>
/// <returns>Returns with the result of the original window procedure for anything this
/// routine does not handle itself.</returns>
LRESULT CALLBACK EditBoxCtrlProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	WNDPROC proc = NULL;
	OriginalWndProcs.getValue(window, proc);

	WinData * data = NULL;
	ODWinData.getPointer(window, &data);

	if (GetFocus() == window && !data->Edit.focusEnabled) {
		data->Edit.focusPending = 1;
		SetFocus(MainWindow);
	}

	LONG style = GetWindowLong(window, GWL_STYLE);
	char class_name[32];

	if (message != WM_KEYUP && message != WM_KEYDOWN || wparam != VK_TAB) {

		if (message == OD_SUBCLASSED) {
			RECT window_rect;
			GetWindowRect(window, &window_rect);

			RECT client_rect;
			GetClientRect(window, &client_rect);

			RECT parent_rect;
			GetWindowRect(GetParent(window), &parent_rect);

			MoveWindow(window, window_rect.left - parent_rect.left + 1, window_rect.top - parent_rect.top + 1, client_rect.right - 2, client_rect.bottom - 2, FALSE);

			if (GetFocus() == window) {
				data->Edit.focusPending = 1;
				SetFocus(MainWindow);
			}

			if (style & WS_TABSTOP) {
				data->Edit.hadTabStop = 1;
				SetWindowLong(window, GWL_STYLE, style & ~WS_TABSTOP);
			}
		}

		else if (message == WM_SETFOCUS) {
			SendMessage(window, EM_SETSEL, (WPARAM)-1, (LPARAM)-1);
			if (!data->Edit.focusEnabled) {
				PostMessage(window, OD_REFOCUS, 0, 0);
			}
			goto invalidate_and_default;
		}

		else if (message == WM_GETTEXT) {
			LRESULT text_len = CallWindowProc(proc, window, WM_GETTEXT, wparam, lparam);
			char * buffer = new char[text_len + 2];
			memset(buffer, 0, text_len + 2);

			int out_len = 0;
			int i = 0;
			for (; i < text_len; ++i) {
				char ch = ((char *)lparam)[i];
				if (ch != '\r' && ch != '\n') {
					buffer[out_len++] = ch;
				}
			}

			if (i != out_len) {
				strcat(buffer, "\r\n");
				out_len += 2;
			}

			strcpy((char *)lparam, buffer);
			delete[] buffer;
			return(out_len);
		}

		else if (message == WM_CHAR) {
			if (wparam == VK_RETURN) {
				if (style & ES_MULTILINE) {
					WPARAM len = SendMessage(window, WM_GETTEXTLENGTH, 0, 0) + 3;
					char * text = new char[len];
					SendMessage(window, WM_GETTEXT, len, (LPARAM)text);
					strcat(text, "\r\n");
					SendMessage(window, WM_SETTEXT, 0, (LPARAM)text);
					delete[] text;

					SendMessage(GetParent(window), WM_COMMAND, (unsigned short)GetWindowLong(window, GWL_ID) | 0x5010000, (LPARAM)window);
					return(0);
				}
			} else if (wparam == VK_TAB) {
				HWND next = window;
				HWND tab_item = GetNextDlgTabItem(GetParent(window), window, FALSE);
				if (tab_item != NULL) {
					next = tab_item;
				}
				SetFocus(next);
				return(0);
			}
			goto call_default;
		}

		else if (message == OD_ACTIVATE) {

			int focus_state = data->Edit.focusPending;
			data->Edit.focusEnabled = 1;
			if (focus_state) {
				SetFocus(window);
				data->Edit.focusPending = 0;
			}
			if (data->Edit.hadTabStop) {
				SetWindowLong(window, GWL_STYLE, style | WS_TABSTOP);
			}
		}

		else {
			if (message == WM_PAINT || message == WM_ERASEBKGND) {
				RECT display_rect;
				Get_Display_Rect(window, &display_rect);

				GetClassName(GetParent(window), class_name, sizeof(class_name)/2);
				bool is_combo = strcmp(class_name, "ComboBox") == 0;

				RECT update_rect;
				if (message == WM_PAINT && GetUpdateRect(window, &update_rect, FALSE)) {
					update_rect.right += display_rect.left;
					update_rect.left += display_rect.left;
					update_rect.top += display_rect.top;
					update_rect.bottom += display_rect.top;
				}

				Rect draw_rect;
				draw_rect.X = display_rect.left;
				draw_rect.Y = display_rect.top;
				draw_rect.Width = display_rect.right - display_rect.left + 1;
				draw_rect.Height = display_rect.bottom - display_rect.top + 1;

				ODDrawDimmedBackground(draw_rect, window);
				if (!is_combo) {
					OD_Draw_Rect(*AlternateSurface, draw_rect, 1, 0xFFFFFFFF);
				}

				static char _buffer[512];
				SendMessage(window, WM_GETTEXT, 500, (LPARAM)_buffer);

				Rect text_rect;
				text_rect.X = display_rect.left;
				text_rect.Y = display_rect.top;
				text_rect.Width = 0;
				text_rect.Height = 0;

				if (GetWindowLong(window, GWL_STYLE) & ES_PASSWORD) {
					for (int i = 0; i < (int)strlen(_buffer); ++i) {
						_buffer[i] = '*';
					}
				}

				OD_Draw_Text(ODColorText, ODFontPtr, text_rect, _buffer, strlen(_buffer), 0, 0, 0);

				WPARAM em_wparam;
				LPARAM em_lparam;
				unsigned int sel = ((unsigned int)SendMessage(window, EM_GETSEL, (WPARAM)&em_wparam, (LPARAM)&em_lparam)) >> 16;
				if (HIWORD(sel) > LOWORD(sel)) {
					sel >>= 16;
				}

				int charidx = LOWORD(sel);
				if (charidx < (int)strlen(_buffer)) {
					SendMessage(window, EM_POSFROMCHAR, charidx, 0);
				}

				ValidateRect(window, NULL);
			}

			if (message == WM_CONTEXTMENU) {
				return(1);
			}

			if (message == WM_MOUSEMOVE) {
				return(1);
			}

			if (message == WM_KEYDOWN || message == WM_KEYUP || message == WM_SYSKEYDOWN || message == WM_SYSKEYUP || message == WM_SYSCHAR || message == WM_SYSDEADCHAR || message == WM_KILLFOCUS || message == WM_LBUTTONDOWN) {
			invalidate_and_default:
				GetClassName(GetParent(window), class_name, sizeof(class_name));
				if (strcmp(class_name, "ComboBox") == 0) {
					InvalidateRect(GetParent(window), NULL, FALSE);
				}
				InvalidateRect(window, NULL, FALSE);
			}

		call_default:
			return(CallWindowProc(proc, window, message, wparam, lparam));
		}
	}
	return(0);
}


/// <summary>
/// Handles the messages for an owner-drawn static text control.
/// The caption is kept in the control's owner-draw record rather than in the window, so it
/// can be drawn in the remapped font over a cached copy of the dialog background. The
/// dialog can recolor the text at any time with OD_SETCOLOR.
/// </summary>
/// <returns>Returns with the result of the original window procedure for anything this
/// routine does not handle itself.</returns>
LRESULT CALLBACK StaticCtrlProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	WinData * data = NULL;

	switch (message) {

		case WM_SETTEXT: {
			ODWinData.getPointer(window, &data);

			delete[] data->Static.text;

			const char * text = (const char *)lparam;
			unsigned int text_len = strlen(text) + 1;
			char * copy = new char[text_len];
			memset(copy, 0, text_len);
			strcpy(copy, text);

			data->Static.text = copy;

			Surface * cachedSurf = data->cachedSurface;
			if (cachedSurf != NULL) {
				RECT drect;
				Get_Display_Rect(window, &drect);

				RECT crect;
				GetClientRect(window, &crect);

				Rect dst_rect;
				dst_rect.X = drect.left;
				dst_rect.Y = drect.top;
				dst_rect.Width = crect.right + 1;
				dst_rect.Height = crect.bottom + 1;

				Rect src_rect;
				src_rect.X = 0;
				src_rect.Y = 0;
				src_rect.Width = crect.right + 1;
				src_rect.Height = crect.bottom + 1;

				AlternateSurface->Blit_From(dst_rect, *cachedSurf, src_rect);
				InvalidateRect(window, NULL, FALSE);
			}
			return(1);
		}

		case WM_DESTROY: {
			ODWinData.getPointer(window, &data);

			if (data->Static.text != NULL) {
				delete[] data->Static.text;
				data->Static.text = NULL;
			}
			if (data->cachedSurface != NULL) {
				delete data->cachedSurface;
				data->cachedSurface = NULL;
			}
			break;
		}

		case WM_MOVE:
		case WM_SIZE:
		case WM_WINDOWPOSCHANGED: {
			if (ODWinData.getPointer(window, &data)) {
				if (data != NULL) {
					delete data->cachedSurface;
					data->cachedSurface = NULL;
				}
			}
			InvalidateRect(window, NULL, FALSE);
			return(0);
		}

		case WM_GETTEXT: {
			ODWinData.getPointer(window, &data);

			const char * text = data->Static.text;
			if (strlen(text) + 1 < wparam) {
				wparam = strlen(text) + 1;
			}
			strncpy((char *)lparam, text, wparam);
			return(wparam);
		}

		case WM_PAINT: {
			char text[2048];
			text[2047] = '\0';

			ODWinData.getPointer(window, &data);

			if (data->cachedSurface == NULL) {
				RECT drect;
				Get_Display_Rect(window, &drect);

				RECT crect;
				GetClientRect(window, &crect);

				BSurface * surf = new BSurface(crect.right + 1, crect.bottom + 1, 2);
				data->cachedSurface = surf;
				++_surface_count;

				Rect dst_rect;
				dst_rect.X = 0;
				dst_rect.Y = 0;
				dst_rect.Width = crect.right + 1;
				dst_rect.Height = crect.bottom + 1;

				Rect src_rect;
				src_rect.X = drect.left;
				src_rect.Y = drect.top;
				src_rect.Width = crect.right + 1;
				src_rect.Height = crect.bottom + 1;

				surf->Blit_From(dst_rect, *AlternateSurface, src_rect);
			}

			Rect text_rect;
			Get_Display_Rect(window, (LPRECT)&text_rect);

			GetWindowText(window, text, 2047);

			LONG style = GetWindowLong(window, GWL_STYLE);
			int draw_flags = 16;

			if (style & SS_CENTER) {
				draw_flags = 17;
			} else if (style & SS_RIGHT) {
				draw_flags = 18;
			}

			COLORREF color = data->Static.textColor;
			if ((style & WS_DISABLED) != 0) {
				color = ODColorDisabled;
			}

			OD_Draw_Text_Remap(*AlternateSurface, text, text_rect, "dlgsys", color, draw_flags, 0);

			ValidateRect(window, NULL);
			return(0);
		}

		case OD_SUBCLASSED: {
			WNDPROC proc = NULL;
			ODWinData.getPointer(window, &data);
			OriginalWndProcs.getValue(window, proc);

			unsigned int len = SendMessage(window, WM_GETTEXTLENGTH, 0, 0) + 1;
			char * text = new char[len];
			memset(text, 0, len);

			CallWindowProc(proc, window, WM_GETTEXT, len, (LPARAM)text);

			data->Static.text = text;
			data->Static.textColor = ODColorText;
			return(0);
		}

		case OD_SETCOLOR: {
			ODWinData.getPointer(window, &data);

			if (data != NULL) {
				if ((COLORREF)lparam != data->Static.textColor) {
					InvalidateRect(window, NULL, FALSE);
				}
				if (lparam == -1) {
					data->Static.textColor = ODColorText;
				} else {
					data->Static.textColor = lparam;
				}
			}
			return(0);
		}

		default:
			break;
	}

	WNDPROC proc = NULL;
	OriginalWndProcs.getValue(window, proc);
	return(CallWindowProc(proc, window, message, wparam, lparam));
}


/// <summary>
/// Handles the messages for an owner-drawn check box.
/// The checked state is kept in the control's owner-draw record and painted from the check
/// box artwork, dimmed when the control is disabled. A click on the box itself toggles the
/// state, plays the click sound and notifies the parent dialog.
/// </summary>
/// <returns>Returns with the result of the original window procedure for anything this
/// routine does not handle itself.</returns>
LRESULT CALLBACK CheckBoxCtrlProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	WinData* data = NULL;
	ODWinData.getPointer(window, &data);

	switch (message) {

		case BM_GETCHECK: {
			return(data->CheckBox.checkState);
		}

		case WM_SETFOCUS:
		case WM_KILLFOCUS: {
			InvalidateRect(window, NULL, FALSE);
			break;
		}


		case WM_PAINT: {
			/*
			 * DEAD CODE
			 */
			RECT cr;
			GetClientRect(window, &cr);
			Rect trect;
			Get_Display_Rect(window, (LPRECT)&trect);
			int somebool = 0;
			if (data->CheckBox.checkState == 1) {
				somebool = 1;
			}
			RECT disprect;
			Get_Display_Rect(window, (LPRECT)&disprect);
			Rect drect;
			drect.X = disprect.left;
			drect.Y = disprect.top;
			drect.Width = 18;
			drect.Height = 18;
			int style = GetWindowLong(window, GWL_STYLE);
			char letter = 'u';
			if (somebool) {
				letter = 'c';
			}
			char buf[64];
			sprintf(buf, "c%ce_i.pcx", letter);
			Surface *image = SurfaceCache.GetSurface(buf);
			int image_height = image->Get_Height();
			Rect srect;
			srect.Width = image->Get_Width();
			srect.X = 0;
			srect.Y = 0;
			srect.Height = image_height;
			AlternateSurface->Blit_From(drect, *image, srect);

			if (style & WS_DISABLED) {
				ODFillRectTrans(drect, *AlternateSurface, 0, 128);
			}

			static char _wintext[128];
			GetWindowText(window, _wintext, sizeof(_wintext) - 1);

			trect.X += 20;
			trect.Width -= 20;
			COLORREF color = ODColorText;
			if (style & WS_DISABLED) {
				color = ODColorDisabled;
			}
			OD_Draw_Text_Remap(*AlternateSurface, _wintext, trect, "dlgsys", color, 4, 0);
			ValidateRect(window, NULL);
			return(0);
		}

		case BM_SETCHECK: {
			data->CheckBox.checkState = wparam;
			InvalidateRect(window, NULL, FALSE);
			return(0);
		}

		case WM_LBUTTONDOWN:
		case WM_LBUTTONDBLCLK: {
			int xpos = (unsigned short)LOWORD(lparam);
			int ypos = (unsigned short)HIWORD(lparam);
			if (xpos < 18 && ypos < 18) {
				int checked = data->CheckBox.checkState != 1;
				data->CheckBox.checkState = checked;
				InvalidateRect(window, NULL, FALSE);
				Sound_Effect(Rule->GenericClick);
				HWND parent = GetParent(window);
				SendMessage(parent, WM_COMMAND, MAKEWPARAM(GetWindowLong(window, GWL_ID), checked), (LPARAM)window);
				return(0);
			} else {
				return(0);
			}
		}

		case OD_SUBCLASSED: {
			WNDPROC subproc = NULL;
			OriginalWndProcs.getValue(window, subproc);
			data->CheckBox.checkState = CallWindowProc(subproc, window, BM_GETCHECK, 0L, 0L);
			break;
		}
	}

	WNDPROC proc = NULL;
	OriginalWndProcs.getValue(window, proc);
	return(CallWindowProc(proc, window, message, wparam, lparam));
}


/// <summary>
/// Handles the messages for an owner-drawn combo box.
/// The closed box is painted with its arrow button and the text of the current selection.
/// A click on the arrow drops the list, which is a ComboDropWin window created and
/// destroyed here rather than the stock Windows list.
/// </summary>
/// <returns>Returns with the result of the original window procedure for anything this
/// routine does not handle itself.</returns>
LRESULT CALLBACK ComboBoxCtrlProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	static char _buffer[256];
	RECT display_rect;
	RECT client_rect;
	RECT window_rect;
	Get_Display_Rect(window, &display_rect);
	GetClientRect(window, &client_rect);
	GetWindowRect(window, &window_rect);

	WinData * data = NULL;
	ODWinData.getPointer(window, &data);

	switch (message) {
		case WM_LBUTTONDOWN:
		case WM_LBUTTONDBLCLK: {
			Sound_Effect(Rule->GenericClick);
			if ((unsigned short)LOWORD(lparam) > client_rect.right - 20) {
				LRESULT dropped = SendMessage(window, CB_GETDROPPEDSTATE, 0, 0);
				PostMessage(window, CB_SHOWDROPDOWN, (WPARAM)(dropped != 1), 0);
			}
			return(0);
		}

		case WM_ERASEBKGND:
			return(0);

		case WM_DESTROY:
			SendMessage(window, CB_SHOWDROPDOWN, FALSE, 0);
			break;

		case WM_PAINT: {
			LRESULT dropped = SendMessage(window, CB_GETDROPPEDSTATE, 0, 0);
			RECT dropped_rect;
			RECT wrect;
			GetWindowRect(window, &wrect);	/// result is not used
			SendMessage(window, CB_GETDROPPEDCONTROLRECT, 0, (LPARAM)&dropped_rect);

			Rect rect;
			rect.X = display_rect.left;
			rect.Y = display_rect.top;
			rect.Width = display_rect.right - display_rect.left;
			rect.Height = 24;

			dropped_rect.top += (display_rect.bottom - display_rect.top + 1);
			dropped_rect.bottom = dropped_rect.bottom + display_rect.top - display_rect.bottom - 1;

			int width = display_rect.right - display_rect.left;
			int height = display_rect.bottom - display_rect.top;

			HWND parent = GetParent(window);
			WinData * parent_data = NULL;
			if (parent) {
				ODWinData.getPointer(parent, &parent_data);
			}

			RECT parent_rect;
			Get_Display_Rect(parent, &parent_rect);

			Rect dst_rect(0, 0, width, height);
			Rect src_rect(0, 0, width, height);
			if (parent_data != NULL && parent_data->cachedSurface != NULL) {
				src_rect.X = display_rect.left - parent_rect.left;
				src_rect.Y = display_rect.top - parent_rect.top;
			}

			if (data->cachedSurface == NULL) {
				Surface * surface = new BSurface(width, height, 2);
				data->cachedSurface = surface;
				_surface_count++;

				if (parent_data != NULL && parent_data->cachedSurface != NULL) {
					surface->Blit_From(dst_rect, *parent_data->cachedSurface, src_rect, false, true);
				}

				int total = width * height;
				unsigned short * surfptr = (unsigned short *)surface->Lock();
				if (total > 0) {
					for (int i = 0; i < total; ++i) {
						surfptr[i] = OD_Blend_Color(surfptr[i], 0, ODColorSteps);
					}
				}
				if (surfptr != NULL) {
					surface->Unlock();
				}
			}

			ODDrawDimmedBackground(rect, window);
			ODFillRectTrans(rect, *AlternateSurface, 0, 128);
			OD_Draw_Rect(*AlternateSurface, rect, 1, 0xFFFFFFFF);

			Rect arrow_rect;
			arrow_rect.X = display_rect.right - 19;
			arrow_rect.Y = rect.Y + 1;
			arrow_rect.Width = rect.Width;
			arrow_rect.Height = rect.Height;
			ODDrawArrowBitmap(*AlternateSurface, arrow_rect, dropped, dropped);

			LONG style = GetWindowLong(window, GWL_STYLE);
			if ((style & WS_DISABLED) != 0) {
				ODFillRectTrans(rect, *AlternateSurface, 0, 128);
			}

			if ((style & 3) == 3) {
				sprintf(_buffer, "NULL");
				GetWindowText(window, _buffer, sizeof(_buffer));

				COLORREF text_color = ODColorText;
				if ((style & WS_DISABLED) != 0) {
					text_color = ODColorDisabled;
				}

				FontMetrics font_data;
				if (ODGetFontMetrics("dlgsys", &font_data)) {
					int text_width = 0;
					int max_width = client_rect.right - 28;
					int ellipsis_width = 3 * font_data.charWidths['.'];
					bool clipped = false;
					for (int i = 0; i < (int)strlen(_buffer); ++i) {
						text_width += font_data.charWidths[(unsigned char)_buffer[i]];
					}

					if (text_width >= max_width) {
						while (strlen(_buffer) > 0) {
							int last = (int)strlen(_buffer) - 1;
							text_width -= font_data.charWidths[(unsigned char)_buffer[last]];
							_buffer[last] = '\0';
							if (!clipped) {
								text_width += ellipsis_width;
							}
							clipped = true;
							if (text_width < max_width) {
								strcat(_buffer, "...");
								break;
							}
						}
					}
				}

				RECT text_rect;
				text_rect.left = display_rect.left + 2;
				text_rect.right = display_rect.right;
				text_rect.top = display_rect.top + 3;
				text_rect.bottom = display_rect.bottom;
				OD_Draw_Text_Remap(*AlternateSurface, _buffer, *(Rect *)&text_rect, "dlgsys", text_color, 4, 0);
				ValidateRect(window, NULL);
				return(0);
			}

			ValidateRect(window, NULL);
			return(0);
		}

		case CB_SHOWDROPDOWN: {
			int result = 1;
			if (wparam == 0) {
				if (data->ComboBox.dropdown != NULL) {
					ReleaseCapture();
					HWND parent = GetParent(window);
					SendMessage(parent, OD_SETTOP, (WPARAM)data->ComboBox.dropdown, 0);

					HWND dropdown_window = data->ComboBox.dropdown;
					DestroyWindow(dropdown_window);

					WinData * dropdown_data = NULL;
					ODWinData.getPointer(dropdown_window, &dropdown_data);
					if (dropdown_data != NULL && dropdown_data->cachedSurface != NULL) {
						delete dropdown_data->cachedSurface;
						dropdown_data->cachedSurface = NULL;
						_surface_count--;
					}

					ODWinData.remove(dropdown_window);
					data->ComboBox.dropdown = NULL;
				}
				return(result);
			}

			if (data->ComboBox.dropdown != NULL) {
				return(result);
			}

			SetFocus(window);

			RECT parent_rect;
			Get_Display_Rect(GetParent(window), &parent_rect);

			int count = (int)SendMessage(window, CB_GETCOUNT, 0, 0);
			int item_height = (int)SendMessage(window, CB_GETITEMHEIGHT, 0, 0);
			int dropdown_height = count * item_height + 4;

			if (display_rect.bottom + dropdown_height > parent_rect.bottom - 2 * item_height) {
				dropdown_height = parent_rect.bottom - (2 * item_height + 4) - display_rect.bottom;
				if (dropdown_height < item_height) {
					dropdown_height = parent_rect.bottom - display_rect.bottom;
					dropdown_height -= dropdown_height % item_height;
				}
			}

			RECT parent_display;
			RECT combo_display;
			Get_Display_Rect(GetParent(window), &parent_display);
			Get_Display_Rect(window, &combo_display);

			int x = combo_display.left - parent_display.left;
			int y = client_rect.bottom + combo_display.top - parent_display.top + 2;
			int w = client_rect.right;
			HWND dropdown_window = CreateWindowEx(
				0,
				"ComboDropWin",
				NULL,
				WS_CHILD,
				x,
				y,
				w,
				dropdown_height,
				GetParent(window),
				NULL,
				ProgramInstance,
				window);

			WinData * dropdown_data = NULL;
			if (!ODWinData.getPointer(dropdown_window, &dropdown_data)) {
				WinData tmp;
				memset(&tmp, 0, sizeof(tmp));
				ODWinData.add(dropdown_window, tmp);
			}

			SendMessage(dropdown_window, OD_DROPSUBCLASSED, 0, 0);
			SendMessage(GetParent(window), OD_SETTOP, (WPARAM)dropdown_window, 1);
			SetCapture(dropdown_window);
			ShowWindow(dropdown_window, 1);

			data->ComboBox.dropdown = dropdown_window;
			return(result);
		}

		case OD_SUBCLASSED: {
			if (data->itemHeightSet) {
				if (SendMessage(window, CB_GETITEMHEIGHT, 0, 0) == ODFontSize + 6) {
					memset(data->ComboBox.itemColors, 0xFF, sizeof(data->ComboBox.itemColors));
					break;
				}
			}

			SendMessage(window, CB_SETITEMHEIGHT, (WPARAM)-1, ODFontSize + 2);
			SendMessage(window, CB_SETITEMHEIGHT, 0, ODFontSize + 6);
			data->itemHeightSet = 1;
			memset(data->ComboBox.itemColors, 0xFF, sizeof(data->ComboBox.itemColors));
			break;
		}

		case OD_SETCOLOR:
			if ((unsigned int)wparam <= 50) {
				data->ComboBox.itemColors[wparam] = lparam;
			}
			break;
	}

	WNDPROC proc = NULL;
	OriginalWndProcs.getValue(window, proc);
	return(CallWindowProc(proc, window, message, wparam, lparam));
}


/// <summary>
/// Handles the messages for an owner-drawn list box.
/// Besides repainting the stock list box, this routine provides the multi-column list the
/// dialogs are built around: the columns and the text, color and icon of each cell live in
/// the control's owner-draw record. A scroll bar is attached to, or taken away from, the
/// list as its contents demand.
/// </summary>
/// <returns>Returns with the result of the original window procedure for anything this
/// routine does not handle itself.</returns>
LRESULT CALLBACK ListBoxCtrlProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	HWND parent = GetParent(window);
	int needs_scrollbar = -1;
	int max_position = 0;
	char call_default = 1;
	LRESULT result = 0;
	char string[512];

	WinData * data = NULL;
	ODWinData.getPointer(window, &data);

	int scrollbar_width = 2 * ODBorderThickness + 18;

	RECT client_rect;
	GetClientRect(window, &client_rect);
	RECT display_rect;
	Get_Display_Rect(window, &display_rect);
	display_rect.right -= ODBorderThickness;
	display_rect.left += ODBorderThickness;

	Rect content_rect;
	content_rect.X = display_rect.left;
	client_rect.right -= 2 * ODBorderThickness;
	content_rect.Width = client_rect.right - client_rect.left;
	display_rect.top += ODBorderThickness;
	content_rect.Y = display_rect.top;
	client_rect.bottom -= 2 * ODBorderThickness;
	display_rect.bottom -= ODBorderThickness;
	content_rect.Height = client_rect.bottom - client_rect.top;

	/*
	 * Keep the attached scrollbar synchronized with the listbox state. This is skipped for
	 * the few messages that are queried while computing the scroll state (to avoid recursion).
	 */
	if (message != LB_GETCOUNT && message != LB_GETITEMHEIGHT && message != WM_VSCROLL) {
		int count = SendMessage(window, LB_GETCOUNT, 0, 0);
		int item_height = SendMessage(window, LB_GETITEMHEIGHT, 0, 0);
		if (item_height <= 1) {
			item_height = 1;
		}
		needs_scrollbar = (count * item_height > client_rect.bottom - client_rect.top);
		max_position = count - (client_rect.bottom - client_rect.top) / item_height;
		if ((unsigned int)data->attachedWindow > 1) {
			SCROLLINFO info;
			info.fMask = SIF_RANGE | SIF_POS;
			info.nMin = 0;
			info.nMax = max_position;
			info.nPos = data->ListBox.topIndex;
			info.cbSize = sizeof(SCROLLINFO);
			SendMessage(data->attachedWindow, SBM_SETSCROLLINFO, 0, (LPARAM)&info);
		}
	}

	switch (message) {
		case WM_ERASEBKGND:
			return(0);

		case WM_PAINT: {
			call_default = 0;
			ODDrawDimmedBackground(content_rect, window);
			OD_Draw_Rect(*AlternateSurface, content_rect, 1, 0xFFFFFFFF);

			int fill_color = ODColorToHiColor(ODListBoxColor);

			RECT update_rect;
			if (!GetUpdateRect(window, &update_rect, FALSE)) {
				break;
			}

			int count = SendMessage(window, LB_GETCOUNT, 0, 0);
			int index = SendMessage(window, LB_GETTOPINDEX, 0, 0);
			while (index < count) {
				RECT item_rect;
				if (SendMessage(window, LB_GETITEMRECT, index, (LPARAM)&item_rect) != -1) {
					if (client_rect.top + item_rect.bottom > client_rect.bottom) {
						break;
					}
					SendMessage(window, LB_GETTEXT, index, (LPARAM)string);

					ArrayList<ColumnData> * columns = data->ListBox.columns;
					if (columns != NULL) {
						if (SendMessage(window, LB_GETSEL, index, 0) > 0) {
							Rect fill;
							fill.X = item_rect.left + display_rect.left;
							fill.Y = item_rect.top + display_rect.top;
							fill.Width = item_rect.right - item_rect.left;
							fill.Height = item_rect.bottom - item_rect.top;
							AlternateSurface->Fill_Rect(fill, fill_color);
						}

						HDC dc = GetDC(window);
						for (int col = 0; col < columns->length(); col++) {
							ColumnData * column = NULL;
							columns->getPointer(&column, col);
							CellData * cell = NULL;
							column->cells.getPointer(&cell, index);
							if (cell == NULL || cell->type == CellData::INVALID) {
								continue;
							}

							if (cell->type == CellData::TEXT || cell->type == CellData::PRIMARY) {
								Rect text_rect;
								text_rect.X = display_rect.left + item_rect.left + column->xPos;
								text_rect.Y = display_rect.top + item_rect.top;
								text_rect.Width = 0;
								text_rect.Height = 0;
								if (cell->type == CellData::TEXT) {
									strcpy(string, cell->string.get());
								} else if (cell->type == CellData::PRIMARY) {
									SendMessage(window, LB_GETTEXT, index, (LPARAM)string);
								}
								COLORREF text_color = cell->color;
								if (text_color == -1) {
									text_color = ODColorText;
								}
								int max_width = column->width;
								if (max_width == 0) {
									max_width = 0xFFFF;
								}
								if (display_rect.right - max_width - column->xPos - item_rect.left - display_rect.left < 0) {
									max_width = display_rect.right - column->xPos - item_rect.left - display_rect.left;
								}
								SendMessage(window, OD_RESTOREDC, 0, (LPARAM)dc);
								SIZE ellipsis_size;
								GetTextExtentPoint32(dc, "...", strlen("..."), &ellipsis_size);
								SIZE text_size;
								GetTextExtentPoint32(dc, string, strlen(string), &text_size);
								if (text_size.cx > max_width) {
									while (true) {
										int len = strlen(string);
										if (!len) {
											break;
										}
										string[strlen(string) - 1] = '\0';
										GetTextExtentPoint32(dc, string, strlen(string), &text_size);
										text_size.cx += ellipsis_size.cx;
										if (text_size.cx <= max_width) {
											strcat(string, "...");
											break;
										}
									}
								}
								OD_Draw_Text(text_color, data->font, text_rect, string, strlen(string), 0, 0, NULL);
							} else if (cell->type == CellData::SURFACE) {
								Surface * surface = cell->surf;
								if (surface != NULL) {
									int height = surface->Get_Height();
									int width = surface->Get_Width();
									Rect surface_rect;
									surface_rect.Width = width;
									surface_rect.X = display_rect.left + item_rect.left + column->xPos;
									surface_rect.Height = height;
									surface_rect.Y = display_rect.top + item_rect.top + (item_rect.bottom - item_rect.top - height) / 2;
									SurfaceCache.DrawTrans(surface_rect, *AlternateSurface, *surface, (255 >> DSurface::RedLeft << DSurface::RedRight) | (255u >> DSurface::BlueLeft << DSurface::BlueRight));
								}
							} else {
								int ping = cell->pingtime;
								Rect ping_rect;
								ping_rect.X = display_rect.left + item_rect.left + column->xPos;
								ping_rect.Y = display_rect.top + item_rect.top;
								ping_rect.Width = 28;
								ping_rect.Height = 12;
								unsigned color;
								if (ping < 300) {
									color = DSurface::Build_Hicolor_Pixel(0, 192, 0);
								} else if (ping < 500) {
									color = DSurface::Build_Hicolor_Pixel(192, 192, 0);
								} else {
									color = DSurface::Build_Hicolor_Pixel(192, 0, 0);
								}
								ODDrawGradientRect(ping_rect, *AlternateSurface, color, (ping << 16) / 1000);
							}
						}
						ReleaseDC(window, dc);
					} else {
						ArrayList<int> * row_colors = data->ListBox.rowColors;
						COLORREF text_color;
						int * color_ptr = NULL;
						if (row_colors == NULL || !row_colors->getPointer(&color_ptr, index) || *color_ptr == -1) {
							text_color = ODColorText;
						} else {
							text_color = *color_ptr;
						}

						if (SendMessage(window, LB_GETSEL, index, 0) > 0) {
							RECT fill;
							fill.left = item_rect.left + display_rect.left;
							fill.top = item_rect.top + display_rect.top;
							fill.bottom = item_rect.bottom - item_rect.top;
							fill.right = item_rect.right - item_rect.left;
							AlternateSurface->Fill_Rect(*(Rect *)&fill, fill_color);
						}

						Rect text_rect;
						int max_width = item_rect.right - item_rect.left;
						text_rect.X = item_rect.left + display_rect.left + 2;
						text_rect.Y = display_rect.top + item_rect.top;
						text_rect.Width = 0;
						text_rect.Height = 0;
						if (item_rect.right == item_rect.left) {
							max_width = 0xFFFF;
						}
						HDC dc = GetDC(window);
						if (data->font != NULL) {
							SelectObject(dc, data->font);
						}
						SIZE ellipsis_size;
						GetTextExtentPoint32(dc, "...", strlen("..."), &ellipsis_size);
						SIZE text_size;
						GetTextExtentPoint32(dc, string, strlen(string), &text_size);
						if (text_size.cx > max_width && text_size.cx + ellipsis_size.cx > max_width) {
							while (true) {
								int len = strlen(string);
								if (!len) {
									break;
								}
								string[strlen(string) - 1] = '\0';
								GetTextExtentPoint32(dc, string, strlen(string), &text_size);
								if (text_size.cx + ellipsis_size.cx <= max_width) {
									strcat(string, "...");
									break;
								}
							}
						}
						OD_Draw_Text(text_color, data->font, text_rect, string, strlen(string), 0, 0, NULL);
					}
				}
				index++;
			}
			ValidateRect(window, &update_rect);
			break;
		}

		case WM_SIZE: {
			if ((int)data->attachedWindow > 1) {
				RECT parent_display;
				Get_Display_Rect(GetParent(window), &parent_display);
				Rect win_display;
				Get_Display_Rect(window, (LPRECT)&win_display);
				MoveWindow(data->attachedWindow, win_display.Width - parent_display.left, win_display.Y - parent_display.top, scrollbar_width, win_display.Height - win_display.Y, TRUE);
			}
			Surface * surface = data->cachedSurface;
			if (surface != NULL) {
				if ((unsigned short)lparam != surface->Get_Width() || HIWORD(lparam) != surface->Get_Height()) {
					WinData * cache_data = NULL;
					ODWinData.getPointer(window, &cache_data);
					if (cache_data != NULL && cache_data->cachedSurface != NULL) {
						delete cache_data->cachedSurface;
						cache_data->cachedSurface = NULL;
						_surface_count--;
					}
				}
			}
			break;
		}

		case WM_SETFONT: {
			HDC dc = GetDC(window);
			TEXTMETRIC tm;
			GetTextMetrics(dc, &tm);
			ReleaseDC(window, dc);
			SendMessage(window, LB_SETITEMHEIGHT, (WPARAM)-1, (unsigned short)(LOWORD(tm.tmHeight) + 2));
			data->font = (HFONT)wparam;
			return(0);
		}

		case WM_VSCROLL: {
			call_default = 0;
			LRESULT position = SendMessage(data->attachedWindow, SBM_GETPOS, 0, 0);
			if (position != SendMessage(window, LB_GETTOPINDEX, 0, 0)) {
				SendMessage(window, LB_SETTOPINDEX, position, 0);
			}
			break;
		}

		case LB_ADDSTRING:
			wparam = (WPARAM)-1;
			/// Fall through to insert with an append position.
		case LB_INSERTSTRING: {
			int position = (int)wparam;
			if (position != -1) {
				ArrayList<int> * row_colors = data->ListBox.rowColors;
				if (row_colors != NULL) {
					int color = -1;
					row_colors->add(color, position);
				}
				ArrayList<int> * sel_states = data->ListBox.selStates;
				if (sel_states != NULL) {
					int selected = 0;
					sel_states->add(selected, position);
				}
			}
			if (position < 0) {
				position = SendMessage(window, LB_GETCOUNT, 0, 0);
			}

			ArrayList<ColumnData> * columns = data->ListBox.columns;
			CellData cell;
			if (columns != NULL) {
				for (int col = 0; col < columns->length(); col++) {

					/*
					 * The first column added implicitly holds the row's listbox string,
					 * so the new row gets a PRIMARY cell there and INVALID cells elsewhere.
					 */
					cell.type = (col == 0) ? CellData::PRIMARY : CellData::INVALID;
					ColumnData * column = NULL;
					columns->getPointer(&column, col);
					column->cells.add(cell, position);
				}
			}
			break;
		}

		case LB_SETSEL: {
			int index = (int)lparam;
			if (index < -1) {
				return(-1);
			}
			ArrayList<int> * sel_states = data->ListBox.selStates;
			if (sel_states == NULL) {
				sel_states = new ArrayList<int>;
				data->ListBox.selStates = sel_states;
			}
			if (index >= SendMessage(window, LB_GETCOUNT, 0, 0) - 1) {
				index = SendMessage(window, LB_GETCOUNT, 0, 0) - 1;
			}
			if (index >= sel_states->length()) {
				int unselected = 0;
				sel_states->setSize(index + 1, unselected);
			}
			if (index == -1) {
				for (int i = 0; i < sel_states->length(); i++) {
					int selected = (int)wparam;
					sel_states->replace(selected, i);
				}
			} else {
				int selected = (int)wparam;
				sel_states->replace(selected, index);
			}
			parent = GetParent(window);
			SendMessage(parent, WM_COMMAND, (GetWindowLong(window, GWL_ID) & 0xFFFF) | 0x10000, (LPARAM)window);
			InvalidateRect(window, NULL, FALSE);
			return(0);
		}

		case LB_GETSEL: {
			ArrayList<int> * sel_states = data->ListBox.selStates;
			if (sel_states == NULL) {
				return(0);
			}
			if ((int)wparam >= sel_states->length()) {
				return(0);
			}
			int * selected = NULL;
			if (!sel_states->getPointer(&selected, (int)wparam)) {
				return(0);
			}
			return(*selected);
		}

		case LB_SETCURSEL: {
			int index = (int)wparam;
			if ((int)wparam >= -1 && index < SendMessage(window, LB_GETCOUNT, 0, 0)) {
				if (data->ListBox.curSel != -1) {
					SendMessage(window, LB_SETSEL, 0, data->ListBox.curSel);
				}
				data->ListBox.curSel = index;
				if (index != -1) {
					SendMessage(window, LB_SETSEL, TRUE, index);
				}
			}
			parent = GetParent(window);
			SendMessage(parent, WM_COMMAND, (GetWindowLong(window, GWL_ID) & 0xFFFF) | 0x10000, (LPARAM)window);
			InvalidateRect(window, NULL, FALSE);
			return(0);
		}

		case LB_GETCURSEL:
			return(data->ListBox.curSel);

		case LB_DELETESTRING: {
			ArrayList<int> * row_colors = data->ListBox.rowColors;
			if (row_colors != NULL && row_colors->length() > (int)wparam) {
				row_colors->remove((int)wparam);
			}
			ArrayList<int> * sel_states = data->ListBox.selStates;
			if (sel_states != NULL && sel_states->length() > (int)wparam) {
				sel_states->remove((int)wparam);
			}
			ArrayList<ColumnData> * columns = data->ListBox.columns;
			if (columns != NULL) {
				for (int col = 0; col < columns->length(); col++) {
					ColumnData * column = NULL;
					columns->getPointer(&column, col);
					if (column->cells.length() != 0) {
						column->cells.remove((int)wparam);
					}
				}
			}
			break;
		}

		case WM_NCDESTROY:
		case LB_RESETCONTENT: {
			ArrayList<int> * row_colors = data->ListBox.rowColors;
			if (row_colors != NULL) {
				delete row_colors;
			}
			ArrayList<int> * sel_states = data->ListBox.selStates;
			if (sel_states != NULL) {
				delete sel_states;
			}
			data->ListBox.rowColors = 0;
			data->ListBox.selStates = 0;
			data->ListBox.topIndex = 0;
			data->ListBox.curSel = -1;

			ArrayList<ColumnData> * columns = data->ListBox.columns;
			if (columns != NULL) {
				for (int col = 0; col < columns->length(); col++) {
					ColumnData * column = NULL;
					columns->getPointer(&column, col);
					column->cells.clear();
				}
			}

			if (message == WM_NCDESTROY) {
				if (columns != NULL) {
					delete columns;
				}
				data->ListBox.columns = NULL;
			} else {
				parent = GetParent(window);
				SendMessage(parent, WM_COMMAND, (GetWindowLong(window, GWL_ID) & 0xFFFF) | 0x10000, (LPARAM)window);
			}
			break;
		}

		case LB_GETTOPINDEX:
			return(data->ListBox.topIndex);

		case LB_SETTOPINDEX: {
			int index = (int)wparam;
			int count = SendMessage(window, LB_GETCOUNT, 0, 0);
			int item_height = SendMessage(window, LB_GETITEMHEIGHT, 0, 0);
			if (!count || !item_height) {
				break;
			}
			int visible = (client_rect.bottom - client_rect.top) / item_height;
			if (index < 0) {
				index = 0;
			}
			if (count - visible <= 0) {
				index = 0;
			} else if (index > count - visible) {
				index = count - visible;
			}
			if (index != data->ListBox.topIndex) {
				data->ListBox.topIndex = index;
				InvalidateRect(window, NULL, FALSE);
			}
			return(0);
		}

		case LB_SELITEMRANGE: {
			int last = HIWORD(lparam);
			int first = LOWORD(lparam);
			int count = SendMessage(window, LB_GETCOUNT, 0, 0);
			if (last < 0 || last < first) {
				return(-1);
			}
			if (last >= count) {
				last = count - 1;
			}
			ArrayList<int> * sel_states = data->ListBox.selStates;
			if (sel_states == NULL) {
				sel_states = new ArrayList<int>;
				data->ListBox.selStates = sel_states;
			}
			if (last >= sel_states->length()) {
				int unselected = 0;
				sel_states->setSize(last + 1, unselected);
			}
			for (int i = first; i <= last; i++) {
				int selected = (int)wparam;
				sel_states->replace(selected, i);
			}
			parent = GetParent(window);
			SendMessage(parent, WM_COMMAND, (GetWindowLong(window, GWL_ID) & 0xFFFF) | 0x10000, (LPARAM)window);
			return(0);
		}

		case LB_GETSELCOUNT: {
			ArrayList<int> * sel_states = data->ListBox.selStates;
			if (sel_states == NULL) {
				return(0);
			}
			int count = sel_states->length();
			int selected = 0;
			int total = 0;
			for (int i = 0; i < count; i++) {
				int * value = NULL;
				if (sel_states->getPointer(&value, i)) {
					selected = *value;
				}
				if (selected) {
					total++;
				}
			}
			return(total);
		}

		case LB_GETSELITEMS: {
			ArrayList<int> * sel_states = data->ListBox.selStates;
			int total = 0;
			if (sel_states != NULL) {
				int selected = 0;
				if (sel_states->length() > 0) {
					int * out = (int *)lparam;
					int max = (int)wparam;
					for (int i = 0; i < sel_states->length(); i++) {
						int * value = NULL;
						if (sel_states->getPointer(&value, i)) {
							selected = *value;
						}
						if (selected) {
							*out = i;
							total++;
							out++;
						}
						if (total >= max) {
							break;
						}
					}
				}
			}
			return(total);
		}

		case LB_GETITEMRECT: {
			int index = (int)wparam;
			if (index < data->ListBox.topIndex) {
				return(-1);
			}
			if (index >= SendMessage(window, LB_GETCOUNT, 0, 0)) {
				return(-1);
			}
			int item_height = SendMessage(window, LB_GETITEMHEIGHT, 0, 0);
			int relative = index - data->ListBox.topIndex;
			if (relative > (client_rect.bottom - client_rect.top) / item_height) {
				return(-1);
			}
			RECT * out = (RECT *)lparam;
			out->top = relative * item_height;
			out->bottom = relative * item_height + item_height;
			out->left = client_rect.left;
			out->right = client_rect.right - client_rect.left;
			return(0);
		}

		case WM_LBUTTONDOWN: {
			int top = SendMessage(window, LB_GETTOPINDEX, 0, 0);
			int item_height = SendMessage(window, LB_GETITEMHEIGHT, 0, 0);
			int index = top + (int)HIWORD(lparam) / item_height;
			LONG style = GetWindowLong(window, GWL_STYLE);
			SetFocus(window);
			int paint_disabled = SendMessage(window, OD_DISABLEPAINT, 0, 1);
			if ((style & LBS_MULTIPLESEL) != 0) {
				int select = (SendMessage(window, LB_GETSEL, index, 0) == 0);
				Sound_Effect(Rule->GenericClick);
				SendMessage(window, LB_SETSEL, select, index);
				InvalidateRect(window, NULL, FALSE);
			} else if ((style & LBS_NOSEL) == 0) {
				Sound_Effect(Rule->GenericClick);
				SendMessage(window, LB_SETCURSEL, index, 0);
				InvalidateRect(window, NULL, FALSE);
			}
			SendMessage(window, OD_DISABLEPAINT, 0, paint_disabled);
			parent = GetParent(window);
			SendMessage(parent, WM_COMMAND, (GetWindowLong(window, GWL_ID) & 0xFFFF) | 0x10000, (LPARAM)window);
			return(0);
		}

		case WM_LBUTTONDBLCLK:
			PostMessage(parent, WM_COMMAND, (GetWindowLong(window, GWL_ID) & 0xFFFF) | 0x20000, (LPARAM)window);
			return(0);

		case WM_RBUTTONDOWN:
			SendMessage(window, LB_SETSEL, 0, -1);
			SendMessage(window, LB_SETCURSEL, (WPARAM)-1, 0);
			parent = GetParent(window);
			SendMessage(parent, WM_COMMAND, (GetWindowLong(window, GWL_ID) & 0xFFFF) | 0x10000, (LPARAM)window);
			InvalidateRect(window, NULL, FALSE);
			return(0);

		case OD_GETCELLTIP: {
			int count = SendMessage(window, LB_GETCOUNT, 0, 0);
			int item_height = SendMessage(window, LB_GETITEMHEIGHT, 0, 0);
			ArrayList<ColumnData> * columns = data->ListBox.columns;
			if (columns != NULL) {
				int row = data->ListBox.topIndex + (int)HIWORD(wparam) / item_height;
				int best_column = -1;
				int best_x = 0;
				ColumnData * column = NULL;
				for (int col = 0; col < columns->length(); col++) {
					columns->getPointer(&column, col);
					if (column->xPos <= (int)LOWORD(wparam)) {
						if (column->xPos > best_x) {
							best_column = col;
							best_x = column->xPos;
						}
					}
				}
				if (best_column != -1 && row >= 0 && row < count) {
					column = NULL;
					columns->getPointer(&column, best_column);
					if (column != NULL && row < column->cells.length()) {
						CellData * cell = NULL;
						column->cells.getPointer(&cell, row);
						if (cell != NULL && lparam != 0) {
							strcpy((char *)lparam, cell->hint.get());
							return(strlen(cell->hint.get()) == 0);
						}
					}
				}
			}
			return(0);
		}

		case OD_ADDCOLUMN: {
			ArrayList<ColumnData> * columns = data->ListBox.columns;
			if (columns == NULL) {
				columns = new ArrayList<ColumnData>;
				data->ListBox.columns = columns;
			}
			ColumnData * column = NULL;
			int col = 0;
			while (col < columns->length()) {
				columns->getPointer(&column, col);
				if (column->xPos == lparam) {
					return(lparam);
				}
				col++;
			}
			ColumnData new_column;
			new_column.xPos = lparam;
			new_column.width = wparam;
			int length = columns->length();
			columns->add(new_column, length);
			return(lparam);
		}

		case OD_REMOVECOLUMN: {
			ArrayList<ColumnData> * columns = data->ListBox.columns;
			if (columns != NULL) {
				ColumnData * column = NULL;
				for (int col = 0; col < columns->length(); col++) {
					columns->getPointer(&column, col);
					if (column->xPos == lparam) {
						columns->remove(col);
						return(lparam);
					}
				}
			}
			return(-1);
		}

		case OD_SETCELL: {
			ArrayList<ColumnData> * columns = data->ListBox.columns;
			if (columns == NULL) {
				return(-1);
			}
			int column_id = LOWORD(wparam);
			int row = HIWORD(wparam);
			ColumnData * column = NULL;
			int found = -1;
			for (int col = 0; col < columns->length(); col++) {
				columns->getPointer(&column, col);
				if (column->xPos == column_id) {
					found = col;
					break;
				}
			}
			if (found == -1) {
				return(-1);
			}
			if (row < 0 || row >= SendMessage(window, LB_GETCOUNT, 0, 0)) {
				return(-1);
			}
			CellData filler;
			if (found == 0) {
				filler.type = CellData::PRIMARY;
			}
			if (row >= column->cells.length()) {
				column->cells.setSize(row + 1, filler);
			}
			column->cells.replace(*(CellData *)lparam, row);
			return(column_id);
		}

		case OD_SETCOLOR: {
			ArrayList<int> * row_colors = data->ListBox.rowColors;
			if (row_colors == NULL) {
				row_colors = new ArrayList<int>;
				data->ListBox.rowColors = row_colors;
			}
			int index = (int)wparam;
			if (index >= row_colors->length()) {
				int blank = -1;
				row_colors->setSize(index + 1, blank);
			}
			int color = (int)lparam;
			row_colors->replace(color, index);
			InvalidateRect(window, NULL, FALSE);
			break;
		}

		case OD_SUBCLASSED:
			data->ListBox.curSel = -1;
			data->font = ODListFontPtr;
			SendMessage(window, LB_SETITEMHEIGHT, (WPARAM)-1, (unsigned short)(ODListFontSize + 2));
			break;
	}

	/*
	 * Create or destroy the attached scrollbar based on whether the listbox needs one,
	 * then forward the message to the original Win32 listbox procedure.
	 */
	if (needs_scrollbar == 1) {
		if (data->attachedWindow == NULL) {
			data->attachedWindow = (HWND)1;
			GetWindowLong(window, GWL_WNDPROC);
			parent = GetParent(window);
			RECT parent_display;
			Get_Display_Rect(parent, &parent_display);
			RECT win_display;
			Get_Display_Rect(window, &win_display);
			int x = win_display.left - parent_display.left;
			int y = win_display.top - parent_display.top;
			data->attachedWindow = CreateWindowEx(0, "Scrollbar", NULL, 0x50010001,
				win_display.left - parent_display.left - scrollbar_width + client_rect.right + 1,
				client_rect.top + win_display.top - parent_display.top,
				scrollbar_width, win_display.bottom - win_display.top,
				parent, NULL, ProgramInstance, NULL);
			data->scrollBarWidth = scrollbar_width;
			InitializeCtrl(data->attachedWindow, 0);

			WinData * sb_data = NULL;
			ODWinData.getPointer(data->attachedWindow, &sb_data);
			sb_data->ownerWindow = window;

			SCROLLINFO info;
			info.nMax = max_position;
			info.fMask = SIF_RANGE | SIF_POS;
			info.nMin = 0;
			info.nPos = data->ListBox.topIndex;
			info.cbSize = sizeof(SCROLLINFO);
			SendMessage(data->attachedWindow, SBM_SETSCROLLINFO, 0, (LPARAM)&info);

			SetWindowPos(window, NULL, 0, 0, win_display.right - win_display.left - scrollbar_width, win_display.bottom - win_display.top, SWP_NOMOVE);
			ShowWindow(data->attachedWindow, SW_SHOW);
			BringWindowToTop(data->attachedWindow);
			InvalidateRect(data->attachedWindow, NULL, FALSE);
			UpdateWindow(data->attachedWindow);

			Rect validate_rect;
			validate_rect.X = x;
			validate_rect.Y = y;
			validate_rect.Width = x + client_rect.right + 1;
			validate_rect.Height = client_rect.bottom + y + 1;
			ValidateRect(parent, (const RECT *)&validate_rect);
		}
	} else if (needs_scrollbar == 0 && data->attachedWindow != NULL && !data->paintDisabled) {
		DestroyWindow(data->attachedWindow);
		HWND scrollbar = data->attachedWindow;
		ODRemoveFromDict(scrollbar, 0);
		data->attachedWindow = NULL;
		data->scrollBarWidth = 0;
		SetWindowPos(window, NULL, 0, 0, client_rect.right + ODBorderThickness - client_rect.left + scrollbar_width + 1, client_rect.bottom + 2 * ODBorderThickness - client_rect.top, SWP_NOMOVE);
		parent = GetParent(window);
		RECT parent_display;
		Get_Display_Rect(parent, &parent_display);
		Rect validate_rect;
		validate_rect.X = display_rect.left - parent_display.left - 1;
		validate_rect.Y = display_rect.top - parent_display.top - 1;
		validate_rect.Width = scrollbar_width + client_rect.right + display_rect.left - parent_display.left;
		validate_rect.Height = display_rect.top - parent_display.top + client_rect.bottom + 1;
		ValidateRect(parent, (const RECT *)&validate_rect);
	}

	WNDPROC original_proc = NULL;
	OriginalWndProcs.getValue(window, original_proc);
	if (call_default) {
		result = CallWindowProc(original_proc, window, message, wparam, lparam);
	}

	if (message == WM_NCDESTROY) {
		ArrayList<int> * row_colors = data->ListBox.rowColors;
		if (row_colors != NULL) {
			delete row_colors;
		}
		ArrayList<int> * sel_states = data->ListBox.selStates;
		if (sel_states != NULL) {
			delete sel_states;
		}
	}

	return(result);
}


/// <summary>
/// Handles the messages for an owner-drawn scroll bar.
/// The grip is sized against the scroll range and dragged directly with the mouse, while
/// the arrow buttons repeat on a timer for as long as they are held. The owner is told of
/// the new position as it changes, which is what lets a list box scroll under the mouse.
/// </summary>
/// <returns>Returns with the result of the original window procedure for anything this
/// routine does not handle itself.</returns>
LRESULT CALLBACK ScrollBarCtrlProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	RECT client_rect;
	RECT display_rect;
	GetClientRect(window, &client_rect);
	Get_Display_Rect(window, &display_rect);

	client_rect.right -= 2 * ODBorderThickness;
	client_rect.bottom -= 2 * ODBorderThickness;

	display_rect.left += ODBorderThickness;
	display_rect.right -= ODBorderThickness;
	display_rect.top += ODBorderThickness;
	display_rect.bottom -= ODBorderThickness;

	WinData * data = NULL;
	ODWinData.getPointer(window, &data);

	bool keep_parent_capture = false;
	if (data->ScrollBar.keepCapture) {
		keep_parent_capture = true;
	}
	int dragging = data->ScrollBar.dragging;
	int range = data->ScrollBar.range;
	int message_result = data->ScrollBar.result;
	int position = data->ScrollBar.position;
	int up_pressed = data->ScrollBar.upPressed;
	int down_pressed = data->ScrollBar.downPressed;

	if (!range) {
		range = 100;
	}

	int scroll_code = 0;
	int grip_top = 0;
	int grip_bottom = 0;

	int width = client_rect.right - client_rect.left;
	int travel_height = client_rect.bottom - client_rect.top - 44;
	int grip_height = (int)((double)travel_height - log((double)(range + 1)) * (double)travel_height * 0.2);
	if (grip_height <= 14) {
		grip_height = 14;
	}

	int travel = travel_height - grip_height;
	if (travel <= 1) {
		travel = 1;
	}

	if (message < TBM_GETPOS || message == OD_REFRESHNOPAINT) {
		if (!dragging) {
			grip_top = position * travel / range + client_rect.top + 22;
			grip_bottom = grip_top + grip_height;
		} else {
			POINT cursor;
			Get_Logical_Cursor_Pos(window, cursor);

			grip_top = cursor.y - grip_height / 2;
			if (grip_top < 22) {
				grip_top = 22;
			}

			if (client_rect.bottom - grip_height - 22 < grip_top) {
				grip_top = client_rect.bottom - grip_height - 22;
			}

			grip_bottom = grip_top + grip_height;
			scroll_code = SB_THUMBTRACK;
			position = range * (grip_top - 22) / travel;
		}
	}

	switch (message) {
		case SBM_GETPOS:
			return(position);

		case SBM_SETPOS:
			if ((int)wparam <= range && (int)wparam > 0) {
				position = (int)wparam;
			}
			break;

		case SBM_SETRANGE:
			range = (int)lparam;
			if (position > range) {
				position = range;
			}
			break;

		case SBM_SETSCROLLINFO: {
			SCROLLINFO * info = (SCROLLINFO *)lparam;
			range = info->nMax;
			position = info->nPos;
			break;
		}

		case WM_NCHITTEST:
		case WM_GETDLGCODE: {
			WNDPROC proc = NULL;
			OriginalWndProcs.getValue(window, proc);
			return(CallWindowProc(proc, window, message, wparam, lparam));
		}

		case WM_ERASEBKGND:
			return(0);

		case WM_PAINT: {
			if (data->paintDisabled) {
				return(0);
			}

			/*
			 * The source rect and the blit dest rect get reused for every blit
			 * below; only the Draw and edge glow calls get their own rects.
			 */
			Rect src_rect;
			Rect full_rect;
			full_rect.X = display_rect.left;
			full_rect.Y = display_rect.top;
			full_rect.Width = client_rect.right;
			full_rect.Height = client_rect.bottom;
			src_rect.X = 0;
			src_rect.Y = 0;
			src_rect.Width = client_rect.right;
			src_rect.Height = client_rect.bottom;

			HWND parent = GetParent(window);
			WinData * parent_data = NULL;
			if (parent != NULL) {
				ODWinData.getPointer(parent, &parent_data);
			}

			RECT parent_display;
			Get_Display_Rect(parent, &parent_display);

			Rect source_rect = src_rect;
			if (parent_data != NULL && parent_data->cachedSurface != NULL) {
				source_rect.X = display_rect.left + source_rect.X - parent_display.left;
				source_rect.Y += display_rect.top - parent_display.top;
			}

			if (data->cachedSurface != NULL) {
				if (data->cachedSurface->Get_Width() != client_rect.right || data->cachedSurface->Get_Height() != client_rect.bottom) {
					delete data->cachedSurface;
					data->cachedSurface = NULL;
				}
			}

			if (data->cachedSurface == NULL) {
				BSurface * background = new BSurface(client_rect.right, client_rect.bottom, 2);
				data->cachedSurface = background;
				++_surface_count;

				if (parent_data != NULL && parent_data->cachedSurface != NULL) {
					background->Blit_From(src_rect, *parent_data->cachedSurface, source_rect);
				}

				int pixel_count = client_rect.bottom * client_rect.right;
				unsigned short * pixels = (unsigned short *)background->Lock();
				unsigned short * ptr = pixels;
				for (int i = pixel_count; i > 0; --i) {
					*ptr = OD_Blend_Color(*ptr, 0xFFFF, ODColorSteps);
					ptr++;
				}
				if (pixels != NULL) {
					background->Unlock();
				}
			}

			if (parent_data != NULL && parent_data->cachedSurface != NULL) {
				AlternateSurface->Blit_From(full_rect, *parent_data->cachedSurface, source_rect);
			}

			OD_Draw_Rect(*AlternateSurface, full_rect, ODBorderThickness, 0xFFFFFFFF);

			Rect grip_rect;
			grip_rect.X = display_rect.left;
			full_rect.X = display_rect.left;
			grip_rect.Y = grip_top + display_rect.top;
			grip_rect.Width = client_rect.right;
			grip_rect.Height = grip_bottom - grip_top;
			full_rect.Y = grip_top + display_rect.top;
			full_rect.Width = client_rect.right;
			full_rect.Height = grip_bottom - grip_top;
			src_rect.Width = client_rect.right;
			src_rect.Height = grip_bottom - grip_top;
			src_rect.X = 0;
			src_rect.Y = grip_top;

			Surface * grip_center = SurfaceCache.GetSurface("sbgripm.pcx", NULL);
			if (grip_center != NULL) {
				grip_rect.Width = grip_center->Get_Width();
			}
			SurfaceCache.Draw(grip_rect, *AlternateSurface, *grip_center, 0, 0);

			Rect grip_src(0, 0, grip_center->Get_Width(), grip_center->Get_Height());

			Surface * grip_top_surf = SurfaceCache.GetSurface("sbgript.pcx", NULL);
			if (grip_top_surf != NULL) {
				full_rect.Height = grip_top_surf->Get_Height();
			}
			AlternateSurface->Blit_From(full_rect, *grip_top_surf, grip_src);

			Surface * grip_bottom_surf = SurfaceCache.GetSurface("sbgripb.pcx", NULL);
			if (grip_bottom_surf != NULL) {
				full_rect.Y = display_rect.top + grip_bottom - grip_bottom_surf->Get_Height();
			}
			AlternateSurface->Blit_From(full_rect, *grip_bottom_surf, grip_src);

			Rect up_rect;
			up_rect.X = display_rect.left;
			full_rect.X = display_rect.left;
			src_rect.X = display_rect.left;
			up_rect.Y = display_rect.top;
			full_rect.Y = display_rect.top;
			up_rect.Width = client_rect.right;
			up_rect.Height = 22;
			full_rect.Width = client_rect.right;
			full_rect.Height = 22;
			src_rect.Width = client_rect.right;
			src_rect.Height = 22;
			src_rect.X = 0;
			src_rect.Y = 0;
			AlternateSurface->Blit_From(full_rect, *data->cachedSurface, src_rect);
			ODDrawEdgeGlows(*AlternateSurface, up_rect, up_pressed == 0, 2, ODScrollBarAdj, ODScrollBarAdj, ODScrollBarAdj, ODScrollBarAdj);
			ODDrawArrowBitmap(*AlternateSurface, up_rect, 1, up_pressed);

			Rect down_rect;
			down_rect.X = display_rect.left;
			full_rect.X = display_rect.left;
			down_rect.Y = display_rect.bottom - 22;
			full_rect.Y = display_rect.bottom - 22;
			down_rect.Width = client_rect.right;
			down_rect.Height = 22;
			full_rect.Width = client_rect.right;
			full_rect.Height = 22;
			src_rect.Width = client_rect.right;
			src_rect.Height = 22;
			src_rect.X = 0;
			src_rect.Y = client_rect.bottom - 22;
			AlternateSurface->Blit_From(full_rect, *data->cachedSurface, src_rect);
			ODDrawEdgeGlows(*AlternateSurface, down_rect, down_pressed == 0, 2, ODScrollBarAdj, ODScrollBarAdj, ODScrollBarAdj, ODScrollBarAdj);
			ODDrawArrowBitmap(*AlternateSurface, down_rect, 0, down_pressed);

			ValidateRect(window, NULL);
			break;
		}

		case WM_TIMER: {
			POINT cursor;
			Get_Logical_Cursor_Pos(window, cursor);

			up_pressed = 0;
			down_pressed = 0;

			if (message_result && cursor.x > client_rect.right - width) {
				if (cursor.y < 22) {
					if (!dragging) {
						up_pressed = 1;
					}
					if (position != 0) {
						scroll_code = SB_LINEUP;
						--position;
					}
				} else if (cursor.y > client_rect.bottom - 22) {
					if (!dragging) {
						down_pressed = 1;
					}
					if (position + 1 <= range) {
						scroll_code = SB_LINEDOWN;
						++position;
					}
				}
			}

			SetTimer(window, 0, 0x19, NULL);
			break;
		}

		case WM_MOUSEMOVE: {
			if (dragging) {
				RECT rect;
				rect.left = client_rect.right - width;
				rect.top = client_rect.top;
				rect.right = client_rect.right;
				rect.bottom = client_rect.bottom;
				InvalidateRect(window, &rect, FALSE);
			}

			if (wparam & MK_LBUTTON) {
				break;
			}
		}

		case WM_LBUTTONUP:
			message_result = 0;
			dragging = 0;
			if (up_pressed || down_pressed) {
				InvalidateRect(window, NULL, FALSE);
			}
			up_pressed = 0;
			down_pressed = 0;
			KillTimer(window, 0);
			ReleaseCapture();
			if (keep_parent_capture) {
				SetCapture(data->ownerWindow);
			}
			scroll_code = SB_ENDSCROLL;
			break;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONDBLCLK: {
			if (message == WM_LBUTTONDOWN) {
				message_result = 1;
				SetCapture(window);
				SetTimer(window, 0, 0x1F4, NULL);
			} else {
				message_result = 0;
				dragging = 0;
				KillTimer(window, 0);
				ReleaseCapture();
				if (keep_parent_capture) {
					SetCapture(data->ownerWindow);
				}
			}

			int xpos = (unsigned short)LOWORD(lparam);
			int ypos = (unsigned short)HIWORD(lparam);
			int repeat = (message == WM_LBUTTONDBLCLK) ? 2 : 1;

			up_pressed = 0;
			down_pressed = 0;

			while (repeat > 0) {
				if (xpos > (client_rect.right - width)) {
					if (ypos < 22 && position) {
						up_pressed = 1;
						scroll_code = SB_LINEUP;
						--position;
					} else {
						bool skip_thumb_logic = false;
						if (ypos > client_rect.bottom - 22) {
							if (position + 1 <= range) {
								down_pressed = 1;
								scroll_code = SB_LINEDOWN;
								++position;
								skip_thumb_logic = true;
							}
						}

						if (!skip_thumb_logic) {
							if (ypos < grip_top || ypos >= grip_bottom) {
								grip_top = ypos - grip_height / 2;
								if (grip_top < 22) {
									grip_top = 22;
								}

								int max_top = client_rect.bottom - grip_height - 22;
								if (max_top < grip_top) {
									grip_top = max_top;
								}

								grip_bottom = grip_top + grip_height;
								scroll_code = SB_THUMBTRACK;
								position = range * (grip_top - 22) / travel;
							} else if (message == WM_LBUTTONDOWN) {
								dragging = 1;
							}
						}
					}
				}
				--repeat;
			}
			break;
		}

		case OD_SETKEEPCAPTURE:
			if (lparam != 0) {
				if (data != NULL) {
					data->ScrollBar.keepCapture = 1;
				}
			} else {
				if (data != NULL) {
					data->ScrollBar.keepCapture = 0;
				}
			}
			break;
	}

	int send_notify = 0;
	if (position != data->ScrollBar.position || range != data->ScrollBar.range) {
		if (data->ownerWindow != NULL) {
			send_notify = 1;
		}
	}

	data->ScrollBar.position = position;
	data->ScrollBar.result = message_result;
	data->ScrollBar.dragging = dragging;
	data->ScrollBar.range = range;
	data->ScrollBar.upPressed = up_pressed;
	data->ScrollBar.downPressed = down_pressed;

	if (send_notify) {
		SendMessage(data->ownerWindow, WM_VSCROLL, MAKEWPARAM(scroll_code, (unsigned short)position), (LPARAM)window);
		InvalidateRect(window, NULL, FALSE);
	}

	return(0);
}


/// <summary>
/// Handles the messages for an owner-drawn progress bar.
/// The bar is drawn as a gradient over a cached copy of the dialog background, scaled
/// against the range the dialog set. A position outside that range is clamped rather than
/// refused.
/// </summary>
/// <returns>Returns with zero; nothing this control is sent needs to reach the original
/// window procedure.</returns>
LRESULT CALLBACK ProgressBarCtrlProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	RECT rect;
	Get_Display_Rect(window, &rect);

	OwnerDraw::WinData * data = NULL;

	ODWinData.getPointer(window, &data);

	switch (message) {
		case OD_SUBCLASSED: {
			data->ProgressBar.maximum = 100;
			break;
		}

		case PBM_SETRANGE: {
			data->ProgressBar.minimum = LOWORD(lparam);
			data->ProgressBar.maximum = HIWORD(lparam);
			break;
		}

		case PBM_SETPOS: {
			int pos = (int)wparam;
			if (pos < data->ProgressBar.minimum) {
				pos = data->ProgressBar.minimum;
			}
			if (pos > data->ProgressBar.maximum) {
				pos = data->ProgressBar.maximum;
			}
			data->ProgressBar.position = pos;

			InvalidateRect(window, NULL, FALSE);
			break;
		}

		case WM_PAINT: {
			Rect sourcerect(0, 0, rect.right - rect.left + 1, rect.bottom - rect.top + 1);
			Rect destrect(rect.left, rect.top, rect.right - rect.left + 1, rect.bottom - rect.top + 1);

			if (data->cachedSurface == NULL) {
				Surface * surf = new BSurface(rect.right - rect.left + 1, rect.bottom - rect.top + 1, 2);
				data->cachedSurface = surf;
				_surface_count++;
				surf->Blit_From(sourcerect, *AlternateSurface, destrect);
			}

			AlternateSurface->Blit_From(destrect, *data->cachedSurface, sourcerect);
			int pos = (data->ProgressBar.position * 65536) / (data->ProgressBar.maximum - data->ProgressBar.minimum);
			int color = ODColorToHiColor(0x000000FF);
			ODDrawGradientRect(destrect, *AlternateSurface, color, pos);
			ValidateRect(window, NULL);
			break;
		}
	}

	return(0);
}


/// <summary>
/// Handles the messages for an owner-drawn track bar.
/// The grip is dragged directly with the mouse and snapped to whatever step the dialog
/// asked for, with the current value optionally printed alongside the track. The parent
/// dialog is notified as the value changes, not merely when the drag ends.
/// </summary>
/// <returns>Returns with the result of the original window procedure for anything this
/// routine does not handle itself.</returns>
LRESULT CALLBACK TrackBarCtrlProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	bool play_click = true;

	RECT client_rect;
	RECT display_rect;
	GetClientRect(window, &client_rect);
	Get_Display_Rect(window, &display_rect);
	int number_width = 50;

	WinData * data = NULL;
	ODWinData.getPointer(window, &data);

	int message_result = data->TrackBar.result;
	int dragging = data->TrackBar.dragging;
	int range = data->TrackBar.range;
	int value = data->TrackBar.value;
	int minimum = data->TrackBar.minimum;
	int maximum;
	int thumb_pos = data->TrackBar.thumbPos;
	int step = data->TrackBar.step;
	int show_numbers = data->TrackBar.showNumbers;

	if (!show_numbers) {
		number_width = 0;
	}

	int slider_width = client_rect.right - client_rect.left - number_width - 13;
	if (slider_width <= 1) {
		slider_width = 1;
	}

	if (!range) {
		WNDPROC proc = NULL;
		OriginalWndProcs.getValue(window, proc);

		int range_max = CallWindowProc(proc, window, TBM_GETRANGEMAX, 0, 0);
		minimum = CallWindowProc(proc, window, TBM_GETRANGEMIN, 0, 0);
		range = range_max - minimum;
		value = CallWindowProc(proc, window, TBM_GETPOS, 0, 0) - minimum;
		thumb_pos = value * slider_width / range;
		if (!range) {
			range = 100;
		}

		data->TrackBar.thumbPos = thumb_pos;
		data->TrackBar.range = range;
		data->TrackBar.value = value;
		data->TrackBar.minimum = minimum;
		data->TrackBar.step = step;
		data->TrackBar.showNumbers = show_numbers;
	}

	if (!step) {
		number_width = 50;
		step = 1;
		show_numbers = 1;
	}

	LONG grip_left;
	LONG grip_right;
	if (!dragging) {
		grip_left = client_rect.left + thumb_pos + 1;
		grip_right = grip_left + 12;
	} else {
		POINT point;
		Get_Logical_Cursor_Pos(window, point);

		int xpos = point.x - 6;
		if (xpos < 1) {
			xpos = 1;
		}

		int max_x = client_rect.right - number_width - 12;
		if (max_x < xpos) {
			xpos = max_x;
		}

		int idx = ((range + 1) * (xpos - 1)) / slider_width;
		if (idx >= range) {
			idx = range;
		}

		value = step * ((minimum + idx) / step) - minimum;
		thumb_pos = value * slider_width / range;
		grip_left = thumb_pos + 1;
		grip_right = grip_left + 12;
	}

	switch (message) {
		case WM_NCHITTEST:
		case WM_GETDLGCODE: {
			WNDPROC proc = NULL;
			OriginalWndProcs.getValue(window, proc);
			return(CallWindowProc(proc, window, message, wparam, lparam));
		}

		case WM_ENABLE:
			InvalidateRect(window, NULL, FALSE);
			break;

		case WM_PAINT: {
			Get_Display_Rect(window, &display_rect);
			Rect disp_rect(display_rect.left, display_rect.top, display_rect.right - display_rect.left, display_rect.bottom - display_rect.top);
			LONG style = GetWindowLong(window, GWL_STYLE);

			if (data->cachedSurface == NULL) {
				RECT disp_copy;
				RECT client_copy;
				Get_Display_Rect(window, &disp_copy);
				GetClientRect(window, &client_copy);

				BSurface * surf = new BSurface(client_copy.right + 1, client_copy.bottom + 1, 2);
				data->cachedSurface = surf;
				_surface_count++;

				Rect src_rect(0, 0, client_copy.right + 1, client_copy.bottom + 1);
				Rect dst_rect(disp_copy.left, disp_copy.top, client_copy.right + 1, client_copy.bottom + 1);
				surf->Blit_From(src_rect, *AlternateSurface, dst_rect);
				ODFillRectTrans(src_rect, *surf, 0, 180);
			}

			if (data->cachedSurface != NULL) {
				RECT disp_copy;
				RECT client_copy;
				Get_Display_Rect(window, &disp_copy);
				GetClientRect(window, &client_copy);

				Rect src_rect(0, 0, client_copy.right + 1, client_copy.bottom + 1);
				Rect dst_rect(disp_copy.left, disp_copy.top, client_copy.right + 1, client_copy.bottom + 1);
				AlternateSurface->Blit_From(dst_rect, *data->cachedSurface, src_rect);
			}

			if (show_numbers) {
				Surface * center = SurfaceCache.GetSurface("trofm.pcx", NULL);
				Rect center_rect;
				center_rect.Height = center->Get_Height();
				center_rect.Width = number_width;
				center_rect.X = disp_rect.X + disp_rect.Width - number_width;
				center_rect.Y = disp_rect.Y;
				SurfaceCache.Draw(center_rect, *AlternateSurface, *center, 0, 0);

				Surface * left = SurfaceCache.GetSurface("trofl.pcx", NULL);
				Rect left_src(0, 0, left->Get_Width(), left->Get_Height());
				Rect left_dst(disp_rect.X + disp_rect.Width - number_width, disp_rect.Y, left_src.Width, left_src.Height);
				AlternateSurface->Blit_From(left_dst, *left, left_src);

				Surface * right = SurfaceCache.GetSurface("trofr.pcx", NULL);
				Rect right_src(0, 0, right->Get_Width(), right->Get_Height());
				Rect right_dst(disp_rect.X + disp_rect.Width - right_src.Width, disp_rect.Y, right_src.Width, right_src.Height);
				AlternateSurface->Blit_From(right_dst, *right, right_src);
			}

			Rect grip_rect(grip_left + display_rect.left, display_rect.top, grip_right - grip_left, display_rect.bottom - display_rect.top);
			Surface * grip = SurfaceCache.GetSurface("trakgrip.pcx", NULL);
			Rect grip_src(0, 0, grip->Get_Width(), grip->Get_Height());
			AlternateSurface->Blit_From(grip_rect, *grip, grip_src);

			int frame_color;
			if (ODColorFrame == -1) {
				frame_color = -1;
			} else {
				frame_color = ODColorToHiColor(ODColorFrame);
			}

			if (style & WS_DISABLED) {
				frame_color = ODColorDisabled;
				if (ODColorDisabled != -1) {
					frame_color = ODColorToHiColor(ODColorDisabled);
				}
			}

			Rect frame_rect(disp_rect.X, disp_rect.Y, disp_rect.Width - number_width, disp_rect.Height);
			OD_Draw_Rect(*AlternateSurface, frame_rect, 1, frame_color);

			if (style & WS_DISABLED) {
				ODFillRectTrans(disp_rect, *AlternateSurface, 0, 128);
			}

			if (show_numbers) {
				char buffer[16];
				sprintf(buffer, "%d", step * ((value + minimum) / step));

				COLORREF text_color = ODColorText;
				if (style & WS_DISABLED) {
					text_color = ODColorDisabled;
				}

				RECT text_rect;
				text_rect.left = display_rect.right - 49;
				text_rect.top = display_rect.top;
				text_rect.right = display_rect.right;
				text_rect.bottom = display_rect.bottom;

				OD_Draw_Text_Remap(*AlternateSurface, buffer, *(Rect *)&text_rect, "dlgsys", text_color, 5, 0);
			}

			ValidateRect(window, NULL);
			break;
		}

		case WM_ERASEBKGND:
			return(0);

		case WM_MOUSEMOVE:
			if (dragging) {
				RECT rect = client_rect;
				InvalidateRect(window, &rect, FALSE);
			}
			if (wparam & MK_LBUTTON) {
				break;
			}

		case WM_LBUTTONUP:
			message_result = 0;
			dragging = 0;
			ReleaseCapture();
			break;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONDBLCLK: {
			if (message == WM_LBUTTONDOWN) {
				message_result = 1;
				SetCapture(window);
			} else {
				message_result = 0;
				ReleaseCapture();
			}

			int xpos = (unsigned short)LOWORD(lparam);
			int ypos = (unsigned short)HIWORD(lparam);
			if (ypos > client_rect.bottom - 18) {
				if (xpos >= grip_left && xpos < grip_right) {
					if (message == WM_LBUTTONDOWN) {
						dragging = 1;
					}
				} else {
					int x = xpos - 6;
					if (x < 1) {
						x = 1;
					}

					int max_x = client_rect.right - number_width - 12;
					if (max_x < x) {
						x = max_x;
					}

					int idx = ((range + 1) * (x - 1)) / slider_width;
					if (idx >= range) {
						idx = range;
					}

					value = step * ((minimum + idx) / step) - minimum;
					thumb_pos = value * slider_width / range;
				}
			}
			break;
		}

		case TBM_GETPOS:
			return(step * ((minimum + value) / step));

		case TBM_SETPOS:
			if (lparam - minimum <= range && lparam - minimum >= 0) {
				value = lparam - minimum;
			}
			play_click = false;
			thumb_pos = value * slider_width / range;
			break;

		case TBM_SETRANGE:
			minimum = (unsigned short)LOWORD(lparam);
			maximum = (unsigned short)HIWORD(lparam);
			range = maximum - minimum;
			if (value > range) {
				value = range;
			}
			if (value < minimum) {
				value = minimum;
			}
			play_click = false;
			thumb_pos = value * slider_width / range;
			break;

		case OD_SETTRACKSTEP:
			step = lparam;
			break;

		case OD_TRACKNUMBERS:
			show_numbers = lparam;
			break;

		case OD_TRACKSILENT:
			data->TrackBar.clickSuppress = (wparam == 0);
			break;
	}

	int changed = 0;
	if (value != data->TrackBar.value || range != data->TrackBar.range || minimum != data->TrackBar.minimum) {
		changed = 1;
	}

	data->TrackBar.result = message_result;
	data->TrackBar.dragging = dragging;
	data->TrackBar.thumbPos = thumb_pos;
	data->TrackBar.range = range;
	data->TrackBar.value = value;
	data->TrackBar.minimum = minimum;
	data->TrackBar.step = step;
	data->TrackBar.showNumbers = show_numbers;

	if (changed) {
		InvalidateRect(window, NULL, FALSE);

		HWND parent = GetParent(window);
		SendMessage(parent, WM_HSCROLL, MAKEWPARAM(TB_THUMBTRACK, (unsigned short)(value + minimum)), (LPARAM)window);

		if (play_click == true && !data->TrackBar.clickSuppress) {
			Sound_Effect(Rule->GenericClick);
		}
	}

	return(0);
}


/// <summary>
/// Handles the messages for an owner-drawn group box.
/// The frame is drawn as four lines with a gap left in the top edge for the caption, which
/// is written through the surface's device context in the dialog font.
/// </summary>
/// <returns>Returns with the result of the original window procedure for anything this
/// routine does not paint itself.</returns>
LRESULT CALLBACK GroupBoxCtrlProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	switch (message) {
		case WM_PAINT: {
			int locks = 0;
			while (((DSurface *)AlternateSurface)->Is_Locked()) {
				locks++;
				AlternateSurface->Unlock();
			}

			HDC hdc = ((DSurface *)AlternateSurface)->GetDC();
			SelectObject(hdc, ODFontPtr);
			SetTextColor(hdc, ODColorText);
			SetBkMode(hdc, TRANSPARENT);

			char text[256];
			GetWindowText(window, text, sizeof(text));

			SIZE text_size;
			GetTextExtentPoint32(hdc, text, strlen(text), &text_size);

			RECT rect;
			Get_Display_Rect(window, &rect);

			int y = rect.top + text_size.cy / 2;
			TextOut(hdc, rect.left + 10, rect.top, text, strlen(text));

			((DSurface *)AlternateSurface)->ReleaseDC(hdc);

			while (locks > 0) {
				((DSurface *)AlternateSurface)->Lock();
				locks--;
			}

			int color = ODColorToHiColor(ODColorFrame);

			AlternateSurface->Draw_Line(Point2D(rect.left, y), Point2D(rect.left + 8, y), color);

			/// With an empty caption the top edge is one full-width line, so the segment
			/// starts at rect.left. Do NOT change this to rect.right: it draws a visibly
			/// broken, empty top edge. That change has been made and backed out twice.
			int spacing = 12;
			if (text_size.cx == 0) {
				spacing = 0;
			}

			AlternateSurface->Draw_Line(Point2D(text_size.cx + rect.left + spacing, y), Point2D(rect.right, y), color);
			AlternateSurface->Draw_Line(Point2D(rect.left, y), Point2D(rect.left, rect.bottom), color);
			AlternateSurface->Draw_Line(Point2D(rect.left, rect.bottom), Point2D(rect.right, rect.bottom), color);
			AlternateSurface->Draw_Line(Point2D(rect.right, y), Point2D(rect.right, rect.bottom), color);

			ValidateRect(window, NULL);
			return(0);
		}

		case WM_ERASEBKGND:
			return(1);

		case WM_NCPAINT:
			return(0);
	}

	WNDPROC proc = NULL;
	OriginalWndProcs.getValue(window, proc);
	return(CallWindowProc(proc, window, message, wparam, lparam));
}


/// <summary>
/// Handles the messages for an owner-drawn hotkey control.
/// The key the control currently holds is spelled out by Build_Hotkey_String and drawn
/// inside a border, over a cached copy of the dialog background.
/// </summary>
/// <returns>Returns with the result of the original window procedure for anything this
/// routine does not paint itself.</returns>
LRESULT CALLBACK HotkeyCtrlProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	switch (message) {
		case WM_PAINT: {
			WinData* data = NULL;
			RECT rect;
			Get_Display_Rect(window, &rect);
			Rect alternate_rect(rect.left, rect.top, rect.right - rect.left + 1, rect.bottom - rect.top + 1);
			Rect win_rect(0, 0, alternate_rect.Width, alternate_rect.Height);
			ODWinData.getPointer(window, &data);
			if (data->cachedSurface == NULL) {
				BSurface * surf = new BSurface(alternate_rect.Width, alternate_rect.Height, 2);
				data->cachedSurface = surf;
				_surface_count++;
				surf->Blit_From(win_rect, *AlternateSurface, alternate_rect);
			}

			char string[64];
			int key = SendMessage(window, HKM_GETHOTKEY, 0, 0);
			Build_Hotkey_String((KeyNumType)key, string);

			if (data->cachedSurface != NULL) {
				AlternateSurface->Blit_From(alternate_rect, *data->cachedSurface, win_rect);
			}
			OD_Draw_Rect(*AlternateSurface, alternate_rect, 1, 0xFFFFFFFF);
			if (strlen(string)) {
				rect.left += 4;
				rect.top += 4;
				rect.right -= 4;
				rect.bottom -= 4;
				ODDrawTextBG(*AlternateSurface, string, &rect, ODFontPtr, ODColorText, DT_SINGLELINE|DT_VCENTER);
			}
			ValidateRect(window, NULL);
			return(0);
		}

		case WM_ERASEBKGND:
			return(1);

		case WM_NCPAINT:
			return(0);
	}

	WNDPROC proc = NULL;
	OriginalWndProcs.getValue(window, proc);
	return(CallWindowProc(proc, window, message, wparam, lparam));
}


/// <summary>
/// Converts a key code into its printable name.
/// This routine is used by the hotkey control to show a binding the way the player's own
/// keyboard layout names it, with the modifier names spelled out ahead of the key.
/// </summary>
/// <param name="key">The key, complete with its modifier bits, to spell out.</param>
/// <param name="buffer">Buffer to build the name in.</param>
/// <remarks>Be sure that the buffer is big enough for the modifier names as well.</remarks>
int Build_Hotkey_String(KeyNumType key, char * buffer)
{
	char key_name[32];
	unsigned char modifier = HIBYTE(key);

	buffer[0] = '\0';

	UINT lparam;

	/// (p << 16) - places the scan code into bits 16-23.
	/// (1 <<  0) - purpose unknown; Windows does not document this bit.
	/// (1 << 24) - Extended-key bit. Distinguishes some keys on an enhanced keyboard.
	/// (1 << 25) - "Don't care" bit. Should not distinguish between left and right ctrl and shift keys.

	if ((modifier & (WWKEY_ALT_BIT >> 8)) != 0) {
		lparam = MapVirtualKey(VK_MENU, 0) ;
		lparam = (lparam << 16);
		lparam |= (1 << 0);
		lparam |= (1 << 25);
		GetKeyNameText(lparam, key_name, sizeof(key_name));
		strcat(buffer, key_name);
		strcat(buffer, "+");
	}

	if ((modifier & (WWKEY_CTRL_BIT >> 8)) != 0) {
		lparam = MapVirtualKey(VK_CONTROL, 0);
		lparam = (lparam << 16);
		lparam |= (1 << 0);
		lparam |= (1 << 25);
		GetKeyNameText(lparam, key_name, sizeof(key_name));
		strcat(buffer, key_name);
		strcat(buffer, "+");
	}

	if ((modifier & (WWKEY_SHIFT_BIT >> 8)) != 0) {
		lparam = MapVirtualKey(VK_SHIFT, 0);
		lparam = (lparam << 16);
		lparam |= (1 << 0);
		lparam |= (1 << 25);
		GetKeyNameText(lparam, key_name, sizeof(key_name));
		strcat(buffer, key_name);
		strcat(buffer, "+");
	}

	lparam = MapVirtualKey(key & 0xFF, 0);
	lparam = (lparam << 16);
	lparam |= (1 << 0);
	lparam |= (1 << 25);

	if ((modifier & (WWKEY_RLS_BIT >> 8)) != 0) {
		lparam |= (1 << 24);
	}

	GetKeyNameText(lparam, key_name, sizeof(key_name));
	strcat(buffer, key_name);

	return(0);
}

unsigned short ODRComponentMask;
unsigned short ODGComponentMask;
unsigned short ODBComponentMask;


/// <summary>
/// Sets up the color component masks used for blending.
/// The masks depend on how the display surface packs its pixels, so this routine cannot
/// run until the video mode is known.
/// </summary>
void ODInitMasks(void)
{
	ODRComponentMask = 255;
	ODRComponentMask = ODRComponentMask >> DSurface::Get_Red_Left();
	ODRComponentMask <<= DSurface::Get_Red_Right();

	ODGComponentMask = 255;
	ODGComponentMask = ODGComponentMask >> DSurface::Get_Green_Left();
	ODGComponentMask <<= DSurface::Get_Green_Right();

	ODBComponentMask = 255;
	ODBComponentMask = ODBComponentMask >> DSurface::Get_Blue_Left();
	ODBComponentMask <<= DSurface::Get_Blue_Right();
}


/// <summary>
/// Loads the artwork the owner-draw controls are built from.
/// Every button, tab, arrow, grip and check box piece is pulled into the surface cache up
/// front, so that no control has to reach for the disk while a dialog is painting.
/// </summary>
void ODCacheImages(void)
{
	SurfaceCache.CachePCX("dbak6440.pcx");
	SurfaceCache.CachePCX("gdii.pcx");
	SurfaceCache.CachePCX("nodi.pcx");
	SurfaceCache.CachePCX("arrow_uu.pcx");
	SurfaceCache.CachePCX("arrow_ud.pcx");
	SurfaceCache.CachePCX("arrow_du.pcx");
	SurfaceCache.CachePCX("arrow_dd.pcx");
	SurfaceCache.CachePCX("leftbar.pcx");
	SurfaceCache.CachePCX("rightbar.pcx");
	SurfaceCache.CachePCX("trakgrip.pcx");
	SurfaceCache.CachePCX("sbgript.pcx");
	SurfaceCache.CachePCX("sbgripm.pcx");
	SurfaceCache.CachePCX("sbgripb.pcx");
	SurfaceCache.CachePCX("bar_ll.pcx");
	SurfaceCache.CachePCX("bar_lr.pcx");
	SurfaceCache.CachePCX("bar_ul.pcx");
	SurfaceCache.CachePCX("bar_ur.pcx");
	SurfaceCache.CachePCX("dlgsysi.pcx", 1);
	SurfaceCache.CachePalettedPCX("dlgsysa.pcx");
	SurfaceCache.CachePCX("wouban.pcx");
	SurfaceCache.CachePCX("wodban.pcx");
	SurfaceCache.CachePCX("wouleave.pcx");
	SurfaceCache.CachePCX("wodleave.pcx");
	SurfaceCache.CachePCX("wousqlch.pcx");
	SurfaceCache.CachePCX("wodsqlch.pcx");
	SurfaceCache.CachePCX("woudcon.pcx");
	SurfaceCache.CachePCX("woddcon.pcx");
	SurfaceCache.CachePCX("woukick.pcx");
	SurfaceCache.CachePCX("wodkick.pcx");
	SurfaceCache.CachePCX("wouhelp.pcx");
	SurfaceCache.CachePCX("wodhelp.pcx");
	SurfaceCache.CachePCX("woufind.pcx");
	SurfaceCache.CachePCX("wodfind.pcx");
	SurfaceCache.CachePCX("wouopt.pcx");
	SurfaceCache.CachePCX("wodopt.pcx");
	SurfaceCache.CachePCX("woutrny.pcx");
	SurfaceCache.CachePCX("wodtrny.pcx");
	SurfaceCache.CachePCX("wouclan.pcx");
	SurfaceCache.CachePCX("wodclan.pcx");
	SurfaceCache.CachePCX("woufgame.pcx");
	SurfaceCache.CachePCX("wodfgame.pcx");
	SurfaceCache.CachePCX("wouact.pcx");
	SurfaceCache.CachePCX("wodact.pcx");
	SurfaceCache.CachePCX("wouref.pcx");
	SurfaceCache.CachePCX("wodref.pcx");
	SurfaceCache.CachePCX("tab_tlu.pcx");
	SurfaceCache.CachePCX("tab_tmu.pcx");
	SurfaceCache.CachePCX("tab_tru.pcx");
	SurfaceCache.CachePCX("tab_tld.pcx");
	SurfaceCache.CachePCX("tab_tmd.pcx");
	SurfaceCache.CachePCX("tab_trd.pcx");
	SurfaceCache.CachePCX("tab_ftl.pcx");
	SurfaceCache.CachePCX("tab_ftr.pcx");
	SurfaceCache.CachePCX("tab_ftm.pcx");
	SurfaceCache.CachePCX("tab_fbr.pcx");
	SurfaceCache.CachePCX("tab_fbl.pcx");
	SurfaceCache.CachePCX("tab_fbm.pcx");
	SurfaceCache.CachePCX("tab_fmr.pcx");
	SurfaceCache.CachePCX("tab_fml.pcx");
	SurfaceCache.CachePCX("woloper.pcx");
	SurfaceCache.CachePCX("wolsqlch.pcx");
	SurfaceCache.CachePCX("woltrny.pcx");
	SurfaceCache.CachePCX("woluser.pcx");
	SurfaceCache.CachePCX("wolvoice.pcx");
	SurfaceCache.CachePCX("wolpriv.pcx");
	SurfaceCache.CachePCX("wolacpt.pcx");
	SurfaceCache.CachePCX("wolhost.pcx");
	SurfaceCache.CachePCX("wolclan.pcx");
	SurfaceCache.CachePCX("dnarrowp.pcx");
	SurfaceCache.CachePCX("uparrowp.pcx");
	SurfaceCache.CachePCX("dnarrowr.pcx");
	SurfaceCache.CachePCX("uparrowr.pcx");
	SurfaceCache.CachePCX("trofl.pcx");
	SurfaceCache.CachePCX("trofm.pcx");
	SurfaceCache.CachePCX("trofr.pcx");
	SurfaceCache.CachePCX("sb_psh_u.pcx");
	SurfaceCache.CachePCX("sb_psh_d.pcx");
	SurfaceCache.CachePCX("sb_rel_u.pcx");
	SurfaceCache.CachePCX("sb_rel_d.pcx");
	SurfaceCache.CachePCX("bst_chkd.pcx");
	SurfaceCache.CachePCX("bst_uchk.pcx");
	SurfaceCache.CachePCX("bst_chkg.pcx");
	SurfaceCache.CachePCX("bst_uckg.pcx");
	SurfaceCache.CachePCX("ccd_i.pcx");
	SurfaceCache.CachePCX("cce_i.pcx");
	SurfaceCache.CachePCX("cud_i.pcx");
	SurfaceCache.CachePCX("cue_i.pcx");
	SurfaceCache.CachePCX("bue_li30.pcx");
	SurfaceCache.CachePCX("bue_mi30.pcx");
	SurfaceCache.CachePCX("bue_ri30.pcx");
	SurfaceCache.CachePCX("bde_li30.pcx");
	SurfaceCache.CachePCX("bde_mi30.pcx");
	SurfaceCache.CachePCX("bde_ri30.pcx");
	SurfaceCache.CachePCX("bud_li30.pcx");
	SurfaceCache.CachePCX("bud_mi30.pcx");
	SurfaceCache.CachePCX("bud_ri30.pcx");
	SurfaceCache.CachePCX("bue_li24.pcx");
	SurfaceCache.CachePCX("bue_mi24.pcx");
	SurfaceCache.CachePCX("bue_ri24.pcx");
	SurfaceCache.CachePCX("bde_li24.pcx");
	SurfaceCache.CachePCX("bde_mi24.pcx");
	SurfaceCache.CachePCX("bde_ri24.pcx");
	SurfaceCache.CachePCX("bud_li24.pcx");
	SurfaceCache.CachePCX("bud_mi24.pcx");
	SurfaceCache.CachePCX("bud_ri24.pcx");
}


/// <summary>
/// Draws a single blended line.
/// This is the low level routine behind the frames the owner-draw controls are built from.
/// Every pixel along the line is blended toward the color rather than replaced, so the
/// dialog artwork still shows through the frame.
/// </summary>
/// <param name="surf">The surface to draw upon.</param>
/// <param name="start">One end of the line.</param>
/// <param name="end">The other end of the line.</param>
/// <param name="color">The raw color value to blend toward.</param>
/// <param name="steps">The blend strength, 0 through 255.</param>
/// <returns>bool; Was the line drawn?</returns>
bool ODDrawEdgeGlow(Surface & surf, Point2D const & start, Point2D const & end, int color, unsigned char steps)
{
	Point2D startpoint = start;
	Point2D endpoint = end;

	if (startpoint.X > endpoint.X) {
		std::swap(startpoint, endpoint);
	}

	int bpp = surf.Bytes_Per_Pixel();
	void * buffer = surf.Lock(startpoint);
	if (buffer != NULL) {
		unsigned short blend_color = (unsigned short)color;

		if (startpoint.Y == endpoint.Y) {
			/*
			 * Simplest of the blits, straight horizontal line.
			 */
			if (bpp == 1) {
				memset(buffer, color, endpoint.X - startpoint.X + 1);
			} else {
				for (int i = 0; i <= endpoint.X - startpoint.X; i++) {
					*((unsigned short *)buffer + i) = OD_Blend_Color(*((unsigned short *)buffer + i), blend_color, steps);
				}
			}
		} else if (startpoint.X == endpoint.X) {
			int pitch = startpoint.Y > endpoint.Y ? -surf.Stride() : surf.Stride();

			/*
			 * Straight vertical line.
			 */
			int dy = abs(endpoint.Y - startpoint.Y);
			for (int i = 0; i <= dy; i++) {
				if (bpp == 1) {
					*(unsigned char *)buffer = color;
				} else {
					*(unsigned short *)buffer = OD_Blend_Color(*(unsigned short *)buffer, blend_color, steps);
				}
				buffer = (unsigned char *)buffer + pitch;
			}
		} else {
			/*
			 * Distances to x and y.
			 */
			int dx = endpoint.X - startpoint.X;
			int dy = endpoint.Y - startpoint.Y;
			/*
			 * The line isn't straight so we need to do some maths.
			 */
			int pitch = surf.Stride();
			if (dy < 0) {
				pitch = -pitch;
			}

			dy = abs(dy);
			int dx2 = 2 * dx;
			int dy2 = 2 * dy;

			if (dx > dy) {
				/*
				 * The slope is not steep.
				 */
				int delta = dy2 - dx;

				/*
				 * Plot low line.
				 */
				for (int i = 0; i < dx; i++) {
					if (bpp == 1) {
						*((unsigned char *)buffer + i) = color;
					} else {
						*((unsigned short *)buffer + i) = OD_Blend_Color(*((unsigned short *)buffer + i), blend_color, steps);
					}

					if (delta > 0) {
						buffer = (unsigned char *)buffer + pitch;
						delta -= dx2;
					}

					delta += dy2;
				}
			} else {
				/*
				 * The slope is steep.
				 */
				int delta = dx2 - dy;
				int k = 0;

				/*
				 * Plot high line.
				 */
				for (int i = 0; i < dy; i++) {
					if (bpp == 1) {
						*((unsigned char *)buffer + k) = color;
					} else {
						*((unsigned short *)buffer + k) = OD_Blend_Color(*((unsigned short *)buffer + k), blend_color, steps);
					}

					if (delta > 0) {
						k++;
						delta -= dy2;
					}

					delta += dx2;
					buffer = (unsigned char *)buffer + pitch;
				}
			}
		}

		surf.Unlock();
		return(true);
	}
	return(false);
}


/// <summary>
/// Draws a blended frame around a rectangle.
/// Each of the four edges is blended with its own strength, which is what gives a control
/// its raised or sunken look. Every corner pixel is left to a single edge so that no pixel
/// is blended twice and shows up darker than its neighbors.
/// </summary>
/// <param name="surface">The surface to draw upon.</param>
/// <param name="rect">The rectangle to frame.</param>
/// <param name="raised">Should the frame appear raised rather than sunken?</param>
/// <param name="count">How many nested frames to draw, working inward.</param>
/// <param name="left_alpha">The blend strength for the left edge.</param>
/// <param name="top_alpha">The blend strength for the top edge.</param>
/// <param name="right_alpha">The blend strength for the right edge.</param>
/// <param name="bottom_alpha">The blend strength for the bottom edge.</param>
void ODDrawEdgeGlows(Surface & surface, Rect const & rect, BOOL raised, int count, int left_alpha, int top_alpha, int right_alpha, int bottom_alpha)
{
	/*
	 * The function flips these when the frame is sunken, which is how callers
	 * choose between a raised vs. sunken style (i.e., highlight/shadow swapped).
	 */

	int color2 = 0xFFFF;	// used on top & left edges (default: lighter)
	int color1 = 0;			// used on bottom & right edges (default: darker)

	/*
	 * When the frame is sunken, invert highlight/shadow:
	 * - top/left become darker
	 * - bottom/right become lighter
	 */
	if (!raised) {
		color1 = 0xFFFF;
		color2 = 0;
	}

	for (int i = 0; i < count; i++) {

		Point2D end1;
		Point2D start1;
		Point2D end2;
		Point2D start2;
		Point2D end3;
		Point2D start3;
		Point2D end4;
		Point2D start4;

		/*
		 * Top edge (left -> right). The "-2" keeps this top line from touching the
		 * top-right corner pixel, avoiding overdraw with the right edge drawn below.
		 */
		end1.X = rect.Width - i + rect.X - 2;
		start1.X = i + rect.X;
		end1.Y = i + rect.Y;
		start1.Y = i + rect.Y;
		ODDrawEdgeGlow(surface, start1, end1, color2, top_alpha);

		/*
		 * Bottom edge (left -> right).
		 */
		end2.X = rect.Width - i + rect.X - 1;
		start2.X = i + rect.X;
		end2.Y = rect.Y - i + rect.Height - 1;
		start2.Y = end2.Y;
		ODDrawEdgeGlow(surface, start2, end2, color1, bottom_alpha);

		/*
		 * Left edge (top -> bottom). The "+1" on the top avoids double-hitting the
		 * top-left corner pixel (the top edge already handled it).
		 */
		end3.X = rect.X + i;
		start3.X = end3.X;
		start3.Y = rect.Y + i + 1;
		end3.Y = rect.Y - i + rect.Height - 1;
		ODDrawEdgeGlow(surface, start3, end3, color2, left_alpha);

		/*
		 * Right edge (top -> bottom). The "-2" on the bottom avoids double-hitting
		 * the bottom-right corner pixel (the bottom edge already handled it).
		 */
		end4.X = rect.X - i + rect.Width - 1;
		start4.X = end4.X;
		start4.Y = i + rect.Y;
		end4.Y = rect.Y - i + rect.Height - 2;
		ODDrawEdgeGlow(surface, start4, end4, color1, right_alpha);
	}
}


/// <summary>
/// Draws a scroll arrow bitmap.
/// The image is picked out of the surface cache by direction and press state, and drawn at
/// its own size from the top left corner of the rectangle given.
/// </summary>
/// <param name="surface">The surface to draw upon.</param>
/// <param name="rect">The area whose top left corner the arrow is drawn from.</param>
/// <param name="upward">Should the upward pointing arrow be used?</param>
/// <param name="pressed">Is the arrow button currently held down?</param>
void ODDrawArrowBitmap(Surface & surface, Rect const & rect, BOOL upward, BOOL pressed)
{
	char fname[32];

	char state = 'r';
	if (pressed) {
		state = 'p';
	}

	if (upward) {
		sprintf(fname, "uparrow%c.pcx", state);
	} else {
		sprintf(fname, "dnarrow%c.pcx", state);
	}

	Surface * arrowSurface = SurfaceCache.GetSurface(fname);

	Rect destRect = rect;
	destRect.Width = arrowSurface->Get_Width();
	destRect.Height = arrowSurface->Get_Height();

	Rect srcRect;
	srcRect.Y = 0;
	srcRect.X = 0;
	srcRect.Width = arrowSurface->Get_Width();
	srcRect.Height = arrowSurface->Get_Height();

	surface.Blit_From(destRect, *arrowSurface, srcRect);
}


/// <summary>
/// Converts a Windows color reference into a display pixel.
/// The dialog colors are all written as RGB() values, so they have to be packed into the
/// pixel layout of the display surface before anything can be drawn with them.
/// </summary>
/// <returns>Returns with the packed pixel value. An all-ones color is passed through
/// unchanged.</returns>
int ODColorToHiColor(COLORREF color)
{
	if (color == 0xFFFFFFFF) {
		return(0xFFFFFFFF);
	}
	/// Do not replace the union with direct byte extraction. It improves several callers and
	/// breaks ProgressBarCtrlProc, which is otherwise exact -- and an exact caller outranks the
	/// partial ones.
	union {
		struct {
			unsigned int red : 8;
			unsigned int green : 8;
			unsigned int blue : 8;
			unsigned int a : 8;
		};
		int v;
	} c;

	c.v = color;

	return(DSurface::Build_Hicolor_Pixel(c.red, c.green, c.blue));
}


/// <summary>
/// Draws a rectangular outline around an area.
/// The outline is drawn outside the rectangle given, thickening outward as the offset
/// grows. Use this routine for the plain frames the owner-draw controls sit inside.
/// </summary>
/// <param name="offset">How far beyond the rectangle, in pixels, the outline reaches.</param>
/// <param name="color">The raw color to draw with, or -1 for the common frame color.</param>
void OD_Draw_Rect(Surface & surf, Rect const & rect, int offset, int color)
{
	if (color == -1) {
		color = ODColorToHiColor(ODColorFrame);
	}

	Rect work;
	work.X = rect.X - offset;
	work.Y = rect.Y - offset;
	work.Width = 2 * offset + rect.Width;
	work.Height = 2 * offset + rect.Height;

	for (int i = 0; i < offset; i++) {

		Point2D end1;
		Point2D start1;
		Point2D end2;
		Point2D start2;
		Point2D end3;
		Point2D start3;
		Point2D end4;
		Point2D start4;

		/*
		 * The same eight-point ring that ODDrawEdgeGlows walks.
		 */
		end1.X = work.Width - i + work.X - 2;
		start1.X = i + work.X;
		end1.Y = i + work.Y;
		start1.Y = i + work.Y;
		surf.Draw_Line(start1, end1, color);

		end2.X = work.Width - i + work.X - 1;
		start2.X = i + work.X;
		end2.Y = work.Y - i + work.Height - 1;
		start2.Y = end2.Y;
		surf.Draw_Line(start2, end2, color);

		end3.X = work.X + i;
		start3.X = end3.X;
		start3.Y = work.Y + i + 1;
		end3.Y = work.Y - i + work.Height - 1;
		surf.Draw_Line(start3, end3, color);

		end4.X = work.X - i + work.Width - 1;
		start4.X = end4.X;
		start4.Y = i + work.Y;
		end4.Y = work.Y - i + work.Height - 2;
		surf.Draw_Line(start4, end4, color);
	}
}


/// <summary>
/// Draws the bevelled border of an owner-draw button.
/// Each of the four edges is drawn in its own shade of green, so that the border reads as
/// a raised frame rather than a flat outline. The frame is drawn outside the rectangle.
/// </summary>
/// <param name="offset">How far beyond the rectangle, in pixels, the border reaches.</param>
void ODDrawButtonRect(Surface & surf, Rect const & rect, int offset)
{
	Rect work = rect;

	int color1 = DSurface::Build_Hicolor_Pixel(39, 248, 116);
	int color2 = DSurface::Build_Hicolor_Pixel(19, 123, 57);
	int color3 = DSurface::Build_Hicolor_Pixel(30, 186, 87);
	int color4 = DSurface::Build_Hicolor_Pixel(24, 153, 71);

	work.X += -1 - offset;
	work.Y += -1 - offset;
	work.Width += 2 * offset + 2;
	int y2 = 2 * offset + 2 + work.Height;

	for (int i = 0; i < offset; i++) {

		Point2D end1;
		Point2D start1;
		Point2D end2;
		Point2D start2;
		Point2D end3;
		Point2D start3;
		Point2D end4;
		Point2D start4;

		/*
		 * The same eight-point ring as OD_Draw_Rect / ODDrawEdgeGlows, but each
		 * edge gets its own shade so the border reads as a bevelled button.
		 * r.Height is never written back - y2 carries the grown height.
		 */
		end1.X = work.Width - i + work.X - 2;
		start1.X = i + work.X;
		end1.Y = i + work.Y;
		start1.Y = i + work.Y;
		surf.Draw_Line(start1, end1, color1);

		end2.X = work.Width - i + work.X - 1;
		start2.X = i + work.X;
		end2.Y = work.Y - i + y2 - 1;
		start2.Y = end2.Y;
		surf.Draw_Line(start2, end2, color2);

		end3.X = work.X + i;
		start3.X = end3.X;
		start3.Y = work.Y + i + 1;
		end3.Y = work.Y - i + y2 - 1;
		surf.Draw_Line(start3, end3, color3);

		end4.X = work.X - i + work.Width - 1;
		start4.X = end4.X;
		start4.Y = i + work.Y;
		end4.Y = work.Y - i + y2 - 2;
		surf.Draw_Line(start4, end4, color4);
	}
}


/// <summary>
/// Draws text onto a surface with the Windows text formatter.
/// This routine borrows a device context from the surface, unlocking it as often as it
/// must beforehand, and lets Windows lay the string out within the rectangle given.
/// </summary>
/// <param name="format">The DrawText formatting flags to lay the string out with.</param>
/// <returns>Returns with the pixel width of the string.</returns>
int ODDrawTextBG(Surface & surface, LPCSTR string, LPRECT rect, HGDIOBJ font, COLORREF color, UINT format)
{
	int locks = 0;

	while (((DSurface &)surface).Is_Locked()) {
		locks++;
		surface.Unlock();
	}

	HDC hdc = ((DSurface &)surface).GetDC();

	SelectObject(hdc, font);
	SetTextColor(hdc, color);
	SetBkMode(hdc, TRANSPARENT);

	SIZE char_size;
	GetTextExtentPoint32(hdc, string, strlen(string), &char_size);
	DrawText(hdc, string, strlen(string), rect, format);

	((DSurface &)surface).ReleaseDC(hdc);

	while (locks > 0) {
		((DSurface &)surface).Lock();
		locks--;
	}

	return(char_size.cx);
}


/// <summary>
/// Draws word wrapped text with a remapped bitmap font.
/// This routine breaks the text into lines that will fit the rectangle -- honoring the
/// newlines already in it and breaking at a space wherever one can be found -- and hands
/// each line in turn to ODDrawCharRemap.
/// </summary>
/// <param name="name">The base name of the font sheets to draw with.</param>
/// <param name="flags">The OD_DRAW_CHAR alignment flags to lay each line out with.</param>
/// <param name="char_spacing">The extra spacing to insert between characters.</param>
int OD_Draw_Text_Remap(Surface & surface, const char * text, Rect const & rect, char const * name, COLORREF color, int flags, int char_spacing)
{
	int line_len = strlen(text);
	char const * line_ptr = text;
	Rect draw_rect = rect;

	FontMetrics data;
	if (!ODGetFontMetrics(name, &data)) {
		return(0);
	}

	while (line_len) {
		if (line_ptr) {
			char const * nl_ptr = strchr(line_ptr, '\n');
			if (nl_ptr) {
				int nl_len = (int)(nl_ptr - line_ptr) + 1;
				if (line_len >= nl_len) {
					line_len = nl_len;
				}
			}
		}

		if ((unsigned char)*line_ptr <= ' ') {
			++line_ptr;
			if (--line_len == 0) {
				return(0);
			}
		}

		int text_width = 0;
		for (int i = 0; i < line_len; ++i) {
			text_width += char_spacing + data.charWidths[(unsigned char)text[i]];
		}

		if (text_width > draw_rect.Width - draw_rect.X) {
			int fallback = line_len - 1;
			int cut = line_len - 1;

			flags &= ~4;

			while (cut > 0) {
				if ((unsigned char)line_ptr[cut] <= ' ') {
					break;
				}
				--cut;
			}
			if (cut > 0) {
				line_len = cut;
				if (cut != -1) {
					continue;
				}
			}

			line_len = fallback;
		} else {
			ODDrawCharRemap(surface, line_ptr, line_len, draw_rect, name, color, (char)flags, char_spacing);
			line_ptr += line_len;
			draw_rect.Y += data.glyphHeight;
			line_len = strlen(line_ptr);
		}
	}

	return(0);
}


/// <summary>
/// Determines how strongly a hue should be remapped.
/// The font remapper uses this to pull its hue shift back around the primary colors, so
/// that text tinted near one of them does not swing away from the color asked for.
/// </summary>
/// <param name="hue">The hue to compute the factor for.</param>
/// <returns>Returns with the scale factor; the nearer the hue sits to a primary, the smaller
/// it gets.</returns>
float ODCalcTextRemapFactor(int hue)
{
	float val = 1.0f;

	int arr[3];
	arr[0] = 43;
	arr[1] = 128;
	arr[2] = 213;

	for (int i = 0; i < 3; i++) {
		int value = arr[i];

		if (hue > value - 16 && hue <= value) {
			val = float(value - hue);
			val *= (1.0f / 16);
			val *= (60.0f / 100);
			val += (40.0f / 100);
		} else if (hue > value && hue <= value + 16) {
			val = float(hue - value);
			val *= (1.0f / 16);
			val *= (60.0f / 100);
			val += (40.0f / 100);
		}
	}
	return(val);
}


/// <summary>
/// Draws a line of text with a remapped bitmap font.
/// This routine builds a table that shifts the font's own palette toward the color asked
/// for and then alpha blends each character onto the destination surface. It is the low
/// level draw that all of the owner-draw remapped text ends up going through.
/// </summary>
/// <param name="max_chars">The maximum number of characters of the text to draw.</param>
/// <param name="rect">The rectangle to align the text within.</param>
/// <param name="font_name">The base name of the font sheets to draw with.</param>
/// <param name="flags">The OD_DRAW_CHAR alignment flags to lay the text out with.</param>
/// <param name="char_spacing">The extra spacing to insert between characters.</param>
void ODDrawCharRemap(Surface & dst_surf, const char *text, int max_chars, Rect const & rect, char const *font_name, COLORREF color, char flags, int char_spacing)
{
	int i;
	Rect draw_rect = rect;

	char name_i[64];
	strcpy(name_i, font_name);
	strcat(name_i, "i.pcx");

	char palette[768];
	Surface *sheet_i = SurfaceCache.GetSurface(name_i, palette);
	if (sheet_i == NULL) {
		return;
	}

	char name_a[64];
	strcpy(name_a, font_name);
	strcat(name_a, "a.pcx");

	Surface *sheet_a = SurfaceCache.GetSurface(name_a, NULL);
	if (sheet_a == NULL) {
		return;
	}

	RGBClass remap_rgb((unsigned char)color, (unsigned char)(color >> 8), (unsigned char)(color >> 16));
	HSVClass remap_hsv = remap_rgb;
	RGBClass pal_rgb;
	HSVClass out_hsv;

	int hue = remap_hsv.Get_Hue();

	int end = int(hue + 15.0);
	float min_factor = 1.0f;
	for (i = int(hue - 15.0); i <= end; ++i) {
		float factor = ODCalcTextRemapFactor(i);
		if (factor < min_factor) {
			min_factor = factor;
		}
	}

	unsigned char sat = (unsigned char)remap_hsv.Get_Saturation();
	unsigned char val = (unsigned char)remap_hsv.Get_Value();

	unsigned short remap_table[256];
	float hue_float = (float)hue;
	unsigned char *pal = (unsigned char *)&palette;
	for (i = 0; i < 256; ++i) {
		pal_rgb.Set_Red(pal[0]);
		pal_rgb.Set_Green(pal[1]);
		pal_rgb.Set_Blue(pal[2]);
		HSVClass pal_hsv = pal_rgb;

		/*
		 * Start from the palette entry's HSV and adjust each channel. The
		 * wholesale copy is fully overwritten below.
		 */
		out_hsv = pal_hsv;
		out_hsv.Set_Hue((unsigned char)(int)(hue_float - (int)(68.0f - pal_hsv.Get_Hue()) * min_factor));
		out_hsv.Set_Saturation((unsigned char)((sat * out_hsv.Get_Saturation()) >> 8));
		out_hsv.Set_Value((unsigned char)((val * out_hsv.Get_Value()) >> 8));

		RGBClass out_rgb = out_hsv;
		pal_rgb = out_rgb;

		int packed = (((out_rgb.Get_Blue() << 8) | out_rgb.Get_Green()) << 8) | out_rgb.Get_Red();
		remap_table[i] = (unsigned short)ODColorToHiColor(packed);
		pal += 3;
	}

	FontMetrics font_data;
	if (!ODGetFontMetrics(font_name, &font_data)) {
		return;
	}

	if ((int)strlen(text) < max_chars) {
		max_chars = strlen(text);
	}

	int total_width = 0;
	for (i = 0; i < max_chars; ++i) {
		total_width += font_data.charWidths[(unsigned char)text[i]] + char_spacing;
	}

	if ((flags & OD_DRAW_CHAR_FLAG_HORIZONTAL_CENTER) != 0) {
		draw_rect.X += (draw_rect.Width - draw_rect.X - total_width) / 2;
	} else if ((flags & OD_DRAW_CHAR_ALIGN_FLAG_RIGHT) != 0) {
		draw_rect.X = draw_rect.Width - total_width - 1;
	}

	if ((flags & OD_DRAW_CHAR_FLAG_VERTICAL_CENTER) != 0) {
		draw_rect.Y = draw_rect.Y + (draw_rect.Height - font_data.glyphHeight - draw_rect.Y) / 2;
	}

	draw_rect.Y -= font_data.topMargin;
	--draw_rect.X;

	unsigned char *src_i = (unsigned char *)sheet_i->Lock();
	unsigned char *src_a = (unsigned char *)sheet_a->Lock();
	unsigned char *dst = (unsigned char *)dst_surf.Lock();

	if (src_i != NULL && src_a != NULL && dst != NULL) {
		int cell_w = font_data.glyphWidth + font_data.leftMargin;
		int cell_h = font_data.glyphHeight + font_data.topMargin;
		int chars_per_row = sheet_i->Get_Width() / (font_data.glyphWidth + font_data.leftMargin);
		int dst_stride = dst_surf.Stride() / 2;
		int src_stride = sheet_i->Stride();

		int x = draw_rect.X;
		for (int i = 0; i < max_chars; ++i) {

			if ((unsigned char)text[i] <= ' ') {
				x += font_data.charWidths[(unsigned char)text[i]] + char_spacing;
			} else {
				int glyph = (unsigned char)text[i] + 1;
				int src_x = (glyph % chars_per_row) * cell_w;
				int src_y = (glyph / chars_per_row) * cell_h;

				int src_y_end = src_y + cell_h;
				int src_delta = src_i - src_a;
				unsigned char *alpha_col = src_a + (src_y * src_stride + src_x);
				unsigned char *dst_col = dst + 2 * (dst_stride * draw_rect.Y + x);

				for (int sx = src_x; sx < src_x + cell_w; ++sx) {
					if (src_y < src_y_end) {
						unsigned short *dst_px = (unsigned short *)dst_col;
						unsigned char *alpha_px = alpha_col;

						int sy = src_y_end - src_y;
						do {
							unsigned char alpha = *alpha_px;
							if (alpha != 0) {
								unsigned char index = alpha_px[src_delta];
								*dst_px = OD_Blend_Color(*dst_px, remap_table[index], alpha);
							}

							dst_px += dst_stride;
							alpha_px += src_stride;
							--sy;
						} while (sy != 0);
					}

					++alpha_col;
					dst_col += 2;
				}

				x += font_data.charWidths[(unsigned char)text[i]] + char_spacing;
			}
		}
	}

	if (&dst_surf != NULL) {
		dst_surf.Unlock();
	}
	sheet_a->Unlock();
	sheet_i->Unlock();
}


/// <summary>
/// Fetches the metrics of a remappable bitmap font.
/// This routine measures the font's sheet -- the margins, the size of a character cell and
/// the inked width of every character -- so that the remap text routines know how to lay
/// characters out. Measuring is expensive, so the result is kept by font name.
/// </summary>
/// <param name="font_name">The base name of the font, without the sheet suffix.</param>
/// <param name="metrics">Buffer to fill in with the measurements.</param>
/// <returns>bool; Were the metrics available?</returns>
bool ODGetFontMetrics(char const * font_name, FontMetrics * metrics)
{
	static Dictionary<Wstring, FontMetrics> metricsDict(Wstring_Hash);

	char buf[64];
	strcpy(buf, font_name);
	strcat(buf, "a.pcx");

	Wstring name;
	name = (char *)font_name;
	name.toLower();

	FontMetrics * found = NULL;
	if (metricsDict.getPointer(name, &found)) {
		if (metrics != NULL) {
			*metrics = *found;
			return(true);
		}
	}

	DebugString("TS: Computing font metrics....\n");

	FontMetrics temp;
	memset(&temp, 0, sizeof(temp));

	char palette[768];
	Surface * surf = SurfaceCache.GetSurface(buf, palette);
	if (surf == NULL) {
		return(false);
	}

	char * basePtr = (char *)surf->Lock();
	int stride = surf->Stride();

	/*
	 * ----------------------------------------------------------------
	 * Vertical metrics: topMargin = blank rows above the glyph row,
	 * glyphHeight = inked rows (probed at column 4).
	 * ----------------------------------------------------------------
	 */
	temp.topMargin = 0;
	while (temp.topMargin < surf->Get_Height()) {
		if (basePtr[stride * temp.topMargin + 4] != 0) break;
		++temp.topMargin;
	}
	int y = temp.topMargin;
	while (y < surf->Get_Height()) {
		if (basePtr[stride * y + 4] == 0) break;
		++y;
		++temp.glyphHeight;
	}

	/*
	 * ----------------------------------------------------------------
	 * Horizontal metrics: leftMargin = blank columns before the glyphs,
	 * glyphWidth = inked columns (probed along row 'top').
	 * ----------------------------------------------------------------
	 */
	temp.leftMargin = 0;
	while (temp.leftMargin < surf->Get_Width()) {
		if (basePtr[stride * temp.topMargin + temp.leftMargin] != 0) break;
		++temp.leftMargin;
	}
	int left = temp.leftMargin;

	int x;
	x = left;
	while (x < surf->Get_Width()) {
		if (basePtr[stride * temp.topMargin + x] == 0) break;
		++x;
		++temp.glyphWidth;
	}

	/*
	 * ----------------------------------------------------------------
	 * Compute per-character metrics
	 * ----------------------------------------------------------------
	 */
	int width = surf->Get_Width();
	int charsPerRow = width / (left + temp.glyphWidth);
	for (int ch = 0; ch < 256; ++ch) {

		int left = temp.leftMargin;
		int fontHeight = temp.glyphHeight;
		int top = temp.topMargin;
		int fontWidth = temp.glyphWidth;

		int glyphY = top + (fontHeight + top) * ((ch + 1) / charsPerRow);
		int glyphX = left + (left + fontWidth) * ((ch + 1) % charsPerRow);

		int first = -1;
		int last = 0;

		for (int x = glyphX; x < glyphX + fontWidth; ++x) {
			int nonEmpty = 0;
			for (int y = glyphY; y < glyphY + fontHeight; ++y) {
				if (basePtr[stride * y + x] != 0) ++nonEmpty;
			}
			if (nonEmpty) {
				last = x;
				if (first == -1) first = x;
			}
		}

		if (first != -1) {
			temp.charWidths[ch] = (last - first + 1);
		} else {
			temp.charWidths[ch] = (fontWidth / 3 + 1);
		}
	}

	surf->Unlock();

	/*
	 * ----------------------------------------------------------------
	 * Store result in caller's buffer
	 * ----------------------------------------------------------------
	 */
	memcpy(metrics, &temp, sizeof(FontMetrics));

	metricsDict.add(name, temp);

	return(true);
}


/// <summary>
/// Draws a line of text onto a surface.
/// This routine borrows a device context from the surface, unlocking it as often as it
/// must beforehand, and lets Windows put the text out aligned within the rectangle given.
/// Nothing is drawn while the game does not hold the focus.
/// </summary>
/// <param name="len">The number of characters of the text to draw.</param>
/// <param name="surface">The surface to draw upon, or NULL to draw on the alternate
/// surface.</param>
/// <returns>Returns with the pixel width of the text.</returns>
int OD_Draw_Text(COLORREF color, HFONT font, Rect const & rect, const char * text, int len, int x_alignment, int y_alignment, Surface * surface)
{
	if (!GameInFocus) {
		return(0);
	}

	DSurface *destsurf = (DSurface *)surface;
	if (!surface) {
		destsurf = (DSurface *)AlternateSurface;
	}

	int lock_count = 0;
	while (destsurf->Is_Locked()) {
		lock_count++;
		destsurf->Unlock();
	}

	SIZE text_size;

	HDC hDC = destsurf->GetDC();
	if (hDC) {

		if (font) {
			SelectObject(hDC, font);
		}

		SetTextColor(hDC, color);
		SetBkMode(hDC, TRANSPARENT);

		GetTextExtentPoint32(hDC, text, len, &text_size);

		int x_offset = rect.X;
		int y_offset = rect.Y;

		if (x_alignment == OD_TEXT_ALIGN_MIN) {
			x_offset += (rect.Width - text_size.cx + 1) / 2;
		} else if (x_alignment == OD_TEXT_ALIGN_CENTER) {
			x_offset += (text_size.cx + 1) / -2;
		} else if (x_alignment == OD_TEXT_ALIGN_MAX) {
			x_offset += -1 - text_size.cx;
		}

		if (y_alignment == OD_TEXT_ALIGN_MIN) {
			y_offset += (rect.Height - text_size.cy + 1) / 2;
		} else if (y_alignment == OD_TEXT_ALIGN_CENTER) {
			y_offset += (text_size.cy + 1) / -2;
		} else if (y_alignment == OD_TEXT_ALIGN_MAX) {
			y_offset += -1 - text_size.cy;
		}

		TextOut(hDC, x_offset, y_offset, text, len);
		destsurf->ReleaseDC(hDC);
	} else {
		text_size.cx = 0;
	}

	while (lock_count) {
		destsurf->Lock();
		lock_count--;
	}

	return(text_size.cx);
}


/// <summary>
/// Handles the owner-draw item message for a control.
/// The controls paint themselves through the game's own surfaces rather than through a
/// device context, so this routine only records the item state that Windows handed over
/// and then forces the control to repaint.
/// </summary>
void OwnerDraw::Draw_Item(LPDRAWITEMSTRUCT drawit)
{
	if (VisibleSurface != NULL && AlternateSurface != NULL) {
		RECT rect1;
		Get_Display_Rect(drawit->hwndItem, &rect1);
		RECT rect2;
		GetClientRect(drawit->hwndItem, &rect2);

		if (GetWindowLong(drawit->hwndItem, GWL_STYLE) & WS_BORDER) {
			int x = GetSystemMetrics(SM_CXBORDER);
			int y = GetSystemMetrics(SM_CYBORDER);
			rect1.left += x;
			rect1.right -= x;
			rect1.top += y;
			rect1.bottom -= y;
		}

		if (drawit->CtlType == ODT_BUTTON) {
			WinData * data = NULL;
			HWND window = drawit->hwndItem;

			ODWinData.getPointer(window, &data);

			data->DrawItem.itemState = drawit->itemState;

			InvalidateRect(window, &drawit->rcItem, FALSE);
			UpdateWindow(window);
		}
	}
}


/// <summary>
/// Draws a dimmed copy of the parent's background behind a control.
/// This routine takes the piece of the parent window that the control covers, darkens it
/// and keeps the result on a surface cached against the control, giving the control a
/// smoked glass look. Later calls simply blit the cached copy.
/// </summary>
void ODDrawDimmedBackground(Rect const & rect, HWND hWnd)
{
	WinData * winData = NULL;
	WinData * parentWinData = NULL;

	ODWinData.getPointer(hWnd, &winData);

	RECT rcClient;
	GetClientRect(hWnd, &rcClient);

	RECT dispChild;
	Get_Display_Rect(hWnd, &dispChild);

	Surface * surface = winData->cachedSurface;
	if (surface == NULL) {
		surface = new BSurface(rcClient.right + 1, rcClient.bottom + 1, 2);
		winData->cachedSurface = surface;
		++_surface_count;

		Rect dstFull(0, 0, rcClient.right + 1, rcClient.bottom + 1);

		HWND parent = GetParent(hWnd);
		ODWinData.getPointer(parent, &parentWinData);

		Surface * parentSurf = parentWinData->cachedSurface;
		if (parentSurf) {
			RECT dispParent;
			Get_Display_Rect(parent, &dispParent);

			Rect srcRel;
			srcRel.X = dispChild.left - dispParent.left;
			srcRel.Y = dispChild.top  - dispParent.top;
			srcRel.Width  = RECT_WIDTH(dispChild) + 1;
			srcRel.Height = RECT_HEIGHT(dispChild) + 1;

			surface->Blit_From(dstFull, *parentSurf, srcRel);
		} else {
			Surface *srcSurf = VisibleSurface;

			Rect srcAbs;
			srcAbs.X = dispChild.left;
			srcAbs.Y = dispChild.top;
			srcAbs.Width  = RECT_WIDTH(dispChild) + 1;
			srcAbs.Height = RECT_HEIGHT(dispChild) + 1;
			if (srcSurf) {
				surface->Blit_From(dstFull, *srcSurf, srcAbs);
			}
		}

		unsigned short* surfptr = (unsigned short*)surface->Lock();
		if (surfptr) {
			for (int i = 0; i < surface->Get_Width() * surface->Get_Height(); ++i) {
				surfptr[i] = OD_Blend_Color(surfptr[i], 0, 180);
			}

			surface->Unlock();
		}
	}

	Rect src;
	src.Width  = rect.Width;
	src.Height = rect.Height;
	src.Y = rect.Y - dispChild.top;
	src.X = rect.X - dispChild.left;

	AlternateSurface->Blit_From(rect, *surface, src);
}


/// <summary>
/// Draws a rectangle that fades in from left to right.
/// The fill runs from the left edge as far as the progress value asks for, blending into
/// whatever is already on the surface instead of overwriting it. This is what gives the
/// progress bar its soft leading edge.
/// </summary>
/// <param name="progress">The fill position, as a 16.16 fraction of the rectangle width.</param>
void ODDrawGradientRect(Rect const & rect, Surface & surface, int color, int progress)
{
	int fade = 1;
	int run = (rect.Width * progress) >> 16;

	if (run < 0) return;
	if (run == 0) run = 1;

	unsigned short * surfptr = (unsigned short*)surface.Lock();
	if (!surfptr) return;

	for (int row = 0; row < rect.Height; row++) {
		int idx = rect.X + (rect.Y + row) * (surface.Stride() / 2);

		int q_div4 = rect.Height / 4;	/// a quarter of the height, rounded down
		if (row == (q_div4 * 3)) {
			fade = 1;
		}
		if (row == q_div4) {
			fade = 0;
		}

		if (run > 0) {
			unsigned short * pixptr = &surfptr[idx];
			for (int i = 0; i < run; i++) {
				if (fade) {
					pixptr[i] = OD_Blend_Color(pixptr[i], color, (255 * (i + 1)) / rect.Width);
				} else {
					pixptr[i] = color;
				}
			}
		}
	}

	surface.Unlock();
}


/// <summary>
/// Darkens the left and top edges of a rectangle.
/// This routine halves the intensity of every pixel it covers, which is what gives the
/// tooltip frame its shadowed bevel.
/// </summary>
/// <param name="xpos">The width of the darkened band down the left edge.</param>
/// <param name="ypos">The height of the darkened band across the top edge.</param>
void ODDrawBevelDarken(Rect const & rect, Surface & surface, int xpos, int ypos)
{
	unsigned short * surfptr = (unsigned short *)surface.Lock();

	if (surfptr != NULL) {

		int stride = surface.Stride() / 2;

		/*
		 * The half-intensity mask is respelled at each store with per-component
		 * drift: the green term reuses one load of its mask, the red term's
		 * second read carries a cast (defeating the load reuse), and the blue
		 * term is missing the "& mask" entirely (harmless, since blue is the
		 * low bit field).
		 * NOTE: this is NOT OD_Blend_Color(px, 0, ...) - the blend helper goes
		 * through the alpha statics with multiplies and a >>8 (visible at the
		 * real blend-to-black sites); the binary here has only the shift/mask
		 * arithmetic, and the per-component drift proves it was hand-written.
		 */
		int y;
		for (y = rect.Y; y < rect.Y + rect.Height; ++y) {
			int index = y * stride + rect.X;
			for (int x = rect.X; x < rect.X + xpos; ++x) {
				unsigned short px = surfptr[index];

				surfptr[index] = (px >> 1) & (((ODRComponentMask >> 1) & (unsigned int)ODRComponentMask) | ((ODGComponentMask >> 1) & ODGComponentMask) | (ODBComponentMask >> 1));
				index++;
			}
		}

		for (y = rect.Y; y < rect.Y + ypos; ++y) {
			int index = y * stride + rect.X + xpos;
			for (int x = rect.X + xpos; x < rect.Width + rect.X; ++x) {
				unsigned short px = surfptr[index];

				surfptr[index] = (px >> 1) & (((ODRComponentMask >> 1) & (unsigned int)ODRComponentMask) | ((ODGComponentMask >> 1) & ODGComponentMask) | (ODBComponentMask >> 1));
				index++;
			}
		}

		surface.Unlock();
	}
}


/// <summary>
/// Fills a rectangle with a translucent color.
/// Each pixel of the area is blended toward the color given rather than replaced by it,
/// so whatever was already drawn there still shows through.
/// </summary>
/// <param name="trans">The strength of the blend, from 0 for invisible to 255 for solid.</param>
void ODFillRectTrans(Rect const & rect, Surface & surf, int color, int trans)
{
	unsigned short * surfptr = (unsigned short *)surf.Lock();

	if (surfptr != NULL) {

		int strideWords = surf.Stride() / 2;

		for (int y = rect.Y; y < rect.Height + rect.Y; y++) {
			int rowIndex = strideWords * y;

			for (int x = rect.X; x < rect.X + rect.Width; x++) {
				surfptr[rowIndex + x] = OD_Blend_Color(surfptr[rowIndex + x], color, trans);
			}

			rowIndex += strideWords;
		}

		surf.Unlock();
	}
}


/// <summary>
/// Draws the decorative bit strand along a rectangle.
/// This routine scatters the small "bits" graphics along the top and bottom edges of the
/// area, picking a random variant for each one so that no two dialogs look quite alike.
/// </summary>
/// <param name="surface">The surface to draw the strand upon.</param>
void ODDrawBitsStrand(Rect const & rect, Surface & surface)
{
	unsigned char paldata[768];
	Surface *img = SurfaceCache.GetSurface("bits_i.pcx", paldata);
	Surface *mask = SurfaceCache.GetSurface("bits_a.pcx", NULL);
	int index = 0;
	SurfaceCacheConvertPalette(paldata);
	Rect work = rect;
	int rnd;

	for (int x = 0; x < rect.Width; x += 10) {

		rnd = rand() % 8;
		work.Set(rect.X+x, rect.Y+1, 10, 12);
		SurfaceCache.DrawMasked(work, surface, *img, *mask, paldata, 0, 10 * rnd, 0);

		rnd = rand() % 8;
		work.Set(rect.X+x, rect.Y+rect.Height-16, 10, 12);
		SurfaceCache.DrawMasked(work, surface, *img, *mask, paldata, 0, 10 * rnd, 0);

		index++;

		if ((index % 6) == 0) {
			x += 10;
		}
	}
}


/// <summary>
/// Draws the backdrop of an owner-draw dialog.
/// The first call composes the whole backdrop -- wallpaper, side bars, corner pieces and
/// the glowing border -- onto a surface cached on the dialog. Every later call simply
/// blits that cached surface, which is what makes repainting a dialog cheap.
/// </summary>
void OwnerDraw::Draw_Dialog_Back(HWND window)
{
	WinData * entry = NULL;
	PaletteClass rgb;

	/// Find dictionary entry for this dialog (must exist by invariant set during WM_INITDIALOG).
	if (ODWinData.getEntries() != 0) {
		ODWinData.getPointer(window, &entry);
	}

	/*
	 * Compute client rect and display rect; expand to max of each dimension.
	 */
	RECT rcClient;
	::GetClientRect(window, &rcClient);

	RECT rcDisp;
	Get_Display_Rect(window, &rcDisp);

	Rect rFull;
	rFull.Set(rcDisp.left, rcDisp.top, rcDisp.right - rcDisp.left, rcDisp.bottom - rcDisp.top);

	if (rFull.Width <= rcClient.right) {
		rFull.Width = rcClient.right;
	}
	if (rFull.Height <= rcClient.bottom) {
		rFull.Height = rcClient.bottom;
	}

	Surface * surf = entry->cachedSurface;
	if (surf == NULL) {
		surf = new BSurface(rcClient.right, rcClient.bottom, 2);
		entry->cachedSurface = surf;
		++_surface_count;

		Surface * back = SurfaceCache.GetSurface("dbak6440.pcx");

		Rect dst = rFull;
		Rect src = rFull;

		dst -= Point2D(rcDisp.left, rcDisp.top);

		if (VideoModeWidth > back->Get_Width()) {
			src.X += (VideoModeWidth - back->Get_Width()) / -2;
		}
		if (VideoModeHeight > back->Get_Height()) {
			src.Y += (VideoModeHeight - back->Get_Height()) / -2;
		}

		surf->Blit_From(dst, *back, src);

		/// Side bars
		Surface * leftbar  = SurfaceCache.GetSurface("leftbar.pcx");
		Surface * rightbar = SurfaceCache.GetSurface("rightbar.pcx");

		Rect work;
		if (leftbar != 0) {
			work = rFull;
			work.Width = leftbar->Get_Width();
			work.X = 0; work.Y = 0;
			SurfaceCache.Draw(work, *surf, *leftbar);
		}
		if (rightbar != 0) {
			work = rFull;
			work.X = work.Width - rightbar->Get_Width();
			work.Y = 0;
			work.Width = rightbar->Get_Width();
			SurfaceCache.Draw(work, *surf, *rightbar);
		}

		Surface * bar_ul = SurfaceCache.GetSurface("bar_ul.pcx");
		Rect srcCorner;
		srcCorner.Y = 0;
		srcCorner.X = 0;
		srcCorner.Width = bar_ul->Get_Width();
		srcCorner.Height = bar_ul->Get_Height();

		/// dst rect used for each corner
		Rect dstCorner;
		dstCorner.X = 0;
		dstCorner.Y = 0;
		dstCorner.Width = srcCorner.Width;
		dstCorner.Height = srcCorner.Height;

		surf->Blit_From(dstCorner, *bar_ul, srcCorner);

		Surface * bar_ll = SurfaceCache.GetSurface("bar_ll.pcx");
		dstCorner.Y = rFull.Height - srcCorner.Height;
		surf->Blit_From(dstCorner, *bar_ll, srcCorner);

		Surface * bar_ur = SurfaceCache.GetSurface("bar_ur.pcx");
		dstCorner.Y = 0;
		dstCorner.X = rFull.Width - srcCorner.Width;
		surf->Blit_From(dstCorner, *bar_ur, srcCorner);

		Surface * bar_lr = SurfaceCache.GetSurface("bar_lr.pcx");
		dstCorner.Y = rFull.Height - srcCorner.Height;
		surf->Blit_From(dstCorner, *bar_lr, srcCorner);

		// Edge glow loops (layers 0..15)
		for (int layer = 0; layer < 16; ++layer) {

			/*
			 * Top edge between left and right bars. The bar widths are re-fetched
			 * at every use - Get_Width is virtual, so the four calls per pass
			 * cannot be a cached local.
			 */
			Point2D p2(layer + leftbar->Get_Width(), layer);
			Point2D p3(rFull.Width - layer - rightbar->Get_Width() - 1, layer);

			/// The alpha fades from 96 down to 6 as the layers move inward. The
			/// parameter is an unsigned char, so the whole thing is computed in
			/// 8-bit arithmetic and homed as a byte.
			ODDrawEdgeGlow(*surf, p2, p3, 0xFFFF, 96 - 6 * layer);

			// Bottom edge
			p3.Y = rFull.Height - layer - 1;
			p2.Y = p3.Y;
			ODDrawEdgeGlow(*surf, p2, p3, 0xFFFF, 96 - 6 * layer);

			/// Left vertical
			p2.X = layer + leftbar->Get_Width();
			p3.X = p2.X;
			p2.Y = layer + 1;
			p3.Y = rFull.Height - layer - 2;
			ODDrawEdgeGlow(*surf, p2, p3, 0xFFFF, 96 - 6 * layer);

			/// Right vertical
			p2.X = rFull.Width - layer - rightbar->Get_Width() - 1;
			p3.X = p2.X;
			ODDrawEdgeGlow(*surf, p2, p3, 0xFFFF, 96 - 6 * layer);
		}
	}

	/// Final blit to AlternateSurface unless flagged as "captured" (UserData2 != 0)
	if (surf != NULL) {
		if (entry->paintDisabled == 0) {
			Rect srcOnXs = rFull;
			srcOnXs -= Point2D(rcDisp.left, rcDisp.top);
			AlternateSurface->Blit_From(rFull, *surf, srcOnXs);
		}
	}
}


/// <summary>
/// Frees the cached background of a window that is going away.
/// Owner-draw windows keep the picture of whatever sits behind them on a cached surface.
/// This routine hands that surface back as the window is destroyed.
/// </summary>
void On_WM_NCDESTROY(HWND window)
{
	WinData *data;
	if (ODWinData.getPointer(window, &data)) {
		if (data != NULL && data->cachedSurface != NULL) {
			delete data->cachedSurface;
			data->cachedSurface = NULL;
			_surface_count--;
		}
	}
}


/// <summary>
/// Repaints part of a window right away.
/// The area is marked as needing painting and the paint is forced through, rather than
/// waiting for Windows to get round to it.
/// </summary>
/// <param name="rect">The area of the window to repaint, or NULL for the whole window.</param>
int WINAPI ODUpdateWindowRect(HWND window, RECT *rect)
{
	InvalidateRect(window, rect, FALSE);
	UpdateWindow(window);
	return(1);
}


/// <summary>
/// Adds a window to a list of windows.
/// This routine is the enumeration callback used when a caller needs a snapshot of the
/// child windows of a dialog.
/// </summary>
/// <param name="list">The list to append the window to.</param>
BOOL CALLBACK ODAddWindowToList(HWND window, ArrayList<HWND> * list)
{
	if (list != NULL) {
		list->add(window, list->length());
	}
	return(TRUE);
}


/// <summary>
/// Takes the mouse away from the game so that a dialog may use it.
/// The game cursor gives up its capture, leaving Windows free to drive the dialog and its
/// controls.
/// </summary>
/// <returns>Returns with the number of captures now outstanding.</returns>
/// <remarks>Each call must be matched by a call to Release_Mouse.</remarks>
int OwnerDraw::Capture_Mouse(void)
{
	if (MouseCursor != NULL) {
		if (MouseCursor->Is_Captured() == true) {
			MouseCursor->Release_Mouse();
		}
	}
	_mouse_counter++;
	return(_mouse_counter);
}


/// <summary>
/// Gives the mouse back to the game.
/// This routine undoes one Capture_Mouse. Only when the last dialog has finished with the
/// mouse does the game cursor take it back.
/// </summary>
/// <returns>Returns with the number of captures still outstanding.</returns>
int OwnerDraw::Release_Mouse(void)
{
	if (_mouse_counter > 0) {
		_mouse_counter--;
	}
	if (_mouse_counter == 0) {
		if (MouseCursor != NULL) {
			if (!MouseCursor->Is_Captured()) {
				MouseCursor->Capture_Mouse();
			}
		}
	}
	return(_mouse_counter);
}


/// <summary>
/// Creates a modeless dialog from a resource template.
/// This routine fetches the dialog template out of the game resources, creates the dialog
/// and makes it the top window of the dialog stack. The mouse is captured for it.
/// </summary>
/// <param name="id">The resource ID of the dialog template to create.</param>
/// <param name="proc">The dialog procedure that will drive the dialog.</param>
/// <returns>Returns with the handle of the new dialog, or NULL if it could not be
/// created.</returns>
/// <remarks>Every dialog begun with this routine must be finished with End_Dialog.</remarks>
HWND OwnerDraw::Begin_Dialog(int id, DLGPROC proc)
{
	LPCDLGTEMPLATE templ = (LPCDLGTEMPLATE)Fetch_Resource((LPCSTR)(int)MAKEINTRESOURCE(id), (LPCSTR)RT_DIALOG);
	if (templ == NULL) {
		return(NULL);
	}

	int idx = g_DialogCount;
	g_Dialogs[idx].handle = NULL;
	g_Dialogs[idx].id = 0;
	g_DialogCount++;

	HWND handle = CreateDialogIndirectParam(ProgramInstance, templ, MainWindow, proc, 0);
	if (handle == NULL) {
		g_DialogCount--;
		return(NULL);
	}

	g_Dialogs[idx].handle = handle;
	g_Dialogs[idx].id = (int)MAKEINTRESOURCE(id);

	Capture_Mouse();

	Add_Modeless_Dialog(handle);

	g_TopWindow = handle;
	g_TopWindowID = (int)MAKEINTRESOURCE(id);

	return(handle);
}


/// <summary>
/// Shuts down a dialog and forgets about it.
/// This routine destroys the window and takes it off the stack of open dialogs, handing
/// the focus back to whichever dialog was underneath it -- or to the main game window once
/// the last dialog has gone. The mouse capture taken by Begin_Dialog is given back here.
/// </summary>
void OwnerDraw::End_Dialog(HWND window)
{
	Keyboard->Clear();
	DestroyWindow(window);

	for (int index = 0; index < g_DialogCount; index++) {
		if (g_Dialogs[index].handle == window) {
			memmove(&g_Dialogs[index], &g_Dialogs[index + 1], sizeof(WSDialogStruct) * (g_DialogCount - (index + 1)));

			g_Dialogs[g_DialogCount - 1].handle = NULL;
			g_Dialogs[g_DialogCount - 1].id	 = 0;

			--g_DialogCount;

			if (g_DialogCount > 0) {
				g_TopWindow	= g_Dialogs[g_DialogCount - 1].handle;
				g_TopWindowID = g_Dialogs[g_DialogCount - 1].id;

				SetForegroundWindow(g_TopWindow);
				SetFocus(g_TopWindow);
			} else {
				g_TopWindow	= NULL;
				g_TopWindowID = 0;

				SetForegroundWindow(MainWindow);
				SetFocus(MainWindow);
			}
			break;
		}
	}

	UpdateWindow(MainWindow);
	Release_Mouse();
}


/// <summary>
/// Displays a dialog that has already been created.
/// This routine makes the dialog visible, brings it to the front and flushes the game
/// keyboard so that no stale keystrokes leak into it.
/// </summary>
void OwnerDraw::Display_Dialog(HWND window)
{
	ShowWindow(window, SW_SHOWNORMAL);
	SetForegroundWindow(window);
	Keyboard->Clear();
}


/// <summary>
/// Handles the messages common to every owner-draw dialog.
/// A dialog procedure hands its messages to this routine first and deals with them itself
/// only when they come back unclaimed. This is where the subclassing, centering,
/// background painting and control coloring that all dialogs share is performed.
/// </summary>
/// <returns>Returns with the message result, or zero if the caller should handle it.</returns>
int OwnerDraw::Default_Dialog_Proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	switch (message) {
		case WM_DRAWITEM:
			Draw_Item((DRAWITEMSTRUCT*)lparam);
			return(1);

		case WM_DESTROY:
			Remove_Modeless_Dialog(window);
			--_dialog_count;
			SetFocus(MainWindow);
			return(0);

		case WM_PAINT:
			Draw_Dialog_Back(window);
			ValidateRect(window, 0);
			return(0);

		case WM_ERASEBKGND:
			return(1);

		case WM_INITDIALOG:
			++_dialog_count;
			Subclass_Dialog(window, 0);
			Resize_Dialogs(window);
			Center_Window_Within_Window(window);
			SetFocus(window);
			return(0);

		case WM_CTLCOLORMSGBOX:
		case WM_CTLCOLOREDIT:
		case WM_CTLCOLORLISTBOX:
		case WM_CTLCOLORBTN:
		case WM_CTLCOLORDLG:
		case WM_CTLCOLORSCROLLBAR:
		case WM_CTLCOLORSTATIC:
			return((int)GetStockObject(BLACK_BRUSH));

		case OD_SUBCLASSED:
			SendMessage(window, OD_SETTOP, (WPARAM)window, 1);
			return(0);

		default:
			return(0);
	}
}


/// <summary>
/// Services the game while a dialog is up.
/// A dialog's own message pump calls this routine every pass. It dispatches the pending
/// Windows messages and then either runs the game logic loop -- so that a multiplayer
/// game keeps up while the dialog is showing -- or just the maintenance callback.
/// </summary>
/// <returns>bool; Has the game ended, so that the dialog should be shut down?</returns>
bool OwnerDraw::Dialog_Message_Handler(void)
{
	static bool inmainloop = false;

	Windows_Message_Handler();

	if (Session.Type != GAME_NORMAL && Session.Type != GAME_SKIRMISH && !Session.NetOpen && !Session.Suspended) {
		if (!inmainloop) {
			inmainloop = true;
			bool end = Main_Loop();
			inmainloop = false;
			if (end) {
				return(true);
			}
		}
	} else {
		Call_Back();
	}

	return(false);
}


/// <summary>
/// Moves a dialog to the position specified.
/// The position is relative to the client area of the main game window rather than to the
/// desktop, and the dialog keeps its current size.
/// </summary>
/// <param name="x">The horizontal position to move to, or -1 to leave it where it is.</param>
/// <param name="y">The vertical position to move to, or -1 to leave it where it is.</param>
/// <returns>Returns with non-zero if the dialog was moved.</returns>
int OwnerDraw::Move_Dialog(HWND window, int x, int y)
{
	int xpos;
	int ypos;

	RECT rect1;
	rect1.left = 0;
	rect1.top = 0;
	rect1.right = VideoModeWidth;
	rect1.bottom = VideoModeHeight;

	ClientToScreen(MainWindow, (LPPOINT)&rect1);
	ClientToScreen(MainWindow, (LPPOINT)&rect1.right);

	RECT rect2;
	GetWindowRect(window, &rect2);

	rect2.right -= rect2.left;
	rect2.bottom -= rect2.top;

	if (x == -1) {
		xpos = rect2.left - rect1.left;
	} else {
		xpos = x;
	}
	rect2.left = xpos;

	if (y == -1) {
		ypos = rect2.top - rect1.top;
	} else {
		ypos = y;
	}
	rect2.top = ypos;

	return(MoveWindow(window, rect2.left, rect2.top, rect2.right, rect2.bottom, FALSE));
}


/// <summary>
/// Creates the custom message box dialog.
/// This routine brings the modeless message box up, captures the mouse and makes the box
/// the top window. The second button stays hidden unless a caption is supplied for it.
/// </summary>
/// <param name="btn1txt">The message text to show in the box.</param>
/// <param name="btn2txt">The caption for the cancel button, or NULL to leave it hidden.</param>
/// <param name="cancelled">Flag to be raised if the player cancels the box.</param>
/// <returns>Returns with the handle of the message box, or NULL if it could not be
/// created.</returns>
HWND OwnerDraw::Custom_Message_Box(const char *btn1txt, const char *btn2txt, bool * cancelled)
{
	HWND dlg = OwnerDraw::Begin_Dialog(IDD_MSGBOX_1, (DLGPROC)Custom_Message_Box_Proc);

	SetWindowLong(dlg, 8, (LONG)cancelled);
	SetDlgItemText(dlg, IDC_MSGBOX_TEXT, btn1txt);

	if (btn2txt) {
		HWND handle = GetDlgItem(dlg, IDCANCEL);
		SetWindowText(handle, btn2txt);
		EnableWindow(handle, TRUE);
		ShowWindow(handle, SW_SHOW);
	}

	return(dlg);
}


/// <summary>
/// Handles the messages for a custom message box.
/// This routine lets the default dialog handler have first refusal of the message and
/// then watches for the cancel button. Cancelling feeds an escape key to the game
/// keyboard and raises the flag the box was opened with.
/// </summary>
/// <returns>Returns with the message result, or zero when the message was consumed here.</returns>
LRESULT CALLBACK Custom_Message_Box_Proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	LRESULT res = Default_Dialog_Proc(window, message, wparam, lparam);

	if (res == 0) {
		if (message == WM_COMMAND && wparam == IDCANCEL) {
			bool * cancelled = (bool *)GetWindowLong(window, DWL_USER);
			if (cancelled) {
				Keyboard->Put(KN_ESC);
				*cancelled = true;
			}
		}
		return(0);
	}
	return(res);
}


/// <summary>
/// Sets the text displayed by a custom message box.
/// Use this routine to change the prompt of a message box that is already on the screen.
/// The box is repainted before the routine returns.
/// </summary>
void OwnerDraw::Set_Custom_Message_Box_Text(HWND window, LPCSTR text)
{
	SetDlgItemText(window, IDC_MSGBOX_TEXT, text);
	UpdateWindow(window);
}
