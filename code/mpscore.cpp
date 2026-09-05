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

#include "mpscore.h"

#include "_keyboar.h"
#include "_palette.h"
#include "_surface.h"
#include "ccrand.h"
#include "conquer.h"
#include "data.h"
#include "dbgprint.h"
#include "dsurface.h"
#include "globals.h"
#include "houstype.h"
#include "incdec.h"
#include "keyboard.h"
#include "language/language.h"
#include "msanim.h"
#include "msengine.h"
#include "msfont.h"
#include "scheme.h"
#include "session.h"
#include "stats.h"
#include "surface.h"
#include "windlg.h"
#include "winstub.h"

#include "color.hh"

#include <algorithm>
#include <cstdio>


class MultiScore : public MSEngine
{
	public:
		MultiScore(void);
		virtual ~MultiScore(void);

		virtual void Do_Custom_Draw(Surface * surface);
		virtual void Process_Idle(void);

		bool Init(void);
		void Deinit(void);

		bool User_Input(void);
		void Tally_Score(void);
		void Process_Scores(void);

		void Print_Headings(void);
		void Print_Player_Names(void);
		void Draw_Losses(void);
		void Draw_Kills(void);
		void Draw_Economy(void);
		void Print_Scores(void);
		void Draw_Bars(int * scores, int numScores, int x, int y, int width);
		Rect Print_Score(Surface * surface, int score, int maximum, int x, int y);

		void Callback(void);
		bool Single_Presentation(HouseClass *house);
		bool Multi_Presentation(void);

	private:
		int XPos;
		int YPos;
		Surface * ScoreSurface;
		MSFont * Font;
		/// Unused
		MSFont * UnusedFont;
		MPlayerScoreType * Scores[MAX_PLAYERS];

};


static int _cdecl Sort_Scores(const void *p1, const void *p2);


/// <summary>
/// Determines if the specified house won the round.
/// This routine is used where only the outcome is wanted and no score screen is to be
/// presented.
/// </summary>
/// <param name="house">The house to test for the win.</param>
/// <returns>bool; Did this house come out on top?</returns>
bool Single_Score_Presentation(HouseClass *house)
{
	return(MultiScore().Single_Presentation(house));
}


/// <summary>
/// Presents the multiplayer score screen.
/// This routine is called when a multiplayer game has finished, in order to show the
/// players how the round went.
/// </summary>
void Multi_Score_Presentation(void)
{
	MultiScore().Multi_Presentation();
	Keyboard->Clear();
}


/// <summary>
/// Constructs an idle multiplayer score screen.
/// The screen holds no resources until Init is called on it.
/// </summary>
MultiScore::MultiScore(void) :
	MSEngine(),
	XPos(0),
	YPos(0),
	ScoreSurface(NULL),
	Font(NULL),
	UnusedFont(NULL)
{

}


/// <summary>
/// Destroys the score screen, releasing everything it built.
/// </summary>
MultiScore::~MultiScore(void)
{
	Deinit();
}


/// <summary>
/// Determines if the specified house finished the round in the lead.
/// Only houses that actually take part in the game are considered, so a neutral or
/// spectating house can never take the lead away from a real player.
/// </summary>
/// <param name="house">The house to test for the lead.</param>
/// <returns>bool; Did this house come out on top?</returns>
/// <remarks>This routine tallies the round's scores as a side effect.</remarks>
bool MultiScore::Single_Presentation(HouseClass * house)
{
	Tally_Score();

	int bestScore = 0;
	int bestIndex = -1;

	for (int i = 0; i < Houses.Count(); i++) {
		if (Houses[i] != NULL && !Houses[i]->Class->IsMultiplayPassive && !Houses[i]->IsObserver) {
			if (bestIndex == -1 || bestScore < Session.Score[i].Score[0]) {
				bestScore = Session.Score[i].Score[0];
				bestIndex = i;
			}
		}
	}

	if (bestIndex != -1 && Houses[bestIndex] == house) {
		return(true);
	}
	return(false);
}


/// <summary>
/// Handles the multiplayer score screen.
/// This routine runs the whole end of game presentation. It tears down any dialog still
/// on screen, tallies and ranks the round's scores, animates each column into place, and
/// then waits for the player before restoring the display and the mouse.
/// </summary>
/// <returns>bool; Was the presentation run? It is refused if there is no surface to
/// present on.</returns>
bool MultiScore::Multi_Presentation(void)
{
	if (AlternateSurface == NULL || HiddenSurface == NULL) {
		return(false);
	}

	while (WS_Destroy_Dialog(0, 0)) { }

	if (Init() == true) {
		Keyboard->Clear();
		Callback();
		MouseCursor->Release_Mouse();
		MouseCursor->Hide_Mouse();
		Blit_Rect(HiddenSurface, HiddenSurface->Get_Rect());

		Tally_Score();
		Process_Scores();
		Print_Headings();
		Print_Player_Names();
		Draw_Losses();
		Draw_Kills();
		Draw_Economy();
		Print_Scores();

		MouseCursor->Show_Mouse();
		User_Input();
		MouseCursor->Hide_Mouse();

		HiddenSurface->Fill(TBLACK);
		Add_Update_Rect(HiddenSurface->Get_Rect());
		Blit_All(HiddenSurface);

		Deinit();
		Keyboard->Clear();
		MouseCursor->Show_Mouse();
		MouseCursor->Capture_Mouse();
	}
	return(true);
}


/// <summary>
/// Prepares the score screen for display.
/// This routine builds the offscreen surface the presentation composes into, loads the
/// score screen artwork onto it and onto the visible surfaces, creates the font, and
/// registers the sound effect that the counters tick with.
/// </summary>
/// <returns>bool; Was the screen prepared successfully?</returns>
bool MultiScore::Init(void)
{
	Deinit();

	ScoreSurface = new DSurface(640, 400);
	if (ScoreSurface == NULL) {
		DebugString("MultiScore: Failed to create surface!\n");
		return(false);
	}

	XPos = ((HiddenSurface->Get_Width() - ScoreSurface->Get_Width()) / 2);
	YPos = ((HiddenSurface->Get_Height() - ScoreSurface->Get_Height()) / 2);

	AlternateSurface->Fill(TBLACK);
	Load_Title_Screen("MPSCORE.PCX", AlternateSurface, &CCPalette);
	HiddenSurface->Fill(TBLACK);
	HiddenSurface->Blit_From(*AlternateSurface, false, true);
	ScoreSurface->Blit_From(ScoreSurface->Get_Rect(), *AlternateSurface, Rect(XPos, YPos, ScoreSurface->Get_Width(), ScoreSurface->Get_Height()), false, true);

	Font = new MSFont(false);
	if (Font == NULL) {
		DebugString("MultiScore: Unable to create font!\n");
		return(false);
	}

	Add_Sound_Effect("Graph", "TEXT1.AUD");

	return(true);
}


/// <summary>
/// Releases the resources the score screen allocated.
/// This routine is called by Init before it builds the screen, and again when the
/// presentation is over. It is safe to call at any time.
/// </summary>
void MultiScore::Deinit(void)
{
	if (Font != NULL) {
		delete Font;
		Font = NULL;
	}
	if (UnusedFont != NULL) {
		delete UnusedFont;
		UnusedFont = NULL;
	}
	if (ScoreSurface != NULL) {
		delete ScoreSurface;
		ScoreSurface = NULL;
	}
}


/// <summary>
/// Handles the custom drawing pass of the presentation.
/// The engine offers every animation frame to the derived screen through this routine.
/// The score screen paints itself through its own routines instead, so there is
/// nothing to add here.
/// </summary>
void MultiScore::Do_Custom_Draw(Surface * surface)
{

}


/// <summary>
/// Waits for the player to dismiss the score screen.
/// A prompt is animated in along the bottom of the screen and this routine then sits
/// there until the player clicks or presses escape or space.
/// </summary>
bool MultiScore::User_Input(void)
{
	int key = VK_NONE;
	bool running = true;

	char const * text = Fetch_String(TXT_CLICK_CONTINUE);
	int x = 320 - Font->Get_String_Width(text) / 2;

	MSWordAnim * anim = new MSWordAnim(text, XPos + x, YPos + 370, Font);
	Add_Animation(anim);
	Wait_For_Anim(anim);

	Font->Draw_String(ScoreSurface, (unsigned char const *)text, x, 370, 2);
	Font->Draw_String(AlternateSurface, (unsigned char const *)text, XPos + x, YPos + 370, 2);

	Keyboard->Clear();

	while (running == true) {
		Wait_For_Focus();
		if (Keyboard->Check()) {
			key = Keyboard->Get();
		}
		switch (key) {
			case VK_LBUTTON:
			case VK_ESCAPE:
			case VK_SPACE:
				running = false;
				break;
		}
		Wait_Delay(1);
	}

	Keyboard->Clear();
	return(true);
}


/***************************************************************************
 * HouseClass::Tally_Score -- Fills in the score system for this round     *
 *                                                                         *
 * INPUT:                                                                  *
 *      none.                                                              *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      none.                                                              *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   11/29/1995 BRR : Created.                                             *
 *=========================================================================*/
void MultiScore::Tally_Score(void)
{
	HousesType house;
	HouseClass * hptr;
	HouseClass * hptr2;
	int score_index;
	int i;

	Session.NumScores = 0;

	/*
	 * The economy column measures each house against whoever spent most, so that figure is
	 * wanted before any house is scored.
	 */
	unsigned most_spent = 0;

	for (house = HOUSE_FIRST; house < Houses.Count(); house++) {
		hptr = Houses[house];

		if (hptr != NULL && !hptr->Class->IsMultiplayPassive && !hptr->IsObserver) {
			most_spent = std::max(most_spent, hptr->CreditsSpent);
		}
	}

	/*
	**	Loop through all houses, tallying up each player's score
	*/
	for (house = HOUSE_FIRST; house < Houses.Count(); house++) {
		hptr = Houses[house];

		/*
		**	Skip this house if it's not human.
		*/
		if (!hptr || hptr->Class->IsMultiplayPassive == true || hptr->IsObserver) {
			continue;
		}

		/*
		**	Just add this player to the end of the array, if there's room
		*/
		score_index = Session.NumScores++;

		/*
		**	Initialize this new score entry
		*/
		Session.Score[score_index].Wins = 0;
		strncpy(Session.Score[score_index].Name, hptr->IniName, MPLAYER_NAME_MAX);
		Session.Score[score_index].Name[MPLAYER_NAME_MAX - 1] = '\0';

		/*
		**	Init this player's Kills to 0 (-1 means he didn't play this round;
		**	0 means he played but got no kills).
		*/
		Session.Score[score_index].Lost[0] = 0;
		Session.Score[score_index].Kills[0] = 0;
		Session.Score[score_index].Built[0] = 0;
		Session.Score[score_index].Score[0] = 0;

		/*
		**	Init this player's color to his last-used color index
		*/
		Session.Score[score_index].Scheme = hptr->Scheme;

		/*
		**	If this house was undefeated, it must have been the winner.
		** (If no human houses are undefeated, the computer won.)
		*/
		if (!hptr->IsDefeated) {
			Session.Score[score_index].Wins++;
			Session.Winner = score_index;

			int score = 0;
			int count = 0;

			for (int i = 0; i < Houses.Count(); i++) {
				hptr2 = Houses[i];
				if (hptr2 != NULL && !hptr2->Class->IsMultiplayPassive && !hptr2->IsObserver && hptr2 != hptr) {
					score += hptr2->PointTotal;
					count++;
				}
			}

			if (count) {
				score /= count;
			}

			if (score <= 200) {
				score = 200;
			}

			Session.Score[score_index].Score[0] += score / 2;
		}

		/*
		**	Tally up all kills for this player
		*/
		int kills = 0;
		int losses = 0;

		for (i = 0; i < ARRAY_SIZE(hptr->UnitsKilled); i++) {
			kills += hptr->UnitsKilled[i];
		}
		for (i = 0; i < ARRAY_SIZE(hptr->BuildingsKilled); i++) {
			kills += hptr->BuildingsKilled[i];
		}
		Session.Score[score_index].Kills[0] = kills;

		losses += hptr->UnitsLost;
		losses += hptr->BuildingsLost;

		Session.Score[score_index].Lost[0] = losses;

		float kill_ratio = 0;
		if (losses > 0) {
			kill_ratio = (float)kills / (float)losses;
		}

		// Measuring what was spent leaves a house that was beaten showing what it built.
		float build_economy = most_spent > 0 ? (float)hptr->CreditsSpent / (float)most_spent : 0.0f;

		Session.Score[score_index].Built[0] = (int)(build_economy * 100.0);

		if (hptr->PointTotal > 0) {
			Session.Score[score_index].Score[0] += hptr->PointTotal;
		}

		DebugString(
			"%s: %s\n Scheme: %d\n Lost = %d\n Kills = %d\n Economy = %d\n Score = %d\n",
			Session.Score[score_index].Name,
			Session.Score[score_index].Wins > 0 ? "Winner" : "Loser",
			Session.Score[score_index].Scheme,
			Session.Score[score_index].Lost[0],
			Session.Score[score_index].Kills[0],
			Session.Score[score_index].Built[0],
			Session.Score[score_index].Score[0]);
		DebugString(" KillRatio = %f\n BuildEconomy = %f\n", kill_ratio, build_economy);
	}
}


/// <summary>
/// Prepares the round's scores for presentation.
/// This routine ranks the players and then pads any score that would otherwise
/// undercut the player below it, so that the score column always reads as a
/// descending ladder.
/// </summary>
/// <remarks>Call this routine before any of the drawing routines; they all present the
/// players in the order it establishes.</remarks>
void MultiScore::Process_Scores(void)
{
	int i, j;

	memset(Scores, 0, sizeof(Scores));

	for (i = 0; i < Session.NumScores; i++) {
		Scores[i] = &Session.Score[i];
	}

	if (Session.NumScores > 1) {
		qsort(Scores, Session.NumScores, sizeof(MPlayerScoreType *), Sort_Scores);
	}

	int totalPoints = 0;
	int playerCount = 0;

	for (i = 0; i < Houses.Count(); i++) {
		HouseClass *house = Houses[i];
		if (house != NULL && !house->Class->IsMultiplayPassive && !house->IsObserver) {
			totalPoints += house->PointTotal;
			playerCount++;
		}
	}

	if (playerCount != 0) {
		totalPoints /= playerCount;
	}

	if (totalPoints <= 100) {
		totalPoints = 100;
	}

	for (i = 0; i < Session.NumScores; i++) {
		for (j = 0; j < Session.NumScores - 1; j++) {
			if (Scores[j]->Score[0] < Scores[j + 1]->Score[0]) {
				Scores[j]->Score[0] = Random_Pick(1, totalPoints) + Scores[j + 1]->Score[0];
			}
		}
	}
}


/// <summary>
/// Determines which of two players should be listed first.
/// This routine is the sort callback used to rank the players for the score screen.
/// Games won decide the order, and the round's score settles a tie.
/// </summary>
/// <param name="p1">Pointer to the first player's score record pointer.</param>
/// <param name="p2">Pointer to the second player's score record pointer.</param>
/// <returns>Returns with a negative value if the first player ranks ahead of the second, a
/// positive value if it ranks behind, and zero if they are tied.</returns>
static int _cdecl Sort_Scores(const void *p1, const void *p2)
{
	MPlayerScoreType * mpscore1 = *(MPlayerScoreType **)p1;
	MPlayerScoreType * mpscore2 = *(MPlayerScoreType **)p2;

	int wins1 = mpscore1->Wins;
	int wins2 = mpscore2->Wins;

	if (wins1 > wins2) return(-1);
	if (wins1 < wins2) return(1);

	int score1 = mpscore1->Score[0];
	int score2 = mpscore2->Score[0];

	if (score1 == score2) return(0);
	return(score1 > score2 ? -1 : 1);
}


/// <summary>
/// Formats an elapsed time for display.
/// Absurdly long games are capped rather than printed with a runaway hour count.
/// </summary>
/// <param name="buffer">The buffer to fill in with the formatted time. NULL is tolerated.</param>
/// <param name="time">The elapsed time, in seconds.</param>
/// <remarks>Be sure that the destination buffer is big enough to hold the formatted time.</remarks>
inline void Format_Time(char * buffer, unsigned time)
{
	if (buffer != NULL) {
		if (time / 3600 > 99) {
			time = (100 * 3600) - 1;
		}
		unsigned hours = time / 3600;
		time -= hours * 3600;
		unsigned minutes = time / 60;
		time -= minutes * 60;
		sprintf(buffer, Fetch_String(TXT_TIME_FORMAT_HOURS), hours, minutes, time);
	}
}


/// <summary>
/// Draws the headings of the multiplayer score screen.
/// This routine lays out the top of the screen -- the game number on the left and the
/// elapsed playing time on the right -- and then animates in the heading for each of
/// the score columns over its own tinted backdrop.
/// </summary>
void MultiScore::Print_Headings(void)
{
	/// Display the number of games played
	char buffer[256];
	if (Session.Type == GAME_INTERNET) {
		sprintf(buffer, Fetch_String(TXT_GAME), WestwoodOnline_GameID);
	} else {
		sprintf(buffer, Fetch_String(TXT_GAME), Session.GamesPlayed);
	}

	MSPrintAnim *gameAnim = new MSPrintAnim(buffer, XPos + 15, YPos + 15, Font, RECT_NONE, 0, 4, true, false);
	Add_Animation(gameAnim);
	Wait_For_Anim(gameAnim);
	Font->Draw_String(AlternateSurface, (unsigned char const *)buffer, XPos + 15, YPos + 15, 2);

	Rect rect(XPos + 15, YPos + 15, Font->Get_Font_Width() * strlen(buffer), Font->Get_Font_Height());
	HiddenSurface->Blit_From(rect, *AlternateSurface, rect);
	Font->Draw_String(ScoreSurface, (unsigned char const *)buffer, 15, 15, 2);

	/// Display elapsed time
	Format_Time(buffer, Scen->ElapsedTimer / 60);
	int timeXPos = 625 - Font->Get_String_Width(buffer);

	MSPrintAnim *timeAnim = new MSPrintAnim(buffer, timeXPos + XPos, YPos + 15, Font, RECT_NONE, 0, 4, true, false);
	Add_Animation(timeAnim);
	Wait_For_Anim(timeAnim);
	Font->Draw_String(AlternateSurface, (unsigned char const *)buffer, timeXPos + XPos, YPos + 15, 2);

	int w = Font->Get_Font_Width() * strlen(buffer);
	rect.Set(XPos - w + 625, YPos + 15, w, Font->Get_Font_Height());
	HiddenSurface->Blit_From(rect, *AlternateSurface, rect);
	Font->Draw_String(ScoreSurface, (unsigned char const *)buffer, 625, 15, 2);

	/// Draw section for names
	rect.Set(XPos + 15, YPos + 40, 100, 315);
	AlternateSurface->Fill_Rect_Trans(rect, RGBClass(Font->Get_Red(), Font->Get_Green(), Font->Get_Blue()), 25);
	HiddenSurface->Blit_From(rect, *AlternateSurface, rect);

	const char *namesLabel = Fetch_String(TXT_NAMES);
	int namesLabelXPos = (100 - Font->Get_String_Width(namesLabel)) / 2;
	Font->Draw_String(ScoreSurface, (unsigned char const *)namesLabel, namesLabelXPos + 15, 45, 2);

	ScoreSurface->Draw_Line(Point2D(20, 45 + Font->Get_Font_Height()), Point2D(110, 45 + Font->Get_Font_Height()), Font->Get_Color());
	HiddenSurface->Draw_Line(Point2D(XPos + 20, YPos + 45 + Font->Get_Font_Height()), Point2D(XPos + 110, YPos + 45 + Font->Get_Font_Height()), Font->Get_Color());
	Blit_Rect(HiddenSurface, rect);

	MSWordAnim *namesAnim = new MSWordAnim(TXT_NAMES, namesLabelXPos + XPos + 15, YPos + 45, Font, RECT_NONE, 0, 4, true, false);
	Add_Animation(namesAnim);
	Wait_For_Anim(namesAnim);
	Wait_Delay(7);
	AlternateSurface->Blit_From(rect, *ScoreSurface, Rect(15, 40, 100, 315));
	HiddenSurface->Blit_From(rect, *AlternateSurface, rect);
	Blit_Rect(HiddenSurface, rect);

	/// Draw section for losses
	rect.Set(XPos + 147, YPos + 40, 110, 315);
	AlternateSurface->Fill_Rect_Trans(rect, RGBClass(Font->Get_Red(), Font->Get_Green(), Font->Get_Blue()), 25);
	HiddenSurface->Blit_From(rect, *AlternateSurface, rect);

	const char *lossesLabel = Fetch_String(TXT_LOSSES);
	int lossesLabelXPos = (110 - Font->Get_String_Width(lossesLabel)) / 2;
	Font->Draw_String(ScoreSurface, (unsigned char const *)lossesLabel, lossesLabelXPos + 147, 45, 2);

	ScoreSurface->Draw_Line(Point2D(152, 45 + Font->Get_Font_Height()), Point2D(252, 45 + Font->Get_Font_Height()), Font->Get_Color());
	HiddenSurface->Draw_Line(Point2D(XPos + 152, YPos + 45 + Font->Get_Font_Height()), Point2D(XPos + 252, YPos + 45 + Font->Get_Font_Height()), Font->Get_Color());
	Blit_Rect(HiddenSurface, rect);

	MSWordAnim *lossesAnim = new MSWordAnim(TXT_LOSSES, lossesLabelXPos + XPos + 147, YPos + 45, Font, RECT_NONE, 0, 4, true, false);
	Add_Animation(lossesAnim);
	Wait_For_Anim(lossesAnim);
	Wait_Delay(7);
	AlternateSurface->Blit_From(rect, *ScoreSurface, Rect(147, 40, 110, 315));
	HiddenSurface->Blit_From(rect, *AlternateSurface, rect);
	Blit_Rect(HiddenSurface, rect);

	/// Draw section for kills
	rect.Set(XPos + 269, YPos + 40, 110, 315);
	AlternateSurface->Fill_Rect_Trans(rect, RGBClass(Font->Get_Red(), Font->Get_Green(), Font->Get_Blue()), 25);
	HiddenSurface->Blit_From(rect, *AlternateSurface, rect);

	const char *killsLabel = Fetch_String(TXT_KILLS);
	int killsLabelXPos = (110 - Font->Get_String_Width(killsLabel)) / 2;
	Font->Draw_String(ScoreSurface, (unsigned char const *)killsLabel, killsLabelXPos + 269, 45, 2);

	ScoreSurface->Draw_Line(Point2D(274, 45 + Font->Get_Font_Height()), Point2D(374, 45 + Font->Get_Font_Height()), Font->Get_Color());
	HiddenSurface->Draw_Line(Point2D(XPos + 274, YPos + 45 + Font->Get_Font_Height()), Point2D(XPos + 374, YPos + 45 + Font->Get_Font_Height()), Font->Get_Color());
	Blit_Rect(HiddenSurface, rect);

	MSWordAnim *killsAnim = new MSWordAnim(TXT_KILLS, killsLabelXPos + XPos + 269, YPos + 45, Font, RECT_NONE, 0, 4, true, false);
	Add_Animation(killsAnim);
	Wait_For_Anim(killsAnim);
	Wait_Delay(7);
	AlternateSurface->Blit_From(rect, *ScoreSurface, Rect(269, 40, 110, 315));
	HiddenSurface->Blit_From(rect, *AlternateSurface, rect);
	Blit_Rect(HiddenSurface, rect);

	/// Draw section for economy
	rect.Set(XPos + 391, YPos + 40, 110, 315);
	AlternateSurface->Fill_Rect_Trans(rect, RGBClass(Font->Get_Red(), Font->Get_Green(), Font->Get_Blue()), 25);
	HiddenSurface->Blit_From(rect, *AlternateSurface, rect);

	const char *economyLabel = Fetch_String(TXT_ECONOMY);
	int economyLabelXPos = (110 - Font->Get_String_Width(economyLabel)) / 2;
	Font->Draw_String(ScoreSurface, (unsigned char const *)economyLabel, economyLabelXPos + 391, 45, 2);

	ScoreSurface->Draw_Line(Point2D(396, 45 + Font->Get_Font_Height()), Point2D(496, 45 + Font->Get_Font_Height()), Font->Get_Color());
	HiddenSurface->Draw_Line(Point2D(XPos + 396, YPos + 45 + Font->Get_Font_Height()), Point2D(XPos + 496, YPos + 45 + Font->Get_Font_Height()), Font->Get_Color());
	Blit_Rect(HiddenSurface, rect);

	MSWordAnim *economyAnim = new MSWordAnim(TXT_ECONOMY, economyLabelXPos + XPos + 391, YPos + 45, Font, RECT_NONE, 0, 4, true, false);
	Add_Animation(economyAnim);
	Wait_For_Anim(economyAnim);
	Wait_Delay(7);
	AlternateSurface->Blit_From(rect, *ScoreSurface, Rect(391, 40, 110, 315));
	HiddenSurface->Blit_From(rect, *AlternateSurface, rect);
	Blit_Rect(HiddenSurface, rect);

	/// Draw section for scores
	rect.Set(XPos + 530, YPos + 40, 90, 315);
	AlternateSurface->Fill_Rect_Trans(rect, RGBClass(Font->Get_Red(), Font->Get_Green(), Font->Get_Blue()), 25);
	HiddenSurface->Blit_From(rect, *AlternateSurface, rect);

	const char *scoresLabel = Fetch_String(TXT_SCORE);
	int scoresLabelXPos = (90 - Font->Get_String_Width(scoresLabel)) / 2;
	Font->Draw_String(ScoreSurface, (unsigned char const *)scoresLabel, scoresLabelXPos + 530, 45, 2);

	ScoreSurface->Draw_Line(Point2D(535, 45 + Font->Get_Font_Height()), Point2D(615, 45 + Font->Get_Font_Height()), Font->Get_Color());
	HiddenSurface->Draw_Line(Point2D(XPos + 535, YPos + 45 + Font->Get_Font_Height()), Point2D(XPos + 615, YPos + 45 + Font->Get_Font_Height()), Font->Get_Color());
	Blit_Rect(HiddenSurface, rect);

	MSWordAnim *scoresAnim = new MSWordAnim(TXT_SCORE, scoresLabelXPos + XPos + 530, YPos + 45, Font, RECT_NONE, 0, 4, true, false);
	Add_Animation(scoresAnim);
	Wait_For_Anim(scoresAnim);
	Wait_Delay(7);
	AlternateSurface->Blit_From(rect, *ScoreSurface, Rect(530, 40, 90, 315));
	HiddenSurface->Blit_From(rect, *AlternateSurface, rect);
	Blit_Rect(HiddenSurface, rect);
	Wait_Delay(30);
}


/// <summary>
/// Draws the name column of the score screen.
/// Each player's name is animated into place over a band tinted with his own color
/// scheme, from the winner downward.
/// </summary>
void MultiScore::Print_Player_Names(void)
{
	int yPos = 80;

	for (int i = 0; i < Session.NumScores; i++) {

		RGBClass color = ColorSchemes[Scores[i]->Scheme]->HSV;
		Rect rect(XPos + 15, YPos + yPos - 5, -605, 30);
		AlternateSurface->Fill_Rect_Trans(rect, color, 25);
		HiddenSurface->Blit_From(rect, *AlternateSurface, rect);
		Blit_Rect(HiddenSurface, rect);

		Font->Draw_String(ScoreSurface, (unsigned char const *)Scores[i]->Name, 20, yPos, 2);
		MSWordAnim * anim = new MSWordAnim(Scores[i]->Name, XPos + 20, YPos + yPos, Font);
		Add_Animation(anim);
		Wait_For_Anim(anim);

		Wait_Delay(7);

		Rect r = rect;
		r.X -= XPos;
		r.Y -= YPos;

		AlternateSurface->Blit_From(rect, *ScoreSurface, r);
		HiddenSurface->Blit_From(rect, *AlternateSurface, rect);
		Blit_Rect(HiddenSurface, rect);

		yPos += 35;
	}

	Wait_Delay(30);
}


/// <summary>
/// Draws the losses column of the score screen.
/// This routine collects what each player lost over the round and hands it to the
/// bar graph drawer.
/// </summary>
void MultiScore::Draw_Losses(void)
{
	int scores[MAX_PLAYERS];

	for (int i = 0; i < Session.NumScores; i++) {
		scores[i] = Scores[i]->Lost[0];
	}

	Draw_Bars(scores, Session.NumScores, XPos + 152, YPos + 80, 100);
	Wait_Delay(45);
}


/// <summary>
/// Draws the kills column of the score screen.
/// This routine collects each player's kill count and hands it to the bar graph
/// drawer.
/// </summary>
void MultiScore::Draw_Kills(void)
{
	int scores[MAX_PLAYERS];

	for (int i = 0; i < Session.NumScores; i++) {
		scores[i] = Scores[i]->Kills[0];
	}

	Draw_Bars(scores, Session.NumScores, XPos + 274, YPos + 80, 100);
	Wait_Delay(45);
}


/// <summary>
/// Draws the economy column of the score screen.
/// This routine collects what each player built over the round and hands it to the
/// bar graph drawer.
/// </summary>
void MultiScore::Draw_Economy(void)
{
	int scores[MAX_PLAYERS];

	for (int i = 0; i < Session.NumScores; i++) {
		scores[i] = Scores[i]->Built[0];
	}

	Draw_Bars(scores, Session.NumScores, XPos + 396, YPos + 80, 100);
	Wait_Delay(45);
}


/// <summary>
/// Draws and animates the final score column.
/// This routine is the climax of the presentation. Every player's score counts up
/// together from nothing to its final value, and then the finished numbers are laid
/// down on the working surfaces so that they survive the rest of the screen.
/// </summary>
void MultiScore::Print_Scores(void)
{
	int i;
	int maxScore = 1;

	/// Find the maximum score among all players
	for (i = 0; i < Session.NumScores; i++) {
		if (maxScore <= Scores[i]->Score[0]) {
			maxScore = Scores[i]->Score[0];
		}
	}

	/// Temporary buffer to store rectangles for updating
	Rect updateRects[MAX_PLAYERS];

	/// Animate the scores incrementally
	if (maxScore > 0) {
		for (int percentage = 1; percentage <= 100; percentage++) {
			int yPos = YPos + 80;

			for (i = 0; i < Session.NumScores; i++) {

				int score = Scores[i]->Score[0];

				/// Clear the previous rectangle if valid
				if (updateRects[i].Is_Valid()) {
					HiddenSurface->Blit_From(updateRects[i], *AlternateSurface, updateRects[i]);
					Add_Update_Rect(updateRects[i]);
				}

				int xPos = XPos + 575;

				/// Calculate the current score based on the percentage
				int currentScore = (percentage * score) / 100;
				updateRects[i] = Print_Score(HiddenSurface, currentScore, score, xPos, yPos);
				Add_Update_Rect(updateRects[i]);

				/// Move to the next player's score position
				yPos += 35;
			}

			/// Play the "Graph" sound effect and wait briefly
			Play_Sound_Effect("Graph", 100);
			Wait_Delay(1);
		}
	}

	/// Draw the final scores
	int yPos = YPos + 80;
	for (i = 0; i < Session.NumScores; i++) {
		/// The score and maximum arguments really are the other way round here. Print_Score
		/// only uses 'maximum' to clamp, so the smaller of 100 and the player's score is
		/// printed either way.
		int score = Scores[i]->Score[0];
		Print_Score(AlternateSurface, 100, score, XPos + 575, yPos);
		Print_Score(ScoreSurface, 100, score, 575, yPos - YPos);
		yPos += 35;
	}
}


/// <summary>
/// Converts a color into the display's pixel format.
/// </summary>
/// <returns>Returns with the pixel value that the display surfaces expect.</returns>
static inline unsigned RGB_To_Pixel(RGBClass &rgb)
{
	return (unsigned((rgb.Get_Blue() >> DSurface::BlueLeft) << DSurface::BlueRight)
		  | unsigned((rgb.Get_Red() >> DSurface::RedLeft) << DSurface::RedRight)
		  | unsigned((rgb.Get_Green() >> DSurface::GreenLeft) << DSurface::GreenRight));
}


/// <summary>
/// Draws and animates one bar graph column of the score screen.
/// This routine is used by the losses, kills and economy columns. Each player gets a
/// bar in his own color scheme, and the bars grow together with the running value
/// printed alongside until every bar has reached its final length.
/// </summary>
/// <param name="scores">The value to graph for each player, in score screen display order.</param>
/// <param name="x">The screen position of the first bar.</param>
/// <param name="y">The screen position of the first bar.</param>
/// <param name="width">The length of a full length bar.</param>
void MultiScore::Draw_Bars(int * scores, int numScores, int x, int y, int width)
{
	int i;

	int maxScore = 1;

	/// Find the maximum score among all players
	for (i = 0; i < numScores; i++) {
		if (scores[i] > maxScore) {
			maxScore = scores[i];
		}
	}

	/// Calculate bar widths based on scores
	int adjustedScores[MAX_PLAYERS];

	for (i = 0; i < numScores; i++) {
		adjustedScores[i] = width * scores[i] / maxScore;
	}

	/// Scale scores if the maximum score is less than 20
	if (maxScore < 20) {
		for (i = 0; i < numScores; i++) {
			adjustedScores[i] = scores[i] * 5;
		}
	}

	int maxWidth = 1;
	/// Find the maximum score among all players
	for (i = 0; i < numScores; i++) {
		if (adjustedScores[i] > maxWidth) {
			maxWidth = adjustedScores[i];
		}
	}

	/// Draw the bars
	int yPos = y - YPos;
	for (i = 0; i < numScores; i++) {
		/// Get the color for the player's bar
		RGBClass color = ColorSchemes[Scores[i]->Scheme]->HSV;

		/// Draw the bar background
		Rect barRect(x - XPos, yPos, adjustedScores[i], 20);
		ScoreSurface->Fill_Rect_Trans(barRect, color, 50);

		int colorInt = RGB_To_Pixel(color);

		/// Draw the bar outline
		barRect.Set(x - XPos, yPos, width, 20);
		ScoreSurface->Draw_Rect(barRect, colorInt);

		/// Draw the bar on the alternate and hidden surfaces
		barRect.Set(x, yPos + YPos, width, 20);
		AlternateSurface->Draw_Rect(barRect, colorInt);
		HiddenSurface->Draw_Rect(barRect, colorInt);

		/// Add the bar to the update rects
		Add_Update_Rect(barRect);

		/// Move to the next bar position
		yPos += 35;
	}

	/// Temporary buffer to store rectangles for updating
	Rect updateRects[MAX_PLAYERS];

	Rect dr;
	Rect sr;

	/// Animate the bars incrementally
	for (int percentage = 1; percentage <= maxWidth; percentage += 2) {
		yPos = y;
		for (i = 0; i < numScores; i++) {

			/// Clear the previous rectangle if valid
			if (updateRects[i].Is_Valid()) {
				HiddenSurface->Blit_From(updateRects[i], *AlternateSurface, updateRects[i]);
				Add_Update_Rect(dr);
			}

			if (percentage <= adjustedScores[i]) {
				dr.Set(x, yPos, percentage, 20);
				sr.Set(x - XPos, yPos - YPos, percentage, 20);
				HiddenSurface->Blit_From(dr, *ScoreSurface, sr);
				AlternateSurface->Blit_From(dr, *ScoreSurface, sr);
				Add_Update_Rect(dr);
				updateRects[i] = Print_Score(HiddenSurface, scores[i] * percentage / maxWidth, scores[i], x + width / 2, yPos);
				Add_Update_Rect(updateRects[i]);
			} else {
				updateRects[i] = Print_Score(HiddenSurface, scores[i], scores[i], x + width / 2, yPos);
				Add_Update_Rect(updateRects[i]);
				updateRects[i].Set(0, 0, 0, 0);
			}

			/// Move to the next bar position
			yPos += 35;
		}

		/// Play the "Graph" sound effect and wait briefly
		Play_Sound_Effect("Graph", 100);
		Wait_Delay(1);
	}

	// The animation steps two pixels at a time, so its last frame stops short of the longest
	// bar's own figure, and that figure has to come off the screen before the finished one.
	for (i = 0; i < numScores; i++) {
		if (updateRects[i].Is_Valid()) {
			HiddenSurface->Blit_From(updateRects[i], *AlternateSurface, updateRects[i]);
			Add_Update_Rect(updateRects[i]);
		}
	}

	/// Draw the final bars and scores
	int srcYPos = y - YPos;
	for (i = 0; i < numScores; i++) {
		/// Draw the final bar
		dr.Set(x, y, adjustedScores[i], 20);
		sr.Set(x - XPos, srcYPos, adjustedScores[i], 20);
		AlternateSurface->Blit_From(dr, *ScoreSurface, sr);

		/// Draw the final score text
		Print_Score(AlternateSurface, scores[i], scores[i], x + width / 2, y);
		Print_Score(ScoreSurface, scores[i], scores[i], x + width / 2 - XPos, srcYPos);
		Add_Update_Rect(Print_Score(HiddenSurface, scores[i], scores[i], x + width / 2, y));

		/// Move to the next bar position
		y += 35;
		srcYPos += 35;
	}
}


/// <summary>
/// Draws a score value onto a surface.
/// The number is centered about the horizontal position given. The rectangle returned
/// lets the animating callers erase the number again before they print the next one.
/// </summary>
/// <param name="maximum">The largest value that may be printed; a larger score is
/// clamped to it.</param>
/// <param name="x">The horizontal center of the printed number.</param>
/// <returns>Returns with the rectangle that the number occupies.</returns>
Rect MultiScore::Print_Score(Surface * surface, int score, int maximum, int x, int y)
{
	char buffer[32];

	sprintf(buffer, "%d", score > maximum ? maximum : score);

	int stringWidth = Font->Get_String_Width(buffer);
	Font->Draw_String(surface, (unsigned char const *)buffer, x - stringWidth / 2, y, 2);

	int totalStringWidth = Font->Get_Font_Width() * strlen(buffer);
	return(Rect(x - totalStringWidth / 2, y, totalStringWidth, Font->Get_Font_Height()));
}


/// <summary>
/// Handles idle time during the score presentation.
/// The engine calls this routine whenever it is waiting on an animation or a delay.
/// </summary>
void MultiScore::Process_Idle(void)
{
	Callback();
}


/// <summary>
/// Services the network link while the score screen is up.
/// This routine is called from the presentation's idle handler.
/// </summary>
void MultiScore::Callback(void)
{
	Call_Back();
}
