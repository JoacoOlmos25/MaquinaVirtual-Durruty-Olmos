// #include "operadores.h"
// #include "MaquinaVirtual.h"
// #include <stdint.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include "mascaras.h"
// #include "OperacionesMem.h"

// //Los 28 operaciones del ASSEMBLER
// // firmas de los operadores

// uint32_t obtenerDato(TipoMV *MV, int OP){
//     uint32_t dato;
//     uint8_t tipo;
//     tipo = MV->registros[OP] >> 24; 
//     dato = MV->registros[OP] & 0x00FFFFFF;//seteo el valor en 24 bits
//     if (tipo == 1){ //registro
//         dato = MV->registros[dato];
//     }else
//        if (tipo == 2) { // Inmediato
//         uint16_t inmediato_16 = dato & 0xFFFF;
//         if (inmediato_16 & 0x8000) {
//             // Es negativo. Extendemos el signo encendiendo los 16 bits superiores.
//             dato = 0xFFFF0000 | inmediato_16;
//         } else {
//             dato = inmediato_16;
//         }
//         }else{
//             TraigoDeMemoria(MV,OP);
//             dato = MV->registros[MBR];    
//         }
//     return dato;
// }

// void actualizaCC(TipoMV *MV, int64_t res_con_signo, uint64_t res_sin_signo) {
//     MV->registros[CC] &= 0x0FFFFFFF;
//     int32_t res32 = (int32_t)res_con_signo;

//     if (res32 < 0) {
//         MV->registros[CC] |= (1 << 31);
//     }
//     if (res32 == 0) {
//         MV->registros[CC] |= (1 << 30);
//     }
//     if (res_sin_signo > 0xFFFFFFFF) {
//         MV->registros[CC] |= (1 << 29);
//     }
//     if (res_con_signo > 2147483647LL || res_con_signo < -2147483648LL) { 
//         MV->registros[CC] |= (1 << 28);
//     }
// }

// void guardarDato(TipoMV *MV,int OP ,int32_t dato){ 
//    // Extraemos el tipo desplazando 24 bits lógicos a la derecha
//     uint8_t tipo = MV->registros[OP] >> 24;
    
//     int32_t valor_bruto = MV->registros[OP] & 0x00FFFFFF;

//     if (tipo == 1) { 
//         // Es un Registro (1 byte)
//         MV->registros[valor_bruto] = dato;
//     } 
//     else if (tipo == 3) { 
//         MV->registros[MBR] = dato;
//         lecturaDeMemoria(MV, OP);
//         CargaAMemoria(MV,OP);
//     }
// }

// void MOV(TipoMV *MV){
//     int32_t val_origen = obtenerDato(MV, OP2);
//     guardarDato(MV, OP1, val_origen);

//     int64_t res_con_signo = (int64_t)val_origen;
//     uint64_t res_sin_signo = (uint64_t)val_origen & 0xFFFFFFFF; 
    
//     actualizaCC(MV, res_con_signo, res_sin_signo);
// }

// void ADD(TipoMV *MV) {
//     int32_t val_destino = obtenerDato(MV, OP1);
//     int32_t val_origen = obtenerDato(MV, OP2);
//     // Calculamos los resultados en 64 bits para evaluar desbordes
    
//     // Resultado con signo tradicional
//     int64_t res_con_signo = (int64_t)val_destino + (int64_t)val_origen;
//     // Resultado sin signo usando máscaras AND
//     uint64_t res_sin_signo = ((uint64_t)val_destino & 0xFFFFFFFF) + ((uint64_t)val_origen & 0xFFFFFFFF);
    
//     guardarDato(MV, OP1, (int32_t)res_con_signo);//HAY QUE MODIFICARLO CON RESPECTO A LAS FUNCIONES DE operacionesMem.c
//     // Actualizamos las banderas N, Z, C, y V
//     actualizaCC(MV, res_con_signo, res_sin_signo);
// }

// void SUB(TipoMV *MV){
//     int32_t val_destino = obtenerDato(MV, OP1);
//     int32_t val_origen = obtenerDato(MV, OP2);
//     if (val_origen >0){
//         val_origen= ~val_origen + 1;    
//     }
//     int64_t res_con_signo = (int64_t)val_destino + (int64_t)val_origen;
//     uint64_t res_sin_signo = ((uint64_t)val_destino & 0xFFFFFFFF) + ((uint64_t)val_origen & 0xFFFFFFFF);

//     guardarDato(MV, OP1, (int32_t)res_con_signo);
//     actualizaCC(MV, res_con_signo, res_sin_signo);
//     printf("\n0x%X \n",MV->registros[CC]);
// }

// void MUL(TipoMV *MV){
//     int32_t val_destino = obtenerDato(MV, OP1);
//     int32_t val_origen = obtenerDato(MV, OP2);

//     int64_t res_con_signo = (int64_t)val_destino * (int64_t)val_origen;
//     uint64_t res_sin_signo = ((uint64_t)val_destino & 0xFFFFFFFF) * ((uint64_t)val_origen & 0xFFFFFFFF);

//     guardarDato(MV, OP1, (int32_t)res_con_signo);
//     actualizaCC(MV, res_con_signo, res_sin_signo);
// }

// void DIV(TipoMV *MV){
//     int32_t val_destino = obtenerDato(MV, OP1);
//     int32_t val_origen = obtenerDato(MV, OP2);

//     if (val_origen == 0) {
//         printf("ERROR: Division por cero.\n");
//         MV->registros[IP] = 0xFFFFFFFF; 
//         return; 
//     }

//     int64_t res_con_signo = (int64_t)val_destino / (int64_t)val_origen;
//     uint64_t res_sin_signo = ((uint64_t)val_destino & 0xFFFFFFFF) / ((uint64_t)val_origen & 0xFFFFFFFF);
    
//     guardarDato(MV, OP1, (int32_t)res_con_signo);
//      // El DIV guarda obligatoriamente el resto en el registro AC
//     MV->registros[AC] = val_destino % val_origen;
//     actualizaCC(MV, res_con_signo, res_sin_signo);
// }

// void CMP(TipoMV *MV){
//     //Igual que sub pero sin guardar el dato
//     int32_t val_destino = obtenerDato(MV, OP1);
//     int32_t val_origen = obtenerDato(MV, OP2);

//     int64_t res_con_signo = (int64_t)val_destino - (int64_t)val_origen;
//     uint64_t res_sin_signo = ((uint64_t)val_destino & 0xFFFFFFFF) - ((uint64_t)val_origen & 0xFFFFFFFF);
    
//     actualizaCC(MV, res_con_signo, res_sin_signo);
// }

// void AND(TipoMV *MV){
//     int32_t val_destino = obtenerDato(MV, OP1);
//     int32_t val_origen = obtenerDato(MV, OP2);

//     int64_t res_con_signo = (int64_t)val_destino & (int64_t)val_origen;
//     uint64_t res_sin_signo = ((uint64_t)val_destino & 0xFFFFFFFF) & ((uint64_t)val_origen & 0xFFFFFFFF);

//     guardarDato(MV, OP1, (int32_t)res_con_signo);
//     actualizaCC(MV, res_con_signo, res_sin_signo);
// }

// void OR(TipoMV *MV){
//     int32_t val_destino = obtenerDato(MV, OP1);
//     int32_t val_origen = obtenerDato(MV, OP2);

//     int64_t res_con_signo = (int64_t)val_destino | (int64_t)val_origen;
//     uint64_t res_sin_signo = ((uint64_t)val_destino & 0xFFFFFFFF) | ((uint64_t)val_origen & 0xFFFFFFFF);

//     guardarDato(MV, OP1, (int32_t)res_con_signo);
//     actualizaCC(MV, res_con_signo, res_sin_signo);
// }

// void XOR(TipoMV *MV){
//     int32_t val_destino = obtenerDato(MV, OP1);
//     int32_t val_origen = obtenerDato(MV, OP2);

//     int64_t res_con_signo = (int64_t)val_destino ^ (int64_t)val_origen;
//     uint64_t res_sin_signo = ((uint64_t)val_destino & 0xFFFFFFFF) ^ ((uint64_t)val_origen & 0xFFFFFFFF);

//     guardarDato(MV, OP1, (int32_t)res_con_signo);
//     actualizaCC(MV, res_con_signo, res_sin_signo);
// }

// void SWAP(TipoMV *MV){
//     int32_t val_destino = obtenerDato(MV, OP1);
//     int32_t val_origen = obtenerDato(MV, OP2);

//     // Intercambio cruzado hay que modificar estas funciones
//     guardarDato(MV,OP1,val_origen);
//     guardarDato(MV, OP2, val_destino);

//     // Afecta a CC simulando un XOR entre ambos pero no le cambio el valor 
//     int64_t res_con_signo = (int64_t)val_destino ^ (int64_t)val_origen;
//     uint64_t res_sin_signo = ((uint64_t)val_destino & 0xFFFFFFFF) ^ ((uint64_t)val_origen & 0xFFFFFFFF);
//     actualizaCC(MV, res_con_signo, res_sin_signo);
// }

// void SHL(TipoMV *MV){
//     // Casteamos a uint32_t para que C desplace los bits sin importar el signo
//     uint32_t val_destino = (uint32_t)obtenerDato(MV, OP1);
//     int32_t val_origen = obtenerDato(MV, OP2);

//     // El desplazamiento en 64 bits sin signo preserva el acarreo de forma segura
//     uint64_t res_64 = (uint64_t)val_destino << val_origen;
//     int32_t res_32 = (int32_t)(res_64 & 0xFFFFFFFF);

//     guardarDato(MV, OP1, res_32);
//     actualizaCC(MV, (int64_t)res_32, res_64);
// }

// void SHR(TipoMV *MV){
//     uint32_t val_destino = (uint32_t)obtenerDato(MV, OP1);  //seteo asi el compilador hace el corrimiento sin desplazar el signo;
//     int32_t val_origen = obtenerDato(MV, OP2);

//     int64_t res_con_signo = (int64_t)(val_destino >> val_origen);
//     uint64_t res_sin_signo = ((uint64_t)val_destino & 0xFFFFFFFF) >> val_origen;


//     guardarDato(MV, OP1, (int32_t)res_con_signo);
//     actualizaCC(MV, res_con_signo, res_sin_signo);
// }

// void SAR(TipoMV *MV){
//     // Mantenemos el signo (int32_t) para que C propague el bit negativo
//     int32_t val_destino = obtenerDato(MV, OP1); 
//     int32_t val_origen = obtenerDato(MV, OP2);

//     int64_t res_con_signo = (int64_t)(val_destino >> val_origen);
//     uint64_t res_sin_signo = ((uint64_t)val_destino & 0xFFFFFFFF) >> val_origen;

//     guardarDato(MV, OP1, (int32_t)res_con_signo); 
//     actualizaCC(MV, res_con_signo, res_sin_signo);
// }

// void LDL(TipoMV *MV){
//     int32_t val_destino = obtenerDato(MV, OP1);
//     int32_t val_origen = obtenerDato(MV, OP2);

//     val_destino &= 0xFFFF0000;

//     int32_t mitad_baja_origen = val_origen & 0x0000FFFF;

//     int32_t resultado = val_destino | mitad_baja_origen;

//     guardarDato(MV, OP1, resultado);
//     actualizaCC(MV, (int64_t)resultado, (uint64_t)resultado & 0xFFFFFFFF);
// }

// void LDH(TipoMV *MV){
//     int32_t val_destino = obtenerDato(MV, OP1);
//     int32_t val_origen = obtenerDato(MV, OP2);

//     val_destino &= 0x0000FFFF;

//     int32_t mitad_alta_origen = (val_origen & 0x0000FFFF) << 16;

//     int32_t resultado = val_destino | mitad_alta_origen;

//     guardarDato(MV, OP1, resultado);
//     actualizaCC(MV, (int64_t)resultado, (uint64_t)resultado & 0xFFFFFFFF);
// }

// void RND(TipoMV *MV){
//     int32_t limite = obtenerDato(MV, OP2);

//     int32_t resultado = rand() % (limite + 1);

//     guardarDato(MV, OP1, resultado);
//     actualizaCC(MV, (int64_t)resultado, (uint64_t)resultado & 0xFFFFFFFF);
// }

// //Funciones auxiliares para modularizar SYS
// //##############################################################################
// void imprimir_segun_formato(uint32_t dato, uint32_t formato, uint16_t tamano) {
//     if (formato & 0x01) printf("%d ", dato); // Decimal
    
//     if (formato & 0x02) { //ASCII
//         if (dato >= 32 && dato <= 126) printf("%c ", dato);
//         else printf(". "); // Si no es imprimible, escribe un punto
//     }
    
//     if (formato & 0x04) printf("0o%o ", dato); // Octal
    
//     if (formato & 0x08) printf("0x%X ", dato); // Hexadecimal
    
//     if (formato & 0x10) { // Binario
//         printf("0b");
//         for (int b = (tamano * 8) - 1; b >= 0; b--) {
//             printf("%d", (dato >> b) & 1);
//         }
//         printf(" ");
//     }
//     printf("\n");
// }

// void sys_write(TipoMV *MV, uint16_t cantidad, uint16_t tamano, uint32_t dir_fisica, uint32_t formato) {
//    for (int i = 0; i < cantidad; i++) {
//         printf("[%04X]: ", dir_fisica);
        
//         // Extracción Big Endian estándar
//         uint32_t dato = 0;
//         for (int b = 0; b < tamano; b++) {
//             dato = (dato << 8) | MV->memoria[dir_fisica + b]; 
//         }
//         imprimir_segun_formato(dato, formato, tamano);
        
//         dir_fisica += tamano; 
//     }
// }

// void sys_read(TipoMV *MV, uint16_t cantidad, uint16_t tamano, uint32_t dir_fisica, uint32_t formato) {
//     for (int i = 0; i < cantidad; i++) {
//         printf("[%04X]: ", dir_fisica); 

//         uint32_t dato_ingresado = 0;
        
//         if (formato & 0x01) scanf("%d", &dato_ingresado);
//         else if (formato & 0x02) { 
//             char c;
//             scanf(" %c", &c);
//             dato_ingresado = c;
//         } 
//         else if (formato & 0x04) scanf("%o", &dato_ingresado);
//         else if (formato & 0x08) scanf("%x", &dato_ingresado);
//         else if (formato & 0x10) { 
//             char strbinario[33];
//             scanf("%32s", strbinario);
//             char *ptr = strbinario;
//             if (strbinario[0] == '0' && (strbinario[1] == 'b' || strbinario[1] == 'B')) ptr += 2; 
//             dato_ingresado = (uint32_t)strtol(ptr, NULL, 2);
//         }

//         // Inyección Big Endian estándar (desde el byte más significativo)
//         for (int b = tamano - 1; b >= 0; b--) {
//             MV->memoria[dir_fisica + b] = dato_ingresado & 0xFF;
//             dato_ingresado >>= 8;
//         }

//         dir_fisica += tamano;
//     }
// }
// //##############################################################################
// void SYS(TipoMV *MV){
//     int32_t tipo_llamada = obtenerDato(MV, OP1);

//     // ECX indica la cantidad en los 2 bytes bajos y el tamaño en los 2 bytes altos
//     uint16_t cantidad = MV->registros[ECX] & 0xFFFF;
//     uint16_t tamano = (MV->registros[ECX] >> 16) & 0xFFFF;

//     // EDX indica la posición de memoria inicial
//     uint16_t dir_fisica = DirecLogica(MV, 0, EDX);

//     if (tipo_llamada == 1) { 
//         sys_read(MV, cantidad, tamano, dir_fisica, MV->registros[EAX]);
//     } 
//     else if (tipo_llamada == 2) { 
//         sys_write(MV, cantidad, tamano, dir_fisica, MV->registros[EAX]);
//     }
//     //  MV->registros[EAX] = FORMATO
// }

// void JMP(TipoMV *MV){
//     MV->registros[IP] = obtenerDato(MV, OP1);
// }

// void JP(TipoMV *MV){
//     // Aislamos N (bit 31) y Z (bit 30)
//     int n = (MV->registros[CC] >> 31) & 1;
//     int z = (MV->registros[CC] >> 30) & 1;

//     if (n == 0 && z == 0) {
//         MV->registros[IP] = obtenerDato(MV, OP1);
//     }
// }

// void JN(TipoMV *MV){
//     int n = (MV->registros[CC] >> 31) & 1;
//     if (n == 1) {
//         MV->registros[IP] = obtenerDato(MV, OP1);
//     }
// }

// void JZ(TipoMV *MV){
//     int z = (MV->registros[CC] >> 30) & 1;
//     // La condición exige que Z sea igual a 1 (resultado anterior == 0)
//     if (z == 1) {
//         MV->registros[IP] = obtenerDato(MV, OP1);
//     }
// }

// void JC(TipoMV *MV){
//     int c = (MV->registros[CC] >> 29) & 1;
    
//     if (c == 1) {
//         MV->registros[IP] = obtenerDato(MV, OP1);
//     }
// }

// void JV(TipoMV *MV){
//      int v = (MV->registros[CC] >> 28) & 1;
    
//     if (v == 1) {
//         MV->registros[IP] = obtenerDato(MV, OP1);
//     }
// }

// void JNP(TipoMV *MV){
//     int n = (MV->registros[CC] >> 31) & 1;
//     int z = (MV->registros[CC] >> 30) & 1;

//     if (n == 1 || z == 1) {
//         MV->registros[IP] = obtenerDato(MV, OP1);
//     }
// }

// void JNN(TipoMV *MV){
//     int n = (MV->registros[CC] >> 31) & 1;
//     if (n == 0) {
//         MV->registros[IP] = obtenerDato(MV, OP1);
//     }
// }

// void JNZ(TipoMV *MV){
//     int z = (MV->registros[CC] >> 30) & 1;
//     if (z == 0) {
//         MV->registros[IP] = obtenerDato(MV, OP1);
//     }
// }

// void NOT(TipoMV *MV){

//     int32_t val = obtenerDato(MV, OP1);

//     int32_t res = ~val;

//     guardarDato(MV, OP1, res);

//     actualizaCC(MV, (int64_t)res, (uint64_t)res & 0xFFFFFFFF);
// }

// void STOP(TipoMV *MV){
//     MV->registros[IP]=-1;
// }

// // ############################################################################################################

#include "operadores.h"
#include "MaquinaVirtual.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mascaras.h"
#include "OperacionesMem.h"

uint32_t obtenerDato(TipoMV *MV, int OP) {
    uint32_t dato;
    uint8_t tipo;
    tipo = (uint32_t)MV->registros[OP] >> 24; 
    dato = MV->registros[OP] & 0x00FFFFFF; // Seteo el valor en 24 bits

    if (tipo == 1) { // Registro (con soporte de sector: 00=4B, 01=AL, 10=AH, 11=AX)
        uint8_t reg = dato & 0x1F;
        uint8_t sec = (dato >> 5) & 0x03;
        int32_t val_reg = MV->registros[reg];

        if (sec == 0) {
            dato = (uint32_t)val_reg;
        } else if (sec == 1) { // AL (1 byte bajo con extensión de signo)
            dato = (uint32_t)(int32_t)(int8_t)(val_reg & 0xFF);
        } else if (sec == 2) { // AH (segundo byte con extensión de signo)
            dato = (uint32_t)(int32_t)(int8_t)((val_reg >> 8) & 0xFF);
        } else {               // AX (2 bytes bajos con extensión de signo)
            dato = (uint32_t)(int32_t)(int16_t)(val_reg & 0xFFFF);
        }
    } 
    else if (tipo == 2) { // Inmediato
        uint16_t inmediato_16 = dato & 0xFFFF;
        if (inmediato_16 & 0x8000) {
            dato = 0xFFFF0000 | inmediato_16;
        } else {
            dato = inmediato_16;
        }
    } 
    else if (tipo == 3) { // Memoria
        TraigoDeMemoria(MV, OP);
        dato = (uint32_t)MV->registros[MBR];    
    }
    return dato;
}

void actualizaCC(TipoMV *MV, int64_t res_con_signo, uint64_t res_sin_signo) {
    MV->registros[CC] &= 0x0FFFFFFF;
    int32_t res32 = (int32_t)res_con_signo;

    if (res32 < 0) {
        MV->registros[CC] |= (1 << 31);
    }
    if (res32 == 0) {
        MV->registros[CC] |= (1 << 30);
    }
    if (res_sin_signo > 0xFFFFFFFFULL) {
        MV->registros[CC] |= (1 << 29);
    }
    if (res_con_signo > 2147483647LL || res_con_signo < -2147483648LL) { 
        MV->registros[CC] |= (1 << 28);
    }
}

void guardarDato(TipoMV *MV, int OP, int32_t dato) { 
    uint8_t tipo = (uint32_t)MV->registros[OP] >> 24;
    int32_t valor_bruto = MV->registros[OP] & 0x00FFFFFF;

    if (tipo == 1) { 
        uint8_t reg = valor_bruto & 0x1F;
        uint8_t sec = (valor_bruto >> 5) & 0x03;

        if (sec == 0) {
            MV->registros[reg] = dato;
        } else if (sec == 1) { // AL
            MV->registros[reg] = (MV->registros[reg] & 0xFFFFFF00) | (dato & 0xFF);
        } else if (sec == 2) { // AH
            MV->registros[reg] = (MV->registros[reg] & 0xFFFF00FF) | ((dato & 0xFF) << 8);
        } else {               // AX
            MV->registros[reg] = (MV->registros[reg] & 0xFFFF0000) | (dato & 0xFFFF);
        }
    } 
    else if (tipo == 3) { 
        MV->registros[MBR] = dato;
        CargaAMemoria(MV, OP);
    }
}

void MOV(TipoMV *MV) {
    int32_t val_origen = obtenerDato(MV, OP2);
    if (MV->registros[IP] == -1) return;
    guardarDato(MV, OP1, val_origen);

    int64_t res_con_signo = (int64_t)val_origen;
    uint64_t res_sin_signo = (uint64_t)val_origen & 0xFFFFFFFF; 
    
    actualizaCC(MV, res_con_signo, res_sin_signo);
}

void ADD(TipoMV *MV) {
    int32_t val_destino = obtenerDato(MV, OP1);
    int32_t val_origen = obtenerDato(MV, OP2);
    if (MV->registros[IP] == -1) return;
    
    int64_t res_con_signo = (int64_t)val_destino + (int64_t)val_origen;
    uint64_t res_sin_signo = ((uint64_t)val_destino & 0xFFFFFFFF) + ((uint64_t)val_origen & 0xFFFFFFFF);
    
    guardarDato(MV, OP1, (int32_t)res_con_signo);
    actualizaCC(MV, res_con_signo, res_sin_signo);
}

void SUB(TipoMV *MV) {
    int32_t val_destino = obtenerDato(MV, OP1);
    int32_t val_origen = obtenerDato(MV, OP2);
    if (MV->registros[IP] == -1) return;

    if (val_origen > 0) {
        val_origen = ~val_origen + 1;    
    }
    int64_t res_con_signo = (int64_t)val_destino + (int64_t)val_origen;
    uint64_t res_sin_signo = ((uint64_t)val_destino & 0xFFFFFFFF) + ((uint64_t)val_origen & 0xFFFFFFFF);

    guardarDato(MV, OP1, (int32_t)res_con_signo);
    actualizaCC(MV, res_con_signo, res_sin_signo);
}

void MUL(TipoMV *MV) {
    int32_t val_destino = obtenerDato(MV, OP1);
    int32_t val_origen = obtenerDato(MV, OP2);
    if (MV->registros[IP] == -1) return;

    int64_t res_con_signo = (int64_t)val_destino * (int64_t)val_origen;
    uint64_t res_sin_signo = ((uint64_t)val_destino & 0xFFFFFFFF) * ((uint64_t)val_origen & 0xFFFFFFFF);

    guardarDato(MV, OP1, (int32_t)res_con_signo);
    actualizaCC(MV, res_con_signo, res_sin_signo);
}

void DIV(TipoMV *MV) {
    int32_t val_destino = obtenerDato(MV, OP1);
    int32_t val_origen = obtenerDato(MV, OP2);
    if (MV->registros[IP] == -1) return;

    if (val_origen == 0) {
        printf("ERROR: Division por cero.\n");
        MV->registros[IP] = -1; 
        return; 
    }

    int64_t res_con_signo = (int64_t)val_destino / (int64_t)val_origen;
    uint64_t res_sin_signo = ((uint64_t)val_destino & 0xFFFFFFFF) / ((uint64_t)val_origen & 0xFFFFFFFF);
    
    guardarDato(MV, OP1, (int32_t)res_con_signo);
    MV->registros[AC] = val_destino % val_origen;
    actualizaCC(MV, res_con_signo, res_sin_signo);
}

void CMP(TipoMV *MV) {
    int32_t val_destino = obtenerDato(MV, OP1);
    int32_t val_origen = obtenerDato(MV, OP2);
    if (MV->registros[IP] == -1) return;

    if (val_origen > 0) {
        val_origen = ~val_origen + 1;
    }
    int64_t res_con_signo = (int64_t)val_destino + (int64_t)val_origen;
    uint64_t res_sin_signo = ((uint64_t)val_destino & 0xFFFFFFFF) + ((uint64_t)val_origen & 0xFFFFFFFF);
    
    actualizaCC(MV, res_con_signo, res_sin_signo);
}

void AND(TipoMV *MV) {
    int32_t val_destino = obtenerDato(MV, OP1);
    int32_t val_origen = obtenerDato(MV, OP2);
    if (MV->registros[IP] == -1) return;

    int64_t res_con_signo = (int64_t)val_destino & (int64_t)val_origen;
    uint64_t res_sin_signo = ((uint64_t)val_destino & 0xFFFFFFFF) & ((uint64_t)val_origen & 0xFFFFFFFF);

    guardarDato(MV, OP1, (int32_t)res_con_signo);
    actualizaCC(MV, res_con_signo, res_sin_signo);
}

void OR(TipoMV *MV) {
    int32_t val_destino = obtenerDato(MV, OP1);
    int32_t val_origen = obtenerDato(MV, OP2);
    if (MV->registros[IP] == -1) return;

    int64_t res_con_signo = (int64_t)val_destino | (int64_t)val_origen;
    uint64_t res_sin_signo = ((uint64_t)val_destino & 0xFFFFFFFF) | ((uint64_t)val_origen & 0xFFFFFFFF);

    guardarDato(MV, OP1, (int32_t)res_con_signo);
    actualizaCC(MV, res_con_signo, res_sin_signo);
}

void XOR(TipoMV *MV) {
    int32_t val_destino = obtenerDato(MV, OP1);
    int32_t val_origen = obtenerDato(MV, OP2);
    if (MV->registros[IP] == -1) return;

    int64_t res_con_signo = (int64_t)val_destino ^ (int64_t)val_origen;
    uint64_t res_sin_signo = ((uint64_t)val_destino & 0xFFFFFFFF) ^ ((uint64_t)val_origen & 0xFFFFFFFF);

    guardarDato(MV, OP1, (int32_t)res_con_signo);
    actualizaCC(MV, res_con_signo, res_sin_signo);
}

void SWAP(TipoMV *MV) {
    int32_t val_destino = obtenerDato(MV, OP1);
    int32_t val_origen = obtenerDato(MV, OP2);
    if (MV->registros[IP] == -1) return;

    guardarDato(MV, OP1, val_origen);
    guardarDato(MV, OP2, val_destino);

    int64_t res_con_signo = (int64_t)val_destino ^ (int64_t)val_origen;
    uint64_t res_sin_signo = ((uint64_t)val_destino & 0xFFFFFFFF) ^ ((uint64_t)val_origen & 0xFFFFFFFF);
    actualizaCC(MV, res_con_signo, res_sin_signo);
}

void SHL(TipoMV *MV) {
    uint32_t val_destino = (uint32_t)obtenerDato(MV, OP1);
    int32_t val_origen = obtenerDato(MV, OP2);
    if (MV->registros[IP] == -1) return;

    uint64_t res_64 = (uint64_t)val_destino << val_origen;
    int32_t res_32 = (int32_t)(res_64 & 0xFFFFFFFF);

    guardarDato(MV, OP1, res_32);
    actualizaCC(MV, (int64_t)res_32, res_64);
}

void SHR(TipoMV *MV) {
    uint32_t val_destino = (uint32_t)obtenerDato(MV, OP1);
    int32_t val_origen = obtenerDato(MV, OP2);
    if (MV->registros[IP] == -1) return;

    int64_t res_con_signo = (int64_t)(val_destino >> val_origen);
    uint64_t res_sin_signo = ((uint64_t)val_destino & 0xFFFFFFFF) >> val_origen;

    guardarDato(MV, OP1, (int32_t)res_con_signo);
    actualizaCC(MV, res_con_signo, res_sin_signo);
}

void SAR(TipoMV *MV) {
    int32_t val_destino = obtenerDato(MV, OP1); 
    int32_t val_origen = obtenerDato(MV, OP2);
    if (MV->registros[IP] == -1) return;

    int64_t res_con_signo = (int64_t)(val_destino >> val_origen);
    uint64_t res_sin_signo = ((uint64_t)val_destino & 0xFFFFFFFF) >> val_origen;

    guardarDato(MV, OP1, (int32_t)res_con_signo); 
    actualizaCC(MV, res_con_signo, res_sin_signo);
}

void LDL(TipoMV *MV) {
    int32_t val_destino = obtenerDato(MV, OP1);
    int32_t val_origen = obtenerDato(MV, OP2);
    if (MV->registros[IP] == -1) return;

    val_destino &= 0xFFFF0000;
    int32_t mitad_baja_origen = val_origen & 0x0000FFFF;
    int32_t resultado = val_destino | mitad_baja_origen;

    guardarDato(MV, OP1, resultado);
    actualizaCC(MV, (int64_t)resultado, (uint64_t)resultado & 0xFFFFFFFF);
}

void LDH(TipoMV *MV) {
    int32_t val_destino = obtenerDato(MV, OP1);
    int32_t val_origen = obtenerDato(MV, OP2);
    if (MV->registros[IP] == -1) return;

    val_destino &= 0x0000FFFF;
    int32_t mitad_alta_origen = (val_origen & 0x0000FFFF) << 16;
    int32_t resultado = val_destino | mitad_alta_origen;

    guardarDato(MV, OP1, resultado);
    actualizaCC(MV, (int64_t)resultado, (uint64_t)resultado & 0xFFFFFFFF);
}

void RND(TipoMV *MV) {
    int32_t limite = obtenerDato(MV, OP2);
    if (MV->registros[IP] == -1) return;

    int32_t resultado = (limite > 0) ? (rand() % (limite + 1)) : 0;

    guardarDato(MV, OP1, resultado);
    actualizaCC(MV, (int64_t)resultado, (uint64_t)resultado & 0xFFFFFFFF);
}

// ============================================================================
// INSTRUCCIONES DE PILA Y SUBRUTINAS (PUSH, POP, CALL, RET)
// ============================================================================

static int apilar32(TipoMV *MV, uint32_t valor) {
    if (MV->registros[SS] == -1) {
        printf("\n--- ERROR: STACK OVERFLOW (No existe Stack Segment) ---\n");
        MV->registros[IP] = -1;
        return 0;
    }

    // 1. Decrementar el valor de SP en 4
    MV->registros[SP] -= 4;

    // 2. Si el valor de SP es menor al de SS -> STACK OVERFLOW
    uint16_t seg_sp = (MV->registros[SP] >> 16) & 0xFFFF;
    uint16_t seg_ss = (MV->registros[SS] >> 16) & 0xFFFF;
    int16_t offset_sp = (int16_t)(MV->registros[SP] & 0xFFFF);

    if (seg_sp != seg_ss || offset_sp < 0 ||
        (offset_sp + 4) > MV->tablaSegmento[seg_ss].tamano) {
        printf("\n--- ERROR: STACK OVERFLOW ---\n");
        MV->registros[IP] = -1;
        return 0;
    }

    // 3. Almacenar en el tope de la pila 4 bytes en Big Endian
    uint16_t dir_fisica = MV->tablaSegmento[seg_ss].base + (uint16_t)offset_sp;
    MV->registros[LAR] = MV->registros[SP];
    MV->registros[MAR] = (4 << 24) | dir_fisica;
    MV->registros[MBR] = (int32_t)valor;

    MV->memoria[dir_fisica]     = (valor >> 24) & 0xFF;
    MV->memoria[dir_fisica + 1] = (valor >> 16) & 0xFF;
    MV->memoria[dir_fisica + 2] = (valor >> 8)  & 0xFF;
    MV->memoria[dir_fisica + 3] = valor & 0xFF;
    return 1;
}

static int desapilar32(TipoMV *MV, uint32_t *destino) {
    if (MV->registros[SS] == -1) {
        printf("\n--- ERROR: STACK UNDERFLOW (No existe Stack Segment) ---\n");
        MV->registros[IP] = -1;
        return 0;
    }

    uint16_t seg_sp = (MV->registros[SP] >> 16) & 0xFFFF;
    uint16_t seg_ss = (MV->registros[SS] >> 16) & 0xFFFF;
    int16_t offset_sp = (int16_t)(MV->registros[SP] & 0xFFFF);
    uint16_t tam_ss = MV->tablaSegmento[seg_ss].tamano;

    // 1. Si no existen 4 bytes en el tope para extraer -> STACK UNDERFLOW
    if (seg_sp != seg_ss || offset_sp < 0 || (offset_sp + 4) > tam_ss) {
        printf("\n--- ERROR: STACK UNDERFLOW ---\n");
        MV->registros[IP] = -1;
        return 0;
    }

    // 2. Extraer 4 bytes desde el tope de la pila
    uint16_t dir_fisica = MV->tablaSegmento[seg_ss].base + (uint16_t)offset_sp;
    MV->registros[LAR] = MV->registros[SP];
    MV->registros[MAR] = (4 << 24) | dir_fisica;

    uint32_t val = ((uint32_t)MV->memoria[dir_fisica]     << 24) |
                   ((uint32_t)MV->memoria[dir_fisica + 1] << 16) |
                   ((uint32_t)MV->memoria[dir_fisica + 2] << 8)  |
                   ((uint32_t)MV->memoria[dir_fisica + 3]);

    MV->registros[MBR] = (int32_t)val;
    *destino = val;

    // 3. Incrementar el valor de SP en 4
    MV->registros[SP] += 4;
    return 1;
}

void PUSH(TipoMV *MV) {
    uint32_t val = obtenerDato(MV, OP1);
    if (MV->registros[IP] == -1) return;
    apilar32(MV, val);
}

void POP(TipoMV *MV) {
    uint32_t val = 0;
    if (!desapilar32(MV, &val)) return;
    guardarDato(MV, OP1, (int32_t)val);
}

void CALL(TipoMV *MV) {
    uint32_t offset_destino = obtenerDato(MV, OP1) & 0xFFFF;
    if (MV->registros[IP] == -1) return;

    if (apilar32(MV, (uint32_t)MV->registros[IP])) {
        MV->registros[IP] = (MV->registros[CS] & 0xFFFF0000) | offset_destino;
    }
}

void RET(TipoMV *MV) {
    uint32_t dir_retorno = 0;
    if (desapilar32(MV, &dir_retorno)) {
        MV->registros[IP] = (int32_t)dir_retorno;
    }
}

// ============================================================================
// LLAMADAS AL SISTEMA (SYS 0, 1, 2, 3, 4, F)
// ============================================================================

void imprimir_segun_formato(uint32_t dato, uint32_t formato, uint16_t tamano) {
    if (formato & 0x10) { // Binario
        printf("0b");
        for (int b = (tamano * 8) - 1; b >= 0; b--) {
            printf("%d", (dato >> b) & 1);
        }
        printf(" ");
    }
    if (formato & 0x08) printf("0x%X ", dato); // Hexadecimal
    if (formato & 0x04) printf("0o%o ", dato); // Octal
    if (formato & 0x02) { // ASCII
        for (int b = tamano - 1; b >= 0; b--) {
            uint8_t c = (dato >> (b * 8)) & 0xFF;
            if (c >= 32 && c <= 126) printf("%c", c);
            else printf(".");
        }
        printf(" ");
    }
    if (formato & 0x01) { // Decimal con signo según tamaño
        int32_t con_signo = (int32_t)dato;
        if (tamano == 1) con_signo = (int32_t)(int8_t)(dato & 0xFF);
        else if (tamano == 2) con_signo = (int32_t)(int16_t)(dato & 0xFFFF);
        printf("%d ", con_signo);
    }
    printf("\n");
}

void sys_write(TipoMV *MV, uint16_t cantidad, uint16_t tamano, uint16_t seg, int32_t offset_ini, uint32_t formato) {
    uint16_t base = MV->tablaSegmento[seg].base;
    uint16_t tam_seg = MV->tablaSegmento[seg].tamano;

    for (int i = 0; i < cantidad; i++) {
        int32_t offset_actual = offset_ini + (i * tamano);
        if (offset_actual < 0 || (offset_actual + tamano) > tam_seg) {
            printf("\n--- ERROR DE SEGMENTO en SYS WRITE ---\n");
            MV->registros[IP] = -1;
            return;
        }
        uint16_t dir_fisica = base + (uint16_t)offset_actual;
        printf("[%04X]: ", dir_fisica);
        
        uint32_t dato = 0;
        for (int b = 0; b < tamano; b++) {
            dato = (dato << 8) | MV->memoria[dir_fisica + b]; 
        }
        imprimir_segun_formato(dato, formato, tamano);
    }
}

void sys_read(TipoMV *MV, uint16_t cantidad, uint16_t tamano, uint16_t seg, int32_t offset_ini, uint32_t formato) {
    uint16_t base = MV->tablaSegmento[seg].base;
    uint16_t tam_seg = MV->tablaSegmento[seg].tamano;

    // No se puede escribir en KS
    if (MV->registros[KS] != -1 && seg == ((MV->registros[KS] >> 16) & 0xFFFF)) {
        printf("\n--- ERROR DE SEGMENTO: Intento de escritura en KS ---\n");
        MV->registros[IP] = -1;
        return;
    }

    for (int i = 0; i < cantidad; i++) {
        int32_t offset_actual = offset_ini + (i * tamano);
        if (offset_actual < 0 || (offset_actual + tamano) > tam_seg) {
            printf("\n--- ERROR DE SEGMENTO en SYS READ ---\n");
            MV->registros[IP] = -1;
            return;
        }
        uint16_t dir_fisica = base + (uint16_t)offset_actual;
        printf("[%04X]: ", dir_fisica); 

        uint32_t dato_ingresado = 0;
        if (formato & 0x01) scanf("%d", &dato_ingresado);
        else if (formato & 0x02) { 
            char c;
            scanf(" %c", &c);
            dato_ingresado = (uint8_t)c;
        } 
        else if (formato & 0x04) scanf("%o", &dato_ingresado);
        else if (formato & 0x08) scanf("%x", &dato_ingresado);
        else if (formato & 0x10) { 
            char strbinario[65];
            scanf("%64s", strbinario);
            char *ptr = strbinario;
            if (strbinario[0] == '0' && (strbinario[1] == 'b' || strbinario[1] == 'B')) ptr += 2; 
            dato_ingresado = (uint32_t)strtoul(ptr, NULL, 2);
        }

        for (int b = tamano - 1; b >= 0; b--) {
            MV->memoria[dir_fisica + b] = dato_ingresado & 0xFF;
            dato_ingresado >>= 8;
        }
    }
}

// SYS 3: STRING READ
static void sys_string_read(TipoMV *MV, uint16_t seg, int32_t offset_ini) {
    if (MV->registros[KS] != -1 && seg == ((MV->registros[KS] >> 16) & 0xFFFF)) {
        printf("\n--- ERROR DE SEGMENTO: Intento de escritura en KS ---\n");
        MV->registros[IP] = -1;
        return;
    }

    uint16_t base = MV->tablaSegmento[seg].base;
    uint16_t tam_seg = MV->tablaSegmento[seg].tamano;
    int16_t max_chars = (int16_t)(MV->registros[ECX] & 0xFFFF); // -1 = sin límite

    char buffer[4096];
    if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
        buffer[0] = '\0';
    } else {
        if (buffer[0] == '\n' && buffer[1] == '\0') {
            if (fgets(buffer, sizeof(buffer), stdin) == NULL) buffer[0] = '\0';
        }
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') buffer[len - 1] = '\0';
    }

    int i = 0;
    while (buffer[i] != '\0' && (max_chars == -1 || i < max_chars)) {
        if ((offset_ini + i) < 0 || (offset_ini + i) >= tam_seg) {
            printf("\n--- ERROR DE SEGMENTO en STRING READ ---\n");
            MV->registros[IP] = -1;
            return;
        }
        MV->memoria[base + offset_ini + i] = (uint8_t)buffer[i];
        i++;
    }

    if ((offset_ini + i) < 0 || (offset_ini + i) >= tam_seg) {
        printf("\n--- ERROR DE SEGMENTO en STRING READ ---\n");
        MV->registros[IP] = -1;
        return;
    }
    MV->memoria[base + offset_ini + i] = '\0';
}

// SYS 4: STRING WRITE
static void sys_string_write(TipoMV *MV, uint16_t seg, int32_t offset_ini) {
    uint16_t base = MV->tablaSegmento[seg].base;
    uint16_t tam_seg = MV->tablaSegmento[seg].tamano;

    int32_t offset_actual = offset_ini;
    while (1) {
        if (offset_actual < 0 || offset_actual >= tam_seg) {
            printf("\n--- ERROR DE SEGMENTO en STRING WRITE ---\n");
            MV->registros[IP] = -1;
            return;
        }
        uint8_t c = MV->memoria[base + offset_actual];
        if (c == '\0') break;
        putchar(c);
        offset_actual++;
    }
    fflush(stdout);
}

// SYS F: BREAKPOINT
void ejecutar_breakpoint(TipoMV *MV) {
    if (MV->arch_vmi[0] == '\0') return;

    guardar_vmi(MV->arch_vmi, MV);

    int c = getchar();
    if (c == 'g' || c == 'G') {
        while (c != '\n' && c != EOF) c = getchar();
        MV->paso_a_paso = 0;
    } else if (c == 'q' || c == 'Q') {
        while (c != '\n' && c != EOF) c = getchar();
        MV->paso_a_paso = 0;
        MV->registros[IP] = -1;
    } else if (c == '\n') {
        MV->paso_a_paso = 1;
    } else {
        while (c != '\n' && c != EOF) c = getchar();
    }
}

void SYS(TipoMV *MV) {
    int32_t tipo_llamada = obtenerDato(MV, OP1);
    if (MV->registros[IP] == -1) return;

    if (tipo_llamada == 0) { // CLEAR SCREEN
        system("clear || cls");
        return;
    }
    if (tipo_llamada == 0x0F || tipo_llamada == 15) { // BREAKPOINT
        ejecutar_breakpoint(MV);
        return;
    }

    uint32_t edx_crudo = (uint32_t)MV->registros[EDX];
    uint16_t seg = (edx_crudo >> 16) & 0xFFFF;
    int32_t offset_ini = (int32_t)(int16_t)(edx_crudo & 0xFFFF);

    if (seg >= SEGMENTOS || MV->tablaSegmento[seg].base == 0xFFFF) {
        printf("\n--- ERROR DE SEGMENTO: EDX no apunta a un segmento valido ---\n");
        MV->registros[IP] = -1;
        return;
    }

    uint16_t cantidad = MV->registros[ECX] & 0xFFFF;
    uint16_t tamano = (MV->registros[ECX] >> 16) & 0xFFFF;

    if (tipo_llamada == 1) { 
        sys_read(MV, cantidad, tamano, seg, offset_ini, MV->registros[EAX]);
    } else if (tipo_llamada == 2) { 
        sys_write(MV, cantidad, tamano, seg, offset_ini, MV->registros[EAX]);
    } else if (tipo_llamada == 3) {
        sys_string_read(MV, seg, offset_ini);
    } else if (tipo_llamada == 4) {
        sys_string_write(MV, seg, offset_ini);
    }
}

// ============================================================================
// SALTOS, NOT Y STOP
// ============================================================================

static void saltar_a_offset(TipoMV *MV, uint16_t offset_destino) {
    MV->registros[IP] = (MV->registros[CS] & 0xFFFF0000) | (offset_destino & 0xFFFF);
}

void JMP(TipoMV *MV) {
    uint16_t dest = (uint16_t)obtenerDato(MV, OP1);
    if (MV->registros[IP] != -1) saltar_a_offset(MV, dest);
}

void JP(TipoMV *MV) {
    uint16_t dest = (uint16_t)obtenerDato(MV, OP1);
    int n = (MV->registros[CC] >> 31) & 1;
    int z = (MV->registros[CC] >> 30) & 1;
    if (MV->registros[IP] != -1 && n == 0 && z == 0) saltar_a_offset(MV, dest);
}

void JN(TipoMV *MV) {
    uint16_t dest = (uint16_t)obtenerDato(MV, OP1);
    int n = (MV->registros[CC] >> 31) & 1;
    if (MV->registros[IP] != -1 && n == 1) saltar_a_offset(MV, dest);
}

void JZ(TipoMV *MV) {
    uint16_t dest = (uint16_t)obtenerDato(MV, OP1);
    int z = (MV->registros[CC] >> 30) & 1;
    if (MV->registros[IP] != -1 && z == 1) saltar_a_offset(MV, dest);
}

void JC(TipoMV *MV) {
    uint16_t dest = (uint16_t)obtenerDato(MV, OP1);
    int c = (MV->registros[CC] >> 29) & 1;
    if (MV->registros[IP] != -1 && c == 1) saltar_a_offset(MV, dest);
}

void JV(TipoMV *MV) {
    uint16_t dest = (uint16_t)obtenerDato(MV, OP1);
    int v = (MV->registros[CC] >> 28) & 1;
    if (MV->registros[IP] != -1 && v == 1) saltar_a_offset(MV, dest);
}

void JNP(TipoMV *MV) {
    uint16_t dest = (uint16_t)obtenerDato(MV, OP1);
    int n = (MV->registros[CC] >> 31) & 1;
    int z = (MV->registros[CC] >> 30) & 1;
    if (MV->registros[IP] != -1 && (n == 1 || z == 1)) saltar_a_offset(MV, dest);
}

void JNN(TipoMV *MV) {
    uint16_t dest = (uint16_t)obtenerDato(MV, OP1);
    int n = (MV->registros[CC] >> 31) & 1;
    if (MV->registros[IP] != -1 && n == 0) saltar_a_offset(MV, dest);
}

void JNZ(TipoMV *MV) {
    uint16_t dest = (uint16_t)obtenerDato(MV, OP1);
    int z = (MV->registros[CC] >> 30) & 1;
    if (MV->registros[IP] != -1 && z == 0) saltar_a_offset(MV, dest);
}

void NOT(TipoMV *MV) {
    int32_t val = obtenerDato(MV, OP1);
    if (MV->registros[IP] == -1) return;

    int32_t res = ~val;
    guardarDato(MV, OP1, res);
    actualizaCC(MV, (int64_t)res, (uint64_t)res & 0xFFFFFFFF);
}

void STOP(TipoMV *MV) {
    MV->registros[IP] = -1;
}