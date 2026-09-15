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

#include "ui/rml/rmlfontsheet.h"
#include "ui/rml/rmlimage.h"
#include "ui/rml/rmlrendermath.h"
#include "ui/uiinput.h"
#include "ui/uireveal.h"
#include "videodirty.h"

#include <array>
#include <cstdint>
#include <cstdio>
#include <cstring>
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


void Test_Render_Transform(void)
{
	float model[16];

	UI_Render_Model_Matrix(nullptr, 10.0f, 20.0f, model);
	Check(model[0] == 1.0f && model[5] == 1.0f && model[10] == 1.0f && model[15] == 1.0f && model[12] == 10.0f && model[13] == 20.0f,
		  "a fragment with no transform travels by its translation alone");

	// Quarter turn: the x axis becomes the y axis, so the translation turns with it.
	float rotation[16] = {};
	rotation[1] = 1.0f;
	rotation[4] = -1.0f;
	rotation[10] = 1.0f;
	rotation[15] = 1.0f;
	UI_Render_Model_Matrix(rotation, 10.0f, 20.0f, model);
	Check(model[12] == -20.0f && model[13] == 10.0f, "a rotated fragment is translated in the rotated frame, not the screen's");
	Check(model[1] == 1.0f && model[4] == -1.0f, "and the transform's own columns are left alone");

	// A transform that already carries a translation keeps it, with the fragment's scaled
	// by the transform and added on top.
	float scaled[16] = {};
	scaled[0] = 2.0f;
	scaled[5] = 2.0f;
	scaled[10] = 1.0f;
	scaled[12] = 5.0f;
	scaled[13] = 6.0f;
	scaled[15] = 1.0f;
	UI_Render_Model_Matrix(scaled, 10.0f, 20.0f, model);
	Check(model[12] == 25.0f && model[13] == 46.0f, "a scaling transform scales the fragment's translation and keeps its own");

	float perspective[16] = {};
	perspective[0] = 1.0f;
	perspective[3] = 0.01f;
	perspective[5] = 1.0f;
	perspective[10] = 1.0f;
	perspective[15] = 1.0f;
	UI_Render_Model_Matrix(perspective, 10.0f, 20.0f, model);
	Check(model[15] > 1.09f && model[15] < 1.11f, "a perspective row reaches the fragment's translation too");
}


void Test_Render_Mask(void)
{
	UIRenderMaskStep step;

	Check(UI_Render_Mask_Step(UI_RENDER_MASK_SET, 7, step) && step.Clear && !step.Increment && step.Write == 1 && step.Reference == 1,
		  "a mask that starts over clears whatever was there and keeps its own shape");
	Check(UI_Render_Mask_Step(UI_RENDER_MASK_SET_INVERSE, 7, step) && step.Clear && !step.Increment && step.Write == 1 && step.Reference == 0,
		  "an inverted mask writes the same shape and keeps everything but it");
	Check(UI_Render_Mask_Step(UI_RENDER_MASK_INTERSECT, 1, step) && !step.Clear && step.Increment && step.Reference == 2,
		  "a narrowing mask counts up from what is already there");
	Check(UI_Render_Mask_Step(UI_RENDER_MASK_INTERSECT, 0, step) && step.Reference == 1, "and counts from nothing as well");
	Check(!UI_Render_Mask_Step(UI_RENDER_MASK_INTERSECT, 255, step), "masks nested past what eight bits count are refused");
}


// Builds an 8-bit PCX the way one is laid out on disk: a 128-byte header, run-length rows,
// then the marker and the 256 colors.
std::vector<std::uint8_t> Build_PCX(int width, int height, int bytesperline, std::vector<std::uint8_t> const & rows, std::vector<std::uint8_t> const & palette)
{
	std::vector<std::uint8_t> file(128, 0);
	file[0] = 10;
	file[1] = 5;
	file[2] = 1;
	file[3] = 8;
	file[8] = (std::uint8_t)((width - 1) & 0xFF);
	file[9] = (std::uint8_t)((width - 1) >> 8);
	file[10] = (std::uint8_t)((height - 1) & 0xFF);
	file[11] = (std::uint8_t)((height - 1) >> 8);
	file[65] = 1;
	file[66] = (std::uint8_t)(bytesperline & 0xFF);
	file[67] = (std::uint8_t)(bytesperline >> 8);

	file.insert(file.end(), rows.begin(), rows.end());
	file.push_back(0x0C);
	file.insert(file.end(), palette.begin(), palette.end());
	file.resize(file.size() + (768 - palette.size()), 0);
	return(file);
}


void Test_Image(void)
{
	// Index 1 is red, index 2 is the magenta this art is keyed with, and index 3 is white.
	std::vector<std::uint8_t> palette = { 0, 0, 0, 255, 0, 0, 255, 0, 255, 255, 255, 255 };

	// Two rows of four, each written as one run of four.
	std::vector<std::uint8_t> compressed = { 0xC4, 1, 0xC4, 3 };
	UIImageIndexed image;
	Check(UI_Decode_PCX(Build_PCX(4, 2, 4, compressed, palette), image), "an 8-bit run-length PCX decodes");
	Check(image.Width == 4 && image.Height == 2 && image.Pixels.size() == 8, "and reports the size its header claims");
	Check(image.Pixels[0] == 1 && image.Pixels[3] == 1 && image.Pixels[4] == 3 && image.Pixels[7] == 3, "a run fills its whole row");
	Check(image.Palette[3] == 255 && image.Palette[4] == 0 && image.Palette[5] == 0, "and the palette comes off the end of the file");

	// The same picture written one literal byte at a time.
	std::vector<std::uint8_t> literal = { 1, 1, 1, 1, 3, 3, 3, 3 };
	UIImageIndexed plain;
	Check(UI_Decode_PCX(Build_PCX(4, 2, 4, literal, palette), plain), "an uncompressed PCX decodes");
	Check(plain.Pixels == image.Pixels, "and holds the same pixels the compressed one does");

	// A row padded to an even length: the fourth pixel is dropped with the padding byte.
	std::vector<std::uint8_t> padded = { 1, 1, 1, 2, 3, 3, 3, 2 };
	UIImageIndexed narrow;
	Check(UI_Decode_PCX(Build_PCX(3, 2, 4, padded, palette), narrow), "a row padded past the picture decodes");
	Check(narrow.Width == 3 && narrow.Pixels.size() == 6 && narrow.Pixels[2] == 1 && narrow.Pixels[3] == 3, "and the padding never reaches the picture");

	std::vector<std::uint8_t> rgba;
	Check(UI_Indexed_To_RGBA(image, rgba) && rgba.size() == 32, "an indexed picture becomes four bytes a pixel");
	Check(rgba[0] == 255 && rgba[1] == 0 && rgba[2] == 0 && rgba[3] == 255, "an opaque pixel takes its palette color");

	UIImageIndexed keyed;
	keyed.Width = 1;
	keyed.Height = 1;
	keyed.Pixels = { 2 };
	std::memcpy(keyed.Palette, palette.data(), palette.size());
	Check(UI_Indexed_To_RGBA(keyed, rgba) && rgba[3] == 0 && rgba[0] == 0 && rgba[1] == 0 && rgba[2] == 0,
		  "a magenta pixel comes back clear, with no color left to bleed");

	UIImageIndexed rejected;
	Check(!UI_Decode_PCX(std::span<std::uint8_t const>(), rejected), "no bytes at all is not a picture");

	std::vector<std::uint8_t> notpcx = Build_PCX(4, 2, 4, compressed, palette);
	notpcx[0] = 11;
	Check(!UI_Decode_PCX(notpcx, rejected), "a file that is not a PCX is refused");

	std::vector<std::uint8_t> truecolor = Build_PCX(4, 2, 4, compressed, palette);
	truecolor[65] = 3;
	Check(!UI_Decode_PCX(truecolor, rejected), "so is a PCX of three planes, which this loader does not read");

	std::vector<std::uint8_t> nomarker = Build_PCX(4, 2, 4, compressed, palette);
	nomarker[nomarker.size() - 769] = 0;
	Check(!UI_Decode_PCX(nomarker, rejected), "a PCX without its palette marker is refused");

	// A picture whose rows run out of bytes before the header's height is met.
	std::vector<std::uint8_t> truncated = { 0xC4, 1 };
	Check(!UI_Decode_PCX(Build_PCX(4, 2, 4, truncated, palette), rejected), "a picture that claims more rows than it holds is refused");

	std::vector<std::uint8_t> emptyrun = { 0xC0, 1, 0xC4, 3 };
	Check(!UI_Decode_PCX(Build_PCX(4, 2, 4, emptyrun, palette), rejected), "a run of no pixels is refused rather than read forever");
}


// Builds a font sheet the way the dialog art is drawn: cells of one size in a grid, with a
// one-pixel margin above and to the left of every glyph. The first cell is filled solid,
// because that is the cell the margins and the glyph size are measured against and no
// character draws from it; every later cell inks `ink` columns, so its advance is known.
UIImageIndexed Build_Sheet(int glyphwidth, int glyphheight, int cellsperrow, int rows, int ink, std::uint8_t value)
{
	int cellw = glyphwidth + 1;
	int cellh = glyphheight + 1;

	UIImageIndexed sheet;
	sheet.Width = cellw * cellsperrow;
	sheet.Height = cellh * rows;
	sheet.Pixels.assign((std::size_t)sheet.Width * sheet.Height, 0);

	// Index 0 is empty and index 1 carries the value, so coverage reads off the red gun.
	sheet.Palette[0] = 0;
	sheet.Palette[3] = value;
	sheet.Palette[4] = value;
	sheet.Palette[5] = value;

	for (int cell = 0; cell < cellsperrow * rows; cell++) {
		int originx = (cell % cellsperrow) * cellw + 1;
		int originy = (cell / cellsperrow) * cellh + 1;
		int columns = (cell == 0) ? glyphwidth : ink;

		for (int y = 0; y < glyphheight; y++) {
			for (int x = 0; x < columns; x++) {
				sheet.Pixels[(std::size_t)(originy + y) * sheet.Width + originx + x] = 1;
			}
		}
	}

	return(sheet);
}


void Test_Sheet_Font(void)
{
	UIImageIndexed alpha = Build_Sheet(6, 8, 16, 16, 4, 255);
	UISheetFontMetrics metrics;

	Check(UI_Sheet_Font_Metrics(alpha, metrics), "a font measures off its alpha sheet");
	Check(metrics.TopMargin == 1 && metrics.LeftMargin == 1, "the blank margins around a cell are found");
	Check(metrics.GlyphWidth == 6 && metrics.GlyphHeight == 8, "and the inked area inside it");
	Check(metrics.CellsPerRow == 16 && metrics.Cell_Width() == 7 && metrics.Cell_Height() == 9, "a row holds as many cells as the sheet is wide");
	Check(metrics.Advance['A'] == 4, "a character advances by the columns it inks");

	// The cell for a character is the one after it, so 'A' at 65 sits in cell 66.
	int x = 0;
	int y = 0;
	Check(UI_Sheet_Font_Cell(metrics, alpha, 'A', x, y) && x == (66 % 16) * 7 && y == (66 / 16) * 9, "a character's cell is the one after it");

	// A sheet with only the first two rows of cells cannot hold the later ones.
	UIImageIndexed shallow = Build_Sheet(6, 8, 16, 2, 4, 255);
	UISheetFontMetrics shallowmetrics;
	Check(UI_Sheet_Font_Metrics(shallow, shallowmetrics), "a short sheet still measures");
	Check(!UI_Sheet_Font_Cell(shallowmetrics, shallow, 'A', x, y), "a character past the end of a sheet has no cell");
	Check(!UI_Sheet_Font_Cell(metrics, alpha, ' ', x, y) && !UI_Sheet_Font_Cell(metrics, alpha, '\t', x, y), "a space is moved over rather than drawn, whatever its cell holds");
	Check(shallowmetrics.Advance['A'] == 6 / 3 + 1, "and falls back to the blank advance, as a space does");

	UIImageIndexed blank = Build_Sheet(6, 8, 16, 16, 4, 255);
	blank.Pixels.assign(blank.Pixels.size(), 0);
	UISheetFontMetrics blankmetrics;
	Check(!UI_Sheet_Font_Metrics(blank, blankmetrics), "a sheet with nothing inked cannot be measured");

	Check(UI_Sheet_Font_Coverage(alpha, 0, 0) == 0, "the margin around a glyph covers nothing");
	Check(UI_Sheet_Font_Coverage(alpha, 8, 1) == 255, "an inked pixel covers fully");
	Check(UI_Sheet_Font_Coverage(alpha, -1, 0) == 0 && UI_Sheet_Font_Coverage(alpha, 0, 10000) == 0, "a pixel off the sheet covers nothing");

	// A palette shading one hue from dark to light: the remap must carry the shading over
	// rather than flatten it to the color asked for.
	std::uint8_t palette[768] = {};
	for (int entry = 0; entry < 256; entry++) {
		palette[entry * 3 + 1] = (std::uint8_t)entry;
	}

	std::uint8_t remapped[768] = {};
	UI_Sheet_Font_Remap(palette, 255, 0, 0, remapped);
	Check(remapped[0] == 0 && remapped[1] == 0 && remapped[2] == 0, "the darkest entry of a font's palette stays dark");
	Check(remapped[255 * 3] > remapped[128 * 3], "a lighter entry stays lighter, so the shading survives");

	std::uint8_t green[768] = {};
	UI_Sheet_Font_Remap(palette, 0, 255, 0, green);
	Check(std::memcmp(remapped, green, sizeof(remapped)) != 0, "asking for another color gives another palette");

	UIImageIndexed index = Build_Sheet(6, 8, 16, 16, 4, 255);
	for (std::size_t pixel = 0; pixel < index.Pixels.size(); pixel++) {
		if (index.Pixels[pixel] != 0) {
			index.Pixels[pixel] = 255;
		}
	}
	std::memcpy(index.Palette, palette, sizeof(palette));

	std::vector<std::uint8_t> atlas;
	Check(UI_Sheet_Font_Atlas(index, alpha, remapped, atlas) && atlas.size() == index.Pixels.size() * 4, "an atlas is four bytes a pixel");

	std::size_t inked = ((std::size_t)1 * index.Width + 8) * 4;
	std::size_t empty = 0;
	Check(atlas[inked + 3] == 255 && atlas[inked] == remapped[255 * 3], "an inked pixel takes its remapped color at full coverage");
	Check(atlas[empty + 3] == 0 && atlas[empty] == 0, "a pixel the sheet does not cover is clear, with no color left to bleed");

	UIImageIndexed mismatched = Build_Sheet(6, 8, 8, 16, 4, 255);
	Check(!UI_Sheet_Font_Atlas(mismatched, alpha, remapped, atlas), "two sheets of different sizes are not a font");
}


void Test_Magnify(void)
{
	// A two by two picture of four distinct pixels, three times over.
	std::uint8_t const picture[16] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };
	std::vector<std::uint8_t> result;

	Check(UI_Render_Magnify_RGBA(std::span<std::uint8_t const>(picture, 16), 2, 2, 3, result) && result.size() == 6 * 6 * 4, "a picture magnified three times over is nine times its size");
	bool repeated = result.size() == 144;
	for (int y = 0; y < 6 && repeated; y++) {
		for (int x = 0; x < 6 && repeated; x++) {
			std::uint8_t const * pixel = picture + ((y / 3) * 2 + (x / 3)) * 4;
			repeated = std::memcmp(result.data() + (y * 6 + x) * 4, pixel, 4) == 0;
		}
	}
	Check(repeated, "and every pixel of it is the one it came from, repeated");

	Check(UI_Render_Magnify_RGBA(std::span<std::uint8_t const>(picture, 16), 2, 2, 1, result) && result.size() == 16 && std::memcmp(result.data(), picture, 16) == 0, "one times over is the picture itself");

	Check(!UI_Render_Magnify_RGBA(std::span<std::uint8_t const>(picture, 12), 2, 2, 2, result) && result.empty(), "a picture short of its size is refused");
	Check(!UI_Render_Magnify_RGBA(std::span<std::uint8_t const>(picture, 16), 2, 2, 0, result), "as is a factor of nothing");
}


void Test_Reveal(void)
{
	// A 300 pixel dialog: 12 pixels a side a frame is 13 frames, and the original's last
	// deadline works out at 276 milliseconds.
	float const full = 300.0f;

	Check(UI_Reveal_Width(full, 1.0f, 0, 0.0f) == 24.0f, "a screen is one step open the moment it appears, 12 each side");
	Check(UI_Reveal_Width(full, 1.0f, 39, 24.0f) == 24.0f, "and stays there until the next frame falls due");
	Check(UI_Reveal_Width(full, 1.0f, 40, 24.0f) == 48.0f, "which opens it another 24");
	Check(UI_Reveal_Width(full, 1.0f, 10000, 24.0f) == 48.0f, "and no more than that however late the pass comes");
	Check(UI_Reveal_Width(full, 1.0f, 10000, full) == full, "a screen open stays open");

	// A pass every millisecond keeps to the schedule: thirteen frames at the first period
	// would take over half a second, and the second half of the reveal takes less time than
	// the first, which is the schedule tightening as it goes.
	int first = 0;
	int second = 0;
	float shown = 0.0f;
	float previous = 0.0f;
	bool stepped = true;
	for (int elapsed = 0; elapsed <= 1000; elapsed++) {
		shown = UI_Reveal_Width(full, 1.0f, elapsed, shown);
		if (shown < previous || shown - previous > 24.0f) {
			stepped = false;
		}
		previous = shown;
		if (first == 0 && shown >= full / 2.0f) {
			first = elapsed;
		}
		if (second == 0 && shown >= full) {
			second = elapsed - first;
		}
	}
	Check(first > 0 && second > 0 && second < first, "a screen opens its second half quicker than its first");
	Check(stepped && shown == full, "a screen only ever opens further, a step at a time, and ends open");
	Check(first + second > 250 && first + second < 300, "and a 300 pixel screen takes the quarter second the original took");

	// A pass every hundred milliseconds cannot keep to the schedule, so it draws the same
	// thirteen bands over a longer time rather than skipping to the last of them.
	int passes = 0;
	shown = 0.0f;
	while (shown < full && passes < 100) {
		shown = UI_Reveal_Width(full, 1.0f, passes * 100, shown);
		passes++;
	}
	Check(passes == 13, "a machine that cannot keep up shows every band of the reveal");

	// Twice the scale is twice the width at the same moment, and the same finish.
	Check(UI_Reveal_Width(full * 2.0f, 2.0f, 40, 48.0f) == 96.0f, "a screen drawn at twice the size opens twice as fast in pixels");
	shown = 0.0f;
	bool opening = false;
	for (int elapsed = 0; elapsed <= 300; elapsed++) {
		shown = UI_Reveal_Width(full * 2.0f, 2.0f, elapsed, shown);
		if (elapsed == 200) {
			opening = shown < full * 2.0f;
		}
	}
	Check(opening && shown == full * 2.0f, "and still takes the same time to open");

	Check(UI_Reveal_Width(0.0f, 1.0f, 0, 0.0f) == 0.0f, "a screen with no width is open already");
	Check(UI_Reveal_Width(full, 0.0f, 40, 24.0f) == 48.0f, "a scale of nothing is read as one to one");
	Check(UI_Reveal_Width(full, 1.0f, -5, 24.0f) == full, "a clock that went backwards opens the screen rather than hiding it");
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
	Test_Render_Transform();
	Test_Render_Mask();
	Test_Image();
	Test_Sheet_Font();
	Test_Reveal();
	Test_Magnify();
	Test_Dirty_State();

	std::printf("\n%s\n", Failures == 0 ? "PASSED" : "FAILED");
	return(Failures == 0 ? 0 : 1);
}
