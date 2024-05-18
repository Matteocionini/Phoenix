#pragma once

#include <mutex>

const int nBitboards = 8; //numero di bitboard necessarie
const int maxAllowedDepth = 20; //profondità massima di ricerca

extern std::mutex mtxReady; //mutex usato per assicurarsi che l'accesso alle variabili condiviso non avvenga in contemporanea
extern bool isReady; //variabile usata per stabilire se il motore è pronto ad accettare nuovi comandi