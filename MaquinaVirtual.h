#pragma once // es para que cuando compilemos no lea este archivo mas de una vez asi no tira error. 

#include <stdint.h>

#define REGISTROS 32    
#define MEMORIA 16384  
#define MEMORIA_DEFECTO 16384
#define SEGMENTOS 8     

// Registros internos de la máquina
#define IP 0
#define OPC 1
#define OP1 2
#define OP2 3
#define LAR 4
#define MAR 5
#define MBR 6

// Registros de Pila (Parte 2)
#define SP 7
#define BP 8

// Registros de propósito general
#define EAX 10
#define EBX 11
#define ECX 12
#define EDX 13
#define EEX 14
#define EFX 15

// Registros especiales y de segmento
#define AC 16
#define CC 17
#define CS 26
#define DS 27
#define ES 28
#define SS 29
#define KS 30
#define PS 31


// La tabla de descriptores de segmentos
typedef struct {
    uint16_t base; //este tipo de dato viene en stdint.h basicamente nos aseguramos que sean de los bits necesarios
    uint16_t tamano; 
} DescriptorSegmento;


// // Estructura principal de la Máquina Virtual
// typedef struct {
//     int32_t registros[REGISTROS];          
//     uint8_t memoria[MEMORIA];              
//     DescriptorSegmento tablaSegmento[SEGMENTOS]; 
// } TipoMV;

// Estructura principal de la Máquina Virtual
typedef struct {
    int32_t registros[REGISTROS];          
    uint8_t *memoria;                      // Memoria dinámica según m=M
    uint32_t tam_memoria;                  // Tamaño total en bytes
    DescriptorSegmento tablaSegmento[SEGMENTOS]; 
    uint16_t entry_point;                  // Offset del entry point (main)
    char arch_vmi[256];                    // Nombre del archivo .vmi (si se especificó)
    int paso_a_paso;                       // Flag para ejecución paso a paso en breakpoint
} TipoMV;

// Prototipo para guardar la imagen .vmi (se usará en SYS 0xF)
int guardar_vmi(const char *nombre_arch, TipoMV *MV);