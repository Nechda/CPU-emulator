.text
    in eax
    mov ebx, 1
    iteration:
        mul ebx, ebx, eax
        sub eax, eax, 1
        jne eax, 1, iteration
    out ebx
    hlt