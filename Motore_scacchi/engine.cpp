#include "engine.h"
#include "engineUtils.h"

bool Engine::m_debugMode = false;
bool Engine::m_ponderMode = false; 
int Engine::m_maxDepth = -1; 
int Engine::m_maxNodes = -1; 
bool Engine::m_stop = false; 
bool Engine::m_lookForMate;
int Engine::m_movesToMate;
int Engine::m_hashTableSize = 3;
bool Engine::m_infinite = false;
bool Engine::m_restrictSearch = false;
int Engine::m_wTime = 0;
bool Engine::m_isWhite;
std::vector<std::string> Engine::m_moveList;
int Engine::m_bTime = 0;
int Engine::m_bInc = 0;
int Engine::m_wInc = 0;
int Engine::m_movesToGo = -1;
int Engine::m_moveTime = -1;

void Engine::engineInit() {
	mtxReady.lock();
	isReady = false;
	mtxReady.unlock();

	m_debugMode = false;
	m_ponderMode = false;
	m_maxDepth = -1;
	m_maxNodes = -1;
	m_stop = false;
	m_lookForMate = false;
	m_movesToMate = -1;
	m_hashTableSize = 3;
	m_infinite = false;
	m_restrictSearch = false;
	m_moveList.clear();
	m_wTime = 0;
	m_bTime = 0;
	m_wInc = 0;
	m_bInc = 0;
	m_movesToGo = -1;
	m_moveTime = -1;

	mtxReady.lock();
	isReady = true;
	mtxReady.unlock();
}

void Engine::startSearchAndEval() {
	return;
}

//metodi setter e getter
void Engine::setDebugMode(bool value) {
	m_debugMode = value;
}

bool Engine::getDebugMode() {
	return m_debugMode;
}

void Engine::setMaxDepth(int value) {
	m_maxDepth = value;
}

int Engine::getMaxDepth() {
	return m_maxDepth;
}

void Engine::setPonderMode(bool value) {
	m_ponderMode = value;
}

bool Engine::getPonderMode() {
	return m_ponderMode;
}

void Engine::setMaxNodes(int value) {
	m_maxNodes = value;
}

int Engine::getMaxNodes() {
	return m_maxNodes;
}

void Engine::setStop(bool value) {
	m_stop = value;
}

bool Engine::getStop() {
	return m_stop;
}

void Engine::setLookForMate(bool lookForMate, int maxMoves) {
	m_lookForMate = lookForMate;
	m_movesToMate = maxMoves;
}

void Engine::setHashSize(int value) {
	m_hashTableSize = value;
}

int Engine::getHashSize() {
	return m_hashTableSize;
}

void Engine::setInfinite(bool value) {
	m_infinite = value;
}

bool Engine::getInfinite() {
	return m_infinite;
}

void Engine::setRestrictSearch(bool value) {
	m_restrictSearch = value;
}

bool Engine::getRestrictSearch() {
	return m_restrictSearch;
}

void Engine::addMoveRestrictSearch(std::string move) {
	m_moveList.push_back(move);
}

void Engine::setWTime(int value) {
	m_wTime = value;
}

int Engine::getWTime() {
	return m_wTime;
}

void Engine::setBTime(int value) {
	m_bTime = value;
}

int Engine::getBTime() {
	return m_bTime;
}

void Engine::setWInc(int value) {
	m_wInc = value;
}

int Engine::getWInc() {
	return m_wInc;
}

void Engine::setBInc(int value) {
	m_bInc = value;
}

int Engine::getBInc() {
	return m_bInc;
}

void Engine::setMovesToGo(int value) {
	m_movesToGo = value;
}

int Engine::getMovesToGo() {
	return m_movesToGo;
}

void Engine::setMoveTime(int value) {
	m_moveTime = value;
}

int Engine::getMoveTime() {
	return m_moveTime;
}

int Engine::getMovesToMate() {
	return m_movesToMate;
}