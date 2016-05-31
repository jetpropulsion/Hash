							.CREF
							.TFCOND
							.LFCOND
							.SFCOND
							.LIST
							.LISTALL
							OPTION		NODOTNAME
							;OPTION		EXPR32
							OPTION		NOM510
							OPTION		NOOLDMACROS
							OPTION		NOOLDSTRUCTS
							OPTION		CASEMAP: NONE
							;OPTION		LANGUAGE: STDCALL
							OPTION		LANGUAGE: C
							OPTION		NOLJMP
							;OPTION		PROLOGUE: NONE
							;OPTION		EPILOGUE: NONE
							.RADIX		10
							;.SEQ

							.686P
;==================================================================================================
HASHCODE			SEGMENT		PARA PUBLIC FLAT READ EXECUTE NOPAGE
							.MODEL		FLAT, C
							.CODE			HASHCODE
							.RADIX		10
;--------------------------------------------------------------------------------------------------
;-	Procedure Jenkins_x86 (Bob Jenkins' Hash implemented in x86 assembly)
;--------------------------------------------------------------------------------------------------
							ALIGN			8
Jenkins_x86		PROC			pcbValue:DWORD, dwLength:DWORD, dwSeed:DWORD
							PUBLIC		Jenkins_x86
							ASSUME		CS:HASHCODE, DS:ERROR, ES:ERROR, FS:ERROR, GS:ERROR, SS:ERROR
							.686P

							push ebx
							push esi
							push edi

							mov eax, [dwSeed]
							mov ecx, [dwLength]
							add eax, 0DEADBEEFh
							add eax, ecx
							mov esi, [pcbValue]
							mov edi, eax
							mov ebx, eax
							mov edx, eax
							jmp L_check
							ALIGN 8
L_loop:
							;//////////////////
							;// xx xx AH AL
							;// 03 02 01 00 == 00010203 (00 00 04 00 == 00400000)
							;// 0  1  2  3
							;///////////////////
							mov eax, [esi]				;	eax = (DWORD)Value[0]
							and eax, 0000ff00h		;	eax = ((BYTE)Value[1]) << 8
							add edi, eax					;	a += ((BYTE)Value[1]) << 8

							mov eax, [esi]				;	eax = (DWORD)Value[0]
							and eax, 00ff0000h		;	eax = ((BYTE)Value[2]) << 16
							add edi, eax					;	a += ((BYTE)Value[2]) << 16

							mov eax, [esi]				;	eax = (DWORD)Value[0]
							and eax, 0ff000000h		;	eax = ((BYTE)Value[3]) << 24
							add edi, eax					;	a += ((BYTE)Value[3]) << 24

							movzx eax, byte ptr [esi]
							add edi, eax					;	a += (BYTE)Value[0]
							;///////////////////
							mov eax, [esi + 4]		;	eax = (DWORD)Value[4]
							and eax, 0000ff00h		;	eax = ((BYTE)Value[5]) << 8
							add ebx, eax					;	b += ((BYTE)Value[5]) << 8

							mov eax, [esi + 4]		;	eax = (DWORD)Value[4]
							and eax, 00ff0000h		;	eax = ((BYTE)Value[6]) << 16
							add ebx, eax					;	b += ((BYTE)Value[6]) << 16

							mov eax, [esi + 4]		;	eax = (DWORD)Value[4]
							and eax, 0ff000000h		;	eax = ((BYTE)Value[7]) << 24
							add ebx, eax					;	b += ((BYTE)Value[7]) << 24

							movzx eax, byte ptr [esi + 4]		;eax = (DWORD)Value[4]
							add ebx, eax					;	b += (BYTE)Value[4]
							;///////////////////
							mov eax, [esi + 8]		;	eax = (DWORD)Value[8]
							and eax, 0000ff00h		;	eax = ((BYTE)Value[9]) << 8
							add edx, eax					;	c += ((BYTE)Value[9]) << 8

							mov eax, [esi + 8]		;	eax = (DWORD)Value[8]
							and eax, 00ff0000h		;	eax = ((BYTE)Value[10]) << 16
							add edx, eax					;	c += ((BYTE)Value[10]) << 16

							mov eax, [esi + 8]		;	eax = (DWORD)Value[8]
							and eax, 0ff000000h		;	eax = ((BYTE)Value[11]) << 24
							add edx, eax					;	c += ((BYTE)Value[11]) << 24

							movzx eax, byte ptr [esi + 8]		;eax = (DWORD)Value[8]
							add edx, eax					;	c += (BYTE)Value[8]
							;///////////////////

							;//a -= c; a ^= _lrotl(c, 4); c += b;
							mov eax, edx		;//
							sub edi, edx		;//	a -= c
							rol eax, 4			;//
							add edx, ebx		;//	c += b
							xor edi, eax		;//	a ^= rol(c, 4)

							;//b -= a; b ^= _lrotl(a, 6); a += c;
							mov eax, edi		;//
							sub ebx, edi		;//	b -= a
							rol eax, 6			;//
							add edi, edx		;//	a += c
							xor ebx, eax		;//	b ^= rol(a, 6)

							;//c -= b; c ^= _lrotl(b, 8); b += a;
							mov eax, ebx		;//
							sub edx, ebx		;//	c -= b
							rol eax, 8			;//
							add ebx, edi		;//	b += a
							xor edx, eax		;//	c ^= rol(b, 8)

							;//a -= c; a ^= _lrotl(c, 16); c += b;
							mov eax, edx		;//
							sub edi, edx		;//	a -= c
							rol eax, 16			;//
							add edx, ebx		;//	c += b
							xor edi, eax		;//	a ^= rol(c, 16)

							;//b -= a; b ^= _lrotl(a, 19); a += c;
							mov eax, edi		;//
							sub ebx, edi		;//	b -= a
							rol eax, 19			;//
							add edi, edx		;//	a += c
							xor ebx, eax		;//	b ^= rol(a, 19)

							;//c -= b; c ^= _lrotl(b, 4); b += a;
							mov eax, ebx		;//
							sub edx, ebx		;//	c -= b
							rol eax, 4			;//
							add ebx, edi		;//	b += a
							xor edx, eax		;//	c ^= rol(b, 4)

							sub ecx, 12
							add esi, 12
L_check:
							cmp ecx, 12
							ja L_loop
;///////////////////////////////////////////////////////////////
L_lesser:
							lea ecx, [L_switch + ecx * 4]
							jmp dword ptr [ecx]
;///////////////////////////////////////////////////////////////
							ALIGN 8
L_switch:
							dd offset L_00
							dd offset L_01
							dd offset L_02
							dd offset L_03
							dd offset L_04
							dd offset L_05
							dd offset L_06
							dd offset L_07
							dd offset L_08
							dd offset L_09
							dd offset L_10
							dd offset L_11
							dd offset L_12
;///////////////////////////////////////////////////////////////
							ALIGN 8
L_12:
							movzx eax, byte ptr [esi + 11]
							shl eax, 24
							add edx, eax		;//	c += Value[11] << 24
L_11:
							movzx eax, byte ptr [esi + 10]
							shl eax, 16
							add edx, eax		;//	c+= Value[10] << 16
L_10:
							movzx eax, byte ptr [esi + 9]
							shl eax, 8
							add edx, eax		;//	c += Value[9] << 8
L_09:
							movzx eax, byte ptr [esi + 8]
							add edx, eax		;//	c += Value[8]
;///////////////////////////////////////////////////////////////
L_08:
							movzx eax, byte ptr [esi + 7]
							shl eax, 24
							add ebx, eax		;//	b += Value[7] << 24
L_07:
							movzx eax, byte ptr [esi + 6]
							shl eax, 16
							add ebx, eax		;//	b += Value[6] << 16
L_06:
							movzx eax, byte ptr [esi + 5]
							shl eax, 8
							add ebx, eax		;//	b += Value[5] << 8
L_05:
							movzx eax, byte ptr [esi + 4]
							add ebx, eax		;//	b += Value[4]
;///////////////////////////////////////////////////////////////
L_04:
							movzx eax, byte ptr [esi + 3]
							shl eax, 24
							add edi, eax		;	a += Value[3] << 24
L_03:
							movzx eax, byte ptr [esi + 2]
							shl eax, 16
							add edi, eax		;	a += Value[2] << 16
L_02:
							movzx eax, byte ptr [esi + 1]
							shl eax, 8
							add edi, eax		;	a += Value[1] << 8
L_01:
							movzx eax, byte ptr [esi]
							add edi, eax		;	a += Value[0]
;///////////////////////////////////////////////////////////////
							;---------------------------
							;	final(a,b,c);
							;---------------------------

							;	c ^= b; c -= _lrotl(b, 14);
							mov eax, ebx
							xor edx, ebx		;	c ^= b
							rol eax, 14
							sub edx, eax		;	c -= rol(b, 14)

							;	a ^= c; a -= _lrotl(c, 11);
							mov eax, edx
							xor edi, edx
							rol eax, 11
							sub edi, eax

							;	b ^= a; b -= _lrotl(a, 25);
							mov eax, edi
							xor ebx, edi
							rol eax, 25
							sub ebx, eax

							;	c ^= b; c -= _lrotl(b, 16);
							mov eax, ebx
							xor edx, ebx
							rol eax, 16
							sub edx, eax

							;	a ^= c; a -= _lrotl(c, 4);
							mov eax, edx
							xor edi, edx
							rol eax, 4
							sub edi, eax

							;	b ^= a; b -= _lrotl(a, 14);
							mov eax, edi
							xor ebx, edi
							rol eax, 14
							sub ebx, eax

							;	c ^= b; c -= _lrotl(b, 24);
							xor edx, ebx
							rol ebx, 24
							sub edx, ebx
L_00:
							mov eax, edx

							pop edi
							pop esi
							pop ebx
							ret																					;return
							ALIGN 8
Jenkins_x86		ENDP

;--------------------------------------------------------------------------------------------------
;-	Procedure FNV1a_x86 (Fowler/Noll/Vo Hash implemented in x86 assembly)
;--------------------------------------------------------------------------------------------------
							ALIGN			8
FNV1a_x86			PROC			pcbValue:DWORD, dwLength:DWORD, dwSeed:DWORD
							PUBLIC		FNV1a_x86
							ASSUME		CS:HASHCODE, DS:ERROR, ES:ERROR, FS:ERROR, GS:ERROR, SS:ERROR
							.686P

							push esi

							mov esi, [pcbValue]
							mov eax, [dwLength]
							lea edx, [esi + eax]
							mov eax, 0811c9dc5h
							cmp esi, edx
							jae L_end
L_calc:
							imul eax, 001000193h
							movzx ecx, byte ptr [esi]
							inc esi
							xor eax, ecx
							cmp esi, edx
							jb L_calc
L_end:
							mov esi, [dwLength]
							add esi, [dwSeed]
							xor eax, esi
							pop esi
							ret
							ALIGN 8
FNV1a_x86			ENDP

;--------------------------------------------------------------------------------------------------
;-	Procedure DJB2_x86 (Daniel J. Bernstein Hash #2 implemented in x86 assembly)
;--------------------------------------------------------------------------------------------------
							ALIGN			8
DJB2_x86			PROC			pcbValue:DWORD, dwLength:DWORD, dwSeed:DWORD
							PUBLIC		DJB2_x86
							ASSUME		CS:HASHCODE, DS:ERROR, ES:ERROR, FS:ERROR, GS:ERROR, SS:ERROR
							.686P
							.XMM
							mov eax, [dwSeed]
							mov ecx, [dwLength]
							test ecx, ecx
							lea eax, [ecx + eax + 5381]
							jz L_end
							mov edx, [pcbValue]
							push esi
L_calc:
							mov esi, eax
							shl esi, 5
							add esi, eax
							movzx eax, byte ptr [edx]
							inc edx
							add eax, esi
							dec ecx
							jnz L_calc
							pop esi
L_end:
							ret
							ALIGN 8
DJB2_x86			ENDP

;--------------------------------------------------------------------------------------------------
;-	Procedure Murmur_x86 (by Austin Appleby in x86 assembly)
;--------------------------------------------------------------------------------------------------
							ALIGN			8
Murmur_x86		PROC			pcbValue:DWORD, dwLength:DWORD, dwSeed:DWORD
							PUBLIC		Murmur_x86
							ASSUME		CS:HASHCODE, DS:ERROR, ES:ERROR, FS:ERROR, GS:ERROR, SS:ERROR, EBX:ERROR
							.686P
							push esi
							push edi
							mov ecx, [dwLength]
							mov eax, [dwSeed]
							mov esi, [pcbValue]
							xor eax, ecx
							jmp L_loop
							ALIGN 8
L_calc:
							mov edx, [esi]
							bswap edx
							imul edx, 5bd1e995h
							add esi, 4

							mov edi, edx
							shr edi, 24
							xor edx, edi

							imul eax, 5bd1e995h
							imul edx, 5bd1e995h
							xor eax, edx							
L_loop:
							sub ecx, 4
							jns L_calc
							add ecx, 4
							lea ecx, [L_table + ecx * 4]
							jmp dword ptr [ecx]
							ALIGN 8
L_table:
							dd offset L_00
							dd offset L_01
							dd offset L_02
							dd offset L_03
							ALIGN 8
L_03:
							movzx edx, byte ptr [esi + 2]
							shl edx, 16
							xor eax, edx
L_02:
							movzx edx, byte ptr [esi + 1]
							shl edx, 8
							xor eax, edx
L_01:
							movzx edx, byte ptr [esi]
							xor eax, edx

							imul eax, 5bd1e995h
L_00:
							mov edx, eax
							shr eax, 13
							xor edx, eax

							imul edx, 5bd1e995h

							mov eax, edx
							shr edx, 15
							xor eax, edx

							pop edi
							pop esi
							ret
							ALIGN 8
Murmur_x86		ENDP

;--------------------------------------------------------------------------------------------------
;-	Procedure CRC32 (Cyclic-Redundancy-Check 32 bit implemented in x86 assembly)
;--------------------------------------------------------------------------------------------------
							ALIGN			8
							EXTERN		CRC32Table : DWORD
CRC32_x86			PROC			pcbValue:DWORD, dwLength:DWORD, dwSeed:DWORD
							PUBLIC		CRC32_x86
							ASSUME		CS:HASHCODE, DS:ERROR, ES:ERROR, FS:ERROR, GS:ERROR, SS:ERROR
							.686P
							mov ecx, [dwLength]
							test ecx, ecx
							mov eax, [dwSeed]
							jz L_end
							push esi
							mov esi, [pcbValue]
L_loop:
							movzx edx, byte ptr [esi]
							xor dl, al
							shr eax, 8
							inc esi
							xor eax, [CRC32Table + edx * 4]
							dec ecx
							jnz L_loop
							pop esi
L_end:
							ret
							ALIGN 8
CRC32_x86			ENDP
;--------------------------------------------------------------------------------------------------
HASHCODE			ENDS
							END
;==================================================================================================
