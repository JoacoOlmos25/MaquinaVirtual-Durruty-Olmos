texto       equ "Constante protegida"

main:       mov     edx, ks
            add     edx, texto
            mov     b[edx], 65      ; Intenta sobrescribir la 'C' con 'A' en el KS
            ret