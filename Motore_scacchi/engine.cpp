#include <iostream>
#include <memory>
#include <bit>
#include <time.h>

#include "engine.h"
#include "boardHelper.h"
#include "uciHandler.h"


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

	negaMaxHandler(8);
	
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

void Engine::negaMaxHandler(int depth) {
	moveArray moves;
	int scores[256] = { 0 }; //vettore in cui salvo i punteggi associati ad ogni singola mossa
	uint32_t bestMove;
	int bestScore = INT_MIN;

	moves = generateLegalMoves(Engine::engineData.m_isWhite);

	engineData.m_nodesSearched = 0;
	
	for (int i = 0; i < moves.getSize(); i++) {
		Board::makeMove(moves.getElem(i));
		scores[i] = negaMax(3, negInfinity, posInfinity, 0); //chiamata iniziale
		Board::unmakeMove(moves.getElem(i));
	}

	for (int i = 0; i < moves.getSize(); i++) {
		if (scores[i] > bestScore) {
			bestScore = scores[i];
			bestMove = moves.getElem(i);
		}
	}

	uciHandler::postBestMove(bestMove);
}

int Engine::negaMax(int depth, int alpha, int beta, int ply) {
	moveArray moves;
	int eval;

	engineData.m_nodesSearched++;

	if (depth == 0) { //se il nodo è un nodo foglia, ritorna la valutazione della posizione
		return -quiescence(-beta, -alpha); //la quiescence search garantisce di terminare la ricerca su una posizione "tranquilla", ovvero non nel mezzo di una serie di catture
	}

	moves = generateLegalMoves(Engine::engineData.m_isWhite);

	if (moves.getSize() == 0) { //se non sono disponibili mosse legali
		if (isKingInCheck(Engine::engineData.m_isWhite, Engine::engineData.m_isWhite ? nWhite : nBlack, Board::m_positionBitBoards[nWhite] | Board::m_positionBitBoards[nBlack], -1)) { //se il re si trova sotto scacco, siamo di fronte ad uno scacco matto
			return negInfinity;
		}
		else { //se il re non si trova sotto scacco, siamo di fronte ad un pareggio
			return 0;
		}
	}

	//sortMoves(moves); //ordina le mosse in maniera da ottimizzare l'alpha-beta pruning
	//TODO: implementare il move ordering

	for (int i = 0; i < moves.getSize(); i++) {
		ply++;
		Board::makeMove(moves.getElem(i));
		eval = -negaMax(depth - 1, -beta, -alpha, ply);
		Board::unmakeMove(moves.getElem(i));
		ply--;

		if (eval >= beta) { //se la valutazione della posizione è maggiore o uguale a beta, vuol dire che il punteggio minimo garantito raggiungibile in	questa posizione è maggiore rispetto al punteggio raggiungibile in un'altra posizione raggiungibile: l'avversario non lascerà che noi arriviamo in questa posizione, per cui non è necessario cercare oltre
			return beta; //ritornando beta, do un limite inferiore alla valutazione del nodo
		}

		if (eval > alpha) { //se eval è maggiore di alpha, ho trovato una nuova miglior mossa
			alpha = eval;
		}
	}

	return alpha; //se arrivo qui, il nodo ha fallito in basso, ovvero ha raggiunto una valutazione minore o uguale ad alpha. Alpha rappresenta un limite superiore alla valutazione del nodo
}

int Engine::evaluate() {
	int score = 0;

	//bitboard contenenti i singoli pezzi
	uint64_t whitePawns = Board::m_positionBitBoards[nPawns] & Board::m_positionBitBoards[nWhite];
	uint64_t whiteBishops = Board::m_positionBitBoards[nBishops] & Board::m_positionBitBoards[nWhite];
	uint64_t whiteKnights = Board::m_positionBitBoards[nKnights] & Board::m_positionBitBoards[nWhite];
	uint64_t whiteRooks = Board::m_positionBitBoards[nRooks] & Board::m_positionBitBoards[nWhite];
	uint64_t whiteQueens = Board::m_positionBitBoards[nQueens] & Board::m_positionBitBoards[nWhite];

	uint64_t blackPawns = Board::m_positionBitBoards[nPawns] & Board::m_positionBitBoards[nBlack];
	uint64_t blackBishops = Board::m_positionBitBoards[nBishops] & Board::m_positionBitBoards[nBlack];
	uint64_t blackKnights = Board::m_positionBitBoards[nKnights] & Board::m_positionBitBoards[nBlack];
	uint64_t blackRooks = Board::m_positionBitBoards[nRooks] & Board::m_positionBitBoards[nBlack];
	uint64_t blackQueens = Board::m_positionBitBoards[nQueens] & Board::m_positionBitBoards[nBlack];

	//conto il valore dei pezzi del bianco
	score += std::popcount(whitePawns) * pawnValue;
	score += std::popcount(whiteBishops) * bishopValue;
	score += std::popcount(whiteKnights) * knightValue;
	score += std::popcount(whiteRooks) * rookValue;
	score += std::popcount(whiteQueens) * queenValue;

	//conto il valore dei pezzi del nero
	score -= std::popcount(blackPawns) * pawnValue;
	score -= std::popcount(blackBishops) * bishopValue;
	score -= std::popcount(blackKnights) * knightValue;
	score -= std::popcount(blackRooks) * rookValue;
	score -= std::popcount(blackQueens) * queenValue;

	return score * Engine::engineData.m_isWhite;
}

int Engine::quiescence(int alpha, int beta) {
	moveArray possibleMoves;
	int standPat = evaluate(); //il punteggio statico della posizione è una sorta di "limite inferiore" al punteggio raggungibile nella posizione. Questa affermazione si basa sull'ipotesi che in ogni posizione esista sempre almeno una mossa in grado di migliorare o uguagliare la valutazione statica della posizione stessa
	int score;
	Position pos;
	int capturedPiece, captureValue;

	engineData.m_nodesSearched++;

	if (standPat + queenValue < alpha) { //delta-pruning: se una posizione è così terribile che neanche catturare una regina può migliorare le cose, ignora la posizione
		return alpha;
	}

	if (standPat >= beta) { //in questo caso il nodo fallisce alto, ovvero la valutazione della posizione è troppo alta e l'avversario non permetterà di giungere qui
		return beta;
	}

	if (standPat > alpha) { //se la valutazione della posizione è maggiore di alpha, ho trovato una nuova miglior mossa
		alpha = standPat;
	}

	possibleMoves = generateLegalMoves(Engine::engineData.m_isWhite); //genero tutte le possibili mosse nella posizione

	for (int i = 0; i < possibleMoves.getSize(); i++) { //gioco solamente le catture al fine di giungere ad una posizione quiescente
		if ((possibleMoves.getElem(i) >> moveIsCaptureOffset) & moveIsCaptureBitMask) {
			//BoardHelper::printBoard();
			//uciHandler::printMove(possibleMoves.getElem(i));
			//std::cout << std::endl;
			
			capturedPiece = (possibleMoves.getElem(i) >> moveCapturePieceOffset) & moveCapturePieceBitMask;

			switch (capturedPiece) {
			case nPawns: {
				captureValue = pawnValue;
				break;
			}
			case nKnights: {
				captureValue = knightValue;
				break;
			}
			case nBishops: {
				captureValue = bishopValue;
				break;
			}
			case nRooks: {
				captureValue = rookValue;
				break;
			}
			case nQueens: {
				captureValue = queenValue;
				break;
			}
			}

			if (standPat + captureValue + delta < alpha) { //delta pruning: se la cattura risulta inutile nel migliorare la posizione, non la considero e passo oltre
				continue;
			}
			

			if (seeCapture(possibleMoves.getElem(i), !engineData.m_isWhite) >= 0) {
				Board::makeMove(possibleMoves.getElem(i)); //simula la cattura
				//BoardHelper::printBoard();
				score = -quiescence(-beta, -alpha); //continua la quiescence search
				Board::unmakeMove(possibleMoves.getElem(i)); //ritorna indietro

				if (score >= beta) { //fallimento alto
					return beta;
				}
				if (score > alpha) { //nuova miglior mossa
					alpha = score;
				}
			}
		}
	}

	return alpha;
}

int Engine::seeCapture(uint32_t move, bool isWhite) { 
	int value = 0;
	int endSquare = (move >> moveEndSquareOffset) & moveEndSquareBitMask;
	int capturedPiece;

	//BoardHelper::printBoard();

	capturedPiece = (move >> moveCapturePieceOffset) & moveCapturePieceBitMask;

	switch (capturedPiece) {
	case nPawns: {
		value += pawnValue;
		break;
	}
	case nKnights: {
		value += knightValue;
		break;
	}
	case nBishops: {
		value += bishopValue;
		break;
	}
	case nRooks: {
		value += rookValue;
		break;
	}
	case nQueens: {
		value += queenValue;
		break;
	}
	}

	Board::makeMove(move);
	//BoardHelper::printBoard();
	value -= see(endSquare, !isWhite);
	Board::unmakeMove(move);

	return value;
}

int Engine::see(int square, bool isWhite) {
	int value = 0, attackerSquare = getLeastValuableAttacker(square, isWhite), capturedPiece = 0, pieceType;
	uint32_t possibleMovesBitboard;
	moveArray moves;

	//BoardHelper::printBoard();

	if (attackerSquare) {
		possibleMovesBitboard = (uint64_t)1 << square;

		for (int i = 3; i < 8; i++) {
			if ((Board::m_positionBitBoards[isWhite ? nWhite : nBlack] >> attackerSquare) & 1) {
				pieceType = i;
				break;
			}
		}

		getLegalMovesFromPossibleSquaresBitboard(possibleMovesBitboard, isWhite ? nWhite : nBlack, Board::m_positionBitBoards[nWhite] | Board::m_positionBitBoards[nBlack], pieceType, attackerSquare, isWhite, std::countr_zero(Board::m_positionBitBoards[isWhite ? nWhite : nBlack] & Board::m_positionBitBoards[nKings]), moves);

		if (moves.getSize() > 0) {
			capturedPiece = (moves.getElem(0) >> moveCapturePieceOffset) & moveCapturePieceBitMask;

			switch (capturedPiece) {
			case nPawns: {
				value += pawnValue;
				break;
			}
			case nKnights: {
				value += knightValue;
				break;
			}
			case nBishops: {
				value += bishopValue;
				break;
			}
			case nRooks: {
				value += rookValue;
				break;
			}
			case nQueens: {
				value += queenValue;
				break;
			}
			}

			Board::makeMove(moves.getElem(0));
			//BoardHelper::printBoard();
			value -= see(square, !isWhite);
			Board::unmakeMove(moves.getElem(0));
		}
	}

	return value;
}

int Engine::getLeastValuableAttacker(int square, bool isWhite) {
	uint64_t squareBitboard = (uint64_t)1 << square; //bitboard il cui unico bit diverso da 0 è quello del della casella di interesse
	uint64_t attackingPieces = Board::m_positionBitBoards[isWhite ? nWhite : nBlack]; //bitboard contenente soltanto i pezzi che possono attaccare la casella di interesse, ovvero i pezzi alleati
	uint64_t dangerousPieces = 0; //bitboard contenente i pezzi pericolosi (es. lungo una diagonale sono pericolosi un alfiere o una regina nemici...)
	uint64_t dangerousSquares = 0; //bitboard contenente le caselle controllate attualmente
	uint64_t blockerBitboard = Board::m_positionBitBoards[nWhite] | Board::m_positionBitBoards[nBlack];
	int friendlyPieces = isWhite ? nWhite : nBlack;

	//controllo se c'è un pezzo nemico sulla casella di interesse
	if (((Board::m_positionBitBoards[!friendlyPieces] >> square) & 1) == 0) { //se non c'è un pezzo nemico sulla casella di interesse, per forza non vi è alcun attaccante
		return 0;
	}

	//controllo ora le caselle da cui un pedone potrebbe attaccare la casella di interesse
	if (isWhite && square > 15) { //se il giocatore è bianco e la casella è sopra la riga 2
		dangerousSquares = 0;
		//le caselle pericolose sono quella in basso a sinistra ed in basso a destra
		if (!BoardHelper::isOnLeftBorder(square)) { //prima di controllare la casella in basso a sinistra verifico che la casella non si trovi sul bordo sinistro della scacchiera
			dangerousSquares = dangerousSquares | ((uint64_t)1 << (square - 9));
		}
		if (!BoardHelper::isOnRightBorder(square)) { //prima di controllare la casella in basso a destra verifico che la casella non si trovi sul bordo destro della scacchiera
			dangerousSquares = dangerousSquares | ((uint64_t)1 << (square - 7));
		}
	}
	else if (!isWhite && square < 48) { //se il giocatore è nero ed è sotto la riga 7
		dangerousSquares = 0;
		//le caselle pericolose sono quella in alto a sinistra e quella in alto a destra
		if (!BoardHelper::isOnLeftBorder(square)) { //prima di controllare la casella in alto a sinistra verifico che la casella non si trovi sul bordo sinistro della scacchiera
			dangerousSquares = dangerousSquares | ((uint64_t)1 << (square + 7));
		}
		if (!BoardHelper::isOnRightBorder(square)) { //prima di controllare la casella in alto a destra verifico che il re non si trovi sul bordo destro della scacchiera
			dangerousSquares = dangerousSquares | ((uint64_t)1 << (square + 9));
		}
	}
	else { //se si arriva qui, il re non può essere messo sotto scacco da un pedone
		dangerousSquares = 0;
	}

	dangerousPieces = 0 | (Board::m_positionBitBoards[nPawns] & attackingPieces);

	if ((dangerousPieces & dangerousSquares) != 0) { //se le due bitboard precedentemente impostate presentano un bit acceso in comune, vi è un pedone che può attaccare la casella. Trovo la sua posizione
		return std::countr_zero(dangerousPieces & dangerousSquares);
	}

	//controllo ora se il re può essere messo sotto attacco da un cavallo
	dangerousSquares = Board::knightMoves(square);
	dangerousPieces = 0 | (Board::m_positionBitBoards[nKnights] & attackingPieces);

	if ((dangerousPieces & dangerousSquares) != 0) { //se le due bitboard precedentemente impostate presentano un bit acceso in comune, vi è un pezzo che tiene sotto attacco la casella
		return std::countr_zero(dangerousPieces & dangerousSquares);
	}

	//controllo le diagonali libere verso la casella
	dangerousSquares = Board::bishopMoves(square, blockerBitboard); //questa bitboard ha i bit impostati ad 1 solo lungo le diagonali libere che conducono verso il re
	dangerousPieces = (Board::m_positionBitBoards[nBishops]) & attackingPieces; //bitboard contenente i pezzi pericolosi lungo le diagonali, ovvero alfieri e regine nemici

	if ((dangerousPieces & dangerousSquares) != 0) { //se le due bitboard precedentemente impostate presentano un bit acceso in comune, vi è un alfiere che attacca la casella
		return std::countr_zero(dangerousPieces & dangerousSquares);
	}

	//controllo ora le "righe" libere verso la casella
	dangerousSquares = Board::rookMoves(square, blockerBitboard); //questa bitboard ha i bit impostati ad 1 solo lungo le righe libere che conducono verso il re
	dangerousPieces = 0 | ((Board::m_positionBitBoards[nRooks]) & attackingPieces); //bitboard contenente solo i pezzi pericolosi lungo le righe,le torri

	if ((dangerousPieces & dangerousSquares) != 0) { //se le due bitboard precedentemente impostate presentano un bit acceso in comune, vi è una torre che tiene sotto attacco la casella
		return std::countr_zero(dangerousPieces & dangerousSquares);
	}

	//controllo infine se una regina tiene sotto attacco la casella

	dangerousSquares = Board::queenMoves(square, blockerBitboard);
	dangerousPieces = Board::m_positionBitBoards[nQueens] & attackingPieces;

	if ((dangerousPieces & dangerousSquares) != 0) { //se le due bitboard precedentemente impostate presentano un bit acceso in comune, vi è una regina che tiene sotto attacco la casella
		return std::countr_zero(dangerousPieces & dangerousSquares);
	}

	return 0;
}

moveArray Engine::generateLegalMoves(bool isWhite) {
	uint64_t blockerBitboard = 0; //bitboard contenente tutti i pezzi sulla scacchiera, da passare alle varie funzioni di generazione mosse per generare le mosse pseudolegali
	int friendlyPieces = isWhite ? nWhite : nBlack; //variabile utilizzata per sapere quale tra la bitboard dei pezzi neri e quella dei pezzi bianchi è da considerare come bitboard dei pezzi alleati
	moveArray moveList; //lista delle mosse legali generate
	uint64_t moves; //bitboard utilizzata per memorizzare temporaneamente le mosse generate da una specifica funzione di generazione mosse pseudo-legali
	int kingSquare; //bitboard utilizzata per memorizzare la casella in cui si trova il re
	uint16_t move = 0; //bitboard contenente temporaneamente una mossa
	int pieceType;
	moveArray legalMoves; //vettore usato per memorizzare temporaneamente delle mosse
	uint64_t bitboardToWorkOn = Board::m_positionBitBoards[friendlyPieces]; //bitboard usata per iterare sulla scacchiera
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

	kingSquare = std::countr_zero(Board::m_positionBitBoards[nKings] & Board::m_positionBitBoards[friendlyPieces]);

	blockerBitboard = Board::m_positionBitBoards[nWhite] | Board::m_positionBitBoards[nBlack];

	dangerousSquares = Board::queenMoves(kingSquare, blockerBitboard) & Board::m_positionBitBoards[friendlyPieces]; //in questa bitboard sono memorizzati i quadrati che è necessario controllare per vedere se i pezzi all'interno di questi sono pinnati

	//BoardHelper::printLegalMoves(dangerousSquares);

	while (dangerousSquares) {
		i = std::countr_zero(dangerousSquares); //per ogni pezzo pericoloso
		dangerousSquares = dangerousSquares ^ ((uint64_t)1 << i);

		temp = ((Board::bishopMoves(i, blockerBitboard) & Board::m_positionBitBoards[!friendlyPieces])) & (Board::m_positionBitBoards[nBishops] | Board::m_positionBitBoards[nQueens]); //bitboard in cui sono memorizzate le donne/alfieri che potrebbero star pinnando il pezzo
		bishop = true;

		if (temp == 0) { //bitboard in cui sono memorizzate le torri/donne che potrebbero star pinnando il pezzo
			temp = ((Board::rookMoves(i, blockerBitboard) & Board::m_positionBitBoards[!friendlyPieces])) & (Board::m_positionBitBoards[nRooks] | Board::m_positionBitBoards[nQueens]);
			bishop = false;
		}

	checkPin:
		while (temp) {
			if (bishop) {
				temp2 = ((Board::m_positionBitBoards[friendlyPieces] & Board::m_positionBitBoards[nKings]) & Board::bishopMoves(std::countr_zero(temp), blockerBitboard ^ (uint64_t)1 << i));
			}
			else {
				temp2 = ((Board::m_positionBitBoards[friendlyPieces] & Board::m_positionBitBoards[nKings]) & Board::rookMoves(std::countr_zero(temp), blockerBitboard ^ (uint64_t)1 << i));
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
			temp = ((Board::rookMoves(i, blockerBitboard) & Board::m_positionBitBoards[!friendlyPieces])) & (Board::m_positionBitBoards[nRooks] | Board::m_positionBitBoards[nQueens]);
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
			if ((Board::m_positionBitBoards[pieceType] >> (currSquare)) & 1) {
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
			moves = Board::m_positionBitBoards[friendlyPieces];
		}
		}

		if (currSquare > 63) {
			BoardHelper::printBoard();
		}

		moves = moves & (~Board::m_positionBitBoards[friendlyPieces]) & pinRays[currSquare]; //le funzioni di generazione di mosse pseudocasuali trattano tutti i pezzi come fossero pezzi nemici. Effettuando un xor tra bitboard dei pezzi alleati e bitboard delle mosse possibili, rimuovo la cattura dei pezzi alleati dalle mosse possibili. Faccio in modo, inoltre che se il pezzo è pinnato esso non possa muoversi lasciando il re in scacco

		legalMoves.Reset();

		//BoardHelper::printLegalMoves(moves);

		getLegalMovesFromPossibleSquaresBitboard(moves, friendlyPieces, blockerBitboard, pieceType, currSquare, isWhite, kingSquare, legalMoves); //genero le mosse legali a partire dalla bitboard delle mosse pseudolegali

		moveList.Append(legalMoves.Begin(), legalMoves.End()); //aggiungo le mosse appena generate alla lista totale di mosse legali

		legalMoves.Reset();
		
		if (i + 1 > 63) {
			bitboardToWorkOn = 0;
		}
		else {
			bitboardToWorkOn = bitboardToWorkOn >> (i + 1);
		}
		
		totalShiftAmount += i + 1;
	}

	//std::cout << "Si puo' arroccare lungo: " << kingCanCastleLong(isWhite, position, friendlyPieces, blockerBitboard) << std::endl;
	//std::cout << "Si puo' arroccare corto: " << kingCanCastleShort(isWhite, position, friendlyPieces, blockerBitboard) << std::endl;

	//la funzione kingMoves di default non restituisce le eventuali mosse di arrocco possibili, per cui esse vengono aggiunte ora
	if (kingCanCastleLong(isWhite, friendlyPieces, blockerBitboard)) {
		move = 0;
		move |= (kingSquare - 2) << moveEndSquareOffset;
		move |= (kingSquare) << moveStartSquareOffset;
		move |= 0 << movePromotionPieceOffset;

		moveList.pushBack(move);
	}

	if (kingCanCastleShort(isWhite, friendlyPieces, blockerBitboard)) {
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
	generatedMoves = Engine::generateLegalMoves(Engine::engineData.m_isWhite);

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

bool Engine::isKingInCheck(const bool& isWhite, const int& friendlyPieces, const uint64_t& blockerBitboard, int kingSquare) {
	uint64_t kingBitboard = Board::m_positionBitBoards[friendlyPieces] & Board::m_positionBitBoards[nKings]; //bitboard il cui unico bit diverso da 0 è quello relativo alla casella contenente il re alleato
	uint64_t enemyPieces = Board::m_positionBitBoards[!friendlyPieces]; //bitboard contenente soltanto i pezzi nemici
	uint64_t dangerousPieces = 0; //bitboard contenente i pezzi pericolosi (es. lungo una diagonale sono pericolosi un alfiere o una regina nemici...)
	uint64_t dangerousSquares = 0; //bitboard contenente le caselle controllate attualmente

	if (kingSquare == -1) { //se kingSquare è uguale a -1, vuol dire che il re si è appena mosso, per cui devo ricalcolare il quadrato in cui si trova il re
		kingSquare = std::countr_zero(kingBitboard);
	}

	//inizio controllando le diagonali libere verso il re
	dangerousSquares = Board::bishopMoves(kingSquare, blockerBitboard); //questa bitboard ha i bit impostati ad 1 solo lungo le diagonali libere che conducono verso il re
	dangerousPieces = (Board::m_positionBitBoards[nBishops] | Board::m_positionBitBoards[nQueens]) & enemyPieces; //bitboard contenente i pezzi pericolosi lungo le diagonali, ovvero alfieri e regine nemici

	if ((dangerousPieces & dangerousSquares) != 0) { //se le due bitboard precedentemente impostate presentano un bit acceso in comune, vi è un pezzo che tiene sotto scacco il re
		return true;
	}

	//controllo ora le "righe" libere verso il re
	dangerousSquares = Board::rookMoves(kingSquare, blockerBitboard); //questa bitboard ha i bit impostati ad 1 solo lungo le righe libere che conducono verso il re
	dangerousPieces = 0 | ((Board::m_positionBitBoards[nRooks] | Board::m_positionBitBoards[nQueens]) & enemyPieces); //bitboard contenente solo i pezzi pericolosi lungo le righe, ovvero regine e torri nemiche

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

	dangerousPieces = 0 | (Board::m_positionBitBoards[nPawns] & enemyPieces);

	if ((dangerousPieces & dangerousSquares) != 0) { //se le due bitboard precedentemente impostate presentano un bit acceso in comune, vi è un pezzo che tiene sotto scacco il re
		return true;
	}

	//controllo ora se il re può essere messo sotto attacco da un cavallo
	dangerousSquares = Board::knightMoves(kingSquare);
	dangerousPieces = 0 | (Board::m_positionBitBoards[nKnights] & enemyPieces);

	if ((dangerousPieces & dangerousSquares) != 0) { //se le due bitboard precedentemente impostate presentano un bit acceso in comune, vi è un pezzo che tiene sotto scacco il re
		return true;
	}

	//controllo, infine, se il re è sotto scacco da parte del re nemico
	dangerousSquares = Board::kingMoves(kingSquare);
	dangerousPieces = 0 | (Board::m_positionBitBoards[nKings] & enemyPieces);

	if ((dangerousPieces & dangerousSquares) != 0) { //se le due bitboard precedentemente impostate presentano un bit acceso in comune, vi è un pezzo che tiene sotto scacco il re
		return true;
	}

	return false;
}

bool Engine::kingCanCastleLong(const bool& isWhite, const int& friendlyPieces, const uint64_t& blockerBitboard) {
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

	if (isKingInCheck(isWhite, friendlyPieces, blockerBitboard, kingSquare) || isKingInCheck(isWhite, friendlyPieces, blockerBitboard, kingSquare - 1) || isKingInCheck(isWhite, friendlyPieces, blockerBitboard, kingSquare - 2)) {
		return false;
	}

	//BoardHelper::printBoard();

	//std::cout << "puo' arroccare" << std::endl;

	return true;
}
bool Engine::kingCanCastleShort(const bool& isWhite, const int& friendlyPieces, const uint64_t& blockerBitboard) {
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

	if (isKingInCheck(isWhite, friendlyPieces, blockerBitboard, kingSquare) || isKingInCheck(isWhite, friendlyPieces, blockerBitboard, kingSquare + 1) || isKingInCheck(isWhite, friendlyPieces, blockerBitboard, kingSquare + 2)) {
		return false;
	}

	return true;
}

void Engine::getLegalMovesFromPossibleSquaresBitboard(uint64_t moves, const int& friendlyPieces, const uint64_t& blockerBitboard, const int& pieceType, const int& startSquare, const bool& isWhite, const int& kingSquare, moveArray& moveList) {
	uint32_t move = 0; //bitboard in cui costruisco e salvo temporaneamente le mosse legali
	int actualKingSquare = pieceType == nKings ? -1 : kingSquare;
	uint64_t dangerousSquares = UINT64_MAX; //variabile in cui salvo le caselle pericolose, ovvero le caselle in cui lo spostamento di un pezzo potrebbe lasciare il re soggetto a scacco
	int i; //indice
	bool isKingInCheck;

	while (moves) {
		i = std::countr_zero(moves);
		moves = moves ^ ((uint64_t)1 << i);

		move = 0;
		move |= pieceType << moveMovedPieceOffset; //salvo all'interno della mossa il tipo di pezzo che si è mosso
		move |= startSquare << moveStartSquareOffset; //la casella di partenza è la stessa per tutte le mosse
		move |= i << moveEndSquareOffset; //imposto la casella di arrivo

		if (pieceType == nPawns && ((isWhite && i >= 56) || (!isWhite && i <= 7))) { //se il pezzo che si muove è un pedone che deve essere promosso, genero tutte le promozioni
			for (int j = 4; j < 8; j++) {
				move |= j << movePromotionPieceOffset; //imposto il pezzo a cui il pedone sarà promosso
				move |= 1 << moveIsPromotionOffset; //segno che la mossa è una promozione

				if ((pieceType == nKings) | (isKingInCheck = Engine::isKingInCheck(isWhite, friendlyPieces, Board::m_positionBitBoards[nBlack] | Board::m_positionBitBoards[nWhite], actualKingSquare))) { //se il pezzo che si sta muovendo è il re o il re è sotto scacco, devo fare la mossa per controllare che sia legale

					move |= isKingInCheck << moveIsKingInCheckOffset; //segno se il re si trova sotto scacco

					Board::makeMove(move); //faccio la mossa

					if (!Engine::isKingInCheck(isWhite, friendlyPieces, Board::m_positionBitBoards[nBlack] | Board::m_positionBitBoards[nWhite], actualKingSquare)) { //se il re alleato non è sotto scacco, la mossa è legale

						Board::unmakeMove(move);
						if ((Board::m_positionBitBoards[!friendlyPieces] >> i) & 1) { //se nella casella di arrivo c'era un pezzo nemico vuol dire che la mossa è stata una cattura
							move |= 1 << moveIsCaptureOffset;

							for (int k = 2; k < 8; k++) { //identifico il tipo di pezzo catturato
								if ((Board::m_positionBitBoards[k] >> i) & 1) {
									move |= k << moveCapturePieceOffset;
									break;
								}
							}
						}
						moveList.pushBack(move); //aggiungo la mossa alla lista delle mosse legali
					}
					else {
						Board::unmakeMove(move);
					}
				}
				else { //altrimenti aggiungo direttamente la mossa alla lista delle mosse legali se il re non è in scacco
					if ((Board::m_positionBitBoards[!friendlyPieces] >> i) & 1) { //se nella casella di arrivo c'era un pezzo nemico vuol dire che la mossa è stata una cattura
						move |= 1 << moveIsCaptureOffset;

						for (int k = 2; k < 8; k++) { //identifico il tipo di pezzo catturato
							if ((Board::m_positionBitBoards[k] >> i) & 1) {
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
			move |= 0 << movePromotionPieceOffset; //imposto il pezzo a cui il pedone sarà promosso

			if ((pieceType == nKings) | (isKingInCheck = Engine::isKingInCheck(isWhite, friendlyPieces, Board::m_positionBitBoards[nBlack] | Board::m_positionBitBoards[nWhite], actualKingSquare))) { //se il pezzo che si sta muovendo è il re o il re è sotto scacco, devo fare la mossa per controllare che sia legale

				move |= isKingInCheck << moveIsKingInCheckOffset; //segno se il re si trova sotto scacco

				Board::makeMove(move); //faccio la mossa

				if (!Engine::isKingInCheck(isWhite, friendlyPieces, Board::m_positionBitBoards[nBlack] | Board::m_positionBitBoards[nWhite], actualKingSquare)) { //se il re alleato non è sotto scacco, la mossa è legale
					Board::unmakeMove(move);

					if ((Board::m_positionBitBoards[!friendlyPieces] >> i) & 1) { //se nella casella di arrivo c'era un pezzo nemico vuol dire che la mossa è stata una cattura
						move |= 1 << moveIsCaptureOffset;

						for (int k = 2; k < 8; k++) { //identifico il tipo di pezzo catturato
							if ((Board::m_positionBitBoards[k] >> i) & 1) {
								move |= k << moveCapturePieceOffset;
								break;
							}
						}
					}
					moveList.pushBack(move); //aggiungo la mossa alla lista delle mosse legali
				}
				else {
					Board::unmakeMove(move);
				}
			}
			else { //altrimenti aggiungo direttamente la mossa alla lista delle mosse legali se il re non è in scacco
				if ((Board::m_positionBitBoards[!friendlyPieces] >> i) & 1) { //se nella casella di arrivo c'era un pezzo nemico vuol dire che la mossa è stata una cattura
					move |= 1 << moveIsCaptureOffset;

					for (int k = 2; k < 8; k++) { //identifico il tipo di pezzo catturato
						if ((Board::m_positionBitBoards[k] >> i) & 1) {
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