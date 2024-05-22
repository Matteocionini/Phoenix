#include <iostream>
#include <memory>

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
	int depth; //variabile in cui � salvata la profondit� di ricerca massima consentita per la ricerca corrente

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

std::shared_ptr<std::vector<uint16_t>> Engine::generateLegalMoves(const Position& position, bool isWhite) {
	uint64_t blockerBitboard = 0; //bitboard contenente tutti i pezzi sulla scacchiera, da passare alle varie funzioni di generazione mosse per generare le mosse pseudolegali
	int friendlyPieces = isWhite ? nWhite : nBlack; //variabile utilizzata per sapere quale tra la bitboard dei pezzi neri e quella dei pezzi bianchi � da considerare come bitboard dei pezzi alleati
	std::shared_ptr<std::vector<uint16_t>> moveList(new std::vector<uint16_t>); //lista delle mosse legali generate. E' necessario che questo vettore sia allocato sull'heap in quanto pu� anche diventare molto grande
	uint64_t currentBitboard; //bitboard corrente che si sta considerando per generare le mosse
	uint64_t moves; //bitboard utilizzata per memorizzare temporaneamente le mosse generate da una specifica funzione di generazione mosse pseudo-legali
	int kingSquare; //bitboard utilizzata per memorizzare la casella in cui si trova il re
	uint16_t move = 0; //bitboard contenente temporaneamente una mossa
	int pieceType;

	

	moveList->reserve(sizeof(uint16_t) * 50000);

	for (kingSquare = 0; kingSquare < 64; kingSquare++) { //ciclo che va alla ricerca della casella contenente il re
		if (((position.bitboards[nKings] & position.bitboards[friendlyPieces]) >> kingSquare) & 1) {
			break;
		}
	}

	for (int i = 0; i <= nBlack; i++) { //per generare la bitboard generale, � sufficiente effettuare un | tra la bitboard contenente i pezzi neri e quella contenente i pezzi bianchi
		blockerBitboard |= position.bitboards[i];
	}

	//std::cout << "La posizione corrente e' scacco: " << isKingInCheck(isWhite, position, friendlyPieces, blockerBitboard) << std::endl;

	//inizio generazione mosse legali
	for (int i = 0; i < 64 && (position.bitboards[friendlyPieces] >> i) != 0; i++) { //la seconda condizione permette un'uscita anticipata dal ciclo nel caso in cui tutti i pezzi alleati siano gi� stati processati
		switch ((position.bitboards[friendlyPieces] >> i) & 1) {
		case 0: {
			break;
		}
		case 1: {
			for (pieceType = 2; pieceType < nBitboards; pieceType++) { //identifico il tipo di pezzo che si sta muovendo
				if ((position.bitboards[pieceType] >> i) & 1) {
					break;
				}
			}

			switch (pieceType) { //genero le mosse per il pezzo che si deve muovere
			case nPawns: {
				moves = Board::pawnMoves(i, blockerBitboard, isWhite);
				break;
			}
			case nBishops: {
				moves = Board::bishopMoves(i, blockerBitboard);
				break;
			}
			case nQueens: {
				moves = Board::queenMoves(i, blockerBitboard);
				break;
			}
			case nKings: {
				moves = Board::kingMoves(i);
				break;
			}
			case nKnights: {
				moves = Board::knightMoves(i);
				break;
			}
			case nRooks: {
				moves = Board::rookMoves(i, blockerBitboard);
				break;
			}
			}

			moves = moves ^ (position.bitboards[friendlyPieces] & moves); //le funzioni di generazione di mosse pseudocasuali trattano tutti i pezzi come fossero pezzi nemici. Effettuando un xor tra bitboard dei pezzi alleati e bitboard delle mosse possibili, rimuovo la cattura dei pezzi alleati dalle mosse possibili

			std::vector<uint16_t> legalMoves = getLegalMovesFromPossibleSquaresBitboard(moves, friendlyPieces, blockerBitboard, pieceType, i, isWhite, kingSquare); //genero le mosse legali a partire dalla bitboard delle mosse pseudolegali

			moveList->insert(moveList->end(), legalMoves.begin(), legalMoves.end()); //aggiungo le mosse appena generate alla lista totale di mosse legali
		}
		}
	}

	//std::cout << "Si puo' arroccare lungo: " << kingCanCastleLong(isWhite, position, friendlyPieces, blockerBitboard) << std::endl;
	//std::cout << "Si puo' arroccare corto: " << kingCanCastleShort(isWhite, position, friendlyPieces, blockerBitboard) << std::endl;

	//la funzione kingMoves di default non restituisce le eventuali mosse di arrocco possibili, per cui esse vengono aggiunte ora
	if (kingCanCastleLong(isWhite, position, friendlyPieces, blockerBitboard)) {
		move = 0;
		move |= (kingSquare - 2) << moveEndSquareOffset;
		move |= (kingSquare) << moveStartSquareOffset;
		move |= (none) << movePromotionPieceOffset;

		Board::makeMove(move);
		Position positionAfterMove;

		if (lastMoveWasCaptureOrCheck(isWhite, positionAfterMove, friendlyPieces)) {
			move |= 1 << moveIsCaptureOrCheckOrPromotionOffset;
		}

		moveList->push_back(move);
	}

	if (kingCanCastleShort(isWhite, position, friendlyPieces, blockerBitboard)) {
		move = 0;
		move |= (kingSquare - 2) << moveEndSquareOffset;
		move |= (kingSquare) << moveStartSquareOffset;
		move |= (none) << movePromotionPieceOffset;

		Board::makeMove(move);
		Position positionAfterMove;

		if (lastMoveWasCaptureOrCheck(isWhite, positionAfterMove, friendlyPieces)) {
			move |= 1 << moveIsCaptureOrCheckOrPromotionOffset;
		}

		moveList->push_back(move);
	}

	return moveList;
}

uint64_t Engine::perft(int depth, bool first) {
	uint64_t moveCount = 0; //contatore delle mosse generate fin'ora
	std::shared_ptr<std::vector<uint16_t>> generatedMoves(new std::vector<uint16_t>); //vettore in cui memorizzo le ultime mosse generate
	uint64_t count;

	if (depth == 0) {
		return 1;
	}

	generatedMoves = Engine::generateLegalMoves(Board::getCurrentPosition(), Engine::engineData.m_isWhite);

	for (uint16_t move : *generatedMoves) {
		Board::makeMove(move);
		count = perft(depth - 1, false);
		moveCount += count;
		Board::unmakeMove(move);
		if (first) {
			int startSquare = (move >> moveStartSquareOffset) & moveStartSquareBitmask;
			int endSquare = (move >> moveEndSquareOffset) & moveEndSquareBitMask;
			char fileStartSquare = 'a' + (startSquare % 8);
			char rankStartSquare = '1' + (startSquare - (startSquare % 8)) / 8;
			char fileEndSquare = 'a' + (endSquare % 8);
			char rankEndSquare = '1' + (endSquare - (endSquare % 8)) / 8;
			std::cout << fileStartSquare << rankStartSquare << fileEndSquare << rankEndSquare << ": " << count << std::endl;
		}
	}

	return moveCount;
}

bool Engine::isKingInCheck(const bool& isWhite, const Position& position, const int& friendlyPieces, const uint64_t& blockerBitboard, int kingSquare) {
	uint64_t kingBitboard = position.bitboards[friendlyPieces] & position.bitboards[nKings]; //bitboard il cui unico bit diverso da 0 � quello relativo alla casella contenente il re alleato
	uint64_t enemyPieces = position.bitboards[!friendlyPieces]; //bitboard contenente soltanto i pezzi nemici
	uint64_t dangerousPieces = 0; //bitboard contenente i pezzi pericolosi (es. lungo una diagonale sono pericolosi un alfiere o una regina nemici...)
	uint64_t dangerousSquares = 0; //bitboard contenente le caselle controllate attualmente

	if (kingSquare == -1) { //se kingSquare � uguale a -1, vuol dire che il re si � appena mosso, per cui devo ricalcolare il quadrato in cui si trova il re
		for (kingSquare = 0; kingSquare < 64; kingSquare++) { //ciclo che va alla ricerca della casella contenente il re
			if ((kingBitboard >> kingSquare) & 1) {
				break;
			}
		}
	}

	//inizio controllando le diagonali libere verso il re
	dangerousSquares = Board::bishopMoves(kingSquare, blockerBitboard); //questa bitboard ha i bit impostati ad 1 solo lungo le diagonali libere che conducono verso il re
	dangerousPieces = (position.bitboards[nBishops] | position.bitboards[nQueens]) & enemyPieces; //bitboard contenente i pezzi pericolosi lungo le diagonali, ovvero alfieri e regine nemici

	if ((dangerousPieces & dangerousSquares) != 0) { //se le due bitboard precedentemente impostate presentano un bit acceso in comune, vi � un pezzo che tiene sotto scacco il re
		return true;
	}

	//controllo ora le "righe" libere verso il re
	dangerousSquares = Board::rookMoves(kingSquare, blockerBitboard); //questa bitboard ha i bit impostati ad 1 solo lungo le righe libere che conducono verso il re
	dangerousPieces = 0 | ((position.bitboards[nRooks] | position.bitboards[nQueens]) & enemyPieces); //bitboard contenente solo i pezzi pericolosi lungo le righe, ovvero regine e torri nemiche

	if ((dangerousPieces & dangerousSquares) != 0) { //se le due bitboard precedentemente impostate presentano un bit acceso in comune, vi � un pezzo che tiene sotto scacco il re
		return true;
	}

	//controllo ora le caselle da cui il pedone potrebbe essere messo sotto scacco da un re
	if (friendlyPieces == nWhite && kingSquare < 48) { //se il re � bianco ed � sotto alla riga 7
		dangerousSquares = 0 | (((uint64_t)1 << (kingSquare + 9)) | ((uint64_t)1 << (kingSquare + 7))); //le caselle pericolose sono quella in alto a sinistra e quella in alto a destra del re
	}
	else if (friendlyPieces == nBlack && kingSquare > 15) { //se il re � nero ed � sopra alla riga 2
		dangerousSquares = 0 | (((uint64_t)1 << (kingSquare - 9)) | ((uint64_t)1 << (kingSquare - 7))); //le caselle pericolose sono quella in basso a sinistra e quella in basso a destra del re
	}
	else { //se si arriva qui, il re non pu� essere messo sotto scacco da un pedone
		dangerousSquares = 0;
	}

	dangerousPieces = 0 | (position.bitboards[nPawns] & enemyPieces);

	if ((dangerousPieces & dangerousSquares) != 0) { //se le due bitboard precedentemente impostate presentano un bit acceso in comune, vi � un pezzo che tiene sotto scacco il re
		return true;
	}

	//controllo ora se il re pu� essere messo sotto attacco da un cavallo
	dangerousSquares = Board::knightMoves(kingSquare);
	dangerousPieces = 0 | (position.bitboards[nKnights] & enemyPieces);

	if ((dangerousPieces & dangerousSquares) != 0) { //se le due bitboard precedentemente impostate presentano un bit acceso in comune, vi � un pezzo che tiene sotto scacco il re
		return true;
	}

	//controllo, infine, se il re � sotto scacco da parte del re nemico
	dangerousSquares = Board::kingMoves(kingSquare);
	dangerousPieces = 0 | (position.bitboards[nKings] & enemyPieces);

	if ((dangerousPieces & dangerousSquares) != 0) { //se le due bitboard precedentemente impostate presentano un bit acceso in comune, vi � un pezzo che tiene sotto scacco il re
		return true;
	}

	return false;
}

bool Engine::kingCanCastleLong(const bool& isWhite, const Position& position, const int& friendlyPieces, const uint64_t& blockerBitboard) {
	if (isWhite) {
		if (!engineData.m_whiteCanCastleLong) {
			return false;
		}
		uint64_t squaresTraversed = ((0 | ((uint64_t)1) << 3) | ((uint64_t)1 << 2)) | ((uint64_t)1 << 1);

		if ((squaresTraversed & blockerBitboard) != 0) {
			return false;
		}
	}
	else {
		if (!engineData.m_blackCanCastleLong) {
			return false;
		}
		uint64_t squaresTraversed = ((0 | ((uint64_t)1) << 59) | ((uint64_t)1 << 58)) | ((uint64_t)1 << 57);

		if ((squaresTraversed & blockerBitboard) != 0) {
			return false;
		}
	}

	int kingSquare = isWhite ? 4 : 60;

	if (isKingInCheck(isWhite, position, friendlyPieces, blockerBitboard, kingSquare) || isKingInCheck(isWhite, position, friendlyPieces, blockerBitboard, kingSquare - 1) || isKingInCheck(isWhite, position, friendlyPieces, blockerBitboard, kingSquare - 2)) {
		return false;
	}

	return true;
}
bool Engine::kingCanCastleShort(const bool& isWhite, const Position& position, const int& friendlyPieces, const uint64_t& blockerBitboard) {
	if (isWhite) {
		if (!engineData.m_whiteCanCastleLong) {
			return false;
		}
		uint64_t squaresTraversed = (0 | ((uint64_t)1) << 5) | ((uint64_t)1 << 6);

		if ((squaresTraversed & blockerBitboard) != 0) {
			return false;
		}
	}
	else {
		if (!engineData.m_blackCanCastleLong) {
			return false;
		}
		uint64_t squaresTraversed = (0 | ((uint64_t)1) << 61) | ((uint64_t)1 << 62);

		if ((squaresTraversed & blockerBitboard) != 0) {
			return false;
		}
	}

	int kingSquare = isWhite ? 4 : 60;

	if (isKingInCheck(isWhite, position, friendlyPieces, blockerBitboard, kingSquare) || isKingInCheck(isWhite, position, friendlyPieces, blockerBitboard, kingSquare + 1) || isKingInCheck(isWhite, position, friendlyPieces, blockerBitboard, kingSquare + 2)) {
		return false;
	}

	return true;
}

std::vector<uint16_t> Engine::getLegalMovesFromPossibleSquaresBitboard(const uint64_t& moves, const int& friendlyPieces, const uint64_t& blockerBitboard, const int& pieceType, const int& startSquare, const bool& isWhite, const int& kingSquare) {
	std::vector<uint16_t> moveList; //vettore che verr� restituito in cui inserisco tutte le mosse legali trovate
	uint16_t move = 0; //bitboard in cui costruisco e salvo temporaneamente le mosse legali
	int actualKingSquare = pieceType == nKings ? -1 : kingSquare;

	for (int i = 0; i < 64 && (moves >> i) != 0; i++) { //itero su tutte le caselle della scacchiera
		switch ((moves >> i) & 1) { //se il quadrato corrente � un quadrato in cui � possibile muovere il pezzo in oggetto
		case 0: {
			break;
		}
		case 1: {
			if (pieceType == nPawns && ((isWhite && i >= 56) || (!isWhite && i <= 7))) { //se il pezzo che si muove � un pedone che deve essere promosso, genero tutte le promozioni
				for (int j = 0; j < 5; j++) {
					move = 0;
					move |= startSquare << moveStartSquareOffset; //la casella di partenza � la stessa per tutte le mosse
					move |= i << moveEndSquareOffset; //imposto la casella di arrivo
					move |= j << movePromotionPieceOffset; //imposto il pezzo a cui il pedone sar� promosso
					Board::makeMove(move); //faccio la mossa

					Position positionAfterMove = Board::getCurrentPosition(); //variabile in cui � contenuta la posizione dopo la mossa

					if (!Engine::isKingInCheck(isWhite, positionAfterMove, friendlyPieces, positionAfterMove.bitboards[nBlack] | positionAfterMove.bitboards[nWhite], actualKingSquare)) { //se il re alleato non � sotto scacco, la mossa � legale
						move |= 1 << moveIsCaptureOrCheckOrPromotionOffset;
						moveList.push_back(move);
					}

					Board::unmakeMove(move);
				}
				
			}
			else {
				move = 0;
				move |= startSquare << moveStartSquareOffset; //la casella di partenza � la stessa per tutte le mosse
				move |= i << moveEndSquareOffset; //imposto la casella di arrivo
				move |= none << movePromotionPieceOffset; //imposto il pezzo a cui il pedone sar� promosso
				Board::makeMove(move); //faccio la mossa

				Position positionAfterMove = Board::getCurrentPosition(); //variabile in cui � contenuta la posizione dopo la mossa

				if (!Engine::isKingInCheck(isWhite, positionAfterMove, friendlyPieces, positionAfterMove.bitboards[nBlack] | positionAfterMove.bitboards[nWhite], actualKingSquare)) { //se il re alleato non � sotto scacco, la mossa � legale
					if (Engine::lastMoveWasCaptureOrCheck(isWhite, positionAfterMove, friendlyPieces)) { //controllo se la mossa precedente � stata una cattura o ha messo sotto scacco il re avversario, ed in caso lo segno
						move |= 1 << moveIsCaptureOrCheckOrPromotionOffset;
					}
					moveList.push_back(move);
				}

				Board::unmakeMove(move);
			}
			
			break;
		}
		}
	}

	return moveList;
}

bool Engine::lastMoveWasCaptureOrCheck(const bool& isWhite, const Position& positionAfterMove, const int& friendlyPieces) {
	if (Engine::isKingInCheck(!isWhite, positionAfterMove, !friendlyPieces, positionAfterMove.bitboards[nBlack] | positionAfterMove.bitboards[nWhite], -1) || ((Board::m_previousPositionCharacteristics.top() >> prevPieceOnEndSquareOffset) & prevPieceOnEndSquareBitMask)) {
		return true;
	}
	else {
		return false;
	}
}