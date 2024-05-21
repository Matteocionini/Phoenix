#pragma once

#include <string>
#include <memory>
#include <stack>

typedef enum { //questo enum serve per poter fare un accesso parametrizzato alle bitboard, in modo da rendere il codice più leggibile
	nWhite = 0, //bitboard contenente tutti i pezzi bianchi
	nBlack, //bitboard contenente tutti i pezzi neri
	nPawns,
	nBishops,
	nQueens,
	nKings,
	nKnights,
	nRooks
} bitBoardType;

enum PositionCharacteristicsOffset { //enum contenente le quantità di cui è necessario rightshiftare l'intero da 32 bit contenente le informazioni su una posizione precedente
	whiteLongCastleRightsOffset = 0,
	whiteShortCastleRightsOffset = 1,
	blackLongCastleRightsOffset = 2,
	blackShortCastleRightsOffset = 3,
	isWhiteOffset = 4,
	enPassantTargetSquareOffset = 5,
	halfMoveClockOffset = 12,
	fullMoveClockOffset = 19,
	prevPieceOnEndSquareOffset = 26,
	colorOfPrevePieceOnEndSquareOffset = 29
};

enum PositionCharacteristicsBitMasks { //raccolta di tutte le possibili bitmask necessarie per accedere alle informazioni memorizzate negli interi che codificano le informazioni su una posizione precedente
	whiteLongCastleRightsBitMask = 1,
	whiteShortCastleRightsBitMask = 1,
	blackLongCastleRightsBitMask = 1,
	blackShortCastleRightsBitMask = 1,
	isWhiteBitMask = 1,
	enPassantTargetSquareBitMask = 127,
	halfMoveClockBitMask = 127,
	fullMoveClockBitMask = 127,
	prevPieceOnEndSquareBitMask = 7,
	colorOfPrevPieceOnEndSquareBitMask = 7
};

struct Position { //struct utilizzato per incapsulare le bitboard relative ad una certa posizione, in modo da poterle ritornare tramite una funzione
	uint64_t bitboards[8];
};

class Board {
public:
	static void resetBoard(); //metodo che consente di riportare la scacchiera alla condizione iniziale
	static void makeMove(uint16_t move); //metodo per fare una mossa sulla scacchiera interna
	static void makeMove(std::string move); //metodo per eseguire una mossa fornita sottoforma di stringa
	static void setPosition(std::string fenstring); //metodo che consente di inserire nella scacchiera interna una data posizione
	static bool isValidMove(std::string move); //metodo per controllare se un token sia effettivamente una mossa valida (vedi formato algebrico puro per le mosse valide)
	static std::shared_ptr<uint64_t[]> getBitBoards();
	static void unmakeMove(uint16_t move); //metodo che consente di annullare l'ultima mossa effettuata
	static void unmakeMove(int startSquare, int endSquare, char promotionPiece, uint32_t previousPositionInfo); //overload della funzione unmakeMove, che consente di fornire le caratteristiche irreversibili della posizione precedente

	static uint64_t rookMoves(int startSquare, uint64_t blockerBitboard); //metodo che consente di generare le mosse pseudo-legali per una data torre
	static uint64_t bishopMoves(int startSquare, uint64_t blockerBitboard); //metodo che consente di generare le mosse pseudo-legali per un dato alfiere
	static uint64_t queenMoves(int startSquare, uint64_t blockerBitboard); //metodo che consente di generare le mosse pseudo-legali per una data regina
	static uint64_t kingMoves(int startSquare); //metodo che consente di generare le mosse pseudo-legali per il re
	static uint64_t knightMoves(int startSquare); //metodo che consente di generare le mosse pseudo-legali per il cavallo
	static uint64_t pawnMoves(int startSquare, uint64_t blockerBitboard, bool isWhite); //metodo per generare le mosse pseudo-legali per il pedone

	static uint64_t getBitboard(int bitboardIndex); //metodo che consente di ottenere una certa bitboard
	static Position getCurrentPosition(); //metodo che consente di ottenere tutte le bitboard relative alla posizione attuale

	static void resetPreviousPositionCharacteristics();
	
private:
	static uint64_t m_bitBoards[]; //insieme di bitboard che rappresentano la scacchiera interna
	static uint64_t allPiecesBitboard();
	static std::stack<uint32_t> m_previousPositionCharacteristics; //stack contenente int da 32 bit contenente le caratteristiche irreversibili di una data posizione
};