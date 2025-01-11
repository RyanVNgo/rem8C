/*  @file   main.c
 *  @brief  Main entry point for the emulator
 *  @author Ryan V. Ngo
 */

#include <stdio.h>
#include <getopt.h>

#include <SDL2/SDL_render.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keyboard.h>

#include "rem8C.h"

SDL_Window* create_window();
void render_screen(SDL_Renderer* renderer, unsigned char data[SCREEN_WIDTH][SCREEN_HEIGHT]);

int main(int argc, char* argv[]) {
  /* argument parsing */
  char* rom_file = NULL;
  unsigned short load_addr = START_ADDR;
  unsigned short start_addr = START_ADDR;

  int opt;
  while ((opt = getopt(argc, argv, "r:l:s:")) != -1) {
    switch (opt) {
      case 'r':
        rom_file = optarg;
        break;
      case 'l':
        load_addr = strtoul(optarg, NULL, 16);
        break;
      case 's':
        start_addr = strtoul(optarg, NULL, 16);
        break;
      default: break;
    }
  }

  if (!rom_file) {
    printf("ROM not provided\n");
    return 1;
  }

  FILE* rom = fopen(rom_file, "rb");
  if (!rom) {
    printf("ROM not found \n");
    return 1;
  }
  printf("ROM found: %s\n", rom_file);

  fseek(rom, 0, SEEK_END);
  long size = ftell(rom);
  printf("ROM size: %ld bytes\n", size);
  rewind(rom);

  if (size > MAX_ADDR - start_addr) {
    fclose(rom);
    printf("! ROM too large | ROM size: %ld bytes!\n", size);
    return 1;
  }

  unsigned char* prog = malloc(sizeof(unsigned char) * size);
  fread(prog, sizeof(char), size, rom);
  fclose(rom);

  /* preparing emulator */
  rem8C* cpu = rem8C_new();
  rem8C_set_start_addr(cpu, start_addr);
  rem8C_memset(cpu, load_addr, prog, size);
  free(prog);

  unsigned char screen_buff[SCREEN_WIDTH][SCREEN_HEIGHT] = {0};

  SDL_Window* window = create_window();
  SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);

  /* running emulator */
  int running = 1;
  int pause = 0;
  Uint32 last_time = 0;
  while (running) {
    /* input logic */
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) running = 0;
      if (event.type == SDL_KEYDOWN) {
        if (event.key.keysym.sym == SDLK_ESCAPE) running = 0;
      }
      if (event.type == SDL_KEYDOWN) rem8C_set_key(cpu, event.key.keysym.sym);
      if (event.type == SDL_KEYUP) rem8C_unset_key(cpu, event.key.keysym.sym);
      if (event.key.keysym.sym == 'm') pause ^= 1;
    }
    
    /* pausing */
    if (pause) continue;

    /* timing and display logic */
    Uint32 curr_time = SDL_GetTicks();
    Uint32 elapsed_time = curr_time - last_time;
    if (elapsed_time >= 17) {
      rem8C_update_timers(cpu);
      int i;
      for (i = 0; i < (elapsed_time / 2); i++) {
        rem8C_cycle(cpu);
      }

      rem8C_read_screen(cpu, 0, 0, screen_buff, sizeof(screen_buff));
      render_screen(renderer, screen_buff);

      last_time = curr_time;
    }

  }

  SDL_DestroyWindow(window);
  SDL_DestroyRenderer(renderer);
  rem8C_free(cpu);
  return 0;
}

SDL_Window* create_window() {
  return SDL_CreateWindow(
      "CHIP-8 Emulator",
      SDL_WINDOWPOS_UNDEFINED,
      SDL_WINDOWPOS_UNDEFINED,
      640,
      320,
      SDL_WINDOW_SHOWN
  );
}

void render_screen(SDL_Renderer* renderer, unsigned char data[SCREEN_WIDTH][SCREEN_HEIGHT]) {
  int y, x;
  for (y = 0; y < SCREEN_HEIGHT; y++) {
    for (x = 0; x < SCREEN_WIDTH; x++) {
      SDL_SetRenderDrawColor(renderer, 150, 104, 23, 255);
      if (data[x][y] == 1) {
        SDL_SetRenderDrawColor(renderer, 252, 204, 46, 255);
      }
      SDL_Rect rect = {x * 10, y * 10, 10, 10};
      SDL_RenderFillRect(renderer, &rect);
    }
  }
  SDL_RenderPresent(renderer);
}

