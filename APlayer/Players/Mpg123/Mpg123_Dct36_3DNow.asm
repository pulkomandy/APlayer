; Dct36_3DNow.asm - 3DNow! optimized dct36()
;
; This code based 'Dct36_3DNow.asm' by Syuuhei Kashiyama
; <squash@mb.kcom.ne.jp>, only two types of changes have been made:
;
; - Remove PREFETCH instruction for speedup
; - Change function name for support 3DNow! automatic detect
;
; Some other changes has been made by Thomas Neumann, and these are:
;
; - Added some extra arguments, so the function is C++ friendly
; - Changed all the references to cos9 and tfcos36 to the new arguments
;
; You can find Kashiyama's original 3DNow! support patch
; (for mpg123-0.59o) at
; http://user.ecc.u-tokyo.ac.jp/~g810370/linux-simd/ (Japanese).
;
; By KIMURA Takuhiro <kim@hannah.ipc.miyakyo-u.ac.jp> - until 31.Mar.1999
;                    <kim@comtec.co.jp>               - after  1.Apr.1999
;
;
; Replacement of Dct36() with AMD's 3DNow! SIMD operations support
;
; Syuuhei Kashiyama <squash@mb.kcom.ne.jp>
;
; The author of this program disclaim whole expressed or implied
; warranties with regard to this program, and in no event shall the
; author of this program liable to whatever resulted from the use of
; this program. Use it at your own risk.
;

; Exports
GLOBAL Dct36_3DNow

; Code
SEGMENT .text align=32 class=CODE use32

Dct36_3DNow:
	push	ebp
	mov		ebp, esp
	sub		esp, 120
	push	ebp						;Added by Thomas Neumann
	push	edi						;-""-
	push	esi
	push	ebx
	mov		eax, [ebp + 8]
	mov		esi, [ebp + 12]
	mov		ecx, [ebp + 16]
	mov		edx, [ebp + 20]
	mov		ebx, [ebp + 24]
	mov		edi, [ebp + 28]			;Added by Thomas Neumann (cos9)
;	lea		esp, [ebp - 128]		;Removed by Thomas Neumann
	mov		ebp, [ebp + 32]			;tfcos36

	femms
	movq	mm0, [eax]
	movq	mm1, [eax + 4]
	pfadd	mm0, mm1
	movq	[eax + 4], mm0
	psrlq	mm1, 32
	movq	mm2, [eax + 12]
	punpckldq mm1, mm2
	pfadd	mm1, mm2
	movq	[eax + 12], mm1
	psrlq	mm2, 32
	movq	mm3, [eax + 20]
	punpckldq mm2, mm3
	pfadd	mm2, mm3
	movq	[eax + 20], mm2
	psrlq	mm3, 32
	movq	mm4, [eax + 28]
	punpckldq mm3, mm4
	pfadd	mm3, mm4
	movq	[eax + 28], mm3
	psrlq	mm4, 32
	movq	mm5, [eax + 36]
	punpckldq mm4, mm5
	pfadd	mm4, mm5
	movq	[eax + 36], mm4
	psrlq	mm5, 32
	movq	mm6, [eax + 44]
	punpckldq mm5, mm6
	pfadd	mm5, mm6
	movq	[eax + 44], mm5
	psrlq	mm6, 32
	movq	mm7, [eax + 52]
	punpckldq mm6, mm7
	pfadd	mm6, mm7
	movq	[eax + 52], mm6
	psrlq	mm7, 32
	movq	mm0, [eax + 60]
	punpckldq mm7, mm0
	pfadd	mm7, mm0
	movq	[eax + 60], mm7
	psrlq	mm0, 32
	movd	mm1, [eax + 68]
	pfadd	mm0, mm1
	movd	[eax + 68], mm0
	movd	mm0, [eax + 4]
	movd	mm1, [eax + 12]
	punpckldq mm0, mm1
	punpckldq mm1, [eax + 20]
	pfadd	mm0, mm1
	movd	[eax + 12], mm0
	psrlq	mm0, 32
	movd	[eax + 20], mm0
	psrlq	mm1, 32
	movd	mm2, [eax + 28]
	punpckldq mm1, mm2
	punpckldq mm2, [eax + 36]
	pfadd	mm1, mm2
	movd	[eax + 28], mm1
	psrlq	mm1, 32
	movd	[eax + 36], mm1
	psrlq	mm2, 32
	movd	mm3, [eax + 44]
	punpckldq mm2, mm3
	punpckldq mm3, [eax + 52]
	pfadd	mm2, mm3
	movd	[eax + 44], mm2
	psrlq	mm2, 32
	movd	[eax + 52], mm2
	psrlq	mm3, 32
	movd	mm4, [eax + 60]
	punpckldq mm3, mm4
	punpckldq mm4, [eax + 68]
	pfadd	mm3, mm4
	movd	[eax + 60], mm3
	psrlq	mm3, 32
	movd	[eax + 68], mm3

	movq	mm0, [eax + 24]
	movq	mm1, [eax + 48]
	movd	mm2, [edi + 12]			;movd mm2, cos9 + 12
	punpckldq mm2, mm2
	movd	mm3, [edi + 24]			;movd mm3, cos9 + 24
	punpckldq mm3, mm3
	pfmul	mm0, mm2
	pfmul	mm1, mm3
	push	eax
	mov		eax, 1
	movd	mm7, eax
	pi2fd	mm7, mm7
	pop		eax
	movq	mm2, [eax + 8]
	movd	mm3, [edi + 4]			;movd mm3, cos9 + 4
	punpckldq mm3, mm3
	pfmul	mm2, mm3
	pfadd	mm2, mm0
	movq	mm3, [eax + 40]
	movd	mm4, [edi + 20]			;movd mm4, cos9 + 20
	punpckldq mm4, mm4
	pfmul	mm3, mm4
	pfadd	mm2, mm3
	movq	mm3, [eax + 56]
	movd	mm4, [edi + 28]			;movd mm4, cos9 + 28
	punpckldq mm4, mm4
	pfmul	mm3, mm4
	pfadd	mm2, mm3
	movq	mm3, [eax]
	movq	mm4, [eax + 16]
	movd	mm5, [edi + 8]			;movd mm5, cos9 + 8
	punpckldq mm5, mm5
	pfmul	mm4, mm5
	pfadd	mm3, mm4
	movq	mm4, [eax + 32]
	movd	mm5, [edi + 16]			;movd mm5, cos9 + 16
	punpckldq mm5, mm5
	pfmul	mm4, mm5
	pfadd	mm3, mm4
	pfadd	mm3, mm1
	movq	mm4, [eax + 64]
	movd	mm5, [edi + 32]			;movd mm5, cos9 + 32
	punpckldq mm5, mm5
	pfmul	mm4, mm5
	pfadd	mm3, mm4
	movq	mm4, mm2
	pfadd	mm4, mm3
	movq	mm5, mm7
	punpckldq mm5, [ebp + 0]		;punpckldq mm5, tfcos36 + 0
	pfmul	mm4, mm5
	movq	mm5, mm4
	pfacc	mm5, mm5
	movd	mm6, [edx + 108]
	punpckldq mm6, [edx + 104]
	pfmul	mm5, mm6
	movd	[ecx + 36], mm5
	psrlq	mm5, 32
	movd	[ecx + 32], mm5
	movq	mm6, mm4
	punpckldq mm5, mm6
	pfsub	mm5, mm6
	punpckhdq mm5, mm5
	movd	mm6, [edx + 32]
	punpckldq mm6, [edx + 36]
	pfmul	mm5, mm6
	movd	mm6, [esi + 32]
	punpckldq mm6, [esi + 36]
	pfadd	mm5, mm6
	movd	[ebx + 1024], mm5
	psrlq	mm5, 32
	movd	[ebx + 1152], mm5
	movq	mm4, mm3
	pfsub	mm4, mm2
	movq	mm5, mm7
	punpckldq mm5, [ebp + 32]		;punpckldq mm5, tfcos36 + 32
	pfmul	mm4, mm5
	movq	mm5, mm4
	pfacc	mm5, mm5
	movd	mm6, [edx + 140]
	punpckldq mm6, [edx + 72]
	pfmul	mm5, mm6
	movd	[ecx + 68], mm5
	psrlq	mm5, 32
	movd	[ecx + 0], mm5
	movq	mm6, mm4
	punpckldq mm5, mm6
	pfsub	mm5, mm6
	punpckhdq mm5, mm5
	movd	mm6, [edx + 0]
	punpckldq mm6, [edx + 68]
	pfmul	mm5, mm6
	movd	mm6, [esi + 0]
	punpckldq mm6, [esi + 68]
	pfadd	mm5, mm6
	movd	[ebx + 0], mm5
	psrlq	mm5, 32
	movd	[ebx + 2176], mm5
	movq	mm2, [eax + 8]
	movq	mm3, [eax + 40]
	pfsub	mm2, mm3
	movq	mm3, [eax + 56]
	pfsub	mm2, mm3
	movd	mm3, [edi + 12]			;movd mm3, cos9 + 12
	punpckldq mm3, mm3
	pfmul	mm2, mm3
	movq	mm3, [eax + 16]
	movq	mm4, [eax + 32]
	pfsub	mm3, mm4
	movq	mm4, [eax + 64]
	pfsub	mm3, mm4
	movd	mm4, [edi + 24]			;movd mm4, cos9 + 24
	punpckldq mm4, mm4
	pfmul	mm3, mm4
	movq	mm4, [eax + 48]
	pfsub	mm3, mm4
	movq	mm4, [eax]
	pfadd	mm3, mm4
	movq	mm4, mm2
	pfadd	mm4, mm3
	movq	mm5, mm7
	punpckldq mm5, [ebp + 4]		;punpckldq mm5, tfcos36 + 4
	pfmul	mm4, mm5
	movq	mm5, mm4
	pfacc	mm5, mm5
	movd	mm6, [edx + 112]
	punpckldq mm6, [edx + 100]
	pfmul	mm5, mm6
	movd	[ecx + 40], mm5
	psrlq	mm5, 32
	movd	[ecx + 28], mm5
	movq	mm6, mm4
	punpckldq mm5, mm6
	pfsub	mm5, mm6
	punpckhdq mm5, mm5
	movd	mm6, [edx + 28]
	punpckldq mm6, [edx + 40]
	pfmul	mm5, mm6
	movd	mm6, [esi + 28]
	punpckldq mm6, [esi + 40]
	pfadd	mm5, mm6
	movd	[ebx + 896], mm5
	psrlq	mm5, 32
	movd	[ebx + 1280], mm5
	movq	mm4, mm3
	pfsub	mm4, mm2
	movq	mm5, mm7
	punpckldq mm5, [ebp + 28]		;punpckldq mm5, tfcos36 + 28
	pfmul	mm4, mm5
	movq	mm5, mm4
	pfacc	mm5, mm5
	movd	mm6, [edx + 136]
	punpckldq mm6, [edx + 76]
	pfmul	mm5, mm6
	movd	[ecx + 64], mm5
	psrlq	mm5, 32
	movd	[ecx + 4], mm5
	movq	mm6, mm4
	punpckldq mm5, mm6
	pfsub	mm5, mm6
	punpckhdq mm5, mm5
	movd	mm6, [edx + 4]
	punpckldq mm6, [edx + 64]
	pfmul	mm5, mm6
	movd	mm6, [esi + 4]
	punpckldq mm6, [esi + 64]
	pfadd	mm5, mm6
	movd	[ebx + 128], mm5
	psrlq	mm5, 32
	movd	[ebx + 2048], mm5

	movq	mm2, [eax + 8]
	movd	mm3, [edi + 20]			;movd mm3, cos9 + 20
	punpckldq mm3, mm3
	pfmul	mm2, mm3
	pfsub	mm2, mm0
	movq	mm3, [eax + 40]
	movd	mm4, [edi + 28]			;movd mm4, cos9 + 28
	punpckldq mm4, mm4
	pfmul	mm3, mm4
	pfsub	mm2, mm3
	movq	mm3, [eax + 56]
	movd	mm4, [edi + 4]			;movd mm4, cos9 + 4
	punpckldq mm4, mm4
	pfmul	mm3, mm4
	pfadd	mm2, mm3
	movq	mm3, [eax]
	movq	mm4, [eax + 16]
	movd	mm5, [edi + 32]			;movd mm5, cos9 + 32
	punpckldq mm5, mm5
	pfmul	mm4, mm5
	pfsub	mm3, mm4
	movq	mm4, [eax + 32]
	movd	mm5, [edi + 8]			;movd mm5, cos9 + 8
	punpckldq mm5, mm5
	pfmul	mm4, mm5
	pfsub	mm3, mm4
	pfadd	mm3, mm1
	movq	mm4, [eax + 64]
	movd	mm5, [edi + 16]			;movd mm5, cos9 + 16
	punpckldq mm5, mm5
	pfmul	mm4, mm5
	pfadd	mm3, mm4
	movq	mm4, mm2
	pfadd	mm4, mm3
	movq	mm5, mm7
	punpckldq mm5, [ebp + 8]		;punpckldq mm5, tfcos36 + 8
	pfmul	mm4, mm5
	movq	mm5, mm4
	pfacc	mm5, mm5
	movd	mm6, [edx + 116]
	punpckldq mm6, [edx + 96]
	pfmul	mm5, mm6
	movd	[ecx + 44], mm5
	psrlq	mm5, 32
	movd	[ecx + 24], mm5
	movq	mm6, mm4
	punpckldq mm5, mm6
	pfsub	mm5, mm6
	punpckhdq mm5,mm5
	movd	mm6, [edx + 24]
	punpckldq mm6, [edx + 44]
	pfmul	mm5, mm6
	movd	mm6, [esi + 24]
	punpckldq mm6, [esi + 44]
	pfadd	mm5, mm6
	movd	[ebx + 768], mm5
	psrlq	mm5, 32
	movd	[ebx + 1408], mm5
	movq	mm4, mm3
	pfsub	mm4, mm2
	movq	mm5, mm7
	punpckldq mm5, [ebp + 24]		;punpckldq mm5, tfcos36 + 24
	pfmul	mm4, mm5
	movq	mm5, mm4
	pfacc	mm5, mm5
	movd	mm6, [edx + 132]
	punpckldq mm6, [edx + 80]
	pfmul	mm5, mm6
	movd	[ecx + 60], mm5
	psrlq	mm5, 32
	movd	[ecx + 8], mm5
	movq	mm6, mm4
	punpckldq mm5, mm6
	pfsub	mm5, mm6
	punpckhdq mm5, mm5
	movd	mm6, [edx + 8]
	punpckldq mm6, [edx + 60]
	pfmul	mm5, mm6
	movd	mm6, [esi + 8]
	punpckldq mm6, [esi + 60]
	pfadd	mm5, mm6
	movd	[ebx + 256], mm5
	psrlq	mm5, 32
	movd	[ebx + 1920], mm5
	movq	mm2, [eax + 8]
	movd	mm3, [edi + 28]			;movd mm3, cos9 + 28
	punpckldq mm3, mm3
	pfmul	mm2, mm3
	pfsub	mm2, mm0
	movq	mm3, [eax + 40]
	movd	mm4, [edi + 4]			;movd mm4, cos9 + 4
	punpckldq mm4, mm4
	pfmul	mm3, mm4
	pfadd	mm2, mm3
	movq	mm3, [eax + 56]
	movd	mm4, [edi + 20]			;movd mm4, cos9 + 20
	punpckldq mm4, mm4
	pfmul	mm3, mm4
	pfsub	mm2, mm3
	movq	mm3, [eax]
	movq	mm4, [eax + 16]
	movd	mm5, [edi + 16]			;movd mm5, cos9 + 16
	punpckldq mm5, mm5
	pfmul	mm4, mm5
	pfsub	mm3, mm4
	movq	mm4, [eax + 32]
	movd	mm5, [edi + 32]			;movd mm5, cos9 + 32
	punpckldq mm5, mm5
	pfmul	mm4, mm5
	pfadd	mm3, mm4
	pfadd	mm3, mm1
	movq	mm4, [eax + 64]
	movd	mm5, [edi + 8]			;movd mm5, cos9 + 8
	punpckldq mm5, mm5
	pfmul	mm4, mm5
	pfsub	mm3, mm4
	movq	mm4, mm2
	pfadd	mm4, mm3
	movq	mm5, mm7
	punpckldq mm5, [ebp + 12]		;punpckldq mm5, tfcos36 + 12
	pfmul	mm4, mm5
	movq	mm5, mm4
	pfacc	mm5, mm5
	movd	mm6, [edx + 120]
	punpckldq mm6, [edx + 92]
	pfmul	mm5, mm6
	movd	[ecx + 48], mm5
	psrlq	mm5, 32
	movd	[ecx + 20], mm5
	movq	mm6, mm4
	punpckldq mm5, mm6
	pfsub	mm5, mm6
	punpckhdq mm5, mm5
	movd	mm6, [edx + 20]
	punpckldq mm6, [edx + 48]
	pfmul	mm5, mm6
	movd	mm6, [esi + 20]
	punpckldq mm6, [esi + 48]
	pfadd	mm5, mm6
	movd	[ebx + 640], mm5
	psrlq	mm5, 32
	movd	[ebx + 1536], mm5
	movq	mm4, mm3
	pfsub	mm4, mm2
	movq	mm5, mm7
	punpckldq mm5, [ebp + 20]		;punpckldq mm5, tfcos36 + 20
	pfmul	mm4, mm5
	movq	mm5, mm4
	pfacc	mm5, mm5
	movd	mm6, [edx + 128]
	punpckldq mm6, [edx + 84]
	pfmul	mm5, mm6
	movd	[ecx + 56], mm5
	psrlq	mm5, 32
	movd	[ecx + 12], mm5
	movq	mm6, mm4
	punpckldq mm5, mm6
	pfsub	mm5, mm6
	punpckhdq mm5, mm5
	movd	mm6, [edx + 12]
	punpckldq mm6, [edx + 56]
	pfmul	mm5, mm6
	movd	mm6, [esi + 12]
	punpckldq mm6, [esi + 56]
	pfadd	mm5, mm6
	movd	[ebx + 384], mm5
	psrlq	mm5, 32
	movd	[ebx + 1792], mm5

	movq	mm4, [eax]
	movq	mm3, [eax + 16]
	pfsub	mm4, mm3
	movq	mm3, [eax + 32]
	pfadd	mm4, mm3
	movq	mm3, [eax + 48]
	pfsub	mm4, mm3
	movq	mm3, [eax + 64]
	pfadd	mm4, mm3
	movq	mm5, mm7
	punpckldq mm5, [ebp + 16]		;punpckldq mm5, tfcos36 + 16
	pfmul	mm4, mm5
	movq	mm5, mm4
	pfacc	mm5, mm5
	movd	mm6, [edx + 124]
	punpckldq mm6, [edx + 88]
	pfmul	mm5, mm6
	movd	[ecx + 52], mm5
	psrlq	mm5, 32
	movd	[ecx + 16], mm5
	movq	mm6, mm4
	punpckldq mm5, mm6
	pfsub	mm5, mm6
	punpckhdq mm5, mm5
	movd	mm6, [edx + 16]
	punpckldq mm6, [edx + 52]
	pfmul	mm5, mm6
	movd	mm6, [esi + 16]
	punpckldq mm6, [esi + 52]
	pfadd	mm5, mm6
	movd	[ebx + 512], mm5
	psrlq	mm5, 32
	movd	[ebx + 1664], mm5

	femms
	pop		ebx
	pop		esi
	pop		edi						;Added by Thomas Neumann
	pop		ebp						;-""-
	mov		esp, ebp
	pop		ebp
	ret

end
