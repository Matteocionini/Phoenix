#pragma once

#include <string>
#include <fstream>

//classe utilizzata per memorizzare le comunicazioni che avvengono tra GUI e motore in un file

class Logger {
public:
	Logger();
	void log(std::string message);

private:
	std::ofstream m_logFile;
};