/*  @file   main.c
 *  @brief  Main entry point for the emulator
 *  @author Ryan V. Ngo
 */

#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <time.h>
#include <ncurses.h>

#include "rem8C.h"

void initialize_display();
void render_display(unsigned char screen_buff[SCREEN_WIDTH][SCREEN_HEIGHT]);
void end_display();

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
  fclose(rom);

  rem8C* cpu = rem8C_new();

  rem8C_memset(cpu, START_ADDR, prog, size);
  free(prog);

  unsigned char screen_buff[SCREEN_WIDTH][SCREEN_HEIGHT] = {0};

  initialize_display();

  struct timespec clock_rate;
  clock_rate.tv_sec = 0;
  clock_rate.tv_nsec = 2000 * 1000;

  int i;
  for (i = 0; i < 512;) {
    rem8C_cycle(cpu);

    rem8C_read_screen(cpu, 0, 0, screen_buff, sizeof(screen_buff));

    int ch = getch();
    if (ch == 'q') break;

    render_display(screen_buff);

    nanosleep(&clock_rate, NULL);
  }

  end_display();
  rem8C_free(cpu);
  return 0;
}

void initialize_display() {
  initscr();
  cbreak();
  noecho();
  curs_set(0);
  keypad(stdscr, TRUE);
  timeout(0);
}

void draw_pixel(int x, int y, int on) {
  mvaddch(y, x, on ? ACS_BLOCK: ' ');
}

void render_display(unsigned char screen_buff[SCREEN_WIDTH][SCREEN_HEIGHT]) {
  int x, y;
  for (y = 0; y < SCREEN_HEIGHT; y++) {
    for (x = 0; x < SCREEN_WIDTH; x++) {
      draw_pixel(x, y, screen_buff[x][y]);
    }
  }
}

void end_display() {
  endwin();
}
