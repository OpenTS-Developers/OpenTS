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
*  File              : soscodec.asm
*  Date Created      : 6/1/94
*  Description       : ADPCM decompression for the SOS audio streams.
*
*  Programmer(s)     : Nick Skrepetos
*  Last Modification : 10/1/94 - 11:37:9 AM
*  Additional Notes  : Modified by Denzil E. Long, Jr.
*
*****************************************************************************
*            Copyright (c) 1994,  HMI, Inc.  All Rights Reserved            *
****************************************************************************/

#include "always.h"

#include "soscomp.h"
#include "vqalib/cmp.h"

/*
 * Three decoders share this file, each replacing an assembly routine of the same name.
 *
 * sosCODECDecompressData is the fast path for 16 bit mono. It walks a difference table
 * indexed by the step index and the token together, and its stream state is that combined
 * index rather than a plain step index: wIndex holds the step index times 32, which is the
 * form the table wants and the form the assembly persisted between calls.
 *
 * General_sosCODECDecompressData handles every other shape. It computes the difference
 * arithmetically from the current step instead of reading a table, which is the same
 * arithmetic the table was built from.
 *
 * VQA_sosCODECDecompressData is the VQA flavour. It takes its shape through arguments,
 * carries only four values of state, and implements 16 bit only; anything else returns
 * having done nothing, exactly as the assembly did.
 */

namespace {

/*
 * Step index adjustment per token, and the step size per index. These belong to the general
 * decoder; the other two reach the same numbers through the difference table below.
 */
short const _SosIndexAdjust[16] = {
	-1, -1, -1, -1, 2, 4, 6, 8,
	-1, -1, -1, -1, 2, 4, 6, 8
};

unsigned short const _SosStepTable[89] = {
	7, 8, 9, 10, 11, 12, 13, 14,
	16, 17, 19, 21, 23, 25, 28, 31,
	34, 37, 41, 45, 50, 55, 60, 66,
	73, 80, 88, 97, 107, 118, 130, 143,
	157, 173, 190, 209, 230, 253, 279, 307,
	337, 371, 408, 449, 494, 544, 598, 658,
	724, 796, 876, 963, 1060, 1166, 1282, 1411,
	1552, 1707, 1878, 2066, 2272, 2499, 2749, 3024,
	3327, 3660, 4026, 4428, 4871, 5358, 5894, 6484,
	7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899,
	15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794,
	32767
};

/*
 * Difference to add to the running sample, and the next step index scaled by 32, both keyed
 * on the step index and token together. Transcribed from the tables the assembly carried.
 */
int const _SosDiffTable[89 * 16] = {
	0, 1, 3, 4, 7, 8, 10, 11,
	0, -1, -3, -4, -7, -8, -10, -11,
	1, 3, 5, 7, 9, 11, 13, 15,
	-1, -3, -5, -7, -9, -11, -13, -15,
	1, 3, 5, 7, 10, 12, 14, 16,
	-1, -3, -5, -7, -10, -12, -14, -16,
	1, 3, 6, 8, 11, 13, 16, 18,
	-1, -3, -6, -8, -11, -13, -16, -18,
	1, 3, 6, 8, 12, 14, 17, 19,
	-1, -3, -6, -8, -12, -14, -17, -19,
	1, 4, 7, 10, 13, 16, 19, 22,
	-1, -4, -7, -10, -13, -16, -19, -22,
	1, 4, 7, 10, 14, 17, 20, 23,
	-1, -4, -7, -10, -14, -17, -20, -23,
	1, 4, 8, 11, 15, 18, 22, 25,
	-1, -4, -8, -11, -15, -18, -22, -25,
	2, 6, 10, 14, 18, 22, 26, 30,
	-2, -6, -10, -14, -18, -22, -26, -30,
	2, 6, 10, 14, 19, 23, 27, 31,
	-2, -6, -10, -14, -19, -23, -27, -31,
	2, 6, 11, 15, 21, 25, 30, 34,
	-2, -6, -11, -15, -21, -25, -30, -34,
	2, 7, 12, 17, 23, 28, 33, 38,
	-2, -7, -12, -17, -23, -28, -33, -38,
	2, 7, 13, 18, 25, 30, 36, 41,
	-2, -7, -13, -18, -25, -30, -36, -41,
	3, 9, 15, 21, 28, 34, 40, 46,
	-3, -9, -15, -21, -28, -34, -40, -46,
	3, 10, 17, 24, 31, 38, 45, 52,
	-3, -10, -17, -24, -31, -38, -45, -52,
	3, 10, 18, 25, 34, 41, 49, 56,
	-3, -10, -18, -25, -34, -41, -49, -56,
	4, 12, 21, 29, 38, 46, 55, 63,
	-4, -12, -21, -29, -38, -46, -55, -63,
	4, 13, 22, 31, 41, 50, 59, 68,
	-4, -13, -22, -31, -41, -50, -59, -68,
	5, 15, 25, 35, 46, 56, 66, 76,
	-5, -15, -25, -35, -46, -56, -66, -76,
	5, 16, 27, 38, 50, 61, 72, 83,
	-5, -16, -27, -38, -50, -61, -72, -83,
	6, 18, 31, 43, 56, 68, 81, 93,
	-6, -18, -31, -43, -56, -68, -81, -93,
	6, 19, 33, 46, 61, 74, 88, 101,
	-6, -19, -33, -46, -61, -74, -88, -101,
	7, 22, 37, 52, 67, 82, 97, 112,
	-7, -22, -37, -52, -67, -82, -97, -112,
	8, 24, 41, 57, 74, 90, 107, 123,
	-8, -24, -41, -57, -74, -90, -107, -123,
	9, 27, 45, 63, 82, 100, 118, 136,
	-9, -27, -45, -63, -82, -100, -118, -136,
	10, 30, 50, 70, 90, 110, 130, 150,
	-10, -30, -50, -70, -90, -110, -130, -150,
	11, 33, 55, 77, 99, 121, 143, 165,
	-11, -33, -55, -77, -99, -121, -143, -165,
	12, 36, 60, 84, 109, 133, 157, 181,
	-12, -36, -60, -84, -109, -133, -157, -181,
	13, 39, 66, 92, 120, 146, 173, 199,
	-13, -39, -66, -92, -120, -146, -173, -199,
	14, 43, 73, 102, 132, 161, 191, 220,
	-14, -43, -73, -102, -132, -161, -191, -220,
	16, 48, 81, 113, 146, 178, 211, 243,
	-16, -48, -81, -113, -146, -178, -211, -243,
	17, 52, 88, 123, 160, 195, 231, 266,
	-17, -52, -88, -123, -160, -195, -231, -266,
	19, 58, 97, 136, 176, 215, 254, 293,
	-19, -58, -97, -136, -176, -215, -254, -293,
	21, 64, 107, 150, 194, 237, 280, 323,
	-21, -64, -107, -150, -194, -237, -280, -323,
	23, 70, 118, 165, 213, 260, 308, 355,
	-23, -70, -118, -165, -213, -260, -308, -355,
	26, 78, 130, 182, 235, 287, 339, 391,
	-26, -78, -130, -182, -235, -287, -339, -391,
	28, 85, 143, 200, 258, 315, 373, 430,
	-28, -85, -143, -200, -258, -315, -373, -430,
	31, 94, 157, 220, 284, 347, 410, 473,
	-31, -94, -157, -220, -284, -347, -410, -473,
	34, 103, 173, 242, 313, 382, 452, 521,
	-34, -103, -173, -242, -313, -382, -452, -521,
	38, 114, 191, 267, 345, 421, 498, 574,
	-38, -114, -191, -267, -345, -421, -498, -574,
	42, 126, 210, 294, 379, 463, 547, 631,
	-42, -126, -210, -294, -379, -463, -547, -631,
	46, 138, 231, 323, 417, 509, 602, 694,
	-46, -138, -231, -323, -417, -509, -602, -694,
	51, 153, 255, 357, 459, 561, 663, 765,
	-51, -153, -255, -357, -459, -561, -663, -765,
	56, 168, 280, 392, 505, 617, 729, 841,
	-56, -168, -280, -392, -505, -617, -729, -841,
	61, 184, 308, 431, 555, 678, 802, 925,
	-61, -184, -308, -431, -555, -678, -802, -925,
	68, 204, 340, 476, 612, 748, 884, 1020,
	-68, -204, -340, -476, -612, -748, -884, -1020,
	74, 223, 373, 522, 672, 821, 971, 1120,
	-74, -223, -373, -522, -672, -821, -971, -1120,
	82, 246, 411, 575, 740, 904, 1069, 1233,
	-82, -246, -411, -575, -740, -904, -1069, -1233,
	90, 271, 452, 633, 814, 995, 1176, 1357,
	-90, -271, -452, -633, -814, -995, -1176, -1357,
	99, 298, 497, 696, 895, 1094, 1293, 1492,
	-99, -298, -497, -696, -895, -1094, -1293, -1492,
	109, 328, 547, 766, 985, 1204, 1423, 1642,
	-109, -328, -547, -766, -985, -1204, -1423, -1642,
	120, 360, 601, 841, 1083, 1323, 1564, 1804,
	-120, -360, -601, -841, -1083, -1323, -1564, -1804,
	132, 397, 662, 927, 1192, 1457, 1722, 1987,
	-132, -397, -662, -927, -1192, -1457, -1722, -1987,
	145, 436, 728, 1019, 1311, 1602, 1894, 2185,
	-145, -436, -728, -1019, -1311, -1602, -1894, -2185,
	160, 480, 801, 1121, 1442, 1762, 2083, 2403,
	-160, -480, -801, -1121, -1442, -1762, -2083, -2403,
	176, 528, 881, 1233, 1587, 1939, 2292, 2644,
	-176, -528, -881, -1233, -1587, -1939, -2292, -2644,
	194, 582, 970, 1358, 1746, 2134, 2522, 2910,
	-194, -582, -970, -1358, -1746, -2134, -2522, -2910,
	213, 639, 1066, 1492, 1920, 2346, 2773, 3199,
	-213, -639, -1066, -1492, -1920, -2346, -2773, -3199,
	234, 703, 1173, 1642, 2112, 2581, 3051, 3520,
	-234, -703, -1173, -1642, -2112, -2581, -3051, -3520,
	258, 774, 1291, 1807, 2324, 2840, 3357, 3873,
	-258, -774, -1291, -1807, -2324, -2840, -3357, -3873,
	284, 852, 1420, 1988, 2556, 3124, 3692, 4260,
	-284, -852, -1420, -1988, -2556, -3124, -3692, -4260,
	312, 936, 1561, 2185, 2811, 3435, 4060, 4684,
	-312, -936, -1561, -2185, -2811, -3435, -4060, -4684,
	343, 1030, 1717, 2404, 3092, 3779, 4466, 5153,
	-343, -1030, -1717, -2404, -3092, -3779, -4466, -5153,
	378, 1134, 1890, 2646, 3402, 4158, 4914, 5670,
	-378, -1134, -1890, -2646, -3402, -4158, -4914, -5670,
	415, 1246, 2078, 2909, 3742, 4573, 5405, 6236,
	-415, -1246, -2078, -2909, -3742, -4573, -5405, -6236,
	457, 1372, 2287, 3202, 4117, 5032, 5947, 6862,
	-457, -1372, -2287, -3202, -4117, -5032, -5947, -6862,
	503, 1509, 2516, 3522, 4529, 5535, 6542, 7548,
	-503, -1509, -2516, -3522, -4529, -5535, -6542, -7548,
	553, 1660, 2767, 3874, 4981, 6088, 7195, 8302,
	-553, -1660, -2767, -3874, -4981, -6088, -7195, -8302,
	608, 1825, 3043, 4260, 5479, 6696, 7914, 9131,
	-608, -1825, -3043, -4260, -5479, -6696, -7914, -9131,
	669, 2008, 3348, 4687, 6027, 7366, 8706, 10045,
	-669, -2008, -3348, -4687, -6027, -7366, -8706, -10045,
	736, 2209, 3683, 5156, 6630, 8103, 9577, 11050,
	-736, -2209, -3683, -5156, -6630, -8103, -9577, -11050,
	810, 2431, 4052, 5673, 7294, 8915, 10536, 12157,
	-810, -2431, -4052, -5673, -7294, -8915, -10536, -12157,
	891, 2674, 4457, 6240, 8023, 9806, 11589, 13372,
	-891, -2674, -4457, -6240, -8023, -9806, -11589, -13372,
	980, 2941, 4902, 6863, 8825, 10786, 12747, 14708,
	-980, -2941, -4902, -6863, -8825, -10786, -12747, -14708,
	1078, 3235, 5393, 7550, 9708, 11865, 14023, 16180,
	-1078, -3235, -5393, -7550, -9708, -11865, -14023, -16180,
	1186, 3559, 5932, 8305, 10679, 13052, 15425, 17798,
	-1186, -3559, -5932, -8305, -10679, -13052, -15425, -17798,
	1305, 3915, 6526, 9136, 11747, 14357, 16968, 19578,
	-1305, -3915, -6526, -9136, -11747, -14357, -16968, -19578,
	1435, 4306, 7178, 10049, 12922, 15793, 18665, 21536,
	-1435, -4306, -7178, -10049, -12922, -15793, -18665, -21536,
	1579, 4737, 7896, 11054, 14214, 17372, 20531, 23689,
	-1579, -4737, -7896, -11054, -14214, -17372, -20531, -23689,
	1737, 5211, 8686, 12160, 15636, 19110, 22585, 26059,
	-1737, -5211, -8686, -12160, -15636, -19110, -22585, -26059,
	1911, 5733, 9555, 13377, 17200, 21022, 24844, 28666,
	-1911, -5733, -9555, -13377, -17200, -21022, -24844, -28666,
	2102, 6306, 10511, 14715, 18920, 23124, 27329, 31533,
	-2102, -6306, -10511, -14715, -18920, -23124, -27329, -31533,
	2312, 6937, 11562, 16187, 20812, 25437, 30062, 34687,
	-2312, -6937, -11562, -16187, -20812, -25437, -30062, -34687,
	2543, 7630, 12718, 17805, 22893, 27980, 33068, 38155,
	-2543, -7630, -12718, -17805, -22893, -27980, -33068, -38155,
	2798, 8394, 13990, 19586, 25183, 30779, 36375, 41971,
	-2798, -8394, -13990, -19586, -25183, -30779, -36375, -41971,
	3077, 9232, 15388, 21543, 27700, 33855, 40011, 46166,
	-3077, -9232, -15388, -21543, -27700, -33855, -40011, -46166,
	3385, 10156, 16928, 23699, 30471, 37242, 44014, 50785,
	-3385, -10156, -16928, -23699, -30471, -37242, -44014, -50785,
	3724, 11172, 18621, 26069, 33518, 40966, 48415, 55863,
	-3724, -11172, -18621, -26069, -33518, -40966, -48415, -55863,
	4095, 12286, 20478, 28669, 36862, 45053, 53245, 61436,
	-4095, -12286, -20478, -28669, -36862, -45053, -53245, -61436

};

unsigned short const _SosIndexTable[89 * 16] = {
	0, 0, 0, 0, 64, 128, 192, 256,
	0, 0, 0, 0, 64, 128, 192, 256,
	0, 0, 0, 0, 96, 160, 224, 288,
	0, 0, 0, 0, 96, 160, 224, 288,
	32, 32, 32, 32, 128, 192, 256, 320,
	32, 32, 32, 32, 128, 192, 256, 320,
	64, 64, 64, 64, 160, 224, 288, 352,
	64, 64, 64, 64, 160, 224, 288, 352,
	96, 96, 96, 96, 192, 256, 320, 384,
	96, 96, 96, 96, 192, 256, 320, 384,
	128, 128, 128, 128, 224, 288, 352, 416,
	128, 128, 128, 128, 224, 288, 352, 416,
	160, 160, 160, 160, 256, 320, 384, 448,
	160, 160, 160, 160, 256, 320, 384, 448,
	192, 192, 192, 192, 288, 352, 416, 480,
	192, 192, 192, 192, 288, 352, 416, 480,
	224, 224, 224, 224, 320, 384, 448, 512,
	224, 224, 224, 224, 320, 384, 448, 512,
	256, 256, 256, 256, 352, 416, 480, 544,
	256, 256, 256, 256, 352, 416, 480, 544,
	288, 288, 288, 288, 384, 448, 512, 576,
	288, 288, 288, 288, 384, 448, 512, 576,
	320, 320, 320, 320, 416, 480, 544, 608,
	320, 320, 320, 320, 416, 480, 544, 608,
	352, 352, 352, 352, 448, 512, 576, 640,
	352, 352, 352, 352, 448, 512, 576, 640,
	384, 384, 384, 384, 480, 544, 608, 672,
	384, 384, 384, 384, 480, 544, 608, 672,
	416, 416, 416, 416, 512, 576, 640, 704,
	416, 416, 416, 416, 512, 576, 640, 704,
	448, 448, 448, 448, 544, 608, 672, 736,
	448, 448, 448, 448, 544, 608, 672, 736,
	480, 480, 480, 480, 576, 640, 704, 768,
	480, 480, 480, 480, 576, 640, 704, 768,
	512, 512, 512, 512, 608, 672, 736, 800,
	512, 512, 512, 512, 608, 672, 736, 800,
	544, 544, 544, 544, 640, 704, 768, 832,
	544, 544, 544, 544, 640, 704, 768, 832,
	576, 576, 576, 576, 672, 736, 800, 864,
	576, 576, 576, 576, 672, 736, 800, 864,
	608, 608, 608, 608, 704, 768, 832, 896,
	608, 608, 608, 608, 704, 768, 832, 896,
	640, 640, 640, 640, 736, 800, 864, 928,
	640, 640, 640, 640, 736, 800, 864, 928,
	672, 672, 672, 672, 768, 832, 896, 960,
	672, 672, 672, 672, 768, 832, 896, 960,
	704, 704, 704, 704, 800, 864, 928, 992,
	704, 704, 704, 704, 800, 864, 928, 992,
	736, 736, 736, 736, 832, 896, 960, 1024,
	736, 736, 736, 736, 832, 896, 960, 1024,
	768, 768, 768, 768, 864, 928, 992, 1056,
	768, 768, 768, 768, 864, 928, 992, 1056,
	800, 800, 800, 800, 896, 960, 1024, 1088,
	800, 800, 800, 800, 896, 960, 1024, 1088,
	832, 832, 832, 832, 928, 992, 1056, 1120,
	832, 832, 832, 832, 928, 992, 1056, 1120,
	864, 864, 864, 864, 960, 1024, 1088, 1152,
	864, 864, 864, 864, 960, 1024, 1088, 1152,
	896, 896, 896, 896, 992, 1056, 1120, 1184,
	896, 896, 896, 896, 992, 1056, 1120, 1184,
	928, 928, 928, 928, 1024, 1088, 1152, 1216,
	928, 928, 928, 928, 1024, 1088, 1152, 1216,
	960, 960, 960, 960, 1056, 1120, 1184, 1248,
	960, 960, 960, 960, 1056, 1120, 1184, 1248,
	992, 992, 992, 992, 1088, 1152, 1216, 1280,
	992, 992, 992, 992, 1088, 1152, 1216, 1280,
	1024, 1024, 1024, 1024, 1120, 1184, 1248, 1312,
	1024, 1024, 1024, 1024, 1120, 1184, 1248, 1312,
	1056, 1056, 1056, 1056, 1152, 1216, 1280, 1344,
	1056, 1056, 1056, 1056, 1152, 1216, 1280, 1344,
	1088, 1088, 1088, 1088, 1184, 1248, 1312, 1376,
	1088, 1088, 1088, 1088, 1184, 1248, 1312, 1376,
	1120, 1120, 1120, 1120, 1216, 1280, 1344, 1408,
	1120, 1120, 1120, 1120, 1216, 1280, 1344, 1408,
	1152, 1152, 1152, 1152, 1248, 1312, 1376, 1440,
	1152, 1152, 1152, 1152, 1248, 1312, 1376, 1440,
	1184, 1184, 1184, 1184, 1280, 1344, 1408, 1472,
	1184, 1184, 1184, 1184, 1280, 1344, 1408, 1472,
	1216, 1216, 1216, 1216, 1312, 1376, 1440, 1504,
	1216, 1216, 1216, 1216, 1312, 1376, 1440, 1504,
	1248, 1248, 1248, 1248, 1344, 1408, 1472, 1536,
	1248, 1248, 1248, 1248, 1344, 1408, 1472, 1536,
	1280, 1280, 1280, 1280, 1376, 1440, 1504, 1568,
	1280, 1280, 1280, 1280, 1376, 1440, 1504, 1568,
	1312, 1312, 1312, 1312, 1408, 1472, 1536, 1600,
	1312, 1312, 1312, 1312, 1408, 1472, 1536, 1600,
	1344, 1344, 1344, 1344, 1440, 1504, 1568, 1632,
	1344, 1344, 1344, 1344, 1440, 1504, 1568, 1632,
	1376, 1376, 1376, 1376, 1472, 1536, 1600, 1664,
	1376, 1376, 1376, 1376, 1472, 1536, 1600, 1664,
	1408, 1408, 1408, 1408, 1504, 1568, 1632, 1696,
	1408, 1408, 1408, 1408, 1504, 1568, 1632, 1696,
	1440, 1440, 1440, 1440, 1536, 1600, 1664, 1728,
	1440, 1440, 1440, 1440, 1536, 1600, 1664, 1728,
	1472, 1472, 1472, 1472, 1568, 1632, 1696, 1760,
	1472, 1472, 1472, 1472, 1568, 1632, 1696, 1760,
	1504, 1504, 1504, 1504, 1600, 1664, 1728, 1792,
	1504, 1504, 1504, 1504, 1600, 1664, 1728, 1792,
	1536, 1536, 1536, 1536, 1632, 1696, 1760, 1824,
	1536, 1536, 1536, 1536, 1632, 1696, 1760, 1824,
	1568, 1568, 1568, 1568, 1664, 1728, 1792, 1856,
	1568, 1568, 1568, 1568, 1664, 1728, 1792, 1856,
	1600, 1600, 1600, 1600, 1696, 1760, 1824, 1888,
	1600, 1600, 1600, 1600, 1696, 1760, 1824, 1888,
	1632, 1632, 1632, 1632, 1728, 1792, 1856, 1920,
	1632, 1632, 1632, 1632, 1728, 1792, 1856, 1920,
	1664, 1664, 1664, 1664, 1760, 1824, 1888, 1952,
	1664, 1664, 1664, 1664, 1760, 1824, 1888, 1952,
	1696, 1696, 1696, 1696, 1792, 1856, 1920, 1984,
	1696, 1696, 1696, 1696, 1792, 1856, 1920, 1984,
	1728, 1728, 1728, 1728, 1824, 1888, 1952, 2016,
	1728, 1728, 1728, 1728, 1824, 1888, 1952, 2016,
	1760, 1760, 1760, 1760, 1856, 1920, 1984, 2048,
	1760, 1760, 1760, 1760, 1856, 1920, 1984, 2048,
	1792, 1792, 1792, 1792, 1888, 1952, 2016, 2080,
	1792, 1792, 1792, 1792, 1888, 1952, 2016, 2080,
	1824, 1824, 1824, 1824, 1920, 1984, 2048, 2112,
	1824, 1824, 1824, 1824, 1920, 1984, 2048, 2112,
	1856, 1856, 1856, 1856, 1952, 2016, 2080, 2144,
	1856, 1856, 1856, 1856, 1952, 2016, 2080, 2144,
	1888, 1888, 1888, 1888, 1984, 2048, 2112, 2176,
	1888, 1888, 1888, 1888, 1984, 2048, 2112, 2176,
	1920, 1920, 1920, 1920, 2016, 2080, 2144, 2208,
	1920, 1920, 1920, 1920, 2016, 2080, 2144, 2208,
	1952, 1952, 1952, 1952, 2048, 2112, 2176, 2240,
	1952, 1952, 1952, 1952, 2048, 2112, 2176, 2240,
	1984, 1984, 1984, 1984, 2080, 2144, 2208, 2272,
	1984, 1984, 1984, 1984, 2080, 2144, 2208, 2272,
	2016, 2016, 2016, 2016, 2112, 2176, 2240, 2304,
	2016, 2016, 2016, 2016, 2112, 2176, 2240, 2304,
	2048, 2048, 2048, 2048, 2144, 2208, 2272, 2336,
	2048, 2048, 2048, 2048, 2144, 2208, 2272, 2336,
	2080, 2080, 2080, 2080, 2176, 2240, 2304, 2368,
	2080, 2080, 2080, 2080, 2176, 2240, 2304, 2368,
	2112, 2112, 2112, 2112, 2208, 2272, 2336, 2400,
	2112, 2112, 2112, 2112, 2208, 2272, 2336, 2400,
	2144, 2144, 2144, 2144, 2240, 2304, 2368, 2432,
	2144, 2144, 2144, 2144, 2240, 2304, 2368, 2432,
	2176, 2176, 2176, 2176, 2272, 2336, 2400, 2464,
	2176, 2176, 2176, 2176, 2272, 2336, 2400, 2464,
	2208, 2208, 2208, 2208, 2304, 2368, 2432, 2496,
	2208, 2208, 2208, 2208, 2304, 2368, 2432, 2496,
	2240, 2240, 2240, 2240, 2336, 2400, 2464, 2528,
	2240, 2240, 2240, 2240, 2336, 2400, 2464, 2528,
	2272, 2272, 2272, 2272, 2368, 2432, 2496, 2560,
	2272, 2272, 2272, 2272, 2368, 2432, 2496, 2560,
	2304, 2304, 2304, 2304, 2400, 2464, 2528, 2592,
	2304, 2304, 2304, 2304, 2400, 2464, 2528, 2592,
	2336, 2336, 2336, 2336, 2432, 2496, 2560, 2624,
	2336, 2336, 2336, 2336, 2432, 2496, 2560, 2624,
	2368, 2368, 2368, 2368, 2464, 2528, 2592, 2656,
	2368, 2368, 2368, 2368, 2464, 2528, 2592, 2656,
	2400, 2400, 2400, 2400, 2496, 2560, 2624, 2688,
	2400, 2400, 2400, 2400, 2496, 2560, 2624, 2688,
	2432, 2432, 2432, 2432, 2528, 2592, 2656, 2720,
	2432, 2432, 2432, 2432, 2528, 2592, 2656, 2720,
	2464, 2464, 2464, 2464, 2560, 2624, 2688, 2752,
	2464, 2464, 2464, 2464, 2560, 2624, 2688, 2752,
	2496, 2496, 2496, 2496, 2592, 2656, 2720, 2784,
	2496, 2496, 2496, 2496, 2592, 2656, 2720, 2784,
	2528, 2528, 2528, 2528, 2624, 2688, 2752, 2816,
	2528, 2528, 2528, 2528, 2624, 2688, 2752, 2816,
	2560, 2560, 2560, 2560, 2656, 2720, 2784, 2816,
	2560, 2560, 2560, 2560, 2656, 2720, 2784, 2816,
	2592, 2592, 2592, 2592, 2688, 2752, 2816, 2816,
	2592, 2592, 2592, 2592, 2688, 2752, 2816, 2816,
	2624, 2624, 2624, 2624, 2720, 2784, 2816, 2816,
	2624, 2624, 2624, 2624, 2720, 2784, 2816, 2816,
	2656, 2656, 2656, 2656, 2752, 2816, 2816, 2816,
	2656, 2656, 2656, 2656, 2752, 2816, 2816, 2816,
	2688, 2688, 2688, 2688, 2784, 2816, 2816, 2816,
	2688, 2688, 2688, 2688, 2784, 2816, 2816, 2816,
	2720, 2720, 2720, 2720, 2816, 2816, 2816, 2816,
	2720, 2720, 2720, 2720, 2816, 2816, 2816, 2816,
	2752, 2752, 2752, 2752, 2816, 2816, 2816, 2816,
	2752, 2752, 2752, 2752, 2816, 2816, 2816, 2816,
	2784, 2784, 2784, 2784, 2816, 2816, 2816, 2816,
	2784, 2784, 2784, 2784, 2816, 2816, 2816, 2816

};


/// <summary>
/// Holds a running sample inside the range a 16 bit sample can carry.
/// </summary>
/// <param name="sample">The sample to bring back into range.</param>
/// <returns>long; The sample, clamped.</returns>
inline long Clamp_Sample(long sample)
{
	if (sample > 32767) {
		return(32767);
	}

	if (sample < -32768) {
		return(-32768);
	}

	return(sample);
}


/// <summary>
/// Decodes 16 bit samples through the difference table, the form both the SOS fast path and
/// the VQA decoder use. Tokens come out of each source byte low half first.
/// </summary>
/// <param name="source">Compressed nybbles.</param>
/// <param name="dest">Where the samples go.</param>
/// <param name="samples">How many samples to produce.</param>
/// <param name="deststride">Distance in shorts between one sample and the next.</param>
/// <param name="predicted">Running sample, carried in and out.</param>
/// <param name="index">Step index times 32, carried in and out.</param>
void Decode_Table_16(unsigned char const * source, short * dest, int samples, int deststride, long & predicted, unsigned short & index)
{
	long sample = predicted;
	unsigned int slotbase = index;

	for (int i = 0; i < samples; i++) {
		unsigned int const byte = source[i >> 1];
		unsigned int const token = ((i & 1) == 0) ? (byte & 0x0F) : ((byte >> 4) & 0x0F);

		/*
		 * The index is a multiple of 32 and the token contributes at most 30, so the
		 * assembly's OR into the low byte is an addition that cannot carry.
		 */
		unsigned int const slot = (slotbase | (token * 2)) >> 1;

		sample += _SosDiffTable[slot];
		slotbase = _SosIndexTable[slot];
		sample = Clamp_Sample(sample);

		dest[i * deststride] = (short)sample;
	}

	predicted = sample;
	index = (unsigned short)slotbase;
}


/*
 * The general decoder keeps a separate copy of this state per channel, so the loop below
 * reaches it through pointers rather than naming the structure's fields twice.
 */
struct SosChannel {
	unsigned long * SampleIndex;
	short * CodeBuf;
	short * Code;
	long * Predicted;
	long * Difference;
	short * Index;
	short * Step;
};


/// <summary>
/// Decodes one channel the general way, computing each difference from the current step.
/// </summary>
/// <param name="channel">The stream state for this channel.</param>
/// <param name="source">Compressed nybbles.</param>
/// <param name="dest">Where the samples go.</param>
/// <param name="samples">How many samples to produce.</param>
/// <param name="sourcestride">Distance in bytes between one source byte and the next.</param>
/// <param name="deststride">Distance in bytes between one sample and the next.</param>
/// <param name="bits">8 or 16, the width of a written sample.</param>
void Decode_General(SosChannel const & channel, unsigned char const * source, unsigned char * dest, int samples, int sourcestride, int deststride, int bits)
{
	for (int i = 0; i < samples; i++) {

		/*
		 * A byte carries two tokens. Odd samples take the half already fetched, which is
		 * why the sample counter and the code buffer are stream state rather than locals.
		 */
		if ((*channel.SampleIndex & 1) != 0) {
			*channel.Code = (short)(((unsigned short)*channel.CodeBuf >> 4) & 0x0F);
		} else {
			*channel.CodeBuf = (short)(unsigned short)*source;
			source += sourcestride;
			*channel.Code = (short)(*channel.CodeBuf & 0x0F);
		}

		int const code = *channel.Code;
		long const step = (long)(unsigned short)*channel.Step;

		long difference = 0;

		if ((code & 4) != 0) {
			difference += step;
		}

		if ((code & 2) != 0) {
			difference += step >> 1;
		}

		if ((code & 1) != 0) {
			difference += step >> 2;
		}

		difference += step >> 3;

		if ((code & 8) != 0) {
			difference = -difference;
		}

		*channel.Difference = difference;

		long const sample = Clamp_Sample(*channel.Predicted + difference);
		*channel.Predicted = sample;

		if (bits == 16) {
			*(short *)dest = (short)sample;
		} else {

			/*
			 * An 8 bit stream carries the top half of the sample, biased to unsigned.
			 */
			*dest = (unsigned char)((((unsigned long)sample >> 8) & 0xFF) ^ 0x80);
		}

		dest += deststride;

		/*
		 * The assembly tests the index as unsigned, so an adjustment that takes it below
		 * zero shows up as a very large value and folds back to zero.
		 */
		unsigned short next = (unsigned short)((unsigned short)*channel.Index + (unsigned short)_SosIndexAdjust[code]);

		if (next >= 0x8000) {
			next = 0;
		} else if (next > 88) {
			next = 88;
		}

		*channel.Index = (short)next;
		*channel.Step = (short)_SosStepTable[next];

		(*channel.SampleIndex)++;
	}
}


SosChannel Left_Channel(_SOS_COMPRESS_INFO * info)
{
	SosChannel channel;
	channel.SampleIndex = &info->dwSampleIndex;
	channel.CodeBuf = &info->wCodeBuf;
	channel.Code = &info->wCode;
	channel.Predicted = &info->dwPredicted;
	channel.Difference = &info->dwDifference;
	channel.Index = &info->wIndex;
	channel.Step = &info->wStep;
	return(channel);
}


SosChannel Right_Channel(_SOS_COMPRESS_INFO * info)
{
	SosChannel channel;
	channel.SampleIndex = &info->dwSampleIndex2;
	channel.CodeBuf = &info->wCodeBuf2;
	channel.Code = &info->wCode2;
	channel.Predicted = &info->dwPredicted2;
	channel.Difference = &info->dwDifference2;
	channel.Index = &info->wIndex2;
	channel.Step = &info->wStep2;
	return(channel);
}

}	// namespace


/// <summary>
/// Starts a compression stream, clearing the running sample and step index for both channels.
/// </summary>
/// <param name="info">The stream to initialize.</param>
void __cdecl sosCODECInitStream(_SOS_COMPRESS_INFO * info)
{
	info->wIndex = 0;
	info->dwPredicted = 0;
	info->wIndex2 = 0;
	info->dwPredicted2 = 0;
}


/// <summary>
/// Decompresses 4:1 ADPCM, the specialized path for 16 bit mono. Anything else is left alone,
/// as it was in the assembly, where only this one shape was ever implemented.
/// </summary>
/// <param name="info">Stream state, source and destination.</param>
/// <param name="bytes">How many bytes of samples to produce.</param>
/// <returns>unsigned long; The byte count asked for, or zero if the shape is not handled.</returns>
unsigned long __cdecl sosCODECDecompressData(_SOS_COMPRESS_INFO * info, unsigned long bytes)
{
	if (info->wBitSize != 16 || info->wChannels != 1) {
		return(0);
	}

	unsigned short index = (unsigned short)info->wIndex;

	Decode_Table_16((unsigned char const *)info->lpSource, (short *)info->lpDest, (int)(bytes / 2), 1, info->dwPredicted, index);

	info->wIndex = (short)index;
	return(bytes);
}


/// <summary>
/// Starts a compression stream for the general decoder.
/// </summary>
/// <param name="info">The stream to initialize.</param>
void __cdecl General_sosCODECInitStream(_SOS_COMPRESS_INFO * info)
{
	info->wIndex = 0;
	info->wStep = (short)_SosStepTable[0];
	info->dwPredicted = 0;
	info->dwSampleIndex = 0;
	info->wIndex2 = 0;
	info->wStep2 = (short)_SosStepTable[0];
	info->dwPredicted2 = 0;
	info->dwSampleIndex2 = 0;
}


/// <summary>
/// Decompresses 4:1 ADPCM for any bit size and channel count.
///
/// The assembly counted down and only then tested for zero, so a request for nothing, or an
/// odd number of samples on the stereo path, never reached the test and ran away through both
/// buffers. Those requests now decode nothing instead.
/// </summary>
/// <param name="info">Stream state, source and destination.</param>
/// <param name="bytes">How many bytes of samples to produce.</param>
/// <returns>unsigned long; The byte count asked for.</returns>
unsigned long __cdecl General_sosCODECDecompressData(_SOS_COMPRESS_INFO * info, unsigned long bytes)
{
	info->dwSampleIndex = 0;
	info->dwSampleIndex2 = 0;

	int const bits = info->wBitSize;
	int const samples = (bits == 16) ? (int)(bytes / 2) : (int)bytes;

	if (samples <= 0) {
		return(bytes);
	}

	unsigned char const * source = (unsigned char const *)info->lpSource;
	unsigned char * dest = (unsigned char *)info->lpDest;

	if (info->wChannels == 2) {
		if ((samples % 2) != 0) {
			return(bytes);
		}

		int const perchannel = samples / 2;
		int const sampled = (bits == 16) ? 4 : 2;

		Decode_General(Left_Channel(info), source, dest, perchannel, 2, sampled, bits);
		Decode_General(Right_Channel(info), source + 1, dest + (bits == 16 ? 2 : 1), perchannel, 2, sampled, bits);
	} else {
		Decode_General(Left_Channel(info), source, dest, samples, 1, (bits == 16) ? 2 : 1, bits);
	}

	return(bytes);
}


/// <summary>
/// Starts a VQA compression stream.
/// </summary>
/// <param name="info">The stream to initialize.</param>
void __cdecl VQA_sosCODECInitStream(_VQA_SOS_COMPRESS_INFO * info)
{
	info->wIndex = 0;
	info->dwPredicted = 0;
	info->wIndex2 = 0;
	info->dwPredicted2 = 0;
}


/// <summary>
/// Decompresses 4:1 ADPCM for VQA audio. Only 16 bit is implemented, mono and stereo; an 8 bit
/// request does nothing, which is what the assembly did.
///
/// A stereo stream carries its two channels as consecutive halves of the source rather than
/// interleaved, and writes them interleaved into the destination.
/// </summary>
/// <param name="src">Compressed nybbles.</param>
/// <param name="dst">Where the samples go.</param>
/// <param name="bits">Sample width; only 16 is handled.</param>
/// <param name="channels">1 or 2.</param>
/// <param name="bytes">How many bytes of samples to produce.</param>
/// <param name="info">Stream state carried between calls.</param>
void __cdecl VQA_sosCODECDecompressData(void * src, void * dst, unsigned short bits, unsigned short channels, unsigned long bytes, _VQA_SOS_COMPRESS_INFO * info)
{
	if (bits != 16) {
		return;
	}

	unsigned char const * source = (unsigned char const *)src;
	short * dest = (short *)dst;

	if (channels == 2) {
		int const perchannel = (int)(bytes / 4);
		unsigned short index = (unsigned short)info->wIndex;
		unsigned short index2 = (unsigned short)info->wIndex2;

		Decode_Table_16(source, dest, perchannel, 2, info->dwPredicted, index);
		Decode_Table_16(source + (bytes >> 3), dest + 1, perchannel, 2, info->dwPredicted2, index2);

		info->wIndex = (short)index;
		info->wIndex2 = (short)index2;
		return;
	}

	if (channels != 1) {
		return;
	}

	unsigned short index = (unsigned short)info->wIndex;

	Decode_Table_16(source, dest, (int)(bytes / 2), 1, info->dwPredicted, index);

	info->wIndex = (short)index;
}
