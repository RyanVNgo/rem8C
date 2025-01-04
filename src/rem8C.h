/*  @file   rem8C.h
 *  @brief  Function prototypes and defines for CHIP-8 emulator
 *  @author Ryan V. Ngo
 */

#ifndef REM8C_H
#define REM8C_H

#include <stdlib.h>

#define START_ADDR  0x0200

typedef struct rem8C rem8C;

/******************** Cpu Operations ********************/

void rem8C_cycle(rem8C* cpu);
void rem8C_print_screen(rem8C* cpu);
void rem8C_memset(rem8C* cpu, unsigned short addr, void* data, size_t size);

/******************** Cpu Debug ********************/

unsigned char rem8C_data_reg(rem8C* cpu, unsigned char reg);

/******************** Create/Destroy Cpu ********************/

rem8C* rem8C_new();
void rem8C_free(rem8C* cpu);

#endif
