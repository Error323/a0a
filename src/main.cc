#include <iostream>
#include <unordered_map>
#include <vector>

#include "version.h"

using namespace std;

int32_t index(int32_t n) { return n * (n + 1) / 2; }

int32_t count(uint16_t left, int row) {
  auto s = 15 - index(row + 1);
  auto line = (left >> s) & ((1 << (row + 1)) - 1);
  return __builtin_popcount(line);
}

int16_t clear_row(uint16_t left, int row) {
  uint16_t s = (1 << (row + 1)) - 1;
  s <<= 15 - index(row + 1);
  return left & ~s;
}

void print_left(uint16_t left) {
  for (int i = 0; i < 5; i++) {
    for (int j = index(i), n = index(i + 1); j < n; j++) {
      cout << ((left >> (14 - j)) & 1) << " ";
    }
    cout << endl;
  }
}

void print_bin(uint16_t v) {
  for (int i = 15; i >= 0; i--) {
    cout << ((v >> i) & 1) << " ";
  }
  cout << endl;
}

int main(void) {
  uint16_t v = 0x6fbf;

  print_left(v);

  cout << endl;
  for (int i = 0; i < 5; i++) {
    cout << count(v, i) << " ";
  }
  cout << endl;

  return 0;
}
