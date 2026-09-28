inicio:   mov   eax, 2147483647  ; Carga el valor máximo positivo de 32 bits
          add   eax, 1           ; Fuerza un overflow. El resultado pasa a ser negativo.
                                 ; Debe encender V (Desbordamiento) y N (Signo).
          
          ; --- IMPRIMIR CC (Prueba 1) ---
          mov   edx, DS          ; EDX apunta al inicio del Data Segment
          mov   [edx], CC        ; Copia el estado actual de los flags a memoria
          mov   eax, 0x10        ; Configura EAX en 0x10 para salida en binario[cite: 7]
          ldh   ecx, 4           ; Tamaño del dato a imprimir: 4 bytes[cite: 7]
          ldl   ecx, 1           ; Cantidad de valores a imprimir: 1[cite: 7]
          sys   2                ; Llamada WRITE para volcar la memoria a la consola[cite: 7]
          
          mov   ebx, -1          ; Carga -1 (0xFFFFFFFF en memoria)
          add   ebx, 1           ; Fuerza un carry. El resultado vuelve a cero.
                                 ; Debe encender C (Acarreo) y Z (Cero).
          
          ; --- IMPRIMIR CC (Prueba 2) ---
          mov   [edx], CC        ; Pisa la memoria con el nuevo estado de los flags
          sys   2                ; Vuelve a imprimir (EAX, EDX y ECX ya quedaron configurados)
          
          stop                   ; Detiene la ejecución del programa[cite: 7]