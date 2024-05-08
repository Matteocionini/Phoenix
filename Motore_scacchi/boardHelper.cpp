#include <iostream>
#include <memory>

#include "boardHelper.h"
#include "engineUtils.h"

void BoardHelper::printBoard() {
	std::shared_ptr<uint64_t[]> bitboards = Board::getBitBoards();
	bitBoardType pieceType, pieceColor;

	std::cout << "\n\n";

	for (int i = 7; i >= 0; i--) {
		std::cout << "---------------------------------" << std::endl;
		std::cout << "| ";
		for (int j = 0; j < 8; j++) {
			if (((bitboards[nPawns] >> (i * 8 + j)) & 1) == 1) {
				pieceType = nPawns;
			}
			else if (((bitboards[nBishops] >> (i * 8 + j)) & 1) == 1) {
				pieceType = nBishops;
			}
			else if (((bitboards[nQueens] >> (i * 8 + j)) & 1) == 1) {
				pieceType = nQueens;
			}
			else if (((bitboards[nKings] >> (i * 8 + j)) & 1) == 1) {
				pieceType = nKings;
			}
			else if (((bitboards[nKnights] >> (i * 8 + j)) & 1) == 1) {
				pieceType = nKnights;
			}
			else if (((bitboards[nRooks] >> (i * 8 + j)) & 1) == 1) {
				pieceType = nRooks;
			}
			else {
				pieceType = nWhite;
			}

			if (((bitboards[nBlack] >> (i * 8 + j)) & 1) == 1) {
				pieceColor = nBlack;
			}
			else {
				pieceColor = nWhite;
			}

			switch (pieceType) {
			case nPawns: {
				if (pieceColor == nWhite) {
					std::cout << "P";
				}
				else {
					std::cout << "p";
				}
				break;
			}

			case nBishops: {
				if (pieceColor == nWhite) {
					std::cout << "B";
				}
				else {
					std::cout << "b";
				}
				break;
			}

			case nQueens: {
				if (pieceColor == nWhite) {
					std::cout << "Q";
				}
				else {
					std::cout << "q";
				}
				break;
			}

			case nKings: {
				if (pieceColor == nWhite) {
					std::cout << "K";
				}
				else {
					std::cout << "k";
				}
				break;
			}

			case nKnights: {
				if (pieceColor == nWhite) {
					std::cout << "N";
				}
				else {
					std::cout << "n";
				}
				break;
			}

			case nRooks: {
				if (pieceColor == nWhite) {
					std::cout << "R";
				}
				else {
					std::cout << "r";
				}
				break;
			}

			default: {
				std::cout << " ";
			}
			}

			std::cout << " | ";
		}
		std::cout << std::endl;
	}
	std::cout << "---------------------------------" << std::endl;
}

void BoardHelper::printLegalMoves(uint64_t moves) {
	for (int i = 7; i >= 0; i--) {
		std::cout << "---------------------------------" << std::endl;
		std::cout << "| ";

		for (int j = 0; j < 8; j++) {
			if ((moves >> (i * 8 + j)) & 1) {
				std::cout << "X";
			}
			else {
				std::cout << " ";
			}

			std::cout << " | ";
		}

		std::cout << std::endl;
	}
	std::cout << "---------------------------------" << std::endl;
}