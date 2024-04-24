#include "board.h"
#include "engineUtils.h"
#include "uciHandler.h"

uint64_t Board::m_bitBoards[nBitboards] = { 0 };

void Board::makeMove(std::string move) {
	return;
}

void Board::resetBoard() {
	for (int i = 0; i < nBitboards; i++) {
		setPosition("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
	}
}

void Board::setPosition(std::string fenstring) {
	std::vector<std::string> fenSplit = uciHandler::split(fenstring);
	int rank = 7, column = 0;

	for (int i = 0; i < fenSplit[0].size(); i++, column++) {

		switch (fenSplit[0][i]) {
		case 'r': { //è una torre nera
			m_bitBoards[nRooks] = (((m_bitBoards[nRooks] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_bitBoards[nRooks];
			m_bitBoards[nBlack] = (((m_bitBoards[nBlack] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_bitBoards[nBlack];
			break;
		}

		case 'n': { //è un cavallo nero
			m_bitBoards[nKnights] = (((m_bitBoards[nKnights] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_bitBoards[nKnights];
			m_bitBoards[nBlack] = (((m_bitBoards[nBlack] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_bitBoards[nBlack];
			break;
		}

		case 'b': { //è un alfiere nero
			m_bitBoards[nBishops] = (((m_bitBoards[nBishops] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_bitBoards[nBishops];
			m_bitBoards[nBlack] = (((m_bitBoards[nBlack] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_bitBoards[nBlack];
			break;
		}

		case 'q': { //è una regina nera
			m_bitBoards[nQueens] = (((m_bitBoards[nQueens] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_bitBoards[nQueens];
			m_bitBoards[nBlack] = (((m_bitBoards[nBlack] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_bitBoards[nBlack];
			break;
		}

		case 'k': { //è un re nero
			m_bitBoards[nKings] = (((m_bitBoards[nKings] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_bitBoards[nKings];
			m_bitBoards[nBlack] = (((m_bitBoards[nBlack] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_bitBoards[nBlack];
			break;
		}

		case 'p': { //è un pedone nero
			m_bitBoards[nPawns] = (((m_bitBoards[nPawns] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_bitBoards[nPawns];
			m_bitBoards[nBlack] = (((m_bitBoards[nBlack] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_bitBoards[nBlack];
			break;
		}

		case 'R': { //è una torre bianca
			m_bitBoards[nRooks] = (((m_bitBoards[nRooks] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_bitBoards[nRooks];
			m_bitBoards[nWhite] = (((m_bitBoards[nWhite] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_bitBoards[nWhite];
			break;
		}

		case 'N': { //è un cavallo bianco
			m_bitBoards[nKnights] = (((m_bitBoards[nKnights] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_bitBoards[nKnights];
			m_bitBoards[nWhite] = (((m_bitBoards[nWhite] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_bitBoards[nWhite];
			break;
		}

		case 'B': { //è un alfiere bianco
			m_bitBoards[nBishops] = (((m_bitBoards[nBishops] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_bitBoards[nBishops];
			m_bitBoards[nWhite] = (((m_bitBoards[nWhite] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_bitBoards[nWhite];
			break;
		}

		case 'Q': { //è una regina bianca
			m_bitBoards[nQueens] = (((m_bitBoards[nQueens] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_bitBoards[nQueens];
			m_bitBoards[nWhite] = (((m_bitBoards[nWhite] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_bitBoards[nWhite];
			break;
		}

		case 'K': { //è il re bianco
			m_bitBoards[nKings] = (((m_bitBoards[nKings] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_bitBoards[nKings];
			m_bitBoards[nWhite] = (((m_bitBoards[nWhite] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_bitBoards[nWhite];
			break;
		}

		case 'P': { //è il pedone bianco
			m_bitBoards[nPawns] = (((m_bitBoards[nPawns] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_bitBoards[nPawns];
			m_bitBoards[nWhite] = (((m_bitBoards[nWhite] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_bitBoards[nWhite];
			break;
		}

		case '1': case'2': case'3': case'4': case '5': case '6': case '7': case '8':{
			column += (fenSplit[0][i] - '0' - 1);
			break;
		}

		case '/':
			rank--;
			column = -1; //riporto a -1 perché all'inizio del ciclo la variabile column verrà ulteriormente incrementata, portando il suo valore a 0
			break;
		}
	}

	if (fenSplit[1][0] == 'w') {
		Engine::setIsWhite(true);
	}
	else {
		Engine::setIsWhite(false);
	}

	if (fenSplit[2][0] == '-') {
		Engine::setWhiteLongCastleRight(false);
		Engine::setWhiteShortCastleRight(false);
		Engine::setBlackLongCastleRight(false);
		Engine::setBlackShortCastleRight(false);
	}
	else {
		for (int i = 0; i < fenSplit[2].size(); i++) {
			switch (fenSplit[2][i]) {
			case 'K': {
				Engine::setWhiteShortCastleRight(true);
				break;
			}

			case 'Q': {
				Engine::setWhiteLongCastleRight(true);
				break;
			}

			case 'q': {
				Engine::setBlackLongCastleRight(true);
				break;
			}

			case 'k': {
				Engine::setBlackShortCastleRight(true);
			}
			}

		}
	}

	if (fenSplit[3].size() == 1) {
		Engine::setEnPassantSquare(-1); //-1 equivale a dire che non è possibile effettuare un en passant
	}
	else {
		Engine::setEnPassantSquare((fenSplit[3][0] - 'a') + (fenSplit[3][1] - '1') * 8);
	}

	Engine::setHalfMoveClock(std::stoi(fenSplit[4]));
	Engine::setFullMoveClock(std::stoi(fenSplit[5]));
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

std::vector<uint64_t> Board::getBitBoards() {
	std::vector<uint64_t> bitboards;
	for (int i = 0; i < 8; i++) {
		bitboards.push_back(m_bitBoards[i]);
	}

	return bitboards;
}