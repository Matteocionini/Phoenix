#pragma once

#include <vector>
#include <atomic>

#include "board.h"
#include "engineUtils.h"

struct EngineData {
	int m_hashTableSize; //dimensione della transposition table
	int m_wTime; //tempo rimanente per il bianco
	int m_bTime; //tempo rimanente per il nero
	int m_wInc; //incremento di tempo che riceve il bianco dopo ogni mossa
	int m_bInc; //incremento di tempo che riceve il nero dopo ogni mossa
	int m_movesToGo; //numero di mosse prima del prossimo time increment
	int m_halfMoveClock; //segna quante mosse sono state fatte dall'ultima volta che si è mosso un pedone (regola delle 50 mosse)
	int m_fullMoveClock; //numero di mosse giocate, viene incrementato dopo ogni mossa del nero


	//variabili search-specific, da riportare al valore di default dopo ogni ricerca
	int m_maxDepth; //salva la massima profondità di ricerca concessa, qualora fosse impostata
	int m_maxNodes; //salva il massimo numero di nodi da esaminare, qualora fosse impostato
	int m_movesToMate; //numero di mosse in cui andrebbe trovato lo scacco matto
	int m_moveTime; //tempo massimo di ricerca
	int m_enPassantSquare; //casella in cui è possibile effettuare un en passant durante la mossa corrente
	//castling rights
	bool m_whiteCanCastleLong;
	bool m_whiteCanCastleShort;
	bool m_blackCanCastleLong;
	bool m_blackCanCastleShort;
	std::vector<std::string> m_moveList; //variabile che salva il subset di mosse su cui restringere la ricerca
	std::atomic<bool> m_stop; //variabile che indica se il motore deve interrompere la ricerca il prima possibile (vedi protocollo UCI)
	std::atomic<bool> m_ponderMode; //se vero, il motore deve cercare in ponder mode
	bool m_lookForMate; //variabile che segnala al motore di cercare uno scacco matto
	bool m_infinite; //modalità ricerca infinita (vedi protocollo UCI)
	bool m_restrictSearch; //variabile che indica se è necessario restringere la ricerca ad un subset di mosse
	
	//variabili non search specific, posizionate al fondo dello struct in modo da ottimizzare l'utilizzo della memoria
	bool m_debugMode; //nel caso in cui la modalità debug sia attivata, il motore invia delle informazioni aggiuntive alla GUI
	bool m_isWhite; //vero se il motore gioca come il bianco, falso se gioca come il nero. Questa informazione è contenuta nella fenstring della posizione corrente
};

struct Move {
	int startSquare, endSquare;
	char promotionPiece;
};

/*
Struct contenente una posizione, ovvero un nodo dell'albero di gioco.
*/
struct Position { 
	uint64_t positionBitboards[nBitboards]; //bitboard rappresentative della posizione
	positionCharacteristics previousPositionInfo; //informazioni relative alla posizione da cui è derivata questa posizione
	std::vector<Position*> childrenpositions; //vettore in cui sono memorizzati i puntatori alle posizioni derivate dalla posizione precedente
	Position* fatherPosition; //posizione da cui è derivata questa posizione
	Move moveToGetHere; //mossa grazie a cui è stato possibile arrivare a questa posizione
};

namespace Engine {
	void engineInit(); //riporta il motore allo stato iniziale
	void startSearchAndEval(); //dai il via al processo di ricerca e valutazione
	void generateMoves(int level); //genera tutte le possibili mosse della posizione corrente
	void printMoves(); //funzione per stampare l'elenco di mosse da valutare

	extern EngineData engineData;
	extern std::vector<Move> movesToEvaluate; //vettore contenente le mosse da valutare relative ad una data posizione
	extern Position rootPosition; //posizione iniziale della ricerca corrente
};


