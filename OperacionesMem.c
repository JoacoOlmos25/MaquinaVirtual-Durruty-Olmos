
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "OperacionesMem.h"
#include "MaquinaVirtual.h"
#include "mascaras.h"

uint16_t DirecLogica(TipoMV *MV, int32_t corrimiento, int8_t reg) {
    uint32_t registro_crudo = (uint32_t)MV->registros[reg];
    uint16_t seg = (registro_crudo >> 16) & 0xFFFF;
    if (seg >= SEGMENTOS || MV->tablaSegmento[seg].base == 0xFFFF) {
        return 0xFFFF;
    }
    uint16_t posini = MV->tablaSegmento[seg].base;
    int16_t offset = (int16_t)(registro_crudo & 0xFFFF);
    return (uint16_t)(posini + offset + corrimiento);
}

// Decodifica la cantidad de bytes del operando de memoria: 1 ('b'), 2 ('w') o 4 ('l')
static int obtenerCantBytesMem(int32_t valor_op) {
    uint8_t mod_bits = (valor_op >> 5) & 0x07;
    if (mod_bits == 1) return 1; // 'b' (byte = 1)
    if (mod_bits == 2) return 2; // 'w' (word = 2)
    return 4;                    // 'l' (long = 4, o 000 en v1)
}

void lecturaDeMemoria(TipoMV *MV, int OP) {
    int32_t valor = MV->registros[OP];
    int8_t reg = valor & Masc_CodO;
    int16_t offset = (int16_t)((valor >> 8) & 0xFFFF);
    int cant_bytes = obtenerCantBytesMem(valor);

    uint32_t reg_crudo = (uint32_t)MV->registros[reg];
    uint16_t seg = (reg_crudo >> 16) & 0xFFFF;
    int32_t offset_total = (int32_t)(int16_t)(reg_crudo & 0xFFFF) + offset;

    // Validar que el segmento exista y que todos los bytes accedidos estén dentro del segmento
    if (seg >= SEGMENTOS || MV->tablaSegmento[seg].base == 0xFFFF ||
        offset_total < 0 || (offset_total + cant_bytes) > MV->tablaSegmento[seg].tamano) {
        MV->registros[IP] = -1;
        printf("\n--- ERROR DE SEGMENTO: Acceso denegado ---\n");
        return;
    }

    uint16_t base = MV->tablaSegmento[seg].base;
    uint16_t dir_fisica = (uint16_t)(base + offset_total);

    // LAR guarda la dirección lógica (segmento en los 2 bytes altos, offset en los 2 bajos)
    MV->registros[LAR] = (seg << 16) | ((uint16_t)offset_total & 0xFFFF);
    // MAR guarda la cantidad de bytes en el byte más alto y la dirección física en los 2 bajos
    MV->registros[MAR] = (cant_bytes << 24) | dir_fisica;
}

void CargaAMemoria(TipoMV *MV, int OP) {
    lecturaDeMemoria(MV, OP);
    if (MV->registros[IP] == -1) return;

    // Validar que no se intente escribir en el Const Segment (KS es de solo lectura)
    if (MV->registros[KS] != -1) {
        uint16_t seg_ks = (MV->registros[KS] >> 16) & 0xFFFF;
        uint16_t seg_acceso = (MV->registros[LAR] >> 16) & 0xFFFF;
        if (seg_acceso == seg_ks) {
            MV->registros[IP] = -1;
            printf("\n--- ERROR DE SEGMENTO: Intento de escritura en KS ---\n");
            return;
        }
    }

    uint16_t dir_fisica = MV->registros[MAR] & 0xFFFF;
    int leer = (MV->registros[MAR] >> 24) & 0xFF; 
    uint32_t dato = (uint32_t)MV->registros[MBR]; 

    for (int i = 0; i < leer; i++) {
        // Guardamos en la memoria RAM byte por byte (Big Endian)
        MV->memoria[dir_fisica + i] = (dato >> (8 * (leer - 1 - i))) & 0xFF;
    }
}

void TraigoDeMemoria(TipoMV *MV, int OP) {
    lecturaDeMemoria(MV, OP);
    if (MV->registros[IP] == -1) return;

    uint16_t valor = MV->registros[MAR] & 0xFFFF;
    int leer = (MV->registros[MAR] >> 24) & Masc_Byte_Menos_Sig;
    
    uint32_t dato = 0;
    for (int i = 0; i < leer; i++) {
        dato = (dato << 8) | MV->memoria[valor + i];
    } 

    // Extensión de signo a 32 bits cuando se leen 1 ('b') o 2 ('w') bytes
    if (leer == 1) {
        MV->registros[MBR] = (int32_t)(int8_t)(dato & 0xFF);
    } else if (leer == 2) {
        MV->registros[MBR] = (int32_t)(int16_t)(dato & 0xFFFF);
    } else {
        MV->registros[MBR] = (int32_t)dato;
    }
}
