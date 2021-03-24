.text
mov edi, 0
movw edi,  [0x9FE]  ;считываем количество колонок <-> x
mov [width], edi

mov edi, 0
movw edi, [0x9FC] ;считваем количество линий <-> y
mov [height], edi

jmp main


;рисует символ из [symbol] по координатам (eax,ebx)
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

;получаем расстояние до пола eax - phi, ebx - theta
;возвращаем ответ в ecx
get_floor_dist:
	push eax
	push ebx

	fcos ebx
	fdiv ecx, -1.0, ebx

	pop ebx
	pop eax

	ret

;получаем расстояние до пола eax, ebx, ecx --- вектор направления
;возвращаемое значение в ecx
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

;получаем яркостный символ исходя из расстояния
;in ecx --- расстояние луча
;out ecx --- код символа
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
	mov ecx, edx  ;символ по яркости

	pop edx

	ret


;получаем символ при столкновении с полом eax,ebx,ecx --- единичный направляющий вектор
;символ в ecx
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

	;первый столбик в матрице
	mov ebp, matrix
	fmul [ebp+0], ecx, eax
	fmul [ebp+12], ecx, ebx
	fmul [ebp+24], -1.0, edx

	;второй столбик в матрице
	add ebp, ebp, 4
	fmul [ebp+0], ebx, -1.0
	mov [ebp+12], eax
	mov [ebp+24], 0.0

	;третий столбик в матрице
	add ebp, ebp, 4
	fmul [ebp+0], edx, eax
	fmul [ebp+12], edx, ebx
	mov [ebp+24], eax


	pop edx
	pop ecx
	pop ebx
	pop eax
	ret

;считает скалярное произведение вектора (eax,ebx,ecx) и вектора из [esi]
;результат в edx
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

;на вход подается (eax,ebx,ecx) вектор
;на выходе получаем новый вектор
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

;функция по полярным углам (eax,ebx) строит единичный вектор (eax,ebx,ecx)
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



;проверка столкновения со сферой центр сферы в (center_x,0,center_z)
; eax, ebx, ecx --- (x,y,z) --- вектор направления
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
		fadd [distance], eax, [distance] ; добавляем пройденное расстояние, точнее "первую" часть
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

	;есть пересечение со сферой
		;distance += |dot(diff,unit)| - sqrt(discriminant);
		fsqrt eax
		fsub [distance], [distance], eax

		;теперь необходимо пустить следующий луч уже из сферы

		push edx
		mov edx, [distance]

		fmul eax, [ebp+12], edx
		fmul ebx, [ebp+16], edx
		fmul ecx, [ebp+20], edx
		pop edx

		;меняем место, из которого выпускается луч
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
		; vec 3 diff == normal from sphere center to side --- проверку прошла

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

		; теперь в eax,ebx,ecx лежит обновленное направление
		; + изменен origin

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


;по esi вектор который нужно нормализовать
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

; подсчет скалярного произведения [esi],[edi] -> edx
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

			call get_angles ;теперь у нас в eax, ebx лежат полярные углы

	


			call check_collision ; если edx == 0, то выводим на экран
			je edx, 0, find_intersection

			mov [distance], 0.0
			call get_uniform ;получили из углов вектор eax,ebx,ecx

			call get_sym_at_floor ;теперь мы имеем в ecx символ, который нужно выводить
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