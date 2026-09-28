#pragma once

#include <stdint.h>
#include "MaquinaVirtual.h"

uint16_t DirecLogica(TipoMV *MV, int32_t corrimiento, int8_t reg);
void lecturaDeMemoria(TipoMV *MV, int OP);
void CargaAMemoria(TipoMV *MV, int OP);
void TraigoDeMemoria(TipoMV *MV, int OP);


