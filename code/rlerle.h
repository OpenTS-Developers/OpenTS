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
 *                     $Archive:: /Commando/Library/RLERLE.h                                  $*
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
**	This class holds the RLE enabled blitter object definitions. There is a blitter object
**	type for every kind of pixel operation required of RLE shapes. These are defined as
**	templates to support the different destination pixel formats.
*/

#include "blitter.h"

#include <cassert>
#include <cstring>


/*
**	This is a helper function that will skip N pixels in the RLE compressed source. This is
**	necessary for clipping purposes. The return value represents the number of transparent
**	pixels before actual pixel data starts when the RLE uncompression is resumed.
*/
inline int Skip_Leading_Pixels(unsigned char const * & sptr, int skipper)
{
	/*
	**	Skip leading pixels as requested.
	*/
	while (skipper > 0) {
		if (*sptr++ == '\0') {
			skipper -= *sptr++;
		} else {
			skipper--;
		}
	}

	/*
	**	Return with then number of leading transparent pixels in the pixel stream
	**	after the end of the skip process. This value must be tracked since the pixel
	**	skip process may have ended in the middle of a transparent pixel run.
	*/
	return(-skipper);
}


/*
**	Blits with transparency checking and translation to destination pixel format.
*/
template<class T>
class RLEBlitTransXlat : public RLEBlitter {
	public:
		RLEBlitTransXlat(T const * translator) : TranslateTable(translator) {assert(TranslateTable != NULL);}

		virtual void Blit(void * dest, void const * source, int length, int leadskip = 0, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 0, int warp_offset = 0, void * zshape = NULL) const override
		{
			unsigned char const * sptr = (unsigned char const *)source;
			T * dptr = (T *)dest;

			/*
			**	Skip any leading pixels as requested.
			*/
			if (leadskip > 0) {
				int transcount = Skip_Leading_Pixels(sptr, leadskip);
				dptr += transcount;
				length -= transcount;
			}

			/*
			**	Uncompress and store the pixel stream until the length has been
			**	exhausted.
			*/
			while (length > 0) {
				unsigned char value = *sptr++;
				if (value == '\0') {
					value = *sptr++;
					length -= value;
					dptr += value;
				} else {
					*dptr++ = TranslateTable[value];
					length -= 1;
				}
			}
		}

	private:
		T const * TranslateTable;
};


template<class T>
class RLEBlitTransXlatAlpha : public RLEBlitter
{
	public:
		RLEBlitTransXlatAlpha(T const * translator, int intensity_levels) : TranslateTable(translator), AlphaLightingRemap(NULL) {AlphaLightingRemap = AlphaLightingRemapInit.Init(intensity_levels);}
		virtual ~RLEBlitTransXlatAlpha(void) override { AlphaLightingRemapInit.Deinit(AlphaLightingRemap); AlphaLightingRemap = NULL; }

		virtual void Blit(void * dest, void const * source, int length, int leadskip = 0, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 0, int warp_offset = 0, void * zshape = NULL) const override
		{
			unsigned char const * sptr = (unsigned char const *)source;
			T * dptr = (T *)dest;
			unsigned short* ap = (unsigned short*)(a_buff);

			const unsigned short* aLUT = AlphaLightingRemap->Get_Table(alpha_level);

			/*
			**	Skip any leading pixels as requested.
			*/
			if (leadskip > 0) {
				int transcount = Skip_Leading_Pixels(sptr, leadskip);
				dptr += transcount;
				length -= transcount;

				ap += transcount;
				ap = Blit_Wrap_A_Buffer(ap);
			}

			/*
			**	Uncompress and store the pixel stream until the length has been
			**	exhausted.
			*/
			while (length > 0) {
				unsigned char value = *sptr++;

				if (value == '\0') {
					value = *sptr++;
					length -= value;
					dptr += value;
					ap += value;
				} else {
					*dptr = TranslateTable[value | aLUT[*ap]];
					length--;
					dptr++;
					ap++;
				}
				ap = Blit_Wrap_A_Buffer(ap);
			}
		}

	private:
		T const * TranslateTable;
		AlphaLightingRemapClass *AlphaLightingRemap;
};


template<class T>
class RLEBlitTransXlatAlphaZRead : public RLEBlitter
{
	public:
		RLEBlitTransXlatAlphaZRead(T const * translator, int intensity_levels) : TranslateTable(translator), AlphaLightingRemap(NULL) {AlphaLightingRemap = AlphaLightingRemapInit.Init(intensity_levels);}
		virtual ~RLEBlitTransXlatAlphaZRead(void) override { AlphaLightingRemapInit.Deinit(AlphaLightingRemap); AlphaLightingRemap = NULL; }

		virtual void Blit(void * dest, void const * source, int length, int leadskip = 0, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 0, int warp_offset = 0, void * zshape = NULL) const override
		{
			unsigned char const * sptr = (unsigned char const *)source;
			T * dptr = (T *)dest;
			unsigned short* zp = (unsigned short*)(z_buff);
			unsigned short* ap = (unsigned short*)(a_buff);
			const signed char* zs = (const signed char*)(zshape);

			const unsigned short* aLUT = AlphaLightingRemap->Get_Table(alpha_level);

			/*
			**	Skip any leading pixels as requested.
			*/
			if (leadskip > 0) {
				int transcount = Skip_Leading_Pixels(sptr, leadskip);
				dptr += transcount;
				length -= transcount;

				zp += transcount;
				zp = Blit_Wrap_Z_Buffer(zp);

				ap += transcount;
				ap = Blit_Wrap_A_Buffer(ap);
			}

			/*
			**	Uncompress and store the pixel stream until the length has been
			**	exhausted.
			*/
			while (length > 0) {
				unsigned char value = *sptr++;

				if (value == '\0') {
					value = *sptr++;
					length -= value;
					dptr += value;
					zp += value;
					zs += value;
					ap += value;
				} else {
					if ((z_min - (int)*zs++) < (int)*zp++) {
						*dptr++ = TranslateTable[value | aLUT[*ap]];
						length--;
					} else {
						dptr++;
						length--;
					}
					ap++;
				}
				zp = Blit_Wrap_Z_Buffer(zp);
				ap = Blit_Wrap_A_Buffer(ap);
			}
		}

	private:
		T const * TranslateTable;
		AlphaLightingRemapClass *AlphaLightingRemap;
};


template<class T>
class RLEBlitTransXlatAlphaZReadWrite : public RLEBlitter
{
	public:
		RLEBlitTransXlatAlphaZReadWrite(T const * translator, int intensity_levels) : TranslateTable(translator), AlphaLightingRemap(NULL) {AlphaLightingRemap = AlphaLightingRemapInit.Init(intensity_levels);}
		virtual ~RLEBlitTransXlatAlphaZReadWrite(void) override { AlphaLightingRemapInit.Deinit(AlphaLightingRemap); AlphaLightingRemap = NULL; }

		virtual void Blit(void * dest, void const * source, int length, int leadskip = 0, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 0, int warp_offset = 0, void * zshape = NULL) const override
		{
			unsigned char const * sptr = (unsigned char const *)source;
			T * dptr = (T *)dest;
			unsigned short* zp = (unsigned short*)(z_buff);
			unsigned short* ap = (unsigned short*)(a_buff);
			const signed char* zs = (const signed char*)(zshape);

			const unsigned short* aLUT = AlphaLightingRemap->Get_Table(alpha_level);

			/*
			**	Skip any leading pixels as requested.
			*/
			if (leadskip > 0) {
				int transcount = Skip_Leading_Pixels(sptr, leadskip);
				dptr += transcount;
				length -= transcount;

				zp += transcount;
				zp = Blit_Wrap_Z_Buffer(zp);

				ap += transcount;
				ap = Blit_Wrap_A_Buffer(ap);
			}

			/*
			**	Uncompress and store the pixel stream until the length has been
			**	exhausted.
			*/
			while (length > 0) {
				unsigned char value = *sptr++;

				if (value == '\0') {
					value = *sptr++;
					length -= value;
					dptr += value;
					zp += value;
					zs += value;
					ap += value;
				} else {
					if ((z_min - (int)*zs) < (int)*zp) {
						*dptr++ = TranslateTable[value | aLUT[*ap]];
						*zp++  = z_min - *zs++;
						length--;
					} else {
						zs++;
						dptr++;
						zp++;
						length--;
					}
					ap++;
				}
				zp = Blit_Wrap_Z_Buffer(zp);
				ap = Blit_Wrap_A_Buffer(ap);
			}
		}

	private:
		T const * TranslateTable;
		AlphaLightingRemapClass *AlphaLightingRemap;
};


template<class T>
class RLEBlitTransXlatZRead : public RLEBlitter
{
	public:
		RLEBlitTransXlatZRead(T const * translator) : TranslateTable(translator) {assert(TranslateTable != NULL);}

		virtual void Blit(void * dest, void const * source, int length, int leadskip = 0, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 0, int warp_offset = 0, void * zshape = NULL) const override
		{
			unsigned char const * sptr = (unsigned char const *)source;
			const signed char* zs = (const signed char*)(zshape);
			T * dptr = (T *)dest;
			unsigned short* zp = (unsigned short*)(z_buff);

			/*
			**	Skip any leading pixels as requested.
			*/
			if (leadskip > 0) {
				int transcount = Skip_Leading_Pixels(sptr, leadskip);
				dptr += transcount;
				length -= transcount;

				zp += transcount;
				zp = Blit_Wrap_Z_Buffer(zp);
			}

			/*
			**	Uncompress and store the pixel stream until the length has been
			**	exhausted.
			*/
			while (length > 0) {
				unsigned char value = *sptr++;

				if (value == '\0') {
					value = *sptr++;
					length -= value;
					dptr += value;
					zp += value;
					zs += value;
				} else {
					if ((z_min - (int)*zs++) < (int)*zp++) {
						*dptr = TranslateTable[value];
						dptr++;
						length--;
					} else {
						dptr++;
						length--;
					}
				}
				zp = Blit_Wrap_Z_Buffer(zp);
			}
		}

	private:
		T const * TranslateTable;
};


template<class T>
class RLEBlitTransXlatZReadWrite : public RLEBlitter
{
	public:
		RLEBlitTransXlatZReadWrite(T const * translator) : TranslateTable(translator) {assert(TranslateTable != NULL);}

		virtual void Blit(void * dest, void const * source, int length, int leadskip = 0, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 0, int warp_offset = 0, void * zshape = NULL) const override
		{
			unsigned char const * sptr = (unsigned char const *)source;
			T * dptr = (T *)dest;

			unsigned short* zp = (unsigned short*)(z_buff);
			const signed char* zs = (const signed char*)(zshape);

			/*
			**	Skip any leading pixels as requested.
			*/
			if (leadskip > 0) {
				int transcount = Skip_Leading_Pixels(sptr, leadskip);
				dptr += transcount;
				length -= transcount;
				zp += transcount;
				zp = Blit_Wrap_Z_Buffer(zp);
			}

			/*
			**	Uncompress and store the pixel stream until the length has been
			**	exhausted.
			*/
			while (length > 0) {
				unsigned char value = *sptr++;
				if (value == '\0') {
					value = *sptr++;
					length -= value;
					dptr += value;
					zs += value;
					zp += value;
				} else {
					if ((z_min - (int)*zs) < (int)*zp) {
						*dptr = TranslateTable[value];
						dptr++;
						*zp = z_min - *zs;
						zp++;
						zs++;
						length--;
					} else {
						dptr++;
						zp++;
						zs++;
						length--;
					}
				}
				zp = Blit_Wrap_Z_Buffer(zp);
			}
		}

	private:
		T const * TranslateTable;
};


/*
**	This blits RLE compressed pixels by first remapping through a 256 byte table and then
**	translating the pixel to screen format.
*/
template<class T>
class RLEBlitTransRemapXlat : public RLEBlitter {
	public:
		RLEBlitTransRemapXlat(unsigned char const * remapper, T const * translator) : RemapTable(remapper), TranslateTable(translator) {assert(TranslateTable != NULL);assert(RemapTable != NULL);}

		virtual void Blit(void * dest, void const * source, int length, int leadskip = 0, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 0, int warp_offset = 0, void * zshape = NULL) const override
		{
			unsigned char const * sptr = (unsigned char const *)source;
			T * dptr = (T *)dest;

			/*
			**	Skip any leading pixels as requested.
			*/
			if (leadskip > 0) {
				int transcount = Skip_Leading_Pixels(sptr, leadskip);
				dptr += transcount;
				length -= transcount;
			}

			/*
			**	Uncompress and store the pixel stream until the length has been
			**	exhausted.
			*/
			while (length > 0) {
				unsigned char value = *sptr++;
				if (value == '\0') {
					value = *sptr++;
					length -= value;
					dptr += value;
				} else {
					*dptr++ = TranslateTable[RemapTable[value]];
					length -= 1;
				}
			}
		}

	private:
		unsigned char const * RemapTable;
		T const * TranslateTable;
};


template<class T>
class RLEBlitTransRemapXlatZRead : public RLEBlitter
{
	public:
		RLEBlitTransRemapXlatZRead(unsigned char const * remapper, T const * translator) : RemapTable(remapper), TranslateTable(translator) {assert(TranslateTable != NULL);assert(RemapTable != NULL);}

		virtual void Blit(void * dest, void const * source, int length, int leadskip = 0, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 0, int warp_offset = 0, void * zshape = NULL) const override
		{
			unsigned char const * sptr = (unsigned char const *)source;
			T * dptr = (T *)dest;

			unsigned short* zp = (unsigned short*)(z_buff);
			const signed char* zs = (const signed char*)(zshape);

			/*
			**	Skip any leading pixels as requested.
			*/
			if (leadskip > 0) {
				int transcount = Skip_Leading_Pixels(sptr, leadskip);
				dptr += transcount;
				length -= transcount;

				zp += transcount;
				zp = Blit_Wrap_Z_Buffer(zp);
			}

			/*
			**	Uncompress and store the pixel stream until the length has been
			**	exhausted.
			*/
			while (length > 0) {
				unsigned char value = *sptr++;
				if (value == '\0') {
					value = *sptr++;
					length -= value;
					dptr += value;
					zs += value;
					zp += value;
				} else {
					if (z_min - *zs++ < *zp++) {
						*dptr++ = TranslateTable[RemapTable[value]];
						length--;
					} else {
						dptr++;
						length--;
					}
				}
				zp = Blit_Wrap_Z_Buffer(zp);
			}
		}

	private:
		unsigned char const * RemapTable;
		T const * TranslateTable;
};


template<class T>
class RLEBlitTransRemapXlatZReadWrite : public RLEBlitter
{
	public:
		RLEBlitTransRemapXlatZReadWrite(unsigned char const * remapper, T const * translator) : RemapTable(remapper), TranslateTable(translator) {assert(TranslateTable != NULL);assert(RemapTable != NULL);}

		virtual void Blit(void * dest, void const * source, int length, int leadskip = 0, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 0, int warp_offset = 0, void * zshape = NULL) const override
		{
			unsigned char const * sptr = (unsigned char const *)source;
			T * dptr = (T *)dest;

			unsigned short* zp = (unsigned short*)(z_buff);
			const signed char* zs = (const signed char*)(zshape);

			/*
			**	Skip any leading pixels as requested.
			*/
			if (leadskip > 0) {
				int transcount = Skip_Leading_Pixels(sptr, leadskip);
				dptr += transcount;
				length -= transcount;

				zp += transcount;
				zp = Blit_Wrap_Z_Buffer(zp);
			}

			/*
			**	Uncompress and store the pixel stream until the length has been
			**	exhausted.
			*/
			while (length > 0) {
				unsigned char value = *sptr++;
				if (value == '\0') {
					value = *sptr++;
					length -= value;
					dptr += value;
					zs += value;
					zp += value;
				} else {
					if (z_min - *zs < *zp) {
						*dptr++ = TranslateTable[RemapTable[value]];
						*zp++ = z_min - *zs++;
						length--;
					} else {
						zs++;
						dptr++;
						zp++;
						length--;
					}

				}
				zp = Blit_Wrap_Z_Buffer(zp);
			}
		}

	private:
		unsigned char const * RemapTable;
		T const * TranslateTable;
};


/*
**	This blits RLE compressed pixels by first remapping through a 256 byte table and then
**	translating the pixel to screen format. The remapping table is doubly indirected so that
**	it is possible to change the remapping table pointer without creating a separate blitter
**	object.
*/
template<class T>
class RLEBlitTransZRemapXlat : public RLEBlitter {
	public:
		RLEBlitTransZRemapXlat(unsigned char const * const * remapper, T const * translator) : RemapTable(remapper), TranslateTable(translator) {assert(TranslateTable != NULL);assert(RemapTable != NULL);}

		virtual void Blit(void * dest, void const * source, int length, int leadskip = 0, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 0, int warp_offset = 0, void * zshape = NULL) const override
		{
			unsigned char const * sptr = (unsigned char const *)source;
			T * dptr = (T *)dest;

			unsigned char const * remapper = *RemapTable;

			/*
			**	Skip any leading pixels as requested.
			*/
			if (leadskip > 0) {
				int transcount = Skip_Leading_Pixels(sptr, leadskip);
				dptr += transcount;
				length -= transcount;
			}

			/*
			**	Uncompress and store the pixel stream until the length has been
			**	exhausted.
			*/
			while (length > 0) {
				unsigned char value = *sptr++;
				if (value == '\0') {
					value = *sptr++;
					length -= value;
					dptr += value;
				} else {
					*dptr++ = TranslateTable[remapper[value]];
					length -= 1;
				}
			}
		}

	private:
		unsigned char const * const * RemapTable;
		T const * TranslateTable;
};


template<class T>
class RLEBlitTransZRemapXlatAlpha : public RLEBlitter
{
	public:
		RLEBlitTransZRemapXlatAlpha(unsigned char const * const * remapper, T const * translator, int intensity_levels) : RemapTable(remapper), TranslateTable(translator), AlphaLightingRemap(NULL) {assert(TranslateTable != NULL);assert(RemapTable != NULL); AlphaLightingRemap = AlphaLightingRemapInit.Init(intensity_levels);}
		virtual ~RLEBlitTransZRemapXlatAlpha(void) override { AlphaLightingRemapInit.Deinit(AlphaLightingRemap); AlphaLightingRemap = NULL; }

		virtual void Blit(void * dest, void const * source, int length, int leadskip = 0, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 0, int warp_offset = 0, void * zshape = NULL) const override
		{
			unsigned char const * sptr = (unsigned char const *)source;
			T * dptr = (T *)dest;

			unsigned char const * rtable = *RemapTable;
			unsigned short* ap = (unsigned short*)(a_buff);
			const unsigned short* aLUT = AlphaLightingRemap->Get_Table(alpha_level);

			/*
			**	Skip any leading pixels as requested.
			*/
			if (leadskip > 0) {
				int transcount = Skip_Leading_Pixels(sptr, leadskip);
				dptr += transcount;
				length -= transcount;
				ap += transcount;
				ap = Blit_Wrap_A_Buffer(ap);
			}

			/*
			**	Uncompress and store the pixel stream until the length has been
			**	exhausted.
			*/
			while (length > 0) {
				unsigned char value = *sptr++;
				if (value == '\0') {
					value = *sptr++;
					length -= value;
					dptr += value;
					ap += value;
				} else {
					*dptr++ = TranslateTable[rtable[value] | aLUT[*ap]];
					length -= 1;
					ap++;
				}
				ap = Blit_Wrap_A_Buffer(ap);
			}
		}

	private:
		unsigned char const * const * RemapTable;
		T const * TranslateTable;
		AlphaLightingRemapClass *AlphaLightingRemap;
};


template<class T>
class RLEBlitTransZRemapXlatAlphaZRead : public RLEBlitter
{
	public:
		RLEBlitTransZRemapXlatAlphaZRead(unsigned char const * const * remapper, T const * translator, int intensity_levels) : RemapTable(remapper), TranslateTable(translator), AlphaLightingRemap(NULL) {assert(TranslateTable != NULL);assert(RemapTable != NULL); AlphaLightingRemap = AlphaLightingRemapInit.Init(intensity_levels);}
		virtual ~RLEBlitTransZRemapXlatAlphaZRead(void) override { AlphaLightingRemapInit.Deinit(AlphaLightingRemap); AlphaLightingRemap = NULL; }

		virtual void Blit(void * dest, void const * source, int length, int leadskip = 0, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 0, int warp_offset = 0, void * zshape = NULL) const override
		{
			unsigned char const * sptr = (unsigned char const *)source;
			const signed char* zs = (const signed char*)(zshape);
			T * dptr = (T *)dest;
			unsigned char const * rtable = *RemapTable;
			unsigned short* zp = (unsigned short*)(z_buff);
			unsigned short* ap = (unsigned short*)(a_buff);
			const unsigned short* aLUT = AlphaLightingRemap->Get_Table(alpha_level);

			/*
			**	Skip any leading pixels as requested.
			*/
			if (leadskip > 0) {
				int transcount = Skip_Leading_Pixels(sptr, leadskip);
				dptr += transcount;
				length -= transcount;

				zp += transcount;
				zp = Blit_Wrap_Z_Buffer(zp);

				ap += transcount;
				ap = Blit_Wrap_A_Buffer(ap);
			}

			/*
			**	Uncompress and store the pixel stream until the length has been
			**	exhausted.
			*/
			while (length > 0) {
				unsigned char value = *sptr++;
				if (value == '\0') {
					value = *sptr++;
					length -= value;
					dptr += value;
					zp += value;
					zs += value;
					ap += value;
				} else {
					if ((z_min - (int)*zs++) < (int)*zp++) {
						*dptr++ = TranslateTable[rtable[value] | aLUT[*ap]];
						length--;
					} else {
						dptr++;
						length--;
					}
					ap++;
				}
				zp = Blit_Wrap_Z_Buffer(zp);
				ap = Blit_Wrap_A_Buffer(ap);
			}
		}

	private:
		unsigned char const * const * RemapTable;
		T const * TranslateTable;
		AlphaLightingRemapClass *AlphaLightingRemap;
};


template<class T>
class RLEBlitTransZRemapXlatAlphaZReadWrite : public RLEBlitter
{
	public:
		RLEBlitTransZRemapXlatAlphaZReadWrite(unsigned char const * const * remapper, T const * translator, int intensity_levels) : RemapTable(remapper), TranslateTable(translator), AlphaLightingRemap(NULL) {assert(TranslateTable != NULL);assert(RemapTable != NULL); AlphaLightingRemap = AlphaLightingRemapInit.Init(intensity_levels);}
		virtual ~RLEBlitTransZRemapXlatAlphaZReadWrite(void) override { AlphaLightingRemapInit.Deinit(AlphaLightingRemap); AlphaLightingRemap = NULL; }

		virtual void Blit(void * dest, void const * source, int length, int leadskip = 0, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 0, int warp_offset = 0, void * zshape = NULL) const override
		{
			unsigned char const * sptr = (unsigned char const *)source;
			T * dptr = (T *)dest;

			unsigned char const * rtable = *RemapTable;

			unsigned short* zp = (unsigned short*)(z_buff);
			unsigned short* ap = (unsigned short*)(a_buff);
			const signed char* zs = (const signed char*)(zshape);

			const unsigned short* aLUT = AlphaLightingRemap->Get_Table(alpha_level);

			/*
			**	Skip any leading pixels as requested.
			*/
			if (leadskip > 0) {
				int transcount = Skip_Leading_Pixels(sptr, leadskip);
				dptr += transcount;
				length -= transcount;

				zp += transcount;
				zp = Blit_Wrap_Z_Buffer(zp);

				ap += transcount;
				ap = Blit_Wrap_A_Buffer(ap);
			}

			/*
			**	Uncompress and store the pixel stream until the length has been
			**	exhausted.
			*/
			while (length > 0) {
				unsigned char value = *sptr++;

				if (value == '\0') {
					value = *sptr++;
					length -= value;
					dptr += value;
					zp += value;
					zs += value;
					ap += value;
				} else {
					if ((z_min - (int)*zs) < (int)*zp) {
						*dptr++ = TranslateTable[rtable[value] | aLUT[*ap]];
						*zp++  = z_min - *zs++;
						length--;
					} else {
						zs++;
						dptr++;
						zp++;
						length--;
					}
					ap++;
				}
				zp = Blit_Wrap_Z_Buffer(zp);
				ap = Blit_Wrap_A_Buffer(ap);
			}
		}

	private:
		unsigned char const * const * RemapTable;
		T const * TranslateTable;
		AlphaLightingRemapClass *AlphaLightingRemap;
};


template<class T>
class RLEBlitTransZRemapXlatZRead : public RLEBlitter
{
	public:
		RLEBlitTransZRemapXlatZRead(unsigned char const * const * remapper, T const * translator) : RemapTable(remapper), TranslateTable(translator) {assert(TranslateTable != NULL);assert(RemapTable != NULL);}

		virtual void Blit(void * dest, void const * source, int length, int leadskip = 0, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 0, int warp_offset = 0, void * zshape = NULL) const override
		{
			unsigned char const * sptr = (unsigned char const *)source;
			const signed char* zs = (const signed char*)(zshape);
			T * dptr = (T *)dest;

			unsigned char const * rtable = *RemapTable;
			unsigned short* zp = (unsigned short*)(z_buff);

			/*
			**	Skip any leading pixels as requested.
			*/
			if (leadskip > 0) {
				int transcount = Skip_Leading_Pixels(sptr, leadskip);
				dptr += transcount;
				length -= transcount;
				zp += transcount;
				zp = Blit_Wrap_Z_Buffer(zp);
			}

			/*
			**	Uncompress and store the pixel stream until the length has been
			**	exhausted.
			*/
			while (length > 0) {
				unsigned char value = *sptr++;
				if (value == '\0') {
					value = *sptr++;
					length -= value;
					dptr += value;
					zs += value;
					zp += value;
				} else {
					if (z_min - *zs++ < *zp++) {
						*dptr = TranslateTable[rtable[value]];
						dptr++;
						length--;
					} else {
						dptr++;
						length--;
					}
				}
				zp = Blit_Wrap_Z_Buffer(zp);
			}
		}

	private:
		unsigned char const * const * RemapTable;
		T const * TranslateTable;
};


template<class T>
class RLEBlitTransZRemapXlatZReadWrite : public RLEBlitter
{
	public:
		RLEBlitTransZRemapXlatZReadWrite(unsigned char const * const * remapper, T const * translator) : RemapTable(remapper), TranslateTable(translator) {assert(TranslateTable != NULL);assert(RemapTable != NULL);}

		virtual void Blit(void * dest, void const * source, int length, int leadskip = 0, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 0, int warp_offset = 0, void * zshape = NULL) const override
		{
			unsigned char const * sptr = (unsigned char const *)source;
			unsigned char const * remapper = *RemapTable;
			const signed char* zs = (const signed char*)(zshape);
			T * dptr = (T *)dest;
			unsigned short* zp = (unsigned short*)(z_buff);

			/*
			**	Skip any leading pixels as requested.
			*/
			if (leadskip > 0) {
				int transcount = Skip_Leading_Pixels(sptr, leadskip);
				dptr += transcount;
				length -= transcount;

				zp += transcount;
				zp = Blit_Wrap_Z_Buffer(zp);
			}

			/*
			**	Uncompress and store the pixel stream until the length has been
			**	exhausted.
			*/
			while (length > 0) {
				unsigned char value = *sptr++;
				if (value == '\0') {
					value = *sptr++;
					length -= value;
					dptr += value;
					zs += value;
					zp += value;
				} else {
					/// zs advances +2 per pixel (z-test + z-write both step it) vs zp +1; can overflow.
					if (z_min - *zs++ < *zp) {
						*dptr = TranslateTable[remapper[value]];
						dptr++;
						*zp = z_min - *zs;
						zs++;
						zp++;
						length--;
					} else {
						zs++;
						dptr++;
						zp++;
						length--;
					}

				}
				zp = Blit_Wrap_Z_Buffer(zp);
			}
		}

	private:
		unsigned char const * const * RemapTable;
		T const * TranslateTable;
};


/*
**	This will remap the destination pixels but under the control of the source pixels.
**	Where the source pixel is not transparent, the dest pixel is remapped. This algorithm
**	really only applies to lowcolor display.
*/
template<class T>
class RLEBlitTransRemapDest : public RLEBlitter {
	public:
		RLEBlitTransRemapDest(T const * remap) : RemapTable(remap) {}

		virtual void Blit(void * dest, void const * source, int length, int leadskip = 0, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 0, int warp_offset = 0, void * zshape = NULL) const override
		{
			unsigned char const * sptr = (unsigned char const *)source;
			T * dptr = (T *)dest;

			/*
			**	Skip any leading pixels as requested.
			*/
			if (leadskip > 0) {
				int transcount = Skip_Leading_Pixels(sptr, leadskip);
				dptr += transcount;
				length -= transcount;
			}

			/*
			**	Uncompress and store the pixel stream until the length has been
			**	exhausted.
			*/
			while (length > 0) {
				unsigned char value = *sptr++;
				if (value == '\0') {
					value = *sptr++;
					length -= value;
					dptr += value;
				} else {
					*dptr = RemapTable[*dptr];
					length -= 1;
					dptr++;
				}
			}
		}

	private:
		T const * RemapTable;
};


template<class T>
class RLEBlitTransRemapDestZRead : public RLEBlitter
{
	public:
		RLEBlitTransRemapDestZRead(T const * remap) : RemapTable(remap) {}

		virtual void Blit(void * dest, void const * source, int length, int leadskip = 0, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 0, int warp_offset = 0, void * zshape = NULL) const override
		{
			unsigned char const * sptr = (unsigned char const *)source;
			T * dptr = (T *)dest;
			unsigned short* zp = (unsigned short*)(z_buff);
			const signed char* zs = (const signed char*)(zshape);

			/*
			**	Skip any leading pixels as requested.
			*/
			if (leadskip > 0) {
				int transcount = Skip_Leading_Pixels(sptr, leadskip);
				dptr += transcount;
				length -= transcount;

				zp += transcount;
				zp = Blit_Wrap_Z_Buffer(zp);
			}

			/*
			**	Uncompress and store the pixel stream until the length has been
			**	exhausted.
			*/
			while (length > 0) {
				unsigned char value = *sptr++;

				if (value == '\0') {
					value = *sptr++;
					length -= value;
					dptr += value;
					zs += value;
					zp += value;
				} else {
					if ((z_min - *zs++) < *zp++) {
						*dptr = RemapTable[*((T *)dptr)];
						length--;
						dptr++;
					} else {
						dptr++;
						length--;
					}
				}
				zp = Blit_Wrap_Z_Buffer(zp);
			}
		}

	private:
		T const * RemapTable;
};


template<class T>
class RLEBlitTransRemapDestZReadWrite : public RLEBlitter
{
	public:
		RLEBlitTransRemapDestZReadWrite(T const * remap) : RemapTable(remap) {}

		virtual void Blit(void * dest, void const * source, int length, int leadskip = 0, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 0, int warp_offset = 0, void * zshape = NULL) const override
		{
			unsigned char const * sptr = (unsigned char const *)source;
			T * dptr = (T *)dest;

			unsigned short* zp = (unsigned short*)(z_buff);
			const signed char* zs = (const signed char*)(zshape);

			/*
			**	Skip any leading pixels as requested.
			*/
			if (leadskip > 0) {
				int transcount = Skip_Leading_Pixels(sptr, leadskip);
				dptr += transcount;
				length -= transcount;
				zp += transcount;
				zp = Blit_Wrap_Z_Buffer(zp);
			}

			/*
			**	Uncompress and store the pixel stream until the length has been
			**	exhausted.
			*/
			while (length > 0) {
				unsigned char value = *sptr++;
				if (value == '\0') {
					value = *sptr++;
					length -= value;
					dptr += value;
					zs += value;
					zp += value;
				} else {
					/// zs advances +2 per pixel (z-test + z-write both step it) vs zp +1.
					if ((z_min - (int)*zs++) < (int)*zp) {
						*dptr = RemapTable[*dptr];
						*zp = z_min - *zs;
						zp++;
						zs++;
						length--;
						dptr++;
					} else {
						zs++;
						dptr++;
						zp++;
						length--;
					}
				}
				zp = Blit_Wrap_Z_Buffer(zp);
			}
		}

	private:
		T const * RemapTable;
};


/*
**	Algorithmic darkening of hicolor pixels controlled by the source pixels. The source
**	pixels are examined only to determine if the destination pixel should be darkened.
**	If the source pixel is transparent, then the dest pixel is skipped. The darkening
**	algorithm works only for hicolor pixels.
*/
template<class T>
class RLEBlitTransDarken : public RLEBlitter {
	public:
		RLEBlitTransDarken(T mask) : Mask(mask) {}

		virtual void Blit(void * dest, void const * source, int length, int leadskip = 0, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 0, int warp_offset = 0, void * zshape = NULL) const override
		{
			unsigned char const * sptr = (unsigned char const *)source;
			T * dptr = (T *)dest;

			/*
			**	Skip any leading pixels as requested.
			*/
			if (leadskip > 0) {
				int transcount = Skip_Leading_Pixels(sptr, leadskip);
				dptr += transcount;
				length -= transcount;
			}

			/*
			**	Uncompress and store the pixel stream until the length has been
			**	exhausted.
			*/
			while (length > 0) {
				unsigned char value = *sptr++;
				if (value == '\0') {
					value = *sptr++;
					length -= value;
					dptr += value;
				} else {
					*dptr = (T)((*dptr >> 1) & Mask);
					length -= 1;
					dptr++;
				}
			}
		}

	private:
		T Mask;
};


template<class T>
class RLEBlitTransDarkenZRead : public RLEBlitter
{
	public:
		RLEBlitTransDarkenZRead(T mask) : Mask(mask) {}

		virtual void Blit(void * dest, void const * source, int length, int leadskip = 0, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 0, int warp_offset = 0, void * zshape = NULL) const override
		{
			unsigned char const * sptr = (unsigned char const *)source;
			T * dptr = (T *)dest;

			unsigned short* zp = (unsigned short*)(z_buff);
			const signed char* zs = (const signed char*)(zshape);

			/*
			**	Skip any leading pixels as requested.
			*/
			if (leadskip > 0) {
				int transcount = Skip_Leading_Pixels(sptr, leadskip);
				dptr += transcount;
				length -= transcount;

				zp += transcount;
				zp = Blit_Wrap_Z_Buffer(zp);
			}

			/*
			**	Uncompress and store the pixel stream until the length has been
			**	exhausted.
			*/
			while (length > 0) {
				unsigned char value = *sptr++;
				if (value == '\0') {
					value = *sptr++;
					length -= value;
					dptr += value;
					zs += value;
					zp += value;
				} else {
					if (z_min - *zs++ < *zp++) {
						*dptr = (T)((((*dptr) >> 1) & Mask));
						length--;
						dptr++;
					} else {
						length--;
						dptr++;
					}
				}
				zp = Blit_Wrap_Z_Buffer(zp);
			}
		}

	private:
		T Mask;
};


template<class T>
class RLEBlitTransDarkenZReadWrite : public RLEBlitter
{
	public:
		RLEBlitTransDarkenZReadWrite(T mask) : Mask(mask) {}

		virtual void Blit(void * dest, void const * source, int length, int leadskip = 0, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 0, int warp_offset = 0, void * zshape = NULL) const override
		{
			unsigned char const * sptr = (unsigned char const *)source;
			T * dptr = (T *)dest;

			unsigned short* zp = (unsigned short*)(z_buff);
			const signed char* zs = (const signed char*)(zshape);

			/*
			**	Skip any leading pixels as requested.
			*/
			if (leadskip > 0) {
				int transcount = Skip_Leading_Pixels(sptr, leadskip);
				dptr += transcount;
				length -= transcount;

				zp += transcount;
				zp = Blit_Wrap_Z_Buffer(zp);
			}

			/*
			**	Uncompress and store the pixel stream until the length has been
			**	exhausted.
			*/
			while (length > 0) {
				unsigned char value = *sptr++;
				if (value == '\0') {
					value = *sptr++;
					length -= value;
					dptr += value;
					zs += value;
					zp += value;
				} else {
					if (z_min - *zs < *zp) {
						*dptr = (T)((((*dptr) >> 1) & Mask));
						*zp++ = z_min - *zs++;
						length--;
						dptr++;
					} else {
						zs++;
						length--;
						zp++;
						dptr++;
					}
				}
				zp = Blit_Wrap_Z_Buffer(zp);
			}
		}

	private:
		T Mask;
};


/*
**	This blitter performs 50% translucency as it draws. It is commonly used for animation
**	effects and other stealth like images. It only works with hicolor pixels but is a good
**	candidate for optimization.
*/
template<class T>
class RLEBlitTransLucent50 : public RLEBlitter {
	public:
		RLEBlitTransLucent50(T const * translator, T mask) : TranslateTable(translator), Mask(mask) {}

		virtual void Blit(void * dest, void const * source, int length, int leadskip = 0, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 0, int warp_offset = 0, void * zshape = NULL) const override
		{
			unsigned char const * sptr = (unsigned char const *)source;
			T * dptr = (T *)dest;

			/*
			**	Skip any leading pixels as requested.
			*/
			if (leadskip > 0) {
				int transcount = Skip_Leading_Pixels(sptr, leadskip);
				dptr += transcount;
				length -= transcount;
			}

			/*
			**	Uncompress and store the pixel stream until the length has been
			**	exhausted.
			*/
			while (length > 0) {
				unsigned char value = *sptr++;
				if (value == '\0') {
					value = *sptr++;
					length -= value;
					dptr += value;
				} else {
					*dptr = (T)((((*dptr) >> 1) & Mask) + ((TranslateTable[value] >> 1) & Mask));
					length -= 1;
					dptr++;
				}
			}
		}

	private:
		T const * TranslateTable;
		T Mask;
};


template<class T>
class RLEBlitTransLucent50Alpha : public RLEBlitter
{
	public:
		RLEBlitTransLucent50Alpha(T const * translator, int intensity_levels, T mask) : TranslateTable(translator), Mask(mask), AlphaLightingRemap(NULL) {AlphaLightingRemap = AlphaLightingRemapInit.Init(intensity_levels);}
		virtual ~RLEBlitTransLucent50Alpha(void) override { AlphaLightingRemapInit.Deinit(AlphaLightingRemap); AlphaLightingRemap = NULL; }

		virtual void Blit(void * dest, void const * source, int length, int leadskip = 0, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 0, int warp_offset = 0, void * zshape = NULL) const override
		{
			unsigned char const * sptr = (unsigned char const *)source;
			T * dptr = (T *)dest;
			unsigned short* ap = (unsigned short*)(a_buff);

			const unsigned short* aLUT = AlphaLightingRemap->Get_Table(alpha_level);

			/*
			**	Skip any leading pixels as requested.
			*/
			if (leadskip > 0) {
				int transcount = Skip_Leading_Pixels(sptr, leadskip);
				dptr += transcount;
				length -= transcount;

				ap += transcount;
				ap = Blit_Wrap_A_Buffer(ap);
			}

			/*
			**	Uncompress and store the pixel stream until the length has been
			**	exhausted.
			*/
			while (length > 0) {
				unsigned char value = *sptr++;
				if (value == '\0') {
					value = *sptr++;
					length -= value;
					dptr += value;
					ap += value;
				} else {
					*dptr = (T)((((*dptr) >> 1) & Mask) + ((TranslateTable[value | aLUT[*ap]] >> 1) & Mask));
					length -= 1;
					dptr++;
					ap++;
				}
				ap = Blit_Wrap_A_Buffer(ap);
			}
		}

	private:
		T const * TranslateTable;
		AlphaLightingRemapClass *AlphaLightingRemap;
		T Mask;
};


template<class T>
class RLEBlitTransLucent50AlphaZRead : public RLEBlitter
{
	public:
		RLEBlitTransLucent50AlphaZRead(T const * translator, int intensity_levels, T mask) : TranslateTable(translator), Mask(mask), AlphaLightingRemap(NULL) {AlphaLightingRemap = AlphaLightingRemapInit.Init(intensity_levels);}
		virtual ~RLEBlitTransLucent50AlphaZRead(void) override { AlphaLightingRemapInit.Deinit(AlphaLightingRemap); AlphaLightingRemap = NULL; }

		virtual void Blit(void * dest, void const * source, int length, int leadskip = 0, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 0, int warp_offset = 0, void * zshape = NULL) const override
		{
			unsigned char const * sptr = (unsigned char const *)source;
			T * dptr = (T *)dest;
			unsigned short* zp = (unsigned short*)(z_buff);
			unsigned short* ap = (unsigned short*)(a_buff);
			const signed char* zs = (const signed char*)(zshape);

			const unsigned short* aLUT = AlphaLightingRemap->Get_Table(alpha_level);

			/*
			**	Skip any leading pixels as requested.
			*/
			if (leadskip > 0) {
				int transcount = Skip_Leading_Pixels(sptr, leadskip);
				dptr += transcount;
				length -= transcount;

				zp += transcount;
				zp = Blit_Wrap_Z_Buffer(zp);

				ap += transcount;
				ap = Blit_Wrap_A_Buffer(ap);
			}

			/*
			**	Uncompress and store the pixel stream until the length has been
			**	exhausted.
			*/
			while (length > 0) {
				unsigned char value = *sptr++;

				if (value == '\0') {
					value = *sptr++;
					length -= value;
					dptr += value;
					zp += value;
					zs += value;
					ap += value;
				} else {
					if ((z_min - (int)*zs++) < (int)*zp++) {
						*dptr = (T)((((*dptr) >> 1) & Mask) + ((TranslateTable[value | aLUT[*ap]] >> 1) & Mask));
						length--;
						dptr++;
					} else {
						length--;
						dptr++;
					}
					ap++;
				}
				zp = Blit_Wrap_Z_Buffer(zp);
				ap = Blit_Wrap_A_Buffer(ap);
			}
		}

	private:
		T const * TranslateTable;
		AlphaLightingRemapClass *AlphaLightingRemap;
		T Mask;
};


template<class T>
class RLEBlitTransLucent50AlphaZReadWarp : public RLEBlitter
{
	public:
		RLEBlitTransLucent50AlphaZReadWarp(T const * translator, int intensity_levels, T mask) : TranslateTable(translator), Mask(mask), AlphaLightingRemap(NULL) {AlphaLightingRemap = AlphaLightingRemapInit.Init(intensity_levels);}
		virtual ~RLEBlitTransLucent50AlphaZReadWarp(void) override { AlphaLightingRemapInit.Deinit(AlphaLightingRemap); AlphaLightingRemap = NULL; }

		virtual void Blit(void * dest, void const * source, int length, int leadskip = 0, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 0, int warp_offset = 0, void * zshape = NULL) const override
		{
			unsigned char const * sptr = (unsigned char const *)source;
			T * dptr = (T *)dest;
			unsigned short* zp = (unsigned short*)(z_buff);
			unsigned short* ap = (unsigned short*)(a_buff);
			const signed char* zs = (const signed char*)(zshape);

			const unsigned short* aLUT = AlphaLightingRemap->Get_Table(alpha_level);

			/*
			**	Skip any leading pixels as requested.
			*/
			if (leadskip > 0) {
				int transcount = Skip_Leading_Pixels(sptr, leadskip);
				dptr += transcount;
				length -= transcount;

				zp += transcount;
				zp = Blit_Wrap_Z_Buffer(zp);

				ap += transcount;
				ap = Blit_Wrap_A_Buffer(ap);
			}

			/*
			**	Uncompress and store the pixel stream until the length has been
			**	exhausted.
			*/
			while (length > 0) {
				unsigned char value = *sptr++;
				if (value == '\0') {
					value = *sptr++;
					length -= value;
					dptr += value;
					zp += value;
					zs += value;
					ap += value;
				} else {
					if ((z_min - (int)*zs++) < (int)*zp++) {
						*dptr = (T)((((dptr[warp_offset]) >> 1) & Mask) + ((TranslateTable[value | aLUT[*ap]] >> 1) & Mask));
						length--;
						dptr++;
					} else {
						length--;
						dptr++;
					}
					ap++;
				}
				zp = Blit_Wrap_Z_Buffer(zp);
				ap = Blit_Wrap_A_Buffer(ap);
			}
		}

	private:
		T const * TranslateTable;
		AlphaLightingRemapClass *AlphaLightingRemap;
		T Mask;
};


template<class T>
class RLEBlitTransLucent50AlphaZReadWrite : public RLEBlitter
{
	public:
		RLEBlitTransLucent50AlphaZReadWrite(T const * translator, int intensity_levels, T mask) : TranslateTable(translator), Mask(mask), AlphaLightingRemap(NULL) {AlphaLightingRemap = AlphaLightingRemapInit.Init(intensity_levels);}
		virtual ~RLEBlitTransLucent50AlphaZReadWrite(void) override { AlphaLightingRemapInit.Deinit(AlphaLightingRemap); AlphaLightingRemap = NULL; }

		virtual void Blit(void * dest, void const * source, int length, int leadskip = 0, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 0, int warp_offset = 0, void * zshape = NULL) const override
		{
			unsigned char const * sptr = (unsigned char const *)source;
			T * dptr = (T *)dest;
			unsigned short* zp = (unsigned short*)(z_buff);
			unsigned short* ap = (unsigned short*)(a_buff);
			const signed char* zs = (const signed char*)(zshape);

			const unsigned short* aLUT = AlphaLightingRemap->Get_Table(alpha_level);

			/*
			**	Skip any leading pixels as requested.
			*/
			if (leadskip > 0) {
				int transcount = Skip_Leading_Pixels(sptr, leadskip);
				dptr += transcount;
				length -= transcount;

				zp += transcount;
				zp = Blit_Wrap_Z_Buffer(zp);

				ap += transcount;
				ap = Blit_Wrap_A_Buffer(ap);
			}

			/*
			**	Uncompress and store the pixel stream until the length has been
			**	exhausted.
			*/
			while (length > 0) {
				unsigned char value = *sptr++;

				if (value == '\0') {
					value = *sptr++;
					length -= value;
					dptr += value;
					zp += value;
					zs += value;
					ap += value;
				} else {
					if ((z_min - (int)*zs) < (int)*zp) {
						T qsource = (T)(((TranslateTable[value | aLUT[*ap]] >> 1) & Mask));
						T qdest = (T)(((*dptr) >> 1) & Mask);
						*dptr = (T)(qdest + qsource);
						*zp++  = z_min - *zs++;
						length--;
						dptr++;
					} else {
						zs++;
						dptr++;
						zp++;
						length--;
					}
					ap++;
				}
				zp = Blit_Wrap_Z_Buffer(zp);
				ap = Blit_Wrap_A_Buffer(ap);
			}
		}

	private:
		T const * TranslateTable;
		AlphaLightingRemapClass *AlphaLightingRemap;
		T Mask;
};


template<class T>
class RLEBlitTransLucent50ZRead : public RLEBlitter
{
	public:
		RLEBlitTransLucent50ZRead(T const * translator, T mask) : TranslateTable(translator), Mask(mask) {assert(TranslateTable != NULL);}

		virtual void Blit(void * dest, void const * source, int length, int leadskip = 0, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 0, int warp_offset = 0, void * zshape = NULL) const override
		{
			unsigned short* zp = (unsigned short*)(z_buff);
			T * dptr = (T *)dest;
			const signed char* zs = (const signed char*)(zshape);
			unsigned char const * sptr = (unsigned char const *)source;

			/*
			**	Skip any leading pixels as requested.
			*/
			if (leadskip > 0) {
				int transcount = Skip_Leading_Pixels(sptr, leadskip);
				dptr += transcount;
				length -= transcount;

				zp += transcount;
				zp = Blit_Wrap_Z_Buffer(zp);
			}

			/*
			**	Uncompress and store the pixel stream until the length has been
			**	exhausted.
			*/
			while (length > 0) {
				unsigned char value = *sptr++;
				if (value == '\0') {
					value = *sptr++;
					length -= value;
					dptr += value;
					zs += value;
					zp += value;
				} else {
					if ((z_min - (int)*zs++) < (int)*zp++) {
						*dptr = (T)((((*dptr) >> 1) & Mask) + ((TranslateTable[value] >> 1) & Mask));
						length--;
						dptr++;
					} else {
						length--;
						dptr++;
					}
				}
				zp = Blit_Wrap_Z_Buffer(zp);
			}
		}

	private:
		T const * TranslateTable;
		T Mask;
};


template<class T>
class RLEBlitTransLucent50ZReadWarp : public RLEBlitter
{
	public:
		RLEBlitTransLucent50ZReadWarp(T const * translator, T mask) : TranslateTable(translator), Mask(mask) {assert(TranslateTable != NULL);}

		virtual void Blit(void * dest, void const * source, int length, int leadskip = 0, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 0, int warp_offset = 0, void * zshape = NULL) const override
		{
			unsigned short* zp = (unsigned short*)(z_buff);
			T * dptr = (T *)dest;
			const signed char* zs = (const signed char*)(zshape);
			unsigned char const * sptr = (unsigned char const *)source;

			/*
			**	Skip any leading pixels as requested.
			*/
			if (leadskip > 0) {
				int transcount = Skip_Leading_Pixels(sptr, leadskip);
				dptr += transcount;
				length -= transcount;

				zp += transcount;
				zp = Blit_Wrap_Z_Buffer(zp);
			}

			/*
			**	Uncompress and store the pixel stream until the length has been
			**	exhausted.
			*/
			while (length > 0) {
				unsigned char value = *sptr++;
				if (value == '\0') {
					value = *sptr++;
					length -= value;
					dptr += value;
					zs += value;
					zp += value;
				} else {
					if ((z_min - (int)*zs++) < (int)*zp++) {
						*dptr = (T)((((dptr[warp_offset]) >> 1) & Mask) + ((TranslateTable[value] >> 1) & Mask));
						length--;
						dptr++;
					} else {
						length--;
						dptr++;
					}
				}
				zp = Blit_Wrap_Z_Buffer(zp);
			}
		}

	private:
		T const * TranslateTable;
		T Mask;
};


template<class T>
class RLEBlitTransLucent50ZReadWrite : public RLEBlitter
{
	public:
		RLEBlitTransLucent50ZReadWrite(T const * translator, T mask) : TranslateTable(translator), Mask(mask) {assert(TranslateTable != NULL);}

		virtual void Blit(void * dest, void const * source, int length, int leadskip = 0, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 0, int warp_offset = 0, void * zshape = NULL) const override
		{
			unsigned short* zp = (unsigned short*)(z_buff);
			T * dptr = (T *)dest;
			const signed char* zs = (const signed char*)(zshape);
			unsigned char const * sptr = (unsigned char const *)source;

			/*
			**	Skip any leading pixels as requested.
			*/
			if (leadskip > 0) {
				int transcount = Skip_Leading_Pixels(sptr, leadskip);
				dptr += transcount;
				length -= transcount;

				zp += transcount;
				zp = Blit_Wrap_Z_Buffer(zp);
			}

			/*
			**	Uncompress and store the pixel stream until the length has been
			**	exhausted.
			*/
			while (length > 0) {
				unsigned char value = *sptr++;
				if (value == '\0') {
					value = *sptr++;
					length -= value;
					dptr += value;
					zs += value;
					zp += value;
				} else {
					if ((z_min - (int)*zs) < (int)*zp) {
						*dptr = (T)((((*dptr) >> 1) & Mask) + ((TranslateTable[value] >> 1) & Mask));
						*zp++  = z_min - *zs++;
						length--;
						dptr++;
					} else {
						zs++;
						dptr++;
						zp++;
						length--;
					}
				}
				zp = Blit_Wrap_Z_Buffer(zp);
			}
		}

	private:
		T const * TranslateTable;
		T Mask;
};


/*
**	This blitter performs 25% translucency as it draws. This effect is less than spectacular,
**	but there are some uses for it. It only works with hicolor pixels.
*/
template<class T>
class RLEBlitTransLucent25 : public RLEBlitter {
	public:
		RLEBlitTransLucent25(T const * translator, T mask) : TranslateTable(translator), Mask(mask) {}

		virtual void Blit(void * dest, void const * source, int length, int leadskip = 0, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 0, int warp_offset = 0, void * zshape = NULL) const override
		{
			unsigned char const * sptr = (unsigned char const *)source;
			T * dptr = (T *)dest;

			/*
			**	Skip any leading pixels as requested.
			*/
			if (leadskip > 0) {
				int transcount = Skip_Leading_Pixels(sptr, leadskip);
				dptr += transcount;
				length -= transcount;
			}

			/*
			**	Uncompress and store the pixel stream until the length has been
			**	exhausted.
			*/
			while (length > 0) {
				unsigned char value = *sptr++;
				if (value == '\0') {
					value = *sptr++;
					length -= value;
					dptr += value;
				} else {
					T qsource = (T)(((TranslateTable[value] >> 2) & Mask));
					T qdest = (T)(((*dptr) >> 2) & Mask);
					*dptr++ = (T)(qdest + qsource + qsource + qsource);
					length -= 1;
				}
			}
		}

	private:
		T const * TranslateTable;
		T Mask;
};


template<class T>
class RLEBlitTransLucent25Alpha : public RLEBlitter
{
	public:
		RLEBlitTransLucent25Alpha(T const * translator, int intensity_levels, T mask) : TranslateTable(translator), Mask(mask), AlphaLightingRemap(NULL) {AlphaLightingRemap = AlphaLightingRemapInit.Init(intensity_levels);}
		virtual ~RLEBlitTransLucent25Alpha(void) override { AlphaLightingRemapInit.Deinit(AlphaLightingRemap); AlphaLightingRemap = NULL; }

		virtual void Blit(void * dest, void const * source, int length, int leadskip = 0, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 0, int warp_offset = 0, void * zshape = NULL) const override
		{
			unsigned char const * sptr = (unsigned char const *)source;
			T * dptr = (T *)dest;
			unsigned short* ap = (unsigned short*)(a_buff);

			const unsigned short* aLUT = AlphaLightingRemap->Get_Table(alpha_level);

			/*
			**	Skip any leading pixels as requested.
			*/
			if (leadskip > 0) {
				int transcount = Skip_Leading_Pixels(sptr, leadskip);
				dptr += transcount;
				length -= transcount;

				ap += transcount;
				ap = Blit_Wrap_A_Buffer(ap);
			}

			/*
			**	Uncompress and store the pixel stream until the length has been
			**	exhausted.
			*/
			while (length > 0) {
				unsigned char value = *sptr++;
				if (value == '\0') {
					value = *sptr++;
					length -= value;
					dptr += value;
					ap += value;
				} else {
					T qsource = (T)(((TranslateTable[value | aLUT[*ap]] >> 2) & Mask));
					T qdest = (T)(((*dptr) >> 2) & Mask);
					*dptr++ = (T)(qdest + qsource + qsource + qsource);
					length -= 1;
					ap++;
				}
				ap = Blit_Wrap_A_Buffer(ap);
			}
		}

	private:
		T const * TranslateTable;
		AlphaLightingRemapClass *AlphaLightingRemap;
		T Mask;
};


template<class T>
class RLEBlitTransLucent25AlphaZRead : public RLEBlitter
{
	public:
		RLEBlitTransLucent25AlphaZRead(T const * translator, int intensity_levels, T mask) : TranslateTable(translator), Mask(mask), AlphaLightingRemap(NULL) {AlphaLightingRemap = AlphaLightingRemapInit.Init(intensity_levels);}
		virtual ~RLEBlitTransLucent25AlphaZRead(void) override { AlphaLightingRemapInit.Deinit(AlphaLightingRemap); AlphaLightingRemap = NULL; }

		virtual void Blit(void * dest, void const * source, int length, int leadskip = 0, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 0, int warp_offset = 0, void * zshape = NULL) const override
		{
			unsigned char const * sptr = (unsigned char const *)source;
			T * dptr = (T *)dest;
			unsigned short* zp = (unsigned short*)(z_buff);
			unsigned short* ap = (unsigned short*)(a_buff);
			const signed char* zs = (const signed char*)(zshape);

			const unsigned short* aLUT = AlphaLightingRemap->Get_Table(alpha_level);

			/*
			**	Skip any leading pixels as requested.
			*/
			if (leadskip > 0) {
				int transcount = Skip_Leading_Pixels(sptr, leadskip);
				dptr += transcount;
				length -= transcount;

				zp += transcount;
				zp = Blit_Wrap_Z_Buffer(zp);

				ap += transcount;
				ap = Blit_Wrap_A_Buffer(ap);
			}

			/*
			**	Uncompress and store the pixel stream until the length has been
			**	exhausted.
			*/
			while (length > 0) {
				unsigned char value = *sptr++;
				if (value == '\0') {
					value = *sptr++;
					length -= value;
					dptr += value;
					zp += value;
					zs += value;
					ap += value;
				} else {
					if ((z_min - (int)*zs++) < (int)*zp++) {
						T qsource = (T)(((TranslateTable[value | aLUT[*ap]] >> 2) & Mask));
						T qdest = (T)(((*dptr) >> 2) & Mask);
						*dptr++ = (T)(qdest + qsource + qsource + qsource);
						length--;
					} else {
						dptr++;
						length--;
					}
					ap++;
				}
				zp = Blit_Wrap_Z_Buffer(zp);
				ap = Blit_Wrap_A_Buffer(ap);
			}
		}

	private:
		T const * TranslateTable;
		AlphaLightingRemapClass *AlphaLightingRemap;
		T Mask;
};


template<class T>
class RLEBlitTransLucent25AlphaZReadWarp : public RLEBlitter
{
	public:
		RLEBlitTransLucent25AlphaZReadWarp(T const * translator, int intensity_levels, T mask) : TranslateTable(translator), Mask(mask), AlphaLightingRemap(NULL) {AlphaLightingRemap = AlphaLightingRemapInit.Init(intensity_levels);}
		virtual ~RLEBlitTransLucent25AlphaZReadWarp(void) override { AlphaLightingRemapInit.Deinit(AlphaLightingRemap); AlphaLightingRemap = NULL; }

		virtual void Blit(void * dest, void const * source, int length, int leadskip = 0, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 0, int warp_offset = 0, void * zshape = NULL) const override
		{
			unsigned char const * sptr = (unsigned char const *)source;
			T * dptr = (T *)dest;
			unsigned short* zp = (unsigned short*)(z_buff);
			unsigned short* ap = (unsigned short*)(a_buff);
			const signed char* zs = (const signed char*)(zshape);

			const unsigned short* aLUT = AlphaLightingRemap->Get_Table(alpha_level);

			/*
			**	Skip any leading pixels as requested.
			*/
			if (leadskip > 0) {
				int transcount = Skip_Leading_Pixels(sptr, leadskip);
				dptr += transcount;
				length -= transcount;

				zp += transcount;
				zp = Blit_Wrap_Z_Buffer(zp);

				ap += transcount;
				ap = Blit_Wrap_A_Buffer(ap);
			}

			/*
			**	Uncompress and store the pixel stream until the length has been
			**	exhausted.
			*/
			while (length > 0) {
				unsigned char value = *sptr++;
				if (value == '\0') {
					value = *sptr++;
					length -= value;
					zs += value;
					zp += value;
					dptr += value;
					ap += value;
				} else {
					if ((z_min - (int)*zs++) < (int)*zp++) {
						T qsource = (T)(((TranslateTable[value | aLUT[*ap]] >> 2) & Mask));
						T qdest = (T)(((dptr[warp_offset]) >> 2) & Mask);
						*dptr++ = (T)(qdest + qsource + qsource + qsource);
						length--;
					} else {
						dptr++;
						length--;
					}
					ap++;
				}
				zp = Blit_Wrap_Z_Buffer(zp);
				ap = Blit_Wrap_A_Buffer(ap);
			}
		}

	private:
		T const * TranslateTable;
		AlphaLightingRemapClass *AlphaLightingRemap;
		T Mask;
};


template<class T>
class RLEBlitTransLucent25AlphaZReadWrite : public RLEBlitter
{
	public:
		RLEBlitTransLucent25AlphaZReadWrite(T const * translator, int intensity_levels, T mask) : TranslateTable(translator), Mask(mask), AlphaLightingRemap(NULL) {AlphaLightingRemap = AlphaLightingRemapInit.Init(intensity_levels);}
		virtual ~RLEBlitTransLucent25AlphaZReadWrite(void) override { AlphaLightingRemapInit.Deinit(AlphaLightingRemap); AlphaLightingRemap = NULL; }

		virtual void Blit(void * dest, void const * source, int length, int leadskip = 0, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 0, int warp_offset = 0, void * zshape = NULL) const override
		{
			unsigned char const * sptr = (unsigned char const *)source;
			T * dptr = (T *)dest;
			unsigned short* zp = (unsigned short*)(z_buff);
			unsigned short* ap = (unsigned short*)(a_buff);
			const signed char* zs = (const signed char*)(zshape);

			const unsigned short* aLUT = AlphaLightingRemap->Get_Table(alpha_level);

			/*
			**	Skip any leading pixels as requested.
			*/
			if (leadskip > 0) {
				int transcount = Skip_Leading_Pixels(sptr, leadskip);
				dptr += transcount;
				length -= transcount;

				zp += transcount;
				zp = Blit_Wrap_Z_Buffer(zp);

				ap += transcount;
				ap = Blit_Wrap_A_Buffer(ap);
			}

			/*
			**	Uncompress and store the pixel stream until the length has been
			**	exhausted.
			*/
			while (length > 0) {
				unsigned char value = *sptr++;

				if (value == '\0') {
					value = *sptr++;
					length -= value;
					dptr += value;
					zp += value;
					zs += value;
					ap += value;
				} else {
					if ((z_min - (int)*zs) < (int)*zp) {
						T qsource = (T)(((TranslateTable[value | aLUT[*ap]] >> 2) & Mask));
						T qdest = (T)(((*dptr) >> 2) & Mask);
						*dptr = (T)(qdest + qsource + qsource + qsource);
						dptr++;
						*zp++  = z_min - *zs++;
						length--;
					} else {
						zs++;
						dptr++;
						zp++;
						length--;
					}
					ap++;
				}
				zp = Blit_Wrap_Z_Buffer(zp);
				ap = Blit_Wrap_A_Buffer(ap);
			}
		}

	private:
		T const * TranslateTable;
		AlphaLightingRemapClass *AlphaLightingRemap;
		T Mask;
};


template<class T>
class RLEBlitTransLucent25ZRead : public RLEBlitter
{
	public:
		RLEBlitTransLucent25ZRead(T const * translator, T mask) : TranslateTable(translator), Mask(mask) {assert(TranslateTable != NULL);}

		virtual void Blit(void * dest, void const * source, int length, int leadskip = 0, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 0, int warp_offset = 0, void * zshape = NULL) const override
		{
			unsigned short* zp = (unsigned short*)(z_buff);
			T * dptr = (T *)dest;
			const signed char* zs = (const signed char*)(zshape);
			unsigned char const * sptr = (unsigned char const *)source;

			/*
			**	Skip any leading pixels as requested.
			*/
			if (leadskip > 0) {
				int transcount = Skip_Leading_Pixels(sptr, leadskip);
				dptr += transcount;
				length -= transcount;

				zp += transcount;
				zp = Blit_Wrap_Z_Buffer(zp);
			}

			/*
			**	Uncompress and store the pixel stream until the length has been
			**	exhausted.
			*/
			while (length > 0) {
				unsigned char value = *sptr++;
				if (value == '\0') {
					value = *sptr++;
					length -= value;
					dptr += value;
					zs += value;
					zp += value;
				} else {
					if ((z_min - (int)*zs++) < (int)*zp++) {
						T qsource = (T)(((TranslateTable[value] >> 2) & Mask));
						T qdest = (T)(((*dptr) >> 2) & Mask);
						*dptr++ = (T)(qdest + qsource + qsource + qsource);
						length--;
					} else {
						dptr++;
						length--;
					}
				}
				zp = Blit_Wrap_Z_Buffer(zp);
			}
		}

	private:
		T const * TranslateTable;
		T Mask;
};


template<class T>
class RLEBlitTransLucent25ZReadWarp : public RLEBlitter
{
	public:
		RLEBlitTransLucent25ZReadWarp(T const * translator, T mask) : TranslateTable(translator), Mask(mask) {assert(TranslateTable != NULL);}

		virtual void Blit(void * dest, void const * source, int length, int leadskip = 0, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 0, int warp_offset = 0, void * zshape = NULL) const override
		{
			unsigned short* zp = (unsigned short*)(z_buff);
			unsigned char const * sptr = (unsigned char const *)source;
			T * dptr = (T *)dest;
			const signed char* zs = (const signed char*)(zshape);

			/*
			**	Skip any leading pixels as requested.
			*/
			if (leadskip > 0) {
				int transcount = Skip_Leading_Pixels(sptr, leadskip);
				dptr += transcount;
				length -= transcount;

				zp += transcount;
				zp = Blit_Wrap_Z_Buffer(zp);
			}

			/*
			**	Uncompress and store the pixel stream until the length has been
			**	exhausted.
			*/
			while (length > 0) {
				unsigned char value = *sptr++;
				if (value == '\0') {
					value = *sptr++;
					length -= value;
					dptr += value;
					zs += value;
					zp += value;
				} else {
					if ((z_min - (int)*zs++) < (int)*zp++) {
						T qsource = (T)(((TranslateTable[value] >> 2) & Mask));
						T qdest = (T)(((dptr[warp_offset]) >> 2) & Mask);
						*dptr++ = (T)(qdest + qsource + qsource + qsource);
						length--;
					} else {
						dptr++;
						length--;
					}
				}
				zp = Blit_Wrap_Z_Buffer(zp);
			}
		}

	private:
		T const * TranslateTable;
		T Mask;
};


template<class T>
class RLEBlitTransLucent25ZReadWrite : public RLEBlitter
{
	public:
		RLEBlitTransLucent25ZReadWrite(T const * translator, T mask) : TranslateTable(translator), Mask(mask) {assert(TranslateTable != NULL);}

		virtual void Blit(void * dest, void const * source, int length, int leadskip = 0, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 0, int warp_offset = 0, void * zshape = NULL) const override
		{
			unsigned short* zp = (unsigned short*)(z_buff);
			unsigned char const * sptr = (unsigned char const *)source;
			T * dptr = (T *)dest;
			const signed char* zs = (const signed char*)(zshape);

			/*
			**	Skip any leading pixels as requested.
			*/
			if (leadskip > 0) {
				int transcount = Skip_Leading_Pixels(sptr, leadskip);
				dptr += transcount;
				length -= transcount;

				zp += transcount;
				zp = Blit_Wrap_Z_Buffer(zp);
			}

			/*
			**	Uncompress and store the pixel stream until the length has been
			**	exhausted.
			*/
			while (length > 0) {
				unsigned char value = *sptr++;
				if (value == '\0') {
					value = *sptr++;
					length -= value;
					dptr += value;
					/// Transparent run doesn't step zp/zs, desyncing depth from dptr.
				} else {
					if ((z_min - (int)*zs) < (int)*zp) {
						T qsource = (T)(((TranslateTable[value] >> 2) & Mask));
						T qdest = (T)(((*dptr) >> 2) & Mask);
						*dptr = (T)(qdest + qsource + qsource + qsource);
						dptr++;
						*zp++  = z_min - *zs++;
						length--;
					} else {
						zs++;
						dptr++;
						zp++;
						length--;
					}
				}
				zp = Blit_Wrap_Z_Buffer(zp);
			}
		}

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
class RLEBlitTransLucent75 : public RLEBlitter {
	public:
		RLEBlitTransLucent75(T const * translator, T mask) : TranslateTable(translator), Mask(mask) {}

		virtual void Blit(void * dest, void const * source, int length, int leadskip = 0, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 0, int warp_offset = 0, void * zshape = NULL) const override
		{
			unsigned char const * sptr = (unsigned char const *)source;
			T * dptr = (T *)dest;

			/*
			**	Skip any leading pixels as requested.
			*/
			if (leadskip > 0) {
				int transcount = Skip_Leading_Pixels(sptr, leadskip);
				dptr += transcount;
				length -= transcount;
			}

			/*
			**	Uncompress and store the pixel stream until the length has been
			**	exhausted.
			*/
			while (length > 0) {
				unsigned char value = *sptr++;
				if (value == '\0') {
					value = *sptr++;
					length -= value;
					dptr += value;
				} else {
					T qsource = (T)(((TranslateTable[value] >> 2) & Mask));
					T qdest = (T)(((*dptr) >> 2) & Mask);
					*dptr++ = (T)(qdest + qdest + qdest + qsource);
					length -= 1;
				}
			}
		}

	private:
		T const * TranslateTable;
		T Mask;
};


template<class T>
class RLEBlitTransLucent75Alpha : public RLEBlitter
{
	public:
		RLEBlitTransLucent75Alpha(T const * translator, int intensity_levels, T mask) : TranslateTable(translator), Mask(mask), AlphaLightingRemap(NULL) {AlphaLightingRemap = AlphaLightingRemapInit.Init(intensity_levels);}
		virtual ~RLEBlitTransLucent75Alpha(void) override { AlphaLightingRemapInit.Deinit(AlphaLightingRemap); AlphaLightingRemap = NULL; }

		virtual void Blit(void * dest, void const * source, int length, int leadskip = 0, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 0, int warp_offset = 0, void * zshape = NULL) const override
		{
			unsigned char const * sptr = (unsigned char const *)source;
			T * dptr = (T *)dest;
			unsigned short* ap = (unsigned short*)(a_buff);

			const unsigned short* aLUT = AlphaLightingRemap->Get_Table(alpha_level);

			/*
			**	Skip any leading pixels as requested.
			*/
			if (leadskip > 0) {
				int transcount = Skip_Leading_Pixels(sptr, leadskip);
				dptr += transcount;
				length -= transcount;

				ap += transcount;
				ap = Blit_Wrap_A_Buffer(ap);
			}

			/*
			**	Uncompress and store the pixel stream until the length has been
			**	exhausted.
			*/
			while (length > 0) {
				unsigned char value = *sptr++;
				if (value == '\0') {
					value = *sptr++;
					length -= value;
					dptr += value;
					ap += value;
				} else {
					T qsource = (T)(((TranslateTable[value | aLUT[*ap]] >> 2) & Mask));
					T qdest = (T)(((*dptr) >> 2) & Mask);
					*dptr++ = (T)(qdest + qdest + qdest + qsource);
					length -= 1;
					ap++;
				}
				ap = Blit_Wrap_A_Buffer(ap);
			}
		}

	private:
		T const * TranslateTable;
		AlphaLightingRemapClass *AlphaLightingRemap;
		T Mask;
};


template<class T>
class RLEBlitTransLucent75AlphaZRead : public RLEBlitter
{
	public:
		RLEBlitTransLucent75AlphaZRead(T const * translator, int intensity_levels, T mask) : TranslateTable(translator), Mask(mask), AlphaLightingRemap(NULL) {AlphaLightingRemap = AlphaLightingRemapInit.Init(intensity_levels);}
		virtual ~RLEBlitTransLucent75AlphaZRead(void) override { AlphaLightingRemapInit.Deinit(AlphaLightingRemap); AlphaLightingRemap = NULL; }

		virtual void Blit(void * dest, void const * source, int length, int leadskip = 0, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 0, int warp_offset = 0, void * zshape = NULL) const override
		{
			unsigned char const * sptr = (unsigned char const *)source;
			T * dptr = (T *)dest;
			unsigned short* zp = (unsigned short*)(z_buff);
			unsigned short* ap = (unsigned short*)(a_buff);
			const signed char* zs = (const signed char*)(zshape);

			const unsigned short* aLUT = AlphaLightingRemap->Get_Table(alpha_level);

			/*
			**	Skip any leading pixels as requested.
			*/
			if (leadskip > 0) {
				int transcount = Skip_Leading_Pixels(sptr, leadskip);
				dptr += transcount;
				length -= transcount;

				zp += transcount;
				zp = Blit_Wrap_Z_Buffer(zp);

				ap += transcount;
				ap = Blit_Wrap_A_Buffer(ap);
			}

			/*
			**	Uncompress and store the pixel stream until the length has been
			**	exhausted.
			*/
			while (length > 0) {
				unsigned char value = *sptr++;
				if (value == '\0') {
					value = *sptr++;
					length -= value;
					dptr += value;
					zp += value;
					zs += value;
					ap += value;
				} else {
					if ((z_min - (int)*zs++) < (int)*zp++) {
						T qsource = (T)(((TranslateTable[value | aLUT[*ap]] >> 2) & Mask));
						T qdest = (T)(((*dptr) >> 2) & Mask);
						*dptr++ = (T)(qdest + qdest + qdest + qsource);
						length--;
					} else {
						dptr++;
						length--;
					}
					ap++;
				}
				zp = Blit_Wrap_Z_Buffer(zp);
				ap = Blit_Wrap_A_Buffer(ap);
			}
		}

	private:
		T const * TranslateTable;
		AlphaLightingRemapClass *AlphaLightingRemap;
		T Mask;
};


template<class T>
class RLEBlitTransLucent75AlphaZReadWarp : public RLEBlitter
{
	public:
		RLEBlitTransLucent75AlphaZReadWarp(T const * translator, int intensity_levels, T mask) : TranslateTable(translator), Mask(mask), AlphaLightingRemap(NULL) {AlphaLightingRemap = AlphaLightingRemapInit.Init(intensity_levels);}
		virtual ~RLEBlitTransLucent75AlphaZReadWarp(void) override { AlphaLightingRemapInit.Deinit(AlphaLightingRemap); AlphaLightingRemap = NULL; }

		virtual void Blit(void * dest, void const * source, int length, int leadskip = 0, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 0, int warp_offset = 0, void * zshape = NULL) const override
		{
			unsigned char const * sptr = (unsigned char const *)source;
			T * dptr = (T *)dest;
			unsigned short* zp = (unsigned short*)(z_buff);
			unsigned short* ap = (unsigned short*)(a_buff);
			const signed char* zs = (const signed char*)(zshape);

			const unsigned short* aLUT = AlphaLightingRemap->Get_Table(alpha_level);

			/*
			**	Skip any leading pixels as requested.
			*/
			if (leadskip > 0) {
				int transcount = Skip_Leading_Pixels(sptr, leadskip);
				dptr += transcount;
				length -= transcount;

				zp += transcount;
				zp = Blit_Wrap_Z_Buffer(zp);

				ap += transcount;
				ap = Blit_Wrap_A_Buffer(ap);
			}

			/*
			**	Uncompress and store the pixel stream until the length has been
			**	exhausted.
			*/
			while (length > 0) {
				unsigned char value = *sptr++;
				if (value == '\0') {
					value = *sptr++;
					length -= value;
					zs += value;
					zp += value;
					dptr += value;
					ap += value;
				} else {
					if ((z_min - (int)*zs++) < (int)*zp++) {
						T qsource = (T)(((TranslateTable[value | aLUT[*ap]] >> 2) & Mask));
						T qdest = (T)(((dptr[warp_offset]) >> 2) & Mask);
						*dptr++ = (T)(qdest + qdest + qdest + qsource);
						length--;
					} else {
						dptr++;
						length--;
					}
					ap++;
				}
				zp = Blit_Wrap_Z_Buffer(zp);
				ap = Blit_Wrap_A_Buffer(ap);
			}
		}

	private:
		T const * TranslateTable;
		AlphaLightingRemapClass *AlphaLightingRemap;
		T Mask;
};


template<class T>
class RLEBlitTransLucent75AlphaZReadWrite : public RLEBlitter
{
	public:
		RLEBlitTransLucent75AlphaZReadWrite(T const * translator, int intensity_levels, T mask) : TranslateTable(translator), Mask(mask), AlphaLightingRemap(NULL) {AlphaLightingRemap = AlphaLightingRemapInit.Init(intensity_levels);}
		virtual ~RLEBlitTransLucent75AlphaZReadWrite(void) override { AlphaLightingRemapInit.Deinit(AlphaLightingRemap); AlphaLightingRemap = NULL; }

		virtual void Blit(void * dest, void const * source, int length, int leadskip = 0, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 0, int warp_offset = 0, void * zshape = NULL) const override
		{
			T * dptr = (T *)dest;
			unsigned char const * sptr = (unsigned char const *)source;
			const signed char* zs = (const signed char*)(zshape);
			unsigned short* ap = (unsigned short*)(a_buff);
			unsigned short* zp = (unsigned short*)(z_buff);

			const unsigned short* aLUT = AlphaLightingRemap->Get_Table(alpha_level);

			/*
			**	Skip any leading pixels as requested.
			*/
			if (leadskip > 0) {
				int transcount = Skip_Leading_Pixels(sptr, leadskip);
				dptr += transcount;
				length -= transcount;

				zp += transcount;
				zp = Blit_Wrap_Z_Buffer(zp);

				ap += transcount;
				ap = Blit_Wrap_A_Buffer(ap);
			}

			/*
			**	Uncompress and store the pixel stream until the length has been
			**	exhausted.
			*/
			while (length > 0) {
				unsigned char value = *sptr++;

				if (value == '\0') {
					value = *sptr++;
					length -= value;
					dptr += value;
					zp += value;
					zs += value;
					ap += value;
				} else {
					if ((z_min - (int)*zs) < (int)*zp) {
						T qsource = (T)(((TranslateTable[value | aLUT[*ap]] >> 2) & Mask));
						T qdest = (T)(((*dptr) >> 2) & Mask);
						*dptr = (T)(qdest + qdest + qdest + qsource);
						dptr++;
						*zp = z_min - *zs;
						zs++;
						zp++;
						length--;
					} else {
						zs++;
						dptr++;
						zp++;
						length--;
					}
					ap++;
				}
				zp = Blit_Wrap_Z_Buffer(zp);
				ap = Blit_Wrap_A_Buffer(ap);
			}
		}

	private:
		T const * TranslateTable;
		AlphaLightingRemapClass *AlphaLightingRemap;
		T Mask;
};


template<class T>
class RLEBlitTransLucent75ZRead : public RLEBlitter
{
	public:
		RLEBlitTransLucent75ZRead(T const * translator, T mask) : TranslateTable(translator), Mask(mask) {assert(TranslateTable != NULL);}

		virtual void Blit(void * dest, void const * source, int length, int leadskip = 0, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 0, int warp_offset = 0, void * zshape = NULL) const override
		{
			unsigned short* zp = (unsigned short*)(z_buff);
			T * dptr = (T *)dest;
			const signed char* zs = (const signed char*)(zshape);
			unsigned char const * sptr = (unsigned char const *)source;

			/*
			**	Skip any leading pixels as requested.
			*/
			if (leadskip > 0) {
				int transcount = Skip_Leading_Pixels(sptr, leadskip);
				dptr += transcount;
				length -= transcount;

				zp += transcount;
				zp = Blit_Wrap_Z_Buffer(zp);
			}

			/*
			**	Uncompress and store the pixel stream until the length has been
			**	exhausted.
			*/
			while (length > 0) {
				unsigned char value = *sptr++;
				if (value == '\0') {
					value = *sptr++;
					length -= value;
					dptr += value;
					zs += value;
					zp += value;
				} else {
					if ((z_min - (int)*zs++) < (int)*zp++) {
						T qsource = (T)(((TranslateTable[value] >> 2) & Mask));
						T qdest = (T)(((*dptr) >> 2) & Mask);
						*dptr++ = (T)(qdest + qdest + qdest + qsource);
						length--;
					} else {
						dptr++;
						length--;
					}
				}
				zp = Blit_Wrap_Z_Buffer(zp);
			}
		}

	private:
		T const * TranslateTable;
		T Mask;
};


template<class T>
class RLEBlitTransLucent75ZReadWarp : public RLEBlitter
{
	public:
		RLEBlitTransLucent75ZReadWarp(T const * translator, T mask) : TranslateTable(translator), Mask(mask) {assert(TranslateTable != NULL);}

		virtual void Blit(void * dest, void const * source, int length, int leadskip = 0, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 0, int warp_offset = 0, void * zshape = NULL) const override
		{
			unsigned short* zp = (unsigned short*)(z_buff);
			unsigned char const * sptr = (unsigned char const *)source;
			T * dptr = (T *)dest;
			const signed char* zs = (const signed char*)(zshape);

			/*
			**	Skip any leading pixels as requested.
			*/
			if (leadskip > 0) {
				int transcount = Skip_Leading_Pixels(sptr, leadskip);
				dptr += transcount;
				length -= transcount;

				zp += transcount;
				zp = Blit_Wrap_Z_Buffer(zp);
			}

			/*
			**	Uncompress and store the pixel stream until the length has been
			**	exhausted.
			*/
			while (length > 0) {
				unsigned char value = *sptr++;
				if (value == '\0') {
					value = *sptr++;
					length -= value;
					dptr += value;
					zs += value;
					zp += value;
				} else {
					if ((z_min - (int)*zs++) < (int)*zp++) {
						T qsource = (T)(((TranslateTable[value] >> 2) & Mask));
						T qdest = (T)(((dptr[warp_offset]) >> 2) & Mask);
						*dptr++ = (T)(qdest + qdest + qdest + qsource);
						length--;
					} else {
						dptr++;
						length--;
					}
				}
				zp = Blit_Wrap_Z_Buffer(zp);
			}
		}

	private:
		T const * TranslateTable;
		T Mask;
};


template<class T>
class RLEBlitTransLucent75ZReadWrite : public RLEBlitter
{
	public:
		RLEBlitTransLucent75ZReadWrite(T const * translator, T mask) : TranslateTable(translator), Mask(mask) {assert(TranslateTable != NULL);}

		virtual void Blit(void * dest, void const * source, int length, int leadskip = 0, int z_min = 0, void * z_buff = NULL, void * a_buff = NULL, int alpha_level = 0, int warp_offset = 0, void * zshape = NULL) const override
		{
			unsigned short* zp = (unsigned short*)(z_buff);
			unsigned char const * sptr = (unsigned char const *)source;
			T * dptr = (T *)dest;
			const signed char* zs = (const signed char*)(zshape);

			/*
			**	Skip any leading pixels as requested.
			*/
			if (leadskip > 0) {
				int transcount = Skip_Leading_Pixels(sptr, leadskip);
				dptr += transcount;
				length -= transcount;

				zp += transcount;
				zp = Blit_Wrap_Z_Buffer(zp);
			}

			/*
			**	Uncompress and store the pixel stream until the length has been
			**	exhausted.
			*/
			while (length > 0) {
				unsigned char value = *sptr++;
				if (value == '\0') {
					value = *sptr++;
					length -= value;
					dptr += value;
					/// Transparent run doesn't step zp/zs, desyncing depth from dptr.
#if 0
					zp += value;
					zs += value;
#endif
				} else {
					if ((z_min - (int)*zs) < (int)*zp) {
						T qsource = (T)(((TranslateTable[value] >> 2) & Mask));
						T qdest = (T)(((*dptr) >> 2) & Mask);
						*dptr = (T)(qdest + qdest + qdest + qsource);
						dptr++;
						*zp = z_min - *zs;
						zs++;
						zp++;
						length--;
					} else {
						zs++;
						dptr++;
						zp++;
						length--;
					}
				}
				zp = Blit_Wrap_Z_Buffer(zp);
			}
		}

	private:
		T const * TranslateTable;
		T Mask;
};

