#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "operadoresPrueba.h" 

#define REGISTROS 32    
#define MEMORIA 16384   
#define SEGMENTOS 8     

// Índices de los registros clave
#define IP 0
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



// "Operacion" es un puntero a una función que recibe TipoMV*
typedef void (*Operacion)(TipoMV *MV); 

Operacion instruccion[32] = {
    SYS, JMP, JP, JN, JZ, JC, JV, JNP, // 0x00 a 0x07
    JNN, JNZ, NOT, NULL, NULL, NULL, NULL, STOP, // 0x08 a 0x0F
    MOV, ADD, SUB, MUL, DIV, CMP, AND, OR,  // 0x10 a 0x17
    XOR, SWAP, SHL, SHR, SAR, LDL, LDH, RND // 0x18 a 0x1F
};


// Verifica el identificador y la versión
int verifica_cabecera(uint8_t cabecera[6]) { 
    uint8_t comp[6] = {'V', 'M', 'X', '2', '6', 1};
    for (int i = 0; i < 6; i++) {
        if (cabecera[i] != comp[i]) return 0; // Falla si un byte no coincide
    }
    return 1; // Cabecera correcta
}


// Inicializa la MV y carga el programa en memoria
void inicializacion(char nombre_arch[], TipoMV *MV) {
    FILE *arch = fopen(nombre_arch, "rb");
    if (arch == NULL) {
        printf("Error al abrir el archivo .vmx\n");
        exit(1);
    }

    uint8_t cabecera[6];
    // Leemos los primeros 6 bytes de un tirón
    fread(cabecera, sizeof(uint8_t), 6, arch);

    if (verifica_cabecera(cabecera)) { 
        printf("Cabecera VMX26 y version correctas.\n");

       // Leemos los bytes 6 y 7 como bytes individuales
        uint8_t buffer_tam[2];
        fread(buffer_tam, sizeof(uint8_t), 2, arch);

        // Unimos los bytes en el orden correcto (Big-Endian)
        // Movemos el byte 0 hacia la izquierda y le sumamos el byte 1
        uint16_t tam_codigo = (buffer_tam[0] << 8) | buffer_tam[1];

        // Configuramos la tabla de segmentos (sigue igual)
        MV->tablaSegmento[0].base = 0;
        MV->tablaSegmento[0].tamano = tam_codigo;
        MV->tablaSegmento[1].base = tam_codigo;
        MV->tablaSegmento[1].tamano = MEMORIA - tam_codigo;

        // Entradas sin usar en -1 (0xFFFF)
        for(int i = 2; i < SEGMENTOS; i++) {
            MV->tablaSegmento[i].base = 0xFFFF;
            MV->tablaSegmento[i].tamano = 0xFFFF;
        }

        // Limpiamos e inicializamos los registros
        for(int i = 0; i < REGISTROS; i++)
             MV->registros[i] = 0;
        MV->registros[CS] = 0 << 16; 
        MV->registros[DS] = 1 << 16; 
        MV->registros[IP] = MV->registros[CS];

        // Guardamos las instrucciones en el code segment de la memoria
        int desp = 0;
        while (fread(&MV->memoria[MV->registros[CS] + desp], sizeof(uint8_t), 1, arch) == 1) {
            desp++;    
        }
        printf("Carga exitosa: %d bytes copiados a memoria.\n", desp);


        // Volcado de prueba en consola
        printf("\n--- Volcado de Memoria (Codigo Cargado) ---\n");
        
        // Iteramos exclusivamente sobre el tamaño del código que nos dijo la cabecera
        for (int i = 0; i < tam_codigo; i++) {
            // Imprimimos cada byte en formato hexadecimal estandarizado (%02X)
            printf("%02X ", MV->memoria[MV->registros[CS] + i]);
            
            // Hacemos un salto de línea cada 16 bytes para que se lea fácil
            if ((i + 1) % 16 == 0) {
                printf("\n");
            }
        }
        printf("\n-------------------------------------------\n");

    }
    fclose(arch);
}

int main() {
    TipoMV MV;
    // Llamamos a la función pasando el archivo binario generado
    inicializacion("prueba.vmx", &MV);
    
    return 0;
}

// El Ciclo de EjecuciónSegún el documento de la cátedra, la ejecución es un proceso que se repite y consiste en los siguientes pasos: 
//  Leer la instrucción a la que apunta actualmente el registro IP.  
//  Almacenar el código de operación de esa instrucción en el registro OPC.
//  Guardar en los registros OP1 y OP2 la información de los operandos A y B.
//  Avanzar el registro IP hacia la próxima instrucción (sumando la cantidad de bytes que ocupó la actual).  
//  Realizar la operación correspondiente (sumar, mover, comparar, etc.).
//  El documento también aclara que este ciclo se debe repetir hasta que el registro IP apunte fuera del segmento de código, o hasta que se ejecute una instrucción STOP, la cual le asigna un -1 (o 0xFFFFFFFF) al IP para forzar la detención

// Para decodificar los byte hay que usar las mascaras  OPC con 0x1F ; OP1 con 0x30 OP2 con 0x60;

//Para usar un vector de operadores definimos el indicie como la pos que ocupan y adentro los nombres directamente.

