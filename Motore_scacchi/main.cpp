#include <iostream>
#include <fstream>
#include <string>
#include <thread>

#include "logger.h"
#include "uciHandler.h"
#include "engineUtils.h"

//variabili globali condivise in tutto il programma
bool debug = false;
Board board = Board();
std::mutex mtx;
bool isReady = true;
bool stop = false;
bool ponder = false;
int maxDepth = -1;
int maxNodes = -1;
bool mate = false;

int main(){
    std::ofstream log;
    std::string message = "";
    uciHandler handler = uciHandler();
    engineInit(true);

    std::ios_base::sync_with_stdio(false); //disabilita la sincronizzazione con le funzioni di I/O native di C per un considerevole miglioramento delle prestazioni

    while (message != "quit") {
        std::getline(std::cin, message);
        handler.handle(message);
    }

    handler.closeLog();

    return 0;
}

void engineInit(bool clearBoard) { //funzione che resetta le impostazioni del motore e lo prepara per una nuova partita
    return;
}
