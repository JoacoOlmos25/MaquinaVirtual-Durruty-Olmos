#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "operadores.h"
#include "mascaras.h"
#include "MaquinaVirtual.h"
#include "disassembler.h"

typedef void (*Operacion)(TipoMV *MV); 

Operacion instruccion[32] = {
    SYS, JMP, JP, JN, JZ, JC, JV, JNP, // 0x00 a 0x07
    JNN, JNZ, NOT, NULL, NULL, NULL, NULL, STOP, // 0x08 a 0x0F
    MOV, ADD, SUB, MUL, DIV, CMP, AND, OR,  // 0x10 a 0x17
    XOR, SWAP, SHL, SHR, SAR, LDL, LDH, RND // 0x18 a 0x1F
};

int verifica_cabecera(uint8_t cabecera[6]) { 
    uint8_t comp[6] = {'V', 'M', 'X', '2', '6', 1}; 
    for (int i = 0; i < 6; i++) {
        if (cabecera[i] != comp[i]) return 0; 
    }
    return 1; 
}

void inicializacion(char nombre_arch[], TipoMV *MV,uint16_t *tam_codigo) {
    FILE *arch = fopen(nombre_arch, "rb");
    if (arch == NULL) {
        printf("Error al abrir el archivo .vmx\n");
        exit(1);
    }

    uint8_t cabecera[6];
    fread(cabecera, sizeof(uint8_t), 6, arch);

    if (verifica_cabecera(cabecera)) { 
        printf("Cabecera VMX26 y version correctas.\n");

        uint8_t buffer_tam[2];
        fread(buffer_tam, sizeof(uint8_t), 2, arch);

        *tam_codigo = (buffer_tam[0] << 8) | buffer_tam[1];

        // Configuramos la tabla de segmentos
        MV->tablaSegmento[0].base = 0;
        MV->tablaSegmento[0].tamano = *tam_codigo;
        MV->tablaSegmento[1].base = *tam_codigo;                 
        MV->tablaSegmento[1].tamano = MEMORIA - *tam_codigo; 

        // Entradas sin usar en -1 (0xFFFF)
        for(int i = 2; i < SEGMENTOS; i++) {
            MV->tablaSegmento[i].base = 0xFFFF;
            MV->tablaSegmento[i].tamano = 0xFFFF;
        }

        // Limpiamos e inicializamos los registros Y LA MEMORIA
        for(int i = 0; i < REGISTROS; i++) {
             MV->registros[i] = 0;
        }
        for(int i = 0; i < MEMORIA; i++) {
             MV->memoria[i] = 0;
        }
        // Limpiamos e inicializamos los registros
        for(int i = 0; i < REGISTROS; i++)
             MV->registros[i] = 0;
        MV->registros[CS] = 0 << 16;
        MV->registros[DS] = 1 << 16;
        MV->registros[0] = MV->registros[CS];

        // Guardamos las instrucciones en el code segment de la memoria
        int desp = 0;
        while (fread(&MV->memoria[MV->registros[CS] + desp], sizeof(uint8_t), 1, arch) == 1) {
            desp++;    
        }
        printf("Carga exitosa: %d bytes copiados a memoria.\n", desp);

        printf("\n--- Volcado de Memoria (Codigo Cargado) ---\n");
        for (int i = 0; i < *tam_codigo; i++) {
            printf("%02X ", MV->memoria[MV->registros[CS] + i]);
            if ((i + 1) % 16 == 0) {
                printf("\n");
            }
        }
        printf("\n-------------------------------------------\n");

    }
    fclose(arch);
}

int existeOperacion(uint8_t ope){
    return (ope <= 0x1F) && !(ope >= 0x0B && ope <= 0x0E); 
}

void leer_operando(TipoMV *MV, uint8_t tipo, int OP) {
    int32_t valor = 0; 
    
    // Traducción lógica a física para la lectura del operando
    uint16_t seg_cs = (MV->registros[IP] >> 16) & 0xFFFF;
    uint16_t offset_ip = MV->registros[IP] & 0xFFFF;
    int posmem = MV->tablaSegmento[seg_cs].base + offset_ip;
    
    switch (tipo) {
        case 0: 
            break;
            
        case 1: 
            valor = MV->memoria[posmem];
            MV->registros[IP] += 1;
            break;
            
        case 2: { 
            int16_t inmediato = (MV->memoria[posmem] << 8) | MV->memoria[posmem + 1];
            valor = inmediato; 
            MV->registros[IP] += 2;
            break;
        }   
        case 3: 
            valor = (MV->memoria[posmem] << 16) | 
                    (MV->memoria[posmem + 1] << 8) | 
                    MV->memoria[posmem + 2];
            MV->registros[IP] += 3;
            break;
    }
    
    MV->registros[OP] = (tipo << 24) | (valor & 0x00FFFFFF); 
}

void ejecucion(TipoMV *MV) {
    MV->registros[IP] = MV->registros[CS];

    while (MV->registros[IP] != 0xFFFFFFFF) {
        
        // Extraemos los 16 bits altos (segmento) y los 16 bits bajos (offset) del IP
        uint16_t seg_cs = (MV->registros[IP] >> 16) & 0xFFFF;
        uint16_t offset_ip = MV->registros[IP] & 0xFFFF;
        
        // Calculamos la dirección física sumando la base del segmento y el offset
        int posmem = MV->tablaSegmento[seg_cs].base + offset_ip;
        
        // Leemos la memoria usando la dirección física real
        uint8_t primer_byte = MV->memoria[posmem];
        uint8_t operacion = primer_byte & 0x1F;

        if (existeOperacion(operacion)) {
            MV->registros[OPC] = operacion;
            uint8_t tipo_op1 = 0;
            uint8_t tipo_op2 = 0;

            if (operacion >= 0x10) { 
                tipo_op1 = (primer_byte & 0x30) >> 4;
                tipo_op2 = (primer_byte & 0xC0) >> 6;
            } else if (operacion != 0x0F) { 
                tipo_op1 = (primer_byte & 0xC0) >> 6;
            }

            // Al sumar 1 al IP, incrementamos su offset lógico
            MV->registros[IP] += 1;

            if (tipo_op2 != 0) {
                leer_operando(MV, tipo_op2, OP2);
            } else {
                MV->registros[OP2] = 0; 
            }

            if (tipo_op1 != 0) {
                leer_operando(MV, tipo_op1, OP1);
            } else {
                MV->registros[OP1] = 0;
            }

            if (instruccion[operacion] != NULL) {
                instruccion[operacion](MV);
            }
            
        } else {
            printf("Error: Instruccion invalida (0x%02X) en [0x%04X]\n", operacion, posmem);
            MV->registros[IP] = 0xFFFFFFFF; 
        }
    }
}

int main() {
    TipoMV MV;
    uint16_t tam_codigo;
    
    inicializacion("prueba.vmx", &MV, &tam_codigo);
    // Descomentar si deseas ver el disassembler antes de ejecutar
    generar_disassembler(&MV, tam_codigo);
    
    printf("\n--- Iniciando Ejecucion ---\n");
    ejecucion(&MV);
    printf("\n--- Ejecucion Finalizada ---\n");
    
    return 0;
}