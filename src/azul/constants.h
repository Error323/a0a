#pragma once

#include <stdint.h>

/* tile types 5 in total, tile `FIRST' is handled differently */
/*          0     1       2    3      4      5 */
enum Tile : uint8_t { BLUE, YELLOW, RED, BLACK, WHITE, NUM_TILES };

/* part of an action or move */
enum Position : uint8_t { FAC1, FAC2, FAC3, FAC4, FAC5, CENTER, NUM_POS };

/* part of an action or move */
enum Line : uint8_t { LINE1, LINE2, LINE3, LINE4, LINE5, FLOORLINE, NUM_LINES };

/* maximum number of moves */
static const int kNumMoves = NUM_POS * NUM_TILES * NUM_LINES;

// 1 + 1 + 5 + 5*4 + 15 + 1 + 1 + 1 + 1 + 1 + 1 + 1 = 49
// |   |   |   |     |    |   |   |   |   |   |   |
// |   |   |   |     |    |   |   |   |   |   |   them floor: v in {0,...,7}
// |   |   |   |     |    |   |   |   |   |   them wall: v in {0, 1}
// |   |   |   |     |    |   |   |   |   them left: v in {0,...,5}
// |   |   |   |     |    |   |   |   us floor: v in {0,...,7}
// |   |   |   |     |    |   |   us wall: v in {0, 1}
// |   |   |   |     |    |   us left: v in {0,...,5}
// |   |   |   |     |    1st tile: v in {-1, 0, 1}
// |   |   |   |     center: v in {0,...,5}
// |   |   |   factories: v in {0,...,5}
// |   |   bag: v in {0,...,20}
// |   them score: v in {0,...,255}
// us score: v in {0,...,255}
static const int kNumPlanes = 49;
