sep         equ " -> "

main:       mov     bp, sp
            mov     ecx, l[ebp+4]   ; ECX = argc (cantidad de parametros)
            mov     ebx, l[ebp+8]   ; EBX = *argv (puntero al arreglo de punteros en PS)

ciclo:      cmp     ecx, 0
            jz      fin
            mov     edx, l[ebx]     ; Carga en EDX el puntero al string actual (argv[i])
            sys     4               ; Imprime el parametro i

            mov     edx, ks
            add     edx, sep
            sys     4               ; Imprime " -> "

            add     ebx, 4          ; Avanza al siguiente puntero de 4 bytes en argv
            sub     ecx, 1
            jmp     ciclo

fin:        sys     0xF             ; Breakpoint para ver el PS en vmg (0 a base de KS)
            ret