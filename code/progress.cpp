/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "always.h"

#include "progress.h"

#include "_convert.h"
#include "_mixfile.h"
#include "_surface.h"
#include "convert.h"
#include "data.h"
#include "dialog.h"
#include "draw.h"
#include "gscreen.h"
#include "init.h"
#include "language/language.h"
#include "lightcon.h"
#include "mixfile.h"
#include "ownrdraw.h"
#include "scheme.h"
#include "session.h"
#include "shapeset.h"
#include "surface.h"
#include "voc.h"
#include "windlg.h"

#include <algorithm>


ProgressScreenClass Progress;


/// <summary>
/// Constructs the progress screen object.
/// The screen starts out idle and with no artwork attached. Initialize and
/// Set_Graphic_Data supply both before anything can be displayed.
/// </summary>
ProgressScreenClass::ProgressScreenClass(void)
{
	MainProgress = 0;
	String = NULL;
	Shape = NULL;
	Background = NULL;
	IsActive = false;
	for (int i = 0; i < MAX_PLAYERS; i++) {
		PlayerProgress[i] = 0;
	}
}


/// <summary>
/// Destroys the progress screen object.
/// </summary>
ProgressScreenClass::~ProgressScreenClass(void)
{
}


/// <summary>
/// Prepares the progress screen for a new job.
/// This routine records how much work the job amounts to, clears the progress of every
/// player being tracked, and then either brings up the progress dialog or clears the
/// hidden surface for the full screen presentation.
/// </summary>
/// <param name="progress">The total amount of work this job represents.</param>
/// <param name="count">The number of players whose progress must be tracked.</param>
/// <param name="usedialog">Should the dialog be used instead of the full screen?</param>
void ProgressScreenClass::Initialize(double progress, int count, bool usedialog)
{
	MainProgress = progress;
	PlayerCount = std::max(count, 1);

	for (int i = 0; i < PlayerCount; i++) {
		PlayerProgress[i] = 0;
	}
	IsActive = true;

	if (usedialog) {
		if (Dialog == NULL) {
			Begin_Dialog();
		}
	} else {
		HiddenSurface->Fill(0);
	}
	Percentage = -1;
}


/// <summary>
/// Marks the progress screen as finished.
/// This routine is used when the job is over. Further progress requests are ignored
/// until Initialize starts another one.
/// </summary>
void ProgressScreenClass::End(void)
{
	IsActive = false;
	MainProgress = 0.0;
}


/// <summary>
/// Sets up the artwork for the progress screen.
/// This routine loads the title page to sit behind the bars, fetches the progress bar
/// shape from the mix files, and prints the caption and the player names onto the hidden
/// surface so that the screen is ready before the first progress update arrives.
/// </summary>
/// <param name="progbar">Name of the progress bar shape, or NULL to keep the current one.</param>
/// <param name="background">Name of the title page to load, or NULL to keep the current
/// one.</param>
/// <param name="string">The caption to print above the bars, or NULL for none.</param>
/// <param name="pt">The center of the bars, or Point2D(-1,-1) to center them on the screen.</param>
void ProgressScreenClass::Set_Graphic_Data(const char * progbar, const char * background, const char * string, Point2D pt)
{
	Rect rect;

	String = string;
	if (background != NULL) {
		Background = background;
		Load_Title_Page(Background, false);
	}

	ColorScheme * scheme = Fetch_Scheme_By_Name("Yellow");

	if (pt.X == -1 && pt.Y == -1) {
		Pos = Point2D(HiddenSurface->Get_Width() / 2, HiddenSurface->Get_Height() / 2);
	} else {
		Pos.X = pt.X;
		Pos.Y = pt.Y;
	}

	if (progbar != NULL) {
		Shape = (ShapeSet *)MFCD::Retrieve(progbar);
		if (Shape != NULL) {
			for (int i = 0; i < PlayerCount; i++) {
				rect = Shape->Get_Rect(0);

				rect.X = (Pos.X - rect.Width / 2) - 1;
				if (PlayerCount == 1) {
					rect.Y = (Pos.Y - rect.Height / 2) - 1;
				} else {
					rect.Y = (Pos.Y - rect.Height / 2) + (i * 10) - 1;
				}
				rect.Width = rect.Width + 2;
				rect.Height = rect.Height + 2;
				if (PlayerCount == 1 && Dialog != 0) {
					HiddenSurface->Draw_Rect(rect, NormalDrawer->Convert_Pixel(15));
				}
				if (PlayerCount != 1) {
					pt.X = rect.X - 80;
					pt.Y = rect.Y;
					Fancy_Text_Print(Session.Players[i]->Name, *HiddenSurface, HiddenSurface->Get_Rect(), pt, Fetch_Scheme_By_Name("Green"), 0, TextPrintType(TPF_EFNT|TPF_NOSHADOW));
				}
			}
		}
	}

	if (String != NULL) {
		int ycenter = 0;
		if (Shape != NULL) {
			ycenter = Shape->Get_Height() / 2;
		}
		if (PlayerCount == 1) {
			pt = Point2D(HiddenSurface->Get_Width() / 2, (HiddenSurface->Get_Height() / 2) - ycenter - 20);
		} else {
			rect = Shape->Get_Rect(0);
			pt = Point2D((Pos.X - rect.Width / 2) - 1, Pos.Y - (rect.Height / 2)) + Point2D(-80, 89);
		}
		Fancy_Text_Print(String, *HiddenSurface, HiddenSurface->Get_Rect(), pt, scheme, TBLACK, TextPrintType(TPF_FULLSHADOW|TPF_6PT_GRAD));
	}
	Update_Visible_Surface();
}


/// <summary>
/// Fetches the overall progress of the job.
/// This routine averages the progress of every player being tracked, which is what the
/// network loading code watches to decide when everyone is ready.
/// </summary>
/// <returns>Returns with the progress as a fraction, where 1.0 means the job is done.</returns>
double ProgressScreenClass::Get_Current_Progress(void) const
{
	double total = 0;

	for (int i = 0; i < PlayerCount; i++) {
		total += PlayerProgress[i];
	}

	return(total / PlayerCount / MainProgress);
}


/// <summary>
/// Draws the progress screen.
/// This routine paints a progress bar for every player being tracked, and in the single
/// player case announces the next loading message as the work passes each milestone. The
/// progress percent routines and the dialog's paint handler call it.
/// </summary>
/// <param name="xpt">The screen position to draw at, or Point2D(-1,-1) to use the
/// position established by Set_Graphic_Data.</param>
/// <remarks>Nothing is drawn until Initialize has been called.</remarks>
void ProgressScreenClass::Display_Progress(Point2D xpt)
{
	static struct {
		int Progress;
		int Text;
	}  _progress_messages[MAX_PLAYERS] = {
		{ 0,	TXT_LOADING_GAME1A },
		{ 12,	TXT_LOADING_GAME1B },
		{ 20,	TXT_LOADING_GAME1C },
		{ 30,	TXT_LOADING_GAME1D },
		{ 50,	TXT_LOADING_GAME1E },
		{ 70,	TXT_LOADING_GAME1F },
		{ 80,	TXT_LOADING_GAME1G },
		{ 100,	TXT_LOADING_GAME1H }
	};

	if (IsActive) {
		Point2D pt = xpt;

		Surface *surface;
		if (Dialog == 0) {
			surface = HiddenSurface;
		} else {
			surface = AlternateSurface;
		}

		ConvertClass * drawer = NormalDrawer;
		for (int i = 0; i < PlayerCount; i++) {
			if (PlayerProgress[i] > MainProgress) {
				PlayerProgress[i] = MainProgress;
			}
			if (Shape != NULL) {
				if (pt == Point2D(-1,-1)) {
					if (PlayerCount == 1) {
						if (Dialog) {
							RECT crect;
							Get_Display_Rect(GetDlgItem(Dialog, IDC_PROGRESS_BAR_FRAME), &crect);
							pt = Point2D(crect.left + (crect.right - crect.left) / 2, crect.top + (crect.bottom - crect.top) / 2);
						} else {
							int progress = PlayerProgress[i];
							int percent = Percentage;
							if (progress > percent) {
								for (int j = 0; j < ARRAY_SIZE(_progress_messages); j++) {
									if (_progress_messages[j].Progress <= progress && _progress_messages[j].Progress > percent) {
										Fancy_Text_Print(Fetch_String(_progress_messages[j].Text), *HiddenSurface, HiddenSurface->Get_Rect(), Pos + Point2D(0, 10 * j), Fetch_Scheme_By_Name("Green"), 0, TextPrintType(TPF_NOSHADOW|TPF_EFNT));
										Sound_Effect(VocClass::From_Name("Notify"), 0.4f);
										Percentage = _progress_messages[j].Progress;
										if (surface == HiddenSurface) {
											Update_Visible_Surface();
										}
										break;
									}
								}
							}
							return;
						}
					} else {
						pt = Point2D(Pos.X, Pos.Y + (10 * i));
						drawer = ColorSchemes[Session.Color_Index_To_Scheme(Session.Players[i]->Player.Color)]->Converter;
					}
				}

				Rect rect = Shape->Get_Rect(0);
				pt.X += rect.Width / -2;
				pt.Y += rect.Height / -2;
				rect.Width = rect.Width * Get_Current_Progress(i);
				rect.X = pt.X;
				rect.Y = pt.Y;
				Draw_Shape(
					*surface,
					*drawer,
					Shape,
					0,
					Point2D(0,0),
					rect,
					SHAPE_WIN_REL
				);
			}
			pt = Point2D(-1,-1);
		}

		if (surface == HiddenSurface) {
			Update_Visible_Surface();
		}
	}
}


/// <summary>
/// Fetches the width of the progress bar artwork.
/// </summary>
/// <returns>Returns with the width of the progress bar shape, in pixels.</returns>
/// <remarks>The progress bar shape must have been attached by Set_Graphic_Data first.</remarks>
int ProgressScreenClass::Get_Bar_Width(void) const
{
	return(Shape->Get_Rect(0).Width);
}


/// <summary>
/// Sets a player's progress to a percentage of the job.
/// This routine is used when the loader knows how far along it is rather than how much
/// it has just accomplished. The display is only refreshed when the progress changed.
/// </summary>
/// <param name="index">The player slot whose progress should be set.</param>
/// <param name="value">The percentage of the whole job that has been completed.</param>
/// <param name="pt">The screen position to draw at, or Point2D(-1,-1) for the default.</param>
void ProgressScreenClass::Set_Progress_Percent(int index, double value, Point2D pt)
{
	double prog1 = PlayerProgress[index];
	PlayerProgress[index] = (MainProgress / 100.0) * value;

	if (PlayerProgress[index] != prog1) {
		if (Dialog != NULL) {
			SendMessage(Dialog, WM_PAINT, 0, 0);
		} else {
			Display_Progress(pt);
		}
	}
}


/// <summary>
/// Advances a player's progress by a percentage of the job.
/// This routine is used by the loading code as each stage of its work completes. The
/// display is only refreshed when the progress actually moved.
/// </summary>
/// <param name="index">The player slot whose progress should be advanced.</param>
/// <param name="value">The percentage of the whole job that has just been completed.</param>
/// <param name="pt">The screen position to draw at, or Point2D(-1,-1) for the default.</param>
void ProgressScreenClass::Add_Progress_Percent(int index, double value, Point2D pt)
{
	double prog1 = PlayerProgress[index];
	PlayerProgress[index] += (MainProgress / 100.0) * value;

	if (PlayerProgress[index] != prog1) {
		if (Dialog != NULL) {
			SendMessage(Dialog, WM_PAINT, 0, 0);
		} else {
			Display_Progress(pt);
		}
	}
}


/// <summary>
/// Creates the progress dialog.
/// This routine brings up the owner draw progress dialog and gives it its first
/// paint. Initialize() calls it when the caller asks for the dialog presentation
/// rather than the full screen one.
/// </summary>
void ProgressScreenClass::Begin_Dialog(void)
{
	Dialog = OwnerDraw::Begin_Dialog(IDD_PROGRESS_WAIT, ProgressScreenClass::Dialog_Proc);
	if (Dialog != NULL) {
		SetWindowLong(Dialog, DWL_USER, (LONG)this);
		OwnerDraw::Display_Dialog(Dialog);
		SendMessage(Dialog, WM_PAINT, 0, 0);
	}
}


/// <summary>
/// Takes down the progress dialog.
/// This routine is used when the progress screen is finished with the dialog
/// presentation. It is harmless to call when no dialog was ever created.
/// </summary>
void ProgressScreenClass::End_Dialog(void)
{
	if (Dialog != NULL) {
		OwnerDraw::End_Dialog(Dialog);
		Dialog = NULL;
	}
}


/// <summary>
/// Handles the messages sent to the progress dialog.
/// This routine gives the owner draw default dialog procedure first refusal on every
/// message, and repaints the progress display itself when a paint request comes back
/// unhandled.
/// </summary>
/// <returns>Returns with the dialog result, zero if the message was left unhandled.</returns>
BOOL CALLBACK ProgressScreenClass::Dialog_Proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	HRESULT res = OwnerDraw::Default_Dialog_Proc(window, message, wparam, lparam);
	if (res == 0) {
		if (message == WM_PAINT) {
			ProgressScreenClass *screen = (ProgressScreenClass *)GetWindowLong(window, DWL_USER);
			screen->Display_Progress();
		}
		res = 0;
	}
	return(res);
}
