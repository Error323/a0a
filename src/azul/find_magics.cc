#include <stdint.h>
#include <stdio.h>

#include "magics.h"
#include "utils/random.h"
#include "board.h"

int main(int argc, char **argv) {
  if (argc != 2) return 1;
  int bits = std::atoi(argv[1]);

  printf("static const uint32_t kMagics[] = {\n");
  for (int sq = 0; sq < Board::SIZE * Board::SIZE; sq++) {
    uint32_t magic = FindMagic(sq, bits);
    printf("  0x%xul,\n", magic);
  }
  printf("};\n\n");

  return 0;
}
