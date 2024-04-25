#pragma once

#include <string>
#include <vector>

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

typedef struct { //struct contenente vari stack, ognuno dei quali contenente un'informazione irreversibile delle posizioni passate
	bool whiteLongCastleRights;
	bool whiteShortCastleRights;
	bool blackLongCastleRights;
	bool blackShortCastleRights;
	bool isWhite;

	int enPassantTargetSquare;
	int halfMoveClock;
	int fullMoveClock;
} previousPositionCharacteristics;

class Board {
public:
	static void resetBoard(); //metodo che consente di riportare la scacchiera alla condizione iniziale
	static void makeMove(std::string move); //metodo per fare una mossa sulla scacchiera interna
	static void setPosition(std::string fenstring); //metodo che consente di inserire nella scacchiera interna una data posizione
	static bool isValidMove(std::string move); //metodo per controllare se un token sia effettivamente una mossa valida (vedi formato algebrico puro per le mosse valide)
	static std::vector<uint64_t> getBitBoards();

private:
	static uint64_t m_bitBoards[]; //insieme di bitboard che rappresentano la scacchiera interna
	static previousPositionCharacteristics m_prevChars; //struct contenente vari stack che salvano informazioni irreversibili sull
};