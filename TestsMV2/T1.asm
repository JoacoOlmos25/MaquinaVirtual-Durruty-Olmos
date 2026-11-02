msg_test    equ "Iniciando test de registros...\n"

main:       mov     edx, ks
            add     edx, msg_test
            sys     4               ; Imprime el string desde KS

            ; Volcamos los registros de segmento a registros de propósito general
            mov     eax, ks
            mov     ebx, ss
            mov     ecx, es
            mov     edx, ds
            
            ; Volcamos los registros de pila
            mov     eex, sp
            mov     efx, bp
            
            sys     0xF             ; Breakpoint para inspeccionar
            ret