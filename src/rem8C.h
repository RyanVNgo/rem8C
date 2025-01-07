/*  @file   rem8C.h
 *  @brief  Function prototypes and defines for CHIP-8 emulator
 *  @author Ryan V. Ngo
 */

#ifndef REM8C_H
#define REM8C_H

#include <stdlib.h>

#define MAX_ADDR    0x0FFF
#define START_ADDR  0x0200

#define SCREEN_WIDTH  0x40
#define SCREEN_HEIGHT 0x20

typedef struct rem8C rem8C;

/******************** Cpu Operations ********************/

void rem8C_cycle(rem8C* cpu);
void rem8C_update_timers(rem8C* cpu);
void rem8C_read_screen(rem8C* cpu, int X, int Y, void* buff, long size);
void rem8C_memset(rem8C* cpu, unsigned short addr, void* data, size_t size);
void rem8C_set_key(rem8C* cpu, unsigned char key);
void rem8C_unset_key(rem8C* cpu, unsigned char key);

/******************** Create/Destroy Cpu ********************/

rem8C* rem8C_new();
void rem8C_free(rem8C* cpu);

#endif
