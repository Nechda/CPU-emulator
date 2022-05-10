.text
mov edi, 0
movw edi,  [0x9FE]  ;СЃС‡РёС‚С‹РІР°РµРј РєРѕР»РёС‡РµСЃС‚РІРѕ РєРѕР»РѕРЅРѕРє <-> x
mov [width], edi

mov edi, 0
movw edi, [0x9FC] ;СЃС‡РёС‚РІР°РµРј РєРѕР»РёС‡РµСЃС‚РІРѕ Р»РёРЅРёР№ <-> y
mov [height], edi

jmp main


;СЂРёСЃСѓРµС‚ СЃРёРјРІРѕР» РёР· [symbol] РїРѕ РєРѕРѕСЂРґРёРЅР°С‚Р°Рј (eax,ebx)
draw_symbol:
	mov edi, 0
	movw edi, [width]
	mov edx, 0xA00
	mul ebx, ebx, edi
	;mul ebx, ebx, 2
	add eax, eax, ebx
	add edx, edx, eax
	movb [edx], [symbol]
	ret

;РїРѕР»СѓС‡Р°РµРј СЂР°СЃСЃС‚РѕСЏРЅРёРµ РґРѕ РїРѕР»Р° eax - phi, ebx - theta
;РІРѕР·РІСЂР°С‰Р°РµРј РѕС‚РІРµС‚ РІ ecx
get_floor_dist:
	push eax
	push ebx

	fcos ebx
	fdiv ecx, -1.0, ebx

	pop ebx
	pop eax

	ret

;РїРѕР»СѓС‡Р°РµРј СЂР°СЃСЃС‚РѕСЏРЅРёРµ РґРѕ РїРѕР»Р° eax, ebx, ecx --- РІРµРєС‚РѕСЂ РЅР°РїСЂР°РІР»РµРЅРёСЏ
;РІРѕР·РІСЂР°С‰Р°РµРјРѕРµ Р·РЅР°С‡РµРЅРёРµ РІ ecx
get_floor_dist_uniform:
	push eax
	push ebx

	fja ecx, 0.0, so_far
	
	mov ebp, origin
	fdiv eax, [ebp+12], ecx
	fmul ecx, -1.0, eax

	jmp end_dist_cmp
	so_far:
	mov ecx, 10000.0
	end_dist_cmp:

	pop ebx
	pop eax
	ret

;РїРѕР»СѓС‡Р°РµРј СЏСЂРєРѕСЃС‚РЅС‹Р№ СЃРёРјРІРѕР» РёСЃС…РѕРґСЏ РёР· СЂР°СЃСЃС‚РѕСЏРЅРёСЏ
;in ecx --- СЂР°СЃСЃС‚РѕСЏРЅРёРµ Р»СѓС‡Р°
;out ecx --- РєРѕРґ СЃРёРјРІРѕР»Р°
get_light_symbol:

	push edx

	mov ecx, [distance]

	fmul ecx, ecx, 5.0
	fadd ecx, ecx, 1.0
	fsqrt ecx
	fadd ecx, ecx, 1.0
	fdiv ecx, 52.0, ecx

	fistp ecx

	

	add ecx, ecx, lighting
	mov edx, 0
	movb edx, [ecx]
	mov ecx, edx  ;СЃРёРјРІРѕР» РїРѕ СЏСЂРєРѕСЃС‚Рё

	pop edx

	ret


;РїРѕР»СѓС‡Р°РµРј СЃРёРјРІРѕР» РїСЂРё СЃС‚РѕР»РєРЅРѕРІРµРЅРёРё СЃ РїРѕР»РѕРј eax,ebx,ecx --- РµРґРёРЅРёС‡РЅС‹Р№ РЅР°РїСЂР°РІР»СЏСЋС‰РёР№ РІРµРєС‚РѕСЂ
;СЃРёРјРІРѕР» РІ ecx
get_sym_at_floor:
	
	call get_floor_dist_uniform
	fadd [distance], ecx, [distance]



	fmul eax, ecx, eax
	fmul ebx, ecx, ebx

	

	fistp eax
	fistp ebx

	
	add eax, eax, ebx
	mod eax, 2
		
	je eax, 1, if_0

	call get_light_symbol
	
	jmp end_if_0
	if_0:
    mov ecx, 0x20
	end_if_0:


	ret


get_angles:

	mov edx, 0
	movw edx, [width]
	fild edx
	fild eax
	fdiv eax, eax, edx
	fsub eax, eax, 0.5
	fmul eax, eax, -2.0


	mov edx, 0
	movw edx, [height]
	fild edx
	fild ebx
	fdiv ebx, ebx, edx
	fsub ebx, ebx, 0.5
	fmul ebx, ebx, 2.0

	mov ecx, 1.414

	mov ebp, esp
	mov [ebp+0], ecx
	fmul [ebp+4], eax, -1.0
	mov [ebp+8], ebx

	mov eax, [ebp+0]
	mov ebx, [ebp+4]
	mov ecx, [ebp+8]

	;mov edi, 0.5
	call mul_vector_to_matrix

	mov [ebp+0],eax
	mov [ebp+4],ebx
	mov [ebp+8],ecx

	fpow eax, 2.0
	fpow ebx, 2.0
	fadd eax, ebx, eax
	fpow eax, 0.5
	atan2 eax, ecx
	mov [ebp+12],eax


	mov eax, [ebp+4]
	mov ebx, [ebp+0]
	atan2 eax, ebx

	mov ebx, [ebp+12]

	

	ret



;esi - phi edi -theta 
crate_matrix:
	push eax
	push ebx
	push ecx
	push edx

	mov ecx, edi ; t
	mov edx, edi ; t

	mov eax, 1.0
	mov ebx, 0
	fcos ecx; cos t
	fsin edx; sin t

	;РїРµСЂРІС‹Р№ СЃС‚РѕР»Р±РёРє РІ РјР°С‚СЂРёС†Рµ
	mov ebp, matrix
	fmul [ebp+0], ecx, eax
	fmul [ebp+12], ecx, ebx
	fmul [ebp+24], -1.0, edx

	;РІС‚РѕСЂРѕР№ СЃС‚РѕР»Р±РёРє РІ РјР°С‚СЂРёС†Рµ
	add ebp, ebp, 4
	fmul [ebp+0], ebx, -1.0
	mov [ebp+12], eax
	mov [ebp+24], 0.0

	;С‚СЂРµС‚РёР№ СЃС‚РѕР»Р±РёРє РІ РјР°С‚СЂРёС†Рµ
	add ebp, ebp, 4
	fmul [ebp+0], edx, eax
	fmul [ebp+12], edx, ebx
	mov [ebp+24], eax


	pop edx
	pop ecx
	pop ebx
	pop eax
	ret

;СЃС‡РёС‚Р°РµС‚ СЃРєР°Р»СЏСЂРЅРѕРµ РїСЂРѕРёР·РІРµРґРµРЅРёРµ РІРµРєС‚РѕСЂР° (eax,ebx,ecx) Рё РІРµРєС‚РѕСЂР° РёР· [esi]
;СЂРµР·СѓР»СЊС‚Р°С‚ РІ edx
dot_vector:
	push eax
	push ebx
	push ecx

	fmul eax, [esi+0], eax
	fmul ebx, [esi+4], ebx
	fmul ecx, [esi+8], ecx
	
	mov edx, 0.0
	fadd edx, ebx, eax
	fadd edx, edx, ecx

	pop ecx
	pop ebx
	pop eax
	ret

;РЅР° РІС…РѕРґ РїРѕРґР°РµС‚СЃСЏ (eax,ebx,ecx) РІРµРєС‚РѕСЂ
;РЅР° РІС‹С…РѕРґРµ РїРѕР»СѓС‡Р°РµРј РЅРѕРІС‹Р№ РІРµРєС‚РѕСЂ
mul_vector_to_matrix:
	

	mov esi, matrix
	call dot_vector
	push edx


	add esi, esi, 12
	call dot_vector
	push edx

	add esi, esi, 12
	call dot_vector
	push edx

	pop ecx
	pop ebx
	pop eax


	ret

;С„СѓРЅРєС†РёСЏ РїРѕ РїРѕР»СЏСЂРЅС‹Рј СѓРіР»Р°Рј (eax,ebx) СЃС‚СЂРѕРёС‚ РµРґРёРЅРёС‡РЅС‹Р№ РІРµРєС‚РѕСЂ (eax,ebx,ecx)
get_uniform:
	
	mov ecx, ebx
	mov ebx, eax
	mov edx, ecx

	fcos eax
	fsin ebx
	fcos ecx
	fsin edx

	mov ebp,esp
	mov [ebp+0], edx
	mov [ebp+4], edx
	mov [ebp+8], ecx

	fmul [ebp+0], eax, [ebp+0]
	fmul [ebp+4], ebx, [ebp+4]

	mov eax, [ebp+0]
	mov ebx, [ebp+4]
	mov ecx, [ebp+8]
	
	ret



;РїСЂРѕРІРµСЂРєР° СЃС‚РѕР»РєРЅРѕРІРµРЅРёСЏ СЃРѕ СЃС„РµСЂРѕР№ С†РµРЅС‚СЂ СЃС„РµСЂС‹ РІ (center_x,0,center_z)
; eax, ebx, ecx --- (x,y,z) --- РІРµРєС‚РѕСЂ РЅР°РїСЂР°РІР»РµРЅРёСЏ
check_collision:
	
	push eax
	push ebx

	call get_uniform

	;vec3 diff = (0,0,1) - (5,0,center_z);
	
	;alloc memory
	mov ebp, esp
	add esp, esp, 24 

	;diff vector
	fsub [ebp+0], 0.0, [center_x]
	mov [ebp+4],  0.0
	fsub [ebp+8], [camera_z], [center_z]

	

	;unit vector
	mov [ebp+12],eax
	mov [ebp+16],ebx
	mov [ebp+20],ecx

	

	; doblue discriminant = dot(unit, diff)^2 + radius^2 - dot(diff,diff)

	;eax == dot(unit, diff)^2
	mov esi, ebp
	mov edi, ebp
	add edi, edi, 12
	call dot_vector_mm
	mov eax, edx
		push eax
		fabs eax
		fadd [distance], eax, [distance] ; РґРѕР±Р°РІР»СЏРµРј РїСЂРѕР№РґРµРЅРЅРѕРµ СЂР°СЃСЃС‚РѕСЏРЅРёРµ, С‚РѕС‡РЅРµРµ "РїРµСЂРІСѓСЋ" С‡Р°СЃС‚СЊ
		pop eax
	fpow eax, 2.0

	
	
	;eax == dot(unit, diff)^2 + radius^2
	mov edx, [radius]
	fpow edx, 2.0
	fadd eax, edx, eax
	
	;eax == dot(unit, diff)^2 + radius^2 - dot(diff,diff)
	mov esi, ebp
	mov edi, ebp
	call dot_vector_mm
	fsub eax, eax, edx



	fja 0.0, eax, no_intersection

	;РµСЃС‚СЊ РїРµСЂРµСЃРµС‡РµРЅРёРµ СЃРѕ СЃС„РµСЂРѕР№
		;distance += |dot(diff,unit)| - sqrt(discriminant);
		fsqrt eax
		fsub [distance], [distance], eax

		;С‚РµРїРµСЂСЊ РЅРµРѕР±С…РѕРґРёРјРѕ РїСѓСЃС‚РёС‚СЊ СЃР»РµРґСѓСЋС‰РёР№ Р»СѓС‡ СѓР¶Рµ РёР· СЃС„РµСЂС‹

		push edx
		mov edx, [distance]

		fmul eax, [ebp+12], edx
		fmul ebx, [ebp+16], edx
		fmul ecx, [ebp+20], edx
		pop edx

		;РјРµРЅСЏРµРј РјРµСЃС‚Рѕ, РёР· РєРѕС‚РѕСЂРѕРіРѕ РІС‹РїСѓСЃРєР°РµС‚СЃСЏ Р»СѓС‡
		mov esi, origin
		fadd [esi+0], [esi+0], eax
		fadd [esi+4], [esi+4], ebx
		fadd [esi+8], [esi+8], ecx

		; n = dist * uniform - to_center
		fadd [ebp+0], [ebp+0], eax
		fadd [ebp+4], [ebp+4], ebx
		fadd [ebp+8], [ebp+8], ecx

		mov esi, ebp
		call normalize
		; vec 3 diff == normal from sphere center to side --- РїСЂРѕРІРµСЂРєСѓ РїСЂРѕС€Р»Р°

		mov esi, ebp
		mov edi, ebp
		add edi, edi, 12
		call dot_vector_mm
		fmul edx, edx, 2.0 ; if 2.0 -> refraction, if -2.0 reflection
		
		fmul [ebp+0], edx, [ebp+0]
		fmul [ebp+4], edx, [ebp+4]
		fmul [ebp+8], edx, [ebp+8]

		fadd [ebp+12],[ebp+0],[ebp+12]
		fadd [ebp+16],[ebp+4],[ebp+16]
		fadd [ebp+20],[ebp+8],[ebp+20]

		mov eax, [ebp+12]
		mov ebx, [ebp+16]
		mov ecx, [ebp+20]

		; С‚РµРїРµСЂСЊ РІ eax,ebx,ecx Р»РµР¶РёС‚ РѕР±РЅРѕРІР»РµРЅРЅРѕРµ РЅР°РїСЂР°РІР»РµРЅРёРµ
		; + РёР·РјРµРЅРµРЅ origin

		call get_sym_at_floor
		
		;call get_light_symbol
		
		;mov ecx, 43
		mov edx, 0
	
	jmp end_intersection
	no_intersection:
		mov edx, 0xFF
	end_intersection:

	sub esp, esp, 24
	pop ebx
	pop eax
	ret


;РїРѕ esi РІРµРєС‚РѕСЂ РєРѕС‚РѕСЂС‹Р№ РЅСѓР¶РЅРѕ РЅРѕСЂРјР°Р»РёР·РѕРІР°С‚СЊ
normalize:
	push edx
	push edi
	mov edi, esi
	call dot_vector_mm
	fsqrt edx

	fdiv [esi+0], [esi+0], edx
	fdiv [esi+4], [esi+4], edx
	fdiv [esi+8], [esi+8], edx

	pop edi 
	pop edx
	ret

; РїРѕРґСЃС‡РµС‚ СЃРєР°Р»СЏСЂРЅРѕРіРѕ РїСЂРѕРёР·РІРµРґРµРЅРёСЏ [esi],[edi] -> edx
dot_vector_mm:
	push eax
	mov edx, 0

	mov eax, [esi+0]
	fmul eax, [edi+0], eax
	fadd edx, eax, edx

	mov eax, [esi+4]
	fmul eax, [edi+4], eax
	fadd edx, eax, edx

	mov eax, [esi+8]
	fmul eax, [edi+8], eax
	fadd edx, eax, edx
	

	pop eax
	ret

main:


	mov [time], 0.0
	mov edi, 0.5
	mov esi, 0.0
	call crate_matrix

	infiity_loop:

	

	fadd [time], [time], 0.2

	mov eax, [time]
	fsin eax
	fpow eax, 2.0
	fmul [center_z], 0.7, eax
	fadd [center_z], [center_z], 1.0



	mov eax, 0
	lbl_loop_x:
		mov ebx, 0
		lbl_loop_y:
			
			mov ebp, origin
			mov [ebp+0], 0.0
			mov [ebp+4], 0.0
			mov [ebp+8], [camera_z]


			mov [distance], 0.0
			push eax
			push ebx

			call get_angles ;С‚РµРїРµСЂСЊ Сѓ РЅР°СЃ РІ eax, ebx Р»РµР¶Р°С‚ РїРѕР»СЏСЂРЅС‹Рµ СѓРіР»С‹

	


			call check_collision ; РµСЃР»Рё edx == 0, С‚Рѕ РІС‹РІРѕРґРёРј РЅР° СЌРєСЂР°РЅ
			je edx, 0, find_intersection

			mov [distance], 0.0
			call get_uniform ;РїРѕР»СѓС‡РёР»Рё РёР· СѓРіР»РѕРІ РІРµРєС‚РѕСЂ eax,ebx,ecx

			call get_sym_at_floor ;С‚РµРїРµСЂСЊ РјС‹ РёРјРµРµРј РІ ecx СЃРёРјРІРѕР», РєРѕС‚РѕСЂС‹Р№ РЅСѓР¶РЅРѕ РІС‹РІРѕРґРёС‚СЊ
			 find_intersection:

			pop ebx
			pop eax

			push eax
			push ebx
			sub ebx, [height], ebx 
			movb [symbol], ecx
			call draw_symbol
			pop ebx    
			pop eax

			add ebx, ebx, 1
			jne ebx, [height], lbl_loop_y
		
		add eax, eax, 1
		jne eax, [width], lbl_loop_x

		dump 

	;jmp infiity_loop

exit:
	hlt
.data
	@lighting db 0x20,0x2e,0x3a,0x3b,0x7e,0x3d,0x23,0x4f,0x42,0x38,0x25,0x26 alloc 12
	@symbol db 43 alloc 1
	@width  dq 0  alloc 1
	@height dq 0  alloc 1
	@matrix dq 0 alloc 9
	@time dq 0 alloc 1
	@radius dq 1.0 alloc 1
	@center_x dq 3.0 alloc 1 
	@center_z dq 2.0 alloc 1
	@distance dq 0.0 alloc 1
	@origin dq 0 alloc 3
	@camera_z dq 2.5 alloc 3