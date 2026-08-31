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

#include <string.h>

typedef signed char int8_t;
typedef signed short int16_t;
typedef signed int int32_t;

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

void __cdecl UnVQ2_C1_4x4(unsigned char * codebook, unsigned char * pointers, unsigned char * buffer, unsigned long blocksperrow, unsigned long numrows, unsigned long bufwidth)
{
	bufwidth *= 2u;
	uint32_t block_row_stride = bufwidth * 4u;

	uint8_t * data_end = (uint8_t *)buffer + ((uint32_t)numrows * bufwidth * 4u);
	uint8_t * dst = (uint8_t *)buffer;

	uint8_t * row_base = (uint8_t *)buffer;
	uint8_t * row_end = row_base + ((uint32_t)blocksperrow * 8u);

	uint16_t * src = (uint16_t *)pointers;

	while (dst < data_end) {

		uint16_t command = *src;
		int tag = command & 0xF000;
		uint32_t count = command;
		count &= 0xFFFF0FFFu;
		src += 1;

		switch (tag) {

		case 0x0000u: {
			uint32_t n = 2u * count; /* dwords per 4x4 block run per row */
			/* fill count blocks with a single 16-bit color */
			uint32_t w = *src;
			uint32_t v = (w << 16) | w;

			src += 1;

			{
				unsigned int * p = (unsigned int *)(dst);
				unsigned int left = n;
				while (left > 0) {
					*p++ = v;
					left--;
				}
			}
			{
				unsigned int * p = (unsigned int *)(dst + bufwidth);
				unsigned int left = n;
				while (left > 0) {
					*p++ = v;
					left--;
				}
			}
			{
				unsigned int * p = (unsigned int *)(dst + 2u * bufwidth);
				unsigned int left = n;
				while (left > 0) {
					*p++ = v;
					left--;
				}
			}
			{
				unsigned int * p = (unsigned int *)(dst + 3u * bufwidth);
				unsigned int left = n;
				while (left > 0) {
					*p++ = v;
					left--;
				}
			}

			dst += 8u * count; /* advance horizontally by count blocks */
		} break;

		case 0x1000u: {
			/* skip count blocks */
			dst += 8u * count;
		} break;

		case 0x2000u: {
			/* cb = codebook + 32 * *(uint16_t*)pointers; pointers += 2; */
			const uint8_t * cb = codebook + ((uint32_t)*src << 5);
			src += 1;

			for (uint32_t r = 0; r < 4; r++) {

				/* lo = assembled from two 16-bit loads */
				uint32_t lo = ((uint32_t)*(const uint16_t *)(cb + 2) << 16) | *(const uint16_t *)(cb + 0);
				cb += 4;

				/* write left dword of each block across the row (stride 8) */
				uint32_t i;
				{
					uint8_t * p = dst;

					for (i = 0; i < count; i++) {
						*(uint32_t *)p = lo;
						p += 8;
					}
				}

				/* hi = assembled from two 16-bit loads */
				uint32_t hi = ((uint32_t)*(const uint16_t *)(cb + 2) << 16) | *(const uint16_t *)(cb + 0);
				cb += 4;

				/* write right dword (dst + 4) */
				{
					uint8_t * p = dst + 4;

					for (i = 0; i < count; i++) {
						*(uint32_t *)p = hi;
						p += 8;
					}
				}

				dst += bufwidth;
			}

			/* final pointer correction: dst += count*8 - bufwidth*4 */
			dst += (count * 8u) - block_row_stride;
		} break;

		case 0x3000u: {
			/* 4x4 from codebook, but each 16-bit entry can be "masked" by high bit */
			uint16_t cb_index = *src;
			const uint8_t * cb = codebook + 32u * (uint32_t)cb_index;
			uint32_t r;

			src += 1;

			for (r = 0; r < 4; ++r) {
				uint8_t * rowp = dst;
				uint32_t c;

				for (c = 0; c < 4; ++c) {
					uint16_t px = *(const uint16_t *)cb;

					if ((px & 0x8000u) == 0) {
						uint8_t * p = rowp;
						uint32_t i;

						for (i = 0; i < count; ++i) {
							*(uint16_t *)p = px;
							p += 8;
						}
					}

					rowp += 2;
					cb += 2;
				}

				dst += bufwidth;
			}

			dst += 8u * count - block_row_stride;
		} break;


		case 0x5000u: {
			/* 0x5000: draw `count` solid 4x4 blocks,
			   each block uses the next 16-bit color from the stream. */
			uint32_t i;

			/* the row step is deliberately rounded down to a multiple of 4 */
			uint32_t step = 4u * (bufwidth >> 2);
			for (i = 0; i < count; i++) {

				uint32_t j;
				uint32_t v;
				uint32_t r = 4;
				uint8_t * p;

				/* load 16-bit color and replicate to 32-bit */
				{
					uint32_t w = *src;
					v = (w << 16) | w;
					src += 1;
				}

				p = dst;

				/* write 4 rows of the 4x4 solid block */
				for (j = 0; j < r; j++) {
					*(uint32_t *)(p + 0) = v;
					*(uint32_t *)(p + 4) = v;
					p += step;
				}

				dst += 8;
			}
		} break;


		case 0x6000u: {

			/* 0x6000: draw `count` consecutive 4x4 blocks, each block has its own
			   codebook index in the pointer stream (no horizontal replication of one index). */

			uint32_t i;

			/* the row step is deliberately rounded down to a multiple of 4 */
			uint32_t step = 4u * (bufwidth >> 2);
			for (i = 0; i < count; i++) {

				/* cb = codebook + 32 * *(uint16_t*)pointers; pointers += 2; */
				const uint8_t * cb = codebook + ((uint32_t)*src << 5);
				src += 1;

				uint8_t * p = dst;
				for (uint32_t r = 0; r < 4; ++r) {
					uint32_t lo;
					uint32_t hi;

					/* lo = assembled from two 16-bit loads */
					lo = ((uint32_t)*(const uint16_t *)(cb + 2) << 16) | *(const uint16_t *)(cb + 0);
					cb += 4;
					*(uint32_t *)(p + 0) = lo;

					/* hi = assembled from two 16-bit loads */
					hi = ((uint32_t)*(const uint16_t *)(cb + 2) << 16) | *(const uint16_t *)(cb + 0);
					cb += 4;
					*(uint32_t *)(p + 4) = hi;

					p += step;
				}

				dst += 8;
			}
		} break;

		case 0x7000u: {

			uint32_t i;

			for (i = 0; i < count; ++i) {
				uint16_t cb_index = *src;
				const uint8_t * cb = codebook + 32u * (uint32_t)cb_index;
				src += 1;

				uint8_t * p = dst;
				for (uint32_t r = 0; r < 4; ++r) {
					uint8_t * q = p;

					for (uint32_t c = 0; c < 4; ++c) {
						uint16_t px = *(const uint16_t *)cb;

						if ((px & 0x8000u) == 0) {
							*(uint16_t *)q = px;
						}

						q += 2;
						cb += 2;
					}

					p += 2u * (bufwidth >> 1);
				}

				dst += 8;
			}
		} break;
		}


		/* wrap to next 4-row macro-row after blocksperrow blocks */
		if (dst == row_end) {
			row_base += block_row_stride;
			dst = row_base;
			row_end = row_base + (8u * (uint32_t)blocksperrow);
		}
	}
}


void __cdecl UnVQ1_C4_4x4(unsigned char * codebook, unsigned char * pointers, unsigned char * buffer, unsigned long blocksperrow, unsigned long numrows, unsigned long bufwidth)
{
	bufwidth *= 2u;
	uint32_t block_row_stride = bufwidth * 4u;

	uint8_t * end = (uint8_t *)buffer + ((uint32_t)numrows * bufwidth * 4u);
	uint8_t * dst = (uint8_t *)buffer;

	uint8_t * row_base = (uint8_t *)buffer;
	uint8_t * row_end = row_base + ((uint32_t)blocksperrow * 8u);

	uint16_t * src = (uint16_t *)pointers;

	while (dst < end) {

		uint32_t command = (*src & 0xE000);
		uint16_t cb_index = (*src & 0x1FFF);

		src += 1;

		switch (command) {

		/* ------------------------------------------------------------ */
		/* 0x0000 - unmasked 4x4 from codebook                          */
		/* ------------------------------------------------------------ */
		case 0x0000u: {
			const uint8_t * cb = (const uint8_t *)codebook + (cb_index << 5);

			for (uint32_t r = 0; r < 4; r++) {
				uint32_t lo;
				uint32_t hi;

				lo = ((uint32_t)*(const uint16_t *)(cb + 2) << 16) | *(const uint16_t *)(cb + 0);
				cb += 4;
				*(uint32_t *)(dst + 0) = lo;

				hi = ((uint32_t)*(const uint16_t *)(cb + 2) << 16) | *(const uint16_t *)(cb + 0);
				cb += 4;
				*(uint32_t *)(dst + 4) = hi;

				dst += bufwidth;
			}

			dst += 8u - block_row_stride;
		} break;

		/* ------------------------------------------------------------ */
		/* 0x2000 - masked 4x4 from codebook (0x8000 = transparent)     */
		/* ------------------------------------------------------------ */
		case 0x2000u: {
			const uint8_t * cb = (const uint8_t *)codebook + (cb_index << 5);

			for (uint32_t r = 0; r < 4; r++) {
				uint8_t * p = dst;

				for (uint32_t c = 0; c < 4; c++) {
					uint16_t v = *(const uint16_t *)cb;

					if (v != 0x8000u) {
						*(uint16_t *)p = v;
					}

					p += 2;
					cb += 2;
				}

				dst += bufwidth;
			}

			dst += 8u - block_row_stride;
		} break;

		/* ------------------------------------------------------------ */
		/* 0x4000 - horizontal skip of one 4x4 block                    */
		/* ------------------------------------------------------------ */
		case 0x4000u: {
			dst += 8;
		} break;

		/* ------------------------------------------------------------ */
		/* default - fall-through                                       */
		/* ------------------------------------------------------------ */
		default:
			break;
		}

		/* macro-row wrap */
		if (dst == row_end) {
			row_base += block_row_stride;
			dst = row_base;
			row_end = row_base + ((uint32_t)blocksperrow * 8u);
		}
	}
}


void __cdecl UnVQ2_C4_4x4(unsigned char * codebook, unsigned char * pointers, unsigned char * buffer, unsigned long blocksperrow, unsigned long numrows, unsigned long bufwidth)
{
	bufwidth *= 2u;
	uint32_t block_row_stride = bufwidth * 4u;

	uint8_t * end = (uint8_t *)buffer + ((uint32_t)numrows * bufwidth * 4u);
	uint8_t * dst = (uint8_t *)buffer;

	uint8_t * row_base = (uint8_t *)buffer;
	uint8_t * row_end = row_base + ((uint32_t)blocksperrow * 8u);

	uint8_t * src = pointers;

	uint32_t height;
	uint32_t code;

	while (dst < end) {

		uint32_t command = (*(uint16_t *)src & 0xE000);
		uint16_t cb_index = (*(uint16_t *)src & 0x1FFF);
		src += 2;

		switch (command) {

		/* ------------------------------------------------------------ */
		/* 0x4000 - patterned column replicate                          */
		/* ------------------------------------------------------------ */
		case 0x4000u: {
			const uint8_t * cb = codebook + (*(src - 2) << 5);
			uint8_t * rowp = dst;
			uint32_t repeat = ((cb_index >> 7) & 0x3Eu) + 2u;
			uint32_t step = 4u * (bufwidth >> 2);

			for (uint32_t r = 0; r < 4; ++r) {
				uint32_t lo = ((uint32_t)*(uint16_t *)(cb + 2) << 16) | *(uint16_t *)(cb + 0);
				cb += 4;
				*(uint32_t *)(rowp + 0) = lo;

				uint32_t hi = ((uint32_t)*(uint16_t *)(cb + 2) << 16) | *(uint16_t *)(cb + 0);
				cb += 4;
				*(uint32_t *)(rowp + 4) = hi;

				rowp += step;
			}

			uint8_t * cursor = src;
			dst += 8;

			for (uint32_t i = 0; i < repeat; i++) {
				const uint8_t * cb2 = codebook + (*cursor << 5);
				cursor += 1;

				uint8_t * p = dst;
				for (uint32_t r = 0; r < 4; ++r) {
					uint32_t lo = ((uint32_t)*(uint16_t *)(cb2 + 2) << 16) | *(uint16_t *)(cb2 + 0);
					cb2 += 4;
					*(uint32_t *)(p + 0) = lo;

					uint32_t hi = ((uint32_t)*(uint16_t *)(cb2 + 2) << 16) | *(uint16_t *)(cb2 + 0);
					cb2 += 4;
					*(uint32_t *)(p + 4) = hi;

					p += step;
				}
				dst += 8;
			}

			src = cursor;
		} break;

		/* ------------------------------------------------------------ */
		/* 0x2000 - vertical run fill (0xA000 shares the body)          */
		/* ------------------------------------------------------------ */
		case 0x2000u: {
			code = cb_index;
			code &= 0xFFFFu;
			height = ((code >> 7) & 0x3Eu) + 2u;

			if (height != 0u) {
				goto label_masked_index;
			}

		label_run_length:
			cb_index &= 0xFFFFu;
			height = *src;
			src += 1;
			goto label_vertical_fill;
		}

		/* ------------------------------------------------------------ */
		/* 0x0000 - skip N blocks horizontally                          */
		/* ------------------------------------------------------------ */
		case 0x0000u: {
			dst += 8u * (uint8_t)cb_index;
		} break;

		/* ------------------------------------------------------------ */
		/* 0x6000 - unmasked solid 4x4 (no pointer read)                */
		/* ------------------------------------------------------------ */
		case 0x6000u: {
			const uint8_t * cb = codebook + ((cb_index & 0xFFFFu) << 5);

			for (uint32_t r = 0; r < 4; ++r) {
				uint32_t lo = ((uint32_t)*(uint16_t *)(cb + 2) << 16) | *(uint16_t *)(cb + 0);
				cb += 4;
				*(uint32_t *)(dst + 0) = lo;

				uint32_t hi = ((uint32_t)*(uint16_t *)(cb + 2) << 16) | *(uint16_t *)(cb + 0);
				cb += 4;
				*(uint32_t *)(dst + 4) = hi;

				dst += bufwidth;
			}

			dst += 8u - block_row_stride;
		} break;

		/* ------------------------------------------------------------ */
		/* 0xC000 - masked vertical run                                 */
		/* ------------------------------------------------------------ */
		case 0xC000u: {
			height = *src;
			src += 1;

			const uint8_t * cb = codebook + (32u * (uint16_t)cb_index);

			for (uint32_t r = 0; r < 4; ++r) {
				uint8_t * p = dst;

				for (uint32_t c = 0; c < 4; ++c) {
					uint16_t v = *(uint16_t *)cb;

					if (v != 0x8000u) {
						uint8_t * q = p;
						uint32_t i;
						for (i = 0; i < height; i++) {
							*(uint16_t *)q = v;
							q += 8;
						}
					}

					p += 2;
					cb += 2;
				}

				dst += bufwidth;
			}

			dst += 8u * height - block_row_stride;
		} break;

		/* ------------------------------------------------------------ */
		/* 0xA000 - masked vertical, shares the 0x2000 run-length head  */
		/* ------------------------------------------------------------ */
		case 0xA000u:
			goto label_run_length;

		/* ------------------------------------------------------------ */
		/* shared 0x2000/0xA000 vertical run fill                       */
		/* ------------------------------------------------------------ */
		label_masked_index:
			cb_index = (uint8_t)code;

		label_vertical_fill:
			{
				const uint8_t * cb = codebook + (32u * cb_index);

				for (uint32_t r = 0; r < 4; ++r) {

					uint32_t lo = ((uint32_t)*(uint16_t *)(cb + 2) << 16) | *(uint16_t *)(cb + 0);
					cb += 4;

					{
						uint8_t * p = dst;
						uint32_t i;
						for (i = 0; i < height; i++) {
							*(uint32_t *)p = lo;
							p += 8;
						}
					}

					uint32_t hi = ((uint32_t)*(uint16_t *)(cb + 2) << 16) | *(uint16_t *)(cb + 0);
					cb += 4;

					{
						uint8_t * p = dst + 4;
						uint32_t i;
						for (i = 0; i < height; i++) {
							*(uint32_t *)p = hi;
							p += 8;
						}
					}

					dst += bufwidth;
				}

				dst += 8u * height - block_row_stride;
			}
			break;

		/* ------------------------------------------------------------ */
		/* 0x8000 - masked 4x4 (0x8000 = transparent)                   */
		/* ------------------------------------------------------------ */
		case 0x8000u: {
			const uint8_t * cb = codebook + (32u * (uint16_t)cb_index);

			for (uint32_t r = 0; r < 4; ++r) {
				uint8_t * p = dst;

				for (uint32_t c = 0; c < 4; ++c) {
					if (*(uint16_t *)cb != 0x8000u) {
						*(uint16_t *)p = *(uint16_t *)cb;
					}
					p += 2;
					cb += 2;
				}

				dst += bufwidth;
			}

			dst += 8u - block_row_stride;
		} break;
		}

		/* ------------------------------------------------------------ */
		/* macro-row wrap                                               */
		/* ------------------------------------------------------------ */
		if (dst == row_end) {
			row_base += block_row_stride;
			dst = row_base;
			row_end = row_base + ((uint32_t)blocksperrow * 8u);
		}
	}
}


void __cdecl UnVQ1_C4_4x2(unsigned char * codebook, unsigned char * pointers, unsigned char * buffer, unsigned long blocksperrow, unsigned long numrows, unsigned long bufwidth)
{
	bufwidth *= 2u;
	uint32_t block_row_stride = bufwidth * 2u;

	uint8_t * end = (uint8_t *)buffer + ((uint32_t)numrows * bufwidth * 2u);
	uint8_t * dst = (uint8_t *)buffer;

	uint8_t * row_base = (uint8_t *)buffer;
	uint8_t * row_end = row_base + ((uint32_t)blocksperrow * 8u);

	uint16_t * src = (uint16_t *)pointers;

	while (dst < end) {

		uint32_t command = (*src & 0xE000);
		uint16_t cb_index = (*src & 0x1FFF);

		src += 1;

		switch (command) {

		/* ------------------------------------------------------------ */
		/* 0x0000 - unmasked 4x4 from codebook                          */
		/* ------------------------------------------------------------ */
		case 0x0000u: {
			const uint8_t * cb = (const uint8_t *)codebook + (cb_index << 4);

			for (uint32_t r = 0; r < 2; r++) {
				uint32_t lo;
				uint32_t hi;

				lo = ((uint32_t)*(const uint16_t *)(cb + 2) << 16) | *(const uint16_t *)(cb + 0);
				cb += 4;
				*(uint32_t *)(dst + 0) = lo;

				hi = ((uint32_t)*(const uint16_t *)(cb + 2) << 16) | *(const uint16_t *)(cb + 0);
				cb += 4;
				*(uint32_t *)(dst + 4) = hi;

				dst += bufwidth;
			}

			dst += 8u - block_row_stride;
		} break;

		/* ------------------------------------------------------------ */
		/* 0x2000 - masked 4x4 from codebook (0x8000 = transparent)     */
		/* ------------------------------------------------------------ */
		case 0x2000u: {
			const uint8_t * cb = (const uint8_t *)codebook + (cb_index << 4);

			for (uint32_t r = 0; r < 2; r++) {
				uint8_t * p = dst;

				for (uint32_t c = 0; c < 4; c++) {
					uint16_t v = *(const uint16_t *)cb;

					if (v != 0x8000u) {
						*(uint16_t *)p = v;
					}

					p += 2;
					cb += 2;
				}

				dst += bufwidth;
			}

			dst += 8u - block_row_stride;
		} break;

		/* ------------------------------------------------------------ */
		/* 0x4000 - horizontal skip of one 4x4 block                    */
		/* ------------------------------------------------------------ */
		case 0x4000u: {
			dst += 8;
		} break;

		/* ------------------------------------------------------------ */
		/* default - fall-through                                       */
		/* ------------------------------------------------------------ */
		default:
			break;
		}

		/* macro-row wrap */
		if (dst == row_end) {
			row_base += block_row_stride;
			dst = row_base;
			row_end = row_base + ((uint32_t)blocksperrow * 8u);
		}
	}
}


void __cdecl UnVQ2_C4_4x2(unsigned char * codebook, unsigned char * pointers, unsigned char * buffer, unsigned long blocksperrow, unsigned long numrows, unsigned long bufwidth)
{
	bufwidth *= 2u;
	uint32_t block_row_stride = bufwidth * 2u;

	uint8_t * end = (uint8_t *)buffer + ((uint32_t)numrows * bufwidth * 2u);
	uint8_t * dst = (uint8_t *)buffer;

	uint8_t * row_base = (uint8_t *)buffer;
	uint8_t * row_end = row_base + ((uint32_t)blocksperrow * 8u);

	uint8_t * src = pointers;

	uint32_t height;
	uint32_t code;

	while (dst < end) {

		uint32_t command = (*(uint16_t *)src & 0xE000);
		uint32_t cb_index = (*(uint16_t *)src & 0x1FFF);
		src += 2;

		switch (command) {

		/* ------------------------------------------------------------ */
		/* 0x4000 - patterned column replicate                          */
		/* ------------------------------------------------------------ */
		case 0x4000u: {
			const uint8_t * cb = codebook + (*(src - 2) << 4);
			uint8_t * rowp = dst;
			uint32_t repeat = ((cb_index >> 7) & 0x3Eu) + 2u;
			uint32_t step = 4u * (bufwidth >> 2);

			for (uint32_t r = 0; r < 2; ++r) {
				uint32_t lo = ((uint32_t)*(uint16_t *)(cb + 2) << 16) | *(uint16_t *)(cb + 0);
				cb += 4;
				*(uint32_t *)(rowp + 0) = lo;

				uint32_t hi = ((uint32_t)*(uint16_t *)(cb + 2) << 16) | *(uint16_t *)(cb + 0);
				cb += 4;
				*(uint32_t *)(rowp + 4) = hi;

				rowp += step;
			}

			uint8_t * cursor = src;
			dst += 8;

			for (int32_t i = 0; i < repeat; i++) {
				uint8_t * p = dst;
				const uint8_t * cb2 = codebook + (*cursor << 4);
				cursor += 1;

				for (uint32_t r = 0; r < 2; ++r) {
					uint32_t lo = ((uint32_t)*(uint16_t *)(cb2 + 2) << 16) | *(uint16_t *)(cb2 + 0);
					cb2 += 4;
					*(uint32_t *)(p + 0) = lo;

					uint32_t hi = ((uint32_t)*(uint16_t *)(cb2 + 2) << 16) | *(uint16_t *)(cb2 + 0);
					cb2 += 4;
					*(uint32_t *)(p + 4) = hi;

					p += step;
				}

				dst += 8;
			}

			src = cursor;
		} break;

		/* ------------------------------------------------------------ */
		/* 0x2000 - vertical run fill (0xA000 shares the body)          */
		/* ------------------------------------------------------------ */
		case 0x2000u: {
			code = cb_index;
			code &= 0xFFFFu;
			height = ((code >> 7) & 0x3Eu) + 2u;

			if (height == 0u) {
			label_run_length:
				cb_index &= 0xFFFFu;
				height = *src;
				src += 1;
				goto label_vertical_fill;
			} else {
				goto label_masked_index;
			}

		}

		/* ------------------------------------------------------------ */
		/* 0x0000 - skip N blocks horizontally                          */
		/* ------------------------------------------------------------ */
		case 0x0000u: {
			dst += 8u * (uint8_t)cb_index;
		} break;

		/* ------------------------------------------------------------ */
		/* 0x6000 - unmasked solid 4x2 (no pointer read)                */
		/* ------------------------------------------------------------ */
		case 0x6000u: {
			const uint8_t * cb = codebook + ((cb_index & 0xFFFFu) << 4);

			for (uint32_t r = 0; r < 2; ++r) {
				uint32_t lo = ((uint32_t)*(uint16_t *)(cb + 2) << 16) | *(uint16_t *)(cb + 0);
				cb += 4;
				*(uint32_t *)(dst + 0) = lo;

				uint32_t hi = ((uint32_t)*(uint16_t *)(cb + 2) << 16) | *(uint16_t *)(cb + 0);
				cb += 4;
				*(uint32_t *)(dst + 4) = hi;

				dst += bufwidth;
			}

			dst += 8u - block_row_stride;
		} break;

		/* ------------------------------------------------------------ */
		/* 0xC000 - masked vertical run                                 */
		/* ------------------------------------------------------------ */
		case 0xC000u: {
			height = *src;
			src += 1;

			const uint8_t * cb = codebook + (16u * (uint16_t)cb_index);

			for (uint32_t r = 0; r < 2; ++r) {
				uint8_t * p = dst;

				for (uint32_t c = 0; c < 4; ++c) {
					uint16_t v = *(uint16_t *)cb;

					if (v != 0x8000u) {
						uint8_t * q = p;
						for (uint32_t i = 0; i < height; i++) {
							*(uint16_t *)q = v;
							q += 8;
						}
					}

					p += 2;
					cb += 2;
				}

				dst += bufwidth;
			}

			dst += 8u * height - block_row_stride;
		} break;

		/* ------------------------------------------------------------ */
		/* 0xA000 - masked vertical, shares the 0x2000 run-length head  */
		/* ------------------------------------------------------------ */
		case 0xA000u:
			goto label_run_length;

		/* ------------------------------------------------------------ */
		/* shared 0x2000/0xA000 vertical run fill                       */
		/* ------------------------------------------------------------ */
		label_masked_index:
			cb_index = (uint8_t)code;

		label_vertical_fill:
			{
				const uint8_t * cb = codebook + (16u * cb_index);

				for (uint32_t r = 0; r < 2; ++r) {

					uint32_t lo = ((uint32_t)*(uint16_t *)(cb + 2) << 16) | *(uint16_t *)(cb + 0);
					cb += 4;

					{
						uint8_t * p = dst;
						uint32_t i;
						for (i = 0; i < height; i++) {
							*(uint32_t *)p = lo;
							p += 8;
						}
					}

					uint32_t hi = ((uint32_t)*(uint16_t *)(cb + 2) << 16) | *(uint16_t *)(cb + 0);
					cb += 4;

					{
						uint8_t * p = dst + 4;
						uint32_t i;
						for (i = 0; i < height; i++) {
							*(uint32_t *)p = hi;
							p += 8;
						}
					}

					dst += bufwidth;
				}

				dst += 8u * height - block_row_stride;
			}
			break;

		/* ------------------------------------------------------------ */
		/* 0x8000 - masked 4x2 (0x8000 = transparent)                   */
		/* ------------------------------------------------------------ */
		case 0x8000u: {
			const uint8_t * cb = codebook + (16u * (uint16_t)cb_index);

			for (uint32_t r = 0; r < 2; ++r) {
				uint8_t * p = dst;

				for (uint32_t c = 0; c < 4; ++c) {
					if (*(uint16_t *)cb != 0x8000u) {
						*(uint16_t *)p = *(uint16_t *)cb;
					}
					p += 2;
					cb += 2;
				}

				dst += bufwidth;
			}

			dst += 8u - block_row_stride;
		} break;
		}

		/* ------------------------------------------------------------ */
		/* macro-row wrap                                               */
		/* ------------------------------------------------------------ */
		if (dst == row_end) {
			row_base += block_row_stride;
			dst = row_base;
			row_end = row_base + ((uint32_t)blocksperrow * 8u);
		}
	}
}


void __cdecl UnVQ2_C0_4x4_TRANS(unsigned char * codebook, unsigned char * pointers, unsigned char * buffer, unsigned long blocksperrow, unsigned long numrows, unsigned long bufwidth)
{
	uint8_t * dst = (uint8_t *)buffer;
	uint8_t * row_base = (uint8_t *)buffer;
	uint16_t * src = (uint16_t *)pointers;
	uint32_t blocks = 0;
	uint32_t total = blocksperrow * numrows;

	if (total > 0u) {

		while (1) {

			uint16_t word = *src;

			if ((word & 0x8000u) != 0) {

				int32_t op = word & 0xF000u;

				if (op <= 0xC000) {

					if (op != 0xC000) {

						switch (op) {

						case 0xB000: {
							/* 0xB000 - solid color fill, count in bits 8-11 */
							uint32_t count = ((uint32_t)word >> 8) & 0xFu;
							int r;

							if ((uint8_t)word) {
								uint32_t n = 4u * count;

								for (r = 0; r < 4; ++r) {
									for (uint32_t k = 0; k < n; ++k) {
										*dst++ = (uint8_t)word;
									}
									dst -= n;
									dst += bufwidth;
								}

								dst -= 4u * bufwidth;
							}

							dst += 4u * count;
							blocks += count;
							src += 1;
						} break;

						case 0xA000: {
							/* 0xA000 - read a second word, select sub-mode */
							uint16_t word2 = src[1];
							src += 1;
							uint32_t count = word & 0xFFFu;
							int r;

							if ((word2 & 0x8000u) != 0) {
								int sub = word2 & 0xC000;

								switch (sub) {

								case 0xC000: {
									/* transparent codebook run */
									const uint8_t * cb = codebook + ((uint32_t)(word2 & 0xFFFu) << 4);

									for (r = 0; r < 4; ++r) {
										uint32_t nn;
										for (nn = 0; nn < count; nn++) {
											for (int c = 0; c < 4; ++c) {
												if (*cb) {
													*dst = *cb;
												}
												++dst;
												++cb;
											}
											cb -= 4;
										}
										cb += 4;
										dst -= 4u * count;
										dst += bufwidth;
									}

									dst += 4u * count;

									dst -= 4u * bufwidth;
									blocks += count;
									src += 1;
								} break;

								case 0x8000: {
									/* solid color run, color = low byte of word2 */
									if ((uint8_t)word2) {
										uint32_t n = 4u * count;

										for (r = 0; r < 4; ++r) {
											for (uint32_t k = 0; k < n; ++k) {
												*dst++ = (uint8_t)word2;
											}
											dst -= n;
											dst += bufwidth;
										}

										dst -= 4u * bufwidth;
									}

									dst += 4u * count;
									blocks += count;
									src += 1;

								} break;
								}

							} else {
								/* opaque codebook run */
								const uint8_t * cb = codebook + ((uint32_t)word2 << 4);

								for (r = 0; r < 4; ++r) {
									uint32_t nn;
									for (nn = 0; nn < count; nn++) {
										for (int c = 0; c < 4; ++c) {
											*dst++ = *cb++;
										}
										cb -= 4;
									}
									cb += 4;
									dst -= 4u * count;
									dst += bufwidth;
								}

								src += 1;
								dst += 4u * count;
								dst -= 4u * bufwidth;
								blocks += count;
							}

						} break;
						}

					} else {
						/* 0xC000 - paired solid colors, two blocks per stream word */
						uint32_t count = word & 0xFFFu;
						int r;
						src += 1;

						uint32_t pairs = count >> 1;

						if (pairs != 0u) {
							do {
								uint16_t word2 = *src;
								uint8_t hi = (uint8_t)(word2 >> 8);

								if (hi != 0u) {
									for (r = 0; r < 4; ++r) {
										for (int c = 0; c < 4; ++c) {
											*dst++ = hi;
										}
										dst -= 4u;
										dst += bufwidth;
									}
									dst -= 4u * bufwidth;
								}
								dst += 4;

								if ((uint8_t)word2) {
									for (r = 0; r < 4; ++r) {
										for (int c = 0; c < 4; ++c) {
											*dst++ = (uint8_t)word2;
										}
										dst -= 4u;
										dst += bufwidth;
									}
									dst -= 4u * bufwidth;
								}
								dst += 4;

								src += 1;
								pairs--;
							} while (pairs);
						}

						blocks += count;
					}

				} else {

					switch (op) {

					case 0xF000: {
						/* 0xF000 - solid single block, color = low byte */
						int r;
						if ((uint8_t)word) {
							for (r = 0; r < 4; ++r) {
								for (int c = 0; c < 4; ++c) {
									*dst++ = (uint8_t)word;
								}
								dst -= 4u;
								dst += bufwidth;
							}
							dst -= 4u * bufwidth;
						}

						dst += 4;
						src += 1;
						blocks += 1;
					} break;

					case 0xE000: {
						/* 0xE000 - transparent single block from codebook */
						const uint8_t * cb = codebook + ((uint32_t)(word & 0xFFFu) << 4);
						int r;

						for (r = 0; r < 4; ++r) {
							for (int c = 0; c < 4; ++c) {
								if (*cb) {
									*dst = *cb;
								}
								++dst;
								++cb;
							}
							dst -= 4u;
							dst += bufwidth;
						}

						dst -= 4u * bufwidth;

						dst += 4u;
						src += 1;
						blocks += 1;

					} break;
					}
				}

			} else {
				/* high bit clear - opaque single block from codebook */
				const uint8_t * cb = codebook + ((uint32_t)*src << 4);
				int r;

				for (r = 0; r < 4; ++r) {
					for (int c = 0; c < 4; ++c) {
						*dst++ = *cb++;
					}
					dst -= 4u;
					dst += bufwidth;
				}

				dst -= 4u * bufwidth;

				dst += 4u;
				src += 1;
				blocks += 1;
			}

			/* macro-row wrap */
			if (blocks % blocksperrow == 0u) {
				dst = row_base + 4u * bufwidth;
				row_base = dst;
			}

			if (blocks >= total) {
				return;
			}
		}
	}
}


void __cdecl UnVQ2_C0_4x4_KEY(unsigned char * codebook, unsigned char * pointers, unsigned char * buffer, unsigned long blocksperrow, unsigned long numrows, unsigned long bufwidth)
{
	uint8_t * dst = (uint8_t *)buffer;
	uint8_t * row_base = (uint8_t *)buffer;
	uint16_t * src = (uint16_t *)pointers;
	uint32_t blocks = 0;
	uint32_t total = numrows * blocksperrow;

	if (total > 0u) {

		while (1) {

			uint16_t word = *src;

			if ((word & 0x8000u) != 0) {

				int32_t op = word & 0xF000u;

				if (op <= 0xC000) {

					if (op != 0xC000) {

						switch (op) {

						case 0xB000: {
						/* 0xB000 - solid color fill, count in bits 8-11 */
						uint32_t count = ((uint32_t)word >> 8) & 0xFu;
						int r;

						uint32_t n = 4u * count;

						for (r = 0; r < 4; ++r) {
							for (uint32_t k = 0; k < n; ++k) {
								*dst++ = (uint8_t)word;
							}
							dst -= n;
							dst += bufwidth;
						}

						dst += 4u * count;

						dst -= 4u * bufwidth;
						blocks += count;
						src += 1;
						} break;


						case 0xA000: {
						/* 0xA000 - read a second word, select sub-mode */
						int16_t word2 = (int16_t)src[1];
						src += 1;
						uint32_t count = word & 0xFFFu;
						int r;

						if (word2 < 0) {
							int sub = word2 & 0xC000;

							if (sub != 0x8000) {

								if (sub == 0xC000) {
									/* opaque codebook run, masked index */
									const uint8_t * cb = codebook + ((uint32_t)(word2 & 0xFFF) << 4);

									for (r = 0; r < 4; ++r) {
										uint32_t nn;
										for (nn = 0; nn < count; nn++) {
											for (int c = 0; c < 4; ++c) {
												*dst++ = *cb++;
											}
											cb -= 4;
										}
										cb += 4;
										dst -= 4u * count;
										dst += bufwidth;
									}

									dst += 4u * count;

									dst -= 4u * bufwidth;
									blocks += count;
									src += 1;
								}

							} else {
								/* solid color run, color = low byte of word2 */
								uint32_t n = 4u * count;

								for (r = 0; r < 4; ++r) {
									for (uint32_t k = 0; k < n; ++k) {
										*dst++ = (uint8_t)word2;
									}
									dst -= n;
									dst += bufwidth;
								}

								dst += 4u * count;

								dst -= 4u * bufwidth;
								blocks += count;
								src += 1;
							}

						} else {
							/* opaque codebook run */
							const uint8_t * cb = codebook + ((uint32_t)*src << 4);

							for (r = 0; r < 4; ++r) {
								uint32_t nn;
								for (nn = 0; nn < count; nn++) {
									for (int c = 0; c < 4; ++c) {
										*dst++ = *cb++;
									}
									cb -= 4;
								}
								cb += 4;
								dst -= 4u * count;
								dst += bufwidth;
							}

							dst += 4u * count;

							dst -= 4u * bufwidth;
							blocks += count;
							src += 1;
						}
						} break;

						}

					} else {

						/* 0xC000 - paired solid colors, two blocks per stream word */
						uint32_t count = word & 0xFFFu;
						int r;

						src += 1;

						if ((count >> 1) != 0u) {
							uint32_t pairs = count >> 1;

							do {
								uint16_t word2 = *src;
								uint8_t hi = (uint8_t)(word2 >> 8);

								for (r = 0; r < 4; ++r) {
									for (int c = 0; c < 4; ++c) {
										*dst++ = hi;
									}
									dst -= 4u;
									dst += bufwidth;
								}
								dst -= 4u * bufwidth;
								dst += 4u;

								for (r = 0; r < 4; ++r) {
									for (int c = 0; c < 4; ++c) {
										*dst++ = (uint8_t)word2;
									}
									dst -= 4u;
									dst += bufwidth;
								}
								dst -= 4u * bufwidth;
								dst += 4u;

								src += 1;
								pairs--;
							} while (pairs);
						}

						blocks += count;
					}

				} else {

					switch (op) {

					case 0xF000: {
						/* 0xF000 - solid single block, color = low byte */
						int r;

						for (r = 0; r < 4; ++r) {
							for (int c = 0; c < 4; ++c) {
								*dst++ = (uint8_t)word;
							}
							dst -= 4u;
							dst += bufwidth;
						}

						dst -= 4u * bufwidth;

						dst += 4u;
						src += 1;
						blocks += 1;
					} break;

					case 0xE000: {
						/* 0xE000 - opaque single block from codebook */
						const uint8_t * cb = codebook + ((uint32_t)(word & 0xFFFu) << 4);
						int r;

						for (r = 0; r < 4; ++r) {
							for (int c = 0; c < 4; ++c) {
								*dst++ = *cb++;
							}
							dst -= 4u;
							dst += bufwidth;
						}

						dst -= 4u * bufwidth;

						dst += 4u;
						src += 1;
						blocks += 1;
					} break;
					}

				}

			} else {
				/* high bit clear - opaque single block from codebook */
				const uint8_t * cb = codebook + ((uint32_t)*src << 4);
				int r;

				for (r = 0; r < 4; ++r) {
					for (int c = 0; c < 4; ++c) {
						*dst++ = *cb++;
					}
					dst -= 4u;
					dst += bufwidth;
				}

				dst -= 4u * bufwidth;

				dst += 4u;
				src += 1;
				blocks += 1;
			}

			/* macro-row wrap */
			if (blocks % blocksperrow == 0u) {
				dst = row_base + 4u * bufwidth;
				row_base = dst;
			}

			if (blocks >= total) {
				return;
			}
		}
	}
}


void __cdecl UnVQ2_C0_4x4_TRANS_HALF(unsigned char * codebook, unsigned char * pointers, unsigned char * buffer, unsigned long blocksperrow, unsigned long numrows, unsigned long bufwidth)
{
	uint8_t * dst = (uint8_t *)buffer;
	uint8_t * row_base = (uint8_t *)buffer;
	uint16_t * src = (uint16_t *)pointers;
	uint32_t blocks = 0;
	uint32_t total = numrows * blocksperrow;

	if (total > 0u) {

		while (1) {

			uint16_t word = *src;

			if ((word & 0x8000u) != 0) {

				int op = (word & 0xFFFF) & 0xF000;

				if (op <= 0xC000) {

					if (op != 0xC000) {

						switch (op) {

						case 0xB000: {
							/* 0xB000 - solid color fill, count in bits 8-11 */
							uint32_t count = ((uint32_t)word >> 8) & 0xFu;
							int r;

							if ((uint8_t)word) {
								uint32_t n = 2u * count;

								for (r = 0; r < 2; ++r) {
									for (uint32_t k = 0; k < n; ++k) {
										*dst++ = (uint8_t)word;
									}
									dst -= n;
									dst += bufwidth;
								}

								dst -= 2u * bufwidth;
							}

							dst += 2u * count;
							blocks += count;
							src += 1;
						} break;

						case 0xA000: {
							/* 0xA000 - read a second word, select sub-mode */
							uint16_t word2 = src[1];
							src += 1;
							uint32_t count = word & 0xFFFu;
							int r;

							if ((word2 & 0x8000u) != 0) {
								int sub = word2 & 0xC000;

								switch (sub) {

								case 0xC000: {
									/* transparent codebook run */
									const uint8_t * cb = codebook + ((uint32_t)(word2 & 0xFFFu) << 4);

									for (r = 0; r < 2; ++r) {
										uint32_t nn;
										for (nn = 0; nn < count; nn++) {
											for (int c = 0; c < 2; ++c) {
												if (*cb) {
													*dst = *cb;
												}
												++dst;
												cb += 2;
											}
											cb -= 4;
										}
										cb += 4;
										dst -= 2u * count;
										dst += bufwidth;
									}

									dst += 2u * count;

									dst -= 2u * bufwidth;
									blocks += count;
									src += 1;
								} break;

								case 0x8000: {
									/* solid color run, color = low byte of word2 */
									if ((uint8_t)word2) {
										uint32_t n = 2u * count;

										for (r = 0; r < 2; ++r) {
											for (uint32_t k = 0; k < n; ++k) {
												*dst++ = (uint8_t)word2;
											}
											dst -= n;
											dst += bufwidth;
										}

										dst -= 2u * bufwidth;
									}

									dst += 2u * count;
									blocks += count;
									src += 1;

								} break;
								}

							} else {
								/* opaque codebook run */
								const uint8_t * cb = codebook + ((uint32_t)word2 << 4);

								for (r = 0; r < 2; ++r) {
									uint32_t nn;
									for (nn = 0; nn < count; nn++) {
											for (int c = 0; c < 2; ++c) {
												*dst++ = *cb;
												cb += 2;
											}
											cb -= 4;
									}
									cb += 4;
									dst -= 2u * count;
									dst += bufwidth;
								}

								src += 1;
								dst += 2u * count;
								dst -= 2u * bufwidth;
								blocks += count;
							}

						} break;
						}

					} else {
						/* 0xC000 - paired solid colors, two blocks per stream word */
						uint32_t count = word & 0xFFFu;
						int r;
						src += 1;

						uint32_t pairs = count >> 1;

						if (pairs != 0u) {
							do {
								uint16_t word2 = *src;
								uint8_t hi = (uint8_t)(word2 >> 8);

								if (hi != 0u) {
									for (r = 0; r < 2; ++r) {
										for (int c = 0; c < 2; ++c) {
											*dst++ = hi;
										}
										dst -= 2u;
										dst += bufwidth;
									}
									dst -= 2u * bufwidth;
								}
								dst += 2;

								if ((uint8_t)word2 != 0u) {
									for (r = 0; r < 2; ++r) {
										for (int c = 0; c < 2; ++c) {
											*dst++ = (uint8_t)word2;
										}
										dst -= 2u;
										dst += bufwidth;
									}
									dst -= 2u * bufwidth;
								}
								dst += 2;

								src += 1;
								pairs--;
							} while (pairs);
						}

						blocks += count;
					}

				} else {

					switch (op) {

					case 0xF000: {
						/* 0xF000 - solid single block, color = low byte */
						uint32_t r;
						if ((uint8_t)word) {
							for (r = 0; r < 2; ++r) {
								for (int c = 0; c < 2; ++c) {
									*dst++ = (uint8_t)word;
								}
								dst -= 2u;
								dst += bufwidth;
							}
							dst -= 2u * bufwidth;
						}

						dst += 2;
						src += 1;
						blocks += 1;
					} break;

					case 0xE000: {
						/* 0xE000 - transparent single block from codebook */
						const uint8_t * cb = codebook + ((uint32_t)(word & 0xFFFu) << 4);
						int r;

						for (r = 0; r < 2; ++r) {
							for (int c = 0; c < 2; ++c) {
								if (*cb) {
									*dst = *cb;
								}
								++dst;
								cb += 2;
							}
							dst -= 2u;
							dst += bufwidth;
						}

						dst -= 2u * bufwidth;

						dst += 2u;
						src += 1;
						blocks += 1;

					} break;
					}
				}

			} else {
				/* high bit clear - opaque single block from codebook */
				const uint8_t * cb = codebook + ((uint32_t)*src << 4);
				int r;

				for (r = 0; r < 2; ++r) {
					for (int c = 0; c < 2; ++c) {
						*dst++ = *cb;
						cb += 2;
					}
					dst -= 2u;
					dst += bufwidth;
				}

				dst -= 2u * bufwidth;

				dst += 2u;
				src += 1;
				blocks += 1;
			}

			/* macro-row wrap */
			if (blocks % blocksperrow == 0u) {
				dst = row_base + 2u * bufwidth;
				row_base = dst;
			}

			if (blocks >= total) {
				return;
			}
		}
	}
}


void __cdecl UnVQ2_C0_4x2_TRANS(unsigned char * codebook, unsigned char * pointers, unsigned char * buffer, unsigned long blocksperrow, unsigned long numrows, unsigned long bufwidth)
{
	uint8_t * dst = (uint8_t *)buffer;
	uint16_t * src = (uint16_t *)pointers;
	uint32_t blocks = 0;
	uint8_t * row_base = (uint8_t *)buffer;
	uint32_t total = numrows * blocksperrow;

	if (total > 0u) {

		while (1) {

			uint16_t word = *src;

			if ((word & 0x8000u) != 0) {

				int32_t op = word & 0xF000u;

				if (op <= 0xC000) {

					if (op != 0xC000) {

						switch (op) {

						case 0xB000: {
							/* 0xB000 - solid color fill, count in bits 8-11 */
							uint32_t count = ((uint32_t)word >> 8) & 0xFu;
							int r;

							if ((uint8_t)word) {
								uint32_t n = 4u * count;

								for (r = 0; r < 2; ++r) {
									for (uint32_t k = 0; k < n; ++k) {
										*dst++ = (uint8_t)word;
									}
									dst -= n;
									dst += bufwidth;
								}

								dst -= 2u * bufwidth;
							}

							dst += 4u * count;
							blocks += count;
							src += 1;
						} break;

						case 0xA000: {
							/* 0xA000 - read a second word, select sub-mode */
							src += 1;
							uint16_t word2 = *src;
							uint32_t count = word & 0xFFFu;
							int r;

							if ((word2 & 0x8000u) != 0) {
								int sub = word2 & 0xC000;

								switch (sub) {

								case 0xC000: {
									/* transparent codebook run */
									const uint8_t * cb = codebook + ((uint32_t)(word2 & 0xFFFu) << 3);

									for (r = 0; r < 2; ++r) {
										uint32_t nn;
										for (nn = 0; nn < count; nn++) {
											for (int c = 0; c < 4; ++c) {
												if (*cb) {
													*dst = *cb;
												}
												++dst;
												++cb;
											}
											cb -= 4;
										}
										cb += 4;
										dst -= 4u * count;
										dst += bufwidth;
									}

									src += 1;
									dst += 4u * count;
									dst -= 2u * bufwidth;
									blocks += count;

								} break;

								case 0x8000: {
									/* solid color run, color = low byte of word2 */
									if ((uint8_t)word2) {
										uint32_t n = 4u * count;

										for (r = 0; r < 2; ++r) {
											for (uint32_t k = 0; k < n; ++k) {
												*dst++ = (uint8_t)word2;
											}
											dst -= n;
											dst += bufwidth;
										}

										dst -= 2u * bufwidth;
									}

									blocks += count;
									dst += 4u * count;
									src += 1;

								} break;
								}

							} else {
								/* opaque codebook run */
								const uint8_t * cb = codebook + ((uint32_t)word2 << 3);

								for (r = 0; r < 2; ++r) {
									uint32_t nn;
									for (nn = 0; nn < count; nn++) {
										for (int c = 0; c < 4; ++c) {
											*dst++ = *cb++;
										}
										cb -= 4;
									}
									cb += 4;
									dst -= 4u * count;
									dst += bufwidth;
								}

								src += 1;
								dst += 4u * count;
								dst -= 2u * bufwidth;
								blocks += count;
							}

						} break;
						}

					} else {
						/* 0xC000 - paired solid colors, two blocks per stream word */
						uint32_t count = word & 0xFFFu;
						int r;
						src += 1;

						uint32_t pairs = count >> 1;

						if (pairs != 0u) {
							do {
								uint16_t word2 = *src;
								uint8_t hi = (uint8_t)(word2 >> 8);

								if (hi != 0u) {
									for (r = 0; r < 2; ++r) {
										for (int c = 0; c < 4; ++c) {
											*dst++ = hi;
										}
										dst -= 4u;
										dst += bufwidth;
									}
									dst -= 2u * bufwidth;
								}
								dst += 4;

								uint8_t lo = (uint8_t)word2 & 0xFF;

								if (lo != 0u) {
									for (r = 0; r < 2; ++r) {
										for (int c = 0; c < 4; ++c) {
											*dst++ = lo;
										}
										dst -= 4u;
										dst += bufwidth;
									}
									dst -= 2u * bufwidth;
								}
								dst += 4;

								src += 1;
								pairs--;
							} while (pairs);
						}

						blocks += count;
					}

				} else {

					switch (op) {

					case 0xF000: {
						/* 0xF000 - solid single block, color = low byte */
						int r;
						if ((uint8_t)word) {
							for (r = 0; r < 2; ++r) {
								for (int c = 0; c < 4; ++c) {
									*dst++ = (uint8_t)word;
								}
								dst -= 4u;
								dst += bufwidth;
							}
							dst -= 2u * bufwidth;
						}

						dst += 4;
						src += 1;
						blocks += 1;
					} break;

					case 0xE000: {
						/* 0xE000 - transparent single block from codebook */
						const uint8_t * cb = codebook + ((uint32_t)(word & 0xFFFu) << 3);
						int r;

						for (r = 0; r < 2; ++r) {
							for (int c = 0; c < 4; ++c) {
								if (*cb) {
									*dst = *cb;
								}
								++dst;
								++cb;
							}
							dst -= 4u;
							dst += bufwidth;
						}

						dst -= 2u * bufwidth;

						dst += 4u;
						src += 1;
						blocks += 1;

					} break;
					}
				}

			} else {
				/* high bit clear - opaque single block from codebook */
				const uint8_t * cb = codebook + ((uint32_t)*src << 3);
				int r;

				for (r = 0; r < 2; ++r) {
					for (int c = 0; c < 4; ++c) {
						*dst++ = *cb++;
					}
					dst -= 4u;
					dst += bufwidth;
				}

				dst -= 2u * bufwidth;

				dst += 4u;
				src += 1;
				blocks += 1;
			}

			/* macro-row wrap */
			if (blocks % blocksperrow == 0u) {
				dst = row_base + 2u * bufwidth;
				row_base = dst;
			}

			if (blocks >= total) {
				return;
			}
		}
	}
}


void __cdecl UnVQ2_C0_4x2_KEY(unsigned char * codebook, unsigned char * pointers, unsigned char * buffer, unsigned long blocksperrow, unsigned long numrows, unsigned long bufwidth)
{
	uint8_t * dst = (uint8_t *)buffer;
	uint8_t * row_base = (uint8_t *)buffer;
	uint16_t * src = (uint16_t *)pointers;
	uint32_t blocks = 0;
	uint32_t total = numrows * blocksperrow;

	if (total > 0u) {

		while (1) {

			uint16_t word = *src;

			if ((word & 0x8000u) != 0) {

				int32_t op = word & 0xF000u;

				if (op <= 0xC000) {

					if (op != 0xC000) {

						switch (op) {

						case 0xB000: {
						/* 0xB000 - solid color fill, count in bits 8-11 */
						uint32_t count = ((uint32_t)word >> 8) & 0xFu;
						int r;

						uint32_t n = 4u * count;

						for (r = 0; r < 2; ++r) {
							for (uint32_t k = 0; k < n; ++k) {
								*dst++ = (uint8_t)word;
							}
							dst -= n;
							dst += bufwidth;
						}

						dst += 4u * count;

						dst -= 2u * bufwidth;
						blocks += count;
						src += 1;
						} break;


						case 0xA000: {
						/* 0xA000 - read a second word, select sub-mode */
						int16_t word2 = (int16_t)src[1];
						src += 1;
						uint32_t count = word & 0xFFFu;
						int r;

						if (word2 != 0) {
							int sub = word2 & 0xC000;

							if (sub != 0x8000) {

								if (sub == 0xC000) {
									/* opaque codebook run, masked index */
									const uint8_t * cb = codebook + ((uint32_t)(word2 & 0xFFF) << 3);

									for (r = 0; r < 2; ++r) {
										uint32_t nn;
										for (nn = 0; nn < count; nn++) {
											for (int c = 0; c < 4; ++c) {
												*dst++ = *cb++;
											}
											cb -= 4;
										}
										cb += 4;
										dst -= 4u * count;
										dst += bufwidth;
									}

									dst += 4u * count;

									dst -= 2u * bufwidth;
									blocks += count;
									src += 1;
								}

							} else {
								/* solid color run, color = low byte of word2 */
								uint32_t n = 4u * count;

								for (r = 0; r < 2; ++r) {
									for (uint32_t k = 0; k < n; ++k) {
										*dst++ = (uint8_t)word2;
									}
									dst -= n;
									dst += bufwidth;
								}

								dst += 4u * count;

								dst -= 2u * bufwidth;
								blocks += count;
								src += 1;
							}

						} else {
							/* opaque codebook run */
							const uint8_t * cb = codebook + ((uint32_t)*src << 3);

							for (r = 0; r < 2; ++r) {
								uint32_t nn;
								for (nn = 0; nn < count; nn++) {
									for (int c = 0; c < 4; ++c) {
										*dst++ = *cb++;
									}
									cb -= 4;
								}
								cb += 4;
								dst -= 4u * count;
								dst += bufwidth;
							}

							dst += 4u * count;

							dst -= 2u * bufwidth;
							blocks += count;
							src += 1;
						}
						} break;

						}

					} else {

						/* 0xC000 - paired solid colors, two blocks per stream word */
						uint32_t count = word & 0xFFFu;
						int r;

						src += 1;

						uint32_t pairs = count >> 1;

						if (pairs != 0u) {
							do {
								uint16_t word2 = *src;
								uint8_t hi = (uint8_t)(word2 >> 8);

								for (r = 0; r < 2; ++r) {
									for (int c = 0; c < 4; ++c) {
										*dst++ = hi;
									}
									dst -= 4u;
									dst += bufwidth;
								}
								dst -= 2u * bufwidth;
								dst += 4u;

								for (r = 0; r < 2; ++r) {
									for (int c = 0; c < 4; ++c) {
										*dst++ = (uint8_t)word2;
									}
									dst -= 4u;
									dst += bufwidth;
								}
								dst -= 2u * bufwidth;
								dst += 4u;

								src += 1;
								pairs--;
							} while (pairs);
						}

						blocks += count;
					}

				} else {

					switch (op) {

					case 0xF000: {
						/* 0xF000 - solid single block, color = low byte */
						int r;

						for (r = 0; r < 2; ++r) {
							for (int c = 0; c < 4; ++c) {
								*dst++ = (uint8_t)word;
							}
							dst -= 4u;
							dst += bufwidth;
						}

						dst -= 2u * bufwidth;

						dst += 4u;
						src += 1;
						blocks += 1;
					} break;

					case 0xE000: {
						/* 0xE000 - opaque single block from codebook */
						const uint8_t * cb = codebook + ((uint32_t)(word & 0xFFFu) << 3);
						int r;

						for (r = 0; r < 2; ++r) {
							for (int c = 0; c < 4; ++c) {
								*dst++ = *cb++;
							}
							dst -= 4u;
							dst += bufwidth;
						}

						dst -= 2u * bufwidth;

						dst += 4u;
						src += 1;
						blocks += 1;
					} break;
					}

				}

			} else {
				/* high bit clear - opaque single block from codebook */
				const uint8_t * cb = codebook + ((uint32_t)*src << 3);
				int r;

				for (r = 0; r < 2; ++r) {
					for (int c = 0; c < 4; ++c) {
						*dst++ = *cb++;
					}
					dst -= 4u;
					dst += bufwidth;
				}

				dst -= 2u * bufwidth;

				dst += 4u;
				src += 1;
				blocks += 1;
			}

			/* macro-row wrap */
			if (blocks % blocksperrow == 0u) {
				dst = row_base + 2u * bufwidth;
				row_base = dst;
			}

			if (blocks >= total) {
				return;
			}
		}
	}
}



