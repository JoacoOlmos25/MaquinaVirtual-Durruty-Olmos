inicio:   mov   eax, 15          ; Carga 15 en el registro EAX
          mov   ebx, 0           ; Carga 0 en el registro EBX
          div   eax, ebx         ; Intento de división por cero. El programa debe abortar aquí.
          mov   ecx, 1           ; Esta instrucción nunca debería ejecutarse
          stop                   ; Detiene la ejecución del programa