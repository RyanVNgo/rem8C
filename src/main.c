/*  @file   main.c
 *  @brief  Main entry point for the emulator
 *  @author Ryan V. Ngo
 */

#include <stdio.h>

#include "rem8C.h"

int main(int argc, char* argv[]) {
  if (argc < 2) {
    printf("ROM not provided\n");
    return 1;
  }

  FILE* rom = fopen(argv[1], "rb");
  if (!rom) {
    printf("ROM not found \n");
    return 1;
  }

  printf("ROM found.\n");

  fseek(rom, 0, SEEK_END);
  long size = ftell(rom);
  printf("ROM size: %ld bytes\n", size);
  rewind(rom);

  unsigned char* prog = malloc(sizeof(unsigned char) * size);
  fread(prog, sizeof(char), size, rom);

  rem8C* cpu = rem8C_new();

  rem8C_memset(cpu, START_ADDR, prog, size);

  int i;
  for (i = 0; i < 512; i++) {
    rem8C_cycle(cpu);
    rem8C_print_screen(cpu);
  }

  fclose(rom);
  rem8C_free(cpu);
  return 0;
}

