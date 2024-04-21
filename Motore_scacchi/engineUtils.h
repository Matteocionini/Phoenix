#pragma once

#include <mutex>

#include "board.h"

void engineInit(bool clearBoard);

const int nBitboards = 10; //numero di bitboard necessarie
extern bool debug; //nel caso in cui la modalit� debug sia attivata, il motore invia delle informazioni aggiuntive alla GUI
extern Board board; //scacchiera interna
extern std::mutex mtx; //mutex usato per assicurarsi che l'accesso alle variabili condiviso non avvenga in contemporanea
extern bool isReady; //variabile usata per stabilire se il motore � pronto ad accettare nuovi comandi
extern bool stop; //variabile che salva un eventuale comando di stop inviato dalla GUI, e che segnala al motore di smettere di pensare il prima 
				  //possibile
extern bool ponder; //se vero, il motore deve cercare in ponder mode
extern int maxDepth; //se � un valore maggiore di 0, ovvero impostato dalla GUI, � la massima profondit� di ricerca
extern int maxNodes; //se � un valore maggiore di 0, � il numero massimo di nodi su cui effettuare una ricerca
extern bool mate; //se vero, il motore deve andare alla ricerca di uno scacco matto