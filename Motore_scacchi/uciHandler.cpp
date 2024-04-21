#include <mutex>
#include <iostream>
#include <Windows.h>

#include "uciHandler.h"
#include "engineUtils.h"
#include "board.h"

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

void uciHandler::handle(std::string message) { //funzione che si occupa dell'interazione testuale tra GUI e motore
	std::vector<std::string> messageSplit;

	messageSplit = split(message);

	m_Logger.log(message);

	/*for (int i = 0; i < messageSplit.size(); i++) {
		std::cout << messageSplit.at(i) << " ";
	}*/

	//gestione dei vari comandi che la GUI può inviare al motore
	if (messageSplit.at(0) == "uci") { 
		std::cout << "id name MyEngine\n";
		std::cout << "id author Matteo Cionini\n";
		std::cout << "uciok\n";

		m_Logger.log("id name MyEngine");
		m_Logger.log("id author Matteo Cionini");
		m_Logger.log("uciok");
	}
	else if (messageSplit.at(0) == "isready") {
		mtx.lock();

		while (!isReady) { //la variabile isReady è condivisa tra più thread, per cui è possibile che essi provino a farvi accesso contemporaneamente. Lockando il mutex e, in caso la variabile isReady fosse false, unlockandolo, per poi aspettare per 1 ms, si dà la possibilità ad un altro thread di riportare il valore di isReady a true
			mtx.unlock();
			Sleep(1);
			mtx.lock();
		}

		std::cout << "readyok\n";
		m_Logger.log("readyok");
		mtx.unlock();
		
	}
	else if (messageSplit.at(0) == "ucinewgame") {
		engineInit(false);
	}
	else if (messageSplit.at(0) == "debug") {
		if (messageSplit.at(1) == "on") {
			debug = true;
		}
		else {
			debug = false;
		}
		std::cout << "debug: " << debug << std::endl;
	}
	else if (messageSplit.at(0) == "position") {
		if (messageSplit.at(1) == "startpos") {
			board.resetBoard();
			std::cout << "Scacchiera resettata" << std::endl;
		}
		else {
			board.setPosition(messageSplit.at(1));
			std::cout << "Posizione impostata: " << messageSplit.at(1) << std::endl;
		}

		if (messageSplit.size() > 2) {
			int i;
			for (i = 3; i < messageSplit.size(); i++) {
				board.makeMove(messageSplit.at(i));
				std::cout << "Mossa eseguita: " << messageSplit.at(i) << std::endl;
			}
			m_lastMove = messageSplit.at(i - 1);
			std::cout << "Ultima mossa: " << m_lastMove << std::endl;
		}
	}
}

void uciHandler::closeLog() {
	m_Logger.closeLog();
}

/*
Possibili comandi UCI e loro funzione:

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
*/