#include <gtest/gtest.h>
#include <iostream>

#include "azul/board.cc"
#include "azul/constants.h"

TEST(BoardTest, Score1) {
  Board b;
  Move m(Position::FAC1, Tile::BLUE, Line::LINE3);
  b.ApplyMove(m, 3);
  b.NextRound();
  // EXPECT_EQ(b.Score(), 1);
}
