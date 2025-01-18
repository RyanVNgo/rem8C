/*  @file   rem8C.h
 *  @brief  Function prototypes and defines for CHIP-8 emulator
 *  @author Ryan V. Ngo
 */

#ifndef REM8C_H
#define REM8C_H

#include <stdlib.h>
#include <stdint.h>

#define MAX_ADDR        0x0FFF
#define START_ADDR      0x0200

#define SCREEN_WIDTH  0x40
#define SCREEN_HEIGHT 0x20

typedef struct rem8C rem8C;

/******************** CHIP-8 Operations ********************/

void rem8C_update_timers(rem8C* cpu);
void rem8C_cycle(rem8C* cpu);
void rem8C_read_screen(rem8C* cpu, int X, int Y, void* buff, long size);
void rem8C_set_key(rem8C* cpu, uint8_t key);
void rem8C_unset_key(rem8C* cpu, uint8_t key);

/******************** CHIP-8 Configuration ********************/

void rem8C_set_start_addr(rem8C* cpu, uint16_t addr);
void rem8C_memset(rem8C* cpu, uint16_t addr, void* data, size_t size);

/******************** CHIP-8 Create/Destroy ********************/

rem8C* rem8C_new();
void rem8C_free(rem8C* cpu);

#endif
