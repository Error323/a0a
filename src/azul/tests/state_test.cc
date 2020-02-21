#include "azul/state.h"

#include <gtest/gtest.h>
#include <utils/random.h>

#include "azul/board.h"
#include "azul/center.h"
#include "azul/constants.h"
#include "azul/magics.h"

class StateTest : public testing::Test {
 protected:
  State state_;

  void SetUp() { InitScoreTable(); }
};

TEST_F(StateTest, LegalMoves1) {
  state_.FromString("____001____________________________");
  MoveList moves;
  int n = state_.LegalMoves(moves);
  EXPECT_EQ(n, 12);

  for (int i = 0; i < 6; i++) {
    EXPECT_EQ(moves[i].factory, FAC2);
    EXPECT_EQ(moves[i].line, i);
    EXPECT_EQ(moves[i].tile_type, BLUE);
  }

  for (int i = 6; i < n; i++) {
    EXPECT_EQ(moves[i].factory, FAC2);
    EXPECT_EQ(moves[i].line, i - 6);
    EXPECT_EQ(moves[i].tile_type, YELLOW);
  }
}

TEST_F(StateTest, LegalMoves2) {
  state_.FromString("________2221________44444__________");
  MoveList moves;
  int n = state_.LegalMoves(moves);
  EXPECT_EQ(n, 18);

  Move move(FAC3, YELLOW, LINE1);
  state_.Step(move);
  n = state_.LegalMoves(moves);
  EXPECT_EQ(n, 12);

  move.factory = CENTER;
  move.tile_type = WHITE;
  move.line = LINE5;
  state_.Step(move);
  n = state_.LegalMoves(moves);
  EXPECT_EQ(n, 5);
}

TEST_F(StateTest, SimulateGame) {
  MoveList moves;
  int n;
  while (!state_.IsTerminal()) {
    n = state_.LegalMoves(moves);
    EXPECT_GT(n, 0);
    n = utils::Random::Get().GetInt(0, n - 1);
    state_.Step(moves[n]);
  }
}

TEST_F(StateTest, Assignment) {
  MoveList moves;
  int n;
  State s2 = state_;
  while (!s2.IsTerminal()) {
    n = s2.LegalMoves(moves);
    state_ = s2;
    EXPECT_FALSE(&s2 == &state_);
    EXPECT_EQ(s2.Serialize(), state_.Serialize());
    EXPECT_EQ(std::hash<State>()(s2), std::hash<State>()(state_));
    n = utils::Random::Get().GetInt(0, n - 1);
    s2.Step(moves[n]);
    EXPECT_NE(s2.Serialize(), state_.Serialize());
    EXPECT_NE(std::hash<State>()(s2), std::hash<State>()(state_));
  }
}
