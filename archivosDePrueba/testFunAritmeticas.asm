inicio:   mov   eax, 15          ; Carga el operando inmediato 15 en EAX
          mov   ebx, 10          ; Carga el operando inmediato 10 en EBX
          add   eax, ebx         ; Suma EBX a EAX (15 + 10 = 25)
          mov   edx, DS          ; EDX apunta al segmento de datos
          add   edx, 10          ; Se aplica un desplazamiento a la posición 10
          mov   [edx], eax       ; Guarda el resultado (25) en la memoria apuntada por EDX
          mov   eax, 0x01        ; Configura EAX con 0x01 para que la salida sea en decimal
          ldh   ecx, 4           ; Configura la parte alta de ECX para un tamaño de 4 bytes
          ldl   ecx, 1           ; Configura la parte baja de ECX para imprimir 1 cantidad
          sys   2                ; Ejecuta la llamada WRITE (2)
          stop                   ; Detiene la ejecución del programa