#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "operadores.h"
#include "mascaras.h"
#include "MaquinaVirtual.h"
#include "disassembler.h"

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

void inicializacion(char nombre_arch[], TipoMV *MV,uint16_t *tam_codigo) {
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
        *tam_codigo = (buffer_tam[0] << 8) | buffer_tam[1];

        // Configuramos la tabla de segmentos (sigue igual)
        MV->tablaSegmento[0].base = 0;
        MV->tablaSegmento[0].tamano = *tam_codigo;
        MV->tablaSegmento[1].base = *tam_codigo;                 // El inicio del seg datos == fin del seg codigo
        MV->tablaSegmento[1].tamano = MEMORIA - *tam_codigo; 

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
        MV->registros[0] = MV->registros[CS];

        // Guardamos las instrucciones en el code segment de la memoria
        int desp = 0;
        while (fread(&MV->memoria[MV->registros[CS] + desp], sizeof(uint8_t), 1, arch) == 1) {
            desp++;    
        }
        printf("Carga exitosa: %d bytes copiados a memoria.\n", desp);


        // Volcado de prueba en consola
        printf("\n--- Volcado de Memoria (Codigo Cargado) ---\n");
        
        // Iteramos exclusivamente sobre el tamaño del código que nos dijo la cabecera
        for (int i = 0; i < *tam_codigo; i++) {
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
    uint16_t tam_codigo;
    // Llamamos a la función pasando el archivo binario generado
    inicializacion("prueba.vmx", &MV,&tam_codigo);
    generar_disassembler(&MV,tam_codigo);
    
    return 0;
}

int existeOperacion(uint8_t ope){
    return (ope<0x1F) && !(ope >= 0x0B && ope <= 0x0E ) ; //agregue que si el cod de operaicon cae en uno de los null devuelva 0 tambien
}

void leer_operando(TipoMV *MV, uint8_t tipo, int *posmem, int OP) {
    int32_t valor = 0; 
    
    switch (tipo) {//avanza el IP dependiendo el tamano del operando
        case 0: // Ninguno (0 bytes)
            // No hacemos nada, valor queda en 0 y el IP no avanza
            break;
            
        case 1: // Registro (1 byte)
            valor = MV->memoria[*posmem];
            (*posmem) += 1;
            break;
            
        case 2:{ // Inmediato (2 bytes)
            // Leemos 2 bytes y los guardamos en una variable de 16 bits con signo (int16_t)
            // Esto es vital para que C maneje automáticamente los números negativos (extensión de signo)
            int16_t inmediato = (MV->memoria[*posmem] << 8) | MV->memoria[*posmem + 1];
            valor = inmediato; 
            (*posmem) += 2;
            break;
            }   
        case 3: // Memoria (3 bytes)
            // Según el documento, se compone de 16 bits de desplazamiento y 5 bits de registro.
            // Unimos los 3 bytes desplazándolos a sus posiciones relativas para armar un bloque de 24 bits
            valor = (MV->memoria[*posmem] << 16) | 
                    (MV->memoria[*posmem + 1] << 8) | 
                    MV->memoria[*posmem + 2];
            (*posmem) += 3;
            break;
    }
    
    MV->registros[OP]= (tipo << 24) | (valor & 0x00FFFFFF); //0000 0001(tipo) 0000 0000 0000 0000 0000 0000(valor)
    // Armamos el formato exigido: tipo en el byte más significativo, valor en los 3 menos significativos
    // La máscara 0x00FFFFFF limpia cualquier basura o extensión de signo que haya quedado en el byte alto
    //devuelvo el operando codificado en 32 bits guardado en el registro correspondiente
}

void ejecucion(TipoMV *MV) {
    int posmem=MV->registros[CS];
    MV->registros[IP] = MV->memoria[posmem];
    posmem++;

    // El ciclo corta si IP toma el valor -1 (0xFFFFFFFF) por un STOP o error
    while (MV->registros[IP] != 0xFFFFFFFF) {
        
        // Leemos el primer byte y extraemos máscaras
        //uint8_t primer_byte = MV->memoria[MV->registros[IP]];
        uint8_t operacion = MV->registros[IP] & Masc_CodO;
        uint8_t tipo_op1 = (MV->registros[IP] & Masc_OP1) >> 4;
        uint8_t tipo_op2 = (MV->registros[IP] & Masc_OP2) >> 6; 
        //modifique primer_byte por MV->registros[IP]

        if (existeOperacion(operacion)){ // agregue validacion de codigo;

            //1. Guardamos el código de operación como pide el apunte
            MV->registros[OPC] = operacion;

            //2. utilizamos funcion aux para cargar los operandos
            leer_operando(MV, tipo_op1, &posmem, OP1);
            leer_operando(MV, tipo_op2, &posmem, OP2);
            
            //3. Hago que el registro IP apunte a la siguiente instruccion
            MV->registros[IP] = MV->memoria[posmem];
            posmem++;

            // 4. Ejecutamos la instrucción matematicamente
            if (instruccion[operacion] != NULL) {
                instruccion[operacion](MV);
            } else {
                // Manejo de error: Instrucción Inválid
            }
        }
    }
}