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
GLOBAL MakeDecodeTables_MMX

; Data
SEGMENT .data align=32 class=DATA use32

intWinBase:
	dw		     0,     -1,     -1,     -1,     -1,     -1,     -1,     -2
	dw		    -2,     -2,     -2,     -3,     -3,     -4,     -4,     -5
	dw		    -5,     -6,     -7,     -7,     -8,     -9,    -10,    -11
	dw		   -13,    -14,    -16,    -17,    -19,    -21,    -24,    -26
	dw		   -29,    -31,    -35,    -38,    -41,    -45,    -49,    -53
	dw		   -58,    -63,    -68,    -73,    -79,    -85,    -91,    -97
	dw		  -104,   -111,   -117,   -125,   -132,   -139,   -147,   -154
	dw		  -161,   -169,   -176,   -183,   -190,   -196,   -202,   -208
	dw		  -213,   -218,   -222,   -225,   -227,   -228,   -228,   -227
	dw		  -224,   -221,   -215,   -208,   -200,   -189,   -177,   -163
	dw		  -146,   -127,   -106,    -83,    -57,    -29,      2,     36
	dw		    72,    111,    153,    197,    244,    294,    347,    401
	dw		   459,    519,    581,    645,    711,    779,    848,    919
	dw		   991,   1064,   1137,   1210,   1283,   1356,   1428,   1498
	dw		  1567,   1634,   1698,   1759,   1817,   1870,   1919,   1962
	dw		  2001,   2032,   2057,   2075,   2085,   2087,   2080,   2063
	dw		  2037,   2000,   1952,   1893,   1822,   1739,   1644,   1535
	dw		  1414,   1280,   1131,    970,    794,    605,    402,    185
	dw		   -45,   -288,   -545,   -814,  -1095,  -1388,  -1692,  -2006
	dw		 -2330,  -2663,  -3004,  -3351,  -3705,  -4063,  -4425,  -4788
	dw		 -5153,  -5517,  -5879,  -6237,  -6589,  -6935,  -7271,  -7597
	dw		 -7910,  -8209,  -8491,  -8755,  -8998,  -9219,  -9416,  -9585
	dw		 -9727,  -9838,  -9916,  -9959,  -9966,  -9935,  -9863,  -9750
	dw		 -9592,  -9389,  -9139,  -8840,  -8492,  -8092,  -7640,  -7134
	dw		 -6574,  -5959,  -5288,  -4561,  -3776,  -2935,  -2037,  -1082
	dw		   -70,    998,   2122,   3300,   4533,   5818,   7154,   8540
	dw		  9975,  11455,  12980,  14548,  16155,  17799,  19478,  21189
	dw		 22929,  24694,  26482,  28289,  30112,  31947, -26209, -24360
	dw		-22511, -20664, -18824, -16994, -15179, -13383, -11610,  -9863
	dw		 -8147,  -6466,  -4822,  -3222,  -1667,   -162,   1289,   2684
	dw		  4019,   5290,   6494,   7629,   8692,   9679,  10590,  11420
	dw		 12169,  12835,  13415,  13908,  14313,  14630,  14856,  14992
	dw		 15038

intWinDiv:
	dd		0x47800000				; 65536.0

; Code
SEGMENT .text align=32 class=CODE use32


MakeDecodeTables_MMX:
	push	edi
	push	esi
	push	ebx
	push	ebp						;Added by Thomas Neumann

	xor		ecx, ecx
	xor		ebx, ebx
	mov		esi, 32
	mov		edi, intWinBase
	neg		dword [esp + 20]		;neg dword [esp + 16] ;scaleVal
	mov		ebp, [esp + 28]			;Added by Thomas Neumann (decWins)
	push	dword 2					;intWinBase step
.L00:
	cmp		ecx, 528
	jnc		.L02
	movsx	eax, word [edi]
	cmp		edi, intWinBase + 444
	jc		.L01
	add		eax, 60000
.L01:
	push	eax
	fild	dword [esp]
	fdiv	dword [intWinDiv]
	fimul	dword [esp + 28]		;fimul dword [esp + 24]
	pop		eax
	mov		edx, [esp + 28]			;Added by Thomas Neumann (decWin)
	fst		dword [ecx * 4 + edx]	;fst dword [ecx * 4 + decWin]
	fstp	dword [ecx * 4 + edx + 64] ;fstp dword [ecx * 4 + decWin + 64]
.L02:
	lea		edx, [esi - 1]
	and		edx, ebx
	cmp		edx, 31
	jnz		.L03
	add		ecx, -1023
	test	ebx, esi
	jz		.L03
	neg		dword [esp + 24]		;neg dword [esp + 20]
.L03:
	add		ecx, esi
	add		edi, [esp]
	inc		ebx
	cmp		edi, intWinBase
	jz		.L04
	cmp		ebx, 256
	jnz		.L00
	neg		dword [esp]
	jmp		.L00
.L04:
	pop		eax

	xor		ecx, ecx
	xor		ebx, ebx
	push	dword 2
.L05:
	cmp		ecx, 528
	jnc		.L11
	movsx	eax, word [edi]
	cmp		edi, intWinBase + 444
	jc		.L06
	add		eax, 60000
.L06:
	cdq
	imul	dword [esp + 24]		;imul dword [esp + 20]
	shrd	eax, edx, 17
	cmp		eax, 32767
	mov		edx, 1055
	jle		.L07
	mov		eax, 32767
	jmp		.L08
.L07:
	cmp		eax, -32767
	jge		.L08
	mov		eax, -32767
.L08:
	cmp		ecx, 512
	jnc		.L09
	sub		edx, ecx
	mov		word [edx * 2 + ebp], ax ;mov word [edx * 2 + decWins], ax
	mov		word [edx * 2 + ebp - 32], ax ;mov word [edx * 2 + decWins - 32], ax
.L09:
	test	ecx, 1
	jnz		.L10
	neg		eax
.L10:
	mov		word [ecx * 2 + ebp], ax ;mov word [ecx * 2 + decWins], ax
	mov		word [ecx * 2 + ebp + 32], ax ;mov word [ecx * 2 + decWins + 32], ax
.L11:
	lea		edx, [esi - 1]
	and		edx, ebx
	cmp		edx, 31
	jnz		.L12
	add		ecx, -1023
	test	ebx, esi
	jz		.L12
	neg		dword [esp + 24]		;neg dword [esp + 20]
.L12:
	add		ecx, esi
	add		edi, [esp]
	inc		ebx
	cmp		edi, intWinBase
	jz		.L13
	cmp		ebx, 256
	jnz		near .L05
	neg		dword [esp]
	jmp		.L05
.L13:
	pop		eax
	pop		ebp						;Added by Thomas Neumann
	pop		ebx
	pop		esi
	pop		edi
	ret

end
