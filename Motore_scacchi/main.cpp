#include <iostream>
#include <string>
#include <thread>

#include "logger.h"
#include "uciHandler.h"
#include "engineUtils.h"
#include "engine.h"

//variabili globali condivise in tutto il programma
std::mutex mtxReady;
bool isReady = true;

int main(){
    std::ofstream log;
    std::string message = "";
    Engine::engineInit();

    std::ios_base::sync_with_stdio(false); //disabilita la sincronizzazione con le funzioni di I/O native di C per un considerevole miglioramento delle prestazioni

    while (message != "quit") {
        std::getline(std::cin, message);
        uciHandler::handle(message);
    }

    uciHandler::closeLog();

    return 0;
}