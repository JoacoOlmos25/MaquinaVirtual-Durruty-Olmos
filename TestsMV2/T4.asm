main:       mov     edx, ds
            add     edx, 4          ; Nos movemos a DS + 4
            
            mov     eax, 0xDEAD     ; Dato basura
            mov     [edx-8], eax    ; Intento de Inyección: offset -4 invade el CS
            
            ret