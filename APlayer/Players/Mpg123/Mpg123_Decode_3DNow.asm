; Decode_3DNow.asm - 3DNow! optimized synth_1to1()
;
; This code based 'Decode_3DNow.asm' by Syuuhei Kashiyama
; <squash@mb.kcom.ne.jp>, only two types of changes have been made:
;
; - Remove PREFETCH instruction for speedup
; - Change function name for support 3DNow! automatic detect
;
; You can find Kashiyama's original 3DNow! support patch
; (for mpg123-0.59o) at
; http://user.ecc.u-tokyo.ac.jp/~g810370/linux-simd/ (Japanese).
;
; By KIMURA Takuhiro <kim@hannah.ipc.miyakyo-u.ac.jp> - until 31.Mar.1999
;                    <kim@comtec.co.jp>               - after  1.Apr.1999
;
;
; Replacement of Synth_1to1() with AMD's 3DNow! SIMD operations support
;
; Syuuhei Kashiyama <squash@mb.kcom.ne.jp>
;
; The author of this program disclaim whole expressed or implied
; warranties with regard to this program, and in no event shall the
; author of this program liable to whatever resulted from the use of
; this program. Use it at your own risk.
;

; Imports
EXTERN Dct64_3DNow

; Exports
GLOBAL Synth_1to1_3DNow

; Code
SEGMENT .text align=32 class=CODE use32

Synth_1to1_3DNow:
	sub		esp, 24
	push	ebp
	push	edi
	xor		ebp, ebp
	push	esi
	push	ebx
	mov		esi, [esp + 56]
	mov		edi, [esp + 52]
	mov		esi, [esi + 0]
	mov		ebx, [esp + 48]
	add		esi, edi
	mov		[esp + 16], esi

	femms

	mov		edx, [esp + 64]				;Added by Thomas Neumann (bo)
	test	ebx, ebx
	jne		.L26
	dec		dword [edx]					;dec bo
	mov		ecx, [esp + 60]				;mov ecx, buffs (load address to buffs)
	and		dword [edx], 15				;and bo, 15
	jmp		.L27
.L26:
;	add		dword [esp + 16], 2			;Removed by Thomas Neumann (samples++)
	mov		ecx, [esp + 60]				;mov ecx, buffs + 2176
	add		ecx, 2176					;
.L27:
	mov		edx, [edx]					;mov edx, bo
	test	dl, 1
	je		.L28
	push	dword [esp + 72]			;Added by Thomas Neumann (pnts *)
	mov		[esp + 40], edx				;mov [esp + 36], edx
	mov		ebx, ecx
	mov		esi, [esp + 48]				;mov esi, [esp + 44]
	mov		edi, edx
	push	esi							;bandPtr
	sal		edi, 2
	mov		eax, ebx
	mov		[esp + 28], edi				;mov [esp + 24], edi
	add		eax, edi
	push	eax							;buf[0] + bo
	mov		eax, edx
	inc		eax
	and		eax, 15						;(bo + 1) & 15
	lea		eax, [eax * 4 + 1088]
	add		eax, ebx					;+ buf[1]
	push	eax
	call	Dct64_3DNow
	add		esp, 16						;add esp, 12
	jmp		.L29
.L28:
	push	dword [esp + 72]			;Added by Thomas Neumann (pnts *)
	lea		esi, [edx + 1]				;bo + 1
	mov		edi, [esp + 48]				;mov edi, [esp + 44]
	mov		[esp + 40], esi				;mov [esp + 36], esi
	lea		eax, [ecx + edx * 4 + 1092]	;buf[1] + bo + 1
	push	edi							;bandPtr
	lea		ebx, [ecx + 1088]
	push	eax
	sal		esi, 2
	lea		eax, [ecx + edx * 4]
	push	eax							;buf[0] + bo
	call	Dct64_3DNow
	add		esp, 16						;add esp, 12
	mov		[esp + 20], esi
.L29:
	mov		edx, [esp + 68]				;mov edx, decWin + 64
	add		edx, 64						;
	mov		ecx, 16
	sub		edx, [esp + 20]
	mov		edi, [esp + 16]

	movq	mm0, [edx]
	movq	mm1, [ebx]
.L33:
	movq	mm3, [edx + 8]
	pfmul	mm0, mm1
	movq	mm4, [ebx + 8]
	movq	mm5, [edx + 16]
	pfmul	mm3, mm4
	movq	mm6, [ebx + 16]
	pfadd	mm0, mm3
	movq	mm1, [edx + 24]
	pfmul	mm5, mm6
	movq	mm2, [ebx + 24]
	pfadd	mm0, mm5
	movq	mm3, [edx + 32]
	pfmul	mm1, mm2
	movq	mm4, [ebx + 32]
	pfadd	mm0, mm1
	movq	mm5, [edx + 40]
	pfmul	mm3, mm4
	movq	mm6, [ebx + 40]
	pfadd	mm0, mm3
	movq	mm1, [edx + 48]
	pfmul	mm5, mm6
	movq	mm2, [ebx + 48]
	pfadd	mm5, mm0
	movq	mm3, [edx + 56]
	pfmul	mm2, mm1
	movq	mm4, [ebx + 56]
	pfadd	mm2, mm5
	add		ebx, 64
	sub		edx, -128
	movq	mm0, [edx]
	pfmul	mm3, mm4
	movq	mm1, [ebx]
	pfadd	mm2, mm3
	movq	mm3, mm2
	psrlq	mm3, 32
	pfsub	mm2, mm3
	inc		ebp
	pf2id	mm2, mm2
	packssdw mm2, mm2
	movd	eax, mm2
	mov		[edi + 0], ax
	add		edi, 2						;add edi, 4
	dec		ecx
	jnz		near .L33

	movd	mm0, [ebx]
	movd	mm1, [edx]
	punpckldq mm0, [ebx + 8]
	punpckldq mm1, [edx + 8]
	movd	mm3, [ebx + 16]
	movd	mm4, [edx + 16]
	pfmul	mm0, mm1
	punpckldq mm3, [ebx + 24]
	punpckldq mm4, [edx + 24]
	movd	mm5, [ebx + 32]
	movd	mm6, [edx + 32]
	pfmul	mm3, mm4
	punpckldq mm5, [ebx + 40]
	punpckldq mm6, [edx + 40]
	pfadd	mm0, mm3
	movd	mm1, [ebx + 48]
	movd	mm2, [edx + 48]
	pfmul	mm5, mm6
	punpckldq mm1, [ebx + 56]
	punpckldq mm2, [edx + 56]
	pfadd	mm0, mm5
	pfmul	mm1, mm2
	pfadd	mm0, mm1
	pfacc	mm0, mm1
	pf2id	mm0, mm0
	packssdw mm0, mm0
	movd	eax, mm0
	mov		[edi + 0], ax
	inc		ebp
	mov		esi, [esp + 36]
	add		ebx, -64
	mov		ebp, 15
	add		edi, 2						;add edi, 4
	lea		edx, [edx + esi * 8 - 128]

	mov		ecx, 15
	movd	mm0, [ebx]
	movd	mm1, [edx - 4]
	punpckldq mm0, [ebx + 4]
	punpckldq mm1, [edx - 8]
.L46:
	movd	mm3, [ebx + 8]
	movd	mm4, [edx - 12]
	pfmul	mm0, mm1
	punpckldq mm3, [ebx + 12]
	punpckldq mm4, [edx - 16]
	movd	mm5, [ebx + 16]
	movd	mm6, [edx - 20]
	pfmul	mm3, mm4
	punpckldq mm5, [ebx + 20]
	punpckldq mm6, [edx - 24]
	pfadd	mm0, mm3
	movd	mm1, [ebx + 24]
	movd	mm2, [edx - 28]
	pfmul	mm5, mm6
	punpckldq mm1, [ebx + 28]
	punpckldq mm2, [edx - 32]
	pfadd	mm0, mm5
	movd	mm3, [ebx + 32]
	movd	mm4, [edx - 36]
	pfmul	mm1, mm2
	punpckldq mm3, [ebx + 36]
	punpckldq mm4, [edx - 40]
	pfadd	mm0, mm1
	movd	mm5, [ebx + 40]
	movd	mm6, [edx - 44]
	pfmul	mm3, mm4
	punpckldq mm5, [ebx + 44]
	punpckldq mm6, [edx - 48]
	pfadd	mm0, mm3
	movd	mm1, [ebx + 48]
	movd	mm2, [edx - 52]
	pfmul	mm5, mm6
	punpckldq mm1, [ebx + 52]
	punpckldq mm2, [edx - 56]
	pfadd	mm5, mm0
	movd	mm3, [ebx + 56]
	movd	mm4, [edx - 60]
	pfmul	mm1, mm2
	punpckldq mm3, [ebx + 60]
	punpckldq mm4, [edx]
	pfadd	mm5, mm1
	add		edx, -128
	add		ebx, -64
	movd	mm0, [ebx]
	movd	mm1, [edx - 4]
	pfmul	mm3, mm4
	punpckldq mm0, [ebx + 4]
	punpckldq mm1, [edx - 8]
	pfadd	mm3, mm5
	pfacc	mm3, mm3
	inc		ebp
	pf2id	mm3, mm3
	movd	eax, mm3
	neg		eax
	movd	mm3, eax
	packssdw mm3, mm3
	movd	eax, mm3
	mov 	[edi], ax
	add		edi, 2						;add edi, 4
	dec		ecx
	jnz		near .L46

	femms
	mov		esi, [esp + 56]
	mov		eax, ebp
	sub		dword [esi + 0], -64		;sub dword [esi + 0], -128
	pop		ebx
	pop		esi
	pop		edi
	pop		ebp
	add		esp, 24
	ret

end
