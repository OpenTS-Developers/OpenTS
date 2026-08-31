;******************************************************************************
;*                               O P E N  T S
;******************************************************************************
;* SPDX-License-Identifier: GPL-3.0-or-later
;* Copyright 2025 Electronic Arts Inc.
;* Copyright 2026 OpenTS contributors
;*
;* Contains material derived from Electronic Arts source code.
;* Modified by OpenTS contributors, 2026.
;* EA's GPLv3 Section 7 additional terms and supplemental warranty
;* disclaimers apply; see LICENSE.md.
;******************************************************************************

;***************************************************************************
;**   C O N F I D E N T I A L --- W E S T W O O D   S T U D I O S  I N C  **
;***************************************************************************
;*                                                                         *
;*                 Project Name : Command & Conquer                        *
;*                                                                         *
;*                    File Name : WINSAM.ASM                               *
;*                                                                         *
;*                   Programmer : Steve Tall                               *
;*                                                                         *
;*                   Start Date : October 26th, 1995                       *
;*                                                                         *
;*                  Last Update : October 26th, 1995  [ST]                 *
;*                                                                         *
;*-------------------------------------------------------------------------*
;* Functions:                                                              *
;* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - *


;IDEAL
;.386P ;P386
.686P	; Pentium Pro and above enables CMOV
.mmx
.model	flat, C ;MODEL USE32 FLAT

		; alignment has to be 'page' so that I can use 'align 32' below
		_TEXT$mmx segment page public use32 'CODE';codeseg


;
; externs
;
EXTERN C VoxelPixelDeltaTable : PTR WORD ; short [256][2]	 ; Defined in voxlib.cpp
EXTERN C VoxelNormalTranslateTable : BYTE ; uchar [256]		; Defined in voxlib.cpp
EXTERN C VoxelDrawBuffer : BYTE ; uchar [65536]				; Defined in voxdrsys.cpp
EXTERN C VoxelPaletteTranslateTable : BYTE ;  uchar [32][256]  ; Defined in voxdrsys.cpp
EXTERNDEF C UseCMOV : BYTE
EXTERNDEF C UseMMX : BYTE


; no idea how to get this to appear so fake it
_align MACRO count
	REPT (count / 8)
		;lea esp, [esp+0] but zero is optimized to a byte..
		db 08Dh, 0A4h, 024h, 00h, 00h, 00h, 00h
	ENDM
ENDM

_align_ MACRO
	;lea esp, [esp+0] but zero is optimized to a byte..
	db 08Dh, 064h, 024h, 00h
ENDM

_alignm MACRO reg
	mov reg,reg
ENDM


;
; structs
;
Vector3i16 STRUCT
	I	SWORD	?
	J	SWORD	?
	K	SWORD	?
Vector3i16 ENDS

IF SIZEOF Vector3i16 NE 6
	;.ERR <Vector3i16 size mismatch! Expected 6 bytes>
	ECHO *** Warning: Vector3i16 is not 6 bytes as expected ***
	vecstrucSize DWORD SIZEOF Vector3i16
ENDIF


VoxelFuncArgumentStruct STRUCT ; WARNING: Make sure this struct is updated if the version in voxdrsys.h is modified!
	StartOffset		DWORD   ?				; unsigned char *
	EndOffset		DWORD   ?				; unsigned char *
	DataOffset		DWORD   ?				; unsigned char *
	StartIndex		SDWORD  ?				; int
	StrideX			SDWORD  ?				; int
	StrideY			SDWORD  ?				; int
	TransformMatrix Vector3i16 4 DUP (<>)	; Vector3i16[4]
	XSize			BYTE	?				; unsigned char
	YSize			BYTE	?				; unsigned char
	ZSize			BYTE	?				; unsigned char
	Padding			BYTE	?				; pad to make struct size divisible by 4, matching the C++ implementation layout
VoxelFuncArgumentStruct ENDS


IF SIZEOF VoxelFuncArgumentStruct NE 52
	;.ERR <VoxelFuncArgumentStruct size mismatch! Expected 48 bytes>
	ECHO *** Warning: VoxelFuncArgumentStruct is not 48 bytes as expected ***
	voxstrucSize DWORD SIZEOF VoxelFuncArgumentStruct
ENDIF

TRANSFORM_VECTOR MACRO index:req
	EXITM <dword ptr ((VoxelFuncArgumentStruct.TransformMatrix) + type Vector3i16 * index)>
ENDM

TRANSFORM_COMPONENT MACRO index:req, component:req
	LOCAL offset
	IFIDNI <component>, <I>
		offset = (Vector3i16.I)
	ELSEIFIDNI <component>, <J>
		offset = (Vector3i16.J)
	ELSEIFIDNI <component>, <K>
		offset = (Vector3i16.K)
	ELSE
		.ERR <Invalid component! Use I, J, or K>
	ENDIF

	EXITM <word ptr ((VoxelFuncArgumentStruct.TransformMatrix) + type Vector3i16 * index + offset)>
ENDM

ADJUST_COLOR	MACRO	procname, mode:req

	LOCAL mode
	IFIDNI <mode>, <rgb565>

		R_POS = 11
		G_POS = 5
		B_POS = 0

		R_MASK = 11111000b
		G_MASK = 11111100b
		B_MASK = 11111000b

		R_SHIFT = 8
		G_SHIFT = 3
		B_SHIFT = 3

		R_SHIFT_MMX = 8
		G_SHIFT_MMX = 5
		B_SHIFT_MMX = 19

		R_SHIFT_CMOV = 8
		G_SHIFT_CMOV = 13
		B_SHIFT_CMOV = 19

	ELSEIFIDNI <mode>, <rgb555>

		R_POS = 10
		G_POS = 5
		B_POS = 0

		R_MASK = 11111000b
		G_MASK = 11111000b
		B_MASK = 11111000b

		R_SHIFT = 7
		G_SHIFT = 2
		B_SHIFT = 3

		R_SHIFT_MMX = 7
		G_SHIFT_MMX = 6
		B_SHIFT_MMX = 19

		R_SHIFT_CMOV = 9
		G_SHIFT_CMOV = 14
		B_SHIFT_CMOV = 19

	ELSEIFIDNI <mode>, <rgb556>

		R_POS = 11
		G_POS = 6
		B_POS = 0

		R_MASK = 11111000b
		G_MASK = 11111000b
		B_MASK = 11111100b

		R_SHIFT = 8
		G_SHIFT = 3
		B_SHIFT = 2

		R_SHIFT_MMX = 8
		G_SHIFT_MMX = 5
		B_SHIFT_MMX = 18

		R_SHIFT_CMOV = 8
		G_SHIFT_CMOV = 13
		B_SHIFT_CMOV = 18

	ELSEIFIDNI <mode>, <rgb655>

		R_POS = 11
		G_POS = 5
		B_POS = 0

		R_MASK = 11111100b
		G_MASK = 11111000b
		B_MASK = 11111000b

		R_SHIFT = 8
		G_SHIFT = 2
		B_SHIFT = 3

		R_SHIFT_MMX = 8
		G_SHIFT_MMX = 6
		B_SHIFT_MMX = 19

		R_SHIFT_CMOV = 8
		G_SHIFT_CMOV = 14
		B_SHIFT_CMOV = 19

	ELSE
		.ERR <Invalid mode! Use rgb565, rgb555, rgb556, or rgb655>
	ENDIF
PUBLIC	C &procname

&procname	proc C uses ebx ecx edx esi edi, \
		srcpal:DWORD, \
		dstpal:DWORD, \
		red:DWORD, \
		green:DWORD, \
		blue:DWORD, \
		intensity:DWORD, \
		_mask:DWORD

		LOCAL value:DWORD
		LOCAL counter:DWORD

		cmp	UseMMX, 0
		jnz	??mmx_path

		cmp	UseCMOV, 0
		jnz	??cmov_path

		mov	[counter], 255
		mov	esi, [srcpal]
		mov	edi, [dstpal]
		mov	edx, [_mask]

		; skip first index which is transparent
		add	esi, 3
		; skip first mask index
		inc	edx

		mov	word ptr [edi], 0
		xor	ebx, ebx
		jmp	??regular_loop

??cmov_path:
		pushfd
		cld

		mov	[counter], 255
		mov	esi, [srcpal]
		mov	[value], 000FF0000h
		mov	edi, [dstpal]
		mov	edx, [_mask]

		; skip first index which is transparent
		add	esi, 3
		; skip first mask index
		inc	edx

		mov	word ptr [edi], 0

		; advance destination
		add	edi, 2

		jmp	??cmov_loop

??mmx_path:
		mov	[counter], 255
		mov	esi, [srcpal]
		mov	edi, [dstpal]
		mov	edx, [_mask]

		; skip first index which is transparent
		add	esi, 3
		; skip first mask index
		inc	edx

		sub	edi, 2

		movq	mm1, qword ptr [red]
		movq	mm2, qword ptr [blue]
		psrad   mm1, 4
		psrad   mm2, 4
		packssdw mm1, mm2
		pxor	mm0, mm0
		movq	mm2, mm1
		punpckhwd mm2, mm1
		punpckhwd mm2, mm2

		jmp	??mmx_loop

		align 64

??mmx_loop:
		; advance destination
		add	edi, 2

		packuswb mm0, mm0

		; check mask, if zero use intensity
		cmp	byte ptr [edx], 0
		jz	 ??mmx_use_intensity

		movd	eax, mm0
		pxor	mm0, mm0

		punpcklbw mm0, [esi]

		mov	ebx, eax

		psrlw   mm0, 4

		; convert to wanted format
		and	eax, R_MASK
		mov	ecx, ebx
		shl	eax, R_SHIFT_MMX
		and	ebx, (G_MASK * 256)
		shr	ebx, G_SHIFT_MMX
		and	ecx, (B_MASK * 256 * 256)
		shr	ecx, B_SHIFT_MMX
		pmulhw  mm0, mm1

		; combine red and green
		or	 eax, ebx

		; advance source
		add	esi, 3

		or	 eax, ecx

		; advance mask
		inc	edx

		; store color
		mov	[edi], ax

		dec	[counter]

		jnz	??mmx_loop

??mmx_exit:
		; advance destination
		add	edi, 2

		packuswb mm0, mm0

		movd	eax, mm0
		mov	ebx, eax

		; convert to wanted format
		and	eax, R_MASK
		mov	ecx, ebx
		shl	eax, R_SHIFT_MMX
		and	ebx, (G_MASK * 256)
		shr	ebx, G_SHIFT_MMX
		and	ecx, (B_MASK * 256 * 256)
		shr	ecx, B_SHIFT_MMX

		; combine red and green
		or	 eax, ebx

		; combine blue
		or	 eax, ecx

		; store color
		mov	[edi], ax

		emms
		ret

??mmx_use_intensity:

		movd	eax, mm0
		pxor	mm0, mm0

		punpcklbw mm0, [esi]

		mov	ebx, eax

		psrlw   mm0, 4

		; convert to wanted format
		and	eax, R_MASK
		mov	ecx, ebx
		shl	eax, R_SHIFT_MMX
		and	ebx, (G_MASK * 256)
		shr	ebx, G_SHIFT_MMX
		and	ecx, (B_MASK * 256 * 256)
		shr	ecx, B_SHIFT_MMX

		; scale by intensity
		pmulhw  mm0, mm2

		; combine red and green
		or	 eax, ebx

		; advance source
		add	esi, 3

		; combine blue
		or	 eax, ecx

		; advance mask
		inc	edx

		; store color
		mov	[edi], ax

		dec	[counter]

		jnz	??mmx_loop

		jmp	??mmx_exit

		align 64

??cmov_loop:
		; check mask, if zero use intensity
		cmp	byte ptr [edx], 0

		movzx   eax, byte ptr [esi]
		movzx   ebx, byte ptr [esi+1]
		movzx   ecx, byte ptr [esi+2]
		jz	 ??cmov_use_intensity

		; scale r g b
		imul	eax, [red]
		imul	ebx, [green]
		imul	ecx, [blue]

??cmov_inner:
		; advance source
		add	esi, 3

		; clamp r g b
		test	eax, 0FF000000h
		cmovnz  eax, [value]
		test	ebx, 0FF000000h
		cmovnz  ebx, [value]
		test	ecx, 0FF000000h
		cmovnz  ecx, [value]

		; convert to wanted format
		and	eax, (R_MASK * 256 * 256)
		shr	eax, R_SHIFT_CMOV
		and	ebx, (G_MASK * 256 * 256)
		shr	ebx, G_SHIFT_CMOV
		and	ecx, (B_MASK * 256 * 256)

		; combine red and green
		or	 eax, ebx

		shr	ecx, B_SHIFT_CMOV

		; advance mask
		inc	edx

		; combine blue
		or	 eax, ecx

		; store color
		stosw

		dec	[counter]

		jnz	??cmov_loop

		popfd

		ret

??cmov_use_intensity:
		imul	eax, [intensity]
		imul	ebx, [intensity]
		imul	ecx, [intensity]
		jmp	??cmov_inner

		align 64

??regular_loop:
		xor	eax, eax
		mov	bl, [esi+1]
		xor	ecx, ecx
		mov	al, [esi]

		; check mask, if zero use intensity
		cmp	byte ptr [edx], 0
		jz	 ??regular_use_intensity

		mov	cl, [esi+2]
		inc	edx
		imul	eax, [red]
		imul	ebx, [green]
		imul	ecx, [blue]

		shr	eax, 16

		; advance source
		add	esi, 3

		shr	ebx, 16

		; clamp r g b
		cmp	eax, 255
		setbe   ah
		shr	ecx, 16
		dec	ah
		or	 al, ah
		cmp	ebx, 255
		setbe   bh
		dec	bh
		cmp	ecx, 255
		setbe   ch
		or	 bl, bh
		dec	ch
		or	 cl, ch

		; convert to wanted format
		and	eax, R_MASK
		shl	eax, R_SHIFT
		and	ebx, G_MASK
		shl	ebx, G_SHIFT
		and	ecx, B_MASK
		shr	ecx, B_SHIFT

		; combine red and green
		or	 eax, ebx

		; advance destination
		add	edi, 2

		xor	ebx, ebx

		; combine blue
		or	 eax, ecx

		dec	[counter]

		; store color
		mov	[edi], ax

		jnz	??regular_loop

		ret

??regular_use_intensity:

		mov	cl, [esi+2]

		; advance mask
		inc	edx

		; increase color by intensity
		imul	eax, [intensity]
		imul	ebx, [intensity]
		imul	ecx, [intensity]

		; normalize red
		shr	eax, 16

		; advance source
		add	esi, 3

		; normalize green
		shr	ebx, 16

		; clamp r g b
		cmp	eax, 255
		setbe   ah
		shr	ecx, 16
		dec	ah
		or	 al, ah
		cmp	ebx, 255
		setbe   bh
		dec	bh
		cmp	ecx, 255
		setbe   ch
		or	 bl, bh
		dec	ch
		or	 cl, ch

		; convert to wanted format
		and	eax, R_MASK
		shl	eax, R_SHIFT
		and	ebx, G_MASK
		shl	ebx, G_SHIFT
		and	ecx, B_MASK
		shr	ecx, B_SHIFT

		; combine red and green
		or	 eax, ebx

		; advance destination
		add	edi, 2

		xor	ebx, ebx

		; combine blue
		or	 eax, ecx

		dec	[counter]

		; store color
		mov	[edi], ax

		jnz	??regular_loop

		ret
&procname	ENDP
ENDM

ADJUST_COLOR Adjust_Color_565, rgb565
ADJUST_COLOR Adjust_Color_555, rgb555
ADJUST_COLOR Adjust_Color_556, rgb556
ADJUST_COLOR Adjust_Color_655, rgb655


BRIGHTEN_COLOR	MACRO	procname, mode:req

	LOCAL mode
	IFIDNI <mode>, <rgb565>

		CONSTANT_0 = 8
		CONSTANT_1 = 3
		CONSTANT_2 = 0F8h
		CONSTANT_3 = 0FCh
		CONSTANT_4 = 8
		CONSTANT_5 = 8
		CONSTANT_6 = 3
		CONSTANT_7 = 2
		CONSTANT_8 = 11
		CONSTANT_9 = 5
		CONSTANT_10 = 3
		CONSTANT_11 = 8
		CONSTANT_12 = 3

	ELSEIFIDNI <mode>, <rgb655>

		CONSTANT_0 = 8
		CONSTANT_1 = 2
		CONSTANT_2 = 0FCh
		CONSTANT_3 = 0F8h
		CONSTANT_4 = 8
		CONSTANT_5 = 8
		CONSTANT_6 = 2
		CONSTANT_7 = 3
		CONSTANT_8 = 10
		CONSTANT_9 = 5
		CONSTANT_10 = 3
		CONSTANT_11 = 8
		CONSTANT_12 = 3

	ELSEIFIDNI <mode>, <rgb556>

		CONSTANT_0 = 8
		CONSTANT_1 = 3
		CONSTANT_2 = 0F8h
		CONSTANT_3 = 0F8h
		CONSTANT_4 = 8
		CONSTANT_5 = 8
		CONSTANT_6 = 3
		CONSTANT_7 = 3
		CONSTANT_8 = 11
		CONSTANT_9 = 6
		CONSTANT_10 = 2
		CONSTANT_11 = 8
		CONSTANT_12 = 2

	ELSEIFIDNI <mode>, <rgb555>

		CONSTANT_0 = 7
		CONSTANT_1 = 2
		CONSTANT_2 = 0F8h
		CONSTANT_3 = 0F8h
		CONSTANT_4 = 8
		CONSTANT_5 = 8
		CONSTANT_6 = 3
		CONSTANT_7 = 3
		CONSTANT_8 = 10
		CONSTANT_9 = 5
		CONSTANT_10 = 3
		CONSTANT_11 = 8
		CONSTANT_12 = 3

	ELSE
		.ERR <Invalid mode! Use rgb565, rgb555, rgb556, or rgb655>
	ENDIF
PUBLIC	C &procname

&procname proc C uses ebx ecx edx esi edi, \
		mul_buffer:DWORD, \
		color_buffer:DWORD, \
		mulbuff_width:DWORD, \
		color_buff_width:DWORD, \
		dst_width:DWORD, \
		dst_height:DWORD

		LOCAL wleft:DWORD
		LOCAL hleft:DWORD
		LOCAL local4:DWORD
		LOCAL local3:DWORD
		LOCAL local2:DWORD
		LOCAL local1:DWORD

		xor	eax, eax
		mov	[local2], eax
		mov	eax, [dst_height]
		mov	[hleft], eax
		mov	[local1], 0
		mov	esi, [mul_buffer]
		mov	[local4], esi
		mov	edi, [color_buffer]
		mov	[local3], edi

??loop1:
		mov	eax, [dst_width]
		mov	[wleft], eax
		xor	eax, eax
		xor	ebx, ebx

??loop2:
		mov	al, [esi]
		test	al, al
		jz	 ??inc

		mov	bx, [edi]
		mov	byte ptr [local1], al
		mov	[local2], ebx
		shr	ebx, CONSTANT_0
		mov	eax, dword ptr [local2]
		shr	eax, CONSTANT_1
		and	ebx, CONSTANT_2
		and	eax, CONSTANT_3
		mov	edx, ebx
		mov	ecx, eax
		imul	ebx, [local1]
		imul	eax, [local1]
		shr	ebx, CONSTANT_4
		shr	eax, CONSTANT_5
		add	bl, dl
		setnb   dl
		add	al, cl
		setnb   cl
		dec	dl
		dec	cl
		or	 bl, dl
		or	 al, cl
		shr	ebx, CONSTANT_6
		shr	eax, CONSTANT_7
		shl	ebx, CONSTANT_8
		shl	eax, CONSTANT_9
		mov	ecx, [local2]
		shl	ecx, CONSTANT_10
		or	 eax, ebx
		and	ecx, 0FFh
		mov	edx, ecx
		imul	ecx, [local1]
		shr	ecx, CONSTANT_11
		add	cl, dl
		setnb   dl
		dec	dl
		or	 cl, dl
		shr	ecx, CONSTANT_12
		or	 eax, ecx
		mov	[edi], ax

??inc:
		inc	esi
		add	edi, 2
		dec	[wleft]
		jnz	??loop2

		mov	esi, [local4]
		add	esi, [mulbuff_width]
		mov	[local4], esi
		mov	edi, [local3]
		add	edi, [color_buff_width]
		mov	[local3], edi
		dec	[hleft]
		jnz	??loop1

		ret

&procname	ENDP
ENDM

BRIGHTEN_COLOR Brighten_Color_565, rgb565
BRIGHTEN_COLOR Brighten_Color_655, rgb655

BRIGHTEN_COLOR Brighten_Color_556, rgb556
BRIGHTEN_COLOR Brighten_Color_555, rgb555

align 128

MMX_BRIGHTEN_COLOR	MACRO	procname, mode:req

	LOCAL mode
	IFIDNI <mode>, <rgb565>

		CONSTANT_2 = 2
		CONSTANT_3 = 1
		CONSTANT_4 = 5
		CONSTANT_5 = 10
		CONSTANT_6 = 0F8h

	ELSEIFIDNI <mode>, <rgb555>

		CONSTANT_2 = 3
		CONSTANT_3 = 0
		CONSTANT_4 = 5
		CONSTANT_5 = 10
		CONSTANT_6 = 07Ch

	ELSEIFIDNI <mode>, <rgb556>

		CONSTANT_2 = 2
		CONSTANT_3 = 0
		CONSTANT_4 = 6
		CONSTANT_5 = 10
		CONSTANT_6 = 0F8h

	ELSEIFIDNI <mode>, <rgb655>

		CONSTANT_2 = 2
		CONSTANT_3 = 1
		CONSTANT_4 = 4
		CONSTANT_5 = 10
		CONSTANT_6 = 423A0A60h

	ELSE
		.ERR <Invalid mode! Use rgb565, rgb555, rgb556, or rgb655>
	ENDIF
PUBLIC	C &procname

&procname proc C uses ebx ecx edx esi edi, \
		mul_buffer:DWORD, \
		color_buffer:DWORD, \
		mulbuff_width:DWORD, \
		color_buff_width:DWORD, \
		dst_width:DWORD, \
		dst_height:DWORD, \
		mmx_buffer:DWORD

		LOCAL local7:DWORD
		LOCAL wleft:DWORD
		LOCAL hleft:DWORD
		LOCAL local4:DWORD
		LOCAL local3:DWORD
		LOCAL local2:QWORD
		LOCAL local1:QWORD

		xor	eax, eax
		mov	[local3], eax
		mov	eax, [dst_height]
		mov	[wleft], eax
		mov	dword ptr [local2], 0
		mov	dword ptr [local2+4], 0
		mov	dword ptr [local1], 0
		mov	dword ptr [local1+4], 0
		mov	esi, [mul_buffer]
		mov	[hleft], esi
		mov	edi, [color_buffer]
		mov	[local4], edi
		mov	edx, [mmx_buffer]

??loc_6C8BC1:
		mov	eax, [dst_width]
		mov	[local7], eax
		xor	eax, eax
		xor	ebx, ebx
		jmp	short ??loc_6C8C40

		align 64

??loc_6C8C00:
		inc	esi
		lea	edi, [edi+2]
		dec	[local7]
		jnz	??loc_6C8C40

		mov	esi, [hleft]
		add	esi, [mulbuff_width]
		mov	[hleft], esi
		mov	edi, [local4]
		add	edi, [color_buff_width]
		mov	[local4], edi
		dec	[wleft]
		jnz	??loc_6C8BC1

		jmp	??loc_6C8CB7

		align 64

??loc_6C8C40:
		mov	al, [esi]
		test	al, al
		jz	 ??loc_6C8C00

		mov	byte ptr [local2], al
		mov	bx, [edi]
		mov	byte ptr [local2+2], al
		mov	byte ptr [local2+4], al
		punpcklbw mm0, [edx+ebx*4]
		psrlw   mm0, 8
		movq	mm1, mm0
		pmullw  mm0, [local2]
		psrlw   mm0, 8
		paddusb mm0, mm1
		psrlw   mm0, CONSTANT_2
		movq	[local1], mm0

		xor	ebx, ebx
		xor	eax, eax
		mov	bl, byte ptr [local1]
		mov	al, byte ptr [local1+2]

if CONSTANT_3 gt 0
		shr	bl, CONSTANT_3
endif

		xor	ecx, ecx
		shl	eax, CONSTANT_4
		mov	cl, byte ptr [local1+4]
		shl	ecx, CONSTANT_5
if CONSTANT_6 eq 0423A0A60h
		and	eax, CONSTANT_6
		or	 ebx, eax
else
		or	 ebx, eax
		and	ecx, (CONSTANT_6 * 256)
endif
		inc	esi
		or	 ebx, ecx
		lea	edi, [edi+2]
		mov	[edi-2], bx
		dec	[local7]
		jnz	??loc_6C8C40

		mov	esi, [hleft]
		add	esi, [mulbuff_width]
		mov	[hleft], esi
		mov	edi, [local4]
		add	edi, [color_buff_width]
		mov	[local4], edi
		dec	[wleft]
		jnz	??loc_6C8BC1

??loc_6C8CB7:
		emms
		ret

&procname	ENDP
ENDM


MMX_BRIGHTEN_COLOR MMX_Brighten_Color_565, rgb565
MMX_BRIGHTEN_COLOR MMX_Brighten_Color_555, rgb555
MMX_BRIGHTEN_COLOR MMX_Brighten_Color_556, rgb556
MMX_BRIGHTEN_COLOR MMX_Brighten_Color_655, rgb655

align 8

; VOID cdecl Func(VoxelFuncArgumentStruct * arg1);
Draw_Voxel_Regular_Normals_ASM proc C uses esi edi ebx ecx edx \
		arg1:DWORD

		LOCAL local3:DWORD
		LOCAL local2:DWORD
		LOCAL local1:DWORD

		assume esi:ptr VoxelFuncArgumentStruct

		mov	esi, arg1
		push	ebp
		movsx ebp, WORD PTR [esi + TRANSFORM_COMPONENT(3,I)]
		movsx edx, WORD PTR [esi + TRANSFORM_COMPONENT(3,J)]
		xor	ecx, ecx
		mov	cl, [esi].ZSize
		mov	edi, offset VoxelPixelDeltaTable
		xor	eax, eax
		xor	ebx, ebx
		mov	esi, 4

??loc_6C8E6A:
		mov	[edi], ax
		add	edi, 2
		add	eax, ebp
		mov	[edi], bx
		add	edi, 2
		add	ebx, edx
		dec	ecx
		jnz	??loc_6C8E6A

		pop	ebp
		mov	esi, arg1
		xor	eax, eax
		mov	eax, [esi + TRANSFORM_VECTOR(0)]
		mov	local1, eax
		mov	ch, [esi].YSize

??loc_6C8E8C:
		mov	eax, [esi].StartIndex
		mov	local3, eax
		mov	eax, local1
		mov	local2, eax
		mov	cl, [esi].XSize

??loc_6C8E9B:
		mov	eax, local1
		mov	[esi + TRANSFORM_VECTOR(0)], eax
		xor	edi, edi
		mov	eax, [esi].StartIndex
		mov	ebx, [esi].StartOffset
		or	 edi, [ebx + eax*4]
		jns	??loc_6C8EE0

??loc_6C8EAD:
		mov	eax, [esi + TRANSFORM_VECTOR(1)]
		add	local1, eax
		mov	eax, [esi].StrideX
		add	[esi].StartIndex, eax
		dec	cl
		jnz	??loc_6C8E9B

		mov	eax, local2
		add	eax, [esi + TRANSFORM_VECTOR(2)]
		mov	local1, eax
		mov	eax, local3
		add	eax, [esi].StrideY
		mov	[esi].StartIndex, eax
		dec	ch
		jnz	??loc_6C8E8C

		ret

		align 8

??loc_6C8EE0:
		push	esi
		push	ecx
		push	ebp
		xor	ecx, ecx
		add	edi, [esi].DataOffset
		mov	cl, [esi].ZSize
		mov	bx, [esi + TRANSFORM_COMPONENT(0,J)]
		shl	ebx, 16
		mov	ebp, [esi + TRANSFORM_VECTOR(3)]
		mov	bx, [esi + TRANSFORM_COMPONENT(0,I)]

??loc_6C8EF9:
		xor	eax, eax
		cmp	ecx, 0
		jz	 ??loc_6C8F36

		mov	al, [edi]
		inc	edi
		sub	cl, al

		add	ebx, [VoxelPixelDeltaTable + eax*4]
		mov	ch, [edi]
		inc	edi
		test	ch, ch
		jz	 ??loc_6C8F33

??loc_6C8F13:
		mov	eax, ebx
		dec	cl
		shr	eax, 16
		mov	dl, [edi]
		mov	al, bh
		add	edi, 2
		add	ebx, ebp
		dec	ch
		mov	[VoxelDrawBuffer + eax], dl
		mov	[VoxelDrawBuffer + eax + 1], dl
		jnz	??loc_6C8F13

??loc_6C8F33:
		inc	edi
		jmp	??loc_6C8EF9

??loc_6C8F36:
		pop	ebp
		pop	ecx
		pop	esi
		jmp	??loc_6C8EAD

Draw_Voxel_Regular_Normals_ASM endp

align 8

; VOID cdecl Func(VoxelFuncArgumentStruct * arg1);
Draw_Voxel_Reverse_Normals_ASM proc C uses esi edi ebx ecx edx \
		arg1:DWORD

		LOCAL local3:DWORD
		LOCAL local2:DWORD
		LOCAL local1:DWORD

		assume eax:ptr VoxelFuncArgumentStruct

		mov	esi, arg1
		push	ebp
		movsx ebp, WORD PTR [esi + TRANSFORM_COMPONENT(3,I)]
		movsx edx, WORD PTR [esi + TRANSFORM_COMPONENT(3,J)]
		xor	ecx, ecx
		mov	cl, [esi].ZSize
		mov	edi, offset VoxelPixelDeltaTable
		xor	eax, eax
		xor	ebx, ebx
		mov	esi, 4

??loc_6C8F6A:
		mov	[edi], ax
		add	edi, 2
		add	eax, ebp
		mov	[edi], bx
		add	edi, 2
		add	ebx, edx
		dec	ecx
		jnz	??loc_6C8F6A

		pop	ebp
		mov	esi, arg1
		xor	eax, eax
		mov	eax, [esi + TRANSFORM_VECTOR(0)]
		mov	local1, eax
		mov	ch, [esi].YSize

??loc_6C8F8C:
		mov	eax, [esi].StartIndex
		mov	local3, eax
		mov	eax, local1
		mov	local2, eax
		mov	cl, [esi].XSize

??loc_6C8F9B:
		mov	eax, local1
		mov	[esi + TRANSFORM_VECTOR(0)], eax
		xor	edi, edi
		mov	eax, [esi].StartIndex
		mov	ebx, [esi].EndOffset
		or	 edi, [ebx + eax*4]
		jns	??loc_6C8FE0

??loc_6C8FAE:
		mov	eax, [esi + TRANSFORM_VECTOR(1)]
		add	local1, eax
		mov	eax, [esi].StrideX
		add	[esi].StartIndex, eax
		dec	cl
		jnz	??loc_6C8F9B

		mov	eax, local2
		add	eax, [esi + TRANSFORM_VECTOR(2)]
		mov	local1, eax
		mov	eax, local3
		add	eax, [esi].StrideY
		mov	[esi].StartIndex, eax
		dec	ch
		jnz	??loc_6C8F8C

		ret

		align 8

??loc_6C8FE0:
		push	esi
		push	ecx
		push	ebp
		xor	ecx, ecx
		add	edi, [esi].DataOffset
		mov	cl, [esi].ZSize
		mov	bx, [esi + TRANSFORM_COMPONENT(0,J)]
		shl	ebx, 16
		mov	ebp, [esi + TRANSFORM_VECTOR(3)]
		mov	bx, [esi + TRANSFORM_COMPONENT(0,I)]

??loc_6C8FF9:
		cmp	ecx, 0
		jz	 ??loc_6C9035

		mov	ch, [edi]
		dec	edi
		test	ch, ch
		jz	 ??loc_6C9024

??loc_6C9005:
		mov	eax, ebx
		dec	cl
		dec	edi
		shr	eax, 16
		mov	dl, [edi]
		mov	al, bh
		dec	edi
		add	ebx, ebp
		dec	ch
		mov	[VoxelDrawBuffer + eax], dl
		mov	[VoxelDrawBuffer + eax + 1], dl
		jnz	??loc_6C9005

??loc_6C9024:
		dec	edi
		xor	eax, eax
		mov	al, [edi]
		dec	edi
		sub	cl, al

		add	ebx, [VoxelPixelDeltaTable + eax*4]
		jmp	??loc_6C8FF9

??loc_6C9035:
		pop	ebp
		pop	ecx
		pop	esi
		jmp	??loc_6C8FAE

Draw_Voxel_Reverse_Normals_ASM endp

align 8

; VOID cdecl Func(VoxelFuncArgumentStruct * arg1);
Draw_Voxel_Regular_Lighting_Normals_ASM  proc C uses esi edi ebx ecx edx \
		arg1:DWORD

		LOCAL local3:DWORD
		LOCAL local2:DWORD
		LOCAL local1:DWORD

		assume eax:ptr VoxelFuncArgumentStruct

		mov	esi, arg1
		push	ebp
		movsx ebp, WORD PTR [esi + TRANSFORM_COMPONENT(3,I)]
		movsx edx, WORD PTR [esi + TRANSFORM_COMPONENT(3,J)]
		xor	ecx, ecx
		mov	cl, [esi].ZSize
		mov	edi, offset VoxelPixelDeltaTable
		xor	eax, eax
		xor	ebx, ebx
		mov	esi, 4

??loc_6C906A:
		mov	[edi], ax
		add	edi, 2
		add	eax, ebp
		mov	[edi], bx
		add	edi, 2
		add	ebx, edx
		dec	ecx
		jnz	??loc_6C906A

		pop	ebp
		mov	esi, arg1
		xor	eax, eax
		mov	eax, [esi + TRANSFORM_VECTOR(0)]
		mov	local1, eax
		mov	ch, [esi].YSize

??loc_6C908C:
		mov	eax, [esi].StartIndex
		mov	local3, eax
		mov	eax, local1
		mov	local2, eax
		mov	cl, [esi].XSize

??loc_6C909B:
		mov	eax, local1
		mov	[esi + TRANSFORM_VECTOR(0)], eax
		xor	edi, edi
		mov	eax, [esi].StartIndex
		mov	ebx, [esi].StartOffset
		or	 edi, [ebx + eax*4]
		jns	??loc_6C90E0

??loc_6C90AD:
		mov	eax, [esi + TRANSFORM_VECTOR(1)]
		add	local1, eax
		mov	eax, [esi].StrideX
		add	[esi].StartIndex, eax
		dec	cl
		jnz	??loc_6C909B

		mov	eax, local2
		add	eax, [esi + TRANSFORM_VECTOR(2)]
		mov	local1, eax
		mov	eax, local3
		add	eax, [esi].StrideY
		mov	[esi].StartIndex, eax
		dec	ch
		jnz	??loc_6C908C

		ret

		align 8

??loc_6C90E0:
		push	esi
		push	ecx
		push	ebp
		xor	ecx, ecx
		add	edi, [esi].DataOffset
		mov	cl, [esi].ZSize
		mov	bx, [esi + TRANSFORM_COMPONENT(0,J)]
		shl	ebx, 16
		mov	ebp, [esi + TRANSFORM_VECTOR(3)]
		mov	bx, [esi + TRANSFORM_COMPONENT(0,I)]

??loc_6C90F9:
		xor	eax, eax
		cmp	ecx, 0
		jz	 ??loc_6C9147

		mov	al, [edi]
		inc	edi
		sub	cl, al

		add	ebx, [VoxelPixelDeltaTable + eax*4]
		mov	ch, [edi]
		inc	edi
		test	ch, ch
		jz	 ??loc_6C9144

??loc_6C9113:
		mov	eax, ebx
		dec	cl
		shr	eax, 16
		xor	edx, edx
		mov	dl, [edi+1]
		mov	al, bh
		mov	dh, VoxelNormalTranslateTable[edx]
		mov	dl, [edi]
		add	ebx, ebp
		add	edi, 2
		dec	ch
		mov	dl, VoxelPaletteTranslateTable[edx]
		mov	[VoxelDrawBuffer + eax], dl
		mov	[VoxelDrawBuffer + eax + 1], dl
		jnz	??loc_6C9113

??loc_6C9144:
		inc	edi
		jmp	??loc_6C90F9

??loc_6C9147:
		pop	ebp
		pop	ecx
		pop	esi
		jmp	??loc_6C90AD

Draw_Voxel_Regular_Lighting_Normals_ASM endp

align 8

; VOID cdecl Func(VoxelFuncArgumentStruct * arg1);
Draw_Voxel_Reverse_Lighting_Normals_ASM proc C uses esi edi ebx ecx edx \
		arg1:DWORD

		LOCAL local3:DWORD
		LOCAL local2:DWORD
		LOCAL local1:DWORD

		assume eax:ptr VoxelFuncArgumentStruct

		mov	esi, arg1
		push	ebp
		movsx ebp, WORD PTR [esi + TRANSFORM_COMPONENT(3,I)]
		movsx edx, WORD PTR [esi + TRANSFORM_COMPONENT(3,J)]
		xor	ecx, ecx
		mov	cl, [esi].ZSize
		mov	edi, offset VoxelPixelDeltaTable
		xor	eax, eax
		xor	ebx, ebx
		mov	esi, 4

??loc_6C918A:
		mov	[edi], ax
		add	edi, 2
		add	eax, ebp
		mov	[edi], bx
		add	edi, 2
		add	ebx, edx
		dec	ecx
		jnz	??loc_6C918A

		pop	ebp
		mov	esi, arg1
		xor	eax, eax
		mov	eax, [esi + TRANSFORM_VECTOR(0)]
		mov	local1, eax
		mov	ch, [esi].YSize

??loc_6C91AC:
		mov	eax, [esi].StartIndex
		mov	local3, eax
		mov	eax, local1
		mov	local2, eax
		mov	cl, [esi].XSize

??loc_6C91BB:
		mov	eax, local1
		mov	[esi + TRANSFORM_VECTOR(0)], eax
		xor	edi, edi
		mov	eax, [esi].StartIndex
		mov	ebx, [esi].EndOffset
		or	 edi, [ebx + eax*4]
		jns	??loc_6C9200

??loc_6C91CE:
		mov	eax, [esi + TRANSFORM_VECTOR(1)]
		add	local1, eax
		mov	eax, [esi].StrideX
		add	[esi].StartIndex, eax
		dec	cl
		jnz	??loc_6C91BB

		mov	eax, local2
		add	eax, [esi + TRANSFORM_VECTOR(2)]
		mov	local1, eax
		mov	eax, local3
		add	eax, [esi].StrideY
		mov	[esi].StartIndex, eax
		dec	ch
		jnz	??loc_6C91AC

		ret

		align 8

??loc_6C9200:
		push	esi
		push	ecx
		push	ebp
		xor	ecx, ecx
		add	edi, [esi].DataOffset
		mov	cl, [esi].ZSize
		mov	bx, [esi + TRANSFORM_COMPONENT(0,J)]
		shl	ebx, 16
		mov	ebp, [esi + TRANSFORM_VECTOR(3)]
		mov	bx, [esi + TRANSFORM_COMPONENT(0,I)]

??loc_6C9219:
		cmp	ecx, 0
		jz	 ??loc_6C9267

		mov	ch, [edi]
		dec	edi
		test	ch, ch
		jz	 ??loc_6C9256

??loc_6C9225:
		mov	eax, ebx
		dec	cl
		shr	eax, 16
		xor	edx, edx
		mov	dl, [edi]
		mov	al, bh
		mov	dh, VoxelNormalTranslateTable[edx]
		mov	dl, [edi-1]
		add	ebx, ebp
		sub	edi, 2
		dec	ch
		mov	dl, VoxelPaletteTranslateTable[edx]
		mov	[VoxelDrawBuffer + eax], dl
		mov	[VoxelDrawBuffer + eax + 1], dl
		jnz	??loc_6C9225

??loc_6C9256:
		dec	edi
		xor	eax, eax
		mov	al, [edi]
		dec	edi
		sub	cl, al

		add	ebx, [VoxelPixelDeltaTable + eax*4]
		jmp	??loc_6C9219

??loc_6C9267:
		pop	ebp
		pop	ecx
		pop	esi
		jmp	??loc_6C91CE

Draw_Voxel_Reverse_Lighting_Normals_ASM endp

align 8

; VOID cdecl Func(VoxelFuncArgumentStruct * arg1);
Draw_Voxel_Regular_ASM proc C uses esi edi ebx ecx edx \
		arg1:DWORD

		LOCAL local3:DWORD
		LOCAL local2:DWORD
		LOCAL local1:DWORD

		assume eax:ptr VoxelFuncArgumentStruct

		mov	esi, arg1
		push	ebp
		movsx ebp, WORD PTR [esi + TRANSFORM_COMPONENT(3,I)]
		movsx edx, WORD PTR [esi + TRANSFORM_COMPONENT(3,J)]
		mov	ecx, 0FFh
		sub	cl, [esi].ZSize
		mov	edi, offset VoxelPixelDeltaTable
		xor	eax, eax
		xor	ebx, ebx
		mov	esi, 4

??loc_6C92AD:
		mov	[edi], ax
		add	edi, 2
		add	eax, ebp
		mov	[edi], bx
		add	edi, 2
		add	ebx, edx
		dec	ecx
		jnz	??loc_6C92AD

		pop	ebp
		mov	esi, arg1
		xor	eax, eax
		mov	eax, [esi + TRANSFORM_VECTOR(0)]
		mov	local1, eax
		mov	ch, [esi].YSize

??loc_6C92CF:
		mov	eax, [esi].StartIndex
		mov	local3, eax
		mov	eax, local1
		mov	local2, eax
		mov	cl, [esi].XSize

??loc_6C92DE:
		mov	eax, local1
		mov	[esi + TRANSFORM_VECTOR(0)], eax
		xor	edi, edi
		mov	eax, [esi].StartIndex
		mov	ebx, [esi].StartOffset
		or	 edi, [ebx + eax*4]
		jns	??loc_6C9320

??loc_6C92F0:
		mov	eax, [esi + TRANSFORM_VECTOR(1)]
		add	local1, eax
		mov	eax, [esi].StrideX
		add	[esi].StartIndex, eax
		dec	cl
		jnz	??loc_6C92DE

		mov	eax, local2
		add	eax, [esi + TRANSFORM_VECTOR(2)]
		mov	local1, eax
		mov	eax, local3
		add	eax, [esi].StrideY
		mov	[esi].StartIndex, eax
		dec	ch
		jnz	??loc_6C92CF

		ret

		align 8

??loc_6C9320:
		push	esi
		push	ecx
		push	ebp
		xor	ecx, ecx
		add	edi, [esi].DataOffset
		mov	cl, [esi].ZSize
		mov	bx, [esi + TRANSFORM_COMPONENT(0,J)]
		shl	ebx, 16
		mov	ebp, [esi + TRANSFORM_VECTOR(3)]
		mov	bx, [esi + TRANSFORM_COMPONENT(0,I)]

??loc_6C9339:
		xor	eax, eax
		cmp	ecx, 0
		jz	 ??loc_6C936E

		mov	al, [edi]
		inc	edi
		sub	cl, al

		add	ebx, [VoxelPixelDeltaTable + eax*4]
		mov	ch, [edi]
		inc	edi
		test	ch, ch
		jz	 ??loc_6C936B

??loc_6C9353:
		mov	eax, ebx
		dec	cl
		shr	eax, 16
		mov	dl, [edi]
		mov	al, bh
		inc	edi
		add	ebx, ebp
		dec	ch
		mov	[VoxelDrawBuffer + eax], dl
		jnz	??loc_6C9353

??loc_6C936B:
		inc	edi
		jmp	??loc_6C9339

??loc_6C936E:
		pop	ebp
		pop	ecx
		pop	esi
		jmp	??loc_6C92F0

Draw_Voxel_Regular_ASM endp

align 8

; VOID cdecl Func(VoxelFuncArgumentStruct * arg1);
Draw_Voxel_Reverse_ASM proc C uses esi edi ebx ecx edx \
		arg1:DWORD

		LOCAL local3:DWORD
		LOCAL local2:DWORD
		LOCAL local1:DWORD

		assume eax:ptr VoxelFuncArgumentStruct

		mov	esi, arg1
		push	ebp
		movsx ebp, WORD PTR [esi + TRANSFORM_COMPONENT(3,I)]
		movsx edx, WORD PTR [esi + TRANSFORM_COMPONENT(3,J)]
		mov	ecx, 0FFh
		sub	cl, [esi].ZSize
		mov	edi, offset VoxelPixelDeltaTable
		xor	eax, eax
		xor	ebx, ebx
		mov	esi, 4

??loc_6C93AD:
		mov	[edi], ax
		add	edi, 2
		add	eax, ebp
		mov	[edi], bx
		add	edi, 2
		add	ebx, edx
		dec	ecx
		jnz	??loc_6C93AD

		pop	ebp
		mov	esi, arg1
		xor	eax, eax
		mov	eax, [esi + TRANSFORM_VECTOR(0)]
		mov	local1, eax
		mov	ch, [esi].YSize

??loc_6C93CF:
		mov	eax, [esi].StartIndex
		mov	local3, eax
		mov	eax, local1
		mov	local2, eax
		mov	cl, [esi].XSize

??loc_6C93DE:
		mov	eax, local1
		mov	[esi + TRANSFORM_VECTOR(0)], eax
		xor	edi, edi
		mov	eax, [esi].StartIndex
		mov	ebx, [esi].EndOffset
		or	 edi, [ebx + eax*4]
		jns	??loc_6C9420

??loc_6C93F1:
		mov	eax, [esi + TRANSFORM_VECTOR(1)]
		add	local1, eax
		mov	eax, [esi].StrideX
		add	[esi].StartIndex, eax
		dec	cl
		jnz	??loc_6C93DE

		mov	eax, local2
		add	eax, [esi + TRANSFORM_VECTOR(2)]
		mov	local1, eax
		mov	eax, local3
		add	eax, [esi].StrideY
		mov	[esi].StartIndex, eax
		dec	ch
		jnz	??loc_6C93CF

		ret

		align 8

??loc_6C9420:
		push	esi
		push	ecx
		push	ebp
		xor	ecx, ecx
		add	edi, [esi].DataOffset
		mov	cl, [esi].ZSize
		mov	bx, [esi + TRANSFORM_COMPONENT(0,J)]
		shl	ebx, 16
		mov	ebp, [esi + TRANSFORM_VECTOR(3)]
		mov	bx, [esi + TRANSFORM_COMPONENT(0,I)]

??loc_6C9439:
		cmp	ecx, 0
		jz	 ??loc_6C946E

		mov	ch, [edi]
		dec	edi
		test	ch, ch
		jz	 ??loc_6C945D

??loc_6C9445:
		mov	eax, ebx
		dec	cl
		shr	eax, 16
		mov	dl, [edi]
		mov	al, bh
		dec	edi
		add	ebx, ebp
		dec	ch
		mov	[VoxelDrawBuffer + eax], dl
		jnz	??loc_6C9445

??loc_6C945D:
		dec	edi
		xor	eax, eax
		mov	al, [edi]
		dec	edi
		sub	cl, al

		add	ebx, [VoxelPixelDeltaTable + eax*4]
		jmp	??loc_6C9439

??loc_6C946E:
		pop	ebp
		pop	ecx
		pop	esi
		jmp	??loc_6C93F1

Draw_Voxel_Reverse_ASM endp

align 8

; VOID cdecl Func(VoxelFuncArgumentStruct * arg1);
Draw_Voxel_Regular_UNUSED_ASM proc C uses esi edi ebx ecx edx \
		arg1:DWORD

		LOCAL local3:DWORD
		LOCAL local2:DWORD
		LOCAL local1:DWORD

		assume esi:ptr VoxelFuncArgumentStruct

		mov	esi, arg1
		push	ebp
		movsx ebp, WORD PTR [esi + TRANSFORM_COMPONENT(3,I)]
		movsx edx, WORD PTR [esi + TRANSFORM_COMPONENT(3,J)]
		mov	ecx, 0FFh
		sub	cl, [esi].ZSize
		mov	edi, offset VoxelPixelDeltaTable
		xor	eax, eax
		xor	ebx, ebx
		mov	esi, 4

??loc_6C94AD:
		mov	[edi], ax
		add	edi, 2
		add	eax, ebp
		mov	[edi], bx
		add	edi, 2
		add	ebx, edx
		dec	ecx
		jnz	??loc_6C94AD

		pop	ebp
		mov	esi, arg1
		xor	eax, eax
		mov	eax, [esi + TRANSFORM_VECTOR(0)]
		mov	local1, eax
		mov	ch, [esi].YSize

??loc_6C94CF:
		mov	eax, [esi].StartIndex
		mov	local3, eax
		mov	eax, local1
		mov	local2, eax
		mov	cl, [esi].XSize

??loc_6C94DE:
		mov	eax, local1
		mov	[esi + TRANSFORM_VECTOR(0)], eax
		xor	edi, edi
		mov	eax, [esi].StartIndex
		mov	ebx, [esi].StartOffset
		or	 edi, [ebx + eax*4]
		jns	??loc_6C9520

??loc_6C94F0:
		mov	eax, [esi + TRANSFORM_VECTOR(1)]
		add	local1, eax
		mov	eax, [esi].StrideX
		add	[esi].StartIndex, eax
		dec	cl
		jnz	??loc_6C94DE

		mov	eax, local2
		add	eax, [esi + TRANSFORM_VECTOR(2)]
		mov	local1, eax
		mov	eax, local3
		add	eax, [esi].StrideY
		mov	[esi].StartIndex, eax
		dec	ch
		jnz	??loc_6C94CF

		ret

		align 8

??loc_6C9520:
		push	esi
		push	ecx
		push	ebp
		xor	ecx, ecx
		add	edi, [esi].DataOffset
		mov	cl, [esi].ZSize
		mov	bx, [esi + TRANSFORM_COMPONENT(0,J)]
		shl	ebx, 16
		mov	ebp, [esi + TRANSFORM_VECTOR(3)]
		mov	bx, [esi + TRANSFORM_COMPONENT(0,I)]

??loc_6C9539:
		xor	eax, eax
		cmp	ecx, 0
		jz	 ??loc_6C9574

		mov	al, [edi]
		inc	edi
		sub	cl, al

		add	ebx, [VoxelPixelDeltaTable + eax*4]
		mov	ch, [edi]
		inc	edi
		test	ch, ch
		jz	 ??loc_6C9571

??loc_6C9553:
		mov	eax, ebx
		dec	cl
		shr	eax, 16
		mov	dl, [edi]
		mov	al, bh
		inc	edi
		add	ebx, ebp
		dec	ch
		mov	[VoxelDrawBuffer + eax], dl
		mov	[VoxelDrawBuffer + eax + 1], dl
		jnz	??loc_6C9553

??loc_6C9571:
		inc	edi
		jmp	??loc_6C9539

??loc_6C9574:
		pop	ebp
		pop	ecx
		pop	esi
		jmp	??loc_6C94F0

Draw_Voxel_Regular_UNUSED_ASM endp

align 8

; VOID cdecl Func(VoxelFuncArgumentStruct * arg1);
Draw_Voxel_Reverse_UNUSED_ASM proc C uses esi edi ebx ecx edx \
		arg1:DWORD

		LOCAL local3:DWORD
		LOCAL local2:DWORD
		LOCAL local1:DWORD

		assume esi:ptr VoxelFuncArgumentStruct

		mov	esi, arg1
		push	ebp
		movsx ebp, WORD PTR [esi + TRANSFORM_COMPONENT(3,I)]
		movsx edx, WORD PTR [esi + TRANSFORM_COMPONENT(3,J)]
		mov	ecx, 0FFh
		sub	cl, [esi].ZSize
		mov	edi, offset VoxelPixelDeltaTable
		xor	eax, eax
		xor	ebx, ebx
		mov	esi, 4

??loc_6C95AD:
		mov	[edi], ax
		add	edi, 2
		add	eax, ebp
		mov	[edi], bx
		add	edi, 2
		add	ebx, edx
		dec	ecx
		jnz	??loc_6C95AD

		pop	ebp
		mov	esi, arg1
		xor	eax, eax
		mov	eax, [esi + TRANSFORM_VECTOR(0)]
		mov	local1, eax
		mov	ch, [esi].YSize

??loc_6C95CF:
		mov	eax, [esi].StartIndex
		mov	local3, eax
		mov	eax, local1
		mov	local2, eax
		mov	cl, [esi].XSize

??loc_6C95DE:
		mov	eax, local1
		mov	[esi + TRANSFORM_VECTOR(0)], eax
		xor	edi, edi
		mov	eax, [esi].StartIndex
		mov	ebx, [esi].EndOffset
		or	 edi, [ebx + eax*4]
		jns	??loc_6C9620

??loc_6C95F1:
		mov	eax, [esi + TRANSFORM_VECTOR(1)]
		add	local1, eax
		mov	eax, [esi].StrideX
		add	[esi].StartIndex, eax
		dec	cl
		jnz	??loc_6C95DE

		mov	eax, local2
		add	eax, [esi + TRANSFORM_VECTOR(2)]
		mov	local1, eax
		mov	eax, local3
		add	eax, [esi].StrideY
		mov	[esi].StartIndex, eax
		dec	ch
		jnz	??loc_6C95CF

		ret

		align 8

??loc_6C9620:
		push	esi
		push	ecx
		push	ebp
		xor	ecx, ecx
		add	edi, [esi].DataOffset
		mov	cl, [esi].ZSize
		mov	bx, [esi + TRANSFORM_COMPONENT(0,J)]
		shl	ebx, 16
		mov	ebp, [esi + TRANSFORM_VECTOR(3)]
		mov	bx, [esi + TRANSFORM_COMPONENT(0,I)]

??loc_6C9639:
		cmp	ecx, 0
		jz	 ??loc_6C9674

		mov	ch, [edi]
		dec	edi
		test	ch, ch
		jz	 ??loc_6C9663

??loc_6C9645:
		mov	eax, ebx
		dec	cl
		shr	eax, 16
		mov	dl, [edi]
		mov	al, bh
		dec	edi
		add	ebx, ebp
		dec	ch
		mov	[VoxelDrawBuffer + eax], dl
		mov	[VoxelDrawBuffer + eax + 1], dl
		jnz	??loc_6C9645

??loc_6C9663:
		dec	edi
		xor	eax, eax
		mov	al, [edi]
		dec	edi
		sub	cl, al

		add	ebx, [VoxelPixelDeltaTable + eax*4]
		jmp	??loc_6C9639

??loc_6C9674:
        pop	ebp
        pop	ecx
        pop	esi
        jmp	??loc_6C95F1

Draw_Voxel_Reverse_UNUSED_ASM endp


public _Int3
_Int3 proc near
	;int	3
	ret
_Int3 endp


;proc Stop_Execution C near
;
;	nop
;	ret
;
;endp


		_TEXT$mmx ends
		_TEXT$mycode segment page public use32 'CODE' ; segment mycode page public use32 'code'	; Need stricter segment alignment

public		C	Asm_Interpolate
public		C	Asm_Interpolate_Line_Double
public		C	Asm_Interpolate_Line_Interpolate
extern		C	PaletteInterpolationTable:byte


;*********************************************************************************************
;* Asm_Interpolate -- interpolate a 320x200 buffer to a 640x400 screen                       *
;*                                                                                           *
;* INPUT:	ptr to source buffer (320x200 image)                                         *
;*              ptr to dest buffer (640x400)                                                 *
;*              height of source buffer                                                      *
;*              width of source buffer                                                        *
;*              width of dest buffer                                                         *
;*                                                                                           *
;*                                                                                           *
;* OUTPUT:      none                                                                         *
;*                                                                                           *
;* Warnings:                                                                                 *
;*                                                                                           *
;* HISTORY:                                                                                  *
;*   12/15/95 ST : Created.                                                                  *
;*===========================================================================================*

Asm_Interpolate PROC near USES ebx ecx edi esi \
		\
		, src_ptr:dword \
		, dest_ptr:dword \
		, source_height:dword \
		, source_width:dword \
		, dest_width:dword

		LOCAL	old_dest:dword

		pushad

		mov	eax,[dest_ptr]
		mov	[old_dest],eax

		mov	esi,[src_ptr]

??each_line_loop:
		mov	ecx,[source_width]
		sub	ecx,2
		shr	ecx,1
		mov	edi,[old_dest]
		jmp	??interpolate_loop

		_align	32 ;align	32
		_alignm edi
;
; convert 2 pixels of source into 4 pixels of destination
; so we can write to video memory with dwords
;
??interpolate_loop:
		mov	eax,dword ptr [esi]
		lea	esi,[esi+2]
		mov	edx,eax
		mov	ebx,eax
		and	edx,65535
		ror	ebx,8
		mov	bl,[edx+PaletteInterpolationTable]
		mov	bh,ah
		and	eax,000ffff00h
		ror	ebx,8

;1st 3 pixels now in ebx

		shr	eax,8
		mov	bh,[eax+PaletteInterpolationTable]
		ror	ebx,16
		mov	[edi],ebx
		add	edi,4

		dec	ecx
		jnz	??interpolate_loop

; do the last three pixels and a blank on the end of a row
		xor	eax,eax
		mov	ax,word ptr [esi]
		mov	[edi],al
		inc	edi
		lea	esi,[esi+2]
		mov	al,[eax+PaletteInterpolationTable]
		mov	[edi],al
		inc	edi
		mov	[edi],ah
		inc	edi
		mov	byte ptr[edi],0

		mov	edi,[dest_width]
		add	[old_dest],edi

		dec	[source_height]
		jnz	??each_line_loop

		popad
		ret

Asm_Interpolate endp














Asm_Interpolate_Line_Double PROC near \
		, src_ptr:dword \
		, dest_ptr:dword \
		, source_height:dword \
		, source_width:dword \
		, dest_width:dword \

		LOCAL	old_dest:dword
		LOCAL	width_counter:dword
		LOCAL	pixel_count:dword

		pushad

		mov	eax,[dest_ptr]
		mov	[old_dest],eax

		mov	esi,[src_ptr]
		mov	edi,[dest_ptr]

??each_line_loop:
		mov	[width_counter],0
		mov	ecx,[source_width]
		sub	ecx,2
		shr	ecx,1
		mov	[pixel_count],ecx
		mov	ecx,offset LineBuffer
		mov	edi,[old_dest]
		jmp	??interpolate_loop
		_align 8 ;align	16
		_align_

; convert 2 pixels of source into 4 pixels of destination
??interpolate_loop:
		mov	eax,dword ptr [esi]
		lea	esi,[esi+2]
		mov	edx,eax
		mov	ebx,eax
		and	edx,65535
		ror	ebx,8
		mov	bl,[edx+PaletteInterpolationTable]
		mov	bh,ah
		and	eax,000ffff00h
		ror	ebx,8

		;1st 3 pixels now in ebx
		shr	eax,8
		mov	bh,[eax+PaletteInterpolationTable]
		ror	ebx,16
		mov	[edi],ebx
		mov	[ecx],ebx
		add	edi,4
		add	ecx,4

		dec	[pixel_count]
		jnz	??interpolate_loop

; do the last three pixels and a blank

		xor	eax,eax
		mov	ax,word ptr[esi]
		mov	[edi],al
		mov	[ecx],al
		inc	edi
		inc	ecx
		lea	esi,[esi+2]
		mov	al,[eax+PaletteInterpolationTable]
		mov	[edi],al
		mov	[ecx],al
		inc	edi
		inc	ecx
		mov	[edi],ah
		mov	[ecx],ah
		inc	edi
		inc	ecx
		mov	byte ptr[edi],0
		mov	byte ptr[ecx],0

		mov	edi,[dest_width]
		shr	edi,1
		add	[old_dest],edi
		push	esi
		push	edi
		mov	esi,offset LineBuffer
		mov	edi,[old_dest]
		mov	ecx,[source_width]
		shr	ecx,1
		rep	movsd
		pop	edi
		pop	esi
		add	[old_dest],edi
		mov	edi,[old_dest]

		dec	[source_height]
		jnz	??each_line_loop

		popad
		ret

Asm_Interpolate_Line_Double endp









		;ends

		_TEXT$mycode ends
		.data;dataseg

TopLine		dd	640 dup (?)
BottomLine	dd	640 dup (?)
LineBuffer	dd	640 dup (?)

		_TEXT$mycode segment page public use32 'CODE' ; segment mycode page public use32 'code'	; Need stricter segment alignment


Interpolate_Single_Line proc near \
		\
		, source_ptr:dword \
		, dest_ptr:dword \
		, source_width:dword

		pushad

		mov	ecx,[source_width]
		sub	ecx,2
		shr	ecx,1

		mov	esi,[source_ptr]
		mov	edi,[dest_ptr]

??interpolate_loop:
		mov	eax,dword ptr[esi]
		lea	esi,[esi+2]
		mov	edx,eax
		mov	ebx,eax
		and	edx,65535
		ror	ebx,8
		mov	bl,[edx+PaletteInterpolationTable]
		mov	bh,ah
		and	eax,000ffff00h
		ror	ebx,8

		;1st 3 pixels now in ebx
		shr	eax,8
		mov	bh,[eax+PaletteInterpolationTable]
		ror	ebx,16
		mov	[edi],ebx
		add	edi,4

		dec	ecx
		jnz	??interpolate_loop

; do the last three pixels and a blank

		xor	eax,eax
		mov	ax,word ptr[esi]
		mov	[edi],al
		inc	edi
		mov	al,[eax+PaletteInterpolationTable]
		mov	[edi],al
		inc	edi
		mov	[edi],ah
		inc	edi
		mov	byte ptr[edi],0

		popad
		ret


Interpolate_Single_Line endp


Interpolate_Between_Lines proc near \
		\
		, source1:dword \
		, source2:dword \
		, destination:dword \
		, source_width:dword

		pushad
		mov	esi,[source1]
		mov	edi,[destination]
		mov	ebx,[source2]
		xor	eax,eax
		mov	ecx,[source_width]
		add	ecx,ecx

??interpolate_each_pixel_loop:
		mov	al,byte ptr[esi]
		mov	ah,[ebx]
		inc	esi
		inc	ebx
		mov	dl,[eax+PaletteInterpolationTable]
		mov	[edi],dl
		inc	edi
		dec	ecx
		jnz	??interpolate_each_pixel_loop

		popad
		ret

Interpolate_Between_Lines endp




Lineswp macro
		push	[next_line]
		push	[last_line]
		pop	[next_line]
		pop	[last_line]
endm


Asm_Interpolate_Line_Interpolate PROC near \
		\
		\
		, src_ptr:dword \
		, dest_ptr:dword \
		, source_lines:dword \
		, source_width:dword \
		, dest_width:dword \

		LOCAL	old_dest:dword
		LOCAL	pixel_count:dword
		LOCAL	next_line:dword
		LOCAL	last_line:dword

		pushad

		mov	eax,[dest_ptr]
		mov	[old_dest],eax

		mov	[next_line],offset TopLine
		mov	[last_line],offset BottomLine
		mov	ecx,[source_width]
		shr	ecx,1
		mov	[pixel_count],ecx
		shr	[dest_width],1

		invoke	Interpolate_Single_Line,[src_ptr],[next_line],[source_width]
		mov	esi,[source_width]
		Lineswp
		add	[src_ptr],esi
		dec	[source_lines]


??each_line_pair_loop:
		invoke	Interpolate_Single_Line,[src_ptr],[next_line],[source_width]
		invoke	Interpolate_Between_Lines,[last_line],[next_line],offset LineBuffer,[source_width]

		mov	esi,[last_line]
		mov	edi,[old_dest]
		mov	ecx,[pixel_count]
		rep	movsd

		mov	edi,[old_dest]
		mov	esi,offset LineBuffer
		add	edi,[dest_width]
		mov	ecx,[pixel_count]
		mov	[old_dest],edi
		rep	movsd

		mov	edi,[old_dest]
		mov	esi,[source_width]
		add	edi,[dest_width]
		add	[src_ptr],esi
		mov	[old_dest],edi

		Lineswp
		dec	[source_lines]
		jnz	??each_line_pair_loop

		invoke	Interpolate_Single_Line,[src_ptr],[next_line],[source_width]
		mov	esi,[next_line]
		mov	edi,[old_dest]
		mov	ecx,[pixel_count]
		rep	movsd

		popad
		ret


Asm_Interpolate_Line_Interpolate endp

		;ends	mycode




public		C Asm_Create_Palette_Interpolation_Table
extern		C InterpolationPalette:dword

		;codeseg


Asm_Create_Palette_Interpolation_Table proc near

		LOCAL	palette_counter:dword
		LOCAL	first_palette:dword
		LOCAL	second_palette:dword
		LOCAL	dest_ptr:dword
		LOCAL	count:dword
		LOCAL	closest_colour:dword
		LOCAL	distance_of_closest:dword

		pushad

		mov	[dest_ptr],0
		mov	[palette_counter],256
		mov	esi,[InterpolationPalette]

??palette_outer_loop:
		mov	edi,[InterpolationPalette]
		mov	ecx,256

??palette_inner_loop:
		mov	bl,byte ptr[esi]
		add	bl,[edi]
		shr	bl,1

		mov	bh,byte ptr[esi+1]
		add	bh,[edi+1]
		shr	bh,1

		mov	dl,byte ptr[esi+2]
		add	dl,[edi+2]
		shr	dl,1

		mov	[closest_colour],0
		mov	[distance_of_closest],-1

		push	edi
		push	ecx
		mov	edi,[InterpolationPalette]
		mov	[count],0

??cmp_pal_lp:	xor	eax,eax
		xor	ecx,ecx
		mov	al,[edi]
		sub	al,bl
		imul	al
		mov	ecx,eax
		mov	al,[edi+1]
		sub	al,bh
		imul	al
		add	ecx,eax
		mov	al,[edi+2]
		sub	al,dl
		imul	al
		add	ecx,eax

		cmp	ecx,[distance_of_closest]
		ja	??end_cmp_lp
		mov	[distance_of_closest],ecx
		mov	eax,[count]
		mov	[closest_colour],eax
		test	ecx,ecx
		jz	??got_perfect

??end_cmp_lp:	lea	edi,[edi+3]
		inc	[count]
		cmp	[count],256
		jb	??cmp_pal_lp


??got_perfect:	mov	edi,[dest_ptr]
		mov	eax,[closest_colour]
		mov	[edi+PaletteInterpolationTable],al
		inc	[dest_ptr]

		pop	ecx
		pop	edi
		lea	edi,[edi+3]
		dec	ecx
		jnz	??palette_inner_loop

		lea	esi,[esi+3]
		dec	[palette_counter]
		jnz	??palette_outer_loop

		popad
		ret

Asm_Create_Palette_Interpolation_Table endp


		_TEXT$mycode ends

end
