/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// Pins the UI state that needs no toolkit: who a held key or button belongs to as screens
// open, close and lose the capture or the focus, how the bytes of a narrow window's text
// messages become code points, what the overlay renderer refuses before it draws, and what
// the presenter owes the screen after a present is taken, refused or skipped.

#include "ui/rml/rmlrendermath.h"
#include "ui/uiinput.h"
#include "videodirty.h"

#include <array>
#include <cstdint>
#include <cstdio>
#include <initializer_list>
#include <limits>
#include <vector>

namespace {

int Failures = 0;


void Check(bool condition, char const * what)
{
	std::printf("%-76s %s\n", what, condition ? "ok" : "FAILED");

	if (!condition) {
		Failures++;
	}
}


std::vector<char32_t> Decode(UIUTF8DecoderClass & decoder, std::initializer_list<unsigned char> bytes)
{
	std::vector<char32_t> result;
	for (unsigned char byte : bytes) {
		UIInputText text = decoder.Feed(byte);
		for (unsigned index = 0; index < text.Count; index++) {
			result.push_back(text.Codepoints[index]);
		}
	}
	return(result);
}


void Test_Ownership(void)
{
	UIInputStateClass state;

	Check(state.Gesture_Owner() == UI_INPUT_NONE && !state.Has_UI_Mouse() && !state.Any_Owned(), "the initial state holds no gesture");
	Check(state.Press_Key(65, UI_INPUT_RML) == UI_INPUT_RML, "RmlUi owns the key it first pressed");
	Check(state.Any_Owned(), "a held key counts as owned input");
	Check(state.Press_Key(65, UI_INPUT_IMGUI) == UI_INPUT_RML, "a repeat keeps the first owner of a key");
	Check(state.Release_Key(65) == UI_INPUT_RML, "a release returns the owner of its press");
	Check(state.Release_Key(65) == UI_INPUT_NONE, "a second release has no owner");
	Check(state.Press_Key(65, UI_INPUT_GAME) == UI_INPUT_GAME, "a later press of the same key can belong to the game");
	Check(state.Press_Key(256, UI_INPUT_RML) == UI_INPUT_NONE && state.Key_Owner(256) == UI_INPUT_NONE, "a key out of range takes no owner");
	Check(state.Release_Key(256) == UI_INPUT_NONE, "a release out of range is ignored");

	Check(state.Press_Mouse(0, UI_INPUT_IMGUI) == UI_INPUT_IMGUI, "ImGui owns the button it first pressed");
	Check(state.Press_Mouse(0, UI_INPUT_GAME) == UI_INPUT_IMGUI, "a move across the game cannot change a press's owner");
	Check(state.Release_Mouse(0) == UI_INPUT_IMGUI, "a UI release stays a UI release outside the panel");
	Check(state.Press_Mouse(0, UI_INPUT_GAME) == UI_INPUT_GAME, "the next press can go to the game");
	Check(state.Press_Mouse(0, UI_INPUT_RML) == UI_INPUT_GAME, "a game drag cannot become a document drag");
	Check(state.Release_Mouse(0) == UI_INPUT_GAME, "a game release keeps its owner");
	Check(state.Press_Mouse(0, UI_INPUT_RML) == UI_INPUT_RML, "a fresh press can belong to RmlUi");
	Check(state.Press_Mouse(1, UI_INPUT_RML) == UI_INPUT_RML, "a second held button joins the UI gesture");
	Check(state.Release_Mouse(0) == UI_INPUT_RML && state.Has_UI_Mouse(), "releasing one button keeps the other's capture");
	Check(state.Gesture_Owner() == UI_INPUT_RML, "the remaining button owns the pointer's motion");
	Check(state.Release_Mouse(1) == UI_INPUT_RML && !state.Has_UI_Mouse(), "the capture ends with the last button");
	Check(state.Press_Mouse(5, UI_INPUT_IMGUI) == UI_INPUT_NONE && state.Release_Mouse(5) == UI_INPUT_NONE, "a button out of range is ignored");

	state.Press_Key(66, UI_INPUT_IMGUI);
	state.Press_Mouse(0, UI_INPUT_RML);
	state.Press_Mouse(2, UI_INPUT_GAME);
	Check(!state.Any_Suppressed(), "nothing is suppressed before a cancel");
	state.Cancel_UI();
	Check(state.Key_Owner(65) == UI_INPUT_GAME && state.Mouse_Owner(2) == UI_INPUT_GAME, "closing a screen leaves the game's own presses alone");
	Check(state.Key_Owner(66) == UI_INPUT_SUPPRESSED && state.Mouse_Owner(0) == UI_INPUT_SUPPRESSED, "closing a screen suppresses what the toolkits held");
	Check(state.Any_Suppressed(), "suppressed input is reported");
	Check(state.Press_Key(66, UI_INPUT_GAME) == UI_INPUT_SUPPRESSED, "a repeat of a suppressed key stays suppressed");
	Check(state.Release_Key(66) == UI_INPUT_SUPPRESSED && state.Release_Mouse(0) == UI_INPUT_SUPPRESSED, "a suppressed release never reaches the game");
	Check(state.Press_Key(66, UI_INPUT_GAME) == UI_INPUT_GAME, "a fresh press works once the suppression ended");
	state.Cancel_All();
	Check(state.Release_Key(65) == UI_INPUT_SUPPRESSED && state.Release_Key(66) == UI_INPUT_SUPPRESSED, "losing focus suppresses every held key");
	Check(state.Release_Mouse(2) == UI_INPUT_SUPPRESSED, "losing focus suppresses a game-owned button");
	state.Press_Key(67, UI_INPUT_RML);
	state.Press_Mouse(3, UI_INPUT_IMGUI);
	state.Reset();
	Check(state.Key_Owner(67) == UI_INPUT_NONE && state.Mouse_Owner(3) == UI_INPUT_NONE && state.Gesture_Owner() == UI_INPUT_NONE && !state.Any_Owned(), "a reset forgets every owner");
}


void Test_Reconciliation(void)
{
	UIInputStateClass state;
	std::array<bool, UIInputStateClass::BUTTON_COUNT> released {};

	state.Press_Mouse(0, UI_INPUT_GAME);
	state.Reconcile_Cancelled_Mouse(released);
	Check(state.Gesture_Owner() == UI_INPUT_GAME, "a physical release cannot forget a game gesture before its up message");
	Check(!UI_Consumes_Input(state.Gesture_Owner()), "a game drag's motion stays the game's over a document");
	Check(!UI_Consumes_Input(state.Release_Mouse(0)), "a game release reaches the game after the physical release");
	state.Press_Mouse(0, UI_INPUT_IMGUI);
	state.Reconcile_Cancelled_Mouse(released);
	Check(state.Gesture_Owner() == UI_INPUT_IMGUI && UI_Consumes_Input(state.Release_Mouse(0)), "a physical release cannot hand a UI gesture to the game");

	state.Press_Key(70, UI_INPUT_RML);
	state.Press_Mouse(0, UI_INPUT_GAME);
	state.Press_Mouse(1, UI_INPUT_GAME);
	state.Cancel_Mouse();
	Check(state.Key_Owner(70) == UI_INPUT_RML, "losing the capture leaves the keys alone");
	std::array<bool, UIInputStateClass::BUTTON_COUNT> oneheld {};
	oneheld[1] = true;
	state.Reconcile_Cancelled_Mouse(oneheld);
	Check(state.Mouse_Owner(0) == UI_INPUT_NONE && state.Mouse_Owner(1) == UI_INPUT_SUPPRESSED, "only a cancelled button that is up is forgotten");
	Check(!UI_Consumes_Input(state.Release_Mouse(0)), "a release the shell forgot after reconciliation is the game's");
	Check(UI_Consumes_Input(state.Release_Mouse(1)), "a cancelled button still held keeps its release");

	std::array<bool, UIInputStateClass::KEY_COUNT> keysup {};
	state.Press_Key(71, UI_INPUT_IMGUI);
	state.Cancel_All();
	keysup[70] = true;
	state.Reconcile_Cancelled_Keys(keysup);
	Check(state.Key_Owner(70) == UI_INPUT_SUPPRESSED && state.Key_Owner(71) == UI_INPUT_NONE, "only a cancelled key that is up is forgotten");
	Check(!UI_Consumes_Input(UI_INPUT_GAME) && UI_Consumes_Input(UI_INPUT_RML) && UI_Consumes_Input(UI_INPUT_SUPPRESSED), "delivery follows the owner of the press, not the pointer's position");
	Check(!UI_Consumes_Input(UI_INPUT_NONE), "input nobody owns is the game's");
}


void Test_Text(void)
{
	UIUTF8DecoderClass decoder;

	Check(Decode(decoder, { 0x41, 0x0A }) == std::vector<char32_t> { U'A', U'\n' }, "ASCII text passes through");
	Check(Decode(decoder, { 0xC3, 0xA9, 0xE2, 0x82, 0xAC, 0xF0, 0x9F, 0x98, 0x80 }) == std::vector<char32_t> { 0xE9, 0x20AC, 0x1F600 }, "two-, three- and four-byte sequences decode");
	Check(decoder.Feed(0xE2).Count == 0 && decoder.Feed(0x82).Count == 0, "a partial sequence waits for its bytes");
	UIInputText completed = decoder.Feed(0xAC);
	Check(completed.Count == 1 && completed.Codepoints[0] == 0x20AC, "a sequence can span messages");
	Check(decoder.Feed(0xE2).Count == 0, "a truncated prefix stays pending");
	UIInputText repaired = decoder.Feed(0x42);
	Check(repaired.Count == 2 && repaired.Codepoints[0] == 0xFFFD && repaired.Codepoints[1] == U'B', "a malformed prefix does not swallow the ASCII after it");
	Check(Decode(decoder, { 0xE2, 0xC3, 0xA9 }) == std::vector<char32_t> { 0xFFFD, 0xE9 }, "a malformed prefix does not swallow the sequence after it");
	Check(Decode(decoder, { 0xC0, 0xAF }) == std::vector<char32_t> { 0xFFFD, 0xFFFD }, "invalid lead bytes are rejected");
	Check(Decode(decoder, { 0xE0, 0x80, 0x80 }) == std::vector<char32_t> { 0xFFFD }, "an overlong encoding is rejected");
	Check(Decode(decoder, { 0xED, 0xA0, 0x80 }) == std::vector<char32_t> { 0xFFFD }, "an encoded surrogate is rejected");
	Check(Decode(decoder, { 0xF4, 0x90, 0x80, 0x80 }) == std::vector<char32_t> { 0xFFFD }, "a code point past U+10FFFF is rejected");
	Check(Decode(decoder, { 0x80, 0x41 }) == std::vector<char32_t> { 0xFFFD, U'A' }, "a stray continuation byte does not swallow the text after it");
	decoder.Feed(0xF0);
	decoder.Feed(0x9F);
	decoder.Reset();
	Check(Decode(decoder, { 0x43 }) == std::vector<char32_t> { U'C' }, "a reset drops an incomplete sequence");
	decoder.Feed(0xE2);
	decoder.Reset();
	Check(Decode(decoder, { 0x82, 0xAC }) == std::vector<char32_t> { 0xFFFD, 0xFFFD }, "a reset cannot splice fragments across owners");
}


void Test_Render_Math(void)
{
	std::uint32_t bytes = 0;
	Check(UI_Render_Byte_Count(10, 24, bytes) && bytes == 240, "a byte count is the whole product");
	Check(!UI_Render_Byte_Count(std::numeric_limits<std::size_t>::max(), 2, bytes) && bytes == 0, "a byte count that overflows fails");
	Check(!UI_Render_Byte_Count(0, 4, bytes), "an empty allocation fails");

	int const valid[] = { 2, 0, 1 };
	int const negative[] = { -1, 0, 1 };
	int const outside[] = { 0, 1, 3 };
	int const incomplete[] = { 0, 1 };
	Check(UI_Render_Index_Range(valid, 3), "whole triangles over the vertices pass");
	Check(!UI_Render_Index_Range(negative, 3), "a negative index fails");
	Check(!UI_Render_Index_Range(outside, 3), "an index past the last vertex fails");
	Check(!UI_Render_Index_Range(incomplete, 3), "an incomplete triangle fails");

	UIRenderClip clip;
	Check(UI_Render_Clip_Rect(-2.4f, 1.2f, 12.1f, 25.7f, 100, 50, 10, 20, clip) && clip.X == 100 && clip.Y == 51 && clip.Width == 10 && clip.Height == 19, "a fractional scissor rounds outward, clips, and lands in the target");
	Check(!UI_Render_Clip_Rect(-10.0f, 0.0f, -1.0f, 10.0f, 100, 50, 10, 20, clip), "a scissor outside the viewport draws nothing");
	Check(!UI_Render_Clip_Rect(3.0f, 0.0f, 3.0f, 10.0f, 0, 0, 10, 20, clip), "an empty scissor draws nothing");
	Check(!UI_Render_Clip_Rect(0.0f, 0.0f, std::numeric_limits<float>::infinity(), 10.0f, 0, 0, 10, 20, clip), "a non-finite scissor fails");
	Check(!UI_Render_Clip_Rect(0.0f, 0.0f, 10.0f, 10.0f, 65530, 0, 10, 20, clip), "a scissor cannot wrap the 16-bit target coordinates");

	std::array<std::uint8_t, 48> pixels;
	pixels.fill(0xEE);
	for (int row = 0; row < 3; row++) {
		for (int column = 0; column < 12; column++) {
			pixels[(std::size_t)row * 16 + column] = (std::uint8_t)(row * 12 + column);
		}
	}
	std::vector<std::uint8_t> packed;
	Check(UI_Render_Copy_RGBA_Rect(pixels, 3, 3, 16, 1, 1, 2, 2, packed) && packed.size() == 16 && packed[0] == 16 && packed[8] == 28, "a rectangle out of a pitched image packs tightly");
	Check(!UI_Render_Copy_RGBA_Rect(pixels, 3, 3, 16, 2, 2, 2, 2, packed), "a rectangle past the image fails");
	Check(!UI_Render_Copy_RGBA_Rect(pixels, 3, 4, 16, 0, 0, 1, 1, packed), "an image larger than its bytes fails");
}


void Test_Dirty_State(void)
{
	VideoDirtyStateClass dirty;

	Check(!dirty.Is_Dirty(), "nothing is owed at the start");

	dirty.Mark_Overlay();
	Check(dirty.Is_Dirty(), "an overlay mark makes a present due");
	VideoDirtySnapshotType first = dirty.Consume();
	Check(!first.Game && first.Overlay && first.Upload, "the first present uploads the frame the renderer has never seen");
	Check(!dirty.Is_Dirty(), "consuming takes the marks");

	dirty.Upload_Completed();
	dirty.Mark_Overlay();
	VideoDirtySnapshotType overlay = dirty.Consume();
	Check(overlay.Overlay && !overlay.Upload, "an overlay-only present leaves the uploaded frame alone");

	dirty.Mark_Game();
	VideoDirtySnapshotType game = dirty.Consume();
	Check(game.Game && game.Upload, "a game mark uploads the frame");

	dirty.Mark_Game();
	VideoDirtySnapshotType consumed = dirty.Consume();
	dirty.Mark_Overlay();
	Check(dirty.Is_Dirty(), "a mark raised while a present runs survives it");
	dirty.Restore(consumed);
	VideoDirtySnapshotType restored = dirty.Consume();
	Check(restored.Game && restored.Overlay, "restoring a refused present keeps the marks raised since");

	dirty.Restore(VideoDirtySnapshotType { false, false, false });
	Check(dirty.Is_Dirty() && dirty.Consume().Overlay, "a refused present with nothing marked is retried as an overlay present");

	dirty.Upload_Completed();
	dirty.Invalidate_Frame();
	VideoDirtySnapshotType invalidated = dirty.Consume();
	Check(invalidated.Game && invalidated.Upload, "a renderer that lost the frame gets it uploaded again");

	dirty.Mark_Game();
	dirty.Reset();
	VideoDirtySnapshotType reset = dirty.Consume();
	Check(!dirty.Is_Dirty() && !reset.Game && !reset.Overlay && reset.Upload, "a reset forgets the marks and the upload");
}

}


int main(void)
{
	Test_Ownership();
	Test_Reconciliation();
	Test_Text();
	Test_Render_Math();
	Test_Dirty_State();

	std::printf("\n%s\n", Failures == 0 ? "PASSED" : "FAILED");
	return(Failures == 0 ? 0 : 1);
}
