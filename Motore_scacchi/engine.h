#pragma once

#include <vector>
#include <atomic>

#include "board.h"
#include "engineUtils.h"
#include "moveArray.h"

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

enum MoveOffsets { //enum in cui sono memorizzate le costanti di cui è necessario rightshiftare l'intero di 16 bit contenente una mossa per accedere alle varie informazioni sulla mossa stessa. In una mossa, i primi 6 bit di questo intero contengono la casella di partenza (a partire da sinistra), i seguenti 6 codificano la casella di arrivo ed i seguenti 3 codificano l'eventuale promozione
	moveStartSquareOffset = 0,
	moveEndSquareOffset = 6,
	movePromotionPieceOffset = 12,
	moveIsCaptureOrCheckOrPromotionOffset = 15 //questo bit, quando è 1, segnala che la mossa è una cattura o mette il re nemico in scacco
};

enum MoveBitMasks { //raccolta di tutte le possibili bitmask che possono risultare necessarie per fare accesso alle informazioni contenute nell'intero che codifica una mossa
	moveStartSquareBitmask = 63,
	moveEndSquareBitMask = 63,
	movePromotionPieceBitMask = 7,
	moveIsCaptureOrCheckOrPromotionBitMask = 1
};

enum PromotionPiece { //enum contenente i codici relativi alla promozione ad un pezzo specifico
	none = 0,
	queen = 1,
	knight = 2,
	bishop = 3,
	rook = 4
};



namespace Engine {
	void engineInit(); //riporta il motore allo stato iniziale
	void startSearchAndEval(); //dai il via al processo di ricerca e valutazione

	moveArray generateLegalMoves(const Position& position, bool isWhite); //funzione che si occupa della generazione delle mosse legali
	void getLegalMovesFromPossibleSquaresBitboard(const uint64_t& moves, const int& friendlyPieces, const uint64_t& blockerBitboard, const int& pieceType, const int& startSquare, const bool& isWhite, const int& kingSquare, moveArray& moveList); //funzione che, a partire da una bitboard fornita da una funzione di generazione mosse pseudolegali, genera le mosse effettivamente legali
	uint64_t perft(int depth, bool first); //funzione di test della performance e della correttezza del sistema di generazione mosse

	//funzioni helper
	bool isKingInCheck(const bool& isWhite, const Position& position, const int& friendlyPieces, const uint64_t& blockerBitboard, int kingSquare); //funzione che controlla se nella posizione corrente il re è sotto scacco
	bool kingCanCastleLong(const bool& isWhite, const Position& position, const int& friendlyPieces, const uint64_t& blockerBitboard); //funzione usata per verificare se l'arrocco lungo è legale
	bool kingCanCastleShort(const bool& isWhite, const Position& position, const int& friendlyPieces, const uint64_t& blockerBitboard); //funzione usata per verificare se l'arrocco corto è legale
	bool lastMoveWasCaptureOrCheck(const bool& isWhite, const Position& positionAfterMove, const int& friendlyPieces); //funzione che permette di controllare se la mossa appena fatta mette sotto scacco il re avversario
	

	extern EngineData engineData;
};