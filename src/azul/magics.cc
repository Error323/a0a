#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 5
#define BITS 4
#define MARKER 'X'
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

// masks for column bonus
static const uint32_t ROWS[] = {0x108421, 0x210842, 0x421084, 0x842108,
                                0x1084210};
// masks for row bonus
static const uint32_t COLUMNS[] = {0x1f, 0x3e0, 0x7c00, 0xf8000, 0x1f00000};

void print_bb(uint32_t b) {
  for (int i = 0; i < SIZE; i++) {
    for (int j = 0; j < SIZE; j++) {
      int shift = (SIZE * SIZE) - (i * SIZE + j) - 1;
      printf("%c ", int((b >> shift) & 1) ? MARKER : '.');
    }
    printf("\n");
  }
}

void print_bb(uint32_t b1, uint32_t b2) {
  for (int i = 0; i < SIZE; i++) {
    for (int j = 0; j < SIZE; j++) {
      int shift = (SIZE * SIZE) - (i * SIZE + j) - 1;
      printf("%c ", int((b1 >> shift) & 1) ? MARKER : '.');
    }
    printf("   ");
    for (int j = 0; j < SIZE; j++) {
      int shift = (SIZE * SIZE) - (i * SIZE + j) - 1;
      printf("%c ", int((b2 >> shift) & 1) ? MARKER : '.');
    }
    printf("\n");
  }
}

void print_bb(uint32_t b1, uint32_t b2, uint32_t b3) {
  for (int i = 0; i < SIZE; i++) {
    for (int j = 0; j < SIZE; j++) {
      int shift = (SIZE * SIZE) - (i * SIZE + j) - 1;
      printf("%c ", int((b1 >> shift) & 1) ? MARKER : '.');
    }
    printf("   ");
    for (int j = 0; j < SIZE; j++) {
      int shift = (SIZE * SIZE) - (i * SIZE + j) - 1;
      printf("%c ", int((b2 >> shift) & 1) ? MARKER : '.');
    }
    printf("   ");
    for (int j = 0; j < SIZE; j++) {
      int shift = (SIZE * SIZE) - (i * SIZE + j) - 1;
      printf("%c ", int((b3 >> shift) & 1) ? MARKER : '.');
    }
    printf("\n");
  }
}

uint32_t random_uint32() {
  uint32_t u1, u2;
  u1 = (uint32_t)(random()) & 0xFFFF;
  u2 = (uint32_t)(random()) & 0xFFFF;
  return u1 | (u2 << 16);
}

uint32_t random_uint32_fewbits() {
  return random_uint32() & random_uint32() & random_uint32();
}

int pop_1st_bit(uint32_t *bb) {
  int index = __builtin_ffs(*bb);
  *bb &= (*bb - 1);
  return index - 1;
}

uint32_t index_to_uint32(int index, int bits, uint32_t m) {
  int i, j;
  uint32_t result = 0ul;
  for (i = 0; i < bits; i++) {
    j = pop_1st_bit(&m);
    if (index & (1 << i)) result |= (1ul << j);
  }
  return result;
}

uint32_t rmask(int sq) {
  int row = sq / SIZE, col = sq % SIZE;
  uint32_t result = COLUMNS[row] | ROWS[col];
  result ^= 1 << sq;
  return result;
}

uint32_t cols(int sq, uint32_t block) {
  uint32_t result = 0ul;
  int row = sq / SIZE, col = sq % SIZE, i;
  for (i = col + 1; i <= 4; i++) {
    if (block & (1ul << (i + row * SIZE))) break;
    result |= (1ul << (i + row * SIZE));
  }
  for (i = col - 1; i >= 0; i--) {
    if (block & (1ul << (i + row * SIZE))) break;
    result |= (1ul << (i + row * SIZE));
  }
  return result;
}

uint32_t rows(int sq, uint32_t block) {
  uint32_t result = 0ul;
  int row = sq / SIZE, col = sq % SIZE, i;
  for (i = row + 1; i <= 4; i++) {
    if (block & (1ul << (col + i * SIZE))) break;
    result |= (1ul << (col + i * SIZE));
  }
  for (i = row - 1; i >= 0; i--) {
    if (block & (1ul << (col + i * SIZE))) break;
    result |= (1ul << (col + i * SIZE));
  }
  return result;
}

int transform(uint32_t b, uint32_t magic, int bits) {
  b *= magic;
  b >>= 32 - bits;
  return (int)b;
}

static uint32_t count[SIZE * SIZE][1 << BITS];

uint32_t find_magic(int sq, int m, int hor) {
  uint32_t mask, b[1 << BITS], magic;
  uint32_t c[1 << BITS];
  uint32_t *used = count[sq];
  int i, j, n, fail;
  uint64_t k;

  mask = rmask(sq);
  mask &= hor ? COLUMNS[sq / SIZE] : ROWS[sq % SIZE];
  n = __builtin_popcount(mask);

  for (i = 0; i < (1 << n); i++) {
    b[i] = index_to_uint32(i, n, mask);
    c[i] = hor ? cols(sq, b[i]) : rows(sq, b[i]);
  }

  for (k = 0; k < 100000000ull; k++) {
    magic = random_uint32_fewbits();
    for (i = 0; i < (1 << n); i++) used[i] = 0ul;
    for (i = 0, fail = 0; !fail && i < (1 << n); i++) {
      j = transform(b[i], magic, m);
      if (used[j] == 0ul)
        used[j] = c[i];
      else if (used[j] != c[i])
        fail = 1;
    }
    if (!fail) return magic;
  }
  printf("***Failed***\n");

  return 0ul;
}

int main() {
  uint32_t tests[] = {
      0b0000001000001110000001000,
      0b0000001000001010000001000,
  };

  int squares[] = {
      13,
      13,
  };

  int results[] = {
      3,
      1,
  };
  uint32_t magics[SIZE * SIZE] = {0ul};

  printf("static const uint32_t kMagics[] = {\n");
  for (int square = 0; square < SIZE * SIZE; square++) {
    magics[square] = find_magic(square, BITS, 1);
    printf("  0x%xul,\n", magics[square]);
  }
  printf("};\n\n");

  for (int i = 0, n = ARRAY_SIZE(tests); i < n; i++) {
    int sq = squares[i];
    if (magics[sq] == 0ul) continue;
    uint32_t test = ~tests[i] & COLUMNS[sq / SIZE];
    test ^= 1ul << sq;
    uint32_t lookup = count[sq][transform(test, magics[sq], BITS)];
    print_bb(tests[i], test, lookup);
    int result = __builtin_popcount(lookup);
    printf("expected: %i, actual %i\n", results[i], result);
  }

  return 0;
}
