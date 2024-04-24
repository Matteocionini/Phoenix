#pragma once
#include <vector>
#include <string>

#include "logger.h"
#include "engineUtils.h"

class uciHandler {
public:
	static void handle(std::string message); //metodo principale della classe, che gestisce i messaggi in arrivo dalla GUI
	static void closeLog(); //metodo che consente di chiudere il log, e quindi di salvare le comunicazioni avvenute durante la sessione di gioco
	static std::vector<std::string> split(std::string str);

private:
	static Logger m_Logger;
	static std::string m_lastMove; //variabile in cui viene salvata l'ultima mossa indicata nel comando position, da utilizzare come mossa su cui
								   //ponderare nel caso in cui il comando successivo fosse "go ponder"
};