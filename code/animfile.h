/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include "animate.h"
#include "palette.h"
#include "win.h"

#define MakeID(d,c,b,a) ((LONG)(a)<<24 | (LONG)(b)<<16 | (LONG)(c)<<8 | (LONG)(d) )

/*
 * This class plays back a Deluxe Paint ANM animation file. The format is the one described
 * by the Deluxe Paint SDK.
 */
class AnimFile : public Animate
{
	public:
		AnimFile(void);
		virtual ~AnimFile(void) override;

		int Get_Header_Size(void) const;

		virtual bool Load(FileClass *file) override;

		virtual void Reset(void) override;

		bool Process(void);

		bool Is_Valid(void) const;

		virtual Surface *Load_Frame(int frame) override;

		virtual PaletteClass &Get_Palette(void) override { return(Palette); }

		virtual int Get_Frame_Count(void) const override;
		virtual int Get_Width(void) const override;
		virtual int Get_Height(void) const override;

		int Large_Page_Size_In_File(int nLp) const;
		bool Load_Large_Page(int nLp);

		int Find_Large_Page_For_Record(int nRecord) const;
		void *Get_Large_Page_Pointer_For_Record(int nRecord);
		int Get_Large_Page_Size_For_Record(int nRecord);

		bool Decode_Frame(void *source, int size, void *dest, int dlength);

		enum {
			PALETTE_SIZE = 4 * 256,

			MAX_LARGE_PAGE_SIZE = (256 * 256),

			LARGE_PAGE_FILE_ID = MakeID('L','P','F',' '),
			ANIM_CONTENTS_ID = MakeID('A','N','I','M'),

			LPF_HEADER_HEAD_SIZE_IN_FILE = 256,

			LPF_TABLE_OFFSET = (LPF_HEADER_HEAD_SIZE_IN_FILE + PALETTE_SIZE),

			BBC_RUN_SKIP_DUMP = 1,
			MAX_RECORDS_PER_LP = 256,
			MAX_LARGE_PAGE = 256,
			MAX_RECORDS	= 65535,

			MAXNCYCS = 16,
		};

	private:
		struct LPFHeader {
			unsigned int id;                /// 4 character ID == "LPF "
			unsigned short maxLps;          /// max # largePages allowed. 256 FOR NOW.
			unsigned short nLps;            /// # largePages in this file.
			unsigned int nRecords;          /// # records in this file.  65534 is current limit plus
											/// one for last-to-first delta for looping the animation
			unsigned short maxRecsPerLp;	/// # records permitted in an lp. 256 FOR NOW.
			unsigned short lpfTableOffset;	/// Absolute Seek position of lpfTable.  1280 FOR NOW.
											/// The lpf Table is an array of 256 large page structures
											/// that is used to facilitate finding records in an anim
											/// file without having to seek through all of the Large
											/// Pages to find which one a specific record lives in.
			unsigned int contentType;       /// 4 character ID == "ANIM"
			unsigned short width;           /// Width of screen in pixels.
			unsigned short height;          /// Height of screen in pixels.
			unsigned char variant;          /// 0==ANIM.
			unsigned char version;          /// 0==frame rate is multiple of 18 cycles/sec.
											/// 1==frame rate is multiple of 70 cycles/sec.
			unsigned char hasLastDelta;		/// 1==Last record is a delta from last-to-first frame.
			unsigned char lastDeltaValid;	/// 0==The last-to-first delta (if present) hasn't been
											/// updated to match the current first&last frames, so it
											/// should be ignored.
			unsigned char pixelType;        /// 0==256 color.
			unsigned char highestBBComp;    /// 1==(RunSkipDump) Only one used FOR NOW.
			unsigned char otherRecsPerFrm;  /// 0 FOR NOW.
			unsigned char bitmaptype;       /// 1==320x200, 256-color.  Only one implemented so far.
			unsigned char recordTypes[32];  /// Not yet implemented.
			unsigned int FrameCount;        /// In case future version adds other records at end of
											/// file, we still know how many actual frames.
											/// NOTE: DOES include last-to-first delta when present.
											/// This field is named FrameCount here; the Deluxe Paint
											/// SDK calls it nFrames.
			unsigned short framesPerSecond;	/// Number of frames to play per second.
			unsigned short pad2[29];		/// 58 bytes of filler to round up to 128 bytes total.
		};
		static_assert(sizeof(LPFHeader) == 128, "the LPF header is 128 bytes on disk");

		struct LPDescriptor {
			unsigned short baseRecord;		/// Number of first record in this large page.
			unsigned short nRecords;        /// Number of records in lp.
											/// bit 15 of "nRecords" == "has continuation from previous lp".
											/// bit 14 of "nRecords" == "final record continues on next lp".
			unsigned short nBytes;          /// Total number of bytes of contents, excluding header.
		};
		static_assert(sizeof(LPDescriptor) == 6, "a large page descriptor is 6 bytes on disk");

		/*
		 * Structure of a single Large Page in an anim file.
		 */
		struct LPStruct {
			unsigned short baseRecord;		/// Number of first record in this large page.
			unsigned short nRecords;        /// Number of records in lp.
											/// bit 15 of "nRecords" == "has continuation from previous lp".
											/// bit 14 of "nRecords" == "final record continues on next lp".
			unsigned short nBytes;          /// Total number of bytes of contents, excluding header.

			unsigned short BytesContinued;	/// The number of bytes of the last record of the
											/// previous large page that extend into this large page.
											/// This was never implemented and is always 0.

			unsigned short RecordSizes[1];	/// [nRecords] Array of lengths of each record in the large page.

		};
		static_assert(sizeof(LPStruct) == 10, "a large page header is 8 bytes on disk, and the struct carries the first of the record sizes that follow it");

		/*
		 * These are the color cycling ranges Deluxe Paint stores after the header, each
		 * naming a span of palette entries (low to high), how fast it rotates and how many
		 * times. Nothing here performs the cycling -- they are read in to be skipped over.
		 */
		struct {
			short count;
			short rate;
			short flags;
			unsigned char low, high;
		} Cycles[MAXNCYCS];

		/*
		 * This is the file the animation is being played from. It is borrowed from whoever
		 * called Load rather than owned, so it must outlive this object's use of it.
		 */
		FileClass *File;

		/*
		 * If the header has been read and the playback buffers allocated, then this flag
		 * will be true. No frame can be fetched until it is set.
		 */
		bool Processed;

		/*
		 * If this object was the one that opened the file, then this flag will be true and
		 * Reset will close it again. A file handed over already open is left as it was found.
		 */
		bool FileOpen;

		/*
		 * This is the header read from the front of the animation file. It carries the frame
		 * count, the screen dimensions and the layout of the large pages holding the frames.
		 */
		LPFHeader Header;

		/*
		 * This is the palette exactly as it was read from the file, four bytes per
		 * entry. Process converts it into Palette; it is kept only because its size
		 * forms part of the header size reported by Get_Header_Size.
		 */
		unsigned char FilePalette[256][4];

		/*
		 * This is the animation's palette, converted from the raw entries read into
		 * FilePalette. Callers reach it through the Get_Palette function.
		 */
		PaletteClass Palette;

		/*
		 * This is the large page table read from the file. It records which records live in
		 * which large page, so a frame can be found without seeking through the whole file.
		 */
		LPDescriptor LPFTable[256];

		/*
		 * This is the surface the frames are composed onto. Frames are stored as deltas
		 * against the frame before, so the surface carries the picture forward from one
		 * frame to the next and is what Load_Frame hands back.
		 */
		Surface *SurfacePtr;

		/*
		 * This is the working buffer that one large page is read into at a time. Records are
		 * fetched by pointing into it, so it holds only the most recently loaded page.
		 */
		void *BufferPtr;

		/*
		 * This is the frame currently composed onto the surface. Load_Frame walks forward
		 * from here applying deltas, and starts over from a cleared surface (-1) when the
		 * frame asked for lies behind it.
		 */
		int LastFrame;

		/*
		 * This is the large page currently resident in the buffer, or -1 if none has been
		 * read yet. Asking for another record out of the same page then costs no file access.
		 */
		int LastLargePage;
};
