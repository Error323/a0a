#include <gtest/gtest.h>
#include <iostream>

#include "azul/center.h"
#include "azul/constants.h"

TEST(BagTest, Pop) {
  Bag bag;
  for (int n = 0; n < 3; n++) {
    int count[NUM_TILES] = {0};
    for (int i = 0; i < Bag::BAG_SIZE; i++) {
      Tile tile = bag.Pop();
      count[tile]++;
      int sum = 0;
      for (int j = 0; j < Tile::NUM_TILES; j++) {
        sum += bag.tiles[j];
      }
      EXPECT_EQ(sum, Bag::BAG_SIZE - i - 1);
    }
    for (int i = 0; i < NUM_TILES; i++) {
      EXPECT_EQ(Bag::BAG_SIZE / NUM_TILES, count[i]);
    }
  }
}

TEST(HolderTest, Add) {
  Holder h;
  EXPECT_EQ(h.Add(Tile::BLUE, 3), 3);
  EXPECT_EQ(h.Count(), 3);
  EXPECT_EQ(h.Count(Tile::BLUE), 3);
  EXPECT_EQ(h.Add(Tile::YELLOW, 4), 4);
  EXPECT_EQ(h.Count(), 7);
  EXPECT_EQ(h.Count(Tile::YELLOW), 4);
  EXPECT_EQ(h.Add(Tile::BLUE, 2), 5);
  EXPECT_EQ(h.Count(), 9);
}

TEST(HolderTest, Take) {
  Holder h;
  h.Add(Tile::BLUE, 4);
  h.Add(Tile::RED, 2);
  h.Add(Tile::BLACK, 6);

  EXPECT_EQ(h.Count(), 12);
  EXPECT_EQ(h.Take(Tile::RED), 2);
  EXPECT_EQ(h.Count(), 10);
  EXPECT_EQ(h.Take(Tile::RED), 0);
  EXPECT_EQ(h.Count(), 10);
}

TEST(CenterTest, AddTile) {
  Center c;
  c.Clear();

  c.AddTile(Tile::YELLOW, Position::FAC2, 2);
  EXPECT_EQ(2, c.Count(Position::FAC2));
  EXPECT_EQ(2, c.Count(Position::FAC2, Tile::YELLOW));
  c.AddTile(Tile::BLUE, Position::FAC2, 1);
  EXPECT_EQ(1, c.Count(Position::FAC2, Tile::BLUE));
  EXPECT_EQ(3, c.Count(Position::FAC2));
  c.AddTile(Tile::YELLOW, Position::FAC2, 1);
  EXPECT_EQ(3, c.Count(Position::FAC2, Tile::YELLOW));
  c.AddTile(Tile::RED, Position::CENTER, 6);
  c.AddTile(Tile::BLACK, Position::CENTER, 8);
  EXPECT_EQ(6, c.Count(Position::CENTER, Tile::RED));
  EXPECT_EQ(8, c.Count(Position::CENTER, Tile::BLACK));
  EXPECT_EQ(14, c.Count(Position::CENTER));
}

TEST(CenterTest, CenterFromString) {
  Center c;
  c.CenterFromString("00_12_3344001111__2201234__________");
  EXPECT_EQ(3, c.Count(Position::FAC2));
  EXPECT_EQ(5, c.Count(Position::CENTER));
  EXPECT_EQ(4, c.Count(Position::FAC4, Tile::YELLOW));
  EXPECT_EQ(0, c.Count(Position::FAC4, Tile::WHITE));
  EXPECT_EQ(1, c.Count(Position::CENTER, Tile::RED));
}

TEST(CenterTest, TakeTiles) {
  Center c;
  c.CenterFromString("00_12_3344001111__2201231334_______");
  EXPECT_EQ(2, c.TakeTiles(Position::FAC1, Tile::BLUE));
  EXPECT_EQ(9, c.Count(Position::CENTER));
  EXPECT_EQ(3, c.TakeTiles(Position::CENTER, Tile::BLACK));
  EXPECT_EQ(6, c.Count(Position::CENTER));
}

TEST(CenterTest, IsRoundOver) {
  Center c;
  EXPECT_FALSE(c.IsRoundOver());
  c.Clear();
  EXPECT_TRUE(c.IsRoundOver());
  c.CenterFromString("____________________111____________");
  EXPECT_FALSE(c.IsRoundOver());
  c.TakeTiles(Position::CENTER, Tile::YELLOW);
  EXPECT_TRUE(c.IsRoundOver());
}
