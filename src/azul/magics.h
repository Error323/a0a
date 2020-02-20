#pragma once

#include <stdint.h>
#include <vector>

//=============================================================================
// public functions
//=============================================================================

// score lookup using magics, call InitScoreTable() first
int GetScore(int square, uint32_t board);

// initializes the score lookup table, must be done before calling GetScore()
void InitScoreTable();

// tries to find a magic through random guessing
uint32_t FindMagic(int square, int bits);

