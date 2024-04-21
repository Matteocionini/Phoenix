#pragma once

#include <string>

class Board {
public:
	static void resetBoard(); //metodo che consente di riportare la scacchiera alla condizione iniziale
	static void makeMove(std::string move); //metodo per fare una mossa sulla scacchiera interna
	static void setPosition(std::string fenstring); //metodo che consente di inserire nella scacchiera interna una data posizione

private:
	static unsigned long m_bitBoards[]; //insieme di bitboard che rappresentano la scacchiera interna
};