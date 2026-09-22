#include <stdint.h>
#include <MaquinaVirtual.h>
#include <mascaras.h>
//Operaciones de memoria(LAR, MAR, MBR)

//Faltan las exepciones sobre si me caigo del data segment 

uint16_t DirecLogica(TipoMV *MV, int32_t offset, int8_t reg){
    int seg;
    uint16_t posini;
    seg = MV -> registros[DS] >> 16;
    posini = MV -> tablaSegmento[seg].base;
    if (reg == DS) 
        return posini + offset;
    else 
        return posini + MV->registros[reg] + offset;
}

void lecturaDeMemoria(TipoMV *MV, int OP){
    int32_t valor;
    int8_t reg;
    uint16_t dirlogica, offset;
    valor = MV -> registros[OP];
    reg = valor & Masc_CodO;
    offset = (valor >> 8) & Masc_Compl;
    dirlogica = DirecLogica(MV, offset, reg);
    MV -> registros[MAR] = 4 << 24 | dirlogica;
}

void CargaAMemoria(TipoMV *MV){
    int32_t valor;
    valor = MV ->registros[MAR] & Masc_Compl;
    MV ->registros[MBR] = MV ->memoria[valor];
}

void LecturaDeMemoria(TipoMV *MV){
    int32_t valor;
    valor = MV->registros[MAR] & Masc_Compl;
    MV->memoria[valor] = MV->registros[MBR];
}