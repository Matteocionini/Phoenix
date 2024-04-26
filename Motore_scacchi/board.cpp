#include <iostream>

#include "board.h"
#include "boardHelper.h"
#include "engineUtils.h"
#include "uciHandler.h"

uint64_t Board::m_bitBoards[nBitboards] = { 0 };
previousPositionCharacteristics Board::m_prevChars;

void Board::makeMove(std::string move) { //la mossa viene fornita nel formato <col><rank><col><rank>
	int startSquare, endSquare, bitboardIndexStart, bitboardIndexEnd = -1;
	int pieceColorStart, pieceColorEnd;

	//prima di fare effettivamente la mossa, salva le caratteristiche irreversibili della posizione corrente per poter poi ritornare a questa posizione
	m_prevChars.blackLongCastleRights = Engine::engineData.m_blackCanCastleLong;
	m_prevChars.blackShortCastleRights = Engine::engineData.m_blackCanCastleShort;
	m_prevChars.whiteLongCastleRights = Engine::engineData.m_whiteCanCastleLong;
	m_prevChars.whiteShortCastleRights = Engine::engineData.m_whiteCanCastleShort;
	m_prevChars.isWhite = Engine::engineData.m_isWhite;

	m_prevChars.halfMoveClock = Engine::engineData.m_halfMoveClock;
	m_prevChars.fullMoveClock = Engine::engineData.m_fullMoveClock;
	m_prevChars.enPassantTargetSquare = Engine::engineData.m_enPassantSquare;

	startSquare = (move[0] - 'a') + (move[1] - '1') * 8;
	endSquare = (move[2] - 'a') + (move[3] - '1') * 8;

	//TODO: identificate la casella iniziale e quella finale e salvate le informazioni irreversibili della posizione precedente, ora è necessario fare la mossa
	for (int i = 0; i < nBitboards; i++) { //grazie a questo ciclo for posso identificare di che tipo il pezzo sulla casella di partenza e quello sulla casella di arrivo sono
		if ((m_bitBoards[i] >> startSquare) & 1) { 
			if (i >= 2) { //le bitboard di indice da 2 in poi sono le bitboard specifiche dei pezzi, le altre 2 sono le bitboard che identificano il colore del pezzo
				bitboardIndexStart = i;
			}
			else {
				pieceColorStart = i; //la bitboard contenente i pezzi bianchi ha indice 0
			}
		}
		
		if (((m_bitBoards[i] >> endSquare) & 1) && i >= 2) {
			bitboardIndexEnd = i;
		}
	}

	if (bitboardIndexEnd != -1) { // se c'e' un pezzo nella casella di arrivo lo tolgo (è una cattura)
		pieceColorEnd = !pieceColorStart; //ovvero prendi il colore opposto a quello dela casella di partenza, in quanto non è legale catturare il proprio pezzo
		m_bitBoards[bitboardIndexEnd] = m_bitBoards[bitboardIndexEnd] = m_bitBoards[bitboardIndexEnd] ^ ((uint64_t)1 << endSquare); 
		m_bitBoards[pieceColorEnd] = m_bitBoards[pieceColorEnd] ^ ((uint64_t)1 << endSquare);
	}


	m_bitBoards[bitboardIndexStart] = m_bitBoards[bitboardIndexStart] ^ ((uint64_t)1 << startSquare); //tolgo il pezzo dalla casella di partenza
	m_bitBoards[pieceColorStart] = m_bitBoards[pieceColorStart] ^ ((uint64_t)1 << startSquare);

	m_bitBoards[bitboardIndexStart] = m_bitBoards[bitboardIndexStart] | ((uint64_t)1 << endSquare); //lo metto nella casella di arrivo
	m_bitBoards[pieceColorStart] = m_bitBoards[pieceColorStart] | ((uint64_t)1 << endSquare);
	

	BoardHelper::printBoard();
}

void Board::resetBoard() {
	setPosition("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
}

void Board::setPosition(std::string fenstring) {
	std::vector<std::string> fenSplit = uciHandler::split(fenstring);
	int rank = 7, column = 0;

	for (int i = 0; i < nBitboards; i++) { //prima di impostare ogni posizione è necessario pulire tutte le bitboard, in quanto se ciò non viene fatto la posizione attuale verrà "sovrapposta" alla posizione precedente
		m_bitBoards[i] = 0;
	}

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
		Engine::engineData.m_isWhite = true;
		m_prevChars.isWhite = true;
	}
	else {
		Engine::engineData.m_isWhite = false;
		m_prevChars.isWhite = false;
	}

	//std::cout << "Gioca il bianco? " << Engine::getIsWhite() << std::endl;

	Engine::engineData.m_whiteCanCastleLong = false;
	Engine::engineData.m_whiteCanCastleShort = false;
	Engine::engineData.m_blackCanCastleLong = false;
	Engine::engineData.m_blackCanCastleShort = false;

	if (fenSplit[2][0] != '-') {
		for (int i = 0; i < fenSplit[2].size(); i++) {
			switch (fenSplit[2][i]) {
			case 'K': {
				Engine::engineData.m_whiteCanCastleShort = true;
				break;
			}

			case 'Q': {
				Engine::engineData.m_whiteCanCastleLong = true;
				break;
			}

			case 'q': {
				Engine::engineData.m_blackCanCastleLong = true;
				break;
			}

			case 'k': {
				Engine::engineData.m_blackCanCastleShort = true;
			}
			}

		}
	}

	//std::cout << "Il bianco puo' arroccare lungo: " << Engine::getWhiteLongCastleRight() << std::endl;
	//std::cout << "Il bianco puo' arroccare corto: " << Engine::getWhiteShortCastleRight() << std::endl;
	//std::cout << "Il nero puo' arroccare lungo: " << Engine::getBlackLongCastleRight() << std::endl;
	//std::cout << "Il nero puo' arroccare corto: " << Engine::getBlackShortCastleRight() << std::endl;

	if (fenSplit[3].size() == 1) {
		Engine::engineData.m_enPassantSquare = -1; //-1 equivale a dire che non è possibile effettuare un en passant
		//std::cout << "Non e' possibile fare en passant: " << Engine::getEnPassantSquare() << std::endl;
	}
	else {
		Engine::engineData.m_enPassantSquare = (fenSplit[3][0] - 'a') + (fenSplit[3][1] - '1') * 8;
		//std::cout << "Casella bersaglio dell'en passant: " << Engine::getEnPassantSquare() << std::endl;
	}

	Engine::engineData.m_halfMoveClock = std::stoi(fenSplit[4]);
	//std::cout << "Half move clock: " << Engine::getHalfMoveClock() << std::endl;

	Engine::engineData.m_fullMoveClock = std::stoi(fenSplit[5]);
	//std::cout << "Full move clock: " << Engine::getFullMoveClock() << std::endl;
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

uint64_t Board::allPiecesBitboard() {
	uint64_t out = 0;

	for (int i = 2; i < nBitboards; i++) {
		out = out | m_bitBoards[i];
	}

	return out;
}
