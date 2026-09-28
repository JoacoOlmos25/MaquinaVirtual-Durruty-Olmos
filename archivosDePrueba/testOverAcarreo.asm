
inicio:   ldh   eax, 0x7FFF      ; Carga la parte alta de 2147483647
          ldl   eax, 0xFFFF      ; Carga la parte baja de 2147483647
          add   eax, 1           ; Fuerza un overflow (V=1 y N=1)
          ; --- IMPRIMIR CC (Prueba 1: Overflow y Negativo) ---
          mov   ebx, CC          ; Copiamos el registro CC a un registro general (EBX)
          mov   edx, DS          ; EDX apunta al inicio del Data Segment
          mov   [edx], ebx       ; Guardamos el contenido de EBX en la memoria apuntada por EDX
          mov   eax, 0x10        ; Configura EAX en 0x10 para salida en binario
          ldh   ecx, 4           ; Tamaño del dato a imprimir: 4 bytes
          ldl   ecx, 1           ; Cantidad de valores a imprimir: 1[cite: 18]
          sys   2                ; Llamada WRITE para volcar la memoria a la consola[cite: 18]
          
          mov   ebx, -1          ; Carga -1 (0xFFFFFFFF en memoria)
          add   ebx, 1           ; Fuerza un carry. El resultado vuelve a cero.
                                 ; Debe encender C (Acarreo) y Z (Cero).
          
          ; --- IMPRIMIR CC (Prueba 2: Acarreo y Cero) ---
          mov   ebx, CC          ; Volvemos a capturar el estado actualizado de CC
          mov   [edx], ebx       ; Pisamos la memoria con el nuevo estado de los flags
          mov   eax, 0x10        ; Aseguramos el formato binario en EAX[cite: 18]
          sys   2                ; Vuelve a imprimir (EDX y ECX ya quedaron configurados)
          
          stop                   ; Detiene la ejecución del programa[cite: 18]