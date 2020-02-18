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

//=============================================================================
// private functions
//=============================================================================

// computes the horizontal and vertical mask given the current square
uint32_t ScoreMask(int square);

// computes the score from the current board
int ComputeScore(int square, uint32_t board);

// creates a board from an index
uint32_t IndexToBoard(int index, int bits, uint32_t mask);

// pretty prints a binary board
void DebugBoard(uint32_t board, int square = -1);

// converts a board into an index
int Transform(uint32_t board, uint32_t magic, int bits);

// pops the lsb from a board and returns its position
int Pop1stBit(uint32_t* board);
