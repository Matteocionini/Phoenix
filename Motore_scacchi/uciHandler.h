#pragma once
#include <vector>
#include <string>

#include "logger.h"
#include "engineUtils.h"

enum messageType {
	npsInfo = 0,
	pvInfo = 1,
	depthInfo = 2,
	nodesInfo = 3
};

class uciHandler {
public:
	static void handle(std::string message); //metodo principale della classe, che gestisce i messaggi in arrivo dalla GUI
	static void closeLog(); //metodo che consente di chiudere il log, e quindi di salvare le comunicazioni avvenute durante la sessione di gioco
	static void postInfo(int infoType[], int data[], int nInfo, int pvSize, uint32_t pv[]); //metodo che consente di inviare alla GUI delle informazioni
	static void postBestMove(uint32_t move); //metodo che consente di inviare alla GUI la miglior mossa trovata dal motore
	static std::vector<std::string> split(std::string str);
	static void printMove(uint32_t move); //metodo che consente di trasformare una mossa codificata in stringa

private:
	static Logger m_Logger;
	static std::string m_lastMove; //variabile in cui viene salvata l'ultima mossa indicata nel comando position, da utilizzare come mossa su cui
								   //ponderare nel caso in cui il comando successivo fosse "go ponder"
};