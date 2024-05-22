#include <iostream>

#include "board.h"
#include "boardHelper.h"
#include "engineUtils.h"
#include "uciHandler.h"
#include "engine.h"


uint64_t Board::m_bitBoards[nBitboards] = { 0 };
std::stack<uint32_t> Board::m_previousPositionCharacteristics;

void Board::makeMove(std::string move) { //la mossa viene fornita nel formato <col><rank><col><rank>[<promotion>]
	int startSquare, endSquare, promotionPiece;
	uint16_t moveOut = 0;

	//estrazione di quadrato di partenza, quadrato di arrivo ed eventuale promozione dall'int che rappresenta la mossa
	startSquare = move[0] - 'a' + (move[1] - '1') * 8;
	endSquare = move[2] - 'a' + (move[3] - '1') * 8;
	
	if (move.length() == 5) {
		switch (move[4]) {
		case 'q': {
			promotionPiece = queen;
			break;
		}
		case 'n': {
			promotionPiece = knight;
			break;
		}
		case 'r': {
			promotionPiece = rook;
			break;
		}
		case 'b': {
			promotionPiece = queen;
			break;
		}
		}
	}
	else {
		promotionPiece = none;
	}

	moveOut = ((moveOut | startSquare) | (endSquare << moveEndSquareOffset)) | (promotionPiece << movePromotionPieceOffset);

	makeMove(moveOut);
}

void Board::makeMove(const uint16_t& move) { 
	int bitboardIndexStart, bitboardIndexEnd = -1;
	int pieceColorStart, pieceColorEnd;
	uint32_t previousPositionCharacteristics = 0; //intero da 32 bit contenente le informazioni relative alla posizione precedente

	int startSquare = move & 63;
	int endSquare = (move >> moveEndSquareOffset) & 63;
	int promotionPiece = (move >> movePromotionPieceOffset) & 7;

	//prima di fare effettivamente la mossa, salva le caratteristiche irreversibili della posizione corrente per poter poi ritornare a questa posizione
	previousPositionCharacteristics |= (Engine::engineData.m_blackCanCastleLong << blackLongCastleRightsOffset);
	previousPositionCharacteristics |= (Engine::engineData.m_blackCanCastleShort << blackShortCastleRightsOffset);
	previousPositionCharacteristics |= (Engine::engineData.m_whiteCanCastleLong << whiteLongCastleRightsOffset);
	previousPositionCharacteristics |= (Engine::engineData.m_whiteCanCastleShort << whiteShortCastleRightsOffset);
	previousPositionCharacteristics |= (Engine::engineData.m_isWhite << isWhiteOffset);

	previousPositionCharacteristics |= (Engine::engineData.m_halfMoveClock << halfMoveClockOffset);
	previousPositionCharacteristics |= (Engine::engineData.m_fullMoveClock << fullMoveClockOffset);
	previousPositionCharacteristics |= (Engine::engineData.m_enPassantSquare << enPassantTargetSquareOffset);

	for (int i = 0; i < nBitboards; i++) { //grazie a questo ciclo for posso identificare di che tipo il pezzo sulla casella di partenza e quello sulla casella di arrivo sono
		if ((m_bitBoards[i] >> startSquare) & 1) { 
			if (i >= 2) { //le bitboard di indice da 2 in poi sono le bitboard specifiche dei pezzi, le altre 2 sono le bitboard che identificano il colore del pezzo
				bitboardIndexStart = i;
			}
			else {
				pieceColorStart = i; //la bitboard contenente i pezzi bianchi ha indice 0
			}
		}
		
		if (((m_bitBoards[i] >> endSquare) & 1) && i >= 2) {
			bitboardIndexEnd = i;
		}
	}

	if (bitboardIndexEnd != -1) { // se c'e' un pezzo nella casella di arrivo lo tolgo (è una cattura) e salvo questa informazione per poter tornare indietro
		previousPositionCharacteristics |= (bitboardIndexEnd << prevPieceOnEndSquareOffset);
		previousPositionCharacteristics |= (!pieceColorStart << colorOfPrevePieceOnEndSquareOffset);

		pieceColorEnd = !pieceColorStart; //ovvero prendi il colore opposto a quello dela casella di partenza, in quanto non è legale catturare il proprio pezzo
		m_bitBoards[bitboardIndexEnd] = m_bitBoards[bitboardIndexEnd] = m_bitBoards[bitboardIndexEnd] ^ ((uint64_t)1 << endSquare); 
		m_bitBoards[pieceColorEnd] = m_bitBoards[pieceColorEnd] ^ ((uint64_t)1 << endSquare);
	}

	m_bitBoards[pieceColorStart] = (m_bitBoards[pieceColorStart] ^ ((uint64_t)1 << startSquare)) | ((uint64_t)1 << endSquare); //aggiornamento della bitboard relativa al colore del pezzo mosso
	m_bitBoards[bitboardIndexStart] = (m_bitBoards[bitboardIndexStart] ^ ((uint64_t)1 << startSquare)); //tolgo il pezzo dalla sua casella precedente

	if (promotionPiece != none) { //se la mossa è composta da 5 caratteri, essa è sicuramente la promozione di un pedone
		switch (promotionPiece) { //controllo a che tipo di pezzo il pedone è stato promosso
		case queen: {
			bitboardIndexStart = nQueens;
			break;
		}

		case rook: {
			bitboardIndexStart = nRooks;
			break;
		}

		case knight: {
			bitboardIndexStart = nKnights;
			break;
		}

		case bishop: {
			bitboardIndexStart = nBishops;
			break;
		}
		}

		m_bitBoards[bitboardIndexStart] = m_bitBoards[bitboardIndexStart] | ((uint64_t)1 << endSquare); //metto nella casella di arrivo il pezzo a cui il pedone è stato promosso

		bitboardIndexStart = nPawns; //riporto il tipo di pezzo iniziale a pedone perché non so se cambiando il tipo di pezzo da pedone a regina/cavallo/torre/alfiere si può rompere qualcosa del codice sotto. Concettualmente ciò è corretto, in quanto il pezzo che è stato mosso effettivamente è un pedone
	}
	else {
		m_bitBoards[bitboardIndexStart] = m_bitBoards[bitboardIndexStart] | ((uint64_t)1 << endSquare); //metto nella casella di arrivo il pezzo (aggiorno la bitboard relativa al pezzo)
		

		if (endSquare == Engine::engineData.m_enPassantSquare && bitboardIndexStart == nPawns) { //se la mossa che è appena stata fatta e' un en passant
			Engine::engineData.m_enPassantSquare = 64;

			if (pieceColorStart == nBlack) { //se il pezzo che si è mosso è nero, il pezzo da rimuovere risiederà nella riga sopra rispetto alla casella di arrivo del pedone che si è mosso
				m_bitBoards[bitboardIndexStart] = m_bitBoards[bitboardIndexStart] ^ ((uint64_t)1 << (endSquare + 8)); //tolgo il pedone avversario dalla casella che ha subito un en passant
				m_bitBoards[!pieceColorStart] = m_bitBoards[!pieceColorStart] ^ ((uint64_t)1 << (endSquare + 8));
			}
			else {
				m_bitBoards[bitboardIndexStart] = m_bitBoards[bitboardIndexStart] ^ ((uint64_t)1 << (endSquare - 8)); //tolgo il pedone avversario dalla casella che ha subito un en passant
				m_bitBoards[!pieceColorStart] = m_bitBoards[!pieceColorStart] ^ ((uint64_t)1 << (endSquare - 8));
			}
		}
		else if (bitboardIndexStart == nKings) {
			if (startSquare == 4 && endSquare == 6) { //ovvero se la mossa e' un arrocco corto per il bianco
				m_bitBoards[nRooks] = (m_bitBoards[nRooks] ^ ((uint64_t)1 << 7)) | ((uint64_t)1 << 5);
				m_bitBoards[nWhite] = (m_bitBoards[nWhite] ^ ((uint64_t)1 << 7)) | ((uint64_t)1 << 5);
				Engine::engineData.m_whiteCanCastleShort = false;
			}
			else if (startSquare == 4 && endSquare == 2) { //ovvero se la mossa e' un arrocco lungo per il bianco
				m_bitBoards[nRooks] = (m_bitBoards[nRooks] ^ ((uint64_t)1 << 0)) | ((uint64_t)1 << 3);
				m_bitBoards[nWhite] = (m_bitBoards[nWhite] ^ ((uint64_t)1 << 0)) | ((uint64_t)1 << 3);
				Engine::engineData.m_whiteCanCastleLong = false;
			}
			else if (startSquare == 60 && endSquare == 62) { //se la mossa e' un arrocco corto per il nero
				m_bitBoards[nRooks] = (m_bitBoards[nRooks] ^ ((uint64_t)1 << 63)) | ((uint64_t)1 << 61);
				m_bitBoards[nBlack] = (m_bitBoards[nBlack] ^ ((uint64_t)1 << 63)) | ((uint64_t)1 << 61);
				Engine::engineData.m_blackCanCastleShort = false;
			}
			else if (startSquare == 60 && endSquare == 58) { //se la mossa e' un arrocco lungo per il nero
				m_bitBoards[nRooks] = (m_bitBoards[nRooks] ^ ((uint64_t)1 << 56)) | ((uint64_t)1 << 59);
				m_bitBoards[nBlack] = (m_bitBoards[nBlack] ^ ((uint64_t)1 << 56)) | ((uint64_t)1 << 59);
				Engine::engineData.m_blackCanCastleShort = false;
			}
		}
	}

	//aggiornamento relativo alle informazioni sulla posizione corrente, dopo aver eseguito la mossa
	Engine::engineData.m_enPassantSquare = 64;

	if (bitboardIndexStart == nPawns && abs(startSquare - endSquare) == 16) { //se un pedone è stato spinto di due quadrati, segnalo in quale casella e' possibile fare en passant
		if (pieceColorStart == nWhite) {
			Engine::engineData.m_enPassantSquare = endSquare - 8;
		}
		else {
			Engine::engineData.m_enPassantSquare = endSquare + 8;
		}

		//std::cout << "Casella di en passant: " << Engine::engineData.m_enPassantSquare;
	}
	else if (Engine::engineData.m_whiteCanCastleLong && endSquare == 0 || (startSquare == 0 && bitboardIndexStart == nRooks && pieceColorStart == nWhite)) { //se la torre bianca in a1 si è mossa o è stata catturata, tolgo il diritto di arrocco lungo al bianco
		Engine::engineData.m_whiteCanCastleLong = false;
		//std::cout << "Tolto il privilegio di arrocco lungo del bianco\n";
	}
	else if (Engine::engineData.m_whiteCanCastleShort && endSquare == 7 || (startSquare == 7 && bitboardIndexStart == nRooks && pieceColorStart == nWhite)) { //se la torre bianca in h1 si è mossa o è stata catturata, tolgo il diritto di arrocco corto al bianco
		Engine::engineData.m_whiteCanCastleShort = false;
		//std::cout << "Tolto il privilegio di arrocco corto del bianco\n";
	}
	else if (Engine::engineData.m_blackCanCastleLong && endSquare == 56 || (startSquare == 56 && bitboardIndexStart == nRooks && pieceColorStart == nBlack)) { //se la torre nera in a8 si è mossa o è stata catturata, tolgo il diritto di arrocco lungo al nero
		Engine::engineData.m_blackCanCastleLong = false;
		//std::cout << "Tolto il privilegio di arrocco lungo del nero\n";
	}
	else if (Engine::engineData.m_blackCanCastleShort && endSquare == 63 || (startSquare == 63 && bitboardIndexStart == nRooks && pieceColorStart == nBlack)) { //se la torre nera in h8 si è mossa o è stata catturata, tolgo il diritto di arrocco corto al nero
		//std::cout << "Tolto il privilegio di arrocco corto del nero\n";
		Engine::engineData.m_blackCanCastleShort = false;
	}
	else if (bitboardIndexStart == nKings) {
		if (pieceColorStart == nWhite && (Engine::engineData.m_whiteCanCastleLong || Engine::engineData.m_whiteCanCastleShort)) {
			Engine::engineData.m_whiteCanCastleLong = false;
			Engine::engineData.m_whiteCanCastleShort = false;
			//std::cout << "Rimossi tutti i diritti di arrocco del bianco\n";
		}
		else if (Engine::engineData.m_blackCanCastleLong || Engine::engineData.m_blackCanCastleShort){
			Engine::engineData.m_blackCanCastleLong = false;
			Engine::engineData.m_blackCanCastleShort = false;
			//std::cout << "Rimossi tutti i diritti di arrocco del nero\n";
		}
	}

	m_previousPositionCharacteristics.push(previousPositionCharacteristics);

	Engine::engineData.m_isWhite = !Engine::engineData.m_isWhite;
	//std::cout << "Deve giocare il bianco: " << Engine::engineData.m_isWhite << std::endl;
	//BoardHelper::printBoard();
	//unmakeMove(move); //solo per fini di debug
}

void Board::resetBoard() {
	setPosition("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
}

void Board::setPosition(std::string fenstring) {
	std::vector<std::string> fenSplit = uciHandler::split(fenstring);
	int rank = 7, column = 0;
	uint32_t previousPositionInfo = 0;

	for (int i = 0; i < nBitboards; i++) { //prima di impostare ogni posizione è necessario pulire tutte le bitboard, in quanto se ciò non viene fatto la posizione attuale verrà "sovrapposta" alla posizione precedente
		m_bitBoards[i] = 0;
	}

	for (int i = 0; i < fenSplit[0].size(); i++, column++) {

		switch (fenSplit[0][i]) {
		case 'r': { //è una torre nera
			m_bitBoards[nRooks] = (((m_bitBoards[nRooks] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_bitBoards[nRooks];
			m_bitBoards[nBlack] = (((m_bitBoards[nBlack] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_bitBoards[nBlack];
			break;
		}

		case 'n': { //è un cavallo nero
			m_bitBoards[nKnights] = (((m_bitBoards[nKnights] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_bitBoards[nKnights];
			m_bitBoards[nBlack] = (((m_bitBoards[nBlack] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_bitBoards[nBlack];
			break;
		}

		case 'b': { //è un alfiere nero
			m_bitBoards[nBishops] = (((m_bitBoards[nBishops] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_bitBoards[nBishops];
			m_bitBoards[nBlack] = (((m_bitBoards[nBlack] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_bitBoards[nBlack];
			break;
		}

		case 'q': { //è una regina nera
			m_bitBoards[nQueens] = (((m_bitBoards[nQueens] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_bitBoards[nQueens];
			m_bitBoards[nBlack] = (((m_bitBoards[nBlack] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_bitBoards[nBlack];
			break;
		}

		case 'k': { //è un re nero
			m_bitBoards[nKings] = (((m_bitBoards[nKings] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_bitBoards[nKings];
			m_bitBoards[nBlack] = (((m_bitBoards[nBlack] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_bitBoards[nBlack];
			break;
		}

		case 'p': { //è un pedone nero
			m_bitBoards[nPawns] = (((m_bitBoards[nPawns] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_bitBoards[nPawns];
			m_bitBoards[nBlack] = (((m_bitBoards[nBlack] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_bitBoards[nBlack];
			break;
		}

		case 'R': { //è una torre bianca
			m_bitBoards[nRooks] = (((m_bitBoards[nRooks] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_bitBoards[nRooks];
			m_bitBoards[nWhite] = (((m_bitBoards[nWhite] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_bitBoards[nWhite];
			break;
		}

		case 'N': { //è un cavallo bianco
			m_bitBoards[nKnights] = (((m_bitBoards[nKnights] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_bitBoards[nKnights];
			m_bitBoards[nWhite] = (((m_bitBoards[nWhite] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_bitBoards[nWhite];
			break;
		}

		case 'B': { //è un alfiere bianco
			m_bitBoards[nBishops] = (((m_bitBoards[nBishops] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_bitBoards[nBishops];
			m_bitBoards[nWhite] = (((m_bitBoards[nWhite] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_bitBoards[nWhite];
			break;
		}

		case 'Q': { //è una regina bianca
			m_bitBoards[nQueens] = (((m_bitBoards[nQueens] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_bitBoards[nQueens];
			m_bitBoards[nWhite] = (((m_bitBoards[nWhite] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_bitBoards[nWhite];
			break;
		}

		case 'K': { //è il re bianco
			m_bitBoards[nKings] = (((m_bitBoards[nKings] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_bitBoards[nKings];
			m_bitBoards[nWhite] = (((m_bitBoards[nWhite] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_bitBoards[nWhite];
			break;
		}

		case 'P': { //è il pedone bianco
			m_bitBoards[nPawns] = (((m_bitBoards[nPawns] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_bitBoards[nPawns];
			m_bitBoards[nWhite] = (((m_bitBoards[nWhite] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_bitBoards[nWhite];
			break;
		}

		case '1': case'2': case'3': case'4': case '5': case '6': case '7': case '8':{
			column += (fenSplit[0][i] - '0' - 1);
			break;
		}

		case '/':
			rank--;
			column = -1; //riporto a -1 perché all'inizio del ciclo la variabile column verrà ulteriormente incrementata, portando il suo valore a 0
			break;
		}
	}

	if (fenSplit[1][0] == 'w') {
		Engine::engineData.m_isWhite = true;
		previousPositionInfo |= (1 << isWhiteOffset);
	}
	else {
		Engine::engineData.m_isWhite = false;
		previousPositionInfo |= (0 << isWhiteOffset);
	}

	//std::cout << "Gioca il bianco? " << Engine::getIsWhite() << std::endl;

	Engine::engineData.m_whiteCanCastleLong = false;
	Engine::engineData.m_whiteCanCastleShort = false;
	Engine::engineData.m_blackCanCastleLong = false;
	Engine::engineData.m_blackCanCastleShort = false;

	if (fenSplit[2][0] != '-') {
		for (int i = 0; i < fenSplit[2].size(); i++) {
			switch (fenSplit[2][i]) {
			case 'K': {
				Engine::engineData.m_whiteCanCastleShort = true;
				break;
			}

			case 'Q': {
				Engine::engineData.m_whiteCanCastleLong = true;
				break;
			}

			case 'q': {
				Engine::engineData.m_blackCanCastleLong = true;
				break;
			}

			case 'k': {
				Engine::engineData.m_blackCanCastleShort = true;
			}
			}

		}
	}

	//std::cout << "Il bianco puo' arroccare lungo: " << Engine::getWhiteLongCastleRight() << std::endl;
	//std::cout << "Il bianco puo' arroccare corto: " << Engine::getWhiteShortCastleRight() << std::endl;
	//std::cout << "Il nero puo' arroccare lungo: " << Engine::getBlackLongCastleRight() << std::endl;
	//std::cout << "Il nero puo' arroccare corto: " << Engine::getBlackShortCastleRight() << std::endl;

	if (fenSplit[3].size() == 1) {
		Engine::engineData.m_enPassantSquare = 64; //64 equivale a dire che non è possibile effettuare un en passant
		//std::cout << "Non e' possibile fare en passant: " << Engine::getEnPassantSquare() << std::endl;
	}
	else {
		Engine::engineData.m_enPassantSquare = (fenSplit[3][0] - 'a') + (fenSplit[3][1] - '1') * 8;
		//std::cout << "Casella bersaglio dell'en passant: " << Engine::getEnPassantSquare() << std::endl;
	}

	Engine::engineData.m_halfMoveClock = std::stoi(fenSplit[4]);
	//std::cout << "Half move clock: " << Engine::getHalfMoveClock() << std::endl;

	Engine::engineData.m_fullMoveClock = std::stoi(fenSplit[5]);
	//std::cout << "Full move clock: " << Engine::getFullMoveClock() << std::endl;

	m_previousPositionCharacteristics.push(previousPositionInfo);
}

bool Board::isValidMove(std::string move) {
	if (move.length() < 4 || move[0] < 'a' || move[0] > 'h' || move[2] < 'a' || move[2] > 'h' 
		|| move[1] < '1' || move[1] > '8' || move[3] < '1' || move[3] > '8') {
		return false;
	}

	if (move.length() == 5 && (move[4] != 'q' && move[4] != 'r' && move[4] != 'b' && move[4] != 'n')) {
		return false;
	}

	return true;
}

std::shared_ptr<uint64_t[]> Board::getBitBoards() {
	std::shared_ptr<uint64_t[]> bitBoards(new uint64_t[nBitboards]); //è uno smart pointer, per cui non è necessario chiamare delete per liberare la memoria usata

	for (int i = 0; i < nBitboards; i++) {
		bitBoards[i] = m_bitBoards[i];
	}

	return bitBoards;
}

uint64_t Board::allPiecesBitboard() {
	uint64_t out = 0;

	for (int i = 0; i < 2; i++) {
		out = out | m_bitBoards[i];
	}

	return out;
}

void Board::unmakeMove(const uint16_t& move) {
	int pieceColor, pieceType;

	int startSquare = move & moveStartSquareBitmask;
	int endSquare = (move >> moveEndSquareOffset) & moveEndSquareBitMask;
	int promotionPiece = (move >> movePromotionPieceOffset) & movePromotionPieceBitMask;

	//recupera le informazioni relative alla posizione precedente e rimuovile dallo stack relativo
	uint32_t previousPositionCharacteristics = m_previousPositionCharacteristics.top();
	m_previousPositionCharacteristics.pop();

	//reimposto le caratteristiche irreversibili della posizione
	Engine::engineData.m_whiteCanCastleLong = (previousPositionCharacteristics >> whiteLongCastleRightsOffset) & whiteLongCastleRightsBitMask;
	Engine::engineData.m_whiteCanCastleShort = (previousPositionCharacteristics >> whiteShortCastleRightsOffset) & whiteShortCastleRightsBitMask;
	Engine::engineData.m_blackCanCastleLong = (previousPositionCharacteristics >> blackLongCastleRightsOffset) & blackLongCastleRightsBitMask;
	Engine::engineData.m_blackCanCastleShort = (previousPositionCharacteristics >> blackShortCastleRightsOffset) & blackShortCastleRightsBitMask;
	Engine::engineData.m_isWhite = (previousPositionCharacteristics >> isWhiteOffset) & isWhiteBitMask;

	Engine::engineData.m_enPassantSquare = (previousPositionCharacteristics >> enPassantTargetSquareOffset) & enPassantTargetSquareBitMask;
	Engine::engineData.m_halfMoveClock = (previousPositionCharacteristics >> halfMoveClockOffset) & halfMoveClockBitMask;
	Engine::engineData.m_fullMoveClock = (previousPositionCharacteristics >> fullMoveClockOffset) & fullMoveClockBitMask;

	//il colore del pezzo che si è mosso è uguale al colore del giocatore che doveva giocare nella mossa precedente
	if (Engine::engineData.m_isWhite) {
		pieceColor = nWhite;
	}
	else {
		pieceColor = nBlack;
	}

	//identifico il tipo di pezzo che si è mosso
	for (int i = 2; i < nBitboards; i++) {
		if ((m_bitBoards[i] >> endSquare) & 1) {
			pieceType = i;
		}
	}

	//tolgo il pezzo dalla casella in cui è stato messo
	m_bitBoards[pieceType] = m_bitBoards[pieceType] ^ ((uint64_t)1 << endSquare);
	m_bitBoards[pieceColor] = m_bitBoards[pieceColor] ^ ((uint64_t)1 << endSquare);

	//reinserisco un eventuale pezzo catturato durante la mossa precedente
	if (((previousPositionCharacteristics >> prevPieceOnEndSquareOffset) & prevPieceOnEndSquareBitMask) != 0) {
		m_bitBoards[(previousPositionCharacteristics >> prevPieceOnEndSquareOffset) & prevPieceOnEndSquareBitMask] = m_bitBoards[(previousPositionCharacteristics >> prevPieceOnEndSquareOffset) & prevPieceOnEndSquareBitMask] | ((uint64_t)1 << endSquare);
		m_bitBoards[(previousPositionCharacteristics >> colorOfPrevePieceOnEndSquareOffset) & prevPieceOnEndSquareBitMask] = m_bitBoards[(previousPositionCharacteristics >> colorOfPrevePieceOnEndSquareOffset) & prevPieceOnEndSquareBitMask] | ((uint64_t)1 << endSquare);
	}

	if (promotionPiece != none) { //se la mossa precedente è stata una promozione, il pezzo che si è precedentemente mosso in realtà è un pedone
		pieceType = nPawns;
	}

	//rimetto il pezzo nella casella da cui è partito
	m_bitBoards[pieceType] = m_bitBoards[pieceType] | ((uint64_t)1 << startSquare);
	m_bitBoards[pieceColor] = m_bitBoards[pieceColor] | ((uint64_t)1 << startSquare);

	if (pieceType == nKings) {
		if (startSquare == 4 && endSquare == 2) { //ovvero se la mossa è un arrocco lungo per il bianco
			m_bitBoards[nRooks] = (m_bitBoards[nRooks] ^ ((uint64_t)1 << 3)) | 1; //tolgo la torre bianca dalla casella d1 e la rimetto nella casella a1
			m_bitBoards[pieceColor] = (m_bitBoards[pieceColor] ^ ((uint64_t)1 << 3)) | 1;
		}
		else if (startSquare == 4 && endSquare == 6) { //ovvero se la mossa è un arrocco corto per il bianco
			m_bitBoards[nRooks] = (m_bitBoards[nRooks] ^ ((uint64_t)1 << 5)) | ((uint64_t)1 << 7); //tolgo la torre bianca dalla casella f1 e la rimetto nella casella h1
			m_bitBoards[pieceColor] = (m_bitBoards[pieceColor] ^ ((uint64_t)1 << 5)) | ((uint64_t)1 << 7);
		}
		else if (startSquare == 60 && endSquare == 58) { //ovvero se la mossa è un arrocco lungo per il nero
			m_bitBoards[nRooks] = (m_bitBoards[nRooks] ^ ((uint64_t)1 << 59)) | ((uint64_t)1 << 56); //tolgo la torre bianca dalla casella d8 e la rimetto nella casella a8
			m_bitBoards[pieceColor] = (m_bitBoards[pieceColor] ^ ((uint64_t)1 << 59)) | ((uint64_t)1 << 56);
		}
		else if (startSquare == 60 && endSquare == 62) { //ovvero se la mossa è un arrocco corto per il nero
			m_bitBoards[nRooks] = (m_bitBoards[nRooks] ^ ((uint64_t)1 << 61)) | ((uint64_t)1 << 63); //tolgo la torre bianca dalla casella f8 e la rimetto nella casella h8
			m_bitBoards[pieceColor] = (m_bitBoards[pieceColor] ^ ((uint64_t)1 << 61)) | ((uint64_t)1 << 63);
		}
	}
	else if (endSquare == Engine::engineData.m_enPassantSquare && pieceType == nPawns) { //se la mossa precedente è stata un en passant, rimetto il pedone catturato nella sua casella
		if (pieceColor == nWhite) {
			m_bitBoards[nPawns] = m_bitBoards[nPawns] | ((uint64_t)1 << (endSquare - 8)); 
			m_bitBoards[!pieceColor] = m_bitBoards[!pieceColor] | ((uint64_t)1 << (endSquare - 8));
		}
		else {
			m_bitBoards[nPawns] = m_bitBoards[nPawns] | ((uint64_t)1 << (endSquare + 8));
			m_bitBoards[!pieceColor] = m_bitBoards[!pieceColor] | ((uint64_t)1 << (endSquare + 8));
		}
	}

	//BoardHelper::printBoard();
}

uint64_t Board::rookMoves(const int& startSquare, const uint64_t& blockerBitboard) {
	uint64_t moves = 0;
	int currSquare; //questa variabile viene utilizzata come indice per controllare le possibili caselle in cui la torre può muoversi

	//la torre può muoversi soltanto in linea retta orizzontalmente o verticalmente.
	for (currSquare = startSquare + 8; currSquare <= 63; currSquare += 8) { //analizzo il movimento in verticale verso l'alto
		moves = moves | ((uint64_t)1 << currSquare);
		if (((blockerBitboard >> currSquare) & 1) == 1) {
			break;
		}
	}

	for (currSquare = startSquare - 8; currSquare >= 0; currSquare -= 8) { //analizzo il movimento in verticale verso il basso
		moves = moves | ((uint64_t)1 << currSquare);
		if (((blockerBitboard >> currSquare) & 1) == 1) {
			break;
		}
	}

	for (currSquare = startSquare + 1; currSquare <= (startSquare / 8) * 8 + 7; currSquare += 1) { //analizzo il movimento in orizzontale verso destra
		moves = moves | ((uint64_t)1 << currSquare);
		if (((blockerBitboard >> currSquare) & 1) == 1) {
			break;
		}
	}

	for (currSquare = startSquare - 1; currSquare >= (startSquare / 8) * 8; currSquare -= 1) { //analizzo il movimento in orizzontale verso sinistra
		moves = moves | ((uint64_t)1 << currSquare);
		if (((blockerBitboard >> currSquare) & 1) == 1) {
			break;
		}
	}

	//BoardHelper::printLegalMoves(moves);

	return moves;
}

uint64_t Board::bishopMoves(const int& startSquare, const uint64_t& blockerBitboard) {
	uint64_t moves = 0;
	int currSquare; //questa variabile viene utilizzata come indice per controllare le possibili caselle in cui l'alfiere può muoversi

	if (startSquare != 7 && startSquare != 15 && startSquare != 23 && startSquare != 31 && startSquare != 39 && startSquare != 48 && startSquare != 56 && startSquare != 63) {
		for (currSquare = startSquare + 9; currSquare <= 63; currSquare += 9) { //movimento in diagonale verso destra e verso l'alto
			moves = moves | ((uint64_t)1 << currSquare);
			if (((blockerBitboard >> currSquare) & 1) == 1 || (currSquare + 1) % 8 == 0) {
				break;
			}
		}
	}

	if ((startSquare % 8) != 0) {
		for (currSquare = startSquare - 9; currSquare >= 0; currSquare -= 9) { //movimento in diagonale verso il basso e verso sinistra
			moves = moves | ((uint64_t)1 << currSquare);
			if (((blockerBitboard >> currSquare) & 1) == 1 || currSquare % 8 == 0) {
				break;
			}
		}
	}

	if ((startSquare % 8) != 0) {
		for (currSquare = startSquare + 7; currSquare <= 63; currSquare += 7) { //movimento in diagonale verso l'alto e verso sinistra
			moves = moves | ((uint64_t)1 << currSquare);
			if (((blockerBitboard >> currSquare) & 1) == 1 || currSquare % 8 == 0) {
				break;
			}
		}
	}

	if ((startSquare + 1) % 8 != 0) {
		for (currSquare = startSquare - 7; currSquare >= 0; currSquare -= 7) { //movimento in diagonale verso il basso e verso destra
			moves = moves | ((uint64_t)1 << currSquare);
			if (((blockerBitboard >> currSquare) & 1) == 1 || (currSquare + 1) % 8 == 0) {
				break;
			}
		}
	}
	
	//BoardHelper::printLegalMoves(moves);

	return moves;
}

uint64_t Board::queenMoves(const int& startSquare, const uint64_t& blockerBitboard) {
	uint64_t moves = bishopMoves(startSquare, blockerBitboard) | rookMoves(startSquare, blockerBitboard);

	//BoardHelper::printLegalMoves(moves);

	return moves;
}

uint64_t Board::kingMoves(const int& startSquare) {
	uint64_t moves = 0;

	if (startSquare > 7) {
		moves = moves | ((uint64_t)1 << (startSquare - 8));
	}
	if (startSquare < 56) {
		moves = moves | ((uint64_t)1 << (startSquare + 8));
	}
	if ((startSquare + 1) % 8 != 0) {
		moves = moves | ((uint64_t)1 << (startSquare + 1));

		if (startSquare > 7) {
			moves = moves | ((uint64_t)1 << (startSquare - 7));
		}
		if (startSquare < 56) {
			moves = moves | ((uint64_t)1 << (startSquare + 9));
		}
	}
	if (startSquare % 8 != 0) {
		moves = moves | ((uint64_t)1 << (startSquare - 1));

		if (startSquare > 7) {
			moves = moves | ((uint64_t)1 << (startSquare - 9));
		}
		if (startSquare < 56) {
			moves = moves | ((uint64_t)1 << (startSquare + 7));
		}
	}

	//BoardHelper::printLegalMoves(moves);

	return moves;
}

uint64_t Board::knightMoves(const int& startSquare) {
	uint64_t moves = 0;

	if (startSquare < 48) {
		if (startSquare % 8 != 0) { //movimento a L verticale verso l'alto a sinistra
			moves = moves | ((uint64_t)1 << (startSquare + 15));
		}

		if ((startSquare + 1) % 8 != 0) { //movimento a L verticale verso l'alto a destra
			moves = moves | ((uint64_t)1 << (startSquare + 17));
		}
	}

	if (startSquare > 15) {
		if (startSquare % 8 != 0) { //movimento a L verticale verso il basso a sinistra
			moves = moves | ((uint64_t)1 << (startSquare - 17));
		}

		if ((startSquare + 1) % 8 != 0) { //movimento a L verticale verso il basso a destra
			moves = moves | ((uint64_t)1 << (startSquare - 15));
		}
	}

	if (startSquare < 56) {
		if ((startSquare - 1) % 8 != 0 && startSquare % 8 != 0) { //movimento a L orizzontale verso l'alto a sinistra
			moves = moves | ((uint64_t)1 << (startSquare + 6));
		}

		if ((startSquare + 2) % 8 != 0 && (startSquare + 1) % 8 != 0) { //movimento a L orizzontale verso l'alto a destra
			moves = moves | ((uint64_t)1 << (startSquare + 10));
		}
	}

	if (startSquare > 7) {
		if ((startSquare - 1) % 8 != 0 && startSquare % 8 != 0) { //movimento a L orizzontale verso il basso a sinistra
			moves = moves | ((uint64_t)1 << (startSquare - 10));
		}

		if ((startSquare + 2) % 8 != 0 && (startSquare + 1) % 8 != 0) { //movimento a L orizzontale verso il basso a destra
			moves = moves | ((uint64_t)1 << (startSquare - 6));
		}
	}

	//BoardHelper::printLegalMoves(moves);

	return moves;
}

uint64_t Board::pawnMoves(const int& startSquare, const uint64_t& blockerBitboard, const bool& isWhite) {
	uint64_t moves = 0;

	if (startSquare % 8 != 0) {
		if (isWhite) {
			if ((blockerBitboard >> (startSquare + 7)) & 1 || startSquare + 7 == Engine::engineData.m_enPassantSquare) { //controllo se il pedone bianco può catturare andando verso sinistra
				moves = moves | ((uint64_t)1 << (startSquare + 7));
			}
		}
		else {
			if ((blockerBitboard >> (startSquare - 9)) & 1 || startSquare - 9 == Engine::engineData.m_enPassantSquare) { //controllo se il pedone nero può catturare andando verso sinistra
				moves = moves | ((uint64_t)1 << (startSquare - 9));
			}
		}
	}

	if ((startSquare + 1) % 8 != 0) {
		if (isWhite) {
			if (blockerBitboard >> (startSquare + 9) & 1 || startSquare + 9 == Engine::engineData.m_enPassantSquare) { //controllo se il pedone bianco può catturare verso destra
				moves = moves | ((uint64_t)1 << (startSquare + 9));
			}
		}
		else { //controllo se il pedone nero può catturare verso destra
			if (blockerBitboard >> (startSquare - 7) & 1 || startSquare - 7 == Engine::engineData.m_enPassantSquare) {
				moves = moves | ((uint64_t)1 << (startSquare - 7));
			}
		}
	}

	if (isWhite) {
		if (!((blockerBitboard >> (startSquare + 8)) & 1)) {
			moves = moves | ((uint64_t)1 << (startSquare + 8));

			if (startSquare >= 8 && startSquare <= 15 && !((blockerBitboard >> (startSquare + 16)) & 1)) { //controllo se il pedone bianco può essere spinto
				moves = moves | ((uint64_t)1 << (startSquare + 16));
			}
		}
	}
	else {
		if (!((blockerBitboard >> (startSquare - 8)) & 1)) {
			moves = moves | ((uint64_t)1 << (startSquare - 8));

			if (startSquare >= 48 && startSquare <= 55 && !((blockerBitboard >> (startSquare - 16)) & 1)) { //controllo se il pedone nero può essere spinto
				moves = moves | ((uint64_t)1 << (startSquare - 16));
			}
		}
	}

	//BoardHelper::printLegalMoves(moves);

	return moves;
}

void Board::unmakeMove(int startSquare, int endSquare, char promotionPiece, uint32_t previousPositionInfo) {
	int pieceColor, pieceType;

	//reimposto le caratteristiche irreversibili della posizione
	Engine::engineData.m_whiteCanCastleLong = (previousPositionInfo >> whiteLongCastleRightsOffset) & whiteLongCastleRightsBitMask;
	Engine::engineData.m_whiteCanCastleShort = (previousPositionInfo >> whiteShortCastleRightsOffset) & whiteShortCastleRightsBitMask;
	Engine::engineData.m_blackCanCastleLong = (previousPositionInfo >> blackLongCastleRightsOffset) & blackLongCastleRightsBitMask;
	Engine::engineData.m_blackCanCastleShort = (previousPositionInfo >> blackShortCastleRightsOffset) & blackShortCastleRightsBitMask;
	Engine::engineData.m_isWhite = (previousPositionInfo >> isWhiteOffset) & isWhiteBitMask;

	Engine::engineData.m_enPassantSquare = (previousPositionInfo >> enPassantTargetSquareOffset) & enPassantTargetSquareBitMask;
	Engine::engineData.m_halfMoveClock = (previousPositionInfo >> halfMoveClockOffset) & halfMoveClockBitMask;
	Engine::engineData.m_fullMoveClock = (previousPositionInfo >> fullMoveClockOffset) & fullMoveClockBitMask;

	//il colore del pezzo che si è mosso è uguale al colore del giocatore che doveva giocare nella mossa precedente
	if (Engine::engineData.m_isWhite) {
		pieceColor = nWhite;
	}
	else {
		pieceColor = nBlack;
	}

	//identifico il tipo di pezzo che si è mosso
	for (int i = 2; i < nBitboards; i++) {
		if ((m_bitBoards[i] >> endSquare) & 1) {
			pieceType = i;
		}
	}

	//tolgo il pezzo dalla casella in cui è stato messo
	m_bitBoards[pieceType] = m_bitBoards[pieceType] ^ ((uint64_t)1 << endSquare);
	m_bitBoards[pieceColor] = m_bitBoards[pieceColor] ^ ((uint64_t)1 << endSquare);

	//reinserisco un eventuale pezzo catturato durante la mossa precedente
	if (((previousPositionInfo >> prevPieceOnEndSquareOffset) & prevPieceOnEndSquareBitMask) != 0) {
		m_bitBoards[(previousPositionInfo >> prevPieceOnEndSquareOffset) & prevPieceOnEndSquareBitMask] = m_bitBoards[(previousPositionInfo >> prevPieceOnEndSquareOffset) & prevPieceOnEndSquareBitMask] | ((uint64_t)1 << endSquare);
		m_bitBoards[(previousPositionInfo >> colorOfPrevePieceOnEndSquareOffset) & prevPieceOnEndSquareBitMask] = m_bitBoards[(previousPositionInfo >> colorOfPrevePieceOnEndSquareOffset) & prevPieceOnEndSquareBitMask] | ((uint64_t)1 << endSquare);
	}

	if (promotionPiece != -1) { //se la mossa precedente è stata una promozione, il pezzo che si è precedentemente mosso in realtà è un pedone
		pieceType = nPawns;
	}

	//rimetto il pezzo nella casella da cui è partito
	m_bitBoards[pieceType] = m_bitBoards[pieceType] | ((uint64_t)1 << startSquare);
	m_bitBoards[pieceColor] = m_bitBoards[pieceColor] | ((uint64_t)1 << startSquare);

	if (pieceType == nKings) {
		if (startSquare == 4 && endSquare == 2) { //ovvero se la mossa è un arrocco lungo per il bianco
			m_bitBoards[nRooks] = (m_bitBoards[nRooks] ^ ((uint64_t)1 << 3)) | 1; //tolgo la torre bianca dalla casella d1 e la rimetto nella casella a1
			m_bitBoards[pieceColor] = (m_bitBoards[pieceColor] ^ ((uint64_t)1 << 3)) | 1;
		}
		else if (startSquare == 4 && endSquare == 6) { //ovvero se la mossa è un arrocco corto per il bianco
			m_bitBoards[nRooks] = (m_bitBoards[nRooks] ^ ((uint64_t)1 << 5)) | ((uint64_t)1 << 7); //tolgo la torre bianca dalla casella f1 e la rimetto nella casella h1
			m_bitBoards[pieceColor] = (m_bitBoards[pieceColor] ^ ((uint64_t)1 << 5)) | ((uint64_t)1 << 7);
		}
		else if (startSquare == 60 && endSquare == 58) { //ovvero se la mossa è un arrocco lungo per il nero
			m_bitBoards[nRooks] = (m_bitBoards[nRooks] ^ ((uint64_t)1 << 59)) | ((uint64_t)1 << 56); //tolgo la torre bianca dalla casella d8 e la rimetto nella casella a8
			m_bitBoards[pieceColor] = (m_bitBoards[pieceColor] ^ ((uint64_t)1 << 59)) | ((uint64_t)1 << 56);
		}
		else if (startSquare == 60 && endSquare == 62) { //ovvero se la mossa è un arrocco corto per il nero
			m_bitBoards[nRooks] = (m_bitBoards[nRooks] ^ ((uint64_t)1 << 61)) | ((uint64_t)1 << 63); //tolgo la torre bianca dalla casella f8 e la rimetto nella casella h8
			m_bitBoards[pieceColor] = (m_bitBoards[pieceColor] ^ ((uint64_t)1 << 61)) | ((uint64_t)1 << 63);
		}
	}
	else if (endSquare == Engine::engineData.m_enPassantSquare && pieceType == nPawns) { //se la mossa precedente è stata un en passant, rimetto il pedone catturato nella sua casella
		if (pieceColor == nWhite) {
			m_bitBoards[nPawns] = m_bitBoards[nPawns] | ((uint64_t)1 << (endSquare - 8));
			m_bitBoards[!pieceColor] = m_bitBoards[!pieceColor] | ((uint64_t)1 << (endSquare - 8));
		}
		else {
			m_bitBoards[nPawns] = m_bitBoards[nPawns] | ((uint64_t)1 << (endSquare + 8));
			m_bitBoards[!pieceColor] = m_bitBoards[!pieceColor] | ((uint64_t)1 << (endSquare + 8));
		}
	}

	BoardHelper::printBoard();
}

uint64_t Board::getBitboard(int bitboardIndex) {
	return m_bitBoards[bitboardIndex];
}

void Board::resetPreviousPositionCharacteristics() {
	while (!m_previousPositionCharacteristics.empty()) {
		m_previousPositionCharacteristics.pop();
	}
}

Position Board::getCurrentPosition() {
	Position position;

	for (int i = 0; i < nBitboards; i++) {
		position.bitboards[i] = m_bitBoards[i];
	}

	return position;
}