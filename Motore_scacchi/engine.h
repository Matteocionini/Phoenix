#pragma once

#include <vector>
#include <atomic>

#include "board.h"


class Engine {
public:
	static void engineInit(); //riporta il motore allo stato iniziale
	static void startSearchAndEval(); //dai il via al processo di ricerca e valutazione

public:
	//metodi setter/getter delle variabili membro private
	static void setDebugMode(bool value);
	static bool getDebugMode();
	static void setPonderMode(bool value);
	static bool getPonderMode();
	static void setMaxDepth(int value);
	static int getMaxDepth();
	static void setMaxNodes(int value);
	static int getMaxNodes();
	static void setStop(bool value);
	static bool getStop();
	static void setLookForMate(bool lookForMate, int maxMoves);
	static int getMovesToMate();
	static void setHashSize(int value);
	static int getHashSize();
	static void setInfinite(bool value);
	static bool getInfinite();
	static void setRestrictSearch(bool value);
	static bool getRestrictSearch();
	static void addMoveRestrictSearch(std::string move);
	static void setWTime(int value);
	static int getWTime();
	static void setBTime(int value);
	static int getBTime();
	static void setWInc(int value);
	static int getWInc();
	static void setBInc(int value);
	static int getBInc();
	static void setMovesToGo(int value);
	static int getMovesToGo();
	static void setMoveTime(int value);
	static int getMoveTime();
	static void setWhiteLongCastleRight(bool value);
	static bool getWhiteLongCastleRight();
	static void setWhiteShortCastleRight(bool value);
	static bool getWhiteShortCastleRight();
	static void setBlackLongCastleRight(bool value);
	static bool getBlackLongCastleRight();
	static void setBlackShortCastleRight(bool value);
	static bool getBlackShortCastleRight();
	static void setIsWhite(bool value);
	static bool getIsWhite();
	static void setEnPassantSquare(int square);
	static int getEnPassantSquare();
	static void setHalfMoveClock(int value);
	static int getHalfMoveClock();
	static void setFullMoveClock(int value);
	static int getFullMoveClock();


private:
	static bool m_debugMode; //nel caso in cui la modalit� debug sia attivata, il motore invia delle informazioni aggiuntive alla GUI
	static int m_hashTableSize; //dimensione della transposition table
	static int m_wTime; //tempo rimanente per il bianco
	static bool m_isWhite; //vero se il motore gioca come il bianco, falso se gioca come il nero. Questa informazione � contenuta nella fenstring della posizione corrente
	static int m_bTime; //tempo rimanente per il nero
	static int m_wInc; //incremento di tempo che riceve il bianco dopo ogni mossa
	static int m_bInc; //incremento di tempo che riceve il nero dopo ogni mossa
	static int m_movesToGo; //numero di mosse prima del prossimo time increment
	static int m_halfMoveClock; //segna quante mosse sono state fatte dall'ultima volta che si � mosso un pedone (regola delle 50 mosse)
	static int m_fullMoveClock; //numero di mosse giocate, viene incrementato dopo ogni mossa del nero
	

	//variabili search-specific, da riportare al valore di default dopo ogni ricerca
	static int m_maxDepth; //salva la massima profondit� di ricerca concessa, qualora fosse impostata
	static int m_maxNodes; //salva il massimo numero di nodi da esaminare, qualora fosse impostato
	static bool m_lookForMate; //variabile che segnala al motore di cercare uno scacco matto
	static int m_movesToMate; //numero di mosse in cui andrebbe trovato lo scacco matto
	static bool m_infinite; //modalit� ricerca infinita (vedi protocollo UCI)
	static std::atomic<bool> m_ponderMode; //se vero, il motore deve cercare in ponder mode
	static bool m_restrictSearch; //variabile che indica se � necessario restringere la ricerca ad un subset di mosse
	static std::vector<std::string> m_moveList; //variabile che salva il subset di mosse su cui restringere la ricerca
	static int m_moveTime; //tempo massimo di ricerca
	static std::atomic<bool> m_stop; //variabile che indica se il motore deve interrompere la ricerca il prima possibile (vedi protocollo UCI)
	//castling rights
	static bool m_whiteCanCastleLong; 
	static bool m_whiteCanCastleShort;
	static bool m_blackCanCastleLong;
	static bool m_blackCanCastleShort;
	static int m_enPassantSquare; //casella in cui � possibile effettuare un en passant durante la mossa corrente
};
