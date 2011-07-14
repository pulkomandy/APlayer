; GCC Calling Conventions
;
; Functions must preserve:
;    ebp
;    esi
;    edi
;    ebx
;    st(*) (entire floating-point stack, if used)
;
; Function Return Values:
;    Reference (pointer) : eax
;    32-bit value : eax
;    64-bit value : edx:eax
;    Float : st(0)

; Imports
EXTERN Dct64_MMX

; Exports
GLOBAL Synth_1to1_MMX

; Code
SEGMENT .text align=32 class=CODE use32


Synth_1to1_MMX:
	push	ebp
	push	edi
	push	esi
	push	ebx
	mov		ecx, [esp + 24]
	mov		edi, [esp + 28]
	mov		ebx, 15
	mov		edx, [esp + 36]
;	lea		edi, [edi + ecx * 2]		;Removed by Thomas Neumann (samples++)
	dec		ecx
	mov		esi, [esp + 32]
	mov		eax, [edx]
	jecxz	.L1
	dec		eax
	and		eax, ebx
	lea		esi, [esi + 1088]
	mov		[edx], eax
.L1
	lea		edx, [esi + eax * 2]
	mov		ebp, eax
	inc		eax
	push	dword [esp + 20]
	and		eax, ebx
	lea		ecx, [esi + eax * 2 + 544]
	inc		ebx
	test	eax, 1
	jnz		.L2
	xchg	ecx, edx
	inc		ebp
	lea		esi, [esi + 544]
.L2
	push	edx
	push	ecx
	call	Dct64_MMX
	add		esp, 12
	lea		ecx, [ebx + 1]
	sub		ebx, ebp

	mov		edx, [esp + 40]				;Added by Thomas Neumann
	lea		edx, [ebx + ebx * 1 + edx]	;lea edx, [ebx + ebx * 1 + decWins]
.L3
	movq	mm0, [edx]
	pmaddwd	mm0, [esi]
	movq	mm1, [edx + 8]
	pmaddwd	mm1, [esi + 8]
	movq	mm2, [edx + 16]
	pmaddwd	mm2, [esi + 16]
	movq	mm3, [edx + 24]
	pmaddwd	mm3, [esi + 24]
	paddd	mm0, mm1
	paddd	mm0, mm2
	paddd	mm0, mm3
	movq	mm1, mm0
	psrlq	mm1, 32
	paddd	mm0, mm1
	psrad	mm0, 13
	packssdw mm0, mm0
	movd	eax, mm0
	mov		[edi], ax

	lea		esi, [esi + 32]
	lea		edx, [edx + 64]
	lea		edi, [edi + 2]				;Changed from + 4 to + 2 by Thomas Neumann
	loop	.L3

	sub		esi, 64
	mov		ecx, 15
.L4
	movq	mm0, [edx]
	pmaddwd	mm0, [esi]
	movq	mm1, [edx + 8]
	pmaddwd	mm1, [esi + 8]
	movq	mm2, [edx + 16]
	pmaddwd	mm2, [esi + 16]
	movq	mm3, [edx + 24]
	pmaddwd	mm3, [esi + 24]
	paddd	mm0, mm1
	paddd	mm0, mm2
	paddd	mm0, mm3
	movq	mm1, mm0
	psrlq	mm1, 32
	paddd	mm1, mm0
	psrad	mm1, 13
	packssdw mm1, mm1
	psubd	mm0, mm0
	psubsw	mm0, mm1
	movd	eax, mm0
	mov		[edi], ax

	sub		esi, 32
	add		edx, 64
	lea		edi, [edi + 2]				;Changed from + 4 to + 2 by Thomas Neumann
	loop	.L4

	emms
	pop		ebx
	pop		esi
	pop		edi
	pop		ebp
	ret

end
