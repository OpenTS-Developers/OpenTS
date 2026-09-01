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

/****************************************************************************
*
*        C O N F I D E N T I A L -- W E S T W O O D  S T U D I O S
*
*----------------------------------------------------------------------------
*
* PROJECT
*     VQAPlay32 library. (32-Bit protected mode)
*
* FILE
*     drawer.c
*
* DESCRIPTION
*     Frame drawing and page flip control.
*
* PROGRAMMER
*     Bill Randolph
*     Denzil E. Long, Jr.
*
* DATE
*     June 26, 1995
*
*----------------------------------------------------------------------------
*
* PUBLIC
*     VQA_Configure_Drawer - Configure the drawer routines.
*
* PRIVATE
*     Select_Frame             - Selects frame to draw and preforms frame
*                                skip.
*     Prepare_Frame            - Process/Decompress frame information.
*     DrawFrame_Xmode          - Draws a frame directly to Xmode screen.
*     DrawFrame_XmodeBuf       - Draws a frame in Xmode format to a buffer.
*     DrawFrame_XmodeVRAM      - Draws a frame in Xmode with resident
*                                Codebook.
*     PageFlip_Xmode           - Page flip Xmode display.
*     DrawFrame_MCGA           - Draws a frame directly to MCGA screen.
*     PageFlip_MCGA            - Page flip MCGA display.
*     DrawFrame_MCGABuf        - Draws a frame in MCGA format to a buffer.
*     PageFlip_MCGABuf         - Page flip a buffered MCGA display.
*     DrawFrame_VESA640        - Draws a frame in VESA640 format.
*     DrawFrame_VESA320_32K    - Draws a frame to VESA320_32K screen.
*     DrawFrame_VESA320_32KBuf - Draws a frame in VESA320_32K format to a
*                                buffer.
*     PageFlip_VESA            - Page flip VESA display.
*     DrawFrame_Buffer         - Draw a frame to a buffer.
*     PageFlip_Nop             - Do nothing page flip.
*     UnVQ_Nop                 - Do nothing UnVQ.
*     Mask_Rect                - Sets non-drawable rectangle in image.
*     Mask_Pointers            - Mask vector pointer that are in the mask
*                                rectangle.
*
****************************************************************************/
#include	"vqaplayp.h"
#include	"unvq.h"
#include	"cmp.h"
#include	"vqapalette.h"
#include	<stdio.h>
#include	<string.h>
#include	"video.h"
#include    "../lcw.h"

//forward declarations
_STATIC long Select_Frame(VQAHandleP *vqap);
_STATIC void Prepare_Frame(VQAHandleP *vqap);

long DrawFrame_MCGABuf(VQAHandle *vqa);
long PageFlip_MCGABuf(VQAHandle *vqa);
long DrawFrame_MCGA(VQAHandle *vqa);
long PageFlip_MCGA(VQAHandle *vqa);

void __cdecl UnVQ_Nop(unsigned char *codebook, unsigned char *pointers, unsigned char *buffer, unsigned long blocksperrow, unsigned long numrows, unsigned long bufwidth);
long PageFlip_Nop(VQAHandle *vqa);

void VQA_SetTimer(VQAHandleP *vqap, long time);
void VQA_StepTimer(VQAHandleP *vqap, long step);
unsigned long VQA_GetTime(VQAHandleP *vqap);
_STATIC long VQA_SetCurrentFrameAsLast(VQAHandleP *vqap);
_STATIC void VQA_SetPreviousFrameNode(VQAHandleP *vqap);
_STATIC long VQA_CalcFramesSinceDrawn(VQAHandleP *vqap);

_STATIC long VQA_ComputeDesiredFrame(VQAHandleP *vqap, VQAConfig *config, VQADrawer *drawer, VQAFrameNode *frame, unsigned long time);

void VQA_DispatchFrameChunks(VQAHandleP *vqap, long frame);

/// <summary>
/// Invalidates the drawer's record of the last frame drawn.
/// This routine is used when the player seeks, so that playback progress is not
/// measured against a frame number from before the seek. A value of -1 means that
/// no frame has been drawn yet.
/// </summary>
/// <returns>Returns with VQAERR_NONE.</returns>
long VQA_ResetLastFrameNum(VQAHandle *vqa)
{
	VQAHandleP *vqap;
	VQADrawer *drawer;

	vqap = (VQAHandleP *)vqa;
	drawer = &vqap->Drawer;

	drawer->LastFrameNum = -1;
	return(VQAERR_NONE);
}



/****************************************************************************
*
* NAME
*     VQA_Configure_Drawer - Configure the drawer routines.
*
* SYNOPSIS
*     VQA_Configure_Drawer(VQA)
*
*     void VQA_Configure_Drawer(VQAHandleP *);
*
* FUNCTION
*     Configure the drawing system for the current movie and configuration
*     options.
*
* INPUTS
*     VQA - Pointer to private VQAHandle.
*
* RESULT
*     NONE
*
****************************************************************************/

long VQA_Configure_Drawer(VQAHandleP *vqap)
{
	VQAConfig *config;
	VQAHeader *header;
	VQADrawer *drawer;
	long      origin;
	long      blkdim;

	/* Dereference commonly used data members for quicker access. */
	drawer = &vqap->Drawer;
	header = &vqap->Header;

	/*-------------------------------------------------------------------------
	 * INITIALIZE THE UNVQ ROUTINE FOR THE SPECIFIED VIDEO MODE AND BLOCK SIZE.
	 *-----------------------------------------------------------------------*/

	/* Pre-compute commonly used values for speed. */
	drawer->BlocksPerRow = header->ImageWidth / header->BlockWidth;
	drawer->NumRows = header->ImageHeight / header->BlockHeight;
	drawer->NumBlocks = drawer->BlocksPerRow * drawer->NumRows;
	blkdim = BLOCK_DIM(header->BlockWidth, header->BlockHeight);

	/* Initialize draw routine vectors to a NOP routine in order to prevent
	 * a crash.
	 */
	vqap->UnVQ1 = UnVQ_Nop;
	vqap->UnVQ2 = UnVQ_Nop;
	vqap->Page_Flip = PageFlip_Nop;

	origin = (vqap->Config.DrawFlags & VQACFGF_ORIGIN);

	/* If the client specifies buffering then go ahead an set the unvq
	 * vector. All of the buffered modes use the same unvq routines.
	 */
	if (!(vqap->Config.DrawFlags & VQACFGF_BUFFER)) {
		vqap->Draw_Frame = DrawFrame_MCGA;
		vqap->Page_Flip = PageFlip_MCGA;
	} else {
		vqap->Draw_Frame = DrawFrame_MCGABuf;
		vqap->Page_Flip = PageFlip_MCGABuf;
	}

	switch (blkdim) {
		case BLOCK_4X2:
			switch (header->ColorMode) {
				case 0:
					vqap->UnVQ1 = UnVQ_4x2;

					if (header->Flags & VQAHDF_TRANS) {
						if (vqap->Config.DrawFlags & VQACFGF_NOTRANS) {
							vqap->UnVQ2 = UnVQ2_C0_4x2_KEY;
						} else {
							vqap->UnVQ2 = UnVQ2_C0_4x2_TRANS;
						}
					}
					break;

				//case 1: unhandled

				case 4:
					vqap->UnVQ1 = UnVQ1_C4_4x2;
					vqap->UnVQ2 = UnVQ2_C4_4x2;
					break;
			}
			break;

		case BLOCK_4X4:
			switch (header->ColorMode) {

				case 4:
					vqap->UnVQ1 = UnVQ1_C4_4x4;
					vqap->UnVQ2 = UnVQ2_C4_4x4;
					break;

				case 1:
					vqap->UnVQ1 = UnVQ1_C1_4x4;
					vqap->UnVQ2 = UnVQ2_C1_4x4;
					break;

				case 0:
					vqap->UnVQ1 = UnVQ_4x4;

					if (header->Flags & VQAHDF_TRANS) {
						if (vqap->Config.DrawFlags & VQACFGF_NOTRANS) {
							vqap->UnVQ2 = UnVQ2_C0_4x4_KEY;
						} else if (vqap->Config.DrawFlags & VQACFGF_HALFSIZE) {
							vqap->UnVQ2 = UnVQ2_C0_4x4_TRANS_HALF;
						} else {
							vqap->UnVQ2 = UnVQ2_C0_4x4_TRANS;
						}
					} else {
						if (vqap->Config.DrawFlags & VQACFGF_HALFSIZE) {
							vqap->UnVQ1 = UnVQ_4x4_HALF;
						}
					}
					break;

			}
			break;
	}

	return(VQAERR_NONE);
}


/****************************************************************************
*
* NAME
*     Select_Frame - Selects frame to draw and preforms frame skip.
*
* SYNOPSIS
*     Error = Select_Frame(VQA)
*
*     long Select_Frame(VQAHandleP *);
*
* FUNCTION
*     Select a frame to draw. This is were the frame skipping/delay is
*     performed.
*
* INPUTS
*     VQA - Pointer to private VQAHandle.
*
* RESULT
*     Error - 0 if successful, or VQAERR_??? error code.
*
****************************************************************************/

STATIC long Select_Frame(VQAHandleP *vqap)
{
	VQADrawer    *drawer;
	VQAConfig    *config;
	VQAFrameNode *curframe;
	long         desiredframe;
	// MEG 11.29.95 - changed from long to unsigned long
	unsigned long curtime;

	/* Dereference commonly used data members for quicker access. */
	config = &vqap->Config;
	drawer = &vqap->Drawer;
	curframe = drawer->CurFrame;

	/* Make sure the current frame is drawable. If the frame is not ready
	 * then we must wait for the loader to catch up.
	 */
	if ((curframe->Flags & VQAFRMF_LOADED) == 0) {
		drawer->WaitsOnLoader++;

		if ((drawer->Flags & VQADRWF_REPAINT) && !(vqap->Flags & VQADATF_LDONE)) {
			VQA_SetPreviousFrameNode(vqap);
			if ((drawer->Flags & VQADRWF_FORCEDRAW)) {
				drawer->Flags |= VQADRWF_HOLD;
				curframe->Flags |= VQAFRMF_HOLD;
			}
			return(VQAERR_NONE);
		} else {
			return(VQAERR_NOBUFFER);
		}
	}

	if ((drawer->Flags & VQADRWF_FORCEDRAW)) {
		VQA_SetCurrentFrameAsLast(vqap);
		drawer->Flags |= VQADRWF_HOLD;
		curframe->Flags |= VQAFRMF_HOLD;
		return(VQAERR_NONE);
	}

	/* If single stepping then return with the next frame.*/
	if (config->OptionFlags & VQAOPTF_STEP) {
		drawer->LastFrameNum = curframe->FrameNum;
		curtime = ((curframe->FrameNum * VQA_TIMETICKS) / config->DrawRate);
		VQA_SetTimer(vqap, curtime);
		return(VQAERR_NONE);
	}

	/* Find the frame # we should play (rounded to nearest frame): */
	curtime = VQA_GetTime(vqap);

	/* Handle the cases where the player is going so fast that it's not time
	 * to draw this frame yet.
	 *
	 * - If the Drawer is using a slower frame rate than the Loader, use a
	 *   delta-time-based wait; otherwise, use the frame number as the wait.
	 */

//	desiredframe = ((curtime * config->FrameRate) / VQA_TIMETICKS);
	// MEG MOD 06.22.95 - Should look for the desired frame to draw, not load,
	// right?
	long endtime = 0;
	if ( !(drawer->Flags & VQADRWF_STEP)) {
		endtime = drawer->LastFrameNum + 1;
	} else {
		endtime = drawer->LastFrameNum + 2;
	}

	if ((int)curtime < ((endtime * VQA_TIMETICKS) / config->DrawRate)) {
		if (drawer->Flags & VQADRWF_REPAINT) {
			if (VQA_ComputeDesiredFrame(vqap, config, drawer, curframe, curtime) == drawer->LastFrameNum && (curframe->Flags & VQAFRMF_HOLD)) {
				drawer->LastFrameNum = curframe->FrameNum;
				return(VQAERR_NONE);
			} else {
				drawer->LastFrameNum = curframe->FrameNum;
				drawer->Flags |= VQADRWF_HOLD;
				curframe->Flags |= VQAFRMF_HOLD;
				return(VQAERR_NONE);
			}
		} else {
			return(VQAERR_NOT_TIME);
		}
	}

	/* Determine the frame we must reach. */
	desiredframe = VQA_ComputeDesiredFrame(vqap, config, drawer, curframe, curtime);

	#if(VQAMONO_ON)
	drawer->DesiredFrame = desiredframe;
	#endif

	/* If frame skipping is disabled then draw every frame. */
	if (config->DrawFlags & VQACFGF_NOSKIP) {
		return(VQA_SetCurrentFrameAsLast(vqap));
	}

	/* Limit the number of frames that can be skipped in one shot. */
	long maxskip = (config->FrameRate / 3) - 1;
	if (maxskip < 0) {
		maxskip = 0;
	}

	/* Handle the case where the player is going too slow, so we have to skip
	 * some frames:
	 *
	 * - If this is a Key Frame, draw it
	 * - If this frame's # is less than what we're supposed to draw, skip it
	 *   (Because the 1st 'desiredframe' will be 0, FrameNum MUST be typecast
	 *   to signed WORD for the comparison; otherwise, the comparison uses
	 *   UWORDs, and the first frame is always skipped.)
	 * - If this is a palette-set frame, set the palette before skipping it
	 * - Loop until we get the frame we need, or there's no frames available
	 */
	while (curframe->FrameNum != desiredframe) {

		/* No frame available; return */
		//if ((curframe->Flags & VQAFRMF_LOADED) == 0) {
		//	return(VQAERR_NOBUFFER);
		//}

		/* Force drawing of a Key Frame */
		if (curframe->Flags & VQAFRMF_KEY) {
			break;
		}

		/* Don't skip too many frames at once. */
		if (VQA_CalcFramesSinceDrawn(vqap) > maxskip) {
			break;
		}

		/* Handle a palette in a skipped frame:
		 *
		 * - Stash the palette in Drawer.Palette_24
		 * - Set the Drawer.Flags VQADRWF_SETPAL bit, to tell the page-flip
		 *   routines that this palette must be set
		 */
		if (curframe->Flags & VQAFRMF_PALETTE) {

			/* Un-LCW if needed */
			if (curframe->Flags & VQAFRMF_PALCOMP) {
				curframe->PaletteSize = LCW_Uncomp((char *)curframe->Palette
						+ curframe->PalOffset, (char *)curframe->Palette,
						vqap->Max_Pal_Size);

				curframe->Flags &= ~VQAFRMF_PALCOMP;
			}

			/* Stash the palette */
			memcpy(drawer->Palette_24, curframe->Palette, curframe->PaletteSize);
			drawer->CurPalSize = curframe->PaletteSize;
			drawer->Flags |= VQADRWF_SETPAL;
		}

		/* Dispatch any pending frame events. */
		if (config->EventHandler != NULL) {
			if (curframe->Flags & VQAFRMF_LOOPED) {
				config->EventHandler((VQAHandle *)vqap, VQAEVENT_LOOPED, (void *)curframe->FrameNum, vqap->LoopID);
				curframe->Flags &= ~VQAFRMF_LOOPED;
			}

			if (curframe->Flags & VQAFRMF_LOOPJMP) {
				config->EventHandler((VQAHandle *)vqap, VQAEVENT_LOOPJUMP, (void *)curframe->FrameNum, vqap->LoopID);
				curframe->Flags &= ~VQAFRMF_LOOPJMP;
			}

			if (curframe->Flags & VQAFRMF_CHUNKS) {
				VQA_DispatchFrameChunks(vqap, curframe->FrameNum);
				curframe->Flags &= ~VQAFRMF_CHUNKS;
			}
		}

		/* Invoke callback with NULL screen ptr */
		if (config->DrawerCallback != NULL) {
			if ((config->DrawerCallback(NULL, curframe->FrameNum)) != 0) {
				return(VQAERR_EOF);
			}
		}

		/* Skip the frame */
		curframe->PrevFlags = curframe->Flags;
		curframe->Flags = 0L;
		curframe = curframe->Next;
		drawer->CurFrame = curframe;
		vqap->SkippedFrames++;

		/* If the next frame isn't loaded yet, wait on the loader. */
		if (!(curframe->Flags & VQAFRMF_LOADED)) {
			drawer->WaitsOnLoader++;

			if ((drawer->Flags & VQADRWF_REPAINT) && !(vqap->Flags & VQADATF_LDONE)) {
				VQA_SetPreviousFrameNode(vqap);
				return(VQAERR_NONE);
			}
			return(VQAERR_NOBUFFER);
		}
	}


	//drawer->LastFrame = curframe->FrameNum;
	//drawer->LastTime = curtime;

	return(VQA_SetCurrentFrameAsLast(vqap));
}


/// <summary>
/// Dispatches the custom data chunks attached to a frame.
/// The multi-frame and multi-sound chunk tables are searched for entries recorded
/// against the given frame, and every match is handed to the client's event handler
/// along with its chunk ID, buffer and size. This is the delivery side of the chunk
/// cache that the loader fills.
/// </summary>
/// <param name="frame">The frame number whose chunks are to be dispatched.</param>
void VQA_DispatchFrameChunks(VQAHandleP *vqap, long frame)
{
	unsigned long chunkid;
	unsigned long count1;
	unsigned long count2;
	int i;
	int j;

	VQAMFCInfo *mfci_info = &vqap->MFCInfo;
	VQAMFCInfo::HEADER *mfci_infohdr = &mfci_info->Header;

	count1 = mfci_infohdr->Count;
	for (i = 0; i < count1; i++) {
		chunkid = mfci_info->Table[i].ChunkID;
		VQAMFCInfo::DATA2::DATA *mfci_data2data = mfci_info->Data2[i].Data;
		count2 = mfci_info->Data2[i].Count;
		for (j = 0; j < count2; j++) {
			if (mfci_data2data[j].Frame == frame) {
				vqap->Config.EventHandler((VQAHandle *)vqap, chunkid, mfci_data2data[j].Buffer, mfci_data2data[j].Size);
			}
		}
	}

	VQAMSCInfo *msci_info = &vqap->MSCInfo;
	VQAMSCInfo::HEADER *msci_infohdr = &msci_info->Header;

	count1 = msci_infohdr->Count;
	for (i = 0; i < count1; i++) {
		chunkid = msci_info->Table[i].ChunkID;
		VQAMSCInfo::DATA2::DATA *msci_data2data = msci_info->Data2[i].Data;
		count2 = msci_info->Data2[i].Count;
		for (j = 0; j < count2; j++) {
			if (msci_data2data[j].Frame == frame) {
				vqap->Config.EventHandler((VQAHandle *)vqap, chunkid, msci_data2data[j].Buffer, msci_data2data[j].Size);
			}
		}
	}
}


/// <summary>
/// Determines which frame should be on screen at the given time.
/// The frame number follows from the playback clock and the frame rate. When the
/// movie is looping and the clock has run past the end of the loop, the timer is
/// wound back by the length of the loop so that playback continues seamlessly, and
/// the loop bounds staged by VQA_SetLoop_Internal are promoted to the active ones.
/// When frame skipping is disabled the timer is stalled instead, so that no frame
/// is passed over.
/// </summary>
/// <param name="time">The playback time to evaluate, in VQA_TIMETICKS.</param>
/// <returns>Returns with the frame number that should be drawn.</returns>
long VQA_ComputeDesiredFrame(VQAHandleP *vqap, VQAConfig *config, VQADrawer *drawer, VQAFrameNode *frame, unsigned long time)
{
	VQAHeader *header;
	long frame_rate;
	long frame_num;
	long result;
	bool crossed_loop;
	long loop_end;

	header = &vqap->Header;

	frame_rate = config->FrameRate;
	frame_num = frame->FrameNum;

	result = (time * frame_rate) / (int)VQA_TIMETICKS;

	if (drawer->Flags & VQADRWF_STEP) {
		result--;
	}

	crossed_loop = false;


	loop_end = vqap->LoopEndFrameMode2;

	if (vqap->Flags & VQADATF_LOOPED) {
		if ((vqap->Flags & VQADATF_LOOPJMP)) {
			loop_end = vqap->LoopEndFrame2;
		}

		while (true) {
			if ((config->DrawFlags & VQACFGF_NOSKIP) || result <= loop_end) {
				if (!(config->DrawFlags & VQACFGF_NOSKIP) || frame_num != vqap->LoopStartFrame0 || crossed_loop) {
					break;
				}
			}

			int step_frames = vqap->LoopStartFrame0 - loop_end;
			step_frames--;
			crossed_loop = step_frames == 0;
			VQA_StepTimer(vqap, VQA_TIMETICKS * step_frames / frame_rate);
			result = ((int)VQA_GetTime(vqap) * (int)frame_rate) / (int)VQA_TIMETICKS;
			crossed_loop = true;
			if (step_frames == 0) {
				break;
			}
		}
	}


	if (config->DrawFlags & VQACFGF_NOSKIP) {
		if (result > frame_num) {
			int step_frames = (frame_num - result);
			if (!(header->Flags & VQAHDF_AUDIO) || !(config->OptionFlags & VQAOPTF_AUDIO)) {
				VQA_StepTimer(vqap, VQA_TIMETICKS * step_frames / frame_rate);
			}
			result = frame_num;
			vqap->Flags |= VQADATF_FRAMESTALL;
		} else {
			vqap->Flags &= ~VQADATF_FRAMESTALL;
		}
	}

	if (crossed_loop) {
		if ((drawer->Flags & VQADRWF_STEP) && !(config->DrawFlags & VQACFGF_NOSKIP)) {
			result--;
			if (result < vqap->LoopStartFrame0) {
				result = loop_end;
			}
		}
		if (vqap->LoopEndFrameNormal != -1) {
			vqap->LoopStartFrame0 = vqap->LoopStartFrame1;
			vqap->LoopStartFrame1 = -1;
			vqap->LoopEndFrameMode2 = vqap->LoopEndFrameNormal;
			vqap->LoopEndFrameNormal = -1;
		}
		if (!(vqap->Flags & VQADATF_LOOPJMP)) {
			vqap->LoopStartFrame2 = vqap->LoopStartFrame0;
			vqap->LoopEndFrame2 = vqap->LoopEndFrameMode2;
		}
		vqap->Flags &= ~(VQADATF_LOOPED);
		vqap->Flags &= ~(VQADATF_LOOPJMP);
	}

	return(result);
}


/// <summary>
/// Determines how many frames have elapsed since one was last drawn.
/// The count is the forward distance from the last drawn frame to the current one,
/// or, when the current frame has already wrapped past the end of the loop, the
/// distance the long way around through the loop point. Where neither distance can
/// be measured the routine falls back to a third of a second's worth of frames.
/// This is what limits how many frames may be skipped in one go.
/// </summary>
/// <returns>Returns with the number of frames elapsed since the last draw.</returns>
long VQA_CalcFramesSinceDrawn(VQAHandleP *vqap)
{
	VQAConfig * config;
	VQADrawer * drawer;
	VQAFrameNode *curframe;
	long cur;
	long loop_end;
	int to_loop_end;
	int from_loop_start;
	int fallback;

	drawer = &vqap->Drawer;
	curframe = drawer->CurFrame;
	config = &vqap->Config;

	cur = curframe->FrameNum;
	loop_end = vqap->LoopEndFrame2;

	if (cur >= drawer->LastFrameNum && cur <= loop_end) {
		return cur - drawer->LastFrameNum;
	}

	fallback = config->FrameRate / 3;

	to_loop_end = loop_end - drawer->LastFrameNum;
	if (to_loop_end < 0) {
		return(fallback);
	}

	from_loop_start = curframe->FrameNum - vqap->LoopStartFrame0;
	if (from_loop_start < 0) {
		return fallback;
	}
	return to_loop_end + from_loop_start + 1;
}


/// <summary>
/// Records the current frame as the last one drawn.
/// This is the companion to VQA_ResetLastFrameNum, and is what advances the drawer's
/// notion of playback progress once a frame has been committed.
/// </summary>
/// <returns>Returns with VQAERR_NONE.</returns>
long VQA_SetCurrentFrameAsLast(VQAHandleP *vqap)
{
	VQADrawer * drawer;

	drawer = &vqap->Drawer;

	drawer->LastFrameNum = drawer->CurFrame->FrameNum;

	return(VQAERR_NONE);
}


/// <summary>
/// Backs the drawer up onto the previous frame in the circular frame list.
/// The frame flags stashed before they were cleared are restored, so that the frame
/// can be presented again. This routine is used when the display must be repainted
/// but the next frame has not been loaded yet -- the drawer holds on the frame it
/// already has rather than advancing onto an empty buffer.
/// </summary>
void VQA_SetPreviousFrameNode(VQAHandleP *vqap)
{
	VQADrawer * drawer;

	drawer = &vqap->Drawer;

	drawer->CurFrame = drawer->CurFrame->Prev;
	drawer->CurFrame->Flags = drawer->CurFrame->PrevFlags;
}


/****************************************************************************
*
* NAME
*     Prepare_Frame - Process/Decompress frame information.
*
* SYNOPSIS
*     Prepare_Frame(VQAData)
*
*     void Prepare_Frame(VQAData *);
*
* FUNCTION
*     Decompress and preprocess the various frame elements (codebook,
*     pointers, palette, etc...)
*
* INPUTS
*     VQAData - Pointer to VQAData structure.
*
* RESULT
*     NONE
*
****************************************************************************/

STATIC void Prepare_Frame(VQAHandleP *vqap)
{
	VQADrawer    *drawer;
	VQAFrameNode *curframe;
	VQACBNode    *codebook;

	/* Dereference commonly used data members for quicker access. */
	drawer = &vqap->Drawer;
	curframe = drawer->CurFrame;
	codebook = curframe->Codebook;

	if ((drawer->Flags & VQADRWF_STEP)) {
		return;
	}

	/* Decompress the codebook, if needed */
	if (codebook->Flags & VQACBF_CBCOMP) {

		/* Decompress the codebook. */
		void *cbbuffer = NULL;
		if (vqap->AltBufferFlags & VQAABUFF_ALTCB) {
			cbbuffer = vqap->AltCBBuffer;
			codebook->Flags |= VQACBF_ALTPTR;
		} else {
			cbbuffer = codebook->Buffer;
		}

		codebook->CodebookSize = LCW_Uncomp((char *)(codebook->Buffer + codebook->CBOffset), (char *)cbbuffer, vqap->CBBufferSize);

		/* Mark as uncompressed for the next time we use it */
		codebook->Flags &= (~VQACBF_CBCOMP);
	}

	/* Decompress the palette, if needed */
	if (curframe->Flags & VQAFRMF_PALCOMP) {
		curframe->PaletteSize = LCW_Uncomp((char *)curframe->Palette +
				curframe->PalOffset,(char *)curframe->Palette,vqap->Max_Pal_Size);

		/* Mark as uncompressed */
		curframe->Flags &= ~VQAFRMF_PALCOMP;
	}

	/* Decompress the pointer data, if needed */
	if (curframe->Flags & VQAFRMF_PTRCOMP) {
		void *ptrbuffer = NULL;
		if (vqap->AltBufferFlags & VQAABUFF_ALTPTR) {
			ptrbuffer = vqap->AltPtrBuffer;
			curframe->Flags |= VQAFRMF_ALTPTR;
		} else {
			ptrbuffer = curframe->Pointers;
		}
		LCW_Uncomp((char *)curframe->Pointers + curframe->PtrOffset,
				(char *)ptrbuffer, vqap->PtrBufferSize);

		/* Mark as uncompressed */
		curframe->Flags &= ~VQAFRMF_PTRCOMP;
	}
}


/// <summary>
/// Decodes a frame's vector data into the image buffer.
/// The client is given the opportunity to lock and supply the destination surface,
/// and is notified once whenever a new codebook has been loaded, before the frame's
/// codebook and pointer data are expanded by the decoder chosen for the movie's
/// block size and color mode. Where the codebook or pointer data was decompressed
/// into the alternate buffers, those are decoded from instead.
/// </summary>
/// <param name="frame">The frame to decode.</param>
void VQA_UnVQFrame(VQAHandleP *vqap, VQAFrameNode *frame)
{
	VQACBNode *codebk;
	VQAConfig    *config;
	VQADrawer    *drawer;

	vqap = (VQAHandleP *)vqap;
	config = &vqap->Config;
	drawer = &vqap->Drawer;
	codebk = frame->Codebook;

	if (!(drawer->Flags & VQADRWF_STEP)) {

		unsigned char * buf = NULL;

		if (config->EventHandler != NULL) {
			buf = (unsigned char *)config->EventHandler((VQAHandle *)vqap, VQAEVENT_LOCK, NULL, 0);
		}

		if (buf == NULL) {
			buf = drawer->ImageBuf;
		}

		if (buf == NULL) {
			return;
		}

		unsigned char * buffer = buf + drawer->ScreenOffset;
		long blocksperrow = drawer->BlocksPerRow;
		long numrows = drawer->NumRows;
		unsigned char * codebook;
		unsigned char * pointers;

		if ((frame->Flags & VQAFRMF_ALTPTR)) {
			pointers =  (unsigned char *)vqap->AltPtrBuffer;
		} else {
			pointers = frame->Pointers;
		}


		if ((codebk->Flags & VQACBF_ALTPTR)) {
			codebook =  (unsigned char *)vqap->AltCBBuffer;
		} else {
			codebook = codebk->Buffer;
		}

		if (config->EventHandler != NULL) {
			if ((codebk->Flags & VQACBF_CBFULL)) {
				codebk->Flags &= ~VQACBF_CBFULL;
				config->EventHandler((VQAHandle *)vqap, VQAEVENT_CODEBOOK, codebook, codebk->CodebookSize);
			}
		}

		if (!(frame->Flags & VQAFRMF_RSDCOMP)) {
			vqap->UnVQ1(codebook, pointers, buffer, blocksperrow, numrows, drawer->ImageWidth);
		} else {
			vqap->UnVQ2(codebook, pointers, buffer, blocksperrow, numrows, drawer->ImageWidth);
		}

		if (config->EventHandler != NULL) {
			config->EventHandler((VQAHandle *)vqap, VQAEVENT_UNLOCK, NULL, 0);
		}
	}
}


/****************************************************************************
*
* NAME
*     DrawFrame_MCGA - Draws a frame directly to MCGA screen.
*
* SYNOPSIS
*     Error = DrawFrame_MCGA(VQA)
*
*     long DrawFrame_MCGA(VQAHandle *);
*
* FUNCTION
*     Algorithm:
*       - Skip frames
*       - UnLCW frame
*       - Wait on Update_Enabled
*       - Set Update_Enabled
*       - Go to next frame
*       - User_Update:
*         set palette
*         UnVQ to screen
*
*     This function implements a sort of cooperative multitasking.  If the
*     Drawer hits a "wait state", where it has to wait for Update_Enabled
*     to toggle, it sets a flag and returns.  This flag is checked on entry
*     to see if we need to jump to the proper execution point. This should
*     improve performance on some platforms.
*
*     This routine handles small images by UnVQ'ing into the correct spot on
*     the screen.
*
* INPUTS
*     VQA - Pointer to VQAHandle.
*
* RESULT
*     Error - 0 if successful, or VQAERR_??? error code.
*
****************************************************************************/

long DrawFrame_MCGA(VQAHandle *vqa)
{
	VQAHandleP *vqap;
	VQADrawer    *drawer;
	VQAFrameNode *curframe;
	long         rc;

	/* Dereference commonly used data members for quicker access */
	vqap = (VQAHandleP *)vqa;
	drawer = &vqap->Drawer;

	/* Select the frame to draw. */
	rc = Select_Frame(vqap);
	if (rc != VQAERR_NONE) {
		return(rc);
	}

	/* Uncompress the frame data */
	Prepare_Frame(vqap);

	/* Dereference current frame for quicker access. */
	curframe = drawer->CurFrame;

	//VQA_UnVQFrame(vqap, curframe); // not called in non buf

	/* Tell the flipper which frame to use */
	vqap->Flipper.CurFrame = curframe;

	if (!(drawer->Flags & VQADRWF_HOLD)) {
		/* Move to the next frame */
		drawer->CurFrame = curframe->Next;
	}

	return(VQAERR_NONE);
}


/****************************************************************************
*
* NAME
*     PageFlip_MCGA - Page flip MCGA display.
*
* SYNOPSIS
*     PageFlip_MCGA(VQA)
*
*     long PageFlip_MCGA(VQAHandle *);
*
* FUNCTION
*     Since the MCGA mode only has one buffer, the drawing is actually done
*     at this point.
*
* INPUTS
*     VQA - Pointer to VQAHandle structure.
*
* RESULT
*     Error - 0 if successfull, otherwise VQAERR_???
*
****************************************************************************/

STATIC long PageFlip_MCGA(VQAHandle *vqa)
{
	VQAHandleP *vqap;
	VQADrawer     *drawer;
	VQAFrameNode  *curframe;
	VQAConfig     *config;
	unsigned char *pal;
	long          palsize;
	long          slowpal;

	/* Dereference commonly used data members for quicker access. */
	vqap = (VQAHandleP *)vqa;
	config = &vqap->Config;
	drawer = &vqap->Drawer;
	curframe = vqap->Flipper.CurFrame;

	/*-------------------------------------------------------------------------
	 * WAIT FOR THE VERTICAL BLANK TO SET THE PALETTE.
	 *-----------------------------------------------------------------------*/
	if ((curframe->Flags & VQAFRMF_PALETTE)
			|| (drawer->Flags & VQADRWF_SETPAL)) {

		pal = curframe->Palette;
		palsize = curframe->PaletteSize;
		slowpal = (config->OptionFlags & VQAOPTF_SLOWPAL) ? 1 : 0;


		/* Set the palette. */
		if (curframe->Flags & VQAFRMF_PALETTE) {


			/* Notify the client of the palette change. */
			if (config->EventHandler != NULL) {
				config->EventHandler(vqa, VQAEVENT_PALETTE, (void *)pal, (long)palsize);
			}
		}
		else if (drawer->Flags & VQADRWF_SETPAL) {
			drawer->Flags &= (~VQADRWF_SETPAL);


			/* Notify the client of the palette change. */
			if (config->EventHandler != NULL) {
				config->EventHandler(vqa, VQAEVENT_PALETTE, (void *)drawer->Palette_24, (long)drawer->CurPalSize);
			}
		}
	}

	/* Un-VQ the image */
	VQA_UnVQFrame((VQAHandleP *)vqa, curframe);

	/* Invoke user's callback routine */
	if (config->DrawerCallback != NULL) {
		if ((config->DrawerCallback(vqa, curframe->FrameNum)) != 0) {
			return(VQAERR_EOF);
		}
	}

	return(VQAERR_NONE);
}


/****************************************************************************
*
* NAME
*     DrawFrame_MCGABuf - Draws a frame in MCGA format to a buffer.
*
* SYNOPSIS
*     Error = DrawFrame_MCGABuf(VQA)
*
*     long DrawFrame_MCGABuf(VQAHandle *);
*
* FUNCTION
*     Algorithm:
*       - Skip frames
*       - UnLCW frame
*       - Wait on Update_Enabled (can't use imgbuf til User_Update's done)
*       - Un-VQ into ImageBuf
*       - Set the Flipper's CurFrame
*       - Set Update_Enabled
*       - Go to next frame
*       - User_Update:
*         set Palette from Flipper.CurFrame
*         copy ImageBuf to screen
*
*     This function implements a sort of cooperative multitasking.  If the
*     Drawer hits a "wait state", where it has to wait for Update_Enabled
*     to toggle, it sets a flag and returns.  This flag is checked on entry
*     to see if we need to jump to the proper execution point. This should
*     improve performance on some platforms.
*
*     This routine handles small images by UnVQ'ing into the upper-left
*     corner of ImageBuf, then copying ImageBuf onto the screen.
*
* INPUTS
*     VQA - Pointer to VQAHandle.
*
* RESULT
*     Error - 0 if successful, or VQAERR_??? error code.
*
****************************************************************************/

STATIC long DrawFrame_MCGABuf(VQAHandle *vqa)
{
	long rc;
	VQAHandleP *vqap;
	VQADrawer *drawer;
	VQAFrameNode *curframe;

	/* Dereference commonly used data members for quicker access. */
	vqap = (VQAHandleP *)vqa;
	drawer = &vqap->Drawer;

	/* Find the frame to draw */
	rc = Select_Frame(vqap);
	if (rc != VQAERR_NONE) {
		return(rc);
	}

	/* Uncompress the frame data */
	Prepare_Frame(vqap);

	/* Dereference current frame for quicker access. */
	curframe = drawer->CurFrame;

	/* Un-VQ the image */
	VQA_UnVQFrame(vqap, curframe);

	/* Tell the flipper which frame to use */
	vqap->Flipper.CurFrame = curframe;

	if ((drawer->Flags & VQADRWF_HOLD) == 0) {
		/* Move to the next frame */
		drawer->CurFrame = curframe->Next;
	}

	return(VQAERR_NONE);
}


/****************************************************************************
*
* NAME
*     PageFlip_MCGABuf - Page flip a buffered MCGA display.
*
* SYNOPSIS
*     PageFlip_MCGABuf(VQA)
*
*     void PageFlip_MCGABuf(VQAHandle *);
*
* FUNCTION
*
* INPUTS
*     VQA - Pointer to VQAHandle.
*
* RESULT
*     Error - 0 if successfull, otherwise VQAERR_???
*
****************************************************************************/

STATIC long PageFlip_MCGABuf(VQAHandle *vqa)
{
	VQAHandleP *vqap;
	VQADrawer *drawer;
	VQAFrameNode *curframe;
	VQAConfig *config;
	unsigned char *pal;
	long palsize;
	long slowpal;

	/* Derefernce commonly used data members for quicker access. */
	vqap = (VQAHandleP *)vqa;
	config = &vqap->Config;
	drawer = &vqap->Drawer;
	curframe = vqap->Flipper.CurFrame;


	if (curframe->Flags & VQAFRMF_PALETTE || drawer->Flags & VQADRWF_SETPAL) {

		/* Pre-decode 'vqa' for speed */
		pal = curframe->Palette;
		palsize = curframe->PaletteSize;
		slowpal = (config->OptionFlags & VQAOPTF_SLOWPAL) ? 1 : 0;

		/* - If this is a palette-set frame:
		 * - Wait for the vertical blank
		 * - Set the palette
		 * - Copy ImageBuf to SEENPAGE:
		 * - use blit routine if image is smaller than full-screen, since the
		 *   buffer copy assumes a full-screen image
		 */

		/* Notify the client of the palette change. */
		if (curframe->Flags & VQAFRMF_PALETTE) {


			/* Notify the client of the palette change. */
			if (config->EventHandler != NULL) {
				config->EventHandler(vqa, VQAEVENT_PALETTE, (void *)pal, (long)palsize);
			}
		} else if (drawer->Flags & VQADRWF_SETPAL) {
			drawer->Flags &= ~VQADRWF_SETPAL;


			if (config->EventHandler != NULL) {
				config->EventHandler(vqa, VQAEVENT_PALETTE, (void *)drawer->Palette_24,
						(long)drawer->CurPalSize);
			}
		}
	}

	/* Draw image to the screen. */
	//VQA_UnVQFrame(vqap, curframe); //Buffered doesn't call it here

	/* Invoke user's callback routine */
	if (config->DrawerCallback != NULL) {
		if ((config->DrawerCallback(vqa, curframe->FrameNum)) != 0) {
			return(VQAERR_EOF);
		}
	}

	return(VQAERR_NONE);
}




/****************************************************************************
*
* NAME
*     UnVQ_Nop - Do nothing UnVQ.
*
* SYNOPSIS
*     UnVQ_Nop(Codebook, Pointers, Buffer, BPR, Rows, BufWidth)
*
*     void UnVQ_Nop(unsigned char *, unsigned char *, unsigned char *,
*                   unsigned long, unsigned long, unsigned long);
* FUNCTION
*
* INPUTS
*     Codebook - Not used. (Prototype placeholder)
*     Pointers - Not used. (Prototype placeholder)
*     Buffer   - Not used. (Prototype placeholder)
*     BPR      - Not used. (Prototype placeholder)
*     Rows     - Not used. (Prototype placeholder)
*     BufWidth - Not used. (Prototype placeholder)
*
* RESULT
*     NONE
*
****************************************************************************/

STATIC void __cdecl UnVQ_Nop(unsigned char *codebook, unsigned char *pointers,
		unsigned char *buffer, unsigned long blocksperrow,
		unsigned long numrows, unsigned long bufwidth)
{
	/* Suppress compiler warnings */
	codebook = codebook;
	pointers = pointers;
	buffer = buffer;
	blocksperrow = blocksperrow;
	numrows = numrows;
	bufwidth = bufwidth;
}


/****************************************************************************
*
* NAME
*     PageFlip_Nop - Do nothing page flip.
*
* SYNOPSIS
*     PageFlip_Nop(VQA)
*
*     void PageFlip_Nop(VQAHandle *);
*
* FUNCTION
*
* INPUTS
*     VQA - Pointer to VQA handle.
*
* RESULT
*     NONE
*
****************************************************************************/

STATIC long PageFlip_Nop(VQAHandle *vqa)
{
	//shut up compiler warnings
	vqa = vqa;

	return(VQAERR_NONE);
}
