#include <iostream>
#include <fstream>
#include <string>
#include <thread>

#include "logger.h"
#include "uciHandler.h"
#include "engineUtils.h"

int main(){
    std::ofstream log;
    std::string message = "";
    char c = 0;
    char* command = &c;
    uciHandler handler = uciHandler(command);

    while (message != "quit") {
        handler.handle(message);
    }
}

void engineInit() { //funzione che resetta le impostazioni del motore e lo prepara per una nuova partita
    return;
}
