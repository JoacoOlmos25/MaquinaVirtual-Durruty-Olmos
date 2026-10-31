main:       mov     eax, 0
            mov     al, 255         ; al = 0xFF (1 byte)
            mov     ah, 1           ; ah = 0x01 -> ax queda en 0x01FF (511)
            
            mov     b[ds], al       ; Guarda 1 byte (0xFF) en [DS+0]
            mov     w[ds+1], ax     ; Guarda 2 bytes (0x01FF) en [DS+1]
            mov     l[ds+3], eax    ; Guarda 4 bytes (0x000001FF) en [DS+3]
            sys     0xF             ; Breakpoint para inspeccionar
            ; Configura la impresión para volcar la memoria afectada
            mov     edx, ds
            mov     eax, 0x10       ; Formato Hexadecimal
            ldh     ecx, 4          ; Tamaño de 4 bytes
            ldl     ecx, 2          ; Imprime 2 bloques (8 bytes totales)
            sys     2
            sys     0xF             ; Breakpoint para inspeccionar
            ret