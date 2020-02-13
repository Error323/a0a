#pragma once
// 1 + 1 + 5 + 5*4 + 16 + 1 + 2 + 7 + 2 + 7 = 62
// |   |   |   |     |    |   |   |   |   |
// |   |   |   |     |    |   |   |   |   them floor: v in {0,...,6}
// |   |   |   |     |    |   |   |   them board: v in {0,...,5}
// |   |   |   |     |    |   |   us floor: v in {0,...,6}
// |   |   |   |     |    |   us board: v in {0,...,5}
// |   |   |   |     |    us 1st tile: v in {0, 1}
// |   |   |   |     center: v in {0,...,6}
// |   |   |   factories: v in {0,...,5}
// |   |   bag: v in {0,...,20}
// |   them score: v in {0,...,255}
// us score: v in {0,...,255}
//
// 8 + 8 + 5*5 + 5*4*3 + 16*3 + 1 + 2*(5*5*3 + 7*3 + 15*3) = 432 bit = 54 bytes

/* tile types 5 in total, tile `FIRST' is handled differently */
/*          0     1       2    3      4      5 */
enum Tile { BLUE, YELLOW, RED, BLACK, WHITE, NUM_TILES };

/* part of an action or move */
enum Position { FAC1, FAC2, FAC3, FAC4, FAC5, CENTER, NUM_POS };

/* part of an action or move */
enum Line { LINE1, LINE2, LINE3, LINE4, LINE5, FLOORLINE, NUM_LINES };

