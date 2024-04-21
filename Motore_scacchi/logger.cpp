#include "logger.h"
#include <iostream>

Logger::Logger() {
	m_logFile.open("C:\\Users\\asus\\Desktop\\Informatica\\C++\\Motore_scacchi\\Logs\\log.txt", std::ios_base::out);
	if (!m_logFile.is_open()) {
		std::cout << "Errore nell'apertura del file" << std::endl;
	}
}

void Logger::log(std::string message) {
	m_logFile << message << std::endl;
}

void Logger::closeLog() {
	m_logFile.close();
}