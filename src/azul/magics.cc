#include "magics.h"

#include <glog/logging.h>
#include <utils/random.h>

#include <vector>

#include "board.h"

// nof bits used per square entry
static const int kMagicBits = 8;

// magics for 8 bit table, found using find_magics.cc
static const uint32_t kMagics[] = {
    0x401041aul,  0x7c23192ul,  0x8044180ul,  0x18032080ul, 0x8091009ul,
    0x14200419ul, 0x20100120ul, 0x2fc02180ul, 0x9008088ul,  0x803149ul,
    0x1210022ul,  0x20258020ul, 0x8020110ul,  0x38060048ul, 0x6440041ul,
    0x940d8801ul, 0xe0048c01ul, 0x90082020ul, 0x18019008ul, 0x4622011ul,
    0x4c0a0480ul, 0x58244480ul, 0x9804c080ul, 0x28096280ul, 0x8010480ul};

// score table for all squares
static std::vector<uint8_t> scores_;


// computes the horizontal and vertical mask given the current square
static uint32_t ScoreMask(int square);
// computes the score from the current board
static int ComputeScore(int square, uint32_t board);
// creates a board from an index
static uint32_t IndexToBoard(int index, int bits, uint32_t mask);
// converts a board into an index
static int Transform(uint32_t board, uint32_t magic, int bits);
// pops the lsb from a board and returns its position
static int Pop1stBit(uint32_t* board);



//=============================================================================
// public functions
//=============================================================================
void InitScoreTable() {
  uint32_t mask, board;
  int square, i, index, n;

  scores_.resize(Board::SIZE * Board::SIZE * (1 << kMagicBits));

  for (square = 0; square < Board::SIZE * Board::SIZE; square++) {
    mask = ScoreMask(square);
    n = __builtin_popcount(mask);

    for (i = 0; i < (1 << n); i++) {
      board = IndexToBoard(i, n, mask);
      index = square * (1 << kMagicBits);
      index += Transform(board, kMagics[square], kMagicBits);
      scores_[index] = ComputeScore(square, board);
    }
  }

  VLOG(1) << "Initialized score table " << scores_.size() << " bytes";
}


int GetScore(int square, uint32_t board) {
  board = ~board & ScoreMask(square);
  int index = square * (1 << kMagicBits);
  index += Transform(board, kMagics[square], kMagicBits);
  return scores_[index];
}


uint32_t FindMagic(int square, int bits) {
  static constexpr int kBits = 8;
  static constexpr int N = 1 << kBits;

  std::vector<uint8_t> used(1 << bits);
  uint32_t mask = ScoreMask(square);
  uint32_t block[N], magic;
  uint8_t score[N];
  int i, j, fail = 0;

  for (i = 0; i < N; i++) {
    block[i] = IndexToBoard(i, kBits, mask);
    score[i] = ComputeScore(square, block[i]);
  }

  for (uint64_t k = 0; k < 1ull << 40ull; k++) {
    magic = utils::Random::Get().GetFewBits32();
    std::fill(begin(used), end(used), 0);
    for (i = 0, fail = 0; !fail && i < N; i++) {
      j = Transform(block[i], magic, bits);
      if (used[j] == 0) used[j] = score[i];
      else if (used[j] != score[i]) fail = 1;
    }
    if (!fail) return magic;
  }

  return 0ul;
}



//=============================================================================
// private functions
//=============================================================================
uint32_t ScoreMask(int square) {
  int row = square / Board::SIZE, col = square % Board::SIZE;
  uint32_t result = Board::kRows[row] | Board::kColumns[col];
  result ^= 1 << square;
  return result;
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
