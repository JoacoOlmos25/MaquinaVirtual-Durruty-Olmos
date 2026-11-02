msg_in      equ "Ingrese un texto corto: "
msg_out     equ "Memoria capturo: "

main:       mov     edx, ks
            add     edx, msg_in
            sys     4               ; Imprime prompt
            mov     edx, ds
            mov     ecx, 15         ; Limita la lectura a 15 caracteres
            sys     3               ; Pide string por teclado y guarda en DS

            mov     edx, ks
            add     edx, msg_out
            sys 0xF
            sys     4               ; Imprime prefijo de respuesta

            mov     edx, ds
            sys     4               ; Imprime el texto guardado en DS
            sys 0xF
            ret