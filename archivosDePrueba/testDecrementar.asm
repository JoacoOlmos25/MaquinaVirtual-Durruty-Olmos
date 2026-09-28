inicio:   mov   edx, DS          ; EDX apunta al inicio del Data Segment
          mov   eax, 0x01        ; Configura EAX con 0x01 para leer en formato decimal[cite: 13]
          ldh   ecx, 4           ; Configura el tamaño del dato a leer en 4 bytes[cite: 13]
          ldl   ecx, 1           ; Configura la cantidad de valores a leer en 1[cite: 13]
          sys   1                ; Ejecuta la llamada READ (1) para pedir input al usuario[cite: 13]
          mov   ebx, [edx]       ; Carga el valor ingresado desde la memoria hacia EBX

ciclo:    cmp   ebx, 0           ; Resta 0 a EBX y modifica los bits del registro CC sin guardar el resultado[cite: 13]
          jz    fin              ; Evalúa el bit Z (Cero); si es 1, salta a 'fin'[cite: 13]
          mov   [edx], ebx       ; Guarda el valor actual del contador en memoria para imprimirlo
          mov   eax, 0x01        ; Asegura el formato decimal para la salida en pantalla[cite: 13]
          ldh   ecx, 4           ; Tamaño de 4 bytes[cite: 13]
          ldl   ecx, 1           ; Imprime 1 valor[cite: 13]
          sys   2                ; Llamada WRITE (2) para imprimir el valor[cite: 13]
          sub   ebx, 1           ; Resta 1 al contador (afecta el registro CC)[cite: 13]
          jmp   ciclo            ; Salto incondicional hacia el rótulo 'ciclo' para repetir[cite: 13]

fin:      stop                   ; Detiene la ejecución[cite: 13]