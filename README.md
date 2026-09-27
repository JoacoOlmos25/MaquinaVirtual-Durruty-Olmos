# MaquinaVirtual-Durruty-Olmos

testFunAritmeticas.asm = tendria que devolver "[000A]: 25" 
{
 joaco@MacBook-Air-de-Joaco MaquinaVirtual % ./testeoCargaMemoria prueba.vmx
Cabecera VMX26 y version correctas.
Carga exitosa: 39 bytes copiados a memoria.

--- Volcado de Memoria (Codigo Cargado) ---
90 00 0F 0A 90 00 0A 0B 51 0B 0A 50 1B 0D 91 00 
0A 0D 70 0A 00 00 8D 90 00 01 0A 9E 00 04 0C 9D 
00 01 0C 80 00 02 0F 
-------------------------------------------

--- Iniciando Ejecucion ---
[0031]: 25 

--- Ejecucion Finalizada ---   
}

testDecrementar.asm = ingreso un num por teclado y va printieandolo mientras lo decrementa:
{
    joaco@MacBook-Air-de-Joaco MaquinaVirtual % ./testeoCargaMemoria prueba.vmx;     
Cabecera VMX26 y version correctas.
Carga exitosa: 58 bytes copiados a memoria.

--- Volcado de Memoria (Codigo Cargado) ---
50 1B 0D 90 00 01 0A 9E 00 04 0C 9D 00 01 0C 80 
00 01 D0 00 00 8D 0B 95 00 00 0B 84 00 39 70 0B 
00 00 8D 90 00 01 0A 9E 00 04 0C 9D 00 01 0C 80 
00 02 92 00 01 0B 81 00 17 0F 
-------------------------------------------

--- Iniciando Ejecucion ---
[003A]: 3
[003A]: 3 
[003A]: 2 
[003A]: 1 

--- Ejecucion Finalizada ---
}

prueba.asm = contar la cantidad de bits en 1 que conforman a un número ingresado por el usuario
{
 joaco@MacBook-Air-de-Joaco MaquinaVirtual % ./testeoCargaMemoria prueba.vmx;
Cabecera VMX26 y version correctas.
Carga exitosa: 76 bytes copiados a memoria.

--- Volcado de Memoria (Codigo Cargado) ---
90 00 01 0A 50 1B 0D 91 00 04 0D 9E 00 04 0C 9D 
00 01 0C 80 00 01 58 10 10 D0 00 00 8D 0A 95 00 
00 0A 84 00 33 88 00 2C 91 00 01 10 9A 00 01 0A 
81 00 1E 91 00 04 0D 70 10 00 00 8D 90 00 01 0A 
9E 00 04 0C 9D 00 01 0C 80 00 02 0F 
-------------------------------------------

--- Iniciando Ejecucion ---
[0050]: 13
[0054]: 3 

--- Ejecucion Finalizada ---   
}