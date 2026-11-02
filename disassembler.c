
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "disassembler.h"
#include "MaquinaVirtual.h"

// Vector de operaciones con PUSH (0x0B), POP (0x0C), CALL (0x0D) y RET (0x0E)
const char* mnemonicos[32] = {
    "SYS", "JMP", "JP", "JN", "JZ", "JC", "JV", "JNP", 
    "JNN", "JNZ", "NOT", "PUSH", "POP", "CALL", "RET", "STOP", 
    "MOV", "ADD", "SUB", "MUL", "DIV", "CMP", "AND", "OR",  
    "XOR", "SWAP", "SHL", "SHR", "SAR", "LDL", "LDH", "RND" 
};

// Vector de registros actualizado con SP, BP, ES, SS, KS, PS
const char* nom_registros[32] = {
    "IP", "OPC", "OP1", "OP2", "LAR", "MAR", "MBR", "SP",
    "BP", "---", "EAX", "EBX", "ECX", "EDX", "EEX", "EFX",
    "AC", "CC", "---", "---", "---", "---", "---", "---",
    "---", "---", "CS", "DS", "ES", "SS", "KS", "PS"
};

// Auxiliar para obtener el nombre del registro o sub-registro (EAX, AX, AH, AL)
static void formatea_registro(uint8_t byte_op, char *destino) {
    uint8_t reg = byte_op & 0x1F;
    uint8_t sec = (byte_op >> 5) & 0x03;

    if (reg >= EAX && reg <= EFX && sec != 0) {
        char letra = 'A' + (reg - EAX);
        if (sec == 1) sprintf(destino, "%cL", letra);
        else if (sec == 2) sprintf(destino, "%cH", letra);
        else if (sec == 3) sprintf(destino, "%cX", letra);
    } else {
        sprintf(destino, "%s", nom_registros[reg]);
    }
}

// Auxiliar para formatear operando de memoria con prefijo b/w/l
static void formatea_memoria(int16_t desp, uint8_t byte_reg, char *destino) {
    uint8_t reg = byte_reg & 0x1F;
    uint8_t mod_bits = (byte_reg >> 5) & 0x07;
    char prefijo = 'l'; // Por defecto 4 bytes (l)
    
    if (mod_bits == 1) prefijo = 'b';
    else if (mod_bits == 2) prefijo = 'w';
    else if (mod_bits == 4) prefijo = 'l';

    if (desp == 0) sprintf(destino, "%c[%s]", prefijo, nom_registros[reg]);
    else if (desp > 0) sprintf(destino, "%c[%s+%d]", prefijo, nom_registros[reg], desp);
    else sprintf(destino, "%c[%s%d]", prefijo, nom_registros[reg], desp);
}

void generar_disassembler(TipoMV *MV) {
    FILE *arch = fopen("disassembler.txt", "w");

    // 1. DESENSAMBLAR CONST SEGMENT (KS) SI EXISTE
    if (MV->registros[KS] != -1) {
        uint16_t seg_ks = (MV->registros[KS] >> 16) & 0xFFFF;
        uint16_t base_ks = MV->tablaSegmento[seg_ks].base;
        uint16_t tam_ks = MV->tablaSegmento[seg_ks].tamano;

        uint16_t offset = 0;
        while (offset < tam_ks) {
            uint16_t dir_inicio = base_ks + offset;
            uint16_t len = 0;
            while ((offset + len) < tam_ks && MV->memoria[dir_inicio + len] != 0x00) {
                len++;
            }
            uint16_t total_str_bytes = len + 1; // Incluye el '\0'

            // Imprimimos tanto por consola como en el archivo disassembler.txt
            printf(" [%04X] ", dir_inicio);
            if (arch) fprintf(arch, " [%04X] ", dir_inicio);

            if (total_str_bytes > 7) {
                for (int i = 0; i < 6; i++) {
                    printf("%02X ", MV->memoria[dir_inicio + i]);
                    if (arch) fprintf(arch, "%02X ", MV->memoria[dir_inicio + i]);
                }
                printf(".. ");
                if (arch) fprintf(arch, ".. ");
            } else {
                for (int i = 0; i < total_str_bytes; i++) {
                    printf("%02X ", MV->memoria[dir_inicio + i]);
                    if (arch) fprintf(arch, "%02X ", MV->memoria[dir_inicio + i]);
                }
                for (int i = total_str_bytes; i < 7; i++) {
                    printf("   ");
                    if (arch) fprintf(arch, "   ");
                }
            }

            printf("| \"");
            if (arch) fprintf(arch, "| \"");
            for (int i = 0; i < len; i++) {
                uint8_t c = MV->memoria[dir_inicio + i];
                char imp = (c >= 32 && c <= 126) ? (char)c : '.';
                printf("%c", imp);
                if (arch) fprintf(arch, "%c", imp);
            }
            printf("\"\n");
            if (arch) fprintf(arch, "\"\n");

            offset += total_str_bytes;
        }
    }

    // 2. DESENSAMBLAR CODE SEGMENT (CS)
    if (MV->registros[CS] != -1) {
        uint16_t seg_cs = (MV->registros[CS] >> 16) & 0xFFFF;
        uint16_t base_cs = MV->tablaSegmento[seg_cs].base;
        uint16_t tam_codigo = MV->tablaSegmento[seg_cs].tamano;

        int ip_local = 0;
        while (ip_local < tam_codigo) {
            int ip_actual = base_cs + ip_local;
            
            uint8_t primer_byte = MV->memoria[ip_actual];
            uint8_t operacion = primer_byte & 0x1F;
            uint8_t tipo_opA = 0;
            uint8_t tipo_opB = 0;

            // Extraer tipos de operandos (ni RET=0x0E ni STOP=0x0F tienen operandos)
            if (operacion >= 0x10) {
                tipo_opA = (primer_byte & 0x30) >> 4;
                tipo_opB = (primer_byte & 0xC0) >> 6;
            } else if (operacion != 0x0E && operacion != 0x0F) {
                tipo_opA = (primer_byte & 0xC0) >> 6;
            } 

            int total_bytes = 1 + tipo_opA + tipo_opB;
            char marca = (ip_local == MV->entry_point) ? '>' : ' ';

            printf("%c[%04X] ", marca, ip_actual);
            if (arch) fprintf(arch, "%c[%04X] ", marca, ip_actual);

            for (int i = 0; i < total_bytes; i++) {
                printf("%02X ", MV->memoria[ip_actual + i]);
                if (arch) fprintf(arch, "%02X ", MV->memoria[ip_actual + i]);
            }
            for (int i = total_bytes; i < 7; i++) {
                printf("   ");
                if (arch) fprintf(arch, "   ");
            }
            
            const char *mnem = (operacion <= 0x1F) ? mnemonicos[operacion] : "---";
            printf("| %-4s ", mnem);
            if (arch) fprintf(arch, "| %-4s ", mnem);

            char str_opA[32] = "";
            char str_opB[32] = "";
            int offset = 1;

            // LEER OPERANDO B PRIMERO
            if (tipo_opB != 0) {
                if (tipo_opB == 1) { 
                    formatea_registro(MV->memoria[ip_actual + offset], str_opB);
                } else if (tipo_opB == 2) { 
                    int16_t inm = (MV->memoria[ip_actual + offset] << 8) | MV->memoria[ip_actual + offset + 1];
                    sprintf(str_opB, "%d", inm);
                } else if (tipo_opB == 3) { 
                    int16_t desp = (MV->memoria[ip_actual + offset] << 8) | MV->memoria[ip_actual + offset + 1];
                    formatea_memoria(desp, MV->memoria[ip_actual + offset + 2], str_opB);
                }
                offset += tipo_opB;
            }

            // LEER OPERANDO A SEGUNDO
            if (tipo_opA != 0) {
                if (tipo_opA == 1) { 
                    formatea_registro(MV->memoria[ip_actual + offset], str_opA);
                } else if (tipo_opA == 2) { 
                    int16_t inm = (MV->memoria[ip_actual + offset] << 8) | MV->memoria[ip_actual + offset + 1];
                    sprintf(str_opA, "%d", inm);
                } else if (tipo_opA == 3) { 
                    int16_t desp = (MV->memoria[ip_actual + offset] << 8) | MV->memoria[ip_actual + offset + 1];
                    formatea_memoria(desp, MV->memoria[ip_actual + offset + 2], str_opA);
                }
                offset += tipo_opA;
            }

            if (tipo_opA != 0 && tipo_opB != 0) {
                printf("%9s, %9s\n", str_opA, str_opB);
                if (arch) fprintf(arch, "%9s, %9s\n", str_opA, str_opB);
            } else if (tipo_opA != 0) {
                printf("%9s\n", str_opA);
                if (arch) fprintf(arch, "%9s\n", str_opA);
            } else {
                printf("\n");
                if (arch) fprintf(arch, "\n");
            }
            
            ip_local += total_bytes;
        }
    }

    if (arch) fclose(arch);
}