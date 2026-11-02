
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "operadores.h"
#include "mascaras.h"
#include "MaquinaVirtual.h"
#include "disassembler.h"

typedef void (*Operacion)(TipoMV *MV); 

Operacion instruccion[32] = {
    SYS, JMP, JP, JN, JZ, JC, JV, JNP,           // 0x00 a 0x07
    JNN, JNZ, NOT, PUSH, POP, CALL, RET, STOP,   // 0x08 a 0x0F
    MOV, ADD, SUB, MUL, DIV, CMP, AND, OR,       // 0x10 a 0x17
    XOR, SWAP, SHL, SHR, SAR, LDL, LDH, RND      // 0x18 a 0x1F
};

// Auxiliar para escribir un entero de 32 bits en Big Endian en una dirección física
static void escribir_mem32_be(TipoMV *MV, uint32_t dir_fisica, uint32_t valor) {
    MV->memoria[dir_fisica]     = (valor >> 24) & 0xFF;
    MV->memoria[dir_fisica + 1] = (valor >> 16) & 0xFF;
    MV->memoria[dir_fisica + 2] = (valor >> 8)  & 0xFF;
    MV->memoria[dir_fisica + 3] = valor & 0xFF;
}

// Guarda el estado actual de la MV en un archivo de imagen .vmi
int guardar_vmi(const char *nombre_arch, TipoMV *MV) {
    if (nombre_arch == NULL || nombre_arch[0] == '\0') return 0;

    FILE *arch = fopen(nombre_arch, "wb");
    if (!arch) {
        printf("Error al crear el archivo de imagen %s\n", nombre_arch);
        return 0;
    }

    // 1. Header (8 bytes): "VMI26", versión 1, tamaño memoria en KiB (2 bytes BE)
    uint8_t header[8] = {'V', 'M', 'I', '2', '6', 1, 0, 0};
    uint16_t mem_kib = (uint16_t)(MV->tam_memoria / 1024);
    header[6] = (mem_kib >> 8) & 0xFF;
    header[7] = mem_kib & 0xFF;
    fwrite(header, sizeof(uint8_t), 8, arch);

    // 2. Registros (32 x 4 = 128 bytes en Big Endian)
    for (int i = 0; i < REGISTROS; i++) {
        uint32_t reg_val = (uint32_t)MV->registros[i];
        uint8_t buf[4] = {
            (reg_val >> 24) & 0xFF,
            (reg_val >> 16) & 0xFF,
            (reg_val >> 8)  & 0xFF,
            reg_val & 0xFF
        };
        fwrite(buf, sizeof(uint8_t), 4, arch);
    }

    // 3. Tabla de descriptores de segmentos (8 x 4 = 32 bytes en Big Endian)
    for (int i = 0; i < SEGMENTOS; i++) {
        uint16_t base = MV->tablaSegmento[i].base;
        uint16_t tam  = MV->tablaSegmento[i].tamano;
        uint8_t buf[4] = {
            (base >> 8) & 0xFF, base & 0xFF,
            (tam >> 8)  & 0xFF, tam & 0xFF
        };
        fwrite(buf, sizeof(uint8_t), 4, arch);
    }

    // 4. Memoria principal completa
    fwrite(MV->memoria, sizeof(uint8_t), MV->tam_memoria, arch);

    fclose(arch);
    return 1;
}

// Carga el estado de la MV desde un archivo .vmi
void cargar_vmi(const char *nombre_arch, TipoMV *MV) {
    FILE *arch = fopen(nombre_arch, "rb");
    if (!arch) {
        printf("Error al abrir el archivo de imagen %s\n", nombre_arch);
        exit(1);
    }

    uint8_t header[8];
    if (fread(header, sizeof(uint8_t), 8, arch) != 8 ||
        memcmp(header, "VMI26", 5) != 0 || header[5] != 1) {
        printf("Error: Cabecera VMI26 invalida.\n");
        fclose(arch);
        exit(1);
    }

    uint16_t mem_kib = (header[6] << 8) | header[7];
    MV->tam_memoria = (uint32_t)mem_kib * 1024;
    MV->memoria = (uint8_t *)calloc(MV->tam_memoria, sizeof(uint8_t));
    if (!MV->memoria) {
        printf("Error al reservar memoria principal.\n");
        fclose(arch);
        exit(1);
    }

    for (int i = 0; i < REGISTROS; i++) {
        uint8_t buf[4];
        fread(buf, sizeof(uint8_t), 4, arch);
        MV->registros[i] = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
    }

    for (int i = 0; i < SEGMENTOS; i++) {
        uint8_t buf[4];
        fread(buf, sizeof(uint8_t), 4, arch);
        MV->tablaSegmento[i].base   = (buf[0] << 8) | buf[1];
        MV->tablaSegmento[i].tamano = (buf[2] << 8) | buf[3];
    }

    fread(MV->memoria, sizeof(uint8_t), MV->tam_memoria, arch);
    MV->entry_point = MV->registros[IP] & 0xFFFF;

    fclose(arch);
}

// Inicializa la MV desde un archivo .vmx (soporta Versión 1 y Versión 2)
void inicializacion_vmx(const char *nombre_arch, TipoMV *MV, int cant_params, char *params[]) {
    FILE *arch = fopen(nombre_arch, "rb");
    if (arch == NULL) {
        printf("Error al abrir el archivo .vmx: %s\n", nombre_arch);
        exit(1);
    }

    uint8_t cabecera[6];
    if (fread(cabecera, sizeof(uint8_t), 6, arch) != 6 || memcmp(cabecera, "VMX26", 5) != 0) {
        printf("Error: El archivo no posee una cabecera VMX26 valida.\n");
        fclose(arch);
        exit(1);
    }

    uint8_t version = cabecera[5];
    if (version != 1 && version != 2) {
        printf("Error: Version de archivo .vmx no soportada (%d).\n", version);
        fclose(arch);
        exit(1);
    }

    MV->memoria = (uint8_t *)calloc(MV->tam_memoria, sizeof(uint8_t));
    if (!MV->memoria) {
        printf("Error al reservar memoria para la MV.\n");
        fclose(arch);
        exit(1);
    }

    for (int i = 0; i < REGISTROS; i++) MV->registros[i] = 0;
    for (int i = 0; i < SEGMENTOS; i++) {
        MV->tablaSegmento[i].base = 0xFFFF;
        MV->tablaSegmento[i].tamano = 0xFFFF;
    }

    uint16_t tam_ps = 0, tam_ks = 0, tam_cs = 0, tam_ds = 0, tam_es = 0, tam_ss = 0;
    MV->entry_point = 0;

    if (version == 1) {
        uint8_t buf_cs[2];
        fread(buf_cs, sizeof(uint8_t), 2, arch);
        tam_cs = (buf_cs[0] << 8) | buf_cs[1];
        if (tam_cs > MV->tam_memoria) {
            printf("Error: Memoria insuficiente para alojar el proceso.\n");
            fclose(arch);
            exit(1);
        }
        tam_ds = MV->tam_memoria - tam_cs;

        MV->tablaSegmento[0].base = 0;
        MV->tablaSegmento[0].tamano = tam_cs;
        MV->tablaSegmento[1].base = tam_cs;
        MV->tablaSegmento[1].tamano = tam_ds;

        MV->registros[CS] = 0 << 16;
        MV->registros[DS] = 1 << 16;
        MV->registros[ES] = -1;
        MV->registros[SS] = -1;
        MV->registros[KS] = -1;
        MV->registros[PS] = -1;
        MV->registros[IP] = MV->registros[CS];

        fread(&MV->memoria[0], sizeof(uint8_t), tam_cs, arch);
    } 
    else if (version == 2) {
        uint8_t hdr_v2[12];
        fread(hdr_v2, sizeof(uint8_t), 12, arch);

        tam_cs = (hdr_v2[0] << 8) | hdr_v2[1];
        tam_ds = (hdr_v2[2] << 8) | hdr_v2[3];
        tam_es = (hdr_v2[4] << 8) | hdr_v2[5];
        tam_ss = (hdr_v2[6] << 8) | hdr_v2[7];
        tam_ks = (hdr_v2[8] << 8) | hdr_v2[9];
        MV->entry_point = (hdr_v2[10] << 8) | hdr_v2[11];

        if (cant_params > 0) {
            uint32_t bytes_strings = 0;
            for (int i = 0; i < cant_params; i++) {
                bytes_strings += (uint32_t)strlen(params[i]) + 1;
            }
            tam_ps = (uint16_t)(bytes_strings + (cant_params * 4));
        }

        uint32_t total_proceso = (uint32_t)tam_ps + tam_ks + tam_cs + tam_ds + tam_es + tam_ss;
        if (total_proceso > MV->tam_memoria) {
            printf("Error: Memoria insuficiente (requiere %u bytes, disponible %u bytes).\n",
                   total_proceso, MV->tam_memoria);
            fclose(arch);
            exit(1);
        }

        // Orden de los segmentos: PS, KS, CS, DS, ES, SS
        uint16_t tamanhos[6] = {tam_ps, tam_ks, tam_cs, tam_ds, tam_es, tam_ss};
        int regs_seg[6]      = {PS,     KS,     CS,     DS,     ES,     SS};
        uint16_t base_actual = 0;
        int idx_tabla = 0;

        for (int s = 0; s < 6; s++) {
            if (tamanhos[s] > 0) {
                MV->tablaSegmento[idx_tabla].base   = base_actual;
                MV->tablaSegmento[idx_tabla].tamano = tamanhos[s];
                MV->registros[regs_seg[s]]          = (idx_tabla << 16);
                base_actual += tamanhos[s];
                idx_tabla++;
            } else {
                MV->registros[regs_seg[s]] = -1;
            }
        }

        uint32_t offset_argv = 0xFFFFFFFF;
        if (tam_ps > 0) {
            uint16_t pos_actual = 0;
            uint16_t offsets_str[cant_params];

            for (int i = 0; i < cant_params; i++) {
                offsets_str[i] = pos_actual;
                size_t largo = strlen(params[i]) + 1;
                memcpy(&MV->memoria[pos_actual], params[i], largo);
                pos_actual += (uint16_t)largo;
            }
            offset_argv = (0 << 16) | pos_actual;
            for (int i = 0; i < cant_params; i++) {
                uint32_t puntero_str = (0 << 16) | offsets_str[i];
                escribir_mem32_be(MV, pos_actual, puntero_str);
                pos_actual += 4;
            }
        }

        if (tam_cs > 0) {
            uint16_t seg_cs = (MV->registros[CS] >> 16) & 0xFFFF;
            uint16_t base_cs = MV->tablaSegmento[seg_cs].base;
            fread(&MV->memoria[base_cs], sizeof(uint8_t), tam_cs, arch);
        }
        if (tam_ks > 0) {
            uint16_t seg_ks = (MV->registros[KS] >> 16) & 0xFFFF;
            uint16_t base_ks = MV->tablaSegmento[seg_ks].base;
            fread(&MV->memoria[base_ks], sizeof(uint8_t), tam_ks, arch);
        }

        MV->registros[IP] = MV->registros[CS] | (MV->entry_point & 0xFFFF);

        if (MV->registros[SS] != -1) {
            uint16_t seg_ss = (MV->registros[SS] >> 16) & 0xFFFF;
            uint16_t base_ss = MV->tablaSegmento[seg_ss].base;
            MV->registros[SP] = MV->registros[SS] | (tam_ss & 0xFFFF);

            if (tam_ss < 12) {
                printf("Error: Stack overflow al inicializar la subrutina principal.\n");
                fclose(arch);
                exit(1);
            }

            MV->registros[SP] -= 12;
            uint16_t offset_sp = MV->registros[SP] & 0xFFFF;
            uint32_t dir_fisica_sp = base_ss + offset_sp;

            escribir_mem32_be(MV, dir_fisica_sp,     0xFFFFFFFF);           // RET (-1)
            escribir_mem32_be(MV, dir_fisica_sp + 4, (uint32_t)cant_params);// argc
            escribir_mem32_be(MV, dir_fisica_sp + 8, offset_argv);          // *argv
        }
    }

    fclose(arch);
}

int existeOperacion(uint8_t ope) {
    return (ope <= 0x1F);
}

void leer_operando(TipoMV *MV, uint8_t tipo, int OP) {
    int32_t valor = 0; 
    
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
    while (MV->registros[IP] != -1) {
        uint16_t seg_cs = (MV->registros[IP] >> 16) & 0xFFFF;
        uint16_t offset_ip = MV->registros[IP] & 0xFFFF;
        
        if (seg_cs >= SEGMENTOS || MV->registros[CS] == -1 ||
            seg_cs != ((MV->registros[CS] >> 16) & 0xFFFF) ||
            offset_ip >= MV->tablaSegmento[seg_cs].tamano) {
            break;
        }

        int posmem = MV->tablaSegmento[seg_cs].base + offset_ip;
        uint8_t primer_byte = MV->memoria[posmem];
        uint8_t operacion = primer_byte & 0x1F;

        if (existeOperacion(operacion)) {
            MV->registros[OPC] = operacion;
            uint8_t tipo_op1 = 0;
            uint8_t tipo_op2 = 0;

            if (operacion >= 0x10) { 
                tipo_op1 = (primer_byte & 0x30) >> 4;
                tipo_op2 = (primer_byte & 0xC0) >> 6;
            } else if (operacion != 0x0E && operacion != 0x0F) {
                tipo_op1 = (primer_byte & 0xC0) >> 6;
            }

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

            int era_paso_a_paso = MV->paso_a_paso;

            if (instruccion[operacion] != NULL) {
                instruccion[operacion](MV);
            }

            // Si venía ejecutando paso a paso (Enter en un breakpoint) y la instrucción
            // actual no fue otro SYS F ni terminó el programa, disparamos el breakpoint
            if (era_paso_a_paso && MV->paso_a_paso && MV->registros[IP] != -1 &&
                !(operacion == 0x00 && (MV->registros[OP1] & 0xFFFF) == 0x0F)) {
                ejecutar_breakpoint(MV);
            }
        } else {
            printf("Error: Instruccion invalida (0x%02X) en [0x%04X]\n", operacion, posmem);
            MV->registros[IP] = -1; 
        }
    }
}

static int termina_con(const char *str, const char *sufijo) {
    size_t len_str = strlen(str);
    size_t len_suf = strlen(sufijo);
    if (len_suf > len_str) return 0;
    return strcmp(str + len_str - len_suf, sufijo) == 0;
}

int main(int argc, char *argv[]) {
    char *arch_vmx = NULL;
    char *arch_vmi = NULL;
    uint32_t mem_bytes = MEMORIA_DEFECTO;
    int flag_d = 0;
    int idx_p = -1;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-p") == 0) {
            idx_p = i;
            break;
        } else if (strcmp(argv[i], "-d") == 0) {
            flag_d = 1;
        } else if (strncmp(argv[i], "m=", 2) == 0 || strncmp(argv[i], "M=", 2) == 0) {
            int kib = atoi(argv[i] + 2);
            if (kib > 0) mem_bytes = (uint32_t)kib * 1024;
        } else if (termina_con(argv[i], ".vmx")) {
            arch_vmx = argv[i];
        } else if (termina_con(argv[i], ".vmi")) {
            arch_vmi = argv[i];
        }
    }

    if (arch_vmx == NULL && arch_vmi == NULL) {
        printf("Uso: %s [filename.vmx] [filename.vmi] [m=M] [-d] [-p param1 param2 ...]\n", argv[0]);
        return 1;
    }

    TipoMV MV;
    memset(&MV, 0, sizeof(TipoMV));
    MV.tam_memoria = mem_bytes;
    if (arch_vmi != NULL) {
        strncpy(MV.arch_vmi, arch_vmi, sizeof(MV.arch_vmi) - 1);
    }

    if (arch_vmx != NULL) {
        int cant_params = 0;
        char **lista_params = NULL;

        if (idx_p != -1 && (idx_p + 1) < argc) {
            int params_extra = argc - (idx_p + 1);
            cant_params = 1 + params_extra; // argv[0] = nombre .vmx + parametros
            lista_params = (char **)malloc(cant_params * sizeof(char *));
            lista_params[0] = arch_vmx;
            for (int j = 0; j < params_extra; j++) {
                lista_params[j + 1] = argv[idx_p + 1 + j];
            }
        }

        inicializacion_vmx(arch_vmx, &MV, cant_params, lista_params);
        if (lista_params) free(lista_params);
    } else {
        cargar_vmi(arch_vmi, &MV);
    }

    if (flag_d) {
        generar_disassembler(&MV);
    }

    ejecucion(&MV);

    free(MV.memoria);
    return 0;
}