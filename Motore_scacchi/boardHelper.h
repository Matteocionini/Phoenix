#pragma once

#include "board.h"

class BoardHelper {
public:
	static void printBoard();
	static void printLegalMoves(uint64_t moves);
};