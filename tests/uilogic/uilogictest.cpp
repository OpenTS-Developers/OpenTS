/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "gametext.h"
#include "ui/rml/rmlfontfnt.h"
#include "ui/rml/rmlfontsheet.h"
#include "ui/rml/rmlimage.h"
#include "ui/rml/rmlrendermath.h"
#include "ui/uiinput.h"
#include "ui/uireveal.h"
#include "videodirty.h"

#include "doctest/doctest.h"

#include <array>
#include <cstdint>
#include <cstring>
#include <limits>
#include <span>
#include <vector>

namespace {

TEST_CASE("a press keeps the owner it started with, and closing a screen suppresses what the toolkits held")
{
	UIInputStateClass state;

	CHECK_MESSAGE(state.Gesture_Owner() == UI_INPUT_NONE, "the initial state holds no gesture");
	CHECK_MESSAGE(!state.Has_UI_Mouse(), "the initial state holds no gesture");
	CHECK_MESSAGE(!state.Any_Owned(), "the initial state holds no gesture");
	CHECK_MESSAGE(state.Press_Key(65, UI_INPUT_RML) == UI_INPUT_RML, "RmlUi owns the key it first pressed");
	CHECK_MESSAGE(state.Any_Owned(), "a held key counts as owned input");
	CHECK_MESSAGE(state.Press_Key(65, UI_INPUT_IMGUI) == UI_INPUT_RML, "a repeat keeps the first owner of a key");
	CHECK_MESSAGE(state.Release_Key(65) == UI_INPUT_RML, "a release returns the owner of its press");
	CHECK_MESSAGE(state.Release_Key(65) == UI_INPUT_NONE, "a second release has no owner");
	CHECK_MESSAGE(state.Press_Key(65, UI_INPUT_GAME) == UI_INPUT_GAME, "a later press of the same key can belong to the game");
	CHECK_MESSAGE(state.Press_Key(256, UI_INPUT_RML) == UI_INPUT_NONE, "a key out of range takes no owner");
	CHECK_MESSAGE(state.Key_Owner(256) == UI_INPUT_NONE, "a key out of range takes no owner");
	CHECK_MESSAGE(state.Release_Key(256) == UI_INPUT_NONE, "a release out of range is ignored");

	CHECK_MESSAGE(state.Press_Mouse(0, UI_INPUT_IMGUI) == UI_INPUT_IMGUI, "ImGui owns the button it first pressed");
	CHECK_MESSAGE(state.Press_Mouse(0, UI_INPUT_GAME) == UI_INPUT_IMGUI, "a move across the game cannot change a press's owner");
	CHECK_MESSAGE(state.Release_Mouse(0) == UI_INPUT_IMGUI, "a UI release stays a UI release outside the panel");
	CHECK_MESSAGE(state.Press_Mouse(0, UI_INPUT_GAME) == UI_INPUT_GAME, "the next press can go to the game");
	CHECK_MESSAGE(state.Press_Mouse(0, UI_INPUT_RML) == UI_INPUT_GAME, "a game drag cannot become a document drag");
	CHECK_MESSAGE(state.Release_Mouse(0) == UI_INPUT_GAME, "a game release keeps its owner");
	CHECK_MESSAGE(state.Press_Mouse(0, UI_INPUT_RML) == UI_INPUT_RML, "a fresh press can belong to RmlUi");
	CHECK_MESSAGE(state.Press_Mouse(1, UI_INPUT_RML) == UI_INPUT_RML, "a second held button joins the UI gesture");
	CHECK_MESSAGE(state.Release_Mouse(0) == UI_INPUT_RML, "releasing one button keeps the other's capture");
	CHECK_MESSAGE(state.Has_UI_Mouse(), "releasing one button keeps the other's capture");
	CHECK_MESSAGE(state.Gesture_Owner() == UI_INPUT_RML, "the remaining button owns the pointer's motion");
	CHECK_MESSAGE(state.Release_Mouse(1) == UI_INPUT_RML, "the capture ends with the last button");
	CHECK_MESSAGE(!state.Has_UI_Mouse(), "the capture ends with the last button");
	CHECK_MESSAGE(state.Press_Mouse(5, UI_INPUT_IMGUI) == UI_INPUT_NONE, "a button out of range is ignored");
	CHECK_MESSAGE(state.Release_Mouse(5) == UI_INPUT_NONE, "a button out of range is ignored");

	state.Press_Key(66, UI_INPUT_IMGUI);
	state.Press_Mouse(0, UI_INPUT_RML);
	state.Press_Mouse(2, UI_INPUT_GAME);
	CHECK_MESSAGE(!state.Any_Suppressed(), "nothing is suppressed before a cancel");
	state.Cancel_UI();
	CHECK_MESSAGE(state.Key_Owner(65) == UI_INPUT_GAME, "closing a screen leaves the game's presses alone");
	CHECK_MESSAGE(state.Mouse_Owner(2) == UI_INPUT_GAME, "closing a screen leaves the game's presses alone");
	CHECK_MESSAGE(state.Key_Owner(66) == UI_INPUT_SUPPRESSED, "closing a screen suppresses what the toolkits held");
	CHECK_MESSAGE(state.Mouse_Owner(0) == UI_INPUT_SUPPRESSED, "closing a screen suppresses what the toolkits held");
	CHECK_MESSAGE(state.Any_Suppressed(), "suppressed input is reported");
	CHECK_MESSAGE(state.Press_Key(66, UI_INPUT_GAME) == UI_INPUT_SUPPRESSED, "a repeat of a suppressed key stays suppressed");
	CHECK_MESSAGE(state.Release_Key(66) == UI_INPUT_SUPPRESSED, "a suppressed release never reaches the game");
	CHECK_MESSAGE(state.Release_Mouse(0) == UI_INPUT_SUPPRESSED, "a suppressed release never reaches the game");
	CHECK_MESSAGE(state.Press_Key(66, UI_INPUT_GAME) == UI_INPUT_GAME, "a fresh press works once the suppression ended");
	state.Cancel_All();
	CHECK_MESSAGE(state.Release_Key(65) == UI_INPUT_SUPPRESSED, "losing focus suppresses every held key");
	CHECK_MESSAGE(state.Release_Key(66) == UI_INPUT_SUPPRESSED, "losing focus suppresses every held key");
	CHECK_MESSAGE(state.Release_Mouse(2) == UI_INPUT_SUPPRESSED, "losing focus suppresses a game-owned button");
	state.Press_Key(67, UI_INPUT_RML);
	state.Press_Mouse(3, UI_INPUT_IMGUI);
	state.Reset();
	CHECK_MESSAGE(state.Key_Owner(67) == UI_INPUT_NONE, "a reset forgets every owner");
	CHECK_MESSAGE(state.Mouse_Owner(3) == UI_INPUT_NONE, "a reset forgets every owner");
	CHECK_MESSAGE(state.Gesture_Owner() == UI_INPUT_NONE, "a reset forgets every owner");
	CHECK_MESSAGE(!state.Any_Owned(), "a reset forgets every owner");
}


TEST_CASE("reconciling a cancelled capture forgets only what is up, and delivery follows the owner of the press")
{
	UIInputStateClass state;
	std::array<bool, UIInputStateClass::BUTTON_COUNT> released {};

	state.Press_Mouse(0, UI_INPUT_GAME);
	state.Reconcile_Cancelled_Mouse(released);
	CHECK_MESSAGE(state.Gesture_Owner() == UI_INPUT_GAME, "a physical release cannot forget a game gesture before its up message");
	CHECK_MESSAGE(!UI_Consumes_Input(state.Gesture_Owner()), "a game drag's motion stays the game's over a document");
	CHECK_MESSAGE(!UI_Consumes_Input(state.Release_Mouse(0)), "a game release reaches the game after the physical release");
	state.Press_Mouse(0, UI_INPUT_IMGUI);
	state.Reconcile_Cancelled_Mouse(released);
	CHECK_MESSAGE(state.Gesture_Owner() == UI_INPUT_IMGUI, "a physical release cannot hand a UI gesture to the game");
	CHECK_MESSAGE(UI_Consumes_Input(state.Release_Mouse(0)), "a physical release cannot hand a UI gesture to the game");

	state.Press_Key(70, UI_INPUT_RML);
	state.Press_Mouse(0, UI_INPUT_GAME);
	state.Press_Mouse(1, UI_INPUT_GAME);
	state.Cancel_Mouse();
	CHECK_MESSAGE(state.Key_Owner(70) == UI_INPUT_RML, "losing the capture leaves the keys alone");
	std::array<bool, UIInputStateClass::BUTTON_COUNT> oneheld {};
	oneheld[1] = true;
	state.Reconcile_Cancelled_Mouse(oneheld);
	CHECK_MESSAGE(state.Mouse_Owner(0) == UI_INPUT_NONE, "only a cancelled button that is up is forgotten");
	CHECK_MESSAGE(state.Mouse_Owner(1) == UI_INPUT_SUPPRESSED, "only a cancelled button that is up is forgotten");
	CHECK_MESSAGE(!UI_Consumes_Input(state.Release_Mouse(0)), "a release the shell forgot after reconciliation is the game's");
	CHECK_MESSAGE(UI_Consumes_Input(state.Release_Mouse(1)), "a cancelled button still held keeps its release");

	std::array<bool, UIInputStateClass::KEY_COUNT> keysup {};
	state.Press_Key(71, UI_INPUT_IMGUI);
	state.Cancel_All();
	keysup[70] = true;
	state.Reconcile_Cancelled_Keys(keysup);
	CHECK_MESSAGE(state.Key_Owner(70) == UI_INPUT_SUPPRESSED, "only a cancelled key that is up is forgotten");
	CHECK_MESSAGE(state.Key_Owner(71) == UI_INPUT_NONE, "only a cancelled key that is up is forgotten");
	CHECK_MESSAGE(!UI_Consumes_Input(UI_INPUT_GAME), "delivery follows the owner of the press, not the pointer's position");
	CHECK_MESSAGE(UI_Consumes_Input(UI_INPUT_RML), "delivery follows the owner of the press, not the pointer's position");
	CHECK_MESSAGE(UI_Consumes_Input(UI_INPUT_SUPPRESSED), "delivery follows the owner of the press, not the pointer's position");
	CHECK_MESSAGE(!UI_Consumes_Input(UI_INPUT_NONE), "input nobody owns is the game's");
}


TEST_CASE("a render byte count is the whole product, and fails on overflow or an empty allocation")
{
	std::uint32_t bytes = 0;
	CHECK_MESSAGE(UI_Render_Byte_Count(10, 24, bytes), "a byte count is the whole product");
	CHECK_MESSAGE(bytes == 240, "a byte count is the whole product");
	CHECK_MESSAGE(!UI_Render_Byte_Count(std::numeric_limits<std::size_t>::max(), 2, bytes), "a byte count that overflows fails");
	CHECK_MESSAGE(bytes == 0, "a byte count that overflows fails");
	CHECK_MESSAGE(!UI_Render_Byte_Count(0, 4, bytes), "an empty allocation fails");
}


TEST_CASE("render indices must form whole triangles over the vertices")
{
	int const valid[] = { 2, 0, 1 };
	int const negative[] = { -1, 0, 1 };
	int const outside[] = { 0, 1, 3 };
	int const incomplete[] = { 0, 1 };
	CHECK_MESSAGE(UI_Render_Index_Range(valid, 3), "whole triangles over the vertices pass");
	CHECK_MESSAGE(!UI_Render_Index_Range(negative, 3), "a negative index fails");
	CHECK_MESSAGE(!UI_Render_Index_Range(outside, 3), "an index past the last vertex fails");
	CHECK_MESSAGE(!UI_Render_Index_Range(incomplete, 3), "an incomplete triangle fails");
}


TEST_CASE("a scissor rounds outward and clips into the target, and one that is outside, empty, non-finite or wrapping draws nothing")
{
	UIRenderClip clip;
	CHECK_MESSAGE(UI_Render_Clip_Rect(-2.4f, 1.2f, 12.1f, 25.7f, 100, 50, 10, 20, clip), "a fractional scissor rounds outward, clips, and lands in the target");
	CHECK_MESSAGE(clip.X == 100, "a fractional scissor rounds outward, clips, and lands in the target");
	CHECK_MESSAGE(clip.Y == 51, "a fractional scissor rounds outward, clips, and lands in the target");
	CHECK_MESSAGE(clip.Width == 10, "a fractional scissor rounds outward, clips, and lands in the target");
	CHECK_MESSAGE(clip.Height == 19, "a fractional scissor rounds outward, clips, and lands in the target");
	CHECK_MESSAGE(!UI_Render_Clip_Rect(-10.0f, 0.0f, -1.0f, 10.0f, 100, 50, 10, 20, clip), "a scissor outside the viewport draws nothing");
	CHECK_MESSAGE(!UI_Render_Clip_Rect(3.0f, 0.0f, 3.0f, 10.0f, 0, 0, 10, 20, clip), "an empty scissor draws nothing");
	CHECK_MESSAGE(!UI_Render_Clip_Rect(0.0f, 0.0f, std::numeric_limits<float>::infinity(), 10.0f, 0, 0, 10, 20, clip), "a non-finite scissor fails");
	CHECK_MESSAGE(!UI_Render_Clip_Rect(0.0f, 0.0f, 10.0f, 10.0f, 65530, 0, 10, 20, clip), "a scissor cannot wrap the 16-bit target coordinates");
}


TEST_CASE("a rectangle copied out of a pitched image packs tightly, and one past the image fails")
{
	std::array<std::uint8_t, 48> pixels;
	pixels.fill(0xEE);
	for (int row = 0; row < 3; row++) {
		for (int column = 0; column < 12; column++) {
			pixels[(std::size_t)row * 16 + column] = (std::uint8_t)(row * 12 + column);
		}
	}
	std::vector<std::uint8_t> packed;
	CHECK_MESSAGE(UI_Render_Copy_RGBA_Rect(pixels, 3, 3, 16, 1, 1, 2, 2, packed), "a rectangle out of a pitched image packs tightly");
	REQUIRE_MESSAGE(packed.size() == 16, "a rectangle out of a pitched image packs tightly");
	CHECK_MESSAGE(packed[0] == 16, "a rectangle out of a pitched image packs tightly");
	CHECK_MESSAGE(packed[8] == 28, "a rectangle out of a pitched image packs tightly");
	CHECK_MESSAGE(!UI_Render_Copy_RGBA_Rect(pixels, 3, 3, 16, 2, 2, 2, 2, packed), "a rectangle past the image fails");
	CHECK_MESSAGE(!UI_Render_Copy_RGBA_Rect(pixels, 3, 4, 16, 0, 0, 1, 1, packed), "an image larger than its bytes fails");
}


TEST_CASE("a fragment with no transform travels by its translation alone")
{
	float model[16];

	UI_Render_Model_Matrix(nullptr, 10.0f, 20.0f, model);
	CHECK_MESSAGE(model[0] == 1.0f, "a fragment with no transform travels by its translation alone");
	CHECK_MESSAGE(model[5] == 1.0f, "a fragment with no transform travels by its translation alone");
	CHECK_MESSAGE(model[10] == 1.0f, "a fragment with no transform travels by its translation alone");
	CHECK_MESSAGE(model[15] == 1.0f, "a fragment with no transform travels by its translation alone");
	CHECK_MESSAGE(model[12] == 10.0f, "a fragment with no transform travels by its translation alone");
	CHECK_MESSAGE(model[13] == 20.0f, "a fragment with no transform travels by its translation alone");
}


TEST_CASE("a rotated fragment is translated in the rotated frame, and the transform's columns are left alone")
{
	float model[16];
	float rotation[16] = {};
	rotation[1] = 1.0f;
	rotation[4] = -1.0f;
	rotation[10] = 1.0f;
	rotation[15] = 1.0f;
	UI_Render_Model_Matrix(rotation, 10.0f, 20.0f, model);
	CHECK_MESSAGE(model[12] == -20.0f, "a rotated fragment is translated in the rotated frame, not the screen's");
	CHECK_MESSAGE(model[13] == 10.0f, "a rotated fragment is translated in the rotated frame, not the screen's");
	CHECK_MESSAGE(model[1] == 1.0f, "and the transform's columns are left alone");
	CHECK_MESSAGE(model[4] == -1.0f, "and the transform's columns are left alone");
}


TEST_CASE("a scaling transform scales the fragment's translation and keeps its own")
{
	float model[16];
	float scaled[16] = {};
	scaled[0] = 2.0f;
	scaled[5] = 2.0f;
	scaled[10] = 1.0f;
	scaled[12] = 5.0f;
	scaled[13] = 6.0f;
	scaled[15] = 1.0f;
	UI_Render_Model_Matrix(scaled, 10.0f, 20.0f, model);
	CHECK_MESSAGE(model[12] == 25.0f, "a scaling transform scales the fragment's translation and keeps its own");
	CHECK_MESSAGE(model[13] == 46.0f, "a scaling transform scales the fragment's translation and keeps its own");
}


TEST_CASE("a perspective row reaches the fragment's translation too")
{
	float model[16];
	float perspective[16] = {};
	perspective[0] = 1.0f;
	perspective[3] = 0.01f;
	perspective[5] = 1.0f;
	perspective[10] = 1.0f;
	perspective[15] = 1.0f;
	UI_Render_Model_Matrix(perspective, 10.0f, 20.0f, model);
	CHECK_MESSAGE(model[15] > 1.09f, "a perspective row reaches the fragment's translation too");
	CHECK_MESSAGE(model[15] < 1.11f, "a perspective row reaches the fragment's translation too");
}


TEST_CASE("each mask operation gives its stencil step, and nesting past eight bits is refused")
{
	UIRenderMaskStep step;

	CHECK_MESSAGE(UI_Render_Mask_Step(UI_RENDER_MASK_SET, 7, step), "a mask that starts over clears whatever was there and keeps its own shape");
	CHECK_MESSAGE(step.Clear, "a mask that starts over clears whatever was there and keeps its own shape");
	CHECK_MESSAGE(!step.Increment, "a mask that starts over clears whatever was there and keeps its own shape");
	CHECK_MESSAGE(step.Write == 1, "a mask that starts over clears whatever was there and keeps its own shape");
	CHECK_MESSAGE(step.Reference == 1, "a mask that starts over clears whatever was there and keeps its own shape");
	CHECK_MESSAGE(UI_Render_Mask_Step(UI_RENDER_MASK_SET_INVERSE, 7, step), "an inverted mask writes the same shape and keeps everything but it");
	CHECK_MESSAGE(step.Clear, "an inverted mask writes the same shape and keeps everything but it");
	CHECK_MESSAGE(!step.Increment, "an inverted mask writes the same shape and keeps everything but it");
	CHECK_MESSAGE(step.Write == 1, "an inverted mask writes the same shape and keeps everything but it");
	CHECK_MESSAGE(step.Reference == 0, "an inverted mask writes the same shape and keeps everything but it");
	CHECK_MESSAGE(UI_Render_Mask_Step(UI_RENDER_MASK_INTERSECT, 1, step), "a narrowing mask counts up from what is already there");
	CHECK_MESSAGE(!step.Clear, "a narrowing mask counts up from what is already there");
	CHECK_MESSAGE(step.Increment, "a narrowing mask counts up from what is already there");
	CHECK_MESSAGE(step.Reference == 2, "a narrowing mask counts up from what is already there");
	CHECK_MESSAGE(UI_Render_Mask_Step(UI_RENDER_MASK_INTERSECT, 0, step), "and counts from nothing as well");
	CHECK_MESSAGE(step.Reference == 1, "and counts from nothing as well");
	CHECK_MESSAGE(!UI_Render_Mask_Step(UI_RENDER_MASK_INTERSECT, 255, step), "masks nested past what eight bits count are refused");
}


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


TEST_CASE("a run-length PCX, an uncompressed one, and one with padded rows decode to the same pixels and palette")
{
	std::vector<std::uint8_t> palette = { 0, 0, 0, 255, 0, 0, 255, 0, 255, 255, 255, 255 };

	std::vector<std::uint8_t> compressed = { 0xC4, 1, 0xC4, 3 };
	UIImageIndexed image;
	CHECK_MESSAGE(UI_Decode_PCX(Build_PCX(4, 2, 4, compressed, palette), image), "an 8-bit run-length PCX decodes");
	CHECK_MESSAGE(image.Width == 4, "and reports the size its header claims");
	CHECK_MESSAGE(image.Height == 2, "and reports the size its header claims");
	CHECK_MESSAGE(image.Pixels.size() == 8, "and reports the size its header claims");
	CHECK_MESSAGE(image.Pixels[0] == 1, "a run fills its whole row");
	CHECK_MESSAGE(image.Pixels[3] == 1, "a run fills its whole row");
	CHECK_MESSAGE(image.Pixels[4] == 3, "a run fills its whole row");
	CHECK_MESSAGE(image.Pixels[7] == 3, "a run fills its whole row");
	CHECK_MESSAGE(image.Palette[3] == 255, "and the palette comes off the end of the file");
	CHECK_MESSAGE(image.Palette[4] == 0, "and the palette comes off the end of the file");
	CHECK_MESSAGE(image.Palette[5] == 0, "and the palette comes off the end of the file");

	std::vector<std::uint8_t> literal = { 1, 1, 1, 1, 3, 3, 3, 3 };
	UIImageIndexed plain;
	CHECK_MESSAGE(UI_Decode_PCX(Build_PCX(4, 2, 4, literal, palette), plain), "an uncompressed PCX decodes");
	CHECK_MESSAGE(plain.Pixels == image.Pixels, "and holds the same pixels the compressed one does");

	std::vector<std::uint8_t> padded = { 1, 1, 1, 2, 3, 3, 3, 2 };
	UIImageIndexed narrow;
	CHECK_MESSAGE(UI_Decode_PCX(Build_PCX(3, 2, 4, padded, palette), narrow), "a row padded past the picture decodes");
	CHECK_MESSAGE(narrow.Width == 3, "and the padding never reaches the picture");
	REQUIRE_MESSAGE(narrow.Pixels.size() == 6, "and the padding never reaches the picture");
	CHECK_MESSAGE(narrow.Pixels[2] == 1, "and the padding never reaches the picture");
	CHECK_MESSAGE(narrow.Pixels[3] == 3, "and the padding never reaches the picture");
}


TEST_CASE("an indexed picture becomes four bytes a pixel, and a magenta pixel comes back clear")
{
	std::vector<std::uint8_t> palette = { 0, 0, 0, 255, 0, 0, 255, 0, 255, 255, 255, 255 };
	std::vector<std::uint8_t> compressed = { 0xC4, 1, 0xC4, 3 };
	UIImageIndexed image;
	REQUIRE(UI_Decode_PCX(Build_PCX(4, 2, 4, compressed, palette), image));

	std::vector<std::uint8_t> rgba;
	CHECK_MESSAGE(UI_Indexed_To_RGBA(image, rgba), "an indexed picture becomes four bytes a pixel");
	CHECK_MESSAGE(rgba.size() == 32, "an indexed picture becomes four bytes a pixel");
	CHECK_MESSAGE(rgba[0] == 255, "an opaque pixel takes its palette color");
	CHECK_MESSAGE(rgba[1] == 0, "an opaque pixel takes its palette color");
	CHECK_MESSAGE(rgba[2] == 0, "an opaque pixel takes its palette color");
	CHECK_MESSAGE(rgba[3] == 255, "an opaque pixel takes its palette color");

	UIImageIndexed keyed;
	keyed.Width = 1;
	keyed.Height = 1;
	keyed.Pixels = { 2 };
	std::memcpy(keyed.Palette, palette.data(), palette.size());
	CHECK_MESSAGE(UI_Indexed_To_RGBA(keyed, rgba), "a magenta pixel comes back clear, with no color left to bleed");
	CHECK_MESSAGE(rgba[3] == 0, "a magenta pixel comes back clear, with no color left to bleed");
	CHECK_MESSAGE(rgba[0] == 0, "a magenta pixel comes back clear, with no color left to bleed");
	CHECK_MESSAGE(rgba[1] == 0, "a magenta pixel comes back clear, with no color left to bleed");
	CHECK_MESSAGE(rgba[2] == 0, "a magenta pixel comes back clear, with no color left to bleed");
}


TEST_CASE("a PCX that is empty, of another kind, without its palette, short of its rows, or with an empty run is refused")
{
	std::vector<std::uint8_t> palette = { 0, 0, 0, 255, 0, 0, 255, 0, 255, 255, 255, 255 };
	std::vector<std::uint8_t> compressed = { 0xC4, 1, 0xC4, 3 };
	UIImageIndexed rejected;
	CHECK_MESSAGE(!UI_Decode_PCX(std::span<std::uint8_t const>(), rejected), "no bytes at all is not a picture");

	std::vector<std::uint8_t> notpcx = Build_PCX(4, 2, 4, compressed, palette);
	notpcx[0] = 11;
	CHECK_MESSAGE(!UI_Decode_PCX(notpcx, rejected), "a file that is not a PCX is refused");

	std::vector<std::uint8_t> truecolor = Build_PCX(4, 2, 4, compressed, palette);
	truecolor[65] = 3;
	CHECK_MESSAGE(!UI_Decode_PCX(truecolor, rejected), "so is a PCX of three planes, which this loader does not read");

	std::vector<std::uint8_t> nomarker = Build_PCX(4, 2, 4, compressed, palette);
	nomarker[nomarker.size() - 769] = 0;
	CHECK_MESSAGE(!UI_Decode_PCX(nomarker, rejected), "a PCX without its palette marker is refused");

	std::vector<std::uint8_t> truncated = { 0xC4, 1 };
	CHECK_MESSAGE(!UI_Decode_PCX(Build_PCX(4, 2, 4, truncated, palette), rejected), "a picture that claims more rows than it holds is refused");

	std::vector<std::uint8_t> emptyrun = { 0xC0, 1, 0xC4, 3 };
	CHECK_MESSAGE(!UI_Decode_PCX(Build_PCX(4, 2, 4, emptyrun, palette), rejected), "a run of no pixels is refused rather than read forever");
}


UIImageIndexed Build_Sheet(int glyphwidth, int glyphheight, int cellsperrow, int rows, int ink, std::uint8_t value)
{
	int cellw = glyphwidth + 1;
	int cellh = glyphheight + 1;

	UIImageIndexed sheet;
	sheet.Width = cellw * cellsperrow;
	sheet.Height = cellh * rows;
	sheet.Pixels.assign((std::size_t)sheet.Width * sheet.Height, 0);

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


TEST_CASE("a font measures its margins, cells and advances off its alpha sheet, and finds a character's cell")
{
	UIImageIndexed alpha = Build_Sheet(6, 8, 16, 16, 4, 255);
	UISheetFontMetrics metrics;

	CHECK_MESSAGE(UI_Sheet_Font_Metrics(alpha, metrics), "a font measures off its alpha sheet");
	CHECK_MESSAGE(metrics.TopMargin == 1, "the blank margins around a cell are found");
	CHECK_MESSAGE(metrics.LeftMargin == 1, "the blank margins around a cell are found");
	CHECK_MESSAGE(metrics.GlyphWidth == 6, "and the inked area inside it");
	CHECK_MESSAGE(metrics.GlyphHeight == 8, "and the inked area inside it");
	CHECK_MESSAGE(metrics.CellsPerRow == 16, "a row holds as many cells as the sheet is wide");
	CHECK_MESSAGE(metrics.Cell_Width() == 7, "a row holds as many cells as the sheet is wide");
	CHECK_MESSAGE(metrics.Cell_Height() == 9, "a row holds as many cells as the sheet is wide");
	CHECK_MESSAGE(metrics.Advance['A'] == 4, "a character advances by the columns it inks");

	int x = 0;
	int y = 0;
	CHECK_MESSAGE(UI_Sheet_Font_Cell(metrics, alpha, 'A', x, y), "a character's cell is the one after it");
	CHECK_MESSAGE(x == (66 % 16) * 7, "a character's cell is the one after it");
	CHECK_MESSAGE(y == (66 / 16) * 9, "a character's cell is the one after it");
}


TEST_CASE("a character past the end of a sheet, or a space, has no cell and takes the blank advance")
{
	UIImageIndexed alpha = Build_Sheet(6, 8, 16, 16, 4, 255);
	UISheetFontMetrics metrics;
	REQUIRE(UI_Sheet_Font_Metrics(alpha, metrics));
	int x = 0;
	int y = 0;

	UIImageIndexed shallow = Build_Sheet(6, 8, 16, 2, 4, 255);
	UISheetFontMetrics shallowmetrics;
	CHECK_MESSAGE(UI_Sheet_Font_Metrics(shallow, shallowmetrics), "a short sheet still measures");
	CHECK_MESSAGE(!UI_Sheet_Font_Cell(shallowmetrics, shallow, 'A', x, y), "a character past the end of a sheet has no cell");
	CHECK_MESSAGE(!UI_Sheet_Font_Cell(metrics, alpha, ' ', x, y), "a space is moved over rather than drawn, whatever its cell holds");
	CHECK_MESSAGE(!UI_Sheet_Font_Cell(metrics, alpha, '\t', x, y), "a space is moved over rather than drawn, whatever its cell holds");
	CHECK_MESSAGE(shallowmetrics.Advance['A'] == 6 / 3 + 1, "and falls back to the blank advance, as a space does");
}


TEST_CASE("a sheet with nothing inked cannot be measured")
{
	UIImageIndexed blank = Build_Sheet(6, 8, 16, 16, 4, 255);
	blank.Pixels.assign(blank.Pixels.size(), 0);
	UISheetFontMetrics blankmetrics;
	CHECK_MESSAGE(!UI_Sheet_Font_Metrics(blank, blankmetrics), "a sheet with nothing inked cannot be measured");
}


TEST_CASE("a glyph covers fully where it is inked and nothing in its margin or off the sheet")
{
	UIImageIndexed alpha = Build_Sheet(6, 8, 16, 16, 4, 255);
	CHECK_MESSAGE(UI_Sheet_Font_Coverage(alpha, 0, 0) == 0, "the margin around a glyph covers nothing");
	CHECK_MESSAGE(UI_Sheet_Font_Coverage(alpha, 8, 1) == 255, "an inked pixel covers fully");
	CHECK_MESSAGE(UI_Sheet_Font_Coverage(alpha, -1, 0) == 0, "a pixel off the sheet covers nothing");
	CHECK_MESSAGE(UI_Sheet_Font_Coverage(alpha, 0, 10000) == 0, "a pixel off the sheet covers nothing");
}


TEST_CASE("a font's palette is remapped to a color with its shading kept, and built into an atlas with clear uninked pixels")
{
	UIImageIndexed alpha = Build_Sheet(6, 8, 16, 16, 4, 255);
	std::uint8_t palette[768] = {};
	for (int entry = 0; entry < 256; entry++) {
		palette[entry * 3 + 1] = (std::uint8_t)entry;
	}

	std::uint8_t remapped[768] = {};
	UI_Sheet_Font_Remap(palette, 255, 0, 0, remapped);
	CHECK_MESSAGE(remapped[0] == 0, "the darkest entry of a font's palette stays dark");
	CHECK_MESSAGE(remapped[1] == 0, "the darkest entry of a font's palette stays dark");
	CHECK_MESSAGE(remapped[2] == 0, "the darkest entry of a font's palette stays dark");
	CHECK_MESSAGE(remapped[255 * 3] > remapped[128 * 3], "a lighter entry stays lighter, so the shading survives");

	std::uint8_t green[768] = {};
	UI_Sheet_Font_Remap(palette, 0, 255, 0, green);
	CHECK_MESSAGE(std::memcmp(remapped, green, sizeof(remapped)) != 0, "asking for another color gives another palette");

	UIImageIndexed index = Build_Sheet(6, 8, 16, 16, 4, 255);
	for (std::size_t pixel = 0; pixel < index.Pixels.size(); pixel++) {
		if (index.Pixels[pixel] != 0) {
			index.Pixels[pixel] = 255;
		}
	}
	std::memcpy(index.Palette, palette, sizeof(palette));

	std::vector<std::uint8_t> atlas;
	CHECK_MESSAGE(UI_Sheet_Font_Atlas(index, alpha, remapped, atlas), "an atlas is four bytes a pixel");
	CHECK_MESSAGE(atlas.size() == index.Pixels.size() * 4, "an atlas is four bytes a pixel");

	std::size_t inked = ((std::size_t)1 * index.Width + 8) * 4;
	std::size_t empty = 0;
	CHECK_MESSAGE(atlas[inked + 3] == 255, "an inked pixel takes its remapped color at full coverage");
	CHECK_MESSAGE(atlas[inked] == remapped[255 * 3], "an inked pixel takes its remapped color at full coverage");
	CHECK_MESSAGE(atlas[empty + 3] == 0, "a pixel the sheet does not cover is clear, with no color left to bleed");
	CHECK_MESSAGE(atlas[empty] == 0, "a pixel the sheet does not cover is clear, with no color left to bleed");

	UIImageIndexed mismatched = Build_Sheet(6, 8, 8, 16, 4, 255);
	CHECK_MESSAGE(!UI_Sheet_Font_Atlas(mismatched, alpha, remapped, atlas), "two sheets of different sizes are not a font");
}


TEST_CASE("a magnified picture repeats every pixel it came from, and a short picture or a factor of nothing is refused")
{
	std::uint8_t const picture[16] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };
	std::vector<std::uint8_t> result;

	CHECK_MESSAGE(UI_Render_Magnify_RGBA(std::span<std::uint8_t const>(picture, 16), 2, 2, 3, result), "a picture magnified three times over is nine times its size");
	CHECK_MESSAGE(result.size() == 6 * 6 * 4, "a picture magnified three times over is nine times its size");
	bool repeated = result.size() == 144;
	for (int y = 0; y < 6 && repeated; y++) {
		for (int x = 0; x < 6 && repeated; x++) {
			std::uint8_t const * pixel = picture + ((y / 3) * 2 + (x / 3)) * 4;
			repeated = std::memcmp(result.data() + (y * 6 + x) * 4, pixel, 4) == 0;
		}
	}
	CHECK_MESSAGE(repeated, "and every pixel of it is the one it came from, repeated");

	CHECK_MESSAGE(UI_Render_Magnify_RGBA(std::span<std::uint8_t const>(picture, 16), 2, 2, 1, result), "one times over is the picture itself");
	REQUIRE_MESSAGE(result.size() == 16, "one times over is the picture itself");
	CHECK_MESSAGE(std::memcmp(result.data(), picture, 16) == 0, "one times over is the picture itself");

	CHECK_MESSAGE(!UI_Render_Magnify_RGBA(std::span<std::uint8_t const>(picture, 12), 2, 2, 2, result), "a picture short of its size is refused");
	CHECK_MESSAGE(result.empty(), "a picture short of its size is refused");
	CHECK_MESSAGE(!UI_Render_Magnify_RGBA(std::span<std::uint8_t const>(picture, 16), 2, 2, 0, result), "as is a factor of nothing");
}


TEST_CASE("a screen is one step open the moment it appears, and opens a step per frame due")
{
	float const full = 300.0f;

	CHECK_MESSAGE(UI_Reveal_Width(full, 1.0f, 0, 0.0f) == 24.0f, "a screen is one step open the moment it appears, 12 each side");
	CHECK_MESSAGE(UI_Reveal_Width(full, 1.0f, 39, 24.0f) == 24.0f, "and stays there until the next frame falls due");
	CHECK_MESSAGE(UI_Reveal_Width(full, 1.0f, 40, 24.0f) == 48.0f, "which opens it another 24");
	CHECK_MESSAGE(UI_Reveal_Width(full, 1.0f, 10000, 24.0f) == 48.0f, "and no more than that however late the pass comes");
	CHECK_MESSAGE(UI_Reveal_Width(full, 1.0f, 10000, full) == full, "a screen open stays open");
}


TEST_CASE("a screen opens a step at a time, its second half quicker, in the quarter second the original took")
{
	float const full = 300.0f;
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
	CHECK_MESSAGE(first > 0, "a screen opens its second half quicker than its first");
	CHECK_MESSAGE(second > 0, "a screen opens its second half quicker than its first");
	CHECK_MESSAGE(second < first, "a screen opens its second half quicker than its first");
	REQUIRE_MESSAGE(stepped, "a screen only ever opens further, a step at a time, and ends open");
	CHECK_MESSAGE(shown == full, "a screen only ever opens further, a step at a time, and ends open");
	CHECK_MESSAGE(first + second > 250, "and a 300 pixel screen takes the quarter second the original took");
	CHECK_MESSAGE(first + second < 300, "and a 300 pixel screen takes the quarter second the original took");
}


TEST_CASE("a machine that cannot keep up shows every band of the reveal")
{
	float const full = 300.0f;
	float shown = 0.0f;
	int passes = 0;
	shown = 0.0f;
	while (shown < full && passes < 100) {
		shown = UI_Reveal_Width(full, 1.0f, passes * 100, shown);
		passes++;
	}
	CHECK_MESSAGE(passes == 13, "a machine that cannot keep up shows every band of the reveal");
}


TEST_CASE("a screen drawn at twice the size opens twice as fast in pixels and in the same time")
{
	float const full = 300.0f;
	float shown = 0.0f;
	CHECK_MESSAGE(UI_Reveal_Width(full * 2.0f, 2.0f, 40, 48.0f) == 96.0f, "a screen drawn at twice the size opens twice as fast in pixels");
	shown = 0.0f;
	bool opening = false;
	for (int elapsed = 0; elapsed <= 300; elapsed++) {
		shown = UI_Reveal_Width(full * 2.0f, 2.0f, elapsed, shown);
		if (elapsed == 200) {
			opening = shown < full * 2.0f;
		}
	}
	REQUIRE_MESSAGE(opening, "and still takes the same time to open");
	CHECK_MESSAGE(shown == full * 2.0f, "and still takes the same time to open");
}


TEST_CASE("a screen with no width, a scale of nothing, or a clock that went backwards opens the screen")
{
	float const full = 300.0f;
	CHECK_MESSAGE(UI_Reveal_Width(0.0f, 1.0f, 0, 0.0f) == 0.0f, "a screen with no width is open already");
	CHECK_MESSAGE(UI_Reveal_Width(full, 0.0f, 40, 24.0f) == 48.0f, "a scale of nothing is read as one to one");
	CHECK_MESSAGE(UI_Reveal_Width(full, 1.0f, -5, 24.0f) == full, "a clock that went backwards opens the screen rather than hiding it");
}


TEST_CASE("a screen keeps its bars until the original's final wait ends")
{
	float const full = 300.0f;
	CHECK_MESSAGE(!UI_Reveal_Finished(306.0f, 1.0f, 285), "a screen its last band covers keeps its bars until the original's final wait ends");
	CHECK_MESSAGE(UI_Reveal_Finished(306.0f, 1.0f, 286), "a screen its last band covers keeps its bars until the original's final wait ends");
	CHECK_MESSAGE(!UI_Reveal_Finished(full, 1.0f, 275), "which ends with the last band when that band came after it");
	CHECK_MESSAGE(UI_Reveal_Finished(full, 1.0f, 276), "which ends with the last band when that band came after it");
	CHECK_MESSAGE(UI_Reveal_Finished(full * 2.0f, 2.0f, 276), "a screen drawn at twice the size finishes at the same time");
	CHECK_MESSAGE(!UI_Reveal_Finished(full * 2.0f, 2.0f, 275), "a screen drawn at twice the size finishes at the same time");
	CHECK_MESSAGE(UI_Reveal_Finished(full, 1.0f, -5), "a clock that went backwards, or a screen with no width, is finished at once");
	CHECK_MESSAGE(UI_Reveal_Finished(0.0f, 1.0f, 0), "a clock that went backwards, or a screen with no width, is finished at once");
}


TEST_CASE("the dirty marks decide what a present uploads, survive a refused present, and are cleared by a reset")

{
	VideoDirtyStateClass dirty;

	CHECK_MESSAGE(!dirty.Is_Dirty(), "nothing is owed at the start");

	dirty.Mark_Overlay();
	CHECK_MESSAGE(dirty.Is_Dirty(), "an overlay mark makes a present due");
	VideoDirtySnapshotType first = dirty.Consume();
	CHECK_MESSAGE(!first.Game, "the first present uploads the frame the renderer has never seen");
	CHECK_MESSAGE(first.Overlay, "the first present uploads the frame the renderer has never seen");
	CHECK_MESSAGE(first.Upload, "the first present uploads the frame the renderer has never seen");
	CHECK_MESSAGE(!dirty.Is_Dirty(), "consuming takes the marks");

	dirty.Upload_Completed();
	dirty.Mark_Overlay();
	VideoDirtySnapshotType overlay = dirty.Consume();
	CHECK_MESSAGE(overlay.Overlay, "an overlay-only present leaves the uploaded frame alone");
	CHECK_MESSAGE(!overlay.Upload, "an overlay-only present leaves the uploaded frame alone");

	dirty.Mark_Game();
	VideoDirtySnapshotType game = dirty.Consume();
	CHECK_MESSAGE(game.Game, "a game mark uploads the frame");
	CHECK_MESSAGE(game.Upload, "a game mark uploads the frame");

	dirty.Mark_Game();
	VideoDirtySnapshotType consumed = dirty.Consume();
	dirty.Mark_Overlay();
	CHECK_MESSAGE(dirty.Is_Dirty(), "a mark raised while a present runs survives it");
	dirty.Restore(consumed);
	VideoDirtySnapshotType restored = dirty.Consume();
	CHECK_MESSAGE(restored.Game, "restoring a refused present keeps the marks raised since");
	CHECK_MESSAGE(restored.Overlay, "restoring a refused present keeps the marks raised since");

	dirty.Restore(VideoDirtySnapshotType { false, false, false });
	CHECK_MESSAGE(dirty.Is_Dirty(), "a refused present with nothing marked is retried as an overlay present");
	CHECK_MESSAGE(dirty.Consume().Overlay, "a refused present with nothing marked is retried as an overlay present");

	dirty.Upload_Completed();
	dirty.Invalidate_Frame();
	VideoDirtySnapshotType invalidated = dirty.Consume();
	CHECK_MESSAGE(invalidated.Game, "a renderer that lost the frame gets it uploaded again");
	CHECK_MESSAGE(invalidated.Upload, "a renderer that lost the frame gets it uploaded again");

	dirty.Mark_Game();
	dirty.Reset();
	VideoDirtySnapshotType reset = dirty.Consume();
	CHECK_MESSAGE(!dirty.Is_Dirty(), "a reset forgets the marks and the upload");
	CHECK_MESSAGE(!reset.Game, "a reset forgets the marks and the upload");
	CHECK_MESSAGE(!reset.Overlay, "a reset forgets the marks and the upload");
	CHECK_MESSAGE(reset.Upload, "a reset forgets the marks and the upload");
}


struct FntGlyphSpec
{
	int Index = 0;
	int Width = 0;
	int FirstRow = 0;
	int Rows = 0;
	std::vector<std::uint8_t> Pixels;
};


void Put_Word(std::vector<std::uint8_t> & data, std::size_t at, int value)
{
	data[at] = (std::uint8_t)(value & 0xFF);
	data[at + 1] = (std::uint8_t)((value >> 8) & 0xFF);
}


// Lays a font out as the game's files are: header, info, offsets, widths, heights, then pixels.
std::vector<std::uint8_t> Build_Fnt(int count, int height, int maxwidth, bool newstyle, std::vector<FntGlyphSpec> const & glyphs)
{
	std::size_t const info = 14;
	std::size_t const offsets = info + 6;
	std::size_t const widths = offsets + (std::size_t)count * 2;
	std::size_t const heights = widths + (std::size_t)count;
	std::size_t const pixels = heights + (std::size_t)count * 2;

	std::vector<std::uint8_t> data(pixels + 1, 0);
	data[2] = newstyle ? 2 : 0;
	Put_Word(data, 4, (int)info);
	Put_Word(data, 6, (int)offsets);
	Put_Word(data, 8, (int)widths);
	Put_Word(data, 10, (int)pixels);
	Put_Word(data, 12, (int)heights);
	data[info + 4] = (std::uint8_t)height;
	data[info + 5] = (std::uint8_t)maxwidth;

	for (FntGlyphSpec const & glyph : glyphs) {
		std::size_t const start = data.size();
		Put_Word(data, offsets + (std::size_t)glyph.Index * 2, (int)(newstyle ? start - pixels : start));
		data[widths + (std::size_t)glyph.Index] = (std::uint8_t)glyph.Width;
		Put_Word(data, heights + (std::size_t)glyph.Index * 2, (glyph.Rows << 8) | glyph.FirstRow);

		if (newstyle) {
			data.insert(data.end(), glyph.Pixels.begin(), glyph.Pixels.end());
		} else {
			int const stride = (glyph.Width + 1) / 2;
			for (int y = 0; y < glyph.Rows; y++) {
				for (int x = 0; x < stride; x++) {
					std::uint8_t low = glyph.Pixels[(std::size_t)y * glyph.Width + x * 2];
					std::uint8_t high = (x * 2 + 1 < glyph.Width) ? glyph.Pixels[(std::size_t)y * glyph.Width + x * 2 + 1] : 0;
					data.push_back((std::uint8_t)(low | (high << 4)));
				}
			}
		}
	}

	return(data);
}


UIFntFont Read_Built_Fnt(std::vector<std::uint8_t> const & data)
{
	UIFntFont font;
	REQUIRE(UI_Read_Fnt(std::span<std::uint8_t const>(data.data(), data.size()), font));
	return(font);
}


TEST_CASE("a byte-per-pixel font reads each glyph's width, rows and pixel values, and a full table is Windows-1252")
{
	FntGlyphSpec glyph { 'A', 3, 1, 2, { 1, 2, 3, 11, 0, 15 } };
	UIFntFont font = Read_Built_Fnt(Build_Fnt(256, 6, 5, true, { glyph }));

	CHECK_MESSAGE(font.Height == 6, "the font's height and widest glyph come from its info block");
	CHECK_MESSAGE(font.MaxWidth == 5, "the font's height and widest glyph come from its info block");
	CHECK_MESSAGE(font.Windows1252, "a font with the full table is laid out as Windows-1252");
	REQUIRE(font.Glyphs.size() == 256);

	UIFntGlyph const & read = font.Glyphs['A'];
	CHECK_MESSAGE(read.Defined, "a glyph with pixel data is defined");
	CHECK_MESSAGE(read.Width == 3, "a glyph keeps its width, first row and row count");
	CHECK_MESSAGE(read.FirstRow == 1, "a glyph keeps its width, first row and row count");
	CHECK_MESSAGE(read.Rows == 2, "a glyph keeps its width, first row and row count");
	CHECK_MESSAGE(read.Pixels == glyph.Pixels, "every pixel value from 0 to 15 is kept as stored");
	CHECK_MESSAGE(!font.Glyphs['B'].Defined, "a printable glyph with no pixel data is not defined");
	CHECK_MESSAGE(font.Glyphs[' '].Defined, "the space and the control glyphs are always defined");
}


TEST_CASE("a nibble-packed font reads the low nibble first, and a short table is code page 437")
{
	FntGlyphSpec glyph { 'A', 3, 0, 2, { 1, 2, 3, 4, 5, 6 } };
	UIFntFont font = Read_Built_Fnt(Build_Fnt(219, 4, 3, false, { glyph }));

	CHECK_MESSAGE(!font.Windows1252, "a font that stops short of the full table follows code page 437");
	CHECK_MESSAGE(font.Glyphs['A'].Pixels == glyph.Pixels, "each byte holds two pixels, the left one in its low nibble");
}


TEST_CASE("a font that is short of a block or of a glyph's pixels is refused and left empty")
{
	FntGlyphSpec glyph { 'A', 4, 0, 4, std::vector<std::uint8_t>(16, 1) };
	std::vector<std::uint8_t> data = Build_Fnt(256, 4, 4, true, { glyph });

	UIFntFont font;
	std::vector<std::uint8_t> cut(data.begin(), data.end() - 1);
	CHECK_MESSAGE(!UI_Read_Fnt(std::span<std::uint8_t const>(cut.data(), cut.size()), font), "a glyph whose pixels run past the end is refused");
	CHECK_MESSAGE(font.Glyphs.empty(), "a refused font is left empty");

	std::vector<std::uint8_t> header(data.begin(), data.begin() + 10);
	CHECK_MESSAGE(!UI_Read_Fnt(std::span<std::uint8_t const>(header.data(), header.size()), font), "a file shorter than the header is refused");

	std::vector<std::uint8_t> crossed = data;
	Put_Word(crossed, 8, 20);
	CHECK_MESSAGE(!UI_Read_Fnt(std::span<std::uint8_t const>(crossed.data(), crossed.size()), font), "a width block that does not follow the offset block is refused");
}


TEST_CASE("a character finds its glyph in the font's code page, and one the font lacks draws as a question mark")
{
	std::vector<FntGlyphSpec> glyphs;
	for (int index : { (int)'A', (int)'?', 0x82, 0xCE, 0xE9 }) {
		glyphs.push_back(FntGlyphSpec { index, 1, 0, 1, { 1 } });
	}

	UIFntFont western = Read_Built_Fnt(Build_Fnt(256, 1, 1, true, glyphs));
	CHECK_MESSAGE(UI_Fnt_Glyph(western, U'A') == 'A', "a plain letter is its own glyph");
	CHECK_MESSAGE(UI_Fnt_Glyph(western, U'\u00E9') == 0xE9, "a Windows-1252 font finds e-acute at 0xE9");
	CHECK_MESSAGE(UI_Fnt_Glyph(western, U'B') == '?', "a glyph the font does not define draws as a question mark");
	CHECK_MESSAGE(UI_Fnt_Glyph(western, U'\u4E2D') == '?', "a character outside the code page draws as a question mark");
	CHECK_MESSAGE(UI_Fnt_Glyph(western, U'\n') == -1, "a control character has no glyph");

	glyphs.pop_back();
	UIFntFont metal = Read_Built_Fnt(Build_Fnt(219, 1, 1, true, glyphs));
	CHECK_MESSAGE(UI_Fnt_Glyph(metal, U'\u00E9') == 0x82, "a code page 437 font finds e-acute at 0x82");
	CHECK_MESSAGE(UI_Fnt_Glyph(metal, U'\u0153') == 0xCE, "a code page 437 font finds the oe ligature in Westwood's slot at 0xCE");
}


TEST_CASE("a font's spacing settles as the game's font class settles it")
{
	CHECK_MESSAGE(UI_Fnt_Spacing(6, 1, false) == 1, "a narrow font keeps the spacing it is given");
	CHECK_MESSAGE(UI_Fnt_Spacing(10, 1, true) == -1, "an outlined font is pulled two pixels closer");
	CHECK_MESSAGE(UI_Fnt_Spacing(16, 2, true) == 1, "a font sixteen pixels wide gains a pixel");
	CHECK_MESSAGE(UI_Fnt_Spacing(30, 3, false) == 5, "the gain counts the spacing already set");
}


TEST_CASE("a font's default colors follow the game's printing, with shades colored only for a gradient font")
{
	std::uint32_t const green = UI_Fnt_Color(0, 255, 0, 255);
	UIFntPalette plain;
	UI_Fnt_Default_Palette(green, false, plain);
	CHECK_MESSAGE(plain[UI_FNT_CLEAR] == 0, "pixel value 0 is always clear");
	CHECK_MESSAGE(plain[UI_FNT_TEXT] == green, "pixel value 1 takes the text color");
	CHECK_MESSAGE(plain[UI_FNT_SHADOW] == 0, "the shadow and outline values are clear unless asked for");
	CHECK_MESSAGE(plain[UI_FNT_OUTLINE] == 0, "the shadow and outline values are clear unless asked for");
	CHECK_MESSAGE(plain[11] == 0, "a plain font's shade values are clear");

	UIFntPalette gradient;
	UI_Fnt_Default_Palette(green, true, gradient);
	CHECK_MESSAGE(gradient[UI_FNT_FIRST_SHADE] == green, "a gradient font's shade values take the text color");
	CHECK_MESSAGE(gradient[UI_FNT_ROLES - 1] == green, "a gradient font's shade values take the text color");
	CHECK_MESSAGE(gradient[UI_FNT_SHADOW] == 0, "a gradient font's shadow value is still clear");
}


TEST_CASE("a font's atlas gives every glyph its own cell and colors each pixel value from the palette")
{
	FntGlyphSpec first { 'A', 2, 1, 3, { 1, 2, 3, 0, 1, 1 } };
	FntGlyphSpec second { 'B', 2, 0, 1, { 1, 1 } };
	UIFntFont font = Read_Built_Fnt(Build_Fnt(256, 3, 2, true, { first, second }));

	std::vector<UIFntCell> cells;
	int width = 0;
	int height = 0;
	REQUIRE(UI_Fnt_Layout(font, 4096, cells, width, height));
	CHECK_MESSAGE(cells.size() == font.Glyphs.size(), "every glyph has a cell");
	CHECK_MESSAGE(cells['B'].X - cells['A'].X == 3, "neighboring cells are a clear pixel apart");
	CHECK_MESSAGE(!UI_Fnt_Layout(font, 16, cells, width, height), "an atlas larger than the limit is refused");
	REQUIRE(UI_Fnt_Layout(font, 4096, cells, width, height));

	UIFntPalette palette {};
	palette[UI_FNT_TEXT] = UI_Fnt_Color(255, 0, 0, 255);
	palette[UI_FNT_SHADOW] = UI_Fnt_Color(0, 0, 255, 128);

	std::vector<std::uint8_t> rgba;
	REQUIRE(UI_Fnt_Atlas(font, cells, width, height, palette, rgba));
	REQUIRE(rgba.size() == (std::size_t)width * height * 4);

	auto pixel = [&](int x, int y) { return(rgba.data() + ((std::size_t)y * width + x) * 4); };
	std::uint8_t const * text = pixel(cells['A'].X, cells['A'].Y + 1);
	CHECK_MESSAGE(text[0] == 255, "a pixel of value 1 takes the text color, starting at the glyph's first row");
	CHECK_MESSAGE(text[3] == 255, "a pixel of value 1 takes the text color, starting at the glyph's first row");

	std::uint8_t const * shadow = pixel(cells['A'].X + 1, cells['A'].Y + 1);
	CHECK_MESSAGE(shadow[2] == 128, "a translucent color is stored premultiplied");
	CHECK_MESSAGE(shadow[3] == 128, "a translucent color is stored premultiplied");

	std::uint8_t const * outline = pixel(cells['A'].X, cells['A'].Y + 2);
	CHECK_MESSAGE(outline[3] == 0, "a value the palette leaves clear draws nothing");

	CHECK_MESSAGE(pixel(cells['A'].X, cells['A'].Y)[3] == 0, "the rows above a glyph's first row are clear");
	CHECK_MESSAGE(pixel(cells['A'].X + 1, cells['A'].Y + 3)[3] == 0, "a row below the font's height is clipped");

	std::vector<UIFntCell> fewer(cells.begin(), cells.end() - 1);
	CHECK_MESSAGE(!UI_Fnt_Atlas(font, fewer, width, height, palette, rgba), "cells that do not match the font are refused");
}


TEST_CASE("game text breaks its lines at an at sign, and a backslash writes an at sign or a backslash as written")
{
	CHECK_MESSAGE(Game_Text_Lines("Power@Low") == "Power\nLow", "an at sign breaks the line");
	CHECK_MESSAGE(Game_Text_Lines("mail\\@example") == "mail@example", "a backslash and an at sign give an at sign");
	CHECK_MESSAGE(Game_Text_Lines("C:\\\\@next") == "C:\\\nnext", "two backslashes give one, and the at sign after them still breaks the line");
	CHECK_MESSAGE(Game_Text_Lines("C:\\GAME\\") == "C:\\GAME\\", "any other backslash, a trailing one included, is kept");
	CHECK_MESSAGE(Game_Text_Lines("a\nb") == "a\nb", "a newline is kept");

	char text[] = "mail\\@a@b\\\\c\\d\\";
	Game_Text_Lines_In_Place(text);
	CHECK_MESSAGE(std::strcmp(text, "mail@a\nb\\c\\d\\") == 0, "the in-place form gives the same text");
	Game_Text_Lines_In_Place(nullptr);
}

}
