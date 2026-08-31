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

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                     $Archive:: /Commando/Library/blitblit.h                                $*
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 7/22/97 11:37a                                              $*
 *                                                                                             *
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once


/*
**	This module contains the pixel-pushing blitter objects. These objects only
**	serve one purpose. That is, to move pixels from one location to another. These
**	are prime candidates for optimization since they are called frequently and
**	loop greatly.
**
**	The large variety of blitter objects is necessary because there is a rich
**	set of pixel operations required by the game engine. Complicating this is that
**	the game engine must support both 16 bit and 8 bit pixel formats. Some of these
**	blitter objects are templates (this reduces the need for both 8 and 16 bit
**	counterparts if the algorithm is constant between pixel formats). Also note
**	that there are some assembly implementations where it seems appropriate.
**
**	If the blitter object has "Xlat" in the name, then this means that the source
**	pixel is 8 bit and the destination pixel is 16 bit (probably). This hybrid system
**	allows the game artwork to be shared between the two pixel format displays. To
**	accomplish this, a translation table is supplied to the blit operation so that
**	the 8 bit pixel can be converted into the appropriate 16 bit destination pixel.
**	If the destination surface is also 8 bit, then the translation table converts
**	the pixel to the logical palette color index appropriate for the display.
*/

#include "_alpha.h"
#include "_zbuffer.h"
#include "blitter.h"
#include "abuffer.h"
#include "dsurface.h"
#include "zbuffer.h"

#include <algorithm>
#include <cassert>
#include <cstring>


/*
**	Blits without translation and source and dest are same pixel format. Note that
**	this uses the memcpy and memmove routines. The C library has optimized these for
**	maximum performance. This includes alignment issues and performing REP MOVSD
**	instruction. This might be further optimized by using MMX instructions. However,
**	this blitter process is not often required by the game.
*/
template<class T>
class BlitPlain : public Blitter {
	public:
		virtual void BlitForward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000, int warp_offset = 0) const override
		{
			memcpy(dest, source, length*sizeof(T));
		}
		virtual void BlitBackward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000) const override
		{
			memmove(dest, source, length*sizeof(T));
		}
};


/*
**	Blits with transparency checking when and source and dest are same pixel format.
**	This process is not often used.
*/
template<class T>
class BlitTrans : public Blitter {
	public:
		virtual void BlitForward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000, int warp_offset = 0) const override
		{
			for (int index = 0; index < length; index++) {
				T color = *(T const *)source;
				source = ((T *)source) + 1;
				if (color != 0) *((T *)dest) = color;
				dest = ((T *)dest) + 1;
			}
		}

		/*
		**	The backward moving method will probably never be called in actual practice.
		**	Implement in terms of the forward copying method until the need for this
		**	version arrises.
		*/
		virtual void BlitBackward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000) const override {BlitForward(dest, source, length, z_min, z_buff, a_buff, alpha_level);}
};


/*
**	Blits when source 8 bits and dest is T. This process is typically used
**	when loading screen bitmaps or perform other non-transparent image blitting.
**	It is used fairly frequently and is a good candidate for optimization.
*/
template<class T>
class BlitPlainXlat : public Blitter {
	public:
		BlitPlainXlat(T const * translator) : TranslateTable(translator) {assert(TranslateTable != NULL);}
		virtual void BlitForward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000, int warp_offset = 0) const override
		{
			for (int index = 0; index < length; index++) {
				unsigned char color = *(unsigned char const *)source;
				source = ((unsigned char const *)source)+1;
				*((T *)dest) = TranslateTable[color];
				dest = ((T *)dest) + 1;
			}
		}

		/*
		**	The backward moving method will probably never be called in actual practice.
		**	Implement in terms of the forward copying method until the need for this
		**	version arrises.
		*/
		virtual void BlitBackward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000) const override {BlitForward(dest, source, length, z_min, z_buff, a_buff, alpha_level);}

	private:
		T const * TranslateTable;
};


/*
 * Like BlitPlainXlat (source 8 bit, dest T, no transparency), but the translate
 * index is combined with the alpha-lighting remap table sampled from the alpha
 * buffer. This applies scene lighting to every pixel drawn.
 */
template<class T>
class BlitPlainXlatAlpha : public Blitter {
	public:
		BlitPlainXlatAlpha(T const * translator, int intensity_levels) : TranslateTable(translator), AlphaLightingRemap(NULL) {assert(TranslateTable != NULL); AlphaLightingRemap = AlphaLightingRemapInit.Init(intensity_levels);}
		virtual ~BlitPlainXlatAlpha(void) override { AlphaLightingRemapInit.Deinit(AlphaLightingRemap); AlphaLightingRemap = NULL; }

		virtual void BlitForward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000, int warp_offset = 0) const override
		{
			unsigned short *ap = (unsigned short *)a_buff;
			unsigned short *alphatable = AlphaLightingRemap->Get_Table(alpha_level);

			for (int index = 0; index < length; index++) {
				unsigned char color = *(unsigned char const *)source;
				source = ((unsigned char const *)source)+1;
				*((T *)dest) = TranslateTable[color | alphatable[*ap]];
				dest = ((T *)dest) + 1;

				ap++;
				ap = Blit_Wrap_A_Buffer(ap);
			}
		}

		/*
		**	The backward moving method will probably never be called in actual practice.
		**	Implement in terms of the forward copying method until the need for this
		**	version arrises.
		*/
		virtual void BlitBackward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000) const override {BlitForward(dest, source, length, z_min, z_buff, a_buff, alpha_level);}

	private:
		T const * TranslateTable;
		AlphaLightingRemapClass *AlphaLightingRemap;
};


/*
 * Like BlitPlainXlat, but reads the depth buffer first. Each pixel is drawn only
 * if it passes the z-test (z_min nearer than the stored depth). The depth buffer
 * is not modified.
 */
template<class T>
class BlitPlainXlatZRead : public Blitter {
	public:
		BlitPlainXlatZRead(T const * translator) : TranslateTable(translator) {assert(TranslateTable != NULL);}
		virtual void BlitForward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000, int warp_offset = 0) const override
		{
			unsigned short *zb = (unsigned short *)z_buff;

			for (int i = 0; i < length; ++i) {
				unsigned char value = *(unsigned char *)source;

				if (z_min < *zb++) {
					*(T *)dest = TranslateTable[value];
				}

				source = (unsigned char *)source + 1;
				dest = (T *)dest + 1;

				zb = Blit_Wrap_Z_Buffer(zb);
			}
		}

		/*
		**	The backward moving method will probably never be called in actual practice.
		**	Implement in terms of the forward copying method until the need for this
		**	version arrises.
		*/
		virtual void BlitBackward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000) const override {BlitForward(dest, source, length, z_min, z_buff, a_buff, alpha_level);}

	private:
		T const * TranslateTable;
};


/*
 * Like BlitPlainXlatZRead, but also writes z_min back into the depth buffer for
 * each pixel that passes the z-test, so later blits are occluded by this one.
 */
template<class T>
class BlitPlainXlatZReadWrite : public Blitter {
	public:
		BlitPlainXlatZReadWrite(T const * translator) : TranslateTable(translator) {assert(TranslateTable != NULL);}
		virtual void BlitForward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000, int warp_offset = 0) const override
		{
			unsigned short *zb = (unsigned short *)z_buff;

			for (int i = 0; i < length; ++i) {
				unsigned char value = *(unsigned char *)source;

				if (z_min < *zb) {
					*(T *)dest = TranslateTable[value];
					/// The only ZReadWrite that byte-truncates the z-write; caps stored depth at 0..255.
					*zb = (unsigned char)z_min;
				}

				zb++;
				source = (unsigned char *)source + 1;
				dest = (T *)dest + 1;

				zb = Blit_Wrap_Z_Buffer(zb);
			}
		}

		/*
		**	The backward moving method will probably never be called in actual practice.
		**	Implement in terms of the forward copying method until the need for this
		**	version arrises.
		*/
		virtual void BlitBackward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000) const override {BlitForward(dest, source, length, z_min, z_buff, a_buff, alpha_level);}

	private:
		T const * TranslateTable;
};


/*
**	Blits with source 8 bit with transparency and dest is T. This process is used
**	frequently by trees and other terrain objects. It is a good candidate for
**	optimization.
*/
template<class T>
class BlitTransXlat : public Blitter {
	public:
		BlitTransXlat(T const * translator) : TranslateTable(translator) {assert(TranslateTable != NULL);}
		virtual void BlitForward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000, int warp_offset = 0) const override
		{
			for (int index = 0; index < length; index++) {
				unsigned char color = *(unsigned char const *)source;
				source = ((unsigned char const *)source)+1;
				if (color != 0) {
					*((T *)dest) = TranslateTable[color];
				}
				dest = ((T *)dest) + 1;
			}
		}

		/*
		**	The backward moving method will probably never be called in actual practice.
		**	Implement in terms of the forward copying method until the need for this
		**	version arrises.
		*/
		virtual void BlitBackward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000) const override {BlitForward(dest, source, length, z_min, z_buff, a_buff, alpha_level);}

	private:
		T const * TranslateTable;
};


/*
 * Like BlitTransXlat (source 8 bit, transparency, dest T), but the translate index
 * is combined with the alpha-lighting remap table sampled from the alpha buffer,
 * applying scene lighting to each non-transparent pixel.
 */
template<class T>
class BlitTransXlatAlpha : public Blitter {
	public:
		BlitTransXlatAlpha(T const * translator, int intensity_levels) : TranslateTable(translator), AlphaLightingRemap(NULL) {assert(TranslateTable != NULL); AlphaLightingRemap = AlphaLightingRemapInit.Init(intensity_levels);}
		virtual ~BlitTransXlatAlpha(void) override { AlphaLightingRemapInit.Deinit(AlphaLightingRemap); AlphaLightingRemap = NULL; }

		virtual void BlitForward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000, int warp_offset = 0) const override
		{
			unsigned short* ap = (unsigned short*)(a_buff);
			const unsigned short* aLUT = AlphaLightingRemap->Get_Table(alpha_level);

			for (int index = 0; index < length; index++) {
				unsigned char color = *(unsigned char const *)source;
				source = ((unsigned char const *)source)+1;
				if (color != 0) {
					*((T *)dest) = TranslateTable[color | aLUT[*ap]];
				}
				dest = ((T *)dest) + 1;

				ap++;
				ap = Blit_Wrap_A_Buffer(ap);
			}
		}

		/*
		**	The backward moving method will probably never be called in actual practice.
		**	Implement in terms of the forward copying method until the need for this
		**	version arrises.
		*/
		virtual void BlitBackward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000) const override {BlitForward(dest, source, length, z_min, z_buff, a_buff, alpha_level);}

	private:
		T const * TranslateTable;
		AlphaLightingRemapClass *AlphaLightingRemap;
};


/*
 * BlitTransXlatAlpha with a depth-buffer read. Non-transparent pixels are drawn
 * (with alpha lighting) only where they pass the z-test. The depth buffer is not
 * modified.
 */
template<class T>
class BlitTransXlatAlphaZRead : public Blitter {
	public:
		BlitTransXlatAlphaZRead(T const * translator, int intensity_levels) : TranslateTable(translator), AlphaLightingRemap(NULL) {AlphaLightingRemap = AlphaLightingRemapInit.Init(intensity_levels);}
		virtual ~BlitTransXlatAlphaZRead(void) override { AlphaLightingRemapInit.Deinit(AlphaLightingRemap); AlphaLightingRemap = NULL; }

		virtual void BlitForward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000, int warp_offset = 0) const override
		{
			unsigned short *zb = (unsigned short *)z_buff;
			unsigned short *ap = (unsigned short *)a_buff;
			const unsigned short* aLUT = AlphaLightingRemap->Get_Table(alpha_level);

			for (int index = 0; index < length; index++) {
				if (z_min < *zb++) {
					unsigned char color = *(unsigned char const *)source;
					if (color != 0) {
						*((T *)dest) = TranslateTable[color | aLUT[*ap]];
					}
				}
				source = ((unsigned char *)source) + 1;
				dest = ((T *)dest) + 1;

				zb = Blit_Wrap_Z_Buffer(zb);
				ap++;
				ap = Blit_Wrap_A_Buffer(ap);
			}
		}

		/*
		**	The backward moving method will probably never be called in actual practice.
		**	Implement in terms of the forward copying method until the need for this
		**	version arrises.
		*/
		virtual void BlitBackward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000) const override {BlitForward(dest, source, length, z_min, z_buff, a_buff, alpha_level);}

	private:
		T const * TranslateTable;
		AlphaLightingRemapClass *AlphaLightingRemap;
};


/*
 * BlitTransXlatAlpha with a depth-buffer read and write. Non-transparent pixels
 * that pass the z-test are drawn (with alpha lighting) and z_min is written back
 * into the depth buffer.
 */
template<class T>
class BlitTransXlatAlphaZReadWrite : public Blitter {
	public:
		BlitTransXlatAlphaZReadWrite(T const * translator, int intensity_levels) : TranslateTable(translator), AlphaLightingRemap(NULL) {AlphaLightingRemap = AlphaLightingRemapInit.Init(intensity_levels);}
		virtual ~BlitTransXlatAlphaZReadWrite(void) override { AlphaLightingRemapInit.Deinit(AlphaLightingRemap); AlphaLightingRemap = NULL; }

		virtual void BlitForward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000, int warp_offset = 0) const override
		{
			unsigned short *zb = (unsigned short *)z_buff;
			unsigned short *ap = (unsigned short *)a_buff;
			const unsigned short* aLUT = AlphaLightingRemap->Get_Table(alpha_level);

			for (int index = 0; index < length; index++) {
				if (z_min < *zb) {
					unsigned char color = *(unsigned char const *)source;
					if (color != 0) {
						*((T *)dest) = TranslateTable[color | aLUT[*ap]];
						*zb = z_min;
					}
				}
				source = ((unsigned char *)source) + 1;
				dest = ((T *)dest) + 1;

				zb++;
				zb = Blit_Wrap_Z_Buffer(zb);
				ap++;
				ap = Blit_Wrap_A_Buffer(ap);
			}
		}

		/*
		**	The backward moving method will probably never be called in actual practice.
		**	Implement in terms of the forward copying method until the need for this
		**	version arrises.
		*/
		virtual void BlitBackward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000) const override {BlitForward(dest, source, length, z_min, z_buff, a_buff, alpha_level);}

	private:
		T const * TranslateTable;
		AlphaLightingRemapClass *AlphaLightingRemap;
};


/*
 * Does not write color. For each non-transparent source pixel it writes an
 * accumulated alpha value (z_min + alpha_level * source, clamped to 255) into the
 * alpha buffer. Used to build up the alpha/lighting buffer, scaling each
 * contribution by the source pixel value.
 */
template<class T>
class BlitTransXlatMultWriteAlpha : public Blitter {
	public:
		BlitTransXlatMultWriteAlpha(void) {}
		virtual void BlitForward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000, int warp_offset = 0) const override
		{
			unsigned short *ap = (unsigned short *)a_buff;

			for (int i = 0; i < length; ++i) {
				unsigned char value = *(unsigned char *)source;
				source = (unsigned char *)source + 1;

				if (value != 0) {
					*(unsigned short *)ap = std::min(z_min + alpha_level * value, 255);
				}

				ap = ap + 1;

				ap = Blit_Wrap_A_Buffer(ap);
			}
		}

		/*
		**	The backward moving method will probably never be called in actual practice.
		**	Implement in terms of the forward copying method until the need for this
		**	version arrises.
		*/
		virtual void BlitBackward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000) const override {BlitForward(dest, source, length, z_min, z_buff, a_buff, alpha_level);}
};


/*
 * Does not write color. For each non-transparent source pixel it writes an alpha
 * value (z_min + source, clamped to 255) into the alpha buffer. Like
 * BlitTransXlatMultWriteAlpha but without the alpha_level multiply.
 */
template<class T>
class BlitTransXlatWriteAlpha : public Blitter {
	public:
		BlitTransXlatWriteAlpha(void) {}
		virtual void BlitForward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000, int warp_offset = 0) const override
		{
			unsigned short *ap = (unsigned short *)a_buff;

			for (int i = 0; i < length; ++i) {
				unsigned char value = *(unsigned char *)source;
				source = (unsigned char *)source + 1;

				if (value != 0) {
					*(unsigned short *)ap = std::min(z_min + value, 255);
				}

				ap = ap + 1;

				ap = Blit_Wrap_A_Buffer(ap);
			}
		}

		/*
		**	The backward moving method will probably never be called in actual practice.
		**	Implement in terms of the forward copying method until the need for this
		**	version arrises.
		*/
		virtual void BlitBackward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000) const override {BlitForward(dest, source, length, z_min, z_buff, a_buff, alpha_level);}
};


/*
 * BlitTransXlat with a depth-buffer read. Non-transparent pixels are translated
 * and drawn only where they pass the z-test. The depth buffer is not modified.
 */
template<class T>
class BlitTransXlatZRead : public Blitter {
	public:
		BlitTransXlatZRead(T const * translator) : TranslateTable(translator) {assert(TranslateTable != NULL);}
		virtual void BlitForward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000, int warp_offset = 0) const override
		{
			unsigned short *zb = (unsigned short *)z_buff;

			for (int i = 0; i < length; ++i) {

				if (z_min < *zb++) {
					unsigned char value = *(unsigned char *)source;
					if (value != 0) {
						*(T *)dest = TranslateTable[(unsigned char)value];
					}
				}
				source = ((unsigned char *)source) + 1;
				dest = ((T *)dest) + 1;

				zb = Blit_Wrap_Z_Buffer(zb);
			}
		}

		/*
		**	The backward moving method will probably never be called in actual practice.
		**	Implement in terms of the forward copying method until the need for this
		**	version arrises.
		*/
		virtual void BlitBackward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000) const override {BlitForward(dest, source, length, z_min, z_buff, a_buff, alpha_level);}

	private:
		T const * TranslateTable;
};


/*
 * BlitTransXlat with a depth-buffer read and write. Non-transparent pixels that
 * pass the z-test are drawn and z_min is written back into the depth buffer.
 */
template<class T>
class BlitTransXlatZReadWrite : public Blitter {
	public:
		BlitTransXlatZReadWrite(T const * translator) : TranslateTable(translator) {assert(TranslateTable != NULL);}
		virtual void BlitForward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000, int warp_offset = 0) const override
		{
			unsigned short *zb = (unsigned short *)z_buff;

			for (int i = 0; i < length; ++i) {

				if (z_min < *zb) {
					unsigned char value = *(unsigned char *)source;
					if (value != 0) {
						*(T *)dest = TranslateTable[(unsigned char)value];
						*zb = z_min;
					}
				}
				zb++;
				source = ((unsigned char *)source) + 1;
				dest = ((T *)dest) + 1;

				zb = Blit_Wrap_Z_Buffer(zb);
			}
		}

		/*
		**	The backward moving method will probably never be called in actual practice.
		**	Implement in terms of the forward copying method until the need for this
		**	version arrises.
		*/
		virtual void BlitBackward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000) const override {BlitForward(dest, source, length, z_min, z_buff, a_buff, alpha_level);}

	private:
		T const * TranslateTable;
};


/*
**	Blits with source 8 bit, transparency check, then translate to pixel format T. This
**	is occasionally used to render special remapping effects. Since the remap table is
**	not doubly indirected, it is fixed to only using the remap table specified in the
**	constructor. As such, it has limited value.
*/
template<class T>
class BlitTransRemapXlat : public Blitter {
	public:
		BlitTransRemapXlat(unsigned char const * remapper, T const * translator) : RemapTable(remapper), TranslateTable(translator) {assert(RemapTable != NULL);assert(TranslateTable != NULL);}
		virtual void BlitForward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000, int warp_offset = 0) const override
		{
			for (int index = 0; index < length; index++) {
				unsigned char color = *(unsigned char const *)source;
				source = ((unsigned char const *)source)+1;
				if (color != 0) {
					*((T *)dest) = TranslateTable[RemapTable[color]];
				}
				dest = ((T *)dest) + 1;
			}
		}

		/*
		**	The backward moving method will probably never be called in actual practice.
		**	Implement in terms of the forward copying method until the need for this
		**	version arrises.
		*/
		virtual void BlitBackward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000) const override {BlitForward(dest, source, length, z_min, z_buff, a_buff, alpha_level);}

	private:
		unsigned char const * RemapTable;
		T const * TranslateTable;
};


/*
**	Blits with source 8 bit with transparency then remap and dest is T. This is probably
**	the most used blitter process. Units, infantry, buildings, and aircraft use this for
**	their normal drawing needs. If any blitter process is to be optimized, this would be
**	the one. Take note that the remapper table is doubly indirected. This allows a single
**	blitter object to dynamically use alternate remap tables.
*/
template<class T>
class BlitTransZRemapXlat : public Blitter {
	public:
		BlitTransZRemapXlat(unsigned char const * const * remapper, T const * translator) : RemapTable(remapper), TranslateTable(translator) {assert(RemapTable != NULL);assert(TranslateTable != NULL);}
		virtual void BlitForward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000, int warp_offset = 0) const override
		{
			unsigned char const * rtable = *RemapTable;
			for (int index = 0; index < length; index++) {
				unsigned char color = *(unsigned char const *)source;
				source = ((unsigned char const *)source)+1;
				if (color != 0) {
					*((T *)dest) = TranslateTable[rtable[color]];
				}
				dest = ((T *)dest) + 1;
			}
		}

		/*
		**	The backward moving method will probably never be called in actual practice.
		**	Implement in terms of the forward copying method until the need for this
		**	version arrises.
		*/
		virtual void BlitBackward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000) const override {BlitForward(dest, source, length, z_min, z_buff, a_buff, alpha_level);}

	private:
		unsigned char const * const * RemapTable;
		T const * TranslateTable;
};


/*
 * BlitTransZRemapXlat (remap then translate, dynamic doubly-indirected remap
 * table) with the alpha-lighting remap table combined into the translate index,
 * applying scene lighting to each non-transparent remapped pixel.
 */
template<class T>
class BlitTransZRemapXlatAlpha : public Blitter {
	public:
		BlitTransZRemapXlatAlpha(unsigned char const * const * remapper, T const * translator, int intensity_levels) : RemapTable(remapper), TranslateTable(translator), AlphaLightingRemap(NULL) {assert(RemapTable != NULL);assert(TranslateTable != NULL); AlphaLightingRemap = AlphaLightingRemapInit.Init(intensity_levels);}
		virtual ~BlitTransZRemapXlatAlpha(void) override { AlphaLightingRemapInit.Deinit(AlphaLightingRemap); AlphaLightingRemap = NULL; }

		virtual void BlitForward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000, int warp_offset = 0) const override
		{
			unsigned char const * rtable = *RemapTable;
			unsigned short* ap = (unsigned short*)(a_buff);
			const unsigned short* aLUT = AlphaLightingRemap->Get_Table(alpha_level);

			for (int index = 0; index < length; index++) {
				unsigned char color = *(unsigned char const *)source;
				source = ((unsigned char const *)source)+1;
				if (color != 0) {
					*((T *)dest) = TranslateTable[rtable[color] | aLUT[*ap]];
				}
				dest = ((T *)dest) + 1;

				ap++;
				ap = Blit_Wrap_A_Buffer(ap);
			}
		}

		/*
		**	The backward moving method will probably never be called in actual practice.
		**	Implement in terms of the forward copying method until the need for this
		**	version arrises.
		*/
		virtual void BlitBackward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000) const override {BlitForward(dest, source, length, z_min, z_buff, a_buff, alpha_level);}

	private:
		unsigned char const * const * RemapTable;
		T const * TranslateTable;
		AlphaLightingRemapClass *AlphaLightingRemap;
};


/*
 * BlitTransZRemapXlatAlpha with a depth-buffer read. Non-transparent pixels are
 * drawn (remapped, translated, alpha-lit) only where they pass the z-test. The
 * depth buffer is not modified.
 */
template<class T>
class BlitTransZRemapXlatAlphaZRead : public Blitter {
	public:
		BlitTransZRemapXlatAlphaZRead(unsigned char const * const * remapper, T const * translator, int intensity_levels) : RemapTable(remapper), TranslateTable(translator), AlphaLightingRemap(NULL) {assert(RemapTable != NULL);assert(TranslateTable != NULL); AlphaLightingRemap = AlphaLightingRemapInit.Init(intensity_levels);}
		virtual ~BlitTransZRemapXlatAlphaZRead(void) override { AlphaLightingRemapInit.Deinit(AlphaLightingRemap); AlphaLightingRemap = NULL; }

		virtual void BlitForward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000, int warp_offset = 0) const override
		{
			unsigned short *zb = (unsigned short *)z_buff;
			unsigned short *ap = (unsigned short *)a_buff;
			unsigned char const * rtable = *RemapTable;
			const unsigned short* aLUT = AlphaLightingRemap->Get_Table(alpha_level);

			for (int index = 0; index < length; index++) {
				if (z_min < *zb++) {
					unsigned char color = *(unsigned char const *)source;
					if (color != 0) {
						*((T *)dest) = TranslateTable[rtable[color] | aLUT[*ap]];
					}
				}
				source = ((unsigned char *)source) + 1;
				dest = ((T *)dest) + 1;

				zb = Blit_Wrap_Z_Buffer(zb);
				ap++;
				ap = Blit_Wrap_A_Buffer(ap);
			}
		}

		/*
		**	The backward moving method will probably never be called in actual practice.
		**	Implement in terms of the forward copying method until the need for this
		**	version arrises.
		*/
		virtual void BlitBackward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000) const override {BlitForward(dest, source, length, z_min, z_buff, a_buff, alpha_level);}

	private:
		unsigned char const * const * RemapTable;
		T const * TranslateTable;
		AlphaLightingRemapClass *AlphaLightingRemap;
};


/*
 * BlitTransZRemapXlatAlpha with a depth-buffer read and write. Non-transparent
 * pixels that pass the z-test are drawn and z_min is written back into the depth
 * buffer.
 */
template<class T>
class BlitTransZRemapXlatAlphaZReadWrite : public Blitter {
	public:
		BlitTransZRemapXlatAlphaZReadWrite(unsigned char const * const * remapper, T const * translator, int intensity_levels) : RemapTable(remapper), AlphaLightingRemap(NULL), TranslateTable(translator) {assert(RemapTable != NULL);assert(TranslateTable != NULL); AlphaLightingRemap = AlphaLightingRemapInit.Init(intensity_levels);}
		virtual ~BlitTransZRemapXlatAlphaZReadWrite(void) override { AlphaLightingRemapInit.Deinit(AlphaLightingRemap); AlphaLightingRemap = NULL; }

		virtual void BlitForward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000, int warp_offset = 0) const override
		{
			unsigned short *zb = (unsigned short *)z_buff;
			unsigned short *ap = (unsigned short *)a_buff;
			const unsigned short* aLUT = AlphaLightingRemap->Get_Table(alpha_level);
			unsigned char const * rtable = *RemapTable;

			for (int index = 0; index < length; index++) {
				if (z_min < *zb) {
					unsigned char color = *(unsigned char const *)source;
					if (color != 0) {
						*((T *)dest) = TranslateTable[rtable[color] | aLUT[*ap]];
						*zb = z_min;
					}
				}
				source = ((unsigned char *)source) + 1;
				dest = ((T *)dest) + 1;

				zb++;
				/*
				 * `ap` is never advanced, so every pixel samples aLUT[*ap] from a_buff[0].
				 */
				zb = Blit_Wrap_Z_Buffer(zb);
			}
		}

		/*
		**	The backward moving method will probably never be called in actual practice.
		**	Implement in terms of the forward copying method until the need for this
		**	version arrises.
		*/
		virtual void BlitBackward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000) const override {BlitForward(dest, source, length, z_min, z_buff, a_buff, alpha_level);}

	private:
		unsigned char const * const * RemapTable;
		T const * TranslateTable;
		AlphaLightingRemapClass *AlphaLightingRemap;
};


/*
 * BlitTransZRemapXlat with a depth-buffer read. Non-transparent pixels are
 * remapped, translated and drawn only where they pass the z-test. The depth
 * buffer is not modified.
 */
template<class T>
class BlitTransZRemapXlatZRead : public Blitter {
	public:
		BlitTransZRemapXlatZRead(unsigned char const * const * remapper, T const * translator) : RemapTable(remapper), TranslateTable(translator) {assert(RemapTable != NULL);assert(TranslateTable != NULL);}
		virtual void BlitForward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000, int warp_offset = 0) const override
		{
			unsigned short *zb = (unsigned short *)z_buff;
			unsigned char const * rtable = *RemapTable;

			for (int i = 0; i < length; ++i) {

				if (z_min < *zb++) {
					unsigned char value = *(unsigned char *)source;
					if (value != 0) {
						*(T *)dest = TranslateTable[rtable[(unsigned char)value]];
					}
				}
				source = ((unsigned char *)source) + 1;
				dest = ((T *)dest) + 1;

				zb = Blit_Wrap_Z_Buffer(zb);
			}
		}

		/*
		**	The backward moving method will probably never be called in actual practice.
		**	Implement in terms of the forward copying method until the need for this
		**	version arrises.
		*/
		virtual void BlitBackward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000) const override {BlitForward(dest, source, length, z_min, z_buff, a_buff, alpha_level);}

	private:
		unsigned char const * const * RemapTable;
		T const * TranslateTable;
};


/*
 * BlitTransZRemapXlat with a depth-buffer read and write. Non-transparent pixels
 * that pass the z-test are drawn and z_min is written back into the depth buffer.
 */
template<class T>
class BlitTransZRemapXlatZReadWrite : public Blitter {
	public:
		BlitTransZRemapXlatZReadWrite(unsigned char const * const * remapper, T const * translator) : RemapTable(remapper), TranslateTable(translator) {assert(RemapTable != NULL);assert(TranslateTable != NULL);}
		virtual void BlitForward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000, int warp_offset = 0) const override
		{
			unsigned short *zb = (unsigned short *)z_buff;
			unsigned char const * rtable = *RemapTable;

			for (int i = 0; i < length; ++i) {

				if (z_min < *zb) {
					unsigned char value = *(unsigned char *)source;
					if (value != 0) {
						*(T *)dest = TranslateTable[rtable[(unsigned char)value]];
						*zb = z_min;
					}
				}
				zb++;
				source = ((unsigned char *)source) + 1;
				dest = ((T *)dest) + 1;

				zb = Blit_Wrap_Z_Buffer(zb);
			}
		}

		/*
		**	The backward moving method will probably never be called in actual practice.
		**	Implement in terms of the forward copying method until the need for this
		**	version arrises.
		*/
		virtual void BlitBackward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000) const override {BlitForward(dest, source, length, z_min, z_buff, a_buff, alpha_level);}

	private:
		unsigned char const * const * RemapTable;
		T const * TranslateTable;
};


/*
**	Algorithmic darkening of hicolor pixels controlled by the source pixels. The source
**	pixels are examined only to determine if the destination pixel should be darkened.
**	If the source pixel is transparent, then the dest pixel is skipped. The darkening
**	algorithm works only for hicolor pixels.
*/
template<class T>
class BlitTransDarken : public Blitter {
	public:
		BlitTransDarken(T mask) : Mask(mask) {}
		virtual void BlitForward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000, int warp_offset = 0) const override
		{
			for (int index = 0; index < length; index++) {
				unsigned char color = *(unsigned char const *)source;
				source = ((unsigned char const *)source)+1;
				if (color != 0) {
					*((T *)dest) = (T)((((*(T *)dest) >> 1) & Mask));
				}
				dest = ((T *)dest) + 1;
			}
		}

		/*
		**	The backward moving method will probably never be called in actual practice.
		**	Implement in terms of the forward copying method until the need for this
		**	version arrises.
		*/
		virtual void BlitBackward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000) const override {BlitForward(dest, source, length, z_min, z_buff, a_buff, alpha_level);}

	private:
		T Mask;
};


/*
 * BlitTransDarken with a depth-buffer read. A destination pixel is darkened
 * (controlled by non-transparent source pixels) only where it passes the z-test.
 * The depth buffer is not modified.
 */
template<class T>
class BlitTransDarkenZRead : public Blitter {
	public:
		BlitTransDarkenZRead(T mask) : Mask(mask) {}
		virtual void BlitForward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000, int warp_offset = 0) const override
		{
			unsigned short *zb = (unsigned short *)z_buff;

			for (int i = 0; i < length; ++i) {

				if (z_min < *zb++) {
					unsigned char value = *(unsigned char *)source;
					if (value != 0) {
						*((T *)dest) = (T)((((*(T *)dest) >> 1) & Mask));
					}
				}
				source = ((unsigned char *)source) + 1;
				dest = ((T *)dest) + 1;

				zb = Blit_Wrap_Z_Buffer(zb);
			}
		}

		/*
		**	The backward moving method will probably never be called in actual practice.
		**	Implement in terms of the forward copying method until the need for this
		**	version arrises.
		*/
		virtual void BlitBackward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000) const override {BlitForward(dest, source, length, z_min, z_buff, a_buff, alpha_level);}

	private:
		T Mask;
};


/*
 * BlitTransDarken with a depth-buffer read and write. Destination pixels that
 * pass the z-test are darkened and z_min is written back into the depth buffer.
 */
template<class T>
class BlitTransDarkenZReadWrite : public Blitter {
	public:
		BlitTransDarkenZReadWrite(T mask) : Mask(mask) {}
		virtual void BlitForward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000, int warp_offset = 0) const override
		{
			unsigned short *zb = (unsigned short *)z_buff;

			for (int i = 0; i < length; ++i) {

				if (z_min < *zb) {
					unsigned char value = *(unsigned char *)source;
					if (value != 0) {
						*((T *)dest) = (T)((((*(T *)dest) >> 1) & Mask));
						*zb = z_min;
					}
				}
				zb++;
				source = ((unsigned char *)source) + 1;
				dest = ((T *)dest) + 1;

				zb = Blit_Wrap_Z_Buffer(zb);
			}
		}

		/*
		**	The backward moving method will probably never be called in actual practice.
		**	Implement in terms of the forward copying method until the need for this
		**	version arrises.
		*/
		virtual void BlitBackward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000) const override {BlitForward(dest, source, length, z_min, z_buff, a_buff, alpha_level);}

	private:
		T Mask;
};


/*
**	This will remap the destination pixels but under the control of the source pixels.
**	Where the source pixel is not transparent, the dest pixel is remapped. This algorithm
**	really only applies to lowcolor display.
*/
template<class T>
class BlitTransRemapDest : public Blitter {
	public:
		BlitTransRemapDest(T const * remap) : RemapTable(remap) {}
		virtual void BlitForward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000, int warp_offset = 0) const override
		{
			for (int index = 0; index < length; index++) {
				unsigned char color = *(unsigned char const *)source;
				source = ((unsigned char const *)source)+1;
				if (color != 0) {
					*((T *)dest) = RemapTable[*((T *)dest)];
				}
				dest = ((T *)dest) + 1;
			}
		}

		/*
		**	The backward moving method will probably never be called in actual practice.
		**	Implement in terms of the forward copying method until the need for this
		**	version arrises.
		*/
		virtual void BlitBackward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000) const override {BlitForward(dest, source, length, z_min, z_buff, a_buff, alpha_level);}

	private:
		T const * RemapTable;
};


/*
**	This is similar to BlitTransDarken but instead of examining the source to determine what
**	pixels should be darkened, every destination pixel is darkened. This means that the source
**	pointer is unused.
*/
template<class T>
class BlitDarken : public Blitter {
	public:
		BlitDarken(T mask) : Mask(mask) {}
		virtual void BlitForward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000, int warp_offset = 0) const override
		{
			for (int index = 0; index < length; index++) {
				*((T *)dest) = (T)(((*(T *)dest) >> 1) & Mask);
				dest = ((T *)dest) + 1;
			}
		}

		/*
		**	The backward moving method will probably never be called in actual practice.
		**	Implement in terms of the forward copying method until the need for this
		**	version arrises.
		*/
		virtual void BlitBackward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000) const override {BlitForward(dest, source, length, z_min, z_buff, a_buff, alpha_level);}

	private:
		T Mask;
};


/*
**	This blitter performs 50% translucency as it draws. It is commonly used for animation
**	effects and other stealth like images. It only works with hicolor pixels but is a good
**	candidate for optimization.
*/
template<class T>
class BlitTransLucent50 : public Blitter {
	public:
		BlitTransLucent50(T const * translator, T mask) : TranslateTable(translator), Mask(mask) {assert(TranslateTable != NULL);}
		virtual void BlitForward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000, int warp_offset = 0) const override
		{
			for (int index = 0; index < length; index++) {
				unsigned char color = *(unsigned char const *)source;
				source = ((unsigned char *)source) + 1;
				if (color != 0) {
					*((T *)dest) = (T)((((*(T *)dest) >> 1) & Mask) + ((TranslateTable[color] >> 1) & Mask));
				}
				dest = ((T *)dest) + 1;
			}
		}

		/*
		**	The backward moving method will probably never be called in actual practice.
		**	Implement in terms of the forward copying method until the need for this
		**	version arrises.
		*/
		virtual void BlitBackward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000) const override {BlitForward(dest, source, length, z_min, z_buff, a_buff, alpha_level);}

	private:
		T const * TranslateTable;
		T Mask;
};


/*
 * BlitTransLucent50 (50% translucency) with the alpha-lighting remap table
 * combined into the translate index, applying scene lighting to each blended pixel.
 */
template<class T>
class BlitTransLucent50Alpha : public Blitter {
	public:
		BlitTransLucent50Alpha(T const * translator, int intensity_levels, T mask) : TranslateTable(translator), AlphaLightingRemap(NULL), Mask(mask) {AlphaLightingRemap = AlphaLightingRemapInit.Init(intensity_levels);}
		virtual ~BlitTransLucent50Alpha(void) override { AlphaLightingRemapInit.Deinit(AlphaLightingRemap); AlphaLightingRemap = NULL; }

		virtual void BlitForward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000, int warp_offset = 0) const override
		{
			unsigned short *ap = (unsigned short *)a_buff;
			const unsigned short* aLUT = AlphaLightingRemap->Get_Table(alpha_level);

			for (int index = 0; index < length; index++) {
				unsigned char color = *(unsigned char const *)source;
				source = ((unsigned char *)source) + 1;
				if (color != 0) {
					*((T *)dest) = (T)((((*(T *)dest) >> 1) & Mask) + ((TranslateTable[color | aLUT[*ap]] >> 1) & Mask));
				}
				dest = ((T *)dest) + 1;

				ap++;
				ap = Blit_Wrap_A_Buffer(ap);
			}
		}

		/*
		**	The backward moving method will probably never be called in actual practice.
		**	Implement in terms of the forward copying method until the need for this
		**	version arrises.
		*/
		virtual void BlitBackward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000) const override {BlitForward(dest, source, length, z_min, z_buff, a_buff, alpha_level);}

	private:
		T const * TranslateTable;
		AlphaLightingRemapClass *AlphaLightingRemap;
		T Mask;
};


/*
 * BlitTransLucent50Alpha with a depth-buffer read. Pixels are 50% blended (with
 * alpha lighting) only where they pass the z-test. The depth buffer is not modified.
 */
template<class T>
class BlitTransLucent50AlphaZRead : public Blitter {
	public:
		BlitTransLucent50AlphaZRead(T const * translator, int intensity_levels, T mask) : TranslateTable(translator), AlphaLightingRemap(NULL), Mask(mask) {AlphaLightingRemap = AlphaLightingRemapInit.Init(intensity_levels);}
		virtual ~BlitTransLucent50AlphaZRead(void) override { AlphaLightingRemapInit.Deinit(AlphaLightingRemap); AlphaLightingRemap = NULL; }

		virtual void BlitForward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000, int warp_offset = 0) const override
		{
			unsigned short *zb = (unsigned short *)z_buff;
			unsigned short *ap = (unsigned short *)a_buff;
			const unsigned short* aLUT = AlphaLightingRemap->Get_Table(alpha_level);

			for (int index = 0; index < length; index++) {
				if (z_min < *zb++) {
					unsigned char color = *(unsigned char const *)source;
					if (color != 0) {
						T qsource = (T)(((TranslateTable[color | aLUT[*ap]] >> 1) & Mask));
						T qdest = (T)((((*(T *)dest) >> 1) & Mask));
						*((T *)dest) = (T)(qdest + qsource);
					}
				}
				source = ((unsigned char *)source) + 1;
				dest = ((T *)dest) + 1;

				zb = Blit_Wrap_Z_Buffer(zb);
				ap++;
				ap = Blit_Wrap_A_Buffer(ap);
			}
		}

		/*
		**	The backward moving method will probably never be called in actual practice.
		**	Implement in terms of the forward copying method until the need for this
		**	version arrises.
		*/
		virtual void BlitBackward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000) const override {BlitForward(dest, source, length, z_min, z_buff, a_buff, alpha_level);}

	private:
		T const * TranslateTable;
		AlphaLightingRemapClass *AlphaLightingRemap;
		T Mask;
};


/*
 * BlitTransLucent50AlphaZRead, but the destination pixel is sampled through
 * warp_offset before blending, producing a distortion / heat-haze effect. The
 * depth buffer is not modified.
 */
template<class T>
class BlitTransLucent50AlphaZReadWarp : public Blitter {
	public:
		BlitTransLucent50AlphaZReadWarp(T const * translator, int intensity_levels, T mask) : TranslateTable(translator), AlphaLightingRemap(NULL), Mask(mask) {AlphaLightingRemap = AlphaLightingRemapInit.Init(intensity_levels);}
		virtual ~BlitTransLucent50AlphaZReadWarp(void) override { AlphaLightingRemapInit.Deinit(AlphaLightingRemap); AlphaLightingRemap = NULL; }

		virtual void BlitForward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000, int warp_offset = 0) const override
		{
			unsigned short *zb = (unsigned short *)z_buff;
			unsigned short *ap = (unsigned short *)a_buff;
			const unsigned short* aLUT = AlphaLightingRemap->Get_Table(alpha_level);

			for (int index = 0; index < length; index++) {
				if (z_min < *zb++) {
					unsigned char color = *(unsigned char const *)source;
					if (color != 0) {
						*((T *)dest) = (T)((((((T *)dest)[warp_offset]) >> 1) & Mask) + ((TranslateTable[color | aLUT[*ap]] >> 1) & Mask));
					}
				}
				source = ((unsigned char *)source) + 1;
				dest = ((T *)dest) + 1;

				zb = Blit_Wrap_Z_Buffer(zb);
				ap++;
				ap = Blit_Wrap_A_Buffer(ap);
			}
		}

		/*
		**	The backward moving method will probably never be called in actual practice.
		**	Implement in terms of the forward copying method until the need for this
		**	version arrises.
		*/
		virtual void BlitBackward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000) const override {BlitForward(dest, source, length, z_min, z_buff, a_buff, alpha_level);}

	private:
		T const * TranslateTable;
		AlphaLightingRemapClass *AlphaLightingRemap;
		T Mask;
};


/*
 * BlitTransLucent50Alpha with a depth-buffer read and write. Pixels that pass the
 * z-test are 50% blended (with alpha lighting) and z_min is written back into the
 * depth buffer.
 */
template<class T>
class BlitTransLucent50AlphaZReadWrite : public Blitter {
	public:
		BlitTransLucent50AlphaZReadWrite(T const * translator, int intensity_levels, T mask) : TranslateTable(translator), AlphaLightingRemap(NULL), Mask(mask) {AlphaLightingRemap = AlphaLightingRemapInit.Init(intensity_levels);}
		virtual ~BlitTransLucent50AlphaZReadWrite(void) override { AlphaLightingRemapInit.Deinit(AlphaLightingRemap); AlphaLightingRemap = NULL; }

		virtual void BlitForward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000, int warp_offset = 0) const override
		{
			unsigned short *zb = (unsigned short *)z_buff;
			unsigned short *ap = (unsigned short *)a_buff;
			const unsigned short* aLUT = AlphaLightingRemap->Get_Table(alpha_level);

			for (int index = 0; index < length; index++) {
				if (z_min < *zb) {
					unsigned char color = *(unsigned char const *)source;
					if (color != 0) {
						*((T *)dest) = (T)((((*(T *)dest) >> 1) & Mask) + ((TranslateTable[color | aLUT[*ap]] >> 1) & Mask));
						*zb = z_min;
					}
				}
				source = ((unsigned char *)source) + 1;
				dest = ((T *)dest) + 1;

				zb++;
				zb = Blit_Wrap_Z_Buffer(zb);
				ap++;
				ap = Blit_Wrap_A_Buffer(ap);
			}
		}

		/*
		**	The backward moving method will probably never be called in actual practice.
		**	Implement in terms of the forward copying method until the need for this
		**	version arrises.
		*/
		virtual void BlitBackward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000) const override {BlitForward(dest, source, length, z_min, z_buff, a_buff, alpha_level);}

	private:
		T const * TranslateTable;
		AlphaLightingRemapClass *AlphaLightingRemap;
		T Mask;
};


/*
 * BlitTransLucent50 with a depth-buffer read. Pixels are 50% blended only where
 * they pass the z-test. The depth buffer is not modified.
 */
template<class T>
class BlitTransLucent50ZRead : public Blitter {
	public:
		BlitTransLucent50ZRead(T const * translator, T mask) : TranslateTable(translator), Mask(mask) {assert(TranslateTable != NULL);}
		virtual void BlitForward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000, int warp_offset = 0) const override
		{
			unsigned short *zb = (unsigned short *)z_buff;
			for (int index = 0; index < length; index++) {
				if (z_min < *zb++) {
					unsigned char color = *(unsigned char const *)source;
					if (color != 0) {
						*((T *)dest) = (T)((((*(T *)dest) >> 1) & Mask) + ((TranslateTable[color] >> 1) & Mask));
					}
				}
				source = ((unsigned char *)source) + 1;
				dest = ((T *)dest) + 1;

				zb = Blit_Wrap_Z_Buffer(zb);
			}
		}

		/*
		**	The backward moving method will probably never be called in actual practice.
		**	Implement in terms of the forward copying method until the need for this
		**	version arrises.
		*/
		virtual void BlitBackward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000) const override {BlitForward(dest, source, length, z_min, z_buff, a_buff, alpha_level);}

	private:
		T const * TranslateTable;
		T Mask;
};


/*
 * BlitTransLucent50ZRead, but the destination pixel is sampled through warp_offset
 * before blending, producing a distortion / heat-haze effect.
 */
template<class T>
class BlitTransLucent50ZReadWarp : public Blitter {
	public:
		BlitTransLucent50ZReadWarp(T const * translator, T mask) : TranslateTable(translator), Mask(mask) {assert(TranslateTable != NULL);}
		virtual void BlitForward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000, int warp_offset = 0) const override
		{
			unsigned short *zb = (unsigned short *)z_buff;
			for (int index = 0; index < length; index++) {
				if (z_min < *zb++) {
					unsigned char color = *(unsigned char const *)source;
					if (color != 0) {
						*((T *)dest) = (T)((((((T *)dest)[warp_offset]) >> 1) & Mask) + ((TranslateTable[color] >> 1) & Mask));
					}
				}
				source = ((unsigned char *)source) + 1;
				dest = ((T *)dest) + 1;

				zb = Blit_Wrap_Z_Buffer(zb);
			}
		}

		/*
		**	The backward moving method will probably never be called in actual practice.
		**	Implement in terms of the forward copying method until the need for this
		**	version arrises.
		*/
		virtual void BlitBackward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000) const override {BlitForward(dest, source, length, z_min, z_buff, a_buff, alpha_level);}

	private:
		T const * TranslateTable;
		T Mask;
};


/*
 * BlitTransLucent50 with a depth-buffer read and write. Pixels that pass the
 * z-test are 50% blended and z_min is written back into the depth buffer.
 */
template<class T>
class BlitTransLucent50ZReadWrite : public Blitter {
	public:
		BlitTransLucent50ZReadWrite(T const * translator, T mask) : TranslateTable(translator), Mask(mask) {assert(TranslateTable != NULL);}
		virtual void BlitForward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000, int warp_offset = 0) const override
		{
			unsigned short *zb = (unsigned short *)z_buff;
			for (int index = 0; index < length; index++) {
				if (z_min < *zb) {
					unsigned char color = *(unsigned char const *)source;
					if (color != 0) {
						*((T *)dest) = (T)((((*(T *)dest) >> 1) & Mask) + ((TranslateTable[color] >> 1) & Mask));
						*zb = z_min;
					}
				}
				zb++;
				source = ((unsigned char *)source) + 1;
				dest = ((T *)dest) + 1;

				zb = Blit_Wrap_Z_Buffer(zb);
			}
		}

		/*
		**	The backward moving method will probably never be called in actual practice.
		**	Implement in terms of the forward copying method until the need for this
		**	version arrises.
		*/
		virtual void BlitBackward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000) const override {BlitForward(dest, source, length, z_min, z_buff, a_buff, alpha_level);}

	private:
		T const * TranslateTable;
		T Mask;
};


/*
 * 50% translucency gated by the alpha buffer. A non-transparent source pixel is
 * blended only where the alpha-buffer value is nonzero.
 */
template<class T>
class BlitTranslucent50NonzeroAlpha : public Blitter {
	public:
		BlitTranslucent50NonzeroAlpha(T const * translator, T mask) : TranslateTable(translator), Mask(mask) {assert(TranslateTable != NULL);}
		virtual void BlitForward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000, int warp_offset = 0) const override
		{
			unsigned short *ap = (unsigned short *)a_buff;

			for (int i = 0; i < length; ++i) {
				unsigned char value = *(unsigned char *)source;
				source = (unsigned char *)source + 1;

				if (value != 0 && *ap != 0) {
					*(T *)dest = ((*(T *)dest / 2) & Mask) + ((TranslateTable[value] / 2) & Mask);
				}

				ap = ap + 1;
				dest = (T *)dest + 1;

				ap = Blit_Wrap_A_Buffer(ap);
			}
		}

		/*
		**	The backward moving method will probably never be called in actual practice.
		**	Implement in terms of the forward copying method until the need for this
		**	version arrises.
		*/
		virtual void BlitBackward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000) const override {BlitForward(dest, source, length, z_min, z_buff, a_buff, alpha_level);}

	private:
		T const * TranslateTable;
		T Mask;
};


/*
 * 50% translucency gated by the alpha buffer. A non-transparent source pixel is
 * blended only where the alpha-buffer value is zero. Complement of
 * BlitTranslucent50NonzeroAlpha.
 */
template<class T>
class BlitTranslucent50ZeroAlpha : public Blitter {
	public:
		BlitTranslucent50ZeroAlpha(T const * translator, T mask) : TranslateTable(translator), Mask(mask) {assert(TranslateTable != NULL);}
		virtual void BlitForward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000, int warp_offset = 0) const override
		{
			unsigned short *ap = (unsigned short *)a_buff;

			for (int i = 0; i < length; ++i) {
				unsigned char value = *(unsigned char *)source;
				source = (unsigned char *)source + 1;

				if (value != 0 && *ap == 0) {
					*(T *)dest = ((*(T *)dest / 2) & Mask) + ((TranslateTable[value] / 2) & Mask);
				}

				ap = ap + 1;
				dest = (T *)dest + 1;

				ap = Blit_Wrap_A_Buffer(ap);
			}
		}

		/*
		**	The backward moving method will probably never be called in actual practice.
		**	Implement in terms of the forward copying method until the need for this
		**	version arrises.
		*/
		virtual void BlitBackward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000) const override {BlitForward(dest, source, length, z_min, z_buff, a_buff, alpha_level);}

	private:
		T const * TranslateTable;
		T Mask;
};


/*
**	This blitter performs 25% translucency as it draws. This effect is less than spectacular,
**	but there are some uses for it. It only works with hicolor pixels.
*/
template<class T>
class BlitTransLucent25 : public Blitter {
	public:
		BlitTransLucent25(T const * translator, T mask) : TranslateTable(translator), Mask(mask) {assert(TranslateTable != NULL);}
		virtual void BlitForward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000, int warp_offset = 0) const override
		{
			for (int index = 0; index < length; index++) {
				unsigned char color = *(unsigned char const *)source;
				source = ((unsigned char *)source) + 1;
				if (color != 0) {
					T qsource = (T)(((TranslateTable[color] >> 2) & Mask));
					T qdest = (T)((((*(T *)dest) >> 2) & Mask));
					*((T *)dest) = (T)(qdest + qsource + qsource + qsource);
				}
				dest = ((T *)dest) + 1;
			}
		}

		/*
		**	The backward moving method will probably never be called in actual practice.
		**	Implement in terms of the forward copying method until the need for this
		**	version arrises.
		*/
		virtual void BlitBackward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000) const override {BlitForward(dest, source, length, z_min, z_buff, a_buff, alpha_level);}

	private:
		T const * TranslateTable;
		T Mask;
};


/*
 * BlitTransLucent25 (25% translucency) with the alpha-lighting remap table
 * combined into the translate index, applying scene lighting to each blended pixel.
 */
template<class T>
class BlitTransLucent25Alpha : public Blitter {
	public:
		BlitTransLucent25Alpha(T const * translator, int intensity_levels, T mask) : TranslateTable(translator), AlphaLightingRemap(NULL), Mask(mask) {AlphaLightingRemap = AlphaLightingRemapInit.Init(intensity_levels);}
		virtual ~BlitTransLucent25Alpha(void) override { AlphaLightingRemapInit.Deinit(AlphaLightingRemap); AlphaLightingRemap = NULL; }

		virtual void BlitForward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000, int warp_offset = 0) const override
		{
			unsigned short *ap = (unsigned short *)a_buff;
			const unsigned short* aLUT = AlphaLightingRemap->Get_Table(alpha_level);

			for (int index = 0; index < length; index++) {
				unsigned char color = *(unsigned char const *)source;
				source = ((unsigned char *)source) + 1;
				if (color != 0) {
					T qsource = (T)(((TranslateTable[color | aLUT[*ap]] >> 2) & Mask));
					T qdest = (T)((((*(T *)dest) >> 2) & Mask));
					*((T *)dest) = (T)(qdest + qsource + qsource + qsource);
				}
				dest = ((T *)dest) + 1;

				ap++;
				ap = Blit_Wrap_A_Buffer(ap);
			}
		}

		/*
		**	The backward moving method will probably never be called in actual practice.
		**	Implement in terms of the forward copying method until the need for this
		**	version arrises.
		*/
		virtual void BlitBackward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000) const override {BlitForward(dest, source, length, z_min, z_buff, a_buff, alpha_level);}

	private:
		T const * TranslateTable;
		AlphaLightingRemapClass *AlphaLightingRemap;
		T Mask;
};


/*
 * BlitTransLucent25Alpha with a depth-buffer read. Pixels are 25% blended (with
 * alpha lighting) only where they pass the z-test. The depth buffer is not modified.
 */
template<class T>
class BlitTransLucent25AlphaZRead : public Blitter {
	public:
		BlitTransLucent25AlphaZRead(T const * translator, int intensity_levels, T mask) : TranslateTable(translator), AlphaLightingRemap(NULL), Mask(mask) {AlphaLightingRemap = AlphaLightingRemapInit.Init(intensity_levels);}
		virtual ~BlitTransLucent25AlphaZRead(void) override { AlphaLightingRemapInit.Deinit(AlphaLightingRemap); AlphaLightingRemap = NULL; }

		virtual void BlitForward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000, int warp_offset = 0) const override
		{
			unsigned short *zb = (unsigned short *)z_buff;
			unsigned short *ap = (unsigned short *)a_buff;
			const unsigned short* aLUT = AlphaLightingRemap->Get_Table(alpha_level);

			for (int index = 0; index < length; index++) {
				if (z_min < *zb++) {
					unsigned char color = *(unsigned char const *)source;
					if (color != 0) {
						T qsource = (T)(((TranslateTable[color | aLUT[*ap]] >> 2) & Mask));
						T qdest = (T)((((*(T *)dest) >> 2) & Mask));
						*((T *)dest) = (T)(qdest + qsource + qsource + qsource);
					}
				}
				source = ((unsigned char *)source) + 1;
				dest = ((T *)dest) + 1;

				zb = Blit_Wrap_Z_Buffer(zb);
				ap++;
				ap = Blit_Wrap_A_Buffer(ap);
			}
		}

		/*
		**	The backward moving method will probably never be called in actual practice.
		**	Implement in terms of the forward copying method until the need for this
		**	version arrises.
		*/
		virtual void BlitBackward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000) const override {BlitForward(dest, source, length, z_min, z_buff, a_buff, alpha_level);}

	private:
		T const * TranslateTable;
		AlphaLightingRemapClass *AlphaLightingRemap;
		T Mask;
};


/*
 * BlitTransLucent25AlphaZRead, but the destination pixel is sampled through
 * warp_offset before blending, producing a distortion / heat-haze effect.
 */
template<class T>
class BlitTransLucent25AlphaZReadWarp : public Blitter {
	public:
		BlitTransLucent25AlphaZReadWarp(T const * translator, int intensity_levels, T mask) : TranslateTable(translator), AlphaLightingRemap(NULL), Mask(mask) {AlphaLightingRemap = AlphaLightingRemapInit.Init(intensity_levels);}
		virtual ~BlitTransLucent25AlphaZReadWarp(void) override { AlphaLightingRemapInit.Deinit(AlphaLightingRemap); AlphaLightingRemap = NULL; }

		virtual void BlitForward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000, int warp_offset = 0) const override
		{
			unsigned short *zb = (unsigned short *)z_buff;
			unsigned short *ap = (unsigned short *)a_buff;
			const unsigned short* aLUT = AlphaLightingRemap->Get_Table(alpha_level);

			for (int index = 0; index < length; index++) {
				if (z_min < *zb++) {
					unsigned char color = *(unsigned char const *)source;
					if (color != 0) {
						T qsource = (T)(((TranslateTable[color | aLUT[*ap]] >> 2) & Mask));
						T qdest = (T)(((((T *)dest)[warp_offset] >> 2) & Mask));
						*((T *)dest) = (T)(qdest + qsource + qsource + qsource);
					}
				}
				source = ((unsigned char *)source) + 1;
				dest = ((T *)dest) + 1;

				zb = Blit_Wrap_Z_Buffer(zb);
				ap++;
				ap = Blit_Wrap_A_Buffer(ap);
			}
		}

		/*
		**	The backward moving method will probably never be called in actual practice.
		**	Implement in terms of the forward copying method until the need for this
		**	version arrises.
		*/
		virtual void BlitBackward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000) const override {BlitForward(dest, source, length, z_min, z_buff, a_buff, alpha_level);}

	private:
		T const * TranslateTable;
		AlphaLightingRemapClass *AlphaLightingRemap;
		T Mask;
};


/*
 * BlitTransLucent25Alpha with a depth-buffer read and write. Pixels that pass the
 * z-test are 25% blended (with alpha lighting) and z_min is written back into the
 * depth buffer.
 */
template<class T>
class BlitTransLucent25AlphaZReadWrite : public Blitter {
	public:
		BlitTransLucent25AlphaZReadWrite(T const * translator, int intensity_levels, T mask) : TranslateTable(translator), AlphaLightingRemap(NULL), Mask(mask) {AlphaLightingRemap = AlphaLightingRemapInit.Init(intensity_levels);}
		virtual ~BlitTransLucent25AlphaZReadWrite(void) override { AlphaLightingRemapInit.Deinit(AlphaLightingRemap); AlphaLightingRemap = NULL; }

		virtual void BlitForward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000, int warp_offset = 0) const override
		{
			unsigned short *zb = (unsigned short *)z_buff;
			unsigned short *ap = (unsigned short *)a_buff;
			const unsigned short* aLUT = AlphaLightingRemap->Get_Table(alpha_level);

			for (int index = 0; index < length; index++) {
				if (z_min < *zb) {
					unsigned char color = *(unsigned char const *)source;
					if (color != 0) {
						T qsource = (T)(((TranslateTable[color | aLUT[*ap]] >> 2) & Mask));
						T qdest = (T)((((*(T *)dest) >> 2) & Mask));
						*((T *)dest) = (T)(qdest + qsource + qsource + qsource);
						*zb = z_min;
					}
				}
				source = ((unsigned char *)source) + 1;
				dest = ((T *)dest) + 1;

				zb++;
				zb = Blit_Wrap_Z_Buffer(zb);
				ap++;
				ap = Blit_Wrap_A_Buffer(ap);
			}
		}

		/*
		**	The backward moving method will probably never be called in actual practice.
		**	Implement in terms of the forward copying method until the need for this
		**	version arrises.
		*/
		virtual void BlitBackward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000) const override {BlitForward(dest, source, length, z_min, z_buff, a_buff, alpha_level);}

	private:
		T const * TranslateTable;
		AlphaLightingRemapClass *AlphaLightingRemap;
		T Mask;
};


/*
 * BlitTransLucent25 with a depth-buffer read. Pixels are 25% blended only where
 * they pass the z-test. The depth buffer is not modified.
 */
template<class T>
class BlitTransLucent25ZRead : public Blitter {
	public:
		BlitTransLucent25ZRead(T const * translator, T mask) : TranslateTable(translator), Mask(mask) {assert(TranslateTable != NULL);}
		virtual void BlitForward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000, int warp_offset = 0) const override
		{
			unsigned short *zb = (unsigned short *)z_buff;
			for (int index = 0; index < length; index++) {
				if (z_min < *zb++) {
					unsigned char color = *(unsigned char const *)source;
					if (color != 0) {
						T qsource = (T)(((TranslateTable[color] >> 2) & Mask));
						T qdest = (T)((((*(T *)dest) >> 2) & Mask));
						*((T *)dest) = (T)(qdest + qsource + qsource + qsource);
					}
				}
				source = ((unsigned char *)source) + 1;
				dest = ((T *)dest) + 1;

				zb = Blit_Wrap_Z_Buffer(zb);
			}
		}

		/*
		**	The backward moving method will probably never be called in actual practice.
		**	Implement in terms of the forward copying method until the need for this
		**	version arrises.
		*/
		virtual void BlitBackward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000) const override {BlitForward(dest, source, length, z_min, z_buff, a_buff, alpha_level);}

	private:
		T const * TranslateTable;
		T Mask;
};


/*
 * BlitTransLucent25ZRead, but the destination pixel is sampled through warp_offset
 * before blending, producing a distortion / heat-haze effect.
 */
template<class T>
class BlitTransLucent25ZReadWarp : public Blitter {
	public:
		BlitTransLucent25ZReadWarp(T const * translator, T mask) : TranslateTable(translator), Mask(mask) {assert(TranslateTable != NULL);}
		virtual void BlitForward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000, int warp_offset = 0) const override
		{
			unsigned short *zb = (unsigned short *)z_buff;
			for (int index = 0; index < length; index++) {
				if (z_min < *zb++) {
					unsigned char color = *(unsigned char const *)source;
					if (color != 0) {
						T qsource = (T)(((TranslateTable[color] >> 2) & Mask));
						T qdest = (T)(((((T *)dest)[warp_offset] >> 2) & Mask));
						*((T *)dest) = (T)(qdest + qsource + qsource + qsource);
					}
				}
				source = ((unsigned char *)source) + 1;
				dest = ((T *)dest) + 1;

				zb = Blit_Wrap_Z_Buffer(zb);
			}
		}

		/*
		**	The backward moving method will probably never be called in actual practice.
		**	Implement in terms of the forward copying method until the need for this
		**	version arrises.
		*/
		virtual void BlitBackward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000) const override {BlitForward(dest, source, length, z_min, z_buff, a_buff, alpha_level);}

	private:
		T const * TranslateTable;
		T Mask;
};


/*
 * BlitTransLucent25 with a depth-buffer read and write. Pixels that pass the
 * z-test are 25% blended and z_min is written back into the depth buffer.
 */
template<class T>
class BlitTransLucent25ZReadWrite : public Blitter {
	public:
		BlitTransLucent25ZReadWrite(T const * translator, T mask) : TranslateTable(translator), Mask(mask) {assert(TranslateTable != NULL);}
		virtual void BlitForward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000, int warp_offset = 0) const override
		{
			unsigned short *zb = (unsigned short *)z_buff;
			for (int index = 0; index < length; index++) {
				if (z_min < *zb) {
					unsigned char color = *(unsigned char const *)source;
					if (color != 0) {
						T qsource = (T)(((TranslateTable[color] >> 2) & Mask));
						T qdest = (T)((((*(T *)dest) >> 2) & Mask));
						*((T *)dest) = (T)(qdest + qsource + qsource + qsource);
						*zb = z_min;
					}
				}
				zb++;
				source = ((unsigned char *)source) + 1;
				dest = ((T *)dest) + 1;

				zb = Blit_Wrap_Z_Buffer(zb);
			}
		}

		/*
		**	The backward moving method will probably never be called in actual practice.
		**	Implement in terms of the forward copying method until the need for this
		**	version arrises.
		*/
		virtual void BlitBackward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000) const override {BlitForward(dest, source, length, z_min, z_buff, a_buff, alpha_level);}

	private:
		T const * TranslateTable;
		T Mask;
};


/*
**	This blitter performs 75% translucency as it draws. This is quite useful for explosions and
**	other gas animation effects. It only works with hicolor pixels and is a good candidate
**	for optimization.
*/
template<class T>
class BlitTransLucent75 : public Blitter {
	public:
		BlitTransLucent75(T const * translator, T mask) : TranslateTable(translator), Mask(mask) {assert(TranslateTable != NULL);}
		virtual void BlitForward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000, int warp_offset = 0) const override
		{
			for (int index = 0; index < length; index++) {
				unsigned char color = *(unsigned char const *)source;
				source = ((unsigned char *)source) + 1;
				if (color != 0) {
					T qsource = (T)(((TranslateTable[color] >> 2) & Mask));
					T qdest = (T)(((*(T *)dest) >> 2) & Mask);
					*((T *)dest) = (T)(qdest + qdest + qdest + qsource);
				}
				dest = ((T *)dest) + 1;
			}
		}

		/*
		**	The backward moving method will probably never be called in actual practice.
		**	Implement in terms of the forward copying method until the need for this
		**	version arrises.
		*/
		virtual void BlitBackward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000) const override {BlitForward(dest, source, length, z_min, z_buff, a_buff, alpha_level);}

	private:
		T const * TranslateTable;
		T Mask;
};


/*
 * BlitTransLucent75 (75% translucency) with the alpha-lighting remap table
 * combined into the translate index, applying scene lighting to each blended pixel.
 */
template<class T>
class BlitTransLucent75Alpha : public Blitter {
	public:
		BlitTransLucent75Alpha(T const * translator, int intensity_levels, T mask) : TranslateTable(translator), AlphaLightingRemap(NULL), Mask(mask) {AlphaLightingRemap = AlphaLightingRemapInit.Init(intensity_levels);}
		virtual ~BlitTransLucent75Alpha(void) override { AlphaLightingRemapInit.Deinit(AlphaLightingRemap); AlphaLightingRemap = NULL; }

		virtual void BlitForward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000, int warp_offset = 0) const override
		{
			unsigned short *ap = (unsigned short *)a_buff;
			const unsigned short* aLUT = AlphaLightingRemap->Get_Table(alpha_level);

			for (int index = 0; index < length; index++) {
				unsigned char color = *(unsigned char const *)source;
				source = ((unsigned char *)source) + 1;
				if (color != 0) {
					T qsource = (T)(((TranslateTable[color | aLUT[*ap]] >> 2) & Mask));
					T qdest = (T)(((*(T *)dest) >> 2) & Mask);
					*((T *)dest) = (T)(qdest + qdest + qdest + qsource);
				}
				dest = ((T *)dest) + 1;

				ap++;
				ap = Blit_Wrap_A_Buffer(ap);
			}
		}

		/*
		**	The backward moving method will probably never be called in actual practice.
		**	Implement in terms of the forward copying method until the need for this
		**	version arrises.
		*/
		virtual void BlitBackward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000) const override {BlitForward(dest, source, length, z_min, z_buff, a_buff, alpha_level);}

	private:
		T const * TranslateTable;
		AlphaLightingRemapClass *AlphaLightingRemap;
		T Mask;
};


/*
 * BlitTransLucent75Alpha with a depth-buffer read. Pixels are 75% blended (with
 * alpha lighting) only where they pass the z-test. The depth buffer is not modified.
 */
template<class T>
class BlitTransLucent75AlphaZRead : public Blitter {
	public:
		BlitTransLucent75AlphaZRead(T const * translator, int intensity_levels, T mask) : TranslateTable(translator), AlphaLightingRemap(NULL), Mask(mask) {AlphaLightingRemap = AlphaLightingRemapInit.Init(intensity_levels);}
		virtual ~BlitTransLucent75AlphaZRead(void) override { AlphaLightingRemapInit.Deinit(AlphaLightingRemap); AlphaLightingRemap = NULL; }

		virtual void BlitForward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000, int warp_offset = 0) const override
		{
			unsigned short *zb = (unsigned short *)z_buff;
			unsigned short *ap = (unsigned short *)a_buff;
			const unsigned short* aLUT = AlphaLightingRemap->Get_Table(alpha_level);

			for (int index = 0; index < length; index++) {
				if (z_min < *zb++) {
					unsigned char color = *(unsigned char const *)source;
					if (color != 0) {
						*((T *)dest) = (T)((3 * (((*(T *)dest) >> 2) & Mask)) + ((TranslateTable[color | aLUT[*ap]] >> 2) & Mask));
					}
				}
				source = ((unsigned char *)source) + 1;
				dest = ((T *)dest) + 1;

				zb = Blit_Wrap_Z_Buffer(zb);
				ap++;
				ap = Blit_Wrap_A_Buffer(ap);
			}
		}

		/*
		**	The backward moving method will probably never be called in actual practice.
		**	Implement in terms of the forward copying method until the need for this
		**	version arrises.
		*/
		virtual void BlitBackward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000) const override {BlitForward(dest, source, length, z_min, z_buff, a_buff, alpha_level);}

	private:
		T const * TranslateTable;
		AlphaLightingRemapClass *AlphaLightingRemap;
		T Mask;
};


/*
 * BlitTransLucent75AlphaZRead, but the destination pixel is sampled through
 * warp_offset before blending, producing a distortion / heat-haze effect.
 */
template<class T>
class BlitTransLucent75AlphaZReadWarp : public Blitter {
	public:
		BlitTransLucent75AlphaZReadWarp(T const * translator, int intensity_levels, T mask) : TranslateTable(translator), AlphaLightingRemap(NULL), Mask(mask) {AlphaLightingRemap = AlphaLightingRemapInit.Init(intensity_levels);}
		virtual ~BlitTransLucent75AlphaZReadWarp(void) override { AlphaLightingRemapInit.Deinit(AlphaLightingRemap); AlphaLightingRemap = NULL; }

		virtual void BlitForward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000, int warp_offset = 0) const override
		{
			unsigned short *zb = (unsigned short *)z_buff;
			unsigned short *ap = (unsigned short *)a_buff;
			const unsigned short* aLUT = AlphaLightingRemap->Get_Table(alpha_level);

			for (int index = 0; index < length; index++) {
				if (z_min < *zb++) {
					unsigned char color = *(unsigned char const *)source;
					if (color != 0) {
						T qsource = (T)(((TranslateTable[color | aLUT[*ap]] >> 2) & Mask));
						T qdest = (T)(((((T *)dest)[warp_offset] >> 2) & Mask));
						*((T *)dest) = (T)(qdest + qdest + qdest + qsource);
					}
				}
				source = ((unsigned char *)source) + 1;
				dest = ((T *)dest) + 1;

				zb = Blit_Wrap_Z_Buffer(zb);
				ap++;
				ap = Blit_Wrap_A_Buffer(ap);
			}
		}

		/*
		**	The backward moving method will probably never be called in actual practice.
		**	Implement in terms of the forward copying method until the need for this
		**	version arrises.
		*/
		virtual void BlitBackward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000) const override {BlitForward(dest, source, length, z_min, z_buff, a_buff, alpha_level);}

	private:
		T const * TranslateTable;
		AlphaLightingRemapClass *AlphaLightingRemap;
		T Mask;
};


/*
 * BlitTransLucent75Alpha with a depth-buffer read and write. Pixels that pass the
 * z-test are 75% blended (with alpha lighting) and z_min is written back into the
 * depth buffer.
 */
template<class T>
class BlitTransLucent75AlphaZReadWrite : public Blitter {
	public:
		BlitTransLucent75AlphaZReadWrite(T const * translator, int intensity_levels, T mask) : TranslateTable(translator), AlphaLightingRemap(NULL), Mask(mask) {AlphaLightingRemap = AlphaLightingRemapInit.Init(intensity_levels);}
		virtual ~BlitTransLucent75AlphaZReadWrite(void) override { AlphaLightingRemapInit.Deinit(AlphaLightingRemap); AlphaLightingRemap = NULL; }

		virtual void BlitForward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000, int warp_offset = 0) const override
		{
			unsigned short *zb = (unsigned short *)z_buff;
			unsigned short *ap = (unsigned short *)a_buff;
			const unsigned short* aLUT = AlphaLightingRemap->Get_Table(alpha_level);

			for (int index = 0; index < length; index++) {
				if (z_min < *zb) {
					unsigned char color = *(unsigned char const *)source;
					if (color != 0) {
						*((T *)dest) = (T)((3 * (((*(T *)dest) >> 2) & Mask)) + ((TranslateTable[color | aLUT[*ap]] >> 2) & Mask));
						*zb = z_min;
					}
				}
				source = ((unsigned char *)source) + 1;
				dest = ((T *)dest) + 1;

				zb++;
				zb = Blit_Wrap_Z_Buffer(zb);
				ap++;
				ap = Blit_Wrap_A_Buffer(ap);
			}
		}

		/*
		**	The backward moving method will probably never be called in actual practice.
		**	Implement in terms of the forward copying method until the need for this
		**	version arrises.
		*/
		virtual void BlitBackward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000) const override {BlitForward(dest, source, length, z_min, z_buff, a_buff, alpha_level);}

	private:
		T const * TranslateTable;
		AlphaLightingRemapClass *AlphaLightingRemap;
		T Mask;
};


/*
 * BlitTransLucent75 with a depth-buffer read. Pixels are 75% blended only where
 * they pass the z-test. The depth buffer is not modified.
 */
template<class T>
class BlitTransLucent75ZRead : public Blitter {
	public:
		BlitTransLucent75ZRead(T const * translator, T mask) : TranslateTable(translator), Mask(mask) {assert(TranslateTable != NULL);}
		virtual void BlitForward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000, int warp_offset = 0) const override
		{
			unsigned short *zb = (unsigned short *)z_buff;
			for (int index = 0; index < length; index++) {
				if (z_min < *zb++) {
					unsigned char color = *(unsigned char const *)source;
					if (color != 0) {
						T qsource = (T)(((TranslateTable[color] >> 2) & Mask));
						T qdest = (T)(((*(T *)dest) >> 2) & Mask);
						*((T *)dest) = (T)(qdest + qdest + qdest + qsource);
					}
				}
				source = ((unsigned char *)source) + 1;
				dest = ((T *)dest) + 1;

				zb = Blit_Wrap_Z_Buffer(zb);
			}
		}

		/*
		**	The backward moving method will probably never be called in actual practice.
		**	Implement in terms of the forward copying method until the need for this
		**	version arrises.
		*/
		virtual void BlitBackward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000) const override {BlitForward(dest, source, length, z_min, z_buff, a_buff, alpha_level);}

	private:
		T const * TranslateTable;
		T Mask;
};


/*
 * BlitTransLucent75ZRead, but the destination pixel is sampled through warp_offset
 * before blending, producing a distortion / heat-haze effect.
 */
template<class T>
class BlitTransLucent75ZReadWarp : public Blitter {
	public:
		BlitTransLucent75ZReadWarp(T const * translator, T mask) : TranslateTable(translator), Mask(mask) {assert(TranslateTable != NULL);}
		virtual void BlitForward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000, int warp_offset = 0) const override
		{
			unsigned short *zb = (unsigned short *)z_buff;
			for (int index = 0; index < length; index++) {
				if (z_min < *zb++) {
					unsigned char color = *(unsigned char const *)source;
					if (color != 0) {
						T qsource = (T)(((TranslateTable[color] >> 2) & Mask));
						T qdest = (T)(((((T *)dest)[warp_offset]) >> 2) & Mask);
						*((T *)dest) = (T)(qdest + qdest + qdest + qsource);
					}
				}
				source = ((unsigned char *)source) + 1;
				dest = ((T *)dest) + 1;

				zb = Blit_Wrap_Z_Buffer(zb);
			}
		}

		/*
		**	The backward moving method will probably never be called in actual practice.
		**	Implement in terms of the forward copying method until the need for this
		**	version arrises.
		*/
		virtual void BlitBackward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000) const override {BlitForward(dest, source, length, z_min, z_buff, a_buff, alpha_level);}

	private:
		T const * TranslateTable;
		T Mask;
};


/*
 * BlitTransLucent75 with a depth-buffer read and write. Pixels that pass the
 * z-test are 75% blended and z_min is written back into the depth buffer.
 */
template<class T>
class BlitTransLucent75ZReadWrite : public Blitter {
	public:
		BlitTransLucent75ZReadWrite(T const * translator, T mask) : TranslateTable(translator), Mask(mask) {assert(TranslateTable != NULL);}
		virtual void BlitForward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000, int warp_offset = 0) const override
		{
			unsigned short *zb = (unsigned short *)z_buff;
			for (int index = 0; index < length; index++) {
				if (z_min < *zb) {
					unsigned char color = *(unsigned char const *)source;
					if (color != 0) {
						T qsource = (T)(((TranslateTable[color] >> 2) & Mask));
						T qdest = (T)(((*(T *)dest) >> 2) & Mask);
						*((T *)dest) = (T)(qdest + qdest + qdest + qsource);
						*zb = z_min;
					}
				}
				zb++;
				source = ((unsigned char *)source) + 1;
				dest = ((T *)dest) + 1;

				zb = Blit_Wrap_Z_Buffer(zb);
			}
		}

		/*
		**	The backward moving method will probably never be called in actual practice.
		**	Implement in terms of the forward copying method until the need for this
		**	version arrises.
		*/
		virtual void BlitBackward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000) const override {BlitForward(dest, source, length, z_min, z_buff, a_buff, alpha_level);}

	private:
		T const * TranslateTable;
		T Mask;
};


/*
 * 75% translucency gated by the alpha buffer. A non-transparent source pixel is
 * blended only where the alpha-buffer value is nonzero.
 */
template<class T>
class BlitTranslucent75NonzeroAlpha : public Blitter {
	public:
		BlitTranslucent75NonzeroAlpha(T const * translator, T mask) : TranslateTable(translator), Mask(mask) {assert(TranslateTable != NULL);}
		virtual void BlitForward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000, int warp_offset = 0) const override
		{
			unsigned short *ap = (unsigned short *)a_buff;

			for (int i = 0; i < length; ++i) {
				unsigned char value = *(unsigned char *)source;
				source = (unsigned char *)source + 1;

				if (value != 0 && *ap != 0) {
					*(T *)dest = ((TranslateTable[value] >> 2) & Mask) + 3 * ((*(T *)dest >> 2) & Mask);
				}

				ap = ap + 1;
				dest = (T *)dest + 1;

				ap = Blit_Wrap_A_Buffer(ap);
			}
		}

		/*
		**	The backward moving method will probably never be called in actual practice.
		**	Implement in terms of the forward copying method until the need for this
		**	version arrises.
		*/
		virtual void BlitBackward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000) const override {BlitForward(dest, source, length, z_min, z_buff, a_buff, alpha_level);}

	private:
		T const * TranslateTable;
		T Mask;
};


/*
 * 75% translucency gated by the alpha buffer. A non-transparent source pixel is
 * blended only where the alpha-buffer value is zero. Complement of
 * BlitTranslucent75NonzeroAlpha.
 */
template<class T>
class BlitTranslucent75ZeroAlpha : public Blitter {
	public:
		BlitTranslucent75ZeroAlpha(T const * translator, T mask) : TranslateTable(translator), Mask(mask) {assert(TranslateTable != NULL);}
		virtual void BlitForward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000, int warp_offset = 0) const override
		{
			unsigned short *ap = (unsigned short *)a_buff;

			for (int i = 0; i < length; ++i) {
				unsigned char value = *(unsigned char *)source;
				source = (unsigned char *)source + 1;

				if (value != 0 && *ap == 0) {
					*(T *)dest = ((TranslateTable[value] >> 2) & Mask) + 3 * ((*(T *)dest >> 2) & Mask);
				}

				ap = ap + 1;
				dest = (T *)dest + 1;

				ap = Blit_Wrap_A_Buffer(ap);
			}
		}

		/*
		**	The backward moving method will probably never be called in actual practice.
		**	Implement in terms of the forward copying method until the need for this
		**	version arrises.
		*/
		virtual void BlitBackward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000) const override {BlitForward(dest, source, length, z_min, z_buff, a_buff, alpha_level);}

	private:
		T const * TranslateTable;
		T Mask;
};


/*
 * Alpha-blends source into dest using per-pixel alpha from the alpha buffer. For
 * each non-transparent source pixel, the source and destination colors are
 * deconstructed to RGB and combined by the alpha-buffer weight (alpha1 for source,
 * 255 - alpha1 for dest), then rebuilt as a hicolor pixel. Full per-channel alpha
 * compositing rather than the fixed 25/50/75 blends.
 */
template<class T>
class BlitTranslucentWriteAlpha : public Blitter {
	public:
		BlitTranslucentWriteAlpha(T const * translator) : TranslateTable(translator) {assert(TranslateTable != NULL);}
		virtual void BlitForward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000, int warp_offset = 0) const override
		{
			unsigned short *ap = (unsigned short *)a_buff;

			for (int i = 0; i < length; ++i) {
				unsigned char value = *(unsigned char *)source;
				source = (unsigned char *)source + 1;

				if (value != 0) {
					int alpha1 = *ap;
					int alpha2 = 255 - alpha1;
					if (alpha1 == 255) {
						alpha1 = 256;
					}

					RGBClass rgb1 = DSurface::Deconstruct_Hicolor_Pixel(TranslateTable[value]);
					RGBClass rgb2 = DSurface::Deconstruct_Hicolor_Pixel(*(T *)dest);

					int r = ((rgb1.Get_Red() * alpha1) + (rgb2.Get_Red() * alpha2)) >> 8;
					int g = ((rgb1.Get_Green() * alpha1) + (rgb2.Get_Green() * alpha2)) >> 8;
					int b = ((rgb1.Get_Blue() * alpha1) + (rgb2.Get_Blue() * alpha2)) >> 8;

					*(T *)dest = DSurface::Build_Hicolor_Pixel(std::min(255, r), std::min(255, g), std::min(255, b));
				}

				dest = (T *)dest + 1;

				ap++;
				ap = Blit_Wrap_A_Buffer(ap);
			}
		}

		/*
		**	The backward moving method will probably never be called in actual practice.
		**	Implement in terms of the forward copying method until the need for this
		**	version arrises.
		*/
		virtual void BlitBackward(void * dest, void const * source, int length, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 1000) const override {BlitForward(dest, source, length, z_min, z_buff, a_buff, alpha_level);}

	private:
		T const * TranslateTable;
};


/*
**	Assembly versions of some of the templated blitter object functions. Borland and
**	Visual C++ support a compatible inline-assembly formats. However, Borland compiler
**	does not allow inline-assembly to be part of an inline function -- go figure.
**	It will still compile, it just generates warning messages.
*/
#if defined(_MSC_VER)

#pragma optimize("", off)

template <>
inline void BlitTrans<unsigned char>::BlitForward(void * dest, void const * source, int len, int z_min, void *z_buff, void *a_buff, int alpha_level, int warp_offset) const
{
	__asm {
		mov	esi,[source]
		mov	edi,[dest]
		mov	ecx,[len]
		dec	edi
		inc	ecx

		again:
		dec	ecx
		jz		fini
		mov	al,[esi]
		inc	edi
		inc	esi
		test	al,al
		jz		again
		mov	[edi],al
		jmp	again

		fini:
	}
}


template <>
inline void BlitTransXlat<unsigned short>::BlitForward(void * dest, void const * source, int len, int z_min, void *z_buff, void *a_buff, int alpha_level, int warp_offset) const
{
	unsigned short const * xlator = TranslateTable;

	__asm {
		mov	ebx,[xlator]
		mov	ecx,[len]
		inc	ecx
		mov	edi,[dest]
		sub	edi,2
		mov	esi,[source]
		xor	eax,eax

		again:
		dec	ecx
		jz		over
		add	edi,2
		mov	al,[esi]
		inc	esi
		or		al,al
		jz		again
		mov	dx,[ebx+eax*2]
		mov	[edi],dx
		jmp	again

		over:
	}
}


template <>
inline void BlitTransRemapXlat<unsigned short>::BlitForward(void * dest, void const * source, int len, int z_min, void *z_buff, void *a_buff, int alpha_level, int warp_offset) const
{
	unsigned short const * translator = TranslateTable;
	unsigned char const * remapper = RemapTable;

	__asm {
		mov	ecx,[len]
		mov	edi,[dest]
		sub	edi,2
		mov	esi,[source]
		mov	ebx,[remapper]
		mov	edx,[translator]
		xor	eax,eax

	again:
	/*
	**	This block is 11 cycles per pixel, if not transparent, and 5
	**	cycles per pixel, if transparent.
	*/
		dec	ecx
		jz		over
		add	edi,2
		xor	eax,eax
		lodsb
		or		al,al
		jz		again
		mov	al,[ebx+eax]				// First remap step (8 bit to 8 bit).
		mov	ax,[edx+eax*2]				// Second remap step (8 bit to 16 bit).
		mov	[edi],ax
		jmp	again

	over:
	}
}


#if 0
template <>
inline void BlitTransZRemapXlat<unsigned short>::BlitForward(void * dest, void const * source, int len, int z_min, void *z_buff, void *a_buff, int alpha_level, int warp_offset) const
{
	unsigned short const * translator = TranslateTable;
	unsigned char const * remapper = *RemapTable;

	__asm {
		mov	ecx,[len]
		mov	edi,[dest]
		sub	edi,2
		mov	esi,[source]
		mov	ebx,[remapper]
		mov	edx,[translator]
		xor	eax,eax
	}

	/*
	**	This block is 11 cycles per pixel, if not transparent, and 5
	**	cycles per pixel, if transparent.
	*/
again:
	__asm {
		dec	ecx
		jz		over
		add	edi,2
		xor	eax,eax
		lodsb
		or		al,al
		jz		again
		mov	al,[ebx+eax]				// First remap step (8 bit to 8 bit).
		mov	ax,[edx+eax*2]				// Second remap step (8 bit to 16 bit).
		mov	[edi],ax
		jmp	again
	}
over:;
}


template <>
inline void BlitPlainXlat<unsigned short>::BlitForward(void * dest, void const * source, int len, int z_min, void *z_buff, void *a_buff, int alpha_level, int warp_offset) const
{
	unsigned short const * remapper = TranslateTable;
	__asm {
		mov	ebx,[remapper]
		mov	ecx,[len]
		mov	esi,[source]
		mov	edi,[dest]
		sub	edi,2
	}
again:
	/*
	**	This block processes pixels at 7 clocks per pixel.
	*/
	__asm {
		xor	eax,eax
		add	edi,2
		mov	al,[esi]
		inc	esi
		mov	ax,[ebx+eax*2]
		mov	[edi],ax
		dec	ecx
		jnz	again
	}
}


#endif
#endif

#pragma optimize("", on)
