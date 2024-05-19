#include <mutex>
#include <iostream>
#include <Windows.h>

#include "uciHandler.h"
#include "engineUtils.h"
#include "board.h"
#include "engine.h"
#include "boardHelper.h"

Logger uciHandler::m_Logger = Logger();

std::string uciHandler::m_lastMove = "";

std::vector<std::string> uciHandler::split(std::string str) { //funzione che splitta un comando in token separati da spazio
	int posI = 0, posF = 0;
	std::string token;
	std::vector<std::string> splitStr;

	while (posI < str.length()) {
		posF = str.find(' ', posI);
		if (posF == -1) posF = str.length();
		token = str.substr(posI, posF - posI);
		splitStr.push_back(token);
		posI = posF + 1;
	}

	return splitStr;
}

void uciHandler::handle(std::string message) { //funzione che si occupa dell'interazione testuale tra GUI e motore. Riferimenti al protocollo UCI a fine file
	std::vector<std::string> messageSplit;

	messageSplit = split(message);

	m_Logger.log(message);

	/*for (int i = 0; i < messageSplit.size(); i++) {
		std::cout << messageSplit.at(i) << " ";
	}*/

	//gestione dei vari comandi che la GUI può inviare al motore
	if (messageSplit[0] == "uci") { 
		std::cout << "id name MyEngine\n";
		std::cout << "id author Matteo Cionini\n";
		std::cout << "option name Hash type spin default 3 min 3 max 128\n";
		std::cout << "uciok\n";

		m_Logger.log("id name Phoenix 0.1");
		m_Logger.log("id author Matteo Cionini");
		m_Logger.log("option name Hash type spin default 3 min 3 max 128");
		m_Logger.log("uciok");
	}
	else if (messageSplit[0] == "isready") {
		mtxReady.lock();

		while (!isReady) { //la variabile isReady è condivisa tra più thread, per cui è possibile che essi provino a farvi accesso contemporaneamente. Lockando il mutex e, in caso la variabile isReady fosse false, unlockandolo, per poi aspettare per 1 ms, si dà la possibilità ad un altro thread di riportare il valore di isReady a true
			mtxReady.unlock();
			Sleep(1);
			mtxReady.lock();
		}

		std::cout << "readyok\n";
		m_Logger.log("readyok");
		mtxReady.unlock();
		
	}
	else if (messageSplit[0] == "ucinewgame") {
		Engine::engineInit();
	}
	else if (messageSplit[0] == "debug") {
		if (messageSplit[1] == "on") {
			Engine::engineData.m_debugMode = true;
		}
		else {
			Engine::engineData.m_debugMode = false;
		}
		//std::cout << "debug: " << Engine::engineData.m_debugMode << std::endl;
	}
	else if (messageSplit[0] == "position") {
		if (messageSplit[1] == "startpos") {
			Board::resetBoard();
			BoardHelper::printBoard();
			//std::cout << "Scacchiera resettata" << std::endl;
			if (messageSplit.size() > 2) {
				int i;
				for (i = 3; i < messageSplit.size(); i++) {
					Board::makeMove(messageSplit[i]);
					//std::cout << "Mossa eseguita: " << messageSplit[i] << std::endl;
				}
				m_lastMove = messageSplit[i - 1];
				//std::cout << "Ultima mossa: " << m_lastMove << std::endl;
			}
		}
		else {
			std::string position = messageSplit[2] + " " + messageSplit[3] + " " + messageSplit[4] + " " + messageSplit[5] + " " + messageSplit[6] + " " + messageSplit[7];
			Board::setPosition(position);
			BoardHelper::printBoard();
			//std::cout << "Posizione impostata: " << messageSplit.at(1) << std::endl;
			if (messageSplit.size() > 8) {
				int i;
				for (i = 9; i < messageSplit.size(); i++) {
					Board::makeMove(messageSplit[i]);
					//std::cout << "Mossa eseguita: " << messageSplit[i] << std::endl;
				}
				m_lastMove = messageSplit[i - 1];
				//std::cout << "Ultima mossa: " << m_lastMove << std::endl;
			}
		}

		
	}
	else if (messageSplit[0] == "setoption" && messageSplit[1] == "name") {
		if (messageSplit[2] == "Hash") {
			Engine::engineData.m_hashTableSize = std::stoi(messageSplit[4]);
			//std::cout << "Dimensione della hashtable: " << Engine::engineData.m_hashTableSize << " MB\n";
		}
	}
	else if (messageSplit[0] == "go") {
		for (int i = 1; i < messageSplit.size(); i++) {
			if (messageSplit[i] == "searchmoves") {
				Engine::engineData.m_restrictSearch = true;
				//std::cout << "Restrict search: " << Engine::engineData.m_restrictSearch << std::endl;

				while ((i + 1) < messageSplit.size() && Board::isValidMove(messageSplit[i + 1])) {
					//std::cout << "Mossa aggiunta alla lista per la ricerca ristretta: " << messageSplit[i + 1] << std::endl;
					Engine::engineData.m_moveList.push_back(messageSplit[i + 1]);
					i++;
				}
			}
			else if (messageSplit[i] == "ponder") {
				Engine::engineData.m_ponderMode = true;
				//std::cout << "Ponder mode: " << Engine::engineData.m_ponderMode << std::endl;
			}
			else if (messageSplit[i] == "wtime") {
				i++;
				Engine::engineData.m_wTime = std::stoi(messageSplit[i]);
				//std::cout << "Tempo del bianco: " << Engine::engineData.m_wTime << std::endl;
			}
			else if (messageSplit[i] == "btime") {
				i++;
				Engine::engineData.m_bTime = std::stoi(messageSplit[i]);
				//std::cout << "Tempo del nero: " << Engine::engineData.m_bTime << std::endl;
			}
			else if (messageSplit[i] == "winc") {
				i++;
				Engine::engineData.m_wInc = std::stoi(messageSplit[i]);
				//std::cout << "Incremento del bianco: " << Engine::engineData.m_wInc << std::endl;
			}
			else if (messageSplit[i] == "binc") {
				i++;
				Engine::engineData.m_bInc = std::stoi(messageSplit[i]);
				//std::cout << "Incremento del nero: " << Engine::engineData.m_bInc << std::endl;
			}
			else if (messageSplit[i] == "movestogo") {
				i++;
				Engine::engineData.m_movesToGo = std::stoi(messageSplit[i]);
				//std::cout << "Mosse al prossimo time increment: " << Engine::engineData.m_movesToGo << std::endl;
			}
			else if (messageSplit[i] == "depth") {
				i++;
				Engine::engineData.m_maxDepth = std::stoi(messageSplit[i]);
				//std::cout << "Profondita' massima di ricerca: " << Engine::engineData.m_maxDepth << std::endl;
			}
			else if (messageSplit[i] == "nodes") {
				i++;
				Engine::engineData.m_maxNodes = std::stoi(messageSplit[i]);
				//std::cout << "Massimo di nodi valutati: " << Engine::engineData.m_maxNodes << std::endl;
			}
			else if (messageSplit[i] == "mate") {
				i++;
				Engine::engineData.m_lookForMate = true; 
				Engine::engineData.m_movesToMate = std::stoi(messageSplit[i]);
				//std::cout << "Alla ricerca di uno scacco matto in: " << Engine::engineData.m_movesToMate << std::endl;
			}
			else if (messageSplit[i] == "movetime") {
				i++;
				Engine::engineData.m_moveTime = std::stoi(messageSplit[i]);
				//std::cout << "Penso al massimo: " << Engine::engineData.m_moveTime << " ms" << std::endl;
			}
			else if (messageSplit[i] == "infinite") {
				Engine::engineData.m_infinite = true;
				//std::cout << "Modalita infinite: " << Engine::engineData.m_infinite << std::endl;
			}
		}
		Engine::startSearchAndEval();
	}
	else if (messageSplit[0] == "stop") {
		Engine::engineData.m_stop = true;
		//std::cout << "Fermo la ricerca: " << Engine::engineData.m_stop << std::endl;
	}
	else if (messageSplit[0] == "ponderhit") {
		Engine::engineData.m_ponderMode = false;
		//std::cout << "Pondermode: " << Engine::engineData.m_ponderMode << std::endl;
	}
	else {
		return;
	}
}

void uciHandler::closeLog() {
	m_Logger.closeLog();
}

/*
Possibili comandi UCI e loro funzione

Da GUI a motore:
	#uci: comunica al motore di utilizzare il protocollo UCI. Il motore risponde con uciok
	#debug [on | off]: comunica al motore di attivare/disattivare la modalità debug
	#isready: comando che può essere mandato in qualsiasi momento, specialmente dopo l'invio di molti comandi o comandi onerosi per assicurarsi che il
			  motore abbia finito di elaborare tali comandi. Il motore risponde con readyok
	#ucinewgame: comunica al motore che la prossima posizione è da considerarsi appartenente ad una nuova partita. Normalmente questa istruzione è
				 seguita da un comando isready
	#position [fen <fenstring> | startpos] moves move1 ... moven: comando che segnala al motore di impostare sulla propria scacchiera interna 
																  la posizione specificata nella fenstring e esegue le mosse specificate nel comando
	#go: comando che indica al motore di iniziare a calcolare sulla posizione precedentemente inviata dalla GUI. Questo comando può essere seguito, 
		 sulla stessa riga, da molti altri comandi:
			@searchmoves <move1> ... <movei>: restringe la ricerca alle mosse specificate
			@ponder: comando che indica al motore di iniziare la sua ricerca in "ponder mode", ovvero di iniziare a valutare la posizione inviata
					 precedentemente dalla GUI. Il motore NON deve uscire da solo dalla ricerca in ponder mode in nessun caso, deve attendere un comando
					 stop o ponderhit
			@wtime <x>: indica che il bianco ha ancora x millisecondi di tempo
			@btime <x>: indica che il nero ha ancora x millisecondi di tempo
			@winc <x>: indica che dopo ogni mossa, il bianco guadagna x millisecondi di tempo
			@binc <x>: indica che dopo ogni mossa, il nero guadagna x millisecondi di tempo
			@movestogo <x>: indica che mancano x mosse al prossimo time increment
			@depth <x>: imposta la profondità massima di ricerca
			@nodes <x>: limita la ricerca a x nodi
			@mate <x>: vai alla ricerca di uno scacco matto in x mosse
			@movetime <x>: esegui una ricerca della durata di x millisecondi
			@infinite: non uscire dalla modalità di ricerca se non istruito dalla GUI
	#stop: comando che indica al motore di smettere di calcolare il prima possibile
	#ponderhit: comando che comunica al motore che è stata giocata la mossa sulla quale gli era stato detto di ponderare. Il motore può continuare la
				sua ricerca, ma non in ponder mode
	#quit: chiudi il programma il prima possibile
	#setoption name <name> <x>: imposta l'opzione chiamata name, che il motore ha dichiarato di supportare, al valore x
	#register: per ora non ritengo utile implementarlo
*/