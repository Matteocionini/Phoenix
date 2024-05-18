#include <iostream>

#include "engine.h"

EngineData Engine::engineData; //struct contenente i dati del motore
std::vector<Move> Engine::movesToEvaluate; //vettore contenente le mosse che è necessario valutare
Position Engine::rootPosition;

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

	movesToEvaluate.clear();

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

	rootPosition.fatherPosition = NULL; //impostazione del nodo radice
	for (int i = 0; i < nBitboards; i++) {
		rootPosition.positionBitboards[i] = Board::getBitboard(i);
	}

	generateMoves(0);

	for (int i = 0; i < depth; i++) {
		movesToEvaluate.clear();
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
}

void Engine::generateMoves(int level) {
	std::vector<Move> generatedMoves;

	if (engineData.m_restrictSearch) { //nel caso in cui sia necessario restringere la ricerca ad una specifica lista di mosse, inserisci tali mosse nella lista da ricercare e ritorna
		for (int i = 0; i < engineData.m_moveList.size(); i++) {
			Move move;
			move.startSquare = engineData.m_moveList[i][0] - 'a' + (engineData.m_moveList[i][1] - '1') * 8;
			move.endSquare = engineData.m_moveList[i][2] - 'a' + (engineData.m_moveList[i][3] - '1') * 8;

			if (engineData.m_moveList[i].length() == 5) {
				move.promotionPiece = engineData.m_moveList[i][4];
			}
			else {
				move.promotionPiece = -1;
			}

			generatedMoves.push_back(move);
		}

		movesToEvaluate.insert(std::begin(movesToEvaluate), std::begin(generatedMoves), std::end(generatedMoves)); // inserisci le mosse generate al termine della lista delle mosse da valutare
		printMoves();
		return;
	}



	printMoves();
}

void Engine::printMoves() {
	for (int i = 0; i < movesToEvaluate.size(); i++) {
		std::cout << "Mossa numero: " << i + 1 << std::endl;
		std::cout << "Casella di partenza: " << movesToEvaluate[i].startSquare << std::endl;
		std::cout << "Casella di destinazione: " << movesToEvaluate[i].endSquare << std::endl;
		std::cout << "Pezzo di promozione: " << movesToEvaluate[i].promotionPiece << "\n\n";
	}
}