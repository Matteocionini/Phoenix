#include <iostream>

#include "engine.h"

EngineData Engine::engineData; //struct contenente i dati del motore

void Engine::engineInit() {
	mtxReady.lock();
	isReady = false;
	mtxReady.unlock();

	engineData.m_debugMode = false;
	engineData.m_ponderMode = false;
	engineData.m_maxDepth = -1;
	engineData.m_maxNodes = -1;
	engineData.m_stop = false;
	engineData.m_lookForMate = false;
	engineData.m_movesToMate = -1;
	engineData.m_hashTableSize = 3;
	engineData.m_infinite = false;
	engineData.m_restrictSearch = false;
	engineData.m_moveList.clear();
	engineData.m_wTime = 0;
	engineData.m_bTime = 0;
	engineData.m_wInc = 0;
	engineData.m_bInc = 0;
	engineData.m_movesToGo = -1;
	engineData.m_moveTime = -1;
	engineData.m_whiteCanCastleLong = false;
	engineData.m_whiteCanCastleShort = false;
	engineData.m_blackCanCastleLong = false;
	engineData.m_blackCanCastleShort = false;
	engineData.m_enPassantSquare = -1;


	mtxReady.lock();
	isReady = true;
	mtxReady.unlock();
}

void Engine::startSearchAndEval() {
	int depth; //variabile in cui è salvata la profondità di ricerca massima consentita per la ricerca corrente

	if (engineData.m_maxDepth != -1) {
		depth = engineData.m_maxDepth;
	}
	else {
		depth = maxAllowedDepth;
	}

	//reset di tutte le variabili search-specific
	engineData.m_moveList.clear();
	engineData.m_maxDepth = -1;
	engineData.m_maxNodes = -1;
	engineData.m_movesToMate = -1;
	engineData.m_moveTime = -1;
	engineData.m_enPassantSquare = -1;
	engineData.m_moveList.clear();
	engineData.m_stop = false;
	engineData.m_ponderMode = false;
	engineData.m_lookForMate = false;
	engineData.m_infinite = false;
	engineData.m_restrictSearch = false;

	Board::resetPreviousPositionCharacteristics();
}

std::vector<uint16_t> Engine::generateLegalMoves(const Position& position, bool isWhite) {
	uint64_t blockerBitboard = 0; //bitboard contenente tutti i pezzi sulla scacchiera, da passare alle varie funzioni di generazione mosse per generare le mosse pseudolegali
	int friendlyPieces = isWhite ? nWhite : nBlack; //variabile utilizzata per sapere quale tra la bitboard dei pezzi neri e quella dei pezzi bianchi è da considerare come bitboard dei pezzi alleati
	std::vector<uint16_t> moveList; //lista delle mosse legali generate
	uint64_t currentBitboard; //bitboard corrente che si sta considerando per generare le mosse
	uint64_t moves; //bitboard utilizzata per memorizzare temporaneamente le mosse generate da una specifica funzione di generazione mosse pseudo-legali

	for (int i = 0; i <= nBlack; i++) { //per generare la bitboard generale, è sufficiente effettuare un | tra la bitboard contenente i pezzi neri e quella contenente i pezzi bianchi
		blockerBitboard |= position.bitboards[i];
	}

	//generazione delle mosse legali per le torri
	currentBitboard = position.bitboards[friendlyPieces] & position.bitboards[nRooks]; //bitboard contenente solo le torri alleate

	//std::cout << "La posizione corrente e' scacco: " << isKingInCheck(isWhite, position, friendlyPieces, blockerBitboard) << std::endl;

	for (int i = 0; i < 64; i++) {
		if ((currentBitboard >> i) & 1) { //se il bit che sto considerando è un uno, vuol dire che contiene una torre alleata
			moves = Board::rookMoves(i, blockerBitboard) ^ position.bitboards[friendlyPieces]; //facendo un xor con la bitboard contenente i pezzi alleati, faccio in modo che eventuali pezzi alleati, visti come catturabili dalla funzione di generazione mosse pseudo-legali, non lo siano per questa funzione
			for (int j = i + 8; j < 64; j += 8) { //inizio a gestire le mosse che sono degli spostamenti verticali verso l'alto

			}
		}
	}
	

	return moveList;
}

bool Engine::isKingInCheck(const bool& isWhite, const Position& position, const int& friendlyPieces, const uint64_t& blockerBitboard) {
	int kingSquare; //indica la casella in cui si trova il re
	uint64_t kingBitboard = position.bitboards[friendlyPieces] & position.bitboards[nKings]; //bitboard il cui unico bit diverso da 0 è quello relativo alla casella contenente il re alleato
	uint64_t enemyPieces = position.bitboards[!friendlyPieces]; //bitboard contenente soltanto i pezzi nemici
	uint64_t dangerousPieces = 0; //bitboard contenente i pezzi pericolosi (es. lungo una diagonale sono pericolosi un alfiere o una regina nemici...)
	uint64_t dangerousSquares = 0; //bitboard contenente le caselle controllate attualmente

	for (kingSquare = 0; kingSquare < 64; kingSquare++) { //ciclo che va alla ricerca della casella contenente il re
		if ((kingBitboard >> kingSquare) & 1) {
			break;
		}
	}

	//inizio controllando le diagonali libere verso il re
	dangerousSquares = Board::bishopMoves(kingSquare, blockerBitboard); //questa bitboard ha i bit impostati ad 1 solo lungo le diagonali libere che conducono verso il re
	dangerousPieces = (position.bitboards[nBishops] | position.bitboards[nQueens]) & enemyPieces; //bitboard contenente i pezzi pericolosi lungo le diagonali, ovvero alfieri e regine nemici

	if ((dangerousPieces & dangerousSquares) != 0) { //se le due bitboard precedentemente impostate presentano un bit acceso in comune, vi è un pezzo che tiene sotto scacco il re
		return true;
	}

	//controllo ora le "righe" libere verso il re
	dangerousSquares = Board::rookMoves(kingSquare, blockerBitboard); //questa bitboard ha i bit impostati ad 1 solo lungo le righe libere che conducono verso il re
	dangerousPieces = 0 | ((position.bitboards[nRooks] | position.bitboards[nQueens]) & enemyPieces); //bitboard contenente solo i pezzi pericolosi lungo le righe, ovvero regine e torri nemiche

	if ((dangerousPieces & dangerousSquares) != 0) { //se le due bitboard precedentemente impostate presentano un bit acceso in comune, vi è un pezzo che tiene sotto scacco il re
		return true;
	}

	//controllo ora le caselle da cui il pedone potrebbe essere messo sotto scacco da un re
	if (friendlyPieces == nWhite && kingSquare < 48) { //se il re è bianco ed è sotto alla riga 7
		dangerousSquares = 0 | (((uint64_t)1 << (kingSquare + 9)) | ((uint64_t)1 << (kingSquare + 7))); //le caselle pericolose sono quella in alto a sinistra e quella in alto a destra del re
	}
	else if (friendlyPieces == nBlack && kingSquare > 15) { //se il re è nero ed è sopra alla riga 2
		dangerousSquares = 0 | (((uint64_t)1 << (kingSquare - 9)) | ((uint64_t)1 << (kingSquare - 7))); //le caselle pericolose sono quella in basso a sinistra e quella in basso a destra del re
	}
	else { //se si arriva qui, il re non può essere messo sotto scacco da un pedone
		dangerousSquares = 0;
	}

	dangerousPieces = 0 | (position.bitboards[nPawns] & enemyPieces);

	if ((dangerousPieces & dangerousSquares) != 0) { //se le due bitboard precedentemente impostate presentano un bit acceso in comune, vi è un pezzo che tiene sotto scacco il re
		return true;
	}

	//controllo ora se il re può essere messo sotto attacco da un cavallo
	dangerousSquares = Board::knightMoves(kingSquare);
	dangerousPieces = 0 | (position.bitboards[nKnights] & enemyPieces);

	if ((dangerousPieces & dangerousSquares) != 0) { //se le due bitboard precedentemente impostate presentano un bit acceso in comune, vi è un pezzo che tiene sotto scacco il re
		return true;
	}

	return false;
}
