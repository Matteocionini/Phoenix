#include <mutex>
#include <iostream>
#include <Windows.h>

#include "uciHandler.h"
#include "engineUtils.h"
#include "board.h"
#include "engine.h"

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

		m_Logger.log("id name MyEngine");
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
			Engine::setDebugMode(true);
		}
		else {
			Engine::setDebugMode(false);
		}
		//std::cout << "debug: " << Engine::getDebugMode() << std::endl;
	}
	else if (messageSplit[0] == "position") {
		if (messageSplit[1] == "startpos") {
			Board::resetBoard();
			//std::cout << "Scacchiera resettata" << std::endl;
		}
		else {
			Board::setPosition(messageSplit[1]);
			//std::cout << "Posizione impostata: " << messageSplit.at(1) << std::endl;
		}

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
	else if (messageSplit[0] == "setoption" && messageSplit[1] == "name") {
		if (messageSplit[2] == "Hash") {
			Engine::setHashSize(std::stoi(messageSplit[3]));
			//std::cout << "Dimensione della hashtable: " << Engine::getHashSize() << " MB\n";
		}
	}
	else if (messageSplit[0] == "go") {
		for (int i = 1; i < messageSplit.size(); i++) {
			if (messageSplit[i] == "searchmoves") {
				Engine::setRestrictSearch(true);
				//std::cout << "Restrict search: " << Engine::getRestrictSearch() << std::endl;

				while ((i + 1) < messageSplit.size() && Board::isValidMove(messageSplit[i + 1])) {
					//std::cout << "Mossa aggiunta alla lista per la ricerca ristretta: " << messageSplit[i + 1] << std::endl;
					Engine::addMoveRestrictSearch(messageSplit[i + 1]);
					i++;
				}
			}
			else if (messageSplit[i] == "ponder") {
				Engine::setPonderMode(true);
				//std::cout << "Ponder mode: " << Engine::getPonderMode() << std::endl;
			}
			else if (messageSplit[i] == "wtime") {
				i++;
				Engine::setWTime(std::stoi(messageSplit[i]));
				//std::cout << "Tempo del bianco: " << Engine::getWTime() << std::endl;
			}
			else if (messageSplit[i] == "btime") {
				i++;
				Engine::setBTime(std::stoi(messageSplit[i]));
				//std::cout << "Tempo del nero: " << Engine::getBTime() << std::endl;
			}
			else if (messageSplit[i] == "winc") {
				i++;
				Engine::setWInc(std::stoi(messageSplit[i]));
				//std::cout << "Incremento del bianco: " << Engine::getWInc() << std::endl;
			}
			else if (messageSplit[i] == "binc") {
				i++;
				Engine::setBInc(std::stoi(messageSplit[i]));
				//std::cout << "Incremento del nero: " << Engine::getBInc() << std::endl;
			}
			else if (messageSplit[i] == "movestogo") {
				i++;
				Engine::setMovesToGo(std::stoi(messageSplit[i]));
				//std::cout << "Mosse al prossimo time increment: " << Engine::getMovesToGo() << std::endl;
			}
			else if (messageSplit[i] == "depth") {
				i++;
				Engine::setMaxDepth(std::stoi(messageSplit[i]));
				//std::cout << "Profondita' massima di ricerca: " << Engine::getMaxDepth() << std::endl;
			}
			else if (messageSplit[i] == "nodes") {
				i++;
				Engine::setMaxNodes(std::stoi(messageSplit[i]));
				//std::cout << "Massimo di nodi valutati: " << Engine::getMaxNodes() << std::endl;
			}
			else if (messageSplit[i] == "mate") {
				i++;
				Engine::setLookForMate(true, std::stoi(messageSplit[i]));
				//std::cout << "Alla ricerca di uno scacco matto in: " << Engine::getMovesToMate() << std::endl;
			}
			else if (messageSplit[i] == "movetime") {
				i++;
				Engine::setMoveTime(std::stoi(messageSplit[i]));
				//std::cout << "Penso al massimo: " << Engine::getMoveTime() << " ms" << std::endl;
			}
			else if (messageSplit[i] == "infinite") {
				Engine::setInfinite(true);
				//std::cout << "Modalita infinite: " << Engine::getInfinite() << std::endl;
			}
		}
		Engine::startSearchAndEval();
	}
	else if (messageSplit[0] == "stop") {
		Engine::setStop(true);
		//std::cout << "Fermo la ricerca: " << Engine::getStop() << std::endl;
	}
	else if (messageSplit[0] == "ponderhit") {
		Engine::setPonderMode(false);
		//std::cout << "Pondermode: " << Engine::getPonderMode() << std::endl;
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