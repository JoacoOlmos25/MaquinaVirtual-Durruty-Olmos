inicio:   mov   eax, 10
          jmp   2000             ; Salto incondicional hacia una dirección lógica que excede el código
          add   eax, 1           ; Esta instrucción nunca se ejecuta