/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// Holds the depth and alpha buffers to their ring arithmetic: how far the ring spans, where
// a point within it lands, how an address that walks off either end folds back, what a run
// fill writes, and which entries a pan and an update reset.
//
// Every expectation is written as an absolute byte offset from the buffer base and every
// address crosses the API through a cast, so this file reads the same whether the buffers
// carry addresses as integers or as pointers. That is deliberate: it is what lets the same
// expectations be checked before and after that representation changes.
//
// Needs no game data.

#include <cstdint>
#include <cstdio>

#include "abuffer.h"
#include "point.h"
#include "rect.h"
#include "surface.h"
#include "zbuffer.h"

ABuffer * AlphaBuffer;

namespace {

int Failures = 0;
int Checked = 0;

/// The buffer covers this many entries. The width is even so that a full-width run takes
/// the aligned path through Set, and neither dimension divides the other.
int const BUFFER_W = 40;
int const BUFFER_H = 24;

int const BPP = 2;
int const SPAN = BUFFER_W * BUFFER_H * BPP;

unsigned short const MARKER = 0x1234;


void Check(bool passed, char const * what)
{
	Checked++;
	if (!passed) {
		std::printf("FAILED %s\n", what);
		Failures++;
	}
}


void Check_Offset(std::uintptr_t got, std::uintptr_t base, int expected, char const * what)
{
	Checked++;
	if (got - base != (std::uintptr_t)expected) {
		std::printf("FAILED %s: expected base+%d, got base+%lld\n",
			what, expected, (long long)(got - base));
		Failures++;
	}
}


/// The entries as the surface stores them. The ring begins at the surface base, so entry i
/// sits at base + i * BPP whatever the pan offset happens to be.
template<typename BufferT>
unsigned short * Entries(BufferT & buffer)
{
	unsigned short * entries = (unsigned short *)buffer.Get_Surface()->Lock();
	buffer.Get_Surface()->Unlock();
	return(entries);
}


template<typename BufferT>
int Count_Entries(BufferT & buffer, unsigned short value)
{
	unsigned short * entries = Entries(buffer);
	int count = 0;
	for (int i = 0; i < BUFFER_W * BUFFER_H; i++) {
		if (entries[i] == value) {
			count++;
		}
	}
	return(count);
}


template<typename BufferT>
void Fill_Marker(BufferT & buffer)
{
	buffer.Fill(MARKER);
}


/// <summary>
/// Runs the ring checks against one of the two buffer classes.
/// </summary>
/// <param name="name">The buffer's name, for the failure messages.</param>
/// <param name="reset">The value the class resets an exposed or updated entry to.</param>
template<typename BufferT>
void Run(char const * name, unsigned short reset)
{
	char what[160];

	BufferT buffer(Rect(0, 0, BUFFER_W, BUFFER_H));

	/// The API's own address type: an integer before the buffers held pointers, a pointer
	/// after. Every address below round trips through it so that both spellings compile.
	using Addr = decltype(buffer.Get_Buffer_End());

	std::uintptr_t const base = (std::uintptr_t)buffer.Get_Buffer_Offset(Point2D(0, 0));
	std::uintptr_t const end = (std::uintptr_t)buffer.Get_Buffer_End();

	std::snprintf(what, sizeof(what), "%s: the ring spans one entry per covered pixel", name);
	Check(end - base == (std::uintptr_t)SPAN, what);

	std::snprintf(what, sizeof(what), "%s: the buffer reports the width it covers", name);
	Check((int)buffer.Get_Buffer_Width() == BUFFER_W, what);

	/*
	 * An address inside the ring is left alone; one that has walked off either end is
	 * folded back by exactly the span.
	 */
	std::snprintf(what, sizeof(what), "%s: the start does not overflow", name);
	Check_Offset((std::uintptr_t)buffer.Wrap_Overflow((Addr)base), base, 0, what);

	std::snprintf(what, sizeof(what), "%s: the last entry does not overflow", name);
	Check_Offset((std::uintptr_t)buffer.Wrap_Overflow((Addr)(end - BPP)), base, SPAN - BPP, what);

	std::snprintf(what, sizeof(what), "%s: one past the end folds to the start", name);
	Check_Offset((std::uintptr_t)buffer.Wrap_Overflow((Addr)end), base, 0, what);

	std::snprintf(what, sizeof(what), "%s: an address past the end folds back by the span", name);
	Check_Offset((std::uintptr_t)buffer.Wrap_Overflow((Addr)(end + 6 * BPP)), base, 6 * BPP, what);

	std::snprintf(what, sizeof(what), "%s: the start does not underflow", name);
	Check_Offset((std::uintptr_t)buffer.Wrap_Underflow((Addr)base), base, 0, what);

	std::snprintf(what, sizeof(what), "%s: an address before the start folds to the end", name);
	Check_Offset((std::uintptr_t)buffer.Wrap_Underflow((Addr)(base - BPP)), base, SPAN - BPP, what);

	std::snprintf(what, sizeof(what), "%s: an address inside does not underflow", name);
	Check_Offset((std::uintptr_t)buffer.Wrap_Underflow((Addr)(base + 4 * BPP)), base, 4 * BPP, what);

	/*
	 * A point maps to its entry within the covered area. The pan offset is still zero here,
	 * so the mapping is the plain row major one.
	 */
	int const points[][2] = {{0, 0}, {1, 0}, {BUFFER_W - 1, 0}, {0, 1}, {7, 5}, {BUFFER_W - 1, BUFFER_H - 1}};
	for (int i = 0; i < (int)(sizeof(points) / sizeof(points[0])); i++) {
		int const x = points[i][0];
		int const y = points[i][1];
		std::snprintf(what, sizeof(what), "%s: point %d,%d maps to its entry", name, x, y);
		Check_Offset((std::uintptr_t)buffer.Get_Buffer_Offset(Point2D(x, y)), base,
			(y * BUFFER_W + x) * BPP, what);
	}

	/*
	 * Set writes a run of entries and does not wrap; the caller splits a run that would
	 * cross the end. A run that starts on an odd entry writes that one on its own to reach
	 * an int boundary before the wide loop takes over.
	 */
	struct SetCase {
		int Start;			/// Entry the run begins at.
		int Size;			/// Entries the run covers.
		int Written;		/// Entries the run actually writes.
		char const * Note;
	};

	/*
	 * The two runs that write less than they were asked for are the buffer's long standing
	 * behavior, not a mistake in the expectation: the tail write is skipped whenever the
	 * wide loop never ran, so a run of one aligned entry writes nothing at all.
	 */
	SetCase const setcases[] = {
		{0, 0, 0, "an empty run writes nothing"},
		{0, 1, 0, "a single aligned entry writes nothing"},
		{1, 1, 1, "a single unaligned entry writes one"},
		{0, 2, 2, "an aligned pair writes both"},
		{1, 2, 1, "an unaligned pair writes only the first"},
		{0, 3, 3, "an aligned odd run writes all of it"},
		{0, 4, 4, "an aligned even run writes all of it"},
		{1, 5, 5, "an unaligned odd run writes all of it"},
		{BUFFER_W * BUFFER_H - 4, 4, 4, "a run ending at the buffer end writes all of it"},
	};

	for (int i = 0; i < (int)(sizeof(setcases) / sizeof(setcases[0])); i++) {
		SetCase const & test = setcases[i];

		Fill_Marker(buffer);
		buffer.Set((Addr)(base + test.Start * BPP), test.Size, reset);

		unsigned short * entries = Entries(buffer);
		int written = 0;
		int outside = 0;
		for (int e = 0; e < BUFFER_W * BUFFER_H; e++) {
			if (entries[e] == MARKER) continue;
			written++;
			if (e < test.Start || e >= test.Start + test.Size) {
				outside++;
			}
		}

		std::snprintf(what, sizeof(what), "%s: %s", name, test.Note);
		Check(written == test.Written && outside == 0, what);
	}

	/*
	 * Update resets a region through the pan offset, splitting the run when it straddles
	 * the wrap point. Updating the whole covered area reaches every entry wherever the
	 * origin sits, so long as the split does not leave a tail of exactly one entry.
	 */
	int const pans[] = {0, 7, BUFFER_W, BUFFER_W + 3};
	for (int i = 0; i < (int)(sizeof(pans) / sizeof(pans[0])); i++) {
		BufferT panned(Rect(0, 0, BUFFER_W, BUFFER_H));
		panned.Pan(pans[i], 0, MARKER);
		panned.Fill(MARKER);
		panned.Update(Rect(0, 0, BUFFER_W, BUFFER_H));

		std::snprintf(what, sizeof(what), "%s: a full update after a pan of %d resets every entry",
			name, pans[i]);
		Check(Count_Entries(panned, reset) == BUFFER_W * BUFFER_H, what);
	}

	/*
	 * A pan of one entry puts the wrap one entry into the last row, so the tail of the split
	 * is a single entry beginning at the buffer start. Set does not write a lone entry that
	 * began aligned, which the run cases above pin directly, so that entry keeps its old
	 * value and the update leaves the buffer one short.
	 */
	BufferT tail(Rect(0, 0, BUFFER_W, BUFFER_H));
	tail.Pan(1, 0, MARKER);
	tail.Fill(MARKER);
	tail.Update(Rect(0, 0, BUFFER_W, BUFFER_H));

	std::snprintf(what, sizeof(what), "%s: an update whose split leaves one entry does not reset it", name);
	Check(Count_Entries(tail, reset) == BUFFER_W * BUFFER_H - 1, what);

	/*
	 * A pan slides the origin rather than moving the entries, so the point that was at the
	 * pan distance is the one that lands at the origin afterwards.
	 */
	struct PanCase {
		int X;
		int Y;
	};

	PanCase const pancases[] = {{1, 0}, {-1, 0}, {13, 0}, {-13, 0}, {0, 1}, {0, -1}, {0, 5}, {5, 3}, {-5, -3}};

	for (int i = 0; i < (int)(sizeof(pancases) / sizeof(pancases[0])); i++) {
		PanCase const & test = pancases[i];

		BufferT panned(Rect(0, 0, BUFFER_W, BUFFER_H));
		std::uintptr_t const panbase = (std::uintptr_t)panned.Get_Buffer_Offset(Point2D(0, 0));

		panned.Pan(test.X, test.Y, MARKER);

		int shift = (test.X + test.Y * BUFFER_W) * BPP;
		shift = ((shift % SPAN) + SPAN) % SPAN;

		std::snprintf(what, sizeof(what), "%s: a pan of %d,%d slides the origin", name, test.X, test.Y);
		Check_Offset((std::uintptr_t)panned.Get_Buffer_Offset(Point2D(0, 0)), panbase, shift, what);
	}

	/*
	 * A pan far enough to leave nothing worth keeping resets the whole buffer and returns
	 * the depth bias to the middle of its range.
	 */
	BufferT wholesale(Rect(0, 0, BUFFER_W, BUFFER_H));
	wholesale.Fill(MARKER);
	wholesale.Pan(BUFFER_W + 1, 0, reset);

	std::snprintf(what, sizeof(what), "%s: a pan past the width resets the whole buffer", name);
	Check(Count_Entries(wholesale, reset) == BUFFER_W * BUFFER_H, what);

	std::snprintf(what, sizeof(what), "%s: a wholesale pan returns the bias to the middle", name);
	Check((unsigned)wholesale.Get_Scroll() == 0x8000u, what);
}

} // namespace


int main(void)
{
	Run<ZBuffer>("depth", 0xFFFF);
	Run<ABuffer>("alpha", 0x007F);

	std::printf("%-52s %s\n", "Ring addressing in the depth and alpha buffers",
		Failures == 0 ? "ok" : "FAILED");
	std::printf("checked %d cases, %d mismatches\n", Checked, Failures);

	return(Failures == 0 ? 0 : 1);
}
