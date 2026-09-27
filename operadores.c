#include "operadores.h"
#include "MaquinaVirtual.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "mascaras.h"
#include "OperacionesMem.c"

//Los 28 operaciones del ASSEMBLER
// firmas de los operadores

uint32_t obtenerDato(TipoMV *MV, int OP){
    uint32_t dato;
    uint8_t tipo;
    tipo = MV->registros[OP] >> 24; 
    dato = MV->registros[OP] & 0x00FFFFFF;//seteo el valor en 24 bits
    if (tipo == 1){ //registro
        dato = MV->registros[dato];
    }else
        if (tipo == 2) {//inmediato
            if ((dato >> 15) == 1 ){//negativo
                dato = 0xFFFF0000 | dato;
            }
        }else{
            TraigoDeMemoria(MV,OP);
            dato = MV->registros[MBR];    
        }
    return dato;
}

void actualizaCC(TipoMV *MV, int64_t res_con_signo, uint64_t res_sin_signo) {
    // Limpiamos solo los 4 bits más altos (NZCV), preservando los 28 bits reservados
    MV->registros[CC] &= 0x0FFFFFFF;

    // Recortamos el resultado a 32 bits reales para evaluar Signo y Cero
    int32_t res32 = (int32_t)res_con_signo;

    // Bit N (Signo): se activa cuando el resultado es negativo
    if (res32 < 0) {
        MV->registros[CC] |= (1 << 31);
    }

    // Bit Z (Cero): se activa cuando el resultado es cero
    if (res32 == 0) {
        MV->registros[CC] |= (1 << 30);
    }

    // Bit C (Acarreo): se activa cuando el resultado excede los 32 bits disponibles
    if (res_sin_signo > 0xFFFFFFFF) {
        MV->registros[CC] |= (1 << 29);
    }

    // Bit V (Desbordamiento): se activa cuando el resultado es erróneo por overflow con signo
    if (res_con_signo > 2147483647LL || res_con_signo < -2147483648LL) { //LL le decis a C que tome al numero como un long int (de 64bits)
        MV->registros[CC] |= (1 << 28);
    }
}

void guardarDato(TipoMV *MV,int OP ,int32_t dato){ 
   // 1. Extraemos el tipo desplazando 24 bits lógicos a la derecha
    uint8_t tipo = MV->registros[OP] >> 24;
    
    int32_t valor_bruto = MV->registros[OP] & 0x00FFFFFF;

    if (tipo == 1) { 
        // Es un Registro (1 byte)
        MV->registros[valor_bruto] = dato;
    } 
    else if (tipo == 3) { 
        MV->registros[MBR] = dato;
        lecturaDeMemoria(MV, OP);
        CargaAMemoria(MV);
    }
}

void MOV(TipoMV *MV){
    int32_t val_origen = obtenerDato(MV, OP2);
    guardarDato(MV, OP1, val_origen);

    int64_t res_con_signo = (int64_t)val_origen;
    uint64_t res_sin_signo = (uint64_t)val_origen & 0xFFFFFFFF; 
    
    actualizaCC(MV, res_con_signo, res_sin_signo);
}

void ADD(TipoMV *MV) {
    int32_t val_destino = obtenerDato(MV, OP1);
    int32_t val_origen = obtenerDato(MV, OP2);
    // Calculamos los resultados en 64 bits para evaluar desbordes
    
    // Resultado con signo tradicional
    int64_t res_con_signo = (int64_t)val_destino + (int64_t)val_origen;
    // Resultado sin signo usando máscaras AND
    uint64_t res_sin_signo = ((uint64_t)val_destino & 0xFFFFFFFF) + ((uint64_t)val_origen & 0xFFFFFFFF);
    
    guardarDato(MV, OP1, (int32_t)res_con_signo);//HAY QUE MODIFICARLO CON RESPECTO A LAS FUNCIONES DE operacionesMem.c
    // Actualizamos las banderas N, Z, C, y V
    actualizaCC(MV, res_con_signo, res_sin_signo);
}

void SUB(TipoMV *MV){
    int32_t val_destino = obtenerDato(MV, OP1);
    int32_t val_origen = obtenerDato(MV, OP2);

    int64_t res_con_signo = (int64_t)val_destino - (int64_t)val_origen;
    uint64_t res_sin_signo = ((uint64_t)val_destino & 0xFFFFFFFF) - ((uint64_t)val_origen & 0xFFFFFFFF);

    guardarDato(MV, OP1, (int32_t)res_con_signo);
    actualizaCC(MV, res_con_signo, res_sin_signo);
}

void MUL(TipoMV *MV){
    int32_t val_destino = obtenerDato(MV, OP1);
    int32_t val_origen = obtenerDato(MV, OP2);

    int64_t res_con_signo = (int64_t)val_destino * (int64_t)val_origen;
    uint64_t res_sin_signo = ((uint64_t)val_destino & 0xFFFFFFFF) * ((uint64_t)val_origen & 0xFFFFFFFF);

    guardarDato(MV, OP1, (int32_t)res_con_signo);
    actualizaCC(MV, res_con_signo, res_sin_signo);
}

void DIV(TipoMV *MV){
    int32_t val_destino = obtenerDato(MV, OP1);
    int32_t val_origen = obtenerDato(MV, OP2);

    if (val_origen == 0) {
        printf("ERROR: Division por cero.\n");
        MV->registros[IP] = 0xFFFFFFFF; 
        return; 
    }

    int64_t res_con_signo = (int64_t)val_destino / (int64_t)val_origen;
    uint64_t res_sin_signo = ((uint64_t)val_destino & 0xFFFFFFFF) / ((uint64_t)val_origen & 0xFFFFFFFF);
    
    guardarDato(MV, OP1, (int32_t)res_con_signo);
     // El DIV guarda obligatoriamente el resto en el registro AC
    MV->registros[AC] = val_destino % val_origen;
    actualizaCC(MV, res_con_signo, res_sin_signo);
}

void CMP(TipoMV *MV){
    //Igual que sub pero sin guardar el dato
    int32_t val_destino = obtenerDato(MV, OP1);
    int32_t val_origen = obtenerDato(MV, OP2);

    int64_t res_con_signo = (int64_t)val_destino - (int64_t)val_origen;
    uint64_t res_sin_signo = ((uint64_t)val_destino & 0xFFFFFFFF) - ((uint64_t)val_origen & 0xFFFFFFFF);
    
    actualizaCC(MV, res_con_signo, res_sin_signo);
}

void AND(TipoMV *MV){
    int32_t val_destino = obtenerDato(MV, OP1);
    int32_t val_origen = obtenerDato(MV, OP2);

    int64_t res_con_signo = (int64_t)val_destino & (int64_t)val_origen;
    uint64_t res_sin_signo = ((uint64_t)val_destino & 0xFFFFFFFF) & ((uint64_t)val_origen & 0xFFFFFFFF);

    guardarDato(MV, OP1, (int32_t)res_con_signo);
    actualizaCC(MV, res_con_signo, res_sin_signo);
}

void OR(TipoMV *MV){
    int32_t val_destino = obtenerDato(MV, OP1);
    int32_t val_origen = obtenerDato(MV, OP2);

    int64_t res_con_signo = (int64_t)val_destino | (int64_t)val_origen;
    uint64_t res_sin_signo = ((uint64_t)val_destino & 0xFFFFFFFF) | ((uint64_t)val_origen & 0xFFFFFFFF);

    guardarDato(MV, OP1, (int32_t)res_con_signo);
    actualizaCC(MV, res_con_signo, res_sin_signo);
}

void XOR(TipoMV *MV){
    int32_t val_destino = obtenerDato(MV, OP1);
    int32_t val_origen = obtenerDato(MV, OP2);

    int64_t res_con_signo = (int64_t)val_destino ^ (int64_t)val_origen;
    uint64_t res_sin_signo = ((uint64_t)val_destino & 0xFFFFFFFF) ^ ((uint64_t)val_origen & 0xFFFFFFFF);

    guardarDato(MV, OP1, (int32_t)res_con_signo);
    actualizaCC(MV, res_con_signo, res_sin_signo);
}

void SWAP(TipoMV *MV){
    int32_t val_destino = obtenerDato(MV, OP1);
    int32_t val_origen = obtenerDato(MV, OP2);

    // Intercambio cruzado hay que modificar estas funciones
    guardarDato(MV,OP1,val_origen);
    guardarDato(MV, OP2, val_destino);

    // Afecta a CC simulando un XOR entre ambos pero no le cambio el valor 
    int64_t res_con_signo = (int64_t)val_destino ^ (int64_t)val_origen;
    uint64_t res_sin_signo = ((uint64_t)val_destino & 0xFFFFFFFF) ^ ((uint64_t)val_origen & 0xFFFFFFFF);
    actualizaCC(MV, res_con_signo, res_sin_signo);
}

void SHL(TipoMV *MV){
    int32_t val_destino = obtenerDato(MV, OP1);
    int32_t val_origen = obtenerDato(MV, OP2);

    int64_t res_con_signo = (int64_t)val_destino << val_origen;
    uint64_t res_sin_signo = ((uint64_t)val_destino & 0xFFFFFFFF) << val_origen;

    guardarDato(MV, OP1, (int32_t)res_con_signo);
    actualizaCC(MV, res_con_signo, res_sin_signo);
}

void SHR(TipoMV *MV){
    uint32_t val_destino = (uint32_t)obtenerDato(MV, OP1);  //seteo asi el compilador hace el corrimiento sin desplazar el signo;
    int32_t val_origen = obtenerDato(MV, OP2);

    int64_t res_con_signo = (int64_t)(val_destino >> val_origen);
    uint64_t res_sin_signo = ((uint64_t)val_destino & 0xFFFFFFFF) >> val_origen;


    guardarDato(MV, OP1, (int32_t)res_con_signo);
    actualizaCC(MV, res_con_signo, res_sin_signo);
}

void SAR(TipoMV *MV){
    // Mantenemos el signo (int32_t) para que C propague el bit negativo
    int32_t val_destino = obtenerDato(MV, OP1); 
    int32_t val_origen = obtenerDato(MV, OP2);

    int64_t res_con_signo = (int64_t)(val_destino >> val_origen);
    uint64_t res_sin_signo = ((uint64_t)val_destino & 0xFFFFFFFF) >> val_origen;

    guardarDato(MV, OP1, (int32_t)res_con_signo); 
    actualizaCC(MV, res_con_signo, res_sin_signo);
}

void LDL(TipoMV *MV){
    int32_t val_destino = obtenerDato(MV, OP1);
    int32_t val_origen = obtenerDato(MV, OP2);

    val_destino &= 0xFFFF0000;

    int32_t mitad_baja_origen = val_origen & 0x0000FFFF;

    int32_t resultado = val_destino | mitad_baja_origen;

    guardarDato(MV, OP1, resultado);
    actualizaCC(MV, (int64_t)resultado, (uint64_t)resultado & 0xFFFFFFFF);
}

void LDH(TipoMV *MV){
    int32_t val_destino = obtenerDato(MV, OP1);
    int32_t val_origen = obtenerDato(MV, OP2);

    val_destino &= 0x0000FFFF;

    int32_t mitad_alta_origen = (val_origen & 0x0000FFFF) << 16;

    int32_t resultado = val_destino | mitad_alta_origen;

    guardarDato(MV, OP1, resultado);
    actualizaCC(MV, (int64_t)resultado, (uint64_t)resultado & 0xFFFFFFFF);
}

void RND(TipoMV *MV){
    int32_t limite = obtenerDato(MV, OP2);

    int32_t resultado = rand() % (limite + 1);

    guardarDato(MV, OP1, resultado);
    actualizaCC(MV, (int64_t)resultado, (uint64_t)resultado & 0xFFFFFFFF);
}

//Funciones auxiliares para modularizar SYS
//##############################################################################
void imprimir_segun_formato(uint32_t dato, uint32_t formato, uint16_t tamano) {
    if (formato & 0x01) printf("%d ", dato); // Decimal
    
    if (formato & 0x02) { //ASCII
        if (dato >= 32 && dato <= 126) printf("%c ", dato);
        else printf(". "); // Si no es imprimible, escribe un punto
    }
    
    if (formato & 0x04) printf("0o%o ", dato); // Octal
    
    if (formato & 0x08) printf("0x%X ", dato); // Hexadecimal
    
    if (formato & 0x10) { // Binario
        printf("0b");
        for (int b = (tamano * 8) - 1; b >= 0; b--) {
            printf("%d", (dato >> b) & 1);
        }
        printf(" ");
    }
    printf("\n");
}

void sys_write(TipoMV *MV, uint16_t cantidad, uint16_t tamano, uint32_t dir_fisica, uint32_t formato) {
    for (int i = 0; i < cantidad; i++) {
        // Imprimimos el prompt de la direccion fisica del dato
        printf("[%04X]: ", dir_fisica);
        // Extraer de memoria ensamblando los bytes según el tamaño
        uint32_t dato = 0;
        for (int b = 0; b < tamano; b++) {
            dato = (dato << 8) | MV->memoria[dir_fisica + b]; 
        }
        imprimir_segun_formato(dato, formato, tamano);
        
        // Avanzamos a la siguiente celda dependiendo del tamaño del valor
        dir_fisica += tamano; 
    }
}

void sys_read(TipoMV *MV, uint16_t cantidad, uint16_t tamano, uint32_t dir_fisica, uint32_t formato) {
    for (int i = 0; i < cantidad; i++) {
        printf("[%04X]: ", dir_fisica); 

        uint32_t dato_ingresado = 0;
        
        if (formato & 0x01) { 
            // Decimal
            scanf("%d", &dato_ingresado);
        } 
        else if (formato & 0x02) { 
            // Carácter ASCII
            char c;
            scanf(" %c", &c);
            dato_ingresado = c;
        } 
        else if (formato & 0x04) { 
            // Octal
            scanf("%o", &dato_ingresado);
        } 
        else if (formato & 0x08) { 
            // Hexadecimal[cite: 7]
            scanf("%x", &dato_ingresado);
        } 
        else if (formato & 0x10) { 
            // Binario
            // Como scanf no soporta formato binario nativamente en C estándar, 
            // capturamos el string y usamos strtol para la conversión a numero binario.
            char strbinario[33];
            scanf("%32s", strbinario);
            
            char *ptr = strbinario;
            // Si el usuario incluye el prefijo '0b' o '0B', lo ignoramos adelantando el puntero
            if (strbinario[0] == '0' && (strbinario[1] == 'b' || strbinario[1] == 'B')) {
                ptr += 2; 
            }
            // strtol convierte la cadena en base 2 a un número entero
            dato_ingresado = (uint32_t)strtol(ptr, NULL, 2);
        }


        for (int b = tamano - 1; b >= 0; b--) {
            MV->memoria[dir_fisica + b] = dato_ingresado & 0xFF;
            dato_ingresado >>= 8;
        }

        dir_fisica += tamano;
    }
}
//##############################################################################
void SYS(TipoMV *MV){
    // Averiguamos si es READ (1) o WRITE (2)[cite: 7]
    int32_t tipo_llamada = obtenerDato(MV, OP1);

    // ECX indica la cantidad en los 2 bytes bajos y el tamaño en los 2 bytes altos
    uint16_t cantidad = MV->registros[ECX] & 0xFFFF;
    uint16_t tamano = (MV->registros[ECX] >> 16) & 0xFFFF;

    // EDX apunta a la memoria inicial[cite: 7]

    int16_t offset = MV->registros[EDX] & 0xFFFF;
    uint8_t reg_base = (MV->registros[EDX] >> 16) & 0x1F;
    uint16_t dir_fisica = DirecLogica(MV, offset, reg_base);

    if (tipo_llamada == 1) { 
        sys_read(MV, cantidad, tamano, dir_fisica,  MV->registros[EAX]);
    } 
    else if (tipo_llamada == 2) { 
        sys_write(MV, cantidad, tamano, dir_fisica,  MV->registros[EAX]);
    }
    //  MV->registros[EAX] = FORMATO
}

void JMP(TipoMV *MV){
    MV->registros[IP] = obtenerDato(MV, OP1);
}

void JP(TipoMV *MV){
    // Aislamos N (bit 31) y Z (bit 30)
    int n = (MV->registros[CC] >> 31) & 1;
    int z = (MV->registros[CC] >> 30) & 1;

    if (n == 0 && z == 0) {
        MV->registros[IP] = obtenerDato(MV, OP1);
    }
}

void JN(TipoMV *MV){
    int n = (MV->registros[CC] >> 31) & 1;
    if (n == 1) {
        MV->registros[IP] = obtenerDato(MV, OP1);
    }
}

void JZ(TipoMV *MV){
    int z = (MV->registros[CC] >> 30) & 1;
    // La condición exige que Z sea igual a 1 (resultado anterior == 0)
    if (z == 1) {
        MV->registros[IP] = obtenerDato(MV, OP1);
    }
}

void JC(TipoMV *MV){
    int c = (MV->registros[CC] >> 29) & 1;
    
    if (c == 1) {
        MV->registros[IP] = obtenerDato(MV, OP1);
    }
}

void JV(TipoMV *MV){
     int v = (MV->registros[CC] >> 28) & 1;
    
    if (v == 1) {
        MV->registros[IP] = obtenerDato(MV, OP1);
    }
}

void JNP(TipoMV *MV){
    int n = (MV->registros[CC] >> 31) & 1;
    int z = (MV->registros[CC] >> 30) & 1;

    if (n == 1 || z == 1) {
        MV->registros[IP] = obtenerDato(MV, OP1);
    }
}

void JNN(TipoMV *MV){
    int n = (MV->registros[CC] >> 31) & 1;
    if (n == 0) {
        MV->registros[IP] = obtenerDato(MV, OP1);
    }
}

void JNZ(TipoMV *MV){
    int z = (MV->registros[CC] >> 30) & 1;
    if (z == 0) {
        MV->registros[IP] = obtenerDato(MV, OP1);
    }
}

void NOT(TipoMV *MV){

    int32_t val = obtenerDato(MV, OP1);

    int32_t res = ~val;

    guardarDato(MV, OP1, res);

    actualizaCC(MV, (int64_t)res, (uint64_t)res & 0xFFFFFFFF);
}

void STOP(TipoMV *MV){
    MV->registros[IP]=-1;
}

