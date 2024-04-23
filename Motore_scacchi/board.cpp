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

bool Board::isValidMove(std::string move) {
	bool isMove = true;

	if (move.length() < 4 || move[0] < 'a' || move[0] > 'h' || move[2] < 'a' || move[2] > 'h' 
		|| move[1] < '1' || move[1] > '8' || move[3] < '1' || move[3] > '8') {
		isMove = false;
	}

	if (move.length() == 5 && (move[4] != 'q' && move[4] != 'r' && move[4] != 'b' && move[4] != 'n')) {
		isMove = false;
	}

	return isMove;
}