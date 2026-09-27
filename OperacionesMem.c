#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "OperacionesMem.h"
#include "MaquinaVirtual.h"
#include "mascaras.h"
//Operaciones de memoria(LAR, MAR, MBR)

//Faltan las exepciones sobre si me caigo del data segment 

uint16_t DirecLogica(TipoMV *MV, int32_t corrimiento, int8_t reg){
    // int seg;
    // uint16_t posini, offset;
    // seg = (MV -> registros[reg] >> 16) & 0xFFFF; //J cambie Masc_Byte_Menos_Sig -> 0xFFFF//accedo al segmento de la tabla segun el registro
    // //seg = MV -> registros[DS] >> 16; (harcodeado)
    // posini = MV -> tablaSegmento[seg].base; //& Masc_Byte_Menos_Sig; //traigo la posicion de memoria inicial
    // offset = MV -> registros[reg] & 0xFFFF; //J cambie Masc_Byte_Menos_Sig -> 0xFFFF//accedo al offset
    // return posini + offset + corrimiento;
    // Casteamos a uint32_t antes del corrimiento para que C no rompa el signo
    uint32_t registro_crudo = (uint32_t)MV->registros[reg];
    int seg = (registro_crudo >> 16) & 0xFFFF;
    uint16_t posini = MV->tablaSegmento[seg].base;
    uint16_t offset = registro_crudo & 0xFFFF;
    return posini + offset + corrimiento;
}

void lecturaDeMemoria(TipoMV *MV, int OP){
    int32_t valor;
    int8_t reg;
    uint16_t dirlogica, offset;
    valor = MV -> registros[OP];
    reg = valor & Masc_CodO;
    offset = (valor >> 8) & 0xFFFF;//cambie Masc_Compl por 0xFFFF;
    dirlogica = DirecLogica(MV, offset, reg); //tengo q ver si me cai del data segment
    MV->registros[LAR] = dirlogica;
    MV -> registros[MAR] = 4 << 24 | dirlogica;
}

void CargaAMemoria(TipoMV *MV, int OP){
    lecturaDeMemoria(MV, OP);
    uint16_t dir_fisica = MV->registros[MAR] & 0xFFFF;
    int leer = (MV->registros[MAR] >> 24) & 0xFF; 
    
    // Leemos del registro MBR real (32 bits)
    uint32_t dato = MV->registros[MBR]; 

    for (int i = 0; i < leer; i++){
        // Guardamos en la memoria RAM byte por byte (Big Endian)
        MV->memoria[dir_fisica + i] = (dato >> (8 * (leer - 1 - i))) & 0xFF;
    }
}

void TraigoDeMemoria(TipoMV *MV, int OP){
    lecturaDeMemoria(MV, OP);
    int16_t valor; //cambie de 32 a 16bits
    int i, leer;
    valor = MV->registros[MAR] & 0xFFFF;//cambie Masc_Byte_Menos_Sig por 0xFFFF;
    leer = (MV->registros[MAR] >> 24) & Masc_Byte_Menos_Sig;//igual que CargaMem
    uint32_t dato = 0;
    for (i=0; i < leer; i++){
       // Extraemos de la memoria RAM ensamblando los bytes (Big Endian)
        dato = (dato << 8) | MV->memoria[valor + i]; // MV->memoria[MBR] |= MV->registros[valor + i] << (leer - i);
    } 
    // Guardamos en el registro MBR real
    MV->registros[MBR] = dato;
}

// int main(){
//     MV->registro[DS]=1<<24 | 8;
//     MV->Memoria[59]=1;
//     MV->Memoria[60]=1;
//     MV->Memoria[61]=1;
//     MV->Memoria[62]=1;
//     MV->segmentos[1].base=51;
//     TraigoDeMemoria(&MV, OP1);
//     printf("%d",MV->registro[MBR]);
//     return 0;
// }