#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define NUM_TESTS 1024
#define SIZE 5
#define BITS 8

// masks for column bonus
static const uint32_t ROWS[] = {0x108421, 0x210842, 0x421084, 0x842108,
                                0x1084210};
// masks for row bonus
static const uint32_t COLUMNS[] = {0x1f, 0x3e0, 0x7c00, 0xf8000, 0x1f00000};

uint32_t random_uint32() {
  uint32_t u1, u2;
  u1 = (uint32_t)(random()) & 0xFFFF;
  u2 = (uint32_t)(random()) & 0xFFFF;
  return u1 | (u2 << 16);
}

uint32_t random_uint32_fewbits() { return random_uint32() & random_uint32(); }

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

int score(int sq, uint32_t block) {
  uint32_t hor = 0ul, ver = 0ul;
  int row = sq / SIZE, col = sq % SIZE, i;

  // horizontal
  for (i = col + 1; i <= 4; i++) {
    if (block & (1ul << (i + row * SIZE))) break;
    hor |= (1ul << (i + row * SIZE));
  }
  for (i = col - 1; i >= 0; i--) {
    if (block & (1ul << (i + row * SIZE))) break;
    hor |= (1ul << (i + row * SIZE));
  }

  // vertical
  for (i = row + 1; i <= 4; i++) {
    if (block & (1ul << (col + i * SIZE))) break;
    ver |= (1ul << (col + i * SIZE));
  }
  for (i = row - 1; i >= 0; i--) {
    if (block & (1ul << (col + i * SIZE))) break;
    ver |= (1ul << (col + i * SIZE));
  }

  int hor_score = __builtin_popcount(hor);
  int ver_score = __builtin_popcount(ver);

  if (hor_score) hor_score++;
  if (ver_score) ver_score++;
  if (hor_score + ver_score == 0) return 1;

  return hor_score + ver_score;
}

int transform(uint32_t b, uint32_t magic, int bits) {
  b *= magic;
  b >>= 32 - bits;
  return static_cast<int>(b);
}

static uint8_t count[SIZE * SIZE][1 << BITS];

uint32_t find_magic(int sq, int m) {
  uint32_t mask, b[1 << 8], magic;
  uint8_t c[1 << 8];
  uint8_t *used = count[sq];
  int i, j, n, fail;
  uint64_t k;

  mask = rmask(sq);
  n = __builtin_popcount(mask);

  for (i = 0; i < (1 << n); i++) {
    b[i] = index_to_uint32(i, n, mask);
    c[i] = score(sq, b[i]);
  }

  for (k = 0; k < 10000000000ull; k++) {
    magic = random_uint32_fewbits();
    memset(used, 0, (1 << m) * sizeof(used[0]));
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
  srand(time(0));

  uint32_t tests[NUM_TESTS];
  int squares[NUM_TESTS];
  int results[NUM_TESTS];
  for (int i = 0; i < NUM_TESTS; i++) {
    tests[i] = random_uint32() & ((1ul << SIZE * SIZE) - 1);
    squares[i] = (int)random() % (SIZE * SIZE);
    results[i] = score(squares[i], ~tests[i] & rmask(squares[i]));
  }

  uint32_t magics[SIZE * SIZE] = {0ul};

  printf("static const uint32_t kMagics[] = {\n");
  for (int sq = 0; sq < SIZE * SIZE; sq++) {
    magics[sq] = find_magic(sq, BITS);
    printf("  0x%xul,\n", magics[sq]);
  }
  printf("};\n\n");

  for (int i = 0; i < NUM_TESTS; i++) {
    int sq = squares[i];
    int result = count[sq][transform(~tests[i] & rmask(sq), magics[sq], BITS)];
    if (results[i] != result) {
      printf("expected: %i, actual %i\n", results[i], result);
    }
  }

  return 0;
}
