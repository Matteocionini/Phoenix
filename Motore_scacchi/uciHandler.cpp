#include "uciHandler.h"
#include "engineUtils.h"

#include <mutex>
#include <iostream>

Logger uciHandler::m_Logger = Logger();

uciHandler::uciHandler(char* destination) { //la variabile m_destination sarà usata dall'uciHandler per salvare dei codici interni diretti al motore stesso, come ad esempio se è necessario smettere di valutare una posizione su comando della GUI
	m_destination = destination;
}

std::vector<std::string> uciHandler::split(std::string str) { //funzione che splitta un comando in token separati da spazio
	int posI = 0, posF = 0;
	std::string token;
	std::vector<std::string> splitStr;

	while (posI < str.length()) {
		posF = str.find(' ', posI);
		if (posF == -1) posF = str.length();
		token = str.substr(posI, posF - posI);
		splitStr.push_back(token);
		posI = posF + 1;
	}

	return splitStr;
}

void uciHandler::handle(std::string message) { //funzione che si occupa dell'interazione testuale tra GUI e motore
	std::vector<std::string> messageSplit;

	std::getline(std::cin, message);
	messageSplit = split(message);

	m_Logger.log(message);

	for (int i = 0; i < messageSplit.size(); i++) {
		std::cout << messageSplit.at(i) << " ";
	}

	if (messageSplit.at(0) == "uci") {
		std::cout << "id name MyEngine\n";
		std::cout << "id author Matteo Cionini\n";
		std::cout << "uciok\n";

		m_Logger.log("id name MyEngine");
		m_Logger.log("id author Matteo Cionini");
		m_Logger.log("uciok");
	}
	else if (messageSplit.at(0) == "isready") {
		std::cout << "readyok\n";

		m_Logger.log("readyok");
	}
	else if (messageSplit.at(0) == "ucinewgame") {
		engineInit();
	}
}