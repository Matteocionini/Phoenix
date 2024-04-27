#include <iostream>

#include "board.h"
#include "boardHelper.h"
#include "engineUtils.h"
#include "uciHandler.h"
#include "engine.h"

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
	m_prevChars.prevPieceOnEndSquare = -1;

	startSquare = (move[0] - 'a') + (move[1] - '1') * 8;
	endSquare = (move[2] - 'a') + (move[3] - '1') * 8;

	

	//TODO: identificate la casella iniziale e quella finale e salvate le informazioni irreversibili della posizione precedente, ora � necessario fare la mossa
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

	if (bitboardIndexEnd != -1) { // se c'e' un pezzo nella casella di arrivo lo tolgo (� una cattura) e salvo questa informazione per poter tornare indietro
		m_prevChars.prevPieceOnEndSquare = bitboardIndexEnd;

		pieceColorEnd = !pieceColorStart; //ovvero prendi il colore opposto a quello dela casella di partenza, in quanto non � legale catturare il proprio pezzo
		m_bitBoards[bitboardIndexEnd] = m_bitBoards[bitboardIndexEnd] = m_bitBoards[bitboardIndexEnd] ^ ((uint64_t)1 << endSquare); 
		m_bitBoards[pieceColorEnd] = m_bitBoards[pieceColorEnd] ^ ((uint64_t)1 << endSquare);
	}


	m_bitBoards[bitboardIndexStart] = (m_bitBoards[bitboardIndexStart] ^ ((uint64_t)1 << startSquare)) | ((uint64_t)1 << endSquare); //tolgo il pezzo dalla casella di partenza e lo metto in quella di arrivo
	m_bitBoards[pieceColorStart] = (m_bitBoards[pieceColorStart] ^ ((uint64_t)1 << startSquare)) | ((uint64_t)1 << endSquare);

	if (endSquare == Engine::engineData.m_enPassantSquare && bitboardIndexStart == nPawns) { //se la mossa che � appena stata fatta e' un en passant
		Engine::engineData.m_enPassantSquare = -1;
		
		if (pieceColorStart == nBlack) { //se il pezzo che si � mosso � nero, il pezzo da rimuovere risieder� nella riga sopra rispetto alla casella di arrivo del pedone che si � mosso
			m_bitBoards[bitboardIndexStart] = m_bitBoards[bitboardIndexStart] ^ ((uint64_t)1 << (endSquare + 8)); //tolgo il pedone avversario dalla casella che ha subito un en passant
			m_bitBoards[!pieceColorStart] = m_bitBoards[!pieceColorStart] ^ ((uint64_t)1 << (endSquare + 8));
		}
		else {
			m_bitBoards[bitboardIndexStart] = m_bitBoards[bitboardIndexStart] ^ ((uint64_t)1 << (endSquare - 8)); //tolgo il pedone avversario dalla casella che ha subito un en passant
			m_bitBoards[!pieceColorStart] = m_bitBoards[!pieceColorStart] ^ ((uint64_t)1 << (endSquare - 8));
		}
	}

	if (move == "e1g1") { //ovvero se la mossa e' un arrocco corto per il bianco
		m_bitBoards[nRooks] = (m_bitBoards[nRooks] ^ ((uint64_t)1 << 7)) | ((uint64_t)1 << 5);
		m_bitBoards[nWhite] = (m_bitBoards[nWhite] ^ ((uint64_t)1 << 7)) | ((uint64_t)1 << 5);
		Engine::engineData.m_whiteCanCastleShort = false;
	}
	else if (move == "e1c1") { //ovvero se la mossa e' un arrocco lungo per il bianco
		m_bitBoards[nRooks] = (m_bitBoards[nRooks] ^ ((uint64_t)1 << 0)) | ((uint64_t)1 << 3);
		m_bitBoards[nWhite] = (m_bitBoards[nWhite] ^ ((uint64_t)1 << 0)) | ((uint64_t)1 << 3);
		Engine::engineData.m_whiteCanCastleLong = false;
	}
	else if (move == "e8g8") { //se la mossa e' un arrocco corto per il nero
		m_bitBoards[nRooks] = (m_bitBoards[nRooks] ^ ((uint64_t)1 << 63)) | ((uint64_t)1 << 61);
		m_bitBoards[nBlack] = (m_bitBoards[nBlack] ^ ((uint64_t)1 << 63)) | ((uint64_t)1 << 61);
		Engine::engineData.m_blackCanCastleShort = false;
	}
	else if (move == "e8c8") { //se la mossa e' un arrocco lungo per il nero
		m_bitBoards[nRooks] = (m_bitBoards[nRooks] ^ ((uint64_t)1 << 56)) | ((uint64_t)1 << 59);
		m_bitBoards[nBlack] = (m_bitBoards[nBlack] ^ ((uint64_t)1 << 56)) | ((uint64_t)1 << 59);
		Engine::engineData.m_blackCanCastleShort = false;
	}


	//aggiornamento relativo alle informazioni sulla posizione corrente, dopo aver eseguito la mossa
	Engine::engineData.m_enPassantSquare = -1;

	if (bitboardIndexStart == nPawns && abs(startSquare - endSquare) == 16) { //se un pedone � stato spinto di due quadrati, segnalo in quale casella e' possibile fare en passant
		if (pieceColorStart == nWhite) {
			Engine::engineData.m_enPassantSquare = endSquare - 8;
		}
		else {
			Engine::engineData.m_enPassantSquare = endSquare + 8;
		}

		//std::cout << "Casella di en passant: " << Engine::engineData.m_enPassantSquare;
	}
	else if (endSquare == 0 || (startSquare == 0 && bitboardIndexStart == nRooks && pieceColorStart == nWhite)) {
		Engine::engineData.m_whiteCanCastleLong = false;
		//std::cout << "Tolto il privilegio di arrocco lungo del bianco\n";
	}
	else if (endSquare == 7 || (startSquare == 7 && bitboardIndexStart == nRooks && pieceColorStart == nWhite)) {
		Engine::engineData.m_whiteCanCastleShort = false;
		//std::cout << "Tolto il privilegio di arrocco corto del bianco\n";
	}
	else if (endSquare == 56 || (startSquare == 56 && bitboardIndexStart == nRooks && pieceColorStart == nBlack)) {
		Engine::engineData.m_blackCanCastleLong = false;
		//std::cout << "Tolto il privilegio di arrocco lungo del nero\n";
	}
	else if (endSquare == 63 || (startSquare == 63 && bitboardIndexStart == nRooks && pieceColorStart == nBlack)) {
		//std::cout << "Tolto il privilegio di arrocco corto del nero\n";
		Engine::engineData.m_blackCanCastleShort = false;
	}
	else if (bitboardIndexStart == nKings) {
		if (pieceColorStart == nWhite) {
			Engine::engineData.m_whiteCanCastleLong = false;
			Engine::engineData.m_whiteCanCastleShort = false;
			//std::cout << "Rimossi tutti i diritti di arrocco del bianco\n";
		}
		else {
			Engine::engineData.m_blackCanCastleLong = false;
			Engine::engineData.m_blackCanCastleShort = false;
			//std::cout << "Rimossi tutti i diritti di arrocco del nero\n";
		}
	}

	Engine::engineData.m_isWhite = !Engine::engineData.m_isWhite;
	//std::cout << "Deve giocare il bianco: " << Engine::engineData.m_isWhite << std::endl;
	BoardHelper::printBoard();
}

void Board::resetBoard() {
	setPosition("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
}

void Board::setPosition(std::string fenstring) {
	std::vector<std::string> fenSplit = uciHandler::split(fenstring);
	int rank = 7, column = 0;

	for (int i = 0; i < nBitboards; i++) { //prima di impostare ogni posizione � necessario pulire tutte le bitboard, in quanto se ci� non viene fatto la posizione attuale verr� "sovrapposta" alla posizione precedente
		m_bitBoards[i] = 0;
	}

	for (int i = 0; i < fenSplit[0].size(); i++, column++) {

		switch (fenSplit[0][i]) {
		case 'r': { //� una torre nera
			m_bitBoards[nRooks] = (((m_bitBoards[nRooks] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_bitBoards[nRooks];
			m_bitBoards[nBlack] = (((m_bitBoards[nBlack] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_bitBoards[nBlack];
			break;
		}

		case 'n': { //� un cavallo nero
			m_bitBoards[nKnights] = (((m_bitBoards[nKnights] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_bitBoards[nKnights];
			m_bitBoards[nBlack] = (((m_bitBoards[nBlack] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_bitBoards[nBlack];
			break;
		}

		case 'b': { //� un alfiere nero
			m_bitBoards[nBishops] = (((m_bitBoards[nBishops] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_bitBoards[nBishops];
			m_bitBoards[nBlack] = (((m_bitBoards[nBlack] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_bitBoards[nBlack];
			break;
		}

		case 'q': { //� una regina nera
			m_bitBoards[nQueens] = (((m_bitBoards[nQueens] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_bitBoards[nQueens];
			m_bitBoards[nBlack] = (((m_bitBoards[nBlack] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_bitBoards[nBlack];
			break;
		}

		case 'k': { //� un re nero
			m_bitBoards[nKings] = (((m_bitBoards[nKings] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_bitBoards[nKings];
			m_bitBoards[nBlack] = (((m_bitBoards[nBlack] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_bitBoards[nBlack];
			break;
		}

		case 'p': { //� un pedone nero
			m_bitBoards[nPawns] = (((m_bitBoards[nPawns] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_bitBoards[nPawns];
			m_bitBoards[nBlack] = (((m_bitBoards[nBlack] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_bitBoards[nBlack];
			break;
		}

		case 'R': { //� una torre bianca
			m_bitBoards[nRooks] = (((m_bitBoards[nRooks] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_bitBoards[nRooks];
			m_bitBoards[nWhite] = (((m_bitBoards[nWhite] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_bitBoards[nWhite];
			break;
		}

		case 'N': { //� un cavallo bianco
			m_bitBoards[nKnights] = (((m_bitBoards[nKnights] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_bitBoards[nKnights];
			m_bitBoards[nWhite] = (((m_bitBoards[nWhite] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_bitBoards[nWhite];
			break;
		}

		case 'B': { //� un alfiere bianco
			m_bitBoards[nBishops] = (((m_bitBoards[nBishops] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_bitBoards[nBishops];
			m_bitBoards[nWhite] = (((m_bitBoards[nWhite] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_bitBoards[nWhite];
			break;
		}

		case 'Q': { //� una regina bianca
			m_bitBoards[nQueens] = (((m_bitBoards[nQueens] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_bitBoards[nQueens];
			m_bitBoards[nWhite] = (((m_bitBoards[nWhite] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_bitBoards[nWhite];
			break;
		}

		case 'K': { //� il re bianco
			m_bitBoards[nKings] = (((m_bitBoards[nKings] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_bitBoards[nKings];
			m_bitBoards[nWhite] = (((m_bitBoards[nWhite] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_bitBoards[nWhite];
			break;
		}

		case 'P': { //� il pedone bianco
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
			column = -1; //riporto a -1 perch� all'inizio del ciclo la variabile column verr� ulteriormente incrementata, portando il suo valore a 0
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
		Engine::engineData.m_enPassantSquare = -1; //-1 equivale a dire che non � possibile effettuare un en passant
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
	if (move.length() < 4 || move[0] < 'a' || move[0] > 'h' || move[2] < 'a' || move[2] > 'h' 
		|| move[1] < '1' || move[1] > '8' || move[3] < '1' || move[3] > '8') {
		return false;
	}

	if (move.length() == 5 && (move[4] != 'q' && move[4] != 'r' && move[4] != 'b' && move[4] != 'n')) {
		return false;
	}

	return true;
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
