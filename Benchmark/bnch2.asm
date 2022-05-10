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

;РїРѕР»СѓС‡Р°РµРј СЂР°СЃСЃС‚РѕВ¤РЅРёРµ РґРѕ РїРѕР»Р° eax - phi, ebx - theta
;РІРѕР·РІСЂР°С‰Р°РµРј РѕС‚РІРµС‚ РІ ecx
get_floor_dist:
	push eax
	push ebx

	fcos ebx
	fdiv ecx, -1.0, ebx

	pop ebx
	pop eax

	ret

;РїРѕР»СѓС‡Р°РµРј СЂР°СЃСЃС‚РѕВ¤РЅРёРµ РґРѕ РїРѕР»Р° eax, ebx, ecx --- РІРµРєС‚РѕСЂ РЅР°РїСЂР°РІР»РµРЅРёВ¤
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

;РїРѕР»СѓС‡Р°РµРј В¤СЂРєРѕСЃС‚РЅС‹Р№ СЃРёРјРІРѕР» РёСЃС…РѕРґВ¤ РёР· СЂР°СЃСЃС‚РѕВ¤РЅРёВ¤
;in ecx --- СЂР°СЃСЃС‚РѕВ¤РЅРёРµ Р»СѓС‡Р°
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
	mov ecx, edx  ;СЃРёРјРІРѕР» РїРѕ В¤СЂРєРѕСЃС‚Рё
	pop edx
	ret


;РїРѕР»СѓС‡Р°РµРј СЃРёРјРІРѕР» РїСЂРё СЃС‚РѕР»РєРЅРѕРІРµРЅРёРё СЃ РїРѕР»РѕРј eax,ebx,ecx --- РµРґРёРЅРёС‡РЅС‹Р№ РЅР°РїСЂР°РІР»В¤СЋС‰РёР№ РІРµРєС‚РѕСЂ
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
	pusha

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


	popa
	ret

;СЃС‡РёС‚Р°РµС‚ СЃРєР°Р»В¤СЂРЅРѕРµ РїСЂРѕРёР·РІРµРґРµРЅРёРµ РІРµРєС‚РѕСЂР° (eax,ebx,ecx) Рё РІРµРєС‚РѕСЂР° РёР· [esi]
;СЂРµР·СѓР»СЊС‚Р°С‚ РІ edx
dot_vector:

	mov edx, 0
	lexpanda lr0
	
	lmov lr1, [esi], 4
	lfmul lr2, lr1, lr0
	lfaccum edx, lr2
	

	ret

;РЅР° РІС…РѕРґ РїРѕРґР°РµС‚СЃВ¤ (eax,ebx,ecx) РІРµРєС‚РѕСЂ
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

;С„СѓРЅРєС†РёВ¤ РїРѕ РїРѕР»В¤СЂРЅС‹Рј СѓРіР»Р°Рј (eax,ebx) СЃС‚СЂРѕРёС‚ РµРґРёРЅРёС‡РЅС‹Р№ РІРµРєС‚РѕСЂ (eax,ebx,ecx)
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


dot_vector_ll:
	lfmul lr1, lr1, lr0
	lfaccum edx, lr1
	ret

;РїСЂРѕРІРµСЂРєР° СЃС‚РѕР»РєРЅРѕРІРµРЅРёВ¤ СЃРѕ СЃС„РµСЂРѕР№ С†РµРЅС‚СЂ СЃС„РµСЂС‹ РІ (center_x,center_y,center_z)
; eax, ebx, ecx --- (x,y,z) --- РІРµРєС‚РѕСЂ РЅР°РїСЂР°РІР»РµРЅРёВ¤
check_collision:
	
	push eax
	push ebx

	call get_uniform

	;vec3 diff = (0,0,cam_z) - (center_x,center_y,center_z);
	

	;unit <=> lr7
	mov edx, 0.0
	lexpanda lr7

	;diff <=> lr6
	fsub eax, [center_x], 0.0
	fsub ebx, [center_y], 0.0
	fsub ecx, [center_z], [camera_z] 
	lexpanda lr6

	;============evaluating discriminant=======================
	;eax == dot(unit, diff)^2 + radius^2 - dot(diff,diff)
	lmov lr0, lr6, 4
	lmov lr1, lr7, 4 
	call dot_vector_ll ;scalar result in edx

	mov eax, edx
		fadd [distance], eax, [distance] ; РґРѕР±Р°РІР»В¤РµРј РїСЂРѕР№РґРµРЅРЅРѕРµ СЂР°СЃСЃС‚РѕВ¤РЅРёРµ, С‚РѕС‡РЅРµРµ "РїРµСЂРІСѓСЋ" С‡Р°СЃС‚СЊ
	fpow eax, 2.0
	
	;eax == dot(unit, diff)^2 + radius^2
	mov edx, [radius]
	fpow edx, 2.0
	fadd eax, edx, eax
	
	lmov lr0, lr6, 4
	lmov lr1, lr6, 4
	call dot_vector_ll ;scalar result in edx
	fsub eax, eax, edx
	;==========================================================

	fja 0.0, eax, no_intersection

		;РµСЃС‚СЊ РїРµСЂРµСЃРµС‡РµРЅРёРµ СЃРѕ СЃС„РµСЂРѕР№
		;distance += |dot(diff,unit)| - sqrt(discriminant);
		fsqrt eax
		fsub [distance], [distance], eax

		lmov lr5, lr7, 4   ; copy unit vector to lr5
		mov edx, [distance]
		lexpand lr4, 0xFF   ;lr4 == (edx,edx,edx,edx)
		lfmul lr5, lr5, lr4 ;lr5 == unit * dist_to_sphere

		llz lr4
		lmov lr4, [origin], 3
		lfadd lr4, lr4, lr5   ;new_origin = origin + unit * dist_to_sphere
		lmov [origin], lr4, 3 ;save new_origin into [origin]

		lfsub lr5, lr5, lr6 ;lr5 == unit * dist_to_sphere - diff <=> normal

		lmov lr0, lr5, 4
		lmov lr1, lr7, 4
		call dot_vector_ll  ;dot(unit, normal)
		fmul edx, edx, 2.0
		lexpand lr4, 0xFF   ;lr4 == (edx,edx,edx,edx)
		lfmul lr5, lr5, lr4 ;lr5 == 2*dot(n,unit)*n
		lfadd lr7, lr7, lr5 ;lr7 == unit + 2*dot(n,unit)*n
		lshrinka lr7


		;in (eax,ebx,ecx) - new unit vector, origin - point on sphere
		call get_sym_at_floor
		;mov ecx, 43
		mov edx, 0
	
	jmp end_intersection
	no_intersection:
		mov edx, 0xFF
	end_intersection:

	pop ebx
	pop eax

	ret

main:

	llza

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

			call get_angles ;С‚РµРїРµСЂСЊ Сѓ РЅР°СЃ РІ eax, ebx Р»РµР¶Р°С‚ РїРѕР»В¤СЂРЅС‹Рµ СѓРіР»С‹

	


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
	@center_y dq 0.0 alloc 1
	@center_z dq 2.0 alloc 1
	@distance dq 0.0 alloc 1
	@origin dq 0 alloc 3
	@camera_z dq 2.5 alloc 1