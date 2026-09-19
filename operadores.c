#include "operadores.h"
#include "MaquinaVirtual.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

//Los 28 operaciones del ASSEMBLER
// firmas de los operadores

void MOV(TipoMV *MV){

    // 1. EXTRAER TIPO Y VALOR DEL OPERANDO 2 (ORIGEN)
    // Desplazamos 24 bits a la derecha para que quede solo el byte superior (el tipo)
    uint8_t tipo_op2 = MV->registros[OP2] >> 24;
    
    // Usamos una máscara AND para quedarnos solo con los 3 bytes inferiores (el valor)
    int32_t val_op2 = MV->registros[OP2] & 0x00FFFFFF;

    int32_t dato_a_mover = 0;

    // Evaluamos de dónde sacar el dato original
    if (tipo_op2 == 1) { // Es un registro
        // ¡LA MAGIA OCURRE AQUÍ! Si val_op2 es 10, lee MV->registros[10] (EAX)
        dato_a_mover = MV->registros[val_op2]; 
    } 
    else if (tipo_op2 == 2) { // Es un inmediato
        // El dato es directamente el valor que extrajimos
        dato_a_mover = val_op2;
    }
    // (Faltaría la lógica si tipo_op2 == 3, que es memoria)


    // 2. EXTRAER TIPO Y VALOR DEL OPERANDO 1 (DESTINO)
    uint8_t tipo_op1 = MV->registros[OP1] >> 24;
    int32_t val_op1 = MV->registros[OP1] & 0x00FFFFFF;

    // Evaluamos dónde guardar el dato
    if (tipo_op1 == 1) { // Es un registro
        // Si val_op1 es 10, esto equivale a hacer MV->registros[EAX] = 2;
        MV->registros[val_op1] = dato_a_mover;
    }
    // (Faltaría la lógica si tipo_op1 == 3, que es guardar en memoria)

    
    // 3. ACTUALIZAR REGISTRO CC (Flags)
    // El apunte indica que MOV afecta los flags de signo (N) y cero (Z)[cite: 6].
    // ... lógica del registro CC ...

}

void ADD(){

}

void SUB(){

}

void MUL(){
    
}

void DIV(){

}

void CMP(){

}

void AND(){

}

void OR(){

}

void SWAP(){

}

void SHL(){

}

void SHR(){

}

void SAR(){

}

void LDL(){

}

void LDH(){

}

void RND(){

}

