#include <iostream>
#include <memory>
#include <bit>
#include <time.h>

#include "engine.h"
#include "boardHelper.h"


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

	BoardHelper::initBorderBitboards();


	mtxReady.lock();
	isReady = true;
	mtxReady.unlock();
}

void Engine::startSearchAndEval() {
	int depth = maxAllowedInitialDepth; //variabile in cui è salvata la profondità di ricerca massima consentita per la ricerca corrente
	int time = engineData.m_isWhite ? engineData.m_wTime : engineData.m_bTime; //variabile in cui è memorizzato il tempo rimasto al motore
	std::chrono::steady_clock timer; //timer utilizzato per tenere traccia del tempo trascorso dal
	auto startTime = timer.now();
	auto currTime = startTime;
	int maxDepth = 15; //profondità massima di ricerca

	//nel caso in cui vi sia una profondità di ricerca massima, la imposto
	if (engineData.m_maxDepth != -1) {
		maxDepth = engineData.m_maxDepth;
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

int Engine::miniMax(int depth, int alpha, int beta, bool isWhite) {
	moveArray moves;

	if (depth == 0) { //se il nodo corrente è un nodo foglia, ritorna la valutazione della posizione
		//return Engine::evaluate(position);
	}

	moves = generateLegalMoves(Board::getCurrentPosition(), isWhite); //genera le mosse legali disponibili per il giocatore corrente

	if (moves.getSize() == 0) { //se il numero di mosse legali disponibili è 0, il giocatore corrente è sotto scacco matto
		return isWhite ? INT_MIN : INT_MAX;
	}

	if (isWhite) { //il bianco è il giocatore che deve massimizzare il punteggio

	}
}

moveArray Engine::generateLegalMoves(Position position, bool isWhite) {
	uint64_t blockerBitboard = 0; //bitboard contenente tutti i pezzi sulla scacchiera, da passare alle varie funzioni di generazione mosse per generare le mosse pseudolegali
	int friendlyPieces = isWhite ? nWhite : nBlack; //variabile utilizzata per sapere quale tra la bitboard dei pezzi neri e quella dei pezzi bianchi è da considerare come bitboard dei pezzi alleati
	moveArray moveList; //lista delle mosse legali generate
	uint64_t moves; //bitboard utilizzata per memorizzare temporaneamente le mosse generate da una specifica funzione di generazione mosse pseudo-legali
	int kingSquare; //bitboard utilizzata per memorizzare la casella in cui si trova il re
	uint16_t move = 0; //bitboard contenente temporaneamente una mossa
	int pieceType;
	moveArray legalMoves; //vettore usato per memorizzare temporaneamente delle mosse
	uint64_t bitboardToWorkOn = position.bitboards[friendlyPieces]; //bitboard usata per iterare sulla scacchiera
	int totalShiftAmount = 0; //numero di posizioni di cui ho shiftato la bitboard su cui sto lavorando
	int i;
	int currSquare; //quadrato che sto considerando per le mosse
	uint64_t pinnedPieces = 0; //pezzi pinnati nella posizione attuale
	uint64_t dangerousSquares; //quadrati in cui un pezzo potrebbe essere pinnato
	uint64_t temp; //variabile usata temporaneamente per il calcolo dei pezzi pinnati
	bool bishop;
	uint64_t temp2;
	uint64_t pinRays[64]; //vettore in cui memorizzo i pinRay relativi ad ogni pezzo pinnato nella posizione corrente

	moveList.Reset();

	for (int i = 0; i < 64; i++) {
		pinRays[i] = UINT64_MAX; //inizializzazione del vettore pinRays
	}

	kingSquare = std::countr_zero(position.bitboards[nKings] & position.bitboards[friendlyPieces]);

	blockerBitboard = position.bitboards[nWhite] | position.bitboards[nBlack];

	dangerousSquares = Board::queenMoves(kingSquare, blockerBitboard) & position.bitboards[friendlyPieces]; //in questa bitboard sono memorizzati i quadrati che è necessario controllare per vedere se i pezzi all'interno di questi sono pinnati

	//BoardHelper::printLegalMoves(dangerousSquares);

	while (dangerousSquares) {
		i = std::countr_zero(dangerousSquares); //per ogni pezzo pericoloso
		dangerousSquares = dangerousSquares ^ ((uint64_t)1 << i);

		temp = ((Board::bishopMoves(i, blockerBitboard) & position.bitboards[!friendlyPieces])) & (position.bitboards[nBishops] | position.bitboards[nQueens]); //bitboard in cui sono memorizzate le donne/alfieri che potrebbero star pinnando il pezzo
		bishop = true;

		if (temp == 0) { //bitboard in cui sono memorizzate le torri/donne che potrebbero star pinnando il pezzo
			temp = ((Board::rookMoves(i, blockerBitboard) & position.bitboards[!friendlyPieces])) & (position.bitboards[nRooks] | position.bitboards[nQueens]);
			bishop = false;
		}

	checkPin:
		while (temp) {
			if (bishop) {
				temp2 = ((position.bitboards[friendlyPieces] & position.bitboards[nKings]) & Board::bishopMoves(std::countr_zero(temp), blockerBitboard ^ (uint64_t)1 << i));
			}
			else {
				temp2 = ((position.bitboards[friendlyPieces] & position.bitboards[nKings]) & Board::rookMoves(std::countr_zero(temp), blockerBitboard ^ (uint64_t)1 << i));
			}

			//BoardHelper::printBoard(position);
			//BoardHelper::printLegalMoves(temp);
			//BoardHelper::printLegalMoves(temp2);
			//BoardHelper::printLegalMoves(Board::bishopMoves(std::countr_zero(temp), blockerBitboard ^ (uint64_t)1 << i));

			//BoardHelper::printLegalMoves(Board::bishopMoves(i, blockerBitboard));
			//BoardHelper::printLegalMoves(pinRays[i]);

			if (bishop && temp && (Board::bishopMoves(std::countr_zero(temp), blockerBitboard) & Board::bishopMoves(kingSquare, blockerBitboard) & ((uint64_t)1 << i)) && (temp2)) { //se il pezzo è attualmente tenuto pinnato da un alfiere o una donna
				//BoardHelper::printBoard(position);
				//BoardHelper::printLegalMoves(temp);
				//BoardHelper::printLegalMoves(temp2);
				//BoardHelper::printLegalMoves(Board::bishopMoves(std::countr_zero(temp), blockerBitboard ^ (uint64_t)1 << i));

				pinnedPieces |= (uint64_t)1 << i; //aggiungo il pezzo ai pezzi pinnati

				if (pinRays[i] == UINT64_MAX) {
					pinRays[i] = 0;
				}

				pinRays[i] |= Board::bishopMoves(std::countr_zero(temp), blockerBitboard ^ (uint64_t)1 << i) & Board::bishopMoves(i, blockerBitboard) | ((uint64_t)1 << std::countr_zero(temp)); //come suo pinRay, ovvero come quadrati in cui può muoversi, inserisco tutta la diagonale tenuta sotto controllo dall'alfiere/donna avversaria, più il quadrato stesso in cui la donna/alfiere si trova (ovvero il pezzo può catturare il pezzo nemico che lo tiene inchiodato)

				//BoardHelper::printLegalMoves(Board::bishopMoves(i, blockerBitboard));
				//BoardHelper::printLegalMoves(pinRays[i]);
			}
			else if (!bishop && temp && (Board::rookMoves(std::countr_zero(temp), blockerBitboard) & Board::rookMoves(kingSquare, blockerBitboard) & ((uint64_t)1 << i)) && (temp2)) { //se il pezzo è attualmente pinnato da una donna o una torre
				//BoardHelper::printBoard(position);
				//BoardHelper::printLegalMoves(temp);
				//BoardHelper::printLegalMoves(temp2);
				//BoardHelper::printLegalMoves(Board::rookMoves(std::countr_zero(temp), blockerBitboard ^ (uint64_t)1 << i));

				pinnedPieces |= (uint64_t)1 << i; //aggiungo il pezzo ai pezzi pinnati

				if (pinRays[i] == UINT64_MAX) {
					pinRays[i] = 0;
				}

				pinRays[i] |= Board::rookMoves(std::countr_zero(temp), blockerBitboard ^ (uint64_t)1 << i) & Board::rookMoves(i, blockerBitboard) | ((uint64_t)1 << std::countr_zero(temp)); //come pinRay inserisco tutta la fila tenuta sotto controllo dalla torre/regina avversaria, più il quadrato stesso in cui il pezzo nemico si trova

				//BoardHelper::printLegalMoves(pinnedPieces);
				//BoardHelper::printLegalMoves(Board::rookMoves(i, blockerBitboard));
				//BoardHelper::printLegalMoves(pinRays[i]);
			}

			temp = temp ^ ((uint64_t)1 << std::countr_zero(temp));
		}

		if (temp == 0 && bishop) {
			temp = ((Board::rookMoves(i, blockerBitboard) & position.bitboards[!friendlyPieces])) & (position.bitboards[nRooks] | position.bitboards[nQueens]);
			bishop = false;
			goto checkPin;
		}
	}
	

	/*for (int i = 0; i <= nBlack; i++) { //per generare la bitboard generale, è sufficiente effettuare un | tra la bitboard contenente i pezzi neri e quella contenente i pezzi bianchi
		blockerBitboard |= position.bitboards[i];
	}*/

	//std::cout << "La posizione corrente e' scacco: " << isKingInCheck(isWhite, position, friendlyPieces, blockerBitboard) << std::endl;

	//inizio generazione mosse legali
	while(bitboardToWorkOn) {
		i = std::countr_zero(bitboardToWorkOn);

		currSquare = totalShiftAmount + i;
		
		for (pieceType = 2; pieceType < nBitboards; pieceType++) { //identifico il tipo di pezzo che si sta muovendo
			if ((position.bitboards[pieceType] >> (currSquare)) & 1) {
				break;
			}
		}

		switch (pieceType) { //genero le mosse per il pezzo che si deve muovere
		case nPawns: {
			moves = Board::pawnMoves(currSquare, blockerBitboard, isWhite);
			break;
		}
		case nBishops: {
			moves = Board::bishopMoves(currSquare, blockerBitboard);
			break;
		}
		case nQueens: {
			moves = Board::queenMoves(currSquare, blockerBitboard);
			break;
		}
		case nKings: {
			moves = Board::kingMoves(currSquare);
			break;
		}
		case nKnights: {
			moves = Board::knightMoves(currSquare);
			break;
		}
		case nRooks: {
			moves = Board::rookMoves(currSquare, blockerBitboard);
			break;
		}
		default: { //se arrivo qui c'è decisamente un problema
			std::cout << "Siamo alla casella: " << i << std::endl;
			//BoardHelper::printLegalMoves(position.bitboards[friendlyPieces]);
			moves = position.bitboards[friendlyPieces];
		}
		}

		moves = moves & (~position.bitboards[friendlyPieces]) & pinRays[currSquare]; //le funzioni di generazione di mosse pseudocasuali trattano tutti i pezzi come fossero pezzi nemici. Effettuando un xor tra bitboard dei pezzi alleati e bitboard delle mosse possibili, rimuovo la cattura dei pezzi alleati dalle mosse possibili. Faccio in modo, inoltre che se il pezzo è pinnato esso non possa muoversi lasciando il re in scacco

		legalMoves.Reset();

		//BoardHelper::printLegalMoves(moves);

		getLegalMovesFromPossibleSquaresBitboard(moves, friendlyPieces, blockerBitboard, pieceType, currSquare, isWhite, kingSquare, legalMoves); //genero le mosse legali a partire dalla bitboard delle mosse pseudolegali

		moveList.Append(legalMoves.Begin(), legalMoves.End()); //aggiungo le mosse appena generate alla lista totale di mosse legali

		legalMoves.Reset();
		
		bitboardToWorkOn = bitboardToWorkOn >> (i + 1);

		totalShiftAmount += i + 1;
	}

	//std::cout << "Si puo' arroccare lungo: " << kingCanCastleLong(isWhite, position, friendlyPieces, blockerBitboard) << std::endl;
	//std::cout << "Si puo' arroccare corto: " << kingCanCastleShort(isWhite, position, friendlyPieces, blockerBitboard) << std::endl;

	//la funzione kingMoves di default non restituisce le eventuali mosse di arrocco possibili, per cui esse vengono aggiunte ora
	if (kingCanCastleLong(isWhite, position, friendlyPieces, blockerBitboard)) {
		move = 0;
		move |= (kingSquare - 2) << moveEndSquareOffset;
		move |= (kingSquare) << moveStartSquareOffset;
		move |= 0 << movePromotionPieceOffset;

		moveList.pushBack(move);
	}

	if (kingCanCastleShort(isWhite, position, friendlyPieces, blockerBitboard)) {
		move = 0;
		move |= (kingSquare + 2) << moveEndSquareOffset;
		move |= (kingSquare) << moveStartSquareOffset;
		move |= 0 << movePromotionPieceOffset;

		moveList.pushBack(move);
	}

	return moveList;
}

uint64_t Engine::perft(int depth, bool first) {
	uint64_t moveCount = 0; //contatore delle mosse generate fin'ora
	moveArray generatedMoves; //vettore in cui memorizzo le ultime mosse generate
	uint64_t count;
	uint32_t move;

	//bulk-counting
	generatedMoves = Engine::generateLegalMoves(Board::getCurrentPosition(), Engine::engineData.m_isWhite);

	if (depth == 1 && !first) {
		return generatedMoves.getSize();
	}
	else if (depth == 0) {
		return 1;
	}

	for (int i = 0; i < generatedMoves.getSize(); i++) {
		move = generatedMoves.getElem(i);
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
			char promotionPiece = -1;

			switch ((move >> movePromotionPieceOffset) & movePromotionPieceBitMask) {
			case nQueens: {
				promotionPiece = 'q';
				break;
			}
			case nKnights: {
				promotionPiece = 'n';
				break;
			}
			case nBishops: {
				promotionPiece = 'b';
				break;
			}
			case nRooks: {
				promotionPiece = 'r';
				break;
			}
			}

			std::cout << fileStartSquare << rankStartSquare << fileEndSquare << rankEndSquare;
			if (promotionPiece != -1) {
				std::cout << promotionPiece;
			}

			std::cout << ": " << count << std::endl;
		}
	}

	return moveCount;
}

bool Engine::isKingInCheck(const bool& isWhite, const Position& position, const int& friendlyPieces, const uint64_t& blockerBitboard, int kingSquare) {
	uint64_t kingBitboard = position.bitboards[friendlyPieces] & position.bitboards[nKings]; //bitboard il cui unico bit diverso da 0 è quello relativo alla casella contenente il re alleato
	uint64_t enemyPieces = position.bitboards[!friendlyPieces]; //bitboard contenente soltanto i pezzi nemici
	uint64_t dangerousPieces = 0; //bitboard contenente i pezzi pericolosi (es. lungo una diagonale sono pericolosi un alfiere o una regina nemici...)
	uint64_t dangerousSquares = 0; //bitboard contenente le caselle controllate attualmente

	if (kingSquare == -1) { //se kingSquare è uguale a -1, vuol dire che il re si è appena mosso, per cui devo ricalcolare il quadrato in cui si trova il re
		kingSquare = std::countr_zero(kingBitboard);
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

	bool kingOnLeftBorder = BoardHelper::isOnLeftBorder(kingBitboard);
	bool kingOnRightBorder = BoardHelper::isOnRightBorder(kingBitboard);

	//controllo ora le caselle da cui il pedone potrebbe essere messo sotto scacco da un re
	if (friendlyPieces == nWhite && kingSquare < 48) { //se il re è bianco ed è sotto alla riga 7
		dangerousSquares = 0;
		//le caselle pericolose sono quella in alto a sinistra e quella in alto a destra del re
		if (!kingOnLeftBorder) { //prima di controllare la casella in alto a sinistra verifico che il re non si trovi sul bordo sinistro della scacchiera
			dangerousSquares = dangerousSquares | ((uint64_t)1 << (kingSquare + 7));
		}
		if (!kingOnRightBorder) { //prima di controllare la casella in alto a destra verifico che il re non si trovi sul bordo destro della scacchiera
			dangerousSquares = dangerousSquares | ((uint64_t)1 << (kingSquare + 9));
		}
	}
	else if (friendlyPieces == nBlack && kingSquare > 15) { //se il re è nero ed è sopra alla riga 2
		dangerousSquares = 0;
		//le caselle pericolose sono quella in basso a sinistra e quella in basso a destra del re
		if (!kingOnLeftBorder) { //prima di controllare la casella in basso a sinistra verifico che il re non si trovi sul bordo sinistro della scacchiera
			dangerousSquares = dangerousSquares | ((uint64_t)1 << (kingSquare - 9));
		}
		if (!kingOnRightBorder) { //prima di controllare la casella in basso a destra verifico che il re non si trovi sul bordo destro della scacchiera
			dangerousSquares = dangerousSquares | ((uint64_t)1 << (kingSquare - 7));
		}
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

	//controllo, infine, se il re è sotto scacco da parte del re nemico
	dangerousSquares = Board::kingMoves(kingSquare);
	dangerousPieces = 0 | (position.bitboards[nKings] & enemyPieces);

	if ((dangerousPieces & dangerousSquares) != 0) { //se le due bitboard precedentemente impostate presentano un bit acceso in comune, vi è un pezzo che tiene sotto scacco il re
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

	//BoardHelper::printBoard();

	//std::cout << "puo' arroccare" << std::endl;

	return true;
}
bool Engine::kingCanCastleShort(const bool& isWhite, const Position& position, const int& friendlyPieces, const uint64_t& blockerBitboard) {
	if (isWhite) {
		if (!engineData.m_whiteCanCastleShort) {
			return false;
		}
		uint64_t squaresTraversed = (0 | ((uint64_t)1) << 5) | ((uint64_t)1 << 6);

		if ((squaresTraversed & blockerBitboard) != 0) {
			return false;
		}
	}
	else {
		if (!engineData.m_blackCanCastleShort) {
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

void Engine::getLegalMovesFromPossibleSquaresBitboard(uint64_t moves, const int& friendlyPieces, const uint64_t& blockerBitboard, const int& pieceType, const int& startSquare, const bool& isWhite, const int& kingSquare, moveArray& moveList) {
	uint32_t move = 0; //bitboard in cui costruisco e salvo temporaneamente le mosse legali
	int actualKingSquare = pieceType == nKings ? -1 : kingSquare;
	uint64_t dangerousSquares = UINT64_MAX; //variabile in cui salvo le caselle pericolose, ovvero le caselle in cui lo spostamento di un pezzo potrebbe lasciare il re soggetto a scacco
	Position startPos = Board::getCurrentPosition();
	int i; //indice

	while (moves) {
		i = std::countr_zero(moves);
		moves = moves ^ ((uint64_t)1 << i);

		if (pieceType == nPawns && ((isWhite && i >= 56) || (!isWhite && i <= 7))) { //se il pezzo che si muove è un pedone che deve essere promosso, genero tutte le promozioni
			for (int j = 4; j < 8; j++) {
				move = 0;
				move |= startSquare << moveStartSquareOffset; //la casella di partenza è la stessa per tutte le mosse
				move |= i << moveEndSquareOffset; //imposto la casella di arrivo
				move |= j << movePromotionPieceOffset; //imposto il pezzo a cui il pedone sarà promosso
				move |= 1 << moveIsPromotionOffset; //segno che la mossa è una promozione

				if (pieceType == nKings || Engine::isKingInCheck(isWhite, startPos, friendlyPieces, startPos.bitboards[nBlack] | startPos.bitboards[nWhite], actualKingSquare)) { //se il pezzo che si sta muovendo è il re o il re è sotto scacco, devo fare la mossa per controllare che sia legale

					Board::makeMove(move); //faccio la mossa

					Position positionAfterMove = Board::getCurrentPosition(); //variabile in cui è contenuta la posizione dopo la mossa

					if (!Engine::isKingInCheck(isWhite, positionAfterMove, friendlyPieces, positionAfterMove.bitboards[nBlack] | positionAfterMove.bitboards[nWhite], actualKingSquare)) { //se il re alleato non è sotto scacco, la mossa è legale
						if ((startPos.bitboards[!friendlyPieces] >> i) & 1) { //se nella casella di arrivo c'era un pezzo nemico vuol dire che la mossa è stata una cattura
							move |= 1 << moveIsCaptureOffset;

							for (int k = 2; k < 8; k++) { //identifico il tipo di pezzo catturato
								if ((startPos.bitboards[k] >> i) & 1) {
									move |= k << moveCapturePieceOffset;
									break;
								}
							}
						}
						moveList.pushBack(move); //aggiungo la mossa alla lista delle mosse legali
					}

					Board::unmakeMove(move);
				}
				else { //altrimenti aggiungo direttamente la mossa alla lista delle mosse legali se il re non è in scacco
					if ((startPos.bitboards[!friendlyPieces] >> i) & 1) { //se nella casella di arrivo c'era un pezzo nemico vuol dire che la mossa è stata una cattura
						move |= 1 << moveIsCaptureOffset;

						for (int k = 2; k < 8; k++) { //identifico il tipo di pezzo catturato
							if ((startPos.bitboards[k] >> i) & 1) {
								move |= k << moveCapturePieceOffset;
								break;
							}
						}
					}

					moveList.pushBack(move); //aggiungo la mossa alla lista delle mosse legali
				}
			}

		}
		else {
			move = 0;
			move |= startSquare << moveStartSquareOffset; //la casella di partenza è la stessa per tutte le mosse
			move |= i << moveEndSquareOffset; //imposto la casella di arrivo
			move |= 0 << movePromotionPieceOffset; //imposto il pezzo a cui il pedone sarà promosso

			if (pieceType == nKings || Engine::isKingInCheck(isWhite, startPos, friendlyPieces, startPos.bitboards[nBlack] | startPos.bitboards[nWhite], actualKingSquare)) { //se il pezzo che si sta muovendo è il re o il re è sotto scacco, devo fare la mossa per controllare che sia legale
				Board::makeMove(move); //faccio la mossa

				Position positionAfterMove = Board::getCurrentPosition(); //variabile in cui è contenuta la posizione dopo la mossa

				if (!Engine::isKingInCheck(isWhite, positionAfterMove, friendlyPieces, positionAfterMove.bitboards[nBlack] | positionAfterMove.bitboards[nWhite], actualKingSquare)) { //se il re alleato non è sotto scacco, la mossa è legale
					if ((startPos.bitboards[!friendlyPieces] >> i) & 1) { //se nella casella di arrivo c'era un pezzo nemico vuol dire che la mossa è stata una cattura
						move |= 1 << moveIsCaptureOffset;

						for (int k = 2; k < 8; k++) { //identifico il tipo di pezzo catturato
							if ((startPos.bitboards[k] >> i) & 1) {
								move |= k << moveCapturePieceOffset;
								break;
							}
						}
					}
					moveList.pushBack(move); //aggiungo la mossa alla lista delle mosse legali
				}

				Board::unmakeMove(move);
			}
			else { //altrimenti aggiungo direttamente la mossa alla lista delle mosse legali se il re non è in scacco
				if ((startPos.bitboards[!friendlyPieces] >> i) & 1) { //se nella casella di arrivo c'era un pezzo nemico vuol dire che la mossa è stata una cattura
					move |= 1 << moveIsCaptureOffset;

					for (int k = 2; k < 8; k++) { //identifico il tipo di pezzo catturato
						if ((startPos.bitboards[k] >> i) & 1) {
							move |= k << moveCapturePieceOffset;
							break;
						}
					}
				}

				moveList.pushBack(move); //aggiungo la mossa alla lista delle mosse legali
			}
		}
	}
}

/*
bool Engine::lastMoveWasCaptureOrCheck(const bool& isWhite, const Position& positionAfterMove, const int& friendlyPieces) {
	if (Engine::isKingInCheck(!isWhite, positionAfterMove, !friendlyPieces, positionAfterMove.bitboards[nBlack] | positionAfterMove.bitboards[nWhite], -1) || ((Board::m_previousPositionCharacteristics.top() >> prevPieceOnEndSquareOffset) & prevPieceOnEndSquareBitMask)) {
		return true;
	}
	else {
		return false;
	}
}*/