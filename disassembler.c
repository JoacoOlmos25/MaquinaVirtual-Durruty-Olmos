#include <stdio.h>
#include "disassembler.h"

// Inicializamos el vector de operaciones respetando los códigos
const char* mnemonicos[32] = {
    "SYS", "JMP", "JP", "JN", "JZ", "JC", "JV", "JNP", 
    "JNN", "JNZ", "NOT", "---", "---", "---", "---", "STOP", 
    "MOV", "ADD", "SUB", "MUL", "DIV", "CMP", "AND", "OR",  
    "XOR", "SWAP", "SHL", "SHR", "SAR", "LDL", "LDH", "RND" 
};

// Inicializamos los nombres de los registros[cite: 2]
const char* nom_registros[32] = {
    "IP", "OPC", "OP1", "OP2", "LAR", "MAR", "MBR", "---",
    "---", "---", "EAX", "EBX", "ECX", "EDX", "EEX", "EFX",
    "AC", "CC", "---", "---", "---", "---", "---", "---",
    "---", "---", "CS", "DS", "---", "---", "---", "---"
};

void generar_disassembler(TipoMV *MV, int tam_codigo) {
    FILE *arch = fopen("disassembler.txt", "w");
    if (arch == NULL) {
        printf("Error al crear el archivo disassembler.txt\n");
        return;
    }

    int ip_local = 0; 
    while (ip_local < tam_codigo) {
        // 1. Leer el primer byte y extraer operación/tipos
        // 2. Determinar cuántos bytes extra ocupa la instrucción
        // 3. Imprimir en el archivo (fprintf) siguiendo el formato:
        //    fprintf(arch, "[%04X] ... | %s ...\n", ip_local, mnemonicos[cod_op]);
        
        // 4. Avanzar ip_local según el tamaño total de la instrucción
    }
    
    fclose(arch);
    printf("Archivo disassembler.txt generado con éxito.\n");
}