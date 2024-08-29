#pragma once

#include "board.h"

class BoardHelper {
public:
	static void printBoard();
	static void printBoard(Position position);
	static void printLegalMoves(uint64_t moves);

	//funzioni che permettono di verificare molto velocemente se una casella è sul bordo sinistro o destro della scacchiera
	static bool isOnLeftBorder(const int& square);
	static bool isOnRightBorder(const int& square);
	static bool isOnLeftBorder(const uint64_t& square);
	static bool isOnRightBorder(const uint64_t& square);
	static void initBorderBitboards(); //metodo che inizializza le bitboard dei bordi della scacchiera

private:
	static uint64_t m_leftBorderBitboard;
	static uint64_t m_rightBorderBitboard;
};