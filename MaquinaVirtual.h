#define REGISTROS 32    
#define MEMORIA 16384   
#define SEGMENTOS 8     

// Registros internos de la máquina
#define IP 0
#define OPC 1
#define OP1 2
#define OP2 3
#define LAR 4
#define MAR 5
#define MBR 6

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



// La tabla de descriptores de segmentos
typedef struct {
    uint16_t base; //este tipo de dato viene en stdint.h basicamente nos aseguramos que sean de los bits necesarios
    uint16_t tamano; 
} DescriptorSegmento;


// Estructura principal de la Máquina Virtual
typedef struct {
    int32_t registros[REGISTROS];          
    uint8_t memoria[MEMORIA];              
    DescriptorSegmento tablaSegmento[SEGMENTOS]; 
} TipoMV;