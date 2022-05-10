.text

jmp main

    ;in eax
    ;return eax == 1 if |eax| < 0.01
    is_zero:
        fabs eax
        jae eax, 0.01, _isNotZero
        mov eax, 1
        ret
        _isNotZero:
        mov eax, 0
        ret
    
    ;in eax, ebx --- factors in a*x + b = 0
    ;return ecx = -b/a
    solve_linear:
        fmul ebx, ebx, -1.0
        fdiv ecx, ebx, eax
        ret
        
    solve_square:
        mov edx, 1
        
    
        push eax
        call is_zero
        and edx, edx, eax
        
        mov eax, ebx
        call is_zero
        and edx, edx, eax
        mov eax, ecx
        call is_zero
        and edx, edx, eax
        pop eax
        
        
        jne edx, 1, not_all_zero
            ret
        
        not_all_zero:
        
        
        push eax
        call is_zero
        mov edx, eax
        pop eax
        
        jne edx, 1, a_is_not_zero
            mov eax, ebx
            mov ebx, ecx
            call solve_linear
            out ecx
            ret
        
        a_is_not_zero:
        
        push eax
        mov ecx, eax
        call is_zero
        mov edx, eax
        pop eax
        
        jne edx, 1, c_is_not_zero
            out 0x00
            call solve_linear
            out ecx
            ret
        c_is_not_zero:
        
        ;ecx == D^2
        fmul ecx, 4.0, ecx
        fmul ecx, ecx, eax
        push ebx
        fpow ebx, 2.0
        fsub ecx, ebx, ecx
        pop ebx
        
        ja 0.0, ecx, no_roots
        ja ecx, 0.0, two_roots
        
        ;D == 0
        
        fmul eax, -2.0, eax
        fdiv ebx, ebx, eax
        out ebx
        ret
        
        no_roots:
        ret
        
        two_roots:
        fpow ecx, 0.5
        
        fmul eax, eax, 2.0
        fdiv ecx, ecx, eax
        fdiv ebx, ebx, eax
        
        fmul ebx, ebx, -1.0
        
        fadd eax, ebx, ecx
        out eax
        fsub eax, ebx, ecx
        out eax
        ret
        
        
    main:
    fin eax
    fin ebx
    fin ecx
    call solve_square
    hlt