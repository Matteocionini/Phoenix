#pragma once
#include <vector>
#include <string>

#include "logger.h"

class uciHandler {
public:
	uciHandler(char* destination);
	static void handle(std::string message);
	

private:
	char* m_destination;
	static std::vector<std::string> split(std::string str);
	static Logger m_Logger;
};