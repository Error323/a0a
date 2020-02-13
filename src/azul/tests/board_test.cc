#include <gtest/gtest.h>

#include "azul/board.h"
#include "azul/constants.h"
#include "azul/magics.h"

class BoardTest : public testing::Test {
 protected:
  Board board_;

  void SetUp() { InitScoreTable(); }
};

TEST_F(BoardTest, Score1) {
  EXPECT_EQ(board_.Score(), 0);
  Move m(Position::FAC1, Tile::BLUE, Line::LINE3);
  board_.ApplyMove(m, 3);
  board_.NextRound();
  EXPECT_EQ(board_.Score(), 1);
}

TEST_F(BoardTest, ScoreTileBonus) {
  for (int i = 0; i < 5; i++) {
    Move m(Position::FAC1, Tile::WHITE, Line(i));
    board_.ApplyMove(m, i + 1);
  }
  board_.NextRound();
  EXPECT_EQ(board_.Score(), 15);
}

TEST_F(BoardTest, ScoreColumnBonus) {
  Tile order[] = {BLUE, WHITE, BLACK, RED, YELLOW};
  for (int i = 0; i < 5; i++) {
    Move m(Position::FAC1, order[i], Line(i));
    board_.ApplyMove(m, i + 1);
  }
  board_.NextRound();
  EXPECT_EQ(board_.Score(), 22);
}

TEST_F(BoardTest, ScoreRowBonus) {
  int scores[] = {1, 3, 6, 10, 17};
  for (int i = 0; i < 5; i++) {
    Move m(Position::FAC1, Tile(i), LINE2);
    board_.ApplyMove(m, 2);
    board_.NextRound();
    EXPECT_EQ(board_.Score(), scores[i]);
  }

  EXPECT_TRUE(board_.IsTerminal());
}
