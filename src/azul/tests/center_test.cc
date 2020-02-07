#include <gtest/gtest.h>
#include <iostream>

#include "azul/center.cc"

TEST(CenterTest, BagFromString) {
  Center c;
  c.BagFromString("0123401234012340123401234012340123401234012340123401234012340123401234012340123401234012340123401234");
  EXPECT_EQ(4, c.Count(Center::FAC2));
  EXPECT_EQ(0, c.Count(Center::CENTER));
}

TEST(CenterTest, AddTile) {
  Center c;
  c.Clear();

  // flood factory 2
  EXPECT_EQ(0, c.AddTile(Tile::YELLOW, Center::FAC2, 2));
  EXPECT_EQ(2, c.Count(Center::FAC2));
  EXPECT_EQ(2, c.Count(Center::FAC2, Tile::YELLOW));
  EXPECT_EQ(0, c.AddTile(Tile::BLUE, Center::FAC2, 1));
  EXPECT_EQ(1, c.Count(Center::FAC2, Tile::BLUE));
  EXPECT_EQ(3, c.Count(Center::FAC2));
  EXPECT_EQ(2, c.AddTile(Tile::YELLOW, Center::FAC2, 3));
  EXPECT_EQ(3, c.Count(Center::FAC2, Tile::YELLOW));

  // flood the center
  EXPECT_EQ(0, c.AddTile(Tile::RED, Center::CENTER, 6));
  EXPECT_EQ(0, c.AddTile(Tile::BLACK, Center::CENTER, 8));
  EXPECT_EQ(4, c.AddTile(Tile::BLACK, Center::CENTER, 6));

  EXPECT_EQ(10, c.Count(Center::CENTER, Tile::BLACK));
}

TEST(CenterTest, CenterFromString) {
  Center c;
  c.CenterFromString("00_12_3344001111__2201234");
  EXPECT_EQ(3, c.Count(Center::FAC2));
  EXPECT_EQ(5, c.Count(Center::CENTER));
}
