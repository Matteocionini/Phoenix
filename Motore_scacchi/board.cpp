#include "board.h"
#include "engineUtils.h"

unsigned long Board::m_bitBoards[nBitboards] = { 0 };

void Board::makeMove(std::string move) {
	return;
}

void Board::resetBoard() {
	for (int i = 0; i < nBitboards; i++) {
		m_bitBoards[i] = 0;
	}
}

void Board::setPosition(std::string fenstring) {
	return;
}