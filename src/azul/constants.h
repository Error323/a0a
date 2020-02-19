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
