#pragma once

#include <mutex>

const int nBitboards = 8; //numero di bitboard necessarie
const int maxAllowedInitialDepth = 3; //profondità massima iniziale di ricerca
const int negInfinity = -50000;
const int posInfinity = 50000;
const int delta = 500; //valore da utilizzare come delta per il delta pruning

extern std::mutex mtxReady; //mutex usato per assicurarsi che l'accesso alle variabili condiviso non avvenga in contemporanea
extern bool isReady; //variabile usata per stabilire se il motore è pronto ad accettare nuovi comandi