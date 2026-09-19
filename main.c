// #define REGISTROS 32    //cantidad de archivos
// #define MEMORIA 16384   //espacio de memoria arreglo continuo de 16 KiB = 16384 bytes
// #define SEGMENTOS 8     // tabla de segmentos 8 entradas de 32 bits

// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>

// //Indices de los registros
//     // IP = 0, OPC = 1, OP1 = 2, OP2 = 3, LAR = 4, MAR = 5, MBR = 6,
//     // EAX = 10, EBX = 11, ECX = 12, EDX = 13, EEX = 14, EFX = 15,
//     // AC = 16, CC = 17, CS = 26, DS = 27

// // La tabla de descriptores de segmentos permite definir la ubicación y el tamaño de cada segmento del
// // proceso en la memoria principal. Consta de 8 entradas de 32 bits, cada una se divide en dos partes: los
// // primeros 2 bytes son para guardar la dirección física de comienzo del segmento (base) y los siguientes 2
// // bytes la cantidad de bytes que ocupa. Se inicializa en el momento de la carga del programa.


// // Estructura para cada entrada de la tabla de segmentos (4 bytes en total)
// //para mi la tabla segmentos es asi porque cada unsigned short tiene 2 bytes y vos necesitas 2 bytes para la base y 2 para el tamaño

// // typedef struct {
// //     unsigned short base;   // 2 bytes para la dirección física
// //     unsigned short tamano; // 2 bytes para el tamaño
// // } DescriptorSegmento;


// typedef struct{
//     unsigned int registros[REGISTROS]; // --> IP OPC OP1 OP2 LAR MAR MBR EAX .. EFX AC CC CS DS 
//     unsigned char mem[MEMORIA];//cambio de tipo a char porq char ocupa 1 byte
//     unsigned short tablaSegmento[SEGMENTOS];
//     unsigned int saltoError;
// }TipoMV;
 
// int main(){

//     return 0;
// }

// int verifica_cabecera(unsigned char cabecera[5]){ //Verifica la version de la cabecera
//     int i=0;
//     unsigned char comp[5]={'V','M','X','2','6','1'};
//     while (i<6 && comp[i]==cabecera[i])
//         i++;
//     if (i==6)
//         return 1; //Version correcta
//     else
//         return 0; //Version erronea 
// }

// void inicializacion(char nombre_arch[],TipoMV *MV){
//     FILE *arch;
//     int i = 0, desp = 0; 
//     unsigned char cabecera[6]; //aca leo los primeros 5 bytes 
//     MV->codigo_error=0;
//     arch= fopen(nombre_arch, "rb");  //apertura del archivo .asm --> no seria el vmx ?
//         if (arch == NULL)
//             printf("Error al abrir el archivo");
//         else{
//             for (int j=0;i<5;i++)  //lectura de la cabecera
//                 fread(&cabecera[j], sizeof(unsigned char)/*lee un byte*/, 1, arch);
//             fread(&cabecera[5], sizeof(unsigned char), 1, arch);
//             //valido cabecera 
//             if (verifica_cabecera(cabecera)){ 
//                 fread(&MV->tablaSegmento[0],sizeof(unsigned char),1,arch);         // base CS
//                 fread(MV.tablaSegmento[1], sizeof(unsigned char), 1, arch);        // tamano max CS
//                 //seteo tabla segmento con tam de cs y DS
//                 MV->tablaSegmento[2]=MV->tablaSegmento[1];                     //base DS            
//                 MV->tablaSegmento[3]=16384-MV->tablaSegmento[2];                 //tamano max DS
//                 //podriamos verificar con un print si seteo bien el tama;o :
//                  while (fread(&MV.mem[MV->reg[CS]+desp], sizeof(unsigned char), 1, arch) == 1)  //Guarda las instrucciones en el code segment
//                      desp++;    
//                  fclose(arch);
//             } 
//             else
//                 printf("Codigo de cabecera incorrecto");
//         }
// }

// void ejecucion(){
//     inicializacion(,MV);
    
// }    
