/*  @file   rem8C.c
 *  @brief  Function definitions for CHIP-8 emulator
 *  @author Ryan V. Ngo
 */

#include "rem8C.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/******************** CPU & Internal ********************/

#define SCREEN_WIDTH  0x40
#define SCREEN_HEIGHT 0x20
#define KEY_DEFAULT   0xFF

typedef struct rem8C {
  unsigned char data_reg[16];
  unsigned short addr_reg;
  unsigned short stack_pointer;
  unsigned char delay_timer;
  unsigned char sound_timer;
  unsigned short pc;
  unsigned short sprite_addr;
  unsigned char key;
  unsigned char screen[SCREEN_WIDTH][SCREEN_HEIGHT];
  unsigned char memory[0xFFFF];
} rem8C;

#define SPRITE_WIDTH  5

void _rem8C_push_pc_to_stack(rem8C* cpu) {
  cpu->memory[cpu->stack_pointer] = cpu->pc & 0xFF;
  cpu->stack_pointer--;
  cpu->memory[cpu->stack_pointer] = (cpu->pc >> 8) & 0xFF;
  cpu->stack_pointer--;
}

void _rem8C_pull_pc_from_stack(rem8C* cpu) {
  cpu->stack_pointer++;
  unsigned char msb = cpu->memory[cpu->stack_pointer];
  cpu->stack_pointer++;
  unsigned char lsb = cpu->memory[cpu->stack_pointer];
  cpu->pc = (msb << 8) | lsb;
}

void _rem8C_sprite_set(rem8C* cpu, unsigned short loc) {
  unsigned char sprite_data[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, /* 0 */
    0x20, 0x60, 0x20, 0x20, 0x70, /* 1 */
    0xF0, 0x10, 0xF0, 0x80, 0xF0, /* 2 */
    0xF0, 0x10, 0xF0, 0x10, 0xF0, /* 3 */
    0x90, 0x90, 0xF0, 0x10, 0x10, /* 4 */
    0xF0, 0x80, 0xF0, 0x10, 0xF0, /* 5 */
    0xF0, 0x80, 0xF0, 0x90, 0xF0, /* 6 */
    0xF0, 0x10, 0x20, 0x40, 0x40, /* 7 */
    0xF0, 0x90, 0xF0, 0x90, 0xF0, /* 8 */
    0xF0, 0x90, 0xF0, 0x10, 0xF0, /* 9 */
    0xF0, 0x90, 0xF0, 0x90, 0x90, /* A */
    0xE0, 0x90, 0xE0, 0x90, 0xE0, /* B */
    0xF0, 0x80, 0x80, 0x80, 0xF0, /* C */
    0xE0, 0x90, 0x90, 0x90, 0xE0, /* D */
    0xF0, 0x80, 0xF0, 0x80, 0xF0, /* E */
    0xF0, 0x80, 0xF0, 0x80, 0x80  /* F */
  };
  memcpy(&cpu->memory[loc], sprite_data, sizeof(sprite_data));
}

void _rem8C_sprite_draw(rem8C* cpu, char X, char Y, unsigned char sprite) {
  X %= SCREEN_WIDTH;
  Y %= SCREEN_HEIGHT;
  int i;
  for (i = 0; i < 8; i++) {
    cpu->screen[X+i][Y] ^= (sprite >> (7 - i)) & 0x01;
  }
}

void _rem8C_screen_clear(rem8C* cpu) {
  memset(cpu->screen, 0x00, SCREEN_WIDTH * SCREEN_HEIGHT);
}

unsigned char _msb_reg_idx(unsigned char msb) {
  return msb & 0x0F;
}

unsigned char _lsb_reg_idx(unsigned char lsb) {
  return (lsb >> 4) & 0x0F;
}

/******************** Instructions ********************/

#define INSTR_SIZE  2

/* Execute machine language subroutine at address NNN */
void _instr_0NNN(rem8C* cpu) {
  cpu->pc += INSTR_SIZE;
}

/* Clear the screen */
void _instr_00E0(rem8C* cpu) {
  _rem8C_screen_clear(cpu);
  cpu->pc += INSTR_SIZE;
}

/* Return from a subroutine */
void _instr_00EE(rem8C* cpu) {
  _rem8C_pull_pc_from_stack(cpu);
  cpu->pc += INSTR_SIZE;
}

/* Jump to address NNN */
void _instr_1NNN(rem8C* cpu) {
  unsigned char msb = cpu->memory[cpu->pc++];
  unsigned char lsb = cpu->memory[cpu->pc++];
  cpu->pc = ((msb & 0x0F) << 8) | lsb;
}

/* Execute subroutine starting at address NNN */
void _instr_2NNN(rem8C* cpu) {
  unsigned char msb = cpu->memory[cpu->pc++];
  unsigned char lsb = cpu->memory[cpu->pc++];
  _rem8C_push_pc_to_stack(cpu);
  cpu->pc = ((msb & 0x0F) << 8) | lsb;
}

/* Skip following instruction if VX == NN */
void _instr_3XNN(rem8C* cpu) {
  unsigned char X = _msb_reg_idx(cpu->memory[cpu->pc++]);
  unsigned char lsb = cpu->memory[cpu->pc++];
  if (cpu->data_reg[X] == lsb) cpu->pc += INSTR_SIZE;
}

/* Skip following instruction if VX != NN */
void _instr_4XNN(rem8C* cpu) {
  unsigned char X = _msb_reg_idx(cpu->memory[cpu->pc++]);
  unsigned char lsb = cpu->memory[cpu->pc++];
  if (cpu->data_reg[X] != lsb) cpu->pc += INSTR_SIZE;
}

/* Skip following instruction if VX == VY*/
void _instr_5XY0(rem8C* cpu) {
  unsigned char X = _msb_reg_idx(cpu->memory[cpu->pc++]);
  unsigned char Y = _lsb_reg_idx(cpu->memory[cpu->pc++]);
  if (cpu->data_reg[X] == cpu->data_reg[Y]) cpu->pc += INSTR_SIZE;
}

/* Store value NN in VX */
void _instr_6XNN(rem8C* cpu) {
  unsigned char X = _msb_reg_idx(cpu->memory[cpu->pc++]);
  unsigned char lsb = cpu->memory[cpu->pc++];
  cpu->data_reg[X] = lsb;
}

/* Add value NN to VX */
void _instr_7XNN(rem8C* cpu) {
  unsigned char msb = cpu->memory[cpu->pc++];
  unsigned char lsb = cpu->memory[cpu->pc++];
  cpu->data_reg[msb & 0x0F] += lsb;
}

/* Store the value of VY in VX */
void _instr_8XY0(rem8C* cpu) {
  unsigned char X = _msb_reg_idx(cpu->memory[cpu->pc++]);
  unsigned char Y = _lsb_reg_idx(cpu->memory[cpu->pc++]);
  cpu->data_reg[X] = cpu->data_reg[Y];
}

/* Set VX to VX | VY */
void _instr_8XY1(rem8C* cpu) {
  unsigned char X = _msb_reg_idx(cpu->memory[cpu->pc++]);
  unsigned char Y = _lsb_reg_idx(cpu->memory[cpu->pc++]);
  cpu->data_reg[X] |= cpu->data_reg[Y];
}

/* Set VX to VX & VY */
void _instr_8XY2(rem8C* cpu) {
  unsigned char X = _msb_reg_idx(cpu->memory[cpu->pc++]);
  unsigned char Y = _lsb_reg_idx(cpu->memory[cpu->pc++]);
  cpu->data_reg[X] &= cpu->data_reg[Y];
}

/* Set VX to VX ^ VY */
void _instr_8XY3(rem8C* cpu) {
  unsigned char X = _msb_reg_idx(cpu->memory[cpu->pc++]);
  unsigned char Y = _lsb_reg_idx(cpu->memory[cpu->pc++]);
  cpu->data_reg[X] ^= cpu->data_reg[Y];
}

/* Set VX to VX + VY , if overflow VF = 0x01 */
void _instr_8XY4(rem8C* cpu) {
  unsigned char X = _msb_reg_idx(cpu->memory[cpu->pc++]);
  unsigned char Y = _lsb_reg_idx(cpu->memory[cpu->pc++]);
  unsigned char X_init = cpu->data_reg[X];
  cpu->data_reg[X] += cpu->data_reg[Y];
  cpu->data_reg[0x0F] = 0x00;
  if (cpu->data_reg[X] < X_init) cpu->data_reg[0x0F] = 0x01;
}

/* Set VX to VX - VY , if borrow VF = 0x01 */
void _instr_8XY5(rem8C* cpu) {
  unsigned char X = _msb_reg_idx(cpu->memory[cpu->pc++]);
  unsigned char Y = _lsb_reg_idx(cpu->memory[cpu->pc++]);
  unsigned char X_init = cpu->data_reg[X];
  cpu->data_reg[X] -= cpu->data_reg[Y];
  cpu->data_reg[0x0F] = 0x00;
  if (cpu->data_reg[X] > X_init) cpu->data_reg[0x0F] = 0x01;
}

/* Set VX to VY >> 1 , set VF to VY & 0x01 */
void _instr_8XY6(rem8C* cpu) {
  unsigned char X = _msb_reg_idx(cpu->memory[cpu->pc++]);
  unsigned char Y = _lsb_reg_idx(cpu->memory[cpu->pc++]);
  cpu->data_reg[0x0F] = cpu->data_reg[Y] & 0x01;
  cpu->data_reg[X] = cpu->data_reg[Y] >> 1;
}

/* Set VX to VY - VX , if borrow VF = 0x00 */
void _instr_8XY7(rem8C* cpu) {
  unsigned char X = _msb_reg_idx(cpu->memory[cpu->pc++]);
  unsigned char Y = _lsb_reg_idx(cpu->memory[cpu->pc++]);
  unsigned char X_init = cpu->data_reg[X];
  cpu->data_reg[X] = cpu->data_reg[Y] - cpu->data_reg[X];
  cpu->data_reg[0x0F] = 0x01;
  if (cpu->data_reg[Y] > X_init) cpu->data_reg[0x0F] = 0x00;
}

/* Set VX to VY << 1 , set VF to VY & 0x01 */
void _instr_8XYE(rem8C* cpu) {
  unsigned char X = _msb_reg_idx(cpu->memory[cpu->pc++]);
  unsigned char Y = _lsb_reg_idx(cpu->memory[cpu->pc++]);
  cpu->data_reg[0x0F] = cpu->data_reg[Y] & 0x80;
  cpu->data_reg[X] = cpu->data_reg[Y] << 1;
}

/* Skip following instruction if VX != VY */
void _instr_9XY0(rem8C* cpu) {
  unsigned char X = _msb_reg_idx(cpu->memory[cpu->pc++]);
  unsigned char Y = _lsb_reg_idx(cpu->memory[cpu->pc++]);
  if (cpu->data_reg[X] != cpu->data_reg[Y]) cpu->pc += INSTR_SIZE;
}

/* Store NNN in addr register */
void _instr_ANNN(rem8C* cpu) {
  unsigned char msb = cpu->memory[cpu->pc++];
  unsigned char lsb = cpu->memory[cpu->pc++];
  cpu->addr_reg = ((msb & 0x0F) << 8) | lsb;
}

/* Jump to address NNN + V0 */
void _instr_BNNN(rem8C* cpu) {
  unsigned char msb = cpu->memory[cpu->pc++];
  unsigned char lsb = cpu->memory[cpu->pc++];
  cpu->pc = (((msb & 0x0F) << 8) | lsb) + cpu->data_reg[0];
}

/* Set VX to random num with mask NN  */
void _instr_CXNN(rem8C* cpu) {
  unsigned char X = _msb_reg_idx(cpu->memory[cpu->pc++]);
  unsigned char lsb = cpu->memory[cpu->pc++];
  cpu->data_reg[X] = (rand() % 0xFF) & lsb;
}

/* Draw sprite at (VX, VY) 8px wide and Npx tall */
void _instr_DXYN(rem8C* cpu) {
  unsigned char X = _msb_reg_idx(cpu->memory[cpu->pc++]);
  unsigned char lsb = cpu->memory[cpu->pc++];
  unsigned char Y = _lsb_reg_idx(lsb);

  unsigned char X_val = cpu->data_reg[X];
  unsigned char Y_val = cpu->data_reg[Y];
  unsigned char N = lsb & 0x0F;

  int i;
  for (i = 0; i < N; i++) {
    unsigned char sprite_row = cpu->memory[cpu->addr_reg + i];
    _rem8C_sprite_draw(cpu, X_val, Y_val + i, sprite_row);
  }
}

/* Skip following instruction if key == VX */
void _instr_EX9E(rem8C* cpu) {
  unsigned char X = _msb_reg_idx(cpu->memory[cpu->pc++]);
  cpu->pc++;
  if (cpu->data_reg[X] == cpu->key) cpu->pc += 2;
}

/* Skip following instruction if key != VX */
void _instr_EXA1(rem8C* cpu) {
  unsigned char X = _msb_reg_idx(cpu->memory[cpu->pc++]);
  cpu->pc++;
  if (cpu->data_reg[X] != cpu->key) cpu->pc += 2;
}

/* Store delay timer into VX */
void _instr_FX07(rem8C* cpu) {
  unsigned char X = _msb_reg_idx(cpu->memory[cpu->pc++]);
  cpu->data_reg[X] = cpu->delay_timer;
  cpu->pc++;
}

/* Wait for keypress and store result in VX */
void _instr_FX0A(rem8C* cpu) {
  unsigned char X = _msb_reg_idx(cpu->memory[cpu->pc++]);
  cpu->pc++;
  if (cpu->key == KEY_DEFAULT) {
    cpu->pc -= INSTR_SIZE;
  } else {
    cpu->data_reg[X] = cpu->key;
  }
}

/* Set delay timer to value of VX */
void _instr_FX15(rem8C* cpu) {
  unsigned char X = _msb_reg_idx(cpu->memory[cpu->pc++]);
  cpu->delay_timer = cpu->data_reg[X];
  cpu->pc++;
}

/* Set sound timer to value of VX */
void _instr_FX18(rem8C* cpu) {
  unsigned char X = _msb_reg_idx(cpu->memory[cpu->pc++]);
  cpu->sound_timer = cpu->data_reg[X];
  cpu->pc++;
}

/* Add value of VX to addr register  */
void _instr_FX1E(rem8C* cpu) {
  unsigned char X = _msb_reg_idx(cpu->memory[cpu->pc++]);
  cpu->addr_reg += cpu->data_reg[X];
  cpu->pc++;
}

/* Set addr register to sprite address of VX */
void _instr_FX29(rem8C* cpu) {
  unsigned char X = _msb_reg_idx(cpu->memory[cpu->pc++]);
  cpu->addr_reg = cpu->data_reg[X] * SPRITE_WIDTH + cpu->sprite_addr;
  cpu->pc++;
}

/* Store BCD of VX at addr of addr register */
void _instr_FX33(rem8C* cpu) {
  unsigned char X = _msb_reg_idx(cpu->memory[cpu->pc++]);
  unsigned char val = cpu->data_reg[X];
  unsigned int bcd = 0;
  int shift = 0;

  while (val > 0) {
    bcd |= (val % 10) << (shift++ << 2);
    val /= 10;
  }

  cpu->memory[cpu->addr_reg] = (bcd >> 16) & 0xFF;
  cpu->memory[cpu->addr_reg + 1] = (bcd >> 8) & 0xFF;
  cpu->memory[cpu->addr_reg + 2] = bcd & 0xFF;

  cpu->pc++;
}

/* Store V0 to VX in memory starting at addr register */
void _instr_FX55(rem8C* cpu) {
  unsigned char X = _msb_reg_idx(cpu->memory[cpu->pc++]);
  int i;
  for (i = 0; i <= X; i++) {
    cpu->memory[cpu->addr_reg + i] = cpu->data_reg[i];
  }
  cpu->addr_reg += X + 1;
  cpu->pc++;
}

/* File V0 to VX from memory starting at addr register */
void _instr_FX65(rem8C* cpu) {
  unsigned char X = _msb_reg_idx(cpu->memory[cpu->pc++]);
  int i;
  for (i = 0; i <= X; i++) {
    cpu->data_reg[i] = cpu->memory[cpu->addr_reg + i] ;
  }
  cpu->addr_reg += X + 1;
  cpu->pc++;
}

/******************** Cpu Operations ********************/

void rem8C_cycle(rem8C* cpu) {
  unsigned char msb = cpu->memory[cpu->pc];
  unsigned char lsb = cpu->memory[cpu->pc + 1];

  switch (msb & 0xF0) {
    case 0x00:
      switch (msb << 8 | lsb) {
        case 0x00E0:
          _instr_00E0(cpu); break;
        case 0x00EE:
          _instr_00EE(cpu); break;
        default:
          _instr_0NNN(cpu); break;
      }
      break;
    case 0x10:
      _instr_1NNN(cpu); break;
    case 0x20:
      _instr_2NNN(cpu); break;
    case 0x30:
      _instr_3XNN(cpu); break;
    case 0x40:
      _instr_4XNN(cpu); break;
    case 0x50:
      _instr_5XY0(cpu); break;
    case 0x60:
      _instr_6XNN(cpu); break;
    case 0x70:
      _instr_7XNN(cpu); break;
    case 0x80:
      switch (lsb & 0x0F) {
        case 0x00:
          _instr_8XY0(cpu); break;
        case 0x01:
          _instr_8XY1(cpu); break;
        case 0x02:
          _instr_8XY2(cpu); break;
        case 0x03:
          _instr_8XY3(cpu); break;
        case 0x04:
          _instr_8XY4(cpu); break;
        case 0x05:
          _instr_8XY5(cpu); break;
        case 0x06:
          _instr_8XY6(cpu); break;
        case 0x07:
          _instr_8XY7(cpu); break;
        case 0x0E:
          _instr_8XYE(cpu); break;
        default: break;
      }
      break;
    case 0x90:
      _instr_9XY0(cpu); break;
    case 0xA0:
      _instr_ANNN(cpu); break;
    case 0xB0:
      _instr_BNNN(cpu); break;
    case 0xC0:
      _instr_CXNN(cpu); break;
    case 0xD0:
      _instr_DXYN(cpu); break;
    case 0xE0:
      switch (lsb) {
        case 0x9E:
          _instr_EX9E(cpu); break;
        case 0xA1:
          _instr_EXA1(cpu); break;
        default: break;
      }
      break;
    case 0xF0:
      switch (lsb) {
        case 0x07:
          _instr_FX07(cpu); break;
        case 0x0A:
          _instr_FX0A(cpu); break;
        case 0x15:
          _instr_FX15(cpu); break;
        case 0x18:
          _instr_FX18(cpu); break;
        case 0x1E:
          _instr_FX1E(cpu); break;
        case 0x29:
          _instr_FX29(cpu); break;
        case 0x33:
          _instr_FX33(cpu); break;
        case 0x55:
          _instr_FX55(cpu); break;
        case 0x65:
          _instr_FX65(cpu); break;
        default: break;
      }
      break;
    default: break;
  }

}

void rem8C_print_screen(rem8C* cpu) {
  printf("\e[1;1H\e[2J");
  int c, r;
  for (r = 0; r < SCREEN_HEIGHT; r++) {
    for (c = 0; c < SCREEN_WIDTH; c++) {
      if (cpu->screen[c][r]) {
        printf("#");
      } else {
        printf(" ");
      }
    }
    printf("\n");
  }
}

void rem8C_memset(rem8C* cpu, unsigned short addr, void* data, size_t size) {
  memcpy(&cpu->memory[addr], data, size);
}

/******************** Cpu Debug ********************/

/*  @brief  Return data stored at register V{'reg'}.
 *  Note that if 'reg' is outside of the valid range,
 *  the value 0 is returned.
 */
unsigned char rem8C_data_reg(rem8C* cpu, unsigned char reg) {
  if (reg <= 15) return cpu->data_reg[reg];
  return 0;
}

/******************** Create/Destroy Cpu ********************/

rem8C* rem8C_new() {
  rem8C* cpu = malloc(sizeof(rem8C));
  cpu->pc = START_ADDR;
  cpu->stack_pointer = START_ADDR - 0x01;
  cpu->sprite_addr = 0x00;
  _rem8C_sprite_set(cpu, cpu->sprite_addr);
  cpu->key = KEY_DEFAULT;
  return cpu;
}

void rem8C_free(rem8C* cpu) {
  free(cpu);
}