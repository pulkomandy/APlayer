; Replacement of Dct64() with AMD's 3DNow! SIMD operations support
;
; Syuuhei Kashiyama <squash@mb.kcom.ne.jp>
;
; The author of this program disclaim whole expressed or implied
; warranties with regard to this program, and in no event shall the
; author of this program liable to whatever resulted from the use of
; this program. Use it at your own risk.
;

; Exports
GLOBAL Dct64_3DNow

; Code
SEGMENT .text align=32 class=CODE use32

Dct64_3DNow:
	sub		esp, 256
	push	ecx						;Added by Thomas Neumann
	push	ebp
	push	edi
	push	esi
	push	ebx
	lea		ebx, [esp + 20]			;lea ebx, [esp + 16]
	mov		edi, [esp + 288]		;mov edi, [esp + 284]
	mov		ebp, [esp + 280]		;mov ebp, [esp + 276]
	mov		edx, [esp + 284]		;mov edx, [esp + 280]
	mov		ecx, [esp + 292]		;Added by Thomas Neumann (pnts *)
	lea		esi, [ebx + 128]

	; 1
	mov		eax, [ecx]				;mov eax, pnts
	movq	mm0, [edi + 0]
	movq	mm1, mm0
	movd	mm2, [edi + 124]
	punpckldq mm2, [edi + 120]
	movq	mm3, [eax + 0]
	pfadd	mm0, mm2
	movq	[ebx + 0], mm0
	pfsub	mm1, mm2
	pfmul	mm1, mm3
	movd	[ebx + 124], mm1
	psrlq	mm1, 32
	movd	[ebx + 120], mm1
	movq	mm4, [edi + 8]
	movq	mm5, mm4
	movd	mm6, [edi + 116]
	punpckldq mm6, [edi + 112]
	movq	mm7, [eax + 8]
	pfadd	mm4, mm6
	movq	[ebx + 8], mm4
	pfsub	mm5, mm6
	pfmul	mm5, mm7
	movd	[ebx + 116], mm5
	psrlq	mm5, 32
	movd	[ebx + 112], mm5
	movq	mm0, [edi + 16]
	movq	mm1, mm0
	movd	mm2, [edi + 108]
	punpckldq mm2, [edi + 104]
	movq	mm3, [eax + 16]
	pfadd	mm0, mm2
	movq	[ebx + 16], mm0
	pfsub	mm1, mm2
	pfmul	mm1, mm3
	movd	[ebx + 108], mm1
	psrlq	mm1, 32
	movd	[ebx + 104], mm1
	movq	mm4, [edi + 24]
	movq	mm5, mm4
	movd	mm6, [edi + 100]
	punpckldq mm6, [edi + 96]
	movq	mm7, [eax + 24]
	pfadd	mm4, mm6
	movq	[ebx + 24], mm4
	pfsub	mm5, mm6
	pfmul	mm5, mm7
	movd	[ebx + 100], mm5
	psrlq	mm5, 32
	movd	[ebx + 96], mm5
	movq	mm0, [edi + 32]
	movq	mm1, mm0
	movd	mm2, [edi + 92]
	punpckldq mm2, [edi + 88]
	movq	mm3, [eax + 32]
	pfadd	mm0, mm2
	movq	[ebx + 32], mm0
	pfsub	mm1, mm2
	pfmul	mm1, mm3
	movd	[ebx + 92], mm1
	psrlq	mm1, 32
	movd	[ebx + 88], mm1
	movq	mm4, [edi + 40]
	movq	mm5, mm4
	movd	mm6, [edi + 84]
	punpckldq mm6, [edi + 80]
	movq	mm7, [eax + 40]
	pfadd	mm4, mm6
	movq	[ebx + 40], mm4
	pfsub	mm5, mm6
	pfmul	mm5, mm7
	movd	[ebx + 84], mm5
	psrlq	mm5, 32
	movd	[ebx + 80], mm5
	movq	mm0, [edi + 48]
	movq	mm1, mm0
	movd	mm2, [edi + 76]
	punpckldq mm2, [edi + 72]
	movq	mm3, [eax + 48]
	pfadd	mm0, mm2
	movq	[ebx + 48], mm0
	pfsub	mm1, mm2
	pfmul	mm1, mm3
	movd	[ebx + 76], mm1
	psrlq	mm1, 32
	movd	[ebx + 72], mm1
	movq	mm4, [edi + 56]
	movq	mm5, mm4
	movd	mm6, [edi + 68]
	punpckldq mm6, [edi + 64]
	movq	mm7, [eax + 56]
	pfadd	mm4, mm6
	movq	[ebx + 56], mm4
	pfsub	mm5, mm6
	pfmul	mm5, mm7
	movd	[ebx + 68], mm5
	psrlq	mm5, 32
	movd	[ebx + 64], mm5

	; 2
	mov		eax, [ecx + 4]			;mov eax, pnts + 4
	; 0, 14
	movq	mm0, [ebx + 0]
	movq	mm1, mm0
	movd	mm2, [ebx + 60]
	punpckldq mm2, [ebx + 56]
	movq	mm3, [eax + 0]
	pfadd	mm0, mm2
	movq	[esi + 0], mm0
	pfsub	mm1, mm2
	pfmul	mm1, mm3
	movd	[esi + 60], mm1
	psrlq	mm1, 32
	movd	[esi + 56], mm1
	; 16, 30
	movq	mm0, [ebx + 64]
	movq	mm1, mm0
	movd	mm2, [ebx + 124]
	punpckldq mm2, [ebx + 120]
	pfadd	mm0, mm2
	movq	[esi + 64], mm0
	pfsubr	mm1, mm2
	pfmul	mm1, mm3
	movd	[esi + 124], mm1
	psrlq	mm1, 32
	movd	[esi + 120], mm1
	; 2, 12
	movq	mm4, [ebx + 8]
	movq	mm5, mm4
	movd	mm6, [ebx + 52]
	punpckldq mm6, [ebx + 48]
	movq	mm7, [eax + 8]
	pfadd	mm4, mm6
	movq	[esi + 8], mm4
	pfsub	mm5, mm6
	pfmul	mm5, mm7
	movd	[esi + 52], mm5
	psrlq	mm5, 32
	movd	[esi + 48], mm5
	; 18, 28
	movq	mm4, [ebx + 72]
	movq	mm5, mm4
	movd	mm6, [ebx + 116]
	punpckldq mm6, [ebx + 112]
	pfadd	mm4, mm6
	movq	[esi + 72], mm4
	pfsubr	mm5, mm6
	pfmul	mm5, mm7
	movd	[esi + 116], mm5
	psrlq	mm5, 32
	movd	[esi + 112], mm5
	; 4, 10
	movq	mm0, [ebx + 16]
	movq	mm1, mm0
	movd	mm2, [ebx + 44]
	punpckldq mm2, [ebx + 40]
	movq	mm3, [eax + 16]
	pfadd	mm0, mm2
	movq	[esi + 16], mm0
	pfsub	mm1, mm2
	pfmul	mm1, mm3
	movd	[esi + 44], mm1
	psrlq	mm1, 32
	movd	[esi + 40], mm1
	; 20, 26
	movq	mm0, [ebx + 80]
	movq	mm1, mm0
	movd	mm2, [ebx + 108]
	punpckldq mm2, [ebx + 104]
	pfadd	mm0, mm2
	movq	[esi + 80], mm0
	pfsubr	mm1, mm2
	pfmul	mm1, mm3
	movd	[esi + 108], mm1
	psrlq	mm1, 32
	movd	[esi + 104], mm1
	; 6, 8
	movq	mm4, [ebx + 24]
	movq	mm5, mm4
	movd	mm6, [ebx + 36]
	punpckldq mm6, [ebx + 32]
	movq	mm7, [eax + 24]
	pfadd	mm4, mm6
	movq	[esi + 24], mm4
	pfsub	mm5, mm6
	pfmul	mm5, mm7
	movd	[esi + 36], mm5
	psrlq	mm5, 32
	movd	[esi + 32], mm5
	; 22, 24
	movq	mm4, [ebx + 88]
	movq	mm5, mm4
	movd	mm6, [ebx + 100]
	punpckldq mm6, [ebx + 96]
	pfadd	mm4, mm6
	movq	[esi + 88], mm4
	pfsubr	mm5, mm6
	pfmul	mm5, mm7
	movd	[esi + 100], mm5
	psrlq	mm5, 32
	movd	[esi + 96], mm5

	; 3
	mov		eax, [ecx + 8]			;mov eax, pnts + 8
	movq	mm0, [eax + 0]
	movq	mm1, [eax + 8]
	; 0, 6
	movq	mm2, [esi + 0]
	movq	mm3, mm2
	movd	mm4, [esi + 28]
	punpckldq mm4, [esi + 24]
	pfadd	mm2, mm4
	pfsub	mm3, mm4
	pfmul	mm3, mm0
	movq	[ebx + 0], mm2
	movd	[ebx + 28], mm3
	psrlq	mm3, 32
	movd	[ebx + 24], mm3
	; 2, 4
	movq	mm5, [esi + 8]
	movq	mm6, mm5
	movd	mm7, [esi + 20]
	punpckldq mm7, [esi + 16]
	pfadd	mm5, mm7
	pfsub	mm6, mm7
	pfmul	mm6, mm1
	movq	[ebx + 8], mm5
	movd	[ebx + 20], mm6
	psrlq	mm6, 32
	movd	[ebx + 16], mm6
	; 8, 14
	movq	mm2, [esi + 32]
	movq	mm3, mm2
	movd	mm4, [esi + 60]
	punpckldq mm4, [esi + 56]
	pfadd	mm2, mm4
	pfsubr	mm3, mm4
	pfmul	mm3, mm0
	movq	[ebx + 32], mm2
	movd	[ebx + 60], mm3
	psrlq	mm3, 32
	movd	[ebx + 56], mm3
	; 10, 12
	movq	mm5, [esi + 40]
	movq	mm6, mm5
	movd	mm7, [esi + 52]
	punpckldq mm7, [esi + 48]
	pfadd	mm5, mm7
	pfsubr	mm6, mm7
	pfmul	mm6, mm1
	movq	[ebx + 40], mm5
	movd	[ebx + 52], mm6
	psrlq	mm6, 32
	movd	[ebx + 48], mm6
	; 16, 22
	movq	mm2, [esi + 64]
	movq	mm3, mm2
	movd	mm4, [esi + 92]
	punpckldq mm4, [esi + 88]
	pfadd	mm2, mm4
	pfsub	mm3, mm4
	pfmul	mm3, mm0
	movq	[ebx + 64], mm2
	movd	[ebx + 92], mm3
	psrlq	mm3, 32
	movd	[ebx + 88], mm3
	; 18, 20
	movq	mm5, [esi + 72]
	movq	mm6, mm5
	movd	mm7, [esi + 84]
	punpckldq mm7, [esi + 80]
	pfadd	mm5, mm7
	pfsub	mm6, mm7
	pfmul	mm6, mm1
	movq	[ebx + 72], mm5
	movd	[ebx + 84], mm6
	psrlq	mm6, 32
	movd	[ebx + 80], mm6
	; 24, 30
	movq	mm2, [esi + 96]
	movq	mm3, mm2
	movd	mm4, [esi + 124]
	punpckldq mm4, [esi + 120]
	pfadd	mm2, mm4
	pfsubr	mm3, mm4
	pfmul	mm3, mm0
	movq	[ebx + 96], mm2
	movd	[ebx + 124], mm3
	psrlq	mm3, 32
	movd	[ebx + 120], mm3
	; 26, 28
	movq	mm5, [esi + 104]
	movq	mm6, mm5
	movd	mm7, [esi + 116]
	punpckldq mm7, [esi + 112]
	pfadd	mm5, mm7
	pfsubr	mm6, mm7
	pfmul	mm6, mm1
	movq	[ebx + 104], mm5
	movd	[ebx + 116], mm6
	psrlq	mm6, 32
	movd	[ebx + 112], mm6

	; 4
	mov		eax, [ecx + 12]			;mov eax, pnts + 12
	movq	mm0, [eax + 0]
	; 0
	movq	mm1, [ebx + 0]
	movq	mm2, mm1
	movd	mm3, [ebx + 12]
	punpckldq mm3, [ebx + 8]
	pfadd	mm1, mm3
	pfsub	mm2, mm3
	pfmul	mm2, mm0
	movq	[esi + 0], mm1
	movd	[esi + 12], mm2
	psrlq	mm2, 32
	movd	[esi + 8], mm2
	; 4
	movq	mm4, [ebx + 16]
	movq	mm5, mm4
	movd	mm6, [ebx + 28]
	punpckldq mm6, [ebx + 24]
	pfadd	mm4, mm6
	pfsubr	mm5, mm6
	pfmul	mm5, mm0
	movq	[esi + 16], mm4
	movd	[esi + 28], mm5
	psrlq	mm5, 32
	movd	[esi + 24], mm5
	; 8
	movq	mm1, [ebx + 32]
	movq	mm2, mm1
	movd	mm3, [ebx + 44]
	punpckldq mm3, [ebx + 40]
	pfadd	mm1, mm3
	pfsub	mm2, mm3
	pfmul	mm2, mm0
	movq	[esi + 32], mm1
	movd	[esi + 44], mm2
	psrlq	mm2, 32
	movd	[esi + 40], mm2
	; 12
	movq	mm4, [ebx + 48]
	movq	mm5, mm4
	movd	mm6, [ebx + 60]
	punpckldq mm6, [ebx + 56]
	pfadd	mm4, mm6
	pfsubr	mm5, mm6
	pfmul	mm5, mm0
	movq	[esi + 48], mm4
	movd	[esi + 60], mm5
	psrlq	mm5, 32
	movd	[esi + 56], mm5
	; 16
	movq	mm1, [ebx + 64]
	movq	mm2, mm1
	movd	mm3, [ebx + 76]
	punpckldq mm3, [ebx + 72]
	pfadd	mm1, mm3
	pfsub	mm2, mm3
	pfmul	mm2, mm0
	movq	[esi + 64], mm1
	movd	[esi + 76], mm2
	psrlq	mm2, 32
	movd	[esi + 72], mm2
	; 20
	movq	mm4, [ebx + 80]
	movq	mm5, mm4
	movd	mm6, [ebx + 92]
	punpckldq mm6, [ebx + 88]
	pfadd	mm4, mm6
	pfsubr	mm5, mm6
	pfmul	mm5, mm0
	movq	[esi + 80], mm4
	movd	[esi + 92], mm5
	psrlq	mm5, 32
	movd	[esi + 88], mm5
	; 24
	movq	mm1, [ebx + 96]
	movq	mm2, mm1
	movd	mm3, [ebx + 108]
	punpckldq mm3, [ebx + 104]
	pfadd	mm1, mm3
	pfsub	mm2, mm3
	pfmul	mm2, mm0
	movq	[esi + 96], mm1
	movd	[esi + 108], mm2
	psrlq	mm2, 32
	movd	[esi + 104], mm2
	; 28
	movq	mm4, [ebx + 112]
	movq	mm5, mm4
	movd	mm6, [ebx + 124]
	punpckldq mm6, [ebx + 120]
	pfadd	mm4, mm6
	pfsubr	mm5, mm6
	pfmul	mm5, mm0
	movq	[esi + 112], mm4
	movd	[esi + 124], mm5
	psrlq	mm5, 32
	movd	[esi + 120], mm5

	; 5
	mov		eax, -1
	movd	mm1, eax
	mov		eax, 1
	; L | H
	movd	mm0, eax
	punpckldq mm0, mm1
	; 1.0 | -1.0
	pi2fd	mm0, mm0
	movd	mm1, eax
	pi2fd	mm1, mm1
	mov		eax, [ecx + 16]			;mov eax, pnts + 16
	movd	mm2, [eax + 0]
	; 1.0 | cos0
	punpckldq mm1, mm2
	; 0
	movq	mm2, [esi + 0]
	movq	mm3, mm2
	pfmul	mm3, mm0
	pfacc	mm2, mm3
	pfmul	mm2, mm1
	movq	[ebx + 0], mm2
	movq	mm4, [esi + 8]
	movq	mm5, mm4
	pfmul	mm5, mm0
	pfacc	mm4, mm5
	pfmul	mm4, mm0
	pfmul	mm4, mm1
	movq	mm5, mm4
	psrlq	mm5, 32
	pfacc	mm4, mm5
	movq	[ebx + 8], mm4
	; 4
	movq	mm2, [esi + 16]
	movq	mm3, mm2
	pfmul	mm3, mm0
	pfacc	mm2, mm3
	pfmul	mm2, mm1
	movq	mm4, [esi + 24]
	movq	mm5, mm4
	pfmul	mm5, mm0
	pfacc	mm4, mm5
	pfmul	mm4, mm0
	pfmul	mm4, mm1
	movq	mm5, mm4
	psrlq	mm5, 32
	pfacc	mm4, mm5
	movq	mm3, mm2
	psrlq	mm3, 32
	pfadd	mm2, mm4
	pfadd	mm4, mm3
	movq	[ebx + 16], mm2
	movq	[ebx + 24], mm4
	; 8
	movq	mm2, [esi + 32]
	movq	mm3, mm2
	pfmul	mm3, mm0
	pfacc	mm2, mm3
	pfmul	mm2, mm1
	movq	[ebx + 32], mm2
	movq	mm4, [esi + 40]
	movq	mm5, mm4
	pfmul	mm5, mm0
	pfacc	mm4, mm5
	pfmul	mm4, mm0
	pfmul	mm4, mm1
	movq	mm5, mm4
	psrlq	mm5, 32
	pfacc	mm4, mm5
	movq	[ebx + 40], mm4
	; 12
	movq	mm2, [esi + 48]
	movq	mm3, mm2
	pfmul	mm3, mm0
	pfacc	mm2, mm3
	pfmul	mm2, mm1
	movq	mm4, [esi + 56]
	movq	mm5, mm4
	pfmul	mm5, mm0
	pfacc	mm4, mm5
	pfmul	mm4, mm0
	pfmul	mm4, mm1
	movq	mm5, mm4
	psrlq	mm5, 32
	pfacc	mm4, mm5
	movq	mm3, mm2
	psrlq	mm3, 32
	pfadd	mm2, mm4
	pfadd	mm4, mm3
	movq	[ebx + 48], mm2
	movq	[ebx + 56], mm4
	; 16
	movq	mm2, [esi + 64]
	movq	mm3, mm2
	pfmul	mm3, mm0
	pfacc	mm2, mm3
	pfmul	mm2, mm1
	movq	[ebx + 64], mm2
	movq	mm4, [esi + 72]
	movq	mm5, mm4
	pfmul	mm5, mm0
	pfacc	mm4, mm5
	pfmul	mm4, mm0
	pfmul	mm4, mm1
	movq	mm5, mm4
	psrlq	mm5, 32
	pfacc	mm4, mm5
	movq	[ebx + 72], mm4
	; 20
	movq	mm2, [esi + 80]
	movq	mm3, mm2
	pfmul	mm3, mm0
	pfacc	mm2, mm3
	pfmul	mm2, mm1
	movq	mm4, [esi + 88]
	movq	mm5, mm4
	pfmul	mm5, mm0
	pfacc	mm4, mm5
	pfmul	mm4, mm0
	pfmul	mm4, mm1
	movq	mm5, mm4
	psrlq	mm5, 32
	pfacc	mm4, mm5
	movq	mm3, mm2
	psrlq	mm3, 32
	pfadd	mm2, mm4
	pfadd	mm4, mm3
	movq	[ebx + 80], mm2
	movq	[ebx + 88], mm4
	; 24
	movq	mm2, [esi + 96]
	movq	mm3, mm2
	pfmul	mm3, mm0
	pfacc	mm2, mm3
	pfmul	mm2, mm1
	movq	[ebx + 96], mm2
	movq	mm4, [esi + 104]
	movq	mm5, mm4
	pfmul	mm5, mm0
	pfacc	mm4, mm5
	pfmul	mm4, mm0
	pfmul	mm4, mm1
	movq	mm5, mm4
	psrlq	mm5, 32
	pfacc	mm4, mm5
	movq	[ebx + 104], mm4
	; 28
	movq	mm2, [esi + 112]
	movq	mm3, mm2
	pfmul	mm3, mm0
	pfacc	mm2, mm3
	pfmul	mm2, mm1
	movq	mm4, [esi + 120]
	movq	mm5, mm4
	pfmul	mm5, mm0
	pfacc	mm4, mm5
	pfmul	mm4, mm0
	pfmul	mm4, mm1
	movq	mm5, mm4
	psrlq	mm5, 32
	pfacc	mm4, mm5
	movq	mm3, mm2
	psrlq	mm3, 32
	pfadd	mm2, mm4
	pfadd	mm4, mm3
	movq	[ebx + 112], mm2
	movq	[ebx + 120], mm4

	; Phase6
	mov		eax, [ebx + 0]
	mov		[ebp + 1024], eax
	mov		eax, [ebx + 4]
	mov		[ebp + 0], eax
	mov		[edx + 0], eax
	mov		eax, [ebx + 8]
	mov		[ebp + 512], eax
	mov		eax, [ebx + 12]
	mov		[edx + 512], eax

	mov		eax, [ebx + 16]
	mov		[ebp + 768], eax
	mov		eax, [ebx + 20]
	mov		[edx + 256], eax

	mov		eax, [ebx + 24]
	mov		[ebp + 256], eax
	mov		eax, [ebx + 28]
	mov		[edx + 768], eax

	movq	mm0, [ebx + 32]
	movq	mm1, [ebx + 48]
	pfadd	mm0, mm1
	movd	[ebp + 896], mm0
	psrlq	mm0, 32
	movd	[edx + 128], mm0
	movq	mm2, [ebx + 40]
	pfadd	mm1, mm2
	movd	[ebp + 640], mm1
	psrlq	mm1, 32
	movd	[edx + 384], mm1

	movq	mm3, [ebx + 56]
	pfadd	mm2, mm3
	movd	[ebp + 384], mm2
	psrlq	mm2, 32
	movd	[edx + 640], mm2

	movd	mm4, [ebx + 36]
	pfadd	mm3, mm4
	movd	[ebp + 128], mm3
	psrlq	mm3, 32
	movd	[edx + 896], mm3
	movq	mm0, [ebx + 96]
	movq	mm1, [ebx + 64]

	movq	mm2, [ebx + 112]
	pfadd	mm0, mm2
	movq	mm3, mm0
	pfadd	mm3, mm1
	movd	[ebp + 960], mm3
	psrlq	mm3, 32
	movd	[edx + 64], mm3
	movq	mm1, [ebx + 80]
	pfadd	mm0, mm1
	movd	[ebp + 832], mm0
	psrlq	mm0, 32
	movd	[edx + 192], mm0
	movq	mm3, [ebx + 104]
	pfadd	mm2, mm3
	movq	mm4, mm2
	pfadd	mm4, mm1
	movd	[ebp + 704], mm4
	psrlq	mm4, 32
	movd	[edx + 320], mm4
	movq	mm1, [ebx + 72]
	pfadd	mm2, mm1
	movd	[ebp + 576], mm2
	psrlq	mm2, 32
	movd	[edx + 448], mm2

	movq	mm4, [ebx + 120]
	pfadd	mm3, mm4
	movq	mm5, mm3
	pfadd	mm5, mm1
	movd	[ebp + 448], mm5
	psrlq	mm5, 32
	movd	[edx + 576], mm5
	movq	mm1, [ebx + 88]
	pfadd	mm3, mm1
	movd	[ebp + 320], mm3
	psrlq	mm3, 32
	movd	[edx + 704], mm3

	movd	mm5, [ebx + 100]
	pfadd	mm4, mm5
	movq	mm6, mm4
	pfadd	mm6, mm1
	movd	[ebp + 192], mm6
	psrlq	mm6, 32
	movd	[edx + 832], mm6
	movd	mm1, [ebx + 68]
	pfadd	mm4, mm1
	movd	[ebp + 64], mm4
	psrlq	mm4, 32
	movd	[edx + 960], mm4

	pop		ebx
	pop		esi
	pop		edi
	pop		ebp
	pop		ecx						;Added by Thomas Neumann
	add		esp, 256
	ret

end
