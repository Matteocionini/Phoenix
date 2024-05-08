#pragma once

#include <string>
#include <memory>

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

typedef struct { //struct contenente varie informazioni irreversibili di una posizione
	bool whiteLongCastleRights;
	bool whiteShortCastleRights;
	bool blackLongCastleRights;
	bool blackShortCastleRights;
	bool isWhite;

	int enPassantTargetSquare;
	int halfMoveClock;
	int fullMoveClock;
	int prevPieceOnEndSquare;
	int colorOfPrevPieceOnEndSquare;
} positionCharacteristics;

class Board {
public:
	static void resetBoard(); //metodo che consente di riportare la scacchiera alla condizione iniziale
	static void makeMove(int startSquare, int endSquare, char promotionPiece); //metodo per fare una mossa sulla scacchiera interna
	static void makeMove(std::string move); //metodo per eseguire una mossa fornita sottoforma di stringa
	static void setPosition(std::string fenstring); //metodo che consente di inserire nella scacchiera interna una data posizione
	static bool isValidMove(std::string move); //metodo per controllare se un token sia effettivamente una mossa valida (vedi formato algebrico puro per le mosse valide)
	static std::shared_ptr<uint64_t[]> getBitBoards();
	static void unmakeMove(int startSquare, int endSquare, char promotionPiece); //metodo che consente di annullare l'ultima mossa effettuata

	static uint64_t rookMoves(int startSquare, uint64_t blockerBitboard); //metodo che consente di generare le mosse pseudo-legali per una data torre
	static uint64_t bishopMoves(int startSquare, uint64_t blockerBitboard); //metodo che consente di generare le mosse pseudo-legali per un dato alfiere
	static uint64_t queenMoves(int startSquare, uint64_t blockerBitboard); //metodo che consente di generare le mosse pseudo-legali per una data regina
	static uint64_t kingMoves(int startSquare); //metodo che consente di generare le mosse pseudo-legali per il re
	static uint64_t knightMoves(int startSquare); //metodo che consente di generare le mosse pseudo-legali per il cavallo
	static uint64_t pawnMoves(int startSquare, uint64_t blockerBitboard, bool isWhite); //metodo per generare le mosse pseudo-legali per il pedone
	
private:
	static uint64_t m_bitBoards[]; //insieme di bitboard che rappresentano la scacchiera interna
	static positionCharacteristics m_prevChars; //struct contenente vari stack che salvano informazioni irreversibili sulla posizione
	static uint64_t allPiecesBitboard();
};