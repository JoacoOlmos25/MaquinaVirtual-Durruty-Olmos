#include "operadores.h"
#include "MaquinaVirtual.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "mascaras.h"
#include "OperacionesMem.c"

//Los 28 operaciones del ASSEMBLER
// firmas de los operadores

uint32_t obtenerDato(TipoMV *MV, int OP){
    uint32_t dato;
    uint8_t tipo;
    tipo = MV->registros[OP] >> 24; 
    dato = MV->registros[OP] & 0x00FFFFFF;//seteo el valor en 24 bits
    if (tipo == 1){ //registro
        dato = MV->registros[dato];
    }else
        if (tipo == 2) {//inmediato
            if ((dato >> 15) == 1 ){//negativo
                dato = 0xFFFF0000 | dato;
            }
        }else{
            TraigoDeMemoria(MV,OP);
            dato = MV->registros[MBR];    
        }
    return dato;
}

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
    if (res_con_signo > 2147483647LL || res_con_signo < -2147483648LL) { //LL le decis a C que tome al numero como un long int (de 64bits)
        MV->registros[CC] |= (1 << 28);
    }
}

void guardarDato(TipoMV *MV,int OP ,int32_t dato){ 
   // 1. Extraemos el tipo desplazando 24 bits lógicos a la derecha
    uint8_t tipo = MV->registros[OP] >> 24;
    
    int32_t valor_bruto = MV->registros[OP] & 0x00FFFFFF;

    if (tipo == 1) { 
        // Es un Registro (1 byte)
        MV->registros[valor_bruto] = dato;
    } 
    else if (tipo == 3) { 
        MV->registros[MBR] = dato;
        lecturaDeMemoria(MV, OP);
        CargaAMemoria(MV);
    }
}

void MOV(TipoMV *MV){
    int32_t val_origen = obtenerDato(MV, OP2);
    guardarDato(MV, OP1, val_origen);
    // 3. Evaluamos CC. 
    // Usamos el mismo valor puro casteado a 64 bits para que actualizaCC 
    // modifique N y Z, dejando C y V en 0 como exige la tabla
    int64_t res_con_signo = (int64_t)val_origen;
    uint64_t res_sin_signo = (uint64_t)val_origen & 0xFFFFFFFF; 
    
    actualizaCC(MV, res_con_signo, res_sin_signo);
}

void ADD(TipoMV *MV) {
    int32_t val_destino = obtenerDato(MV, OP1);
    int32_t val_origen = obtenerDato(MV, OP2);
    // Calculamos los resultados en 64 bits para evaluar desbordes
    
    // Resultado con signo tradicional
    int64_t res_con_signo = (int64_t)val_destino + (int64_t)val_origen;
    // Resultado sin signo usando máscaras AND
    uint64_t res_sin_signo = ((uint64_t)val_destino & 0xFFFFFFFF) + ((uint64_t)val_origen & 0xFFFFFFFF);
    
    guardarDato(MV, OP1, (int32_t)res_con_signo);//HAY QUE MODIFICARLO CON RESPECTO A LAS FUNCIONES DE operacionesMem.c
    // Actualizamos las banderas N, Z, C, y V
    actualizaCC(MV, res_con_signo, res_sin_signo);
}

void SUB(TipoMV *MV){
    int32_t val_destino = obtenerDato(MV, OP1);
    int32_t val_origen = obtenerDato(MV, OP2);

    int64_t res_con_signo = (int64_t)val_destino - (int64_t)val_origen;
    uint64_t res_sin_signo = ((uint64_t)val_destino & 0xFFFFFFFF) - ((uint64_t)val_origen & 0xFFFFFFFF);

    guardarDato(MV, OP1, (int32_t)res_con_signo);
    actualizaCC(MV, res_con_signo, res_sin_signo);
}

void MUL(TipoMV *MV){
    int32_t val_destino = obtenerDato(MV, OP1);
    int32_t val_origen = obtenerDato(MV, OP2);

    int64_t res_con_signo = (int64_t)val_destino * (int64_t)val_origen;
    uint64_t res_sin_signo = ((uint64_t)val_destino & 0xFFFFFFFF) * ((uint64_t)val_origen & 0xFFFFFFFF);

    guardarDato(MV, OP1, (int32_t)res_con_signo);
    actualizaCC(MV, res_con_signo, res_sin_signo);
}

void DIV(TipoMV *MV){
    int32_t val_destino = obtenerDato(MV, OP1);
    int32_t val_origen = obtenerDato(MV, OP2);

    if (val_origen == 0) {
        printf("ERROR: Division por cero.\n");
        MV->registros[IP] = 0xFFFFFFFF; 
        return; 
    }

    int64_t res_con_signo = (int64_t)val_destino / (int64_t)val_origen;
    uint64_t res_sin_signo = ((uint64_t)val_destino & 0xFFFFFFFF) / ((uint64_t)val_origen & 0xFFFFFFFF);
    
    guardarDato(MV, OP1, (int32_t)res_con_signo);
     // El DIV guarda obligatoriamente el resto en el registro AC
    MV->registros[AC] = val_destino % val_origen;
    actualizaCC(MV, res_con_signo, res_sin_signo);
}

void CMP(TipoMV *MV){
    //Igual que sub pero sin guardar el dato
    int32_t val_destino = obtenerDato(MV, OP1);
    int32_t val_origen = obtenerDato(MV, OP2);

    int64_t res_con_signo = (int64_t)val_destino - (int64_t)val_origen;
    uint64_t res_sin_signo = ((uint64_t)val_destino & 0xFFFFFFFF) - ((uint64_t)val_origen & 0xFFFFFFFF);
    
    actualizaCC(MV, res_con_signo, res_sin_signo);
}

void AND(TipoMV *MV){
    int32_t val_destino = obtenerDato(MV, OP1);
    int32_t val_origen = obtenerDato(MV, OP2);

    int64_t res_con_signo = (int64_t)val_destino & (int64_t)val_origen;
    uint64_t res_sin_signo = ((uint64_t)val_destino & 0xFFFFFFFF) & ((uint64_t)val_origen & 0xFFFFFFFF);

    guardarDato(MV, OP1, (int32_t)res_con_signo);
    actualizaCC(MV, res_con_signo, res_sin_signo);
}

void OR(TipoMV *MV){
    int32_t val_destino = obtenerDato(MV, OP1);
    int32_t val_origen = obtenerDato(MV, OP2);

    int64_t res_con_signo = (int64_t)val_destino | (int64_t)val_origen;
    uint64_t res_sin_signo = ((uint64_t)val_destino & 0xFFFFFFFF) | ((uint64_t)val_origen & 0xFFFFFFFF);

    guardarDato(MV, OP1, (int32_t)res_con_signo);
    actualizaCC(MV, res_con_signo, res_sin_signo);
}

void XOR(TipoMV *MV){
    int32_t val_destino = obtenerDato(MV, OP1);
    int32_t val_origen = obtenerDato(MV, OP2);

    int64_t res_con_signo = (int64_t)val_destino ^ (int64_t)val_origen;
    uint64_t res_sin_signo = ((uint64_t)val_destino & 0xFFFFFFFF) ^ ((uint64_t)val_origen & 0xFFFFFFFF);

    guardarDato(MV, OP1, (int32_t)res_con_signo);
    actualizaCC(MV, res_con_signo, res_sin_signo);
}

void SWAP(TipoMV *MV){
    int32_t val_destino = obtenerDato(MV, OP1);
    int32_t val_origen = obtenerDato(MV, OP2);

    // Intercambio cruzado hay que modificar estas funciones
    guardarDato(MV,OP1,val_origen);
    guardarDato(MV, OP2, val_destino);

    // Afecta a CC simulando un XOR entre ambos pero no le cambio el valor 
    int64_t res_con_signo = (int64_t)val_destino ^ (int64_t)val_origen;
    uint64_t res_sin_signo = ((uint64_t)val_destino & 0xFFFFFFFF) ^ ((uint64_t)val_origen & 0xFFFFFFFF);
    actualizaCC(MV, res_con_signo, res_sin_signo);
}

void SHL(TipoMV *MV){
    int32_t val_destino = obtenerDato(MV, OP1);
    int32_t val_origen = obtenerDato(MV, OP2);

    int64_t res_con_signo = (int64_t)val_destino << val_origen;
    uint64_t res_sin_signo = ((uint64_t)val_destino & 0xFFFFFFFF) << val_origen;

    guardarDato(MV, OP1, (int32_t)res_con_signo);
    actualizaCC(MV, res_con_signo, res_sin_signo);
}

void SHR(TipoMV *MV){
    uint32_t val_destino = (uint32_t)obtenerDato(MV, OP1);  //seteo asi el compilador hace el corrimiento sin desplazar el signo;
    int32_t val_origen = obtenerDato(MV, OP2);

    int64_t res_con_signo = (int64_t)(val_destino >> val_origen);
    uint64_t res_sin_signo = ((uint64_t)val_destino & 0xFFFFFFFF) >> val_origen;


    guardarDato(MV, OP1, (int32_t)res_con_signo);
    actualizaCC(MV, res_con_signo, res_sin_signo);
}

void SAR(TipoMV *MV){
    // Mantenemos el signo (int32_t) para que C propague el bit negativo
    int32_t val_destino = obtenerDato(MV, OP1); 
    int32_t val_origen = obtenerDato(MV, OP2);

    int64_t res_con_signo = (int64_t)(val_destino >> val_origen);
    uint64_t res_sin_signo = ((uint64_t)val_destino & 0xFFFFFFFF) >> val_origen;

    guardarDato(MV, OP1, (int32_t)res_con_signo); 
    actualizaCC(MV, res_con_signo, res_sin_signo);
}

void LDL(TipoMV *MV){
    int32_t val_destino = obtenerDato(MV, OP1);
    int32_t val_origen = obtenerDato(MV, OP2);

    val_destino &= 0xFFFF0000;

    int32_t mitad_baja_origen = val_origen & 0x0000FFFF;

    int32_t resultado = val_destino | mitad_baja_origen;

    guardarDato(MV, OP1, resultado);
    actualizaCC(MV, (int64_t)resultado, (uint64_t)resultado & 0xFFFFFFFF);
}

void LDH(TipoMV *MV){
    int32_t val_destino = obtenerDato(MV, OP1);
    int32_t val_origen = obtenerDato(MV, OP2);

    val_destino &= 0x0000FFFF;

    int32_t mitad_alta_origen = (val_origen & 0x0000FFFF) << 16;

    int32_t resultado = val_destino | mitad_alta_origen;

    guardarDato(MV, OP1, resultado);
    actualizaCC(MV, (int64_t)resultado, (uint64_t)resultado & 0xFFFFFFFF);
}

void RND(TipoMV *MV){
    int32_t limite = obtenerDato(MV, OP2);

    int32_t resultado = rand() % (limite + 1);

    guardarDato(MV, OP1, resultado);
    actualizaCC(MV, (int64_t)resultado, (uint64_t)resultado & 0xFFFFFFFF);
}

void SYS(TipoMV *MV){

}

void JMP(TipoMV *MV){
    MV->registros[IP] = obtenerDato(MV, OP1);
}

void JP(TipoMV *MV){
    // Aislamos N (bit 31) y Z (bit 30)
    int n = (MV->registros[CC] >> 31) & 1;
    int z = (MV->registros[CC] >> 30) & 1;

    if (n == 0 && z == 0) {
        MV->registros[IP] = obtenerDato(MV, OP1);
    }
}

void JN(TipoMV *MV){
    int n = (MV->registros[CC] >> 31) & 1;
    if (n == 1) {
        MV->registros[IP] = obtenerDato(MV, OP1);
    }
}

void JZ(TipoMV *MV){
    int z = (MV->registros[CC] >> 30) & 1;
    // La condición exige que Z sea igual a 1 (resultado anterior == 0)
    if (z == 1) {
        MV->registros[IP] = obtenerDato(MV, OP1);
    }
}

void JC(TipoMV *MV){
    int c = (MV->registros[CC] >> 29) & 1;
    
    if (c == 1) {
        MV->registros[IP] = obtenerDato(MV, OP1);
    }
}

void JV(TipoMV *MV){
     int v = (MV->registros[CC] >> 28) & 1;
    
    if (v == 1) {
        MV->registros[IP] = obtenerDato(MV, OP1);
    }
}

void JNP(TipoMV *MV){
    int n = (MV->registros[CC] >> 31) & 1;
    int z = (MV->registros[CC] >> 30) & 1;

    if (n == 1 || z == 1) {
        MV->registros[IP] = obtenerDato(MV, OP1);
    }
}

void JNN(TipoMV *MV){
    int n = (MV->registros[CC] >> 31) & 1;
    if (n == 0) {
        MV->registros[IP] = obtenerDato(MV, OP1);
    }
}

void JNZ(TipoMV *MV){
    int z = (MV->registros[CC] >> 30) & 1;
    if (z == 0) {
        MV->registros[IP] = obtenerDato(MV, OP1);
    }
}

void NOT(TipoMV *MV){

    int32_t val = obtenerDato(MV, OP1);

    int32_t res = ~val;

    guardarDato(MV, OP1, res);

    actualizaCC(MV, (int64_t)res, (uint64_t)res & 0xFFFFFFFF);
}

void STOP(TipoMV *MV){
    MV->registros[IP]=-1;
}

