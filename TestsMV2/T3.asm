rutina:     push    eax
            pop     ebx
            ret                     ; Retorna limpiamente al main
            
main:       mov     eax, 99
            call    rutina
            sys     0xF             ; Breakpoint
            pop     ecx             ; 1. Saca el -1 (RET)
            pop     edx             ; 2. Saca argc
            pop     eex             ; 3. Saca *argv (aca la pila queda 100% vacia)
            pop     efx             ; 4. ERROR: Intenta sacar de la pila vacia -> STACK UNDERFLOW!
            
            ret