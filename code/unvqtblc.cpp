/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "always.h"

#include "unvqtblc.h"

#include "_vqa.h"
#include "dsurface.h"

#include <cassert>

unsigned short * HicolorTable;

static bool Hicolor_Set_Color_Mode(int cmode);

bool Hicolor_Init_Table(int cmode)
{
	Hicolor_Clear_Table();

	HicolorTable = new unsigned short[65535 - 1];
	if (HicolorTable != NULL) {
		return(Hicolor_Set_Color_Mode(cmode));
	}

	return(false);
}


void Hicolor_Clear_Table(void)
{
	if (HicolorTable != NULL) {
		delete[] HicolorTable;
		HicolorTable = NULL;
	}
}


bool Hicolor_Set_Color_Mode(int cmode)
{
	bool valid = false;

	int redshift, greenshift, blueshift;
	switch (cmode) {
	case COLORMODE_555:
		redshift = 10;
		greenshift = 5;
		blueshift = 0;
		valid = true;
		break;
	case COLORMODE_655:
		redshift = 11;
		greenshift = 5;
		blueshift = 0;
		valid = true;
		break;
	case COLORMODE_565:
		redshift = 11;
		greenshift = 6;
		blueshift = 0;
		valid = true;
		break;
	case COLORMODE_556:
		redshift = 11;
		greenshift = 6;
		blueshift = 1;
		valid = true;
		break;
	default:
		break;
	}

	if (valid) {
		for (int r = 0; r < 32; r++) {
			for (int g = 0; g < 32; g++) {
				for (int b = 0; b < 32; b++) {
					HicolorTable[(unsigned short)(32 * ((32 * r) | g)) | (unsigned short)b] = (r << redshift) | (g << greenshift) | (b << blueshift);
				}
			}
		}

		return(true);
	}

	return(false);
}


void Hicolor_Translate(void * buffer, int size)
{
	unsigned short * buf = (unsigned short *)buffer;
	while (size > 0) {
		*buf = HicolorTable[*buf];
		buf++;
		size--;
	}
}

static inline void memset32(void * D, unsigned int val, unsigned int n)
{
	unsigned int * dst = (unsigned int *)D;
	unsigned int i;

	for (i = 0; i < n; i++) dst[i] = val;
}

void __cdecl UnVQ2_4x4_Table(uint8_t * codebook, uint8_t * pointers, uint8_t * buffer, size_t blocksperrow, size_t numrows, size_t bufwidth)
{
	assert(HicolorTable != 0);
	assert(codebook != 0);
	assert(pointers != 0);
	assert(buffer != 0);
	assert(blocksperrow != 0);
	assert(numrows != 0);
	assert(bufwidth != 0);

	bufwidth = 2 * bufwidth;
	unsigned int last_pos = 8 * blocksperrow;
	unsigned char * data_end = &buffer[4 * numrows * bufwidth];
	unsigned char * last_buf_pos = &buffer[8 * blocksperrow];
	unsigned int blocks_per_rowa = 4 * bufwidth;
	unsigned short * ptrs = (unsigned short *)pointers;
	unsigned char * row_ptr = buffer;

	while (buffer < data_end) {
		unsigned short valw = *ptrs++;
		unsigned int val = valw;
		unsigned int count = val & 0xFFFF0FFF;
		int code = val & 0xF000;
		if (code <= 0x3000) {
			if (code != 0x3000) {
				if (code) {
					if (code != 0x1000) {
						if (code == 0x2000) {
							unsigned int len = 4;
							unsigned short * cbptr = (unsigned short *)&codebook[32 * *ptrs++];
							while (len > 0) {
								unsigned int w0 = cbptr[0];
								unsigned int w1 = cbptr[1];
								int pixels = w0 | (w1 << 16);
								cbptr += 2;
								unsigned char * dptr = buffer;
								unsigned int rows = count;
								while (rows > 0) {
									*(unsigned int *)dptr = pixels;
									dptr += 8;
									--rows;
								}
								w0 = cbptr[0];
								w1 = cbptr[1];
								int pixels2 = w0 | (w1 << 16);
								cbptr += 2;
								unsigned char * dptr2 = buffer + 4;
								unsigned int c2 = count;
								while (c2 > 0) {
									*(unsigned int *)dptr2 = pixels2;
									dptr2 += 8;
									--c2;
								}
								buffer += bufwidth;
								--len;
							}
							buffer += 8 * count - blocks_per_rowa;
						}
					} else {
						buffer += 8 * count;
					}
				} else {
					int len = 2 * count;
					unsigned short pixel = HicolorTable[*ptrs++];
					int value = pixel | (pixel << 16);
					if (2 * count) {
						memset32(buffer, value, len);
					}
					if (len) {
						memset32(&buffer[bufwidth], value, len);
					}
					if (len) {
						memset32(&buffer[2 * bufwidth], value, len);
					}
					if (len) {
						memset32(&buffer[2 * bufwidth + bufwidth], value, len);
					}
					buffer += 8 * count;
				}
			} else {
				unsigned int len = 4;
				unsigned short * cbsrc = (unsigned short *)&codebook[32 * *ptrs++];
				while (len > 0) {
					unsigned short * dstw = (unsigned short *)buffer;
					unsigned int c2 = 4;
					while (c2 > 0) {
						unsigned short cbpixel = *cbsrc;
						if ((cbpixel & 0x8000) == 0) {
							unsigned short * dstwalk = dstw;
							unsigned int c3 = count;
							while (c3 > 0) {
								*dstwalk = cbpixel;
								dstwalk += 4;
								--c3;
							}
						}
						++dstw;
						++cbsrc;
						--c2;
					}
					buffer += bufwidth;
					--len;
				}

				buffer += 8 * count - blocks_per_rowa;
			}
		} else {
			switch (code) {
			case 0x5000:
				if (count > 0) {
					int fillwidth = 4 * (bufwidth >> 2);
					while (count > 0) {
						unsigned int c2 = 4;
						int fillpix = HicolorTable[*ptrs] | (HicolorTable[*ptrs] << 16);
						++ptrs;
						unsigned char * fillptr = buffer;
						while (c2 > 0) {
							*(unsigned int *)fillptr = fillpix;
							*((unsigned int *)fillptr + 1) = fillpix;
							fillptr += fillwidth;
							--c2;
						}
						buffer += 8;
						--count;
					};
				}
				break;
			case 0x6000:
				if (count > 0) {
					int rowwidth = 4 * (bufwidth >> 2);
					while (count > 0) {
						unsigned char * rowptr = buffer;
						unsigned short * cbrow = (unsigned short *)&codebook[32 * *ptrs++];
						unsigned int c2 = 4;
						while (c2 > 0) {
							unsigned short hi0 = *((unsigned short *)cbrow + 1);
							unsigned short lo0 = *(unsigned short *)cbrow;
							unsigned short * cbnext = cbrow + 2;
							*(unsigned int *)rowptr = lo0 | (hi0 << 16);
							unsigned short hi1 = cbnext[1];
							unsigned short lo1 = *cbnext;
							cbrow = (cbnext + 2);
							*((unsigned int *)rowptr + 1) = lo1 | (hi1 << 16);
							rowptr += rowwidth;
							--c2;
						}
						buffer += 8;
						--count;
					};
				}
				break;
			case 0x7000:
				if (count > 0) {
					while (count > 0) {
						int c2 = 4;
						unsigned short * cbline = (unsigned short *)&codebook[32 * *ptrs++];
						unsigned char * lineptr = buffer;
						while (c2) {
							unsigned short * linew = (unsigned short *)lineptr;
							unsigned int c3 = 4;
							while (c3 > 0) {
								if ((*cbline & 0x8000) == 0) {
									*linew = *cbline;
								}
								++linew;
								++cbline;
								--c3;
							}
							--c2;
							lineptr += 2 * (bufwidth >> 1);
						}
						buffer += 8;
						--count;
					}
				}
				break;
			}
		}
		if (buffer == last_buf_pos) {
			buffer = &row_ptr[blocks_per_rowa];
			row_ptr = buffer;
			last_buf_pos = &buffer[last_pos];
		}
	}
}


void __cdecl UnVQ2_4x2_Table(uint8_t * codebook, uint8_t * pointers, uint8_t * buffer, size_t blocksperrow, size_t numrows, size_t bufwidth)
{
	assert(HicolorTable != 0);
	assert(codebook != 0);
	assert(pointers != 0);
	assert(buffer != 0);
	assert(blocksperrow != 0);
	assert(numrows != 0);
	assert(bufwidth != 0);

	bufwidth = 2 * bufwidth;
	unsigned char * data_end = &buffer[4 * numrows * bufwidth];
	unsigned char * last_buf_pos = &buffer[8 * blocksperrow];
	unsigned short * ptrs = (unsigned short *)pointers;
	unsigned char * row_ptr = buffer;

	while (buffer < data_end) {
		unsigned short valw = *ptrs++;
		unsigned int val = valw;
		unsigned int count = val & 0xFFFF0FFF;
		int code = val & 0xF000;
		if (code <= 0x3000) {
			if (code != 0x3000) {
				if (code) {
					if (code != 0x1000) {
						if (code == 0x2000) {
							unsigned int len = 2;
							unsigned short * cbptr = (unsigned short *)&codebook[32 * *ptrs++];
							while (len > 0) {
								unsigned int w0 = cbptr[0];
								unsigned int w1 = cbptr[1];
								unsigned int value = w0 | (w1 << 16);
								cbptr += 2;
								unsigned char * dptr = buffer;
								unsigned int rows = count;
								while (rows > 0) {
									*(unsigned int *)dptr = value;
									dptr += 8;
									--rows;
								}
								w0 = cbptr[0];
								w1 = cbptr[1];
								value = w0 | (w1 << 16);
								cbptr += 6;
								unsigned char * dptr2 = buffer + 4;
								unsigned int c2 = count;
								while (c2 > 0) {
									*(unsigned int *)dptr2 = value;
									dptr2 += 8;
									--c2;
								}
								buffer += 2 * bufwidth;
								--len;
							}
							buffer += 8 * count - (int)(bufwidth << 2);
						}
					} else {
						buffer += 8 * count;
					}
				} else {
					int len = 2 * count;
					unsigned short pixel = HicolorTable[*ptrs++];
					int value = pixel | (pixel << 16);
					if (2 * count) {
						memset32(buffer, value, len);
					}
					if (len) {
						memset32(&buffer[2 * bufwidth], value, len);
					}
					buffer += 8 * count;
				}
			} else {
				unsigned int len = 2;
				unsigned short * cbsrc = (unsigned short *)&codebook[32 * *ptrs++];
				while (len > 0) {
					unsigned short * dstw = (unsigned short *)buffer;
					unsigned int c2 = 4;
					while (c2 > 0) {
						unsigned short cbpixel = *cbsrc;
						if ((cbpixel & 0x8000) == 0) {
							unsigned short * dstwalk = dstw;
							unsigned int c3 = count;
							while (c3 > 0) {
								*dstwalk = cbpixel;
								dstwalk += 4;
								--c3;
							}
						}
						++dstw;
						++cbsrc;
						--c2;
					}
					cbsrc += 4;
					buffer += 2 * bufwidth;
					--len;
				}

				buffer += 8 * count - (int)(bufwidth << 2);
			}
		} else {
			switch (code) {
			case 0x5000:
				if (count > 0) {
					int fillwidth = 4 * (bufwidth >> 1);
					while (count > 0) {
						unsigned int c2 = 2;
						int fillpix = HicolorTable[*ptrs] | (HicolorTable[*ptrs] << 16);
						++ptrs;
						unsigned char * fillptr = buffer;
						while (c2 > 0) {
							*(unsigned int *)fillptr = fillpix;
							*((unsigned int *)fillptr + 1) = fillpix;
							fillptr += fillwidth;
							--c2;
						}
						buffer += 8;
						--count;
					};
				}
				break;
			case 0x6000:
				if (count > 0) {
					int rowwidth = 4 * (bufwidth >> 1);
					while (count > 0) {
						unsigned char * rowptr = buffer;
						unsigned short * cbrow = (unsigned short *)&codebook[32 * *ptrs++];
						unsigned int c2 = 2;
						while (c2 > 0) {
							unsigned short hi0 = *((unsigned short *)cbrow + 1);
							unsigned short lo0 = *(unsigned short *)cbrow;
							unsigned short * cbnext = cbrow + 2;
							*(unsigned int *)rowptr = lo0 | (hi0 << 16);
							unsigned short hi1 = cbnext[1];
							unsigned short lo1 = *cbnext;
							cbrow = cbnext + 6;
							*((unsigned int *)rowptr + 1) = lo1 | (hi1 << 16);
							rowptr += rowwidth;
							--c2;
						}
						buffer += 8;
						--count;
					};
				}
				break;
			case 0x7000:
				if (count > 0) {
					while (count > 0) {
						int c2 = 2;
						unsigned short * cbline = (unsigned short *)&codebook[32 * *ptrs++];
						unsigned char * lineptr = buffer;
						while (c2) {
							unsigned short * linew = (unsigned short *)lineptr;
							unsigned int c3 = 4;
							while (c3 > 0) {
								if ((*cbline & 0x8000) == 0) {
									*linew = *cbline;
								}
								++linew;
								++cbline;
								--c3;
							}
							--c2;
							lineptr += 2 * bufwidth;
						}
						buffer += 8;
						--count;
					}
				}
				break;
			}
		}
		if (buffer == last_buf_pos) {
			buffer = &row_ptr[bufwidth << 2];
			row_ptr = buffer;
			last_buf_pos = &buffer[8 * blocksperrow];
		}
	}
}

#define __int8	char
#define __int16 short
#define __int32 long

void __cdecl UnVQ1_4x4_Table(uint8_t * codebook, uint8_t * pointers, uint8_t * buffer, size_t blocksperrow, size_t numrows, size_t bufwidth)
{
	assert(codebook != 0);
	assert(pointers != 0);
	assert(buffer != 0);
	assert(blocksperrow != 0);
	assert(numrows != 0);
	assert(bufwidth != 0);

	bufwidth = 2 * bufwidth;
	unsigned int total = numrows * bufwidth;
	unsigned long step = 8 * blocksperrow;
	unsigned char * data_end = buffer + (total * 4);
	unsigned char * row_end = buffer + step;
	unsigned char * row_ptr = buffer;
	unsigned short * src = (unsigned short *)pointers;

	unsigned long i;
	unsigned long j;
	unsigned short index;
	unsigned long type;

	unsigned long row_step = bufwidth << 2;

	while (buffer < data_end) {
		type = (*src & 0xE000);
		index = (*src & 0x1FFF);
		src++;

		switch (type) {
		case 0: {
			unsigned short * cb = (unsigned short *)&codebook[32 * index];

			unsigned int w0;
			unsigned int w1;
			unsigned int w2;
			unsigned int w3;
			for (i = 0; i != 2; i++) {
				w0 = cb[0];
				w1 = cb[1];
				((unsigned int *)buffer)[0] = w0 | (w1 << 16);
				cb += 2;
				w2 = cb[0];
				w3 = cb[1];
				((unsigned int *)buffer)[1] = w2 | (w3 << 16);
				cb += 2;

				cb += 4;
				buffer += 2 * bufwidth;
			}
			buffer += 8 - (int)row_step;
			break;
		}

		case 0x2000: {
			unsigned short * cb = (unsigned short *)&codebook[32 * index];
			for (i = 0; i != 2; i++) {
				unsigned short * dstw = (unsigned short *)buffer;
				for (j = 0; j != 4; j++) {
					unsigned short cbword = cb[0];
					if ((cbword & 0x8000) == 0) {
						dstw[0] = cbword;
					}
					dstw++;
					cb++;
				}
				cb += 4;
				buffer += (2 * bufwidth);
			}
			buffer += 8 - (int)row_step;
			break;
		}

		case 0x4000:
			buffer += 8;
			break;
		}

		if (buffer == row_end) {
			buffer = row_ptr + row_step;
			row_ptr = buffer;
			row_end = buffer + (blocksperrow << 3);
		}
	}
}


void __cdecl UnVQ1_4x2_Table(uint8_t * codebook, uint8_t * pointers, uint8_t * buffer, size_t blocksperrow, size_t numrows, size_t bufwidth)
{
	assert(codebook != 0);
	assert(pointers != 0);
	assert(buffer != 0);
	assert(blocksperrow != 0);
	assert(numrows != 0);
	assert(bufwidth != 0);

	bufwidth = 2 * bufwidth;
	unsigned char * data_end = buffer + (numrows * bufwidth * 4);
	unsigned char * last_buf_pos = buffer + 8 * blocksperrow;
	unsigned char * row_ptr = buffer;
	unsigned short * src = (unsigned short *)pointers;

	unsigned long i;
	unsigned short index;
	unsigned long type;
	unsigned int cb_idx;
	unsigned int scatter_count;
	unsigned int code;

	while (buffer < data_end) {
		type = (*src & 0xE000);
		index = (*src & 0x1FFF);
		src++;

		if (type <= 0x6000) {
			if (type != 0x6000) {
				if (type) {
					if (type != 0x2000) {
						if (type == 0x4000) {
							unsigned char primary_idx = *((unsigned char *)src - 2);
							unsigned int count = (((unsigned short)index >> 7) & 0x3E) + 2;
							unsigned short * cb = (unsigned short *)&codebook[32 * primary_idx];
							unsigned int stride = bufwidth >> 2;
							unsigned int w0;
							unsigned int w1;
							unsigned int w2;
							unsigned int w3;
							unsigned char * dstb = buffer;
							int n;
							for (n = 0; n < 2; n++) {
								w0 = cb[0];
								w1 = cb[1];
								cb += 2;
								((unsigned int *)dstb)[0] = w0 | (w1 << 16);
								w2 = cb[0];
								w3 = cb[1];
								cb += 6;
								((unsigned int *)dstb)[1] = w2 | (w3 << 16);
								dstb += 4 * stride;
							}
							buffer += 8;
							unsigned char * byte_src = (unsigned char *)src;

							unsigned int full_stride = 8 * stride;
							unsigned int remaining = count;
							do {
								unsigned char * d2 = buffer;
								unsigned char * next_byte = byte_src + 1;
								unsigned short * cb2 = (unsigned short *)&codebook[32 * *byte_src];
								int n2;
								for (n2 = 0; n2 < 2; n2++) {
									w0 = cb2[0];
									w1 = cb2[1];
									cb2 += 2;
									((unsigned int *)d2)[0] = w0 | (w1 << 16);
									w2 = cb2[0];
									w3 = cb2[1];
									cb2 += 6;
									((unsigned int *)d2)[1] = w2 | (w3 << 16);
									d2 += full_stride;
								}
								byte_src = next_byte;
								buffer += 8;
								--remaining;
							} while (remaining);
							src = (unsigned short *)byte_src;
						}
					} else {
						code = index;
						code &= 0xFFFF;
						cb_idx = code & 0xFF;
						scatter_count = ((code >> 7) & 0x3E) + 2;
						if (scatter_count == 0) {
							goto label_extended_count;
						}

					label_scatter_copy: {
						unsigned int len = 2;
						unsigned short * cb = (unsigned short *)&codebook[32 * cb_idx];
						do {
							unsigned int w0 = cb[0];
							unsigned int w1 = cb[1];
							unsigned int value = w0 | (w1 << 16);
							cb += 2;
							unsigned char * dstb = buffer;
							if (scatter_count > 0) {
								unsigned int remain = scatter_count;
								do {
									*(unsigned int *)dstb = value;
									dstb += 8;
									--remain;
								} while (remain);
							}
							unsigned int w2 = cb[0];
							unsigned int w3 = cb[1];
							value = w2 | (w3 << 16);
							cb += 6;
							unsigned char * d2 = buffer + 4;
							if (scatter_count > 0) {
								unsigned int remain = scatter_count;
								do {
									*(unsigned int *)d2 = value;
									d2 += 8;
									--remain;
								} while (remain);
							}
							buffer += 2 * bufwidth;
							--len;
						} while (len);
						buffer += 8 * scatter_count - (int)(bufwidth << 2);
					}
					}
				} else {
					buffer += 8 * (unsigned char)index;
				}
			} else {
				unsigned short * cb = (unsigned short *)&codebook[32 * index];
				unsigned int w0;
				unsigned int w1;
				unsigned int w2;
				unsigned int w3;
				for (i = 0; i != 4; i++) {
					w0 = cb[0];
					w1 = cb[1];
					((unsigned int *)buffer)[0] = w0 | (w1 << 16);
					cb += 2;
					w2 = cb[0];
					w3 = cb[1];
					((unsigned int *)buffer)[1] = w2 | (w3 << 16);
					cb += 6;
					buffer += 2 * bufwidth;
				}
				buffer += 8 - (int)(bufwidth << 2);
			}
		} else {
			switch (type) {
			case 0x8000: {
				unsigned short * cb = (unsigned short *)&codebook[32 * index];
				int len = 4;
				do {
					unsigned short * dstw = (unsigned short *)buffer;
					int c2 = 4;
					do {
						if ((*cb & 0x8000) == 0) {
							*dstw = *cb;
						}
						dstw++;
						cb++;
						--c2;
					} while (c2);
					cb += 4;
					buffer += 2 * bufwidth;
					--len;
				} while (len);
				buffer += 8 - (int)(bufwidth << 2);
				break;
			}

			case 0xA000: {
			label_extended_count:
				scatter_count = *(unsigned char *)src;
				cb_idx = (unsigned short)index;
				src = (unsigned short *)((unsigned char *)src + 1);
				goto label_scatter_copy;
			}

			case 0xC000: {
				unsigned int count = *(unsigned char *)src;
				src = (unsigned short *)((unsigned char *)src + 1);
				unsigned short * cb = (unsigned short *)&codebook[32 * index];
				unsigned int len;
				for (len = 0; len < 2; len++) {
					unsigned short * dstw = (unsigned short *)buffer;
					unsigned int c2;
					for (c2 = 0; c2 < 4; c2++) {
						unsigned short cbval = *cb;
						if ((*cb & 0x8000) == 0) {
							unsigned short * d2 = dstw;
							unsigned int c3;
							for (c3 = 0; c3 < count; c3++) {
								*d2 = cbval;
								d2 += 4;
							}
						}
						dstw++;
						cb++;
					}
					cb += 4;
					buffer += 2 * bufwidth;
				}

				buffer += 8 * count - (int)(bufwidth << 2);
				break;
			}
			}
		}
		if (buffer == last_buf_pos) {
			buffer = row_ptr + (bufwidth << 2);
			row_ptr = buffer;
			last_buf_pos = buffer + 8 * blocksperrow;
		}
	}
}
