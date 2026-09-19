#include "MaquinaVirtual.h"
#include <stdint.h>

//Operaciones con 2 registros

void MOV(TipoMV *MV);
void ADD(TipoMV *MV);
void SUB(TipoMV *MV);
void MUL(TipoMV *MV);
void DIV(TipoMV *MV);
void CMP(TipoMV *MV);
void AND(TipoMV *MV);
void OR(TipoMV *MV);
void XOR(TipoMV *MV);
void SWAP(TipoMV *MV);
void SHL(TipoMV *MV);
void SHR(TipoMV *MV);
void SAR(TipoMV *MV);
void LDL(TipoMV *MV);
void LDH(TipoMV *MV);
void RND(TipoMV *MV);

//Operaciones con 1 registros

void SYS(TipoMV *MV);
void JMP(TipoMV *MV);
void JP(TipoMV *MV);
void JN(TipoMV *MV);
void JZ(TipoMV *MV);
void JC(TipoMV *MV);
void JV(TipoMV *MV);
void JNP(TipoMV *MV);
void JNN(TipoMV *MV);
void JNZ(TipoMV *MV);
void NOT(TipoMV *MV);

//Operaciones sin registros

void STOP(TipoMV *MV);