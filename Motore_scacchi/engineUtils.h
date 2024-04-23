#pragma once

#include <mutex>
#include "engine.h"

const int nBitboards = 10; //numero di bitboard necessarie

extern std::mutex mtxReady; //mutex usato per assicurarsi che l'accesso alle variabili condiviso non avvenga in contemporanea
extern bool isReady; //variabile usata per stabilire se il motore è pronto ad accettare nuovi comandi