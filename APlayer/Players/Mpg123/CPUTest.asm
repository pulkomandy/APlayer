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

; Exports
GLOBAL HaveMMX
GLOBAL Have3DNow

; Data
SEGMENT .data align=16 class=DATA use32

; Code
SEGMENT .text align=16 class=CODE use32



;/******************************************************************************/
;/* HaveMMX() checks the CPU to see if MMX is available.                       */
;/*                                                                            */
;/* Output: 1 if MMX is available, 0 if not.                                   */
;/******************************************************************************/
HaveMMX:

	; Preserve registers
	push	ebp
	mov		ebp, esp
	push	ebx
	pushfd

	; Get processor signature
	mov		eax, 00000001h
	cpuid
	test	edx, 00800000h
	jz		.NOMMX

	; We got MMX
	mov		eax, 1
	jmp		.OUT

	; We haven't MMX :(
.NOMMX:
	mov		eax, 0

	; Get registers back
.OUT:
	popfd
	pop		ebx
	pop		ebp
	ret



;/******************************************************************************/
;/* Have3DNow() checks the CPU to see if 3DNow! is available.                  */
;/*                                                                            */
;/* Output: 1 if 3DNow! is available, 0 if not.                                */
;/******************************************************************************/
Have3DNow:

	; Preserve registers
	push	ebp
	mov		ebp, esp
	push	ebx
	pushfd

	; Get largest extended value
	mov		eax, 80000000h
	cpuid
	cmp		eax, 80000001h		; Can we execute feature 1?
	jbe		.NO3DNOW

	; Execute feature 1
	mov		eax, 80000001h
	cpuid
	test	edx, 80000000h
	jz		.NO3DNOW

	; We got 3DNow!
	mov		eax, 1
	jmp		.OUT

	; We haven't 3DNow! :(
.NO3DNOW:
	mov		eax, 0

	; Get registers back
.OUT:
	popfd
	pop		ebx
	pop		ebp
	ret

end
