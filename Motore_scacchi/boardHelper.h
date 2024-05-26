#pragma once

#include "board.h"

class BoardHelper {
public:
	static void printBoard();
	static void printBoard(Position position);
	static void printLegalMoves(uint64_t moves);
};