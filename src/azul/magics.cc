#include "magics.h"
#include "board.h"

#include <glog/logging.h>
#include <vector>

// nof bits used per square entry
static const int kMagicBits = 8;

// magics for 8 bit table, found using find_magics.cc
static const uint32_t kMagics[] = {
    0x401041aul,  0x7c23192ul,  0x8044180ul,  0x18032080ul, 0x8091009ul,
    0x14200419ul, 0x20100120ul, 0x2fc02180ul, 0x9008088ul,  0x803149ul,
    0x1210022ul,  0x20258020ul, 0x8020110ul,  0x38060048ul, 0x6440041ul,
    0x940d8801ul, 0xe0048c01ul, 0x90082020ul, 0x18019008ul, 0x4622011ul,
    0x4c0a0480ul, 0x58244480ul, 0x9804c080ul, 0x28096280ul, 0x8010480ul};

static std::vector<uint8_t> scores_;

uint32_t ScoreMask(int square) {
  int row = square / Board::SIZE, col = square % Board::SIZE;
  uint32_t result = Board::kRows[row] | Board::kColumns[col];
  result ^= 1 << square;
  return result;
}

int GetScore(int square, uint32_t board) {
  board = ~board & ScoreMask(square);
  int index = square * (1 << kMagicBits);
  index += Transform(board, kMagics[square], kMagicBits);
  return scores_[index];
}

uint32_t IndexToBoard(int index, int bits, uint32_t mask) {
  int i, j;
  uint32_t result = 0ul;
  for (i = 0; i < bits; i++) {
    j = Pop1stBit(&mask);
    if (index & (1 << i)) result |= (1ul << j);
  }
  return result;
}

int ComputeScore(int square, uint32_t board) {
  int hor = 0, ver = 0;
  int row = square / Board::SIZE, col = square % Board::SIZE, i;

  // horizontal
  for (i = col + 1; i <= 4; i++) {
    if (board & (1ul << (i + row * Board::SIZE))) break;
    hor++;
  }
  for (i = col - 1; i >= 0; i--) {
    if (board & (1ul << (i + row * Board::SIZE))) break;
    hor++;
  }

  // vertical
  for (i = row + 1; i <= 4; i++) {
    if (board & (1ul << (col + i * Board::SIZE))) break;
    ver++;
  }
  for (i = row - 1; i >= 0; i--) {
    if (board & (1ul << (col + i * Board::SIZE))) break;
    ver++;
  }

  // if we connected with tiles on the board, also add a point for our own
  // square
  if (hor > 0) hor++;
  if (ver > 0) ver++;

  // if we didn't connect with tiles on the board, we get just 1 point
  if (hor + ver == 0) return 1;

  return hor + ver;
}

void InitScoreTable() {
  uint32_t mask, board;
  int square, i, j, n;

  scores_.resize(Board::SIZE * Board::SIZE * (1 << kMagicBits));

  for (square = 0; square < Board::SIZE * Board::SIZE; square++) {
    mask = ScoreMask(square);
    n = __builtin_popcount(mask);

    for (i = 0; i < (1 << n); i++) {
      board = IndexToBoard(i, n, mask);
      j = Transform(board, kMagics[square], kMagicBits);
      scores_[square * (1 << kMagicBits) + j] = ComputeScore(square, board);
    }
  }

  VLOG(1) << "Initialized score table " << scores_.size() << " bytes";
}

void DebugBoard(uint32_t board, int square) {
  for (int i = 0; i < Board::SIZE; i++) {
    for (int j = 0; j < Board::SIZE; j++) {
      if (i * Board::SIZE + j == square)
        printf("S ");
      else
        printf("%c ", (board >> (i * Board::SIZE + j)) & 1 ? 'X' : '.');
    }
    printf("\n");
  }
}

int Transform(uint32_t board, uint32_t magic, int bits) {
  board *= magic;
  board >>= 32 - bits;
  return static_cast<int>(board);
}

int Pop1stBit(uint32_t *board) {
  int index = __builtin_ffs(*board);
  *board &= (*board - 1);
  return index - 1;
}
