#include <stdio.h>
#include <stdint.h>
#include "disassembler.h"
#include "MaquinaVirtual.h"

// Inicializamos el vector de operaciones respetando los códigos
const char* mnemonicos[32] = {
    "SYS", "JMP", "JP", "JN", "JZ", "JC", "JV", "JNP", 
    "JNN", "JNZ", "NOT", "---", "---", "---", "---", "STOP", 
    "MOV", "ADD", "SUB", "MUL", "DIV", "CMP", "AND", "OR",  
    "XOR", "SWAP", "SHL", "SHR", "SAR", "LDL", "LDH", "RND" 
};

// Inicializamos los nombres de los registros
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
    int base_cs = MV->registros[CS]; 

    while (ip_local < tam_codigo) {
        int ip_actual = base_cs + ip_local;
        
        uint8_t primer_byte = MV->memoria[ip_actual];
        uint8_t operacion = primer_byte & 0x1F;
        uint8_t tipo_opA = 0;
        uint8_t tipo_opB = 0;

        // Extraer operación y tipos dinámicamente
        if (operacion >= 0x10) {
            tipo_opA = (primer_byte & 0x30) >> 4;
            tipo_opB = (primer_byte & 0xC0) >> 6;
        } else if (operacion != 0x0F) {
            tipo_opA = (primer_byte & 0xC0) >> 6;
        } 

        int total_bytes = 1 + tipo_opA + tipo_opB;
        fprintf(arch, "[%04X] ", ip_actual);

        for (int i = 0; i < total_bytes; i++) {
            fprintf(arch, "%02X ", MV->memoria[ip_actual + i]);
        }
        
        for (int i = total_bytes; i < 7; i++) {
            fprintf(arch, "   ");
        }
        
        fprintf(arch, "| %-4s ", mnemonicos[operacion]);

        char str_opA[32] = "";
        char str_opB[32] = "";
        int offset = 1;

        // LEER OPERANDO B PRIMERO (El orden en memoria está invertido)[cite: 11]
        if (tipo_opB != 0) {
            if (tipo_opB == 1) { 
                int reg = MV->memoria[ip_actual + offset];
                sprintf(str_opB, "%s", nom_registros[reg]);
            } else if (tipo_opB == 2) { 
                int16_t inm = (MV->memoria[ip_actual + offset] << 8) | MV->memoria[ip_actual + offset + 1];
                sprintf(str_opB, "%d", inm);
            } else if (tipo_opB == 3) { 
                int16_t desp = (MV->memoria[ip_actual + offset] << 8) | MV->memoria[ip_actual + offset + 1];
                int reg = MV->memoria[ip_actual + offset + 2] & 0x1F;
                if (desp == 0) sprintf(str_opB, "[%s]", nom_registros[reg]);
                else if (desp > 0) sprintf(str_opB, "[%s+%d]", nom_registros[reg], desp);
                else sprintf(str_opB, "[%s%d]", nom_registros[reg], desp);
            }
            offset += tipo_opB;
        }

        // LEER OPERANDO A SEGUNDO[cite: 11]
        if (tipo_opA != 0) {
            if (tipo_opA == 1) { 
                int reg = MV->memoria[ip_actual + offset];
                sprintf(str_opA, "%s", nom_registros[reg]);
            } else if (tipo_opA == 2) { 
                int16_t inm = (MV->memoria[ip_actual + offset] << 8) | MV->memoria[ip_actual + offset + 1];
                sprintf(str_opA, "%d", inm);
            } else if (tipo_opA == 3) { 
                int16_t desp = (MV->memoria[ip_actual + offset] << 8) | MV->memoria[ip_actual + offset + 1];
                int reg = MV->memoria[ip_actual + offset + 2] & 0x1F;
                if (desp == 0) sprintf(str_opA, "[%s]", nom_registros[reg]);
                else if (desp > 0) sprintf(str_opA, "[%s+%d]", nom_registros[reg], desp);
                else sprintf(str_opA, "[%s%d]", nom_registros[reg], desp);
            }
            offset += tipo_opA;
        }

        // Formatear la salida final
        if (tipo_opA != 0 && tipo_opB != 0) {
            fprintf(arch, "%9s, %9s\n", str_opA, str_opB);
        } else if (tipo_opA != 0) {
            fprintf(arch, "%9s\n", str_opA);
        } else {
            fprintf(arch, "\n");
        }
        
        ip_local += total_bytes;
    }

    fclose(arch);
}