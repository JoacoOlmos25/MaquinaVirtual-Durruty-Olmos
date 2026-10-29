msg_hola    equ "Hola MV2!"

; Subrutina que suma dos parametros recibidos por pila y retorna en EAX
sumar:      push    bp
            mov     bp, sp
            mov     eax, l[bp+8]
            add     eax, l[bp+12]
            mov     sp, bp
            pop     bp
            ret

; Subrutina principal (entry point: no esta al inicio del codigo)
main:       mov     edx, ks
            add     edx, msg_hola
            sys     4 ;imprime "Hola MV2!"

            ; Probamos sub-registros (AL/AH/AX) y tamanios de memoria (b/w)
            mov     ebx, 0
            mov     al, 10
            mov     ah, 20
            mov     b[ds], al       ; Guarda 1 byte (10) en DS+0
            mov     w[ds+2], ax     ; Guarda 2 bytes en DS+2

            ; Probamos CALL, PUSH, POP y RET
            push    25
            push    17
            call    sumar
            add     sp, 8           ; Limpiamos los 2 argumentos de la pila

            mov     l[ds+4], eax    ; Guarda 42 (25 + 17) en DS+4

            ; Imprimimos DS+4 en decimal (debe dar 42)
            mov     edx, ds
            add     edx, 4
            ldh     ecx, 4
            ldl     ecx, 1
            mov     eax, 1
            sys     2

            sys     0xF             ; Breakpoint
            ret                     ; Saca el -1 inicial de la pila y termina