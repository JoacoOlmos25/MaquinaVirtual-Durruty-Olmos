#include "operadores.h"
#include "MaquinaVirtual.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "mascaras.h"
#include "OperacionesMem.c"

//Los 28 operaciones del ASSEMBLER
// firmas de los operadores

void actualizaCC(TipoMV *MV, int64_t res_con_signo, uint64_t res_sin_signo) {
    // Limpiamos solo los 4 bits más altos (NZCV), preservando los 28 bits reservados
    MV->registros[CC] &= 0x0FFFFFFF;

    // Recortamos el resultado a 32 bits reales para evaluar Signo y Cero
    int32_t res32 = (int32_t)res_con_signo;

    // Bit N (Signo): se activa cuando el resultado es negativo
    if (res32 < 0) {
        MV->registros[CC] |= (1 << 31);
    }

    // Bit Z (Cero): se activa cuando el resultado es cero
    if (res32 == 0) {
        MV->registros[CC] |= (1 << 30);
    }

    // Bit C (Acarreo): se activa cuando el resultado excede los 32 bits disponibles
    if (res_sin_signo > 0xFFFFFFFF) {
        MV->registros[CC] |= (1 << 29);
    }

    // Bit V (Desbordamiento): se activa cuando el resultado es erróneo por overflow con signo
    if (res_con_signo > 2147483647LL || res_con_signo < -2147483648LL) {
        MV->registros[CC] |= (1 << 28);
    }
}

void guardarDato(TipoMV *MV, uint8_t tipo_destino, int32_t val_destino, int32_t dato) {
    if (tipo_destino == 1) { 
        // Es un Registro (1 byte)
        // val_destino contiene directamente el índice del registro (ej. 10 para EAX)
        MV->registros[val_destino] = dato;
    } 
    else if (tipo_destino == 3) { 
        // Es Memoria (3 bytes)
        // val_destino contiene la dirección lógica (segmento + desplazamiento)
        
        // 1. Cargamos la dirección lógica original en LAR
        MV->registros[LAR] = val_destino;
        
        // 2. Extraemos el código de segmento y el desplazamiento
        int16_t offset = val_destino & 0xFFFF;
        uint8_t reg_base = (val_destino >> 16) & 0x1F; 
        
        // Obtenemos la dirección física (asumiendo que usas tu función DirecLogica)
        uint16_t dir_fisica = DirecLogica(MV, offset, reg_base);
        
        // 3. MAR guarda la cant. de bytes (4) en la parte alta y la dir. física en la baja
        MV->registros[MAR] = (4 << 16) | dir_fisica; 
        
        // 4. MBR retiene el valor que se desea almacenar en la memoria
        MV->registros[MBR] = dato;
        
        // 5. Ejecutamos la escritura en la RAM byte a byte 
        // (Desplazamos para guardar un int32_t en un arreglo de uint8_t)
        MV->memoria[dir_fisica]     = (dato >> 24) & 0xFF;
        MV->memoria[dir_fisica + 1] = (dato >> 16) & 0xFF;
        MV->memoria[dir_fisica + 2] = (dato >> 8)  & 0xFF;
        MV->memoria[dir_fisica + 3] = dato & 0xFF;
    }
}





void MOV(TipoMV *MV){

    // 1. EXTRAER TIPO Y VALOR DEL OPERANDO 2 (ORIGEN)
    // Desplazamos 24 bits a la derecha para que quede solo el byte superior (el tipo)
    uint8_t tipo_op2 = MV->registros[OP2] >> 24;
    
    // Usamos una máscara AND para quedarnos solo con los 3 bytes inferiores (el valor)
    int32_t val_op2 = MV->registros[OP2] & Masc_Byte_Mas_Sig;

    int32_t dato_a_mover = 0;

    // Evaluamos de dónde sacar el dato original
    if (tipo_op2 == 1) { // Es un registro
        //Si val_op2 es 10, lee MV->registros[10] (EAX)
        dato_a_mover = MV->registros[val_op2]; 
    } 
    else if (tipo_op2 == 2) { // Es un inmediato
        // El dato es directamente el valor que extrajimos
        dato_a_mover = val_op2;
    }
    else {
        //Tipo de dato de memoria
    }


    // 2. EXTRAER TIPO Y VALOR DEL OPERANDO 1 (DESTINO)
    uint8_t tipo_op1 = MV->registros[OP1] >> 24;
    int32_t val_op1 = MV->registros[OP1] & Masc_Byte_Mas_Sig;

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

void ADD(TipoMV *MV) {
    // 1. Extraemos los paquetes completos de los registros OP1 y OP2
    int32_t paquete_op1 = MV->registros[OP1];
    int32_t paquete_op2 = MV->registros[OP2];

    // 2. Extraemos exclusivamente el tipo del destino (Operando 1) para poder guardar después
    // Desplazamos 24 bits lógicos hacia la derecha para aislar el byte más significativo
    uint8_t tipo_destino = (uint32_t)paquete_op1 >> 24;

    // 3. Obtenemos los valores matemáticos reales usando la función traductora
    int32_t val_destino = obtenerDato(MV, paquete_op1);
    int32_t val_origen = obtenerDato(MV, paquete_op2);

    // 4. Calculamos los resultados en 64 bits para evaluar desbordes
    // Resultado con signo tradicional
    int64_t res_con_signo = (int64_t)val_destino + (int64_t)val_origen;
    
    // Resultado sin signo usando máscaras AND (sin doble casteo)
    uint64_t res_sin_signo = ((uint64_t)val_destino & 0xFFFFFFFF) + ((uint64_t)val_origen & 0xFFFFFFFF);

    // 5. Guardamos el resultado de 32 bits en el destino (memoria o registro)
    // Extraemos la ubicación (los 3 bytes bajos) limpiando el tipo
    int32_t ubicacion_destino = paquete_op1 & 0x00FFFFFF; 
    guardarDato(MV, tipo_destino, ubicacion_destino, (int32_t)res_con_signo);

    // 6. Actualizamos las banderas N, Z, C, y V
    actualizaCC(MV, res_con_signo, res_sin_signo);
}

void SUB(TipoMV *MV){

}

void MUL(TipoMV *MV){
    
}

void DIV(TipoMV *MV){

}

void CMP(TipoMV *MV){

}

void AND(TipoMV *MV){

}

void OR(TipoMV *MV){

}

void XOR(TipoMV *MV){

}

void SWAP(TipoMV *MV){

}

void SHL(TipoMV *MV){

}

void SHR(TipoMV *MV){

}

void SAR(TipoMV *MV){

}

void LDL(TipoMV *MV){

}

void LDH(TipoMV *MV){

}

void RND(TipoMV *MV){

}

void SYS(TipoMV *MV){}

void JMP(TipoMV *MV){}

void JP(TipoMV *MV){}

void JN(TipoMV *MV){}

void JZ(TipoMV *MV){}

void JC(TipoMV *MV){}

void JV(TipoMV *MV){}

void JNP(TipoMV *MV){}

void JNN(TipoMV *MV){}

void JNZ(TipoMV *MV){}

void NOT(TipoMV *MV){}


void STOP(TipoMV *MV){}

