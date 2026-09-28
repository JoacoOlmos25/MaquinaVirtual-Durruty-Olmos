inicio:   mov   edx, DS          ; EDX apunta al inicio del Data Segment
          add   edx, 16384       ; Desplaza el puntero fuera de la memoria física total (16 KiB)
          mov   [edx], eax       ; Intenta escribir en una dirección inválida. Debe abortar.
          stop                   ; Detiene la ejecución del programa