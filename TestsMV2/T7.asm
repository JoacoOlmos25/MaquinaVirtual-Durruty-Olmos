\\STACK 32

infinito:   push    eax             ; Gasta 4 bytes de pila
            call    infinito        ; Gasta otros 4 bytes (IP) y se llama a si misma
            ret

main:       mov     eax, 1
            call    infinito
            ret