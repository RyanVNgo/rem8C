/*  @file   main.c
 *  @brief  Main entry point for the emulator
 *  @author Ryan V. Ngo
 */

#include <stdio.h>

#include <SDL2/SDL_render.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keyboard.h>

#include "rem8C.h"

SDL_Window* create_window();
void render_screen(SDL_Renderer* renderer, unsigned char data[SCREEN_WIDTH][SCREEN_HEIGHT]);

struct run_cpu_args {
  rem8C* cpu;
  int* running;
};
void run_cpu(struct run_cpu_args* args);

int main(int argc, char* argv[]) {
  /* rom loading */
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

  if (size > MAX_ADDR - START_ADDR) {
    fclose(rom);
    printf("! ROM too large !\n");
    printf("! ROM size: %ld bytes\n", size);
    printf("! RAM size: %d bytes\n", MAX_ADDR - START_ADDR);
    return 0;
  }

  unsigned char* prog = malloc(sizeof(unsigned char) * size);
  fread(prog, sizeof(char), size, rom);
  fclose(rom);

  /* preparing emulator */
  rem8C* cpu = rem8C_new();

  rem8C_memset(cpu, START_ADDR, prog, size);
  free(prog);

  unsigned char screen_buff[SCREEN_WIDTH][SCREEN_HEIGHT] = {0};

  SDL_Window* window = create_window();
  SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);

  /* running emulator */
  int running = 1;
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
    }

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

