#ifndef DISASSEMBLER_H
#define DISASSEMBLER_H

#include "operadores.h" // Para conocer la estructura TipoMV

// Vector de Mnemónicos (el índice coincide con el código de operación del 0 al 31)
extern const char* mnemonicos[32];

// Vector de Registros (el índice coincide con la tabla de registros de la MV)
extern const char* nom_registros[32];

// Función principal que genera el archivo txt
void generar_disassembler(TipoMV *MV, int tam_codigo);

#endif