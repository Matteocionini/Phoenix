#pragma once

#include <string>

class Board {
public:
	static void resetBoard();
	static void makeMove(std::string move);
	static void setPosition(std::string fenstring);

private:
	static unsigned long m_bitBoards[];
};