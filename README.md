# rem8C - CHIP-8 Emulator

A CHIP-8 emulator made in C.

## Downloading and Compiling
This project uses Make (`make`) as it's build system and SDL2 (`libsdl2-dev`) for graphics. Ensure you have both installed.

To download and build the project:
```sh
$  git clone https://github.com/RyanVNgo/rem8C
$  cd rem8C
$  make
```

## Running
For most CHIP-8 roms, you can just run the following:
```sh
$  ./rem8C -r <ROM File>
```

There are some additional configuration arguments if needed:
```
Usage:
  ./rem8C -r <ROM File> [-l {200}] [-s {200}]

Options:
  -l  <load_addr>     Address, in hex bar the decorator (i.e. 200 not 0x200), to load the ROM .
  -s  <start_addr>    Address, in hex bar the decorator (i.3. 200 not 0x200), to start running.
```

## Input
The keypad input of rem8C is mapped as follows:
  
CHIP-8 Keypad Layout
| 1 | 2 | 3 | C |
|---|---|---|---|
| 4 | 5 | 6 | D |
| 7 | 8 | 9 | E |
| A | 0 | B | F |

Keyboard Layout
| 1 | 2 | 3 | 4 |
|---|---|---|---|
| q | w | e | r |
| a | s | d | f |
| z | x | c | v |

