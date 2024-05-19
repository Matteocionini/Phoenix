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

