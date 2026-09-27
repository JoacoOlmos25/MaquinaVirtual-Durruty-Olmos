#include <stdint.h>
#include <MaquinaVirtual.h>
#include <mascaras.h>
//Operaciones de memoria(LAR, MAR, MBR)

//Faltan las exepciones sobre si me caigo del data segment 

uint16_t DirecLogica(TipoMV *MV, int32_t corrimiento, int8_t reg){
    int seg;
    uint16_t posini, offset;
    seg = MV -> registros[reg] >> 16 & Masc_Byte_Menos_Sig; //accedo al segmento de la tabla segun el registro
    //seg = MV -> registros[DS] >> 16; (harcodeado)
    posini = MV -> tablaSegmento[seg].base & Masc_Byte_Menos_Sig; //traigo la posicion de memoria inicial
    offset = MV -> registros[reg] & Masc_Byte_Menos_Sig; //accedo al offset
    return posini + offset + corrimiento;
}

void lecturaDeMemoria(TipoMV *MV, int OP){
    int32_t valor;
    int8_t reg;
    uint16_t dirlogica, offset;
    valor = MV -> registros[OP];
    reg = valor & Masc_CodO;
    offset = (valor >> 8) & Masc_Compl;
    dirlogica = DirecLogica(MV, offset, reg); //tengo q ver si me cai del data segment
    MV->registros[LAR] = dirlogica;
    MV -> registros[MAR] = 4 << 24 | dirlogica;
}

void CargaAMemoria(TipoMV *MV, int OP){
    int32_t valor;
    int i, leer;
    lecturaDeMemoria(MV, OP);
    valor = MV->registros[MAR] & Masc_Byte_Menos_Sig;
    leer = (MV->registros[MAR] >> 28) & Masc_Byte_Menos_Sig;
    for (i=0; i<leer; i++){
        MV ->registros[valor + i] = (MV ->memoria[MBR] >> (4*(leer - i))) & Masc_Byte_Menos_Sig;
    }    
}

void TraigoDeMemoria(TipoMV *MV, int OP){
    int32_t valor;
    int i, leer;
    lecturaDeMemoria(MV, OP);
    valor = MV->registros[MAR] & Masc_Byte_Menos_Sig;
    leer = (MV->registros[MAR] >> 28) & Masc_Byte_Menos_Sig;
    for (i=0; i < leer; i++){
        MV->memoria[MBR] |= MV->registros[valor + i] << (leer - i);
    } 
}

int main(){
    MV->registro[DS]=1<<24 | 8;
    MV->Memoria[59]=1;
    MV->Memoria[60]=1;
    MV->Memoria[61]=1;
    MV->Memoria[62]=1;
    MV->segmentos[1].base=51;
    TraigoDeMemoria(&MV, OP1);
    printf("%d",MV->registro[MBR]);
    return 0;
}