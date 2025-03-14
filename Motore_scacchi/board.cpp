#include <iostream>
#include <random>
#include <chrono>

#include "board.h"
#include "boardHelper.h"
#include "engineUtils.h"
#include "uciHandler.h"
#include "engine.h"

uint64_t Board::m_rookOccupancyBitmask[64] = { 282578800148862, 565157600297596, 1130315200595066, 2260630401190006, 4521260802379886, 9042521604759646, 18085043209519166, 36170086419038334, 282578800180736, 565157600328704, 1130315200625152, 2260630401218048, 4521260802403840, 9042521604775424, 18085043209518592, 36170086419037696, 282578808340736, 565157608292864, 1130315208328192, 2260630408398848, 4521260808540160, 9042521608822784, 18085043209388032, 36170086418907136, 282580897300736, 565159647117824, 1130317180306432, 2260632246683648, 4521262379438080, 9042522644946944, 18085043175964672, 36170086385483776, 283115671060736, 565681586307584, 1130822006735872, 2261102847592448, 4521664529305600, 9042787892731904, 18085034619584512, 36170077829103616, 420017753620736, 699298018886144, 1260057572672512, 2381576680245248, 4624614895390720, 9110691325681664, 18082844186263552, 36167887395782656, 35466950888980736, 34905104758997504, 34344362452452352, 33222877839362048, 30979908613181440, 26493970160820224, 17522093256097792, 35607136465616896, 9079539427579068672, 8935706818303361536, 8792156787827803136, 8505056726876686336, 7930856604974452736, 6782456361169985536, 4485655873561051136, 9115426935197958144 }; //inizializzazione del vettore contenente le bitmask da usare nell'hashing per le magic bitboard relativo alle torri grazie ai valori precalcolati

//per la generazione di questi due vettori vedere la funzione generateRookMagicNumbers()
uint64_t Board::m_rookMagicNumbers[64] = { 12765338825791934653, 6003356142975670690, 12283358860737182866, 16780013626657448365, 6644383126179250247, 5388561657473499803, 7764550289560872950, 11182115657921111062, 7108287624222568984, 18149769911441700558, 2798070921934123540, 3196394180987118241, 7311249612629803773, 5463974744156384240, 8002156486500580371, 12783015346033590083, 1635254482294719967, 7729233032761743208, 14188794456437276548, 14607725556038480988, 13836227985847663852, 7555441162267721618, 7110907200316343014, 16618236445402391508, 7840045788121688066, 13215341325413387417, 7921009695388696789, 14024389061710998325, 7880677760711774364, 5341740142537501406, 10867118566742130547, 8595853550777471110, 43611992863275751, 10540578615202296880, 4752687527127652046, 8716546488374091641, 5053438310797449946, 17822603509532441848, 16683708196044859765, 9158910590718398507, 7050197630344975693, 6660075362541060509, 11622492273542556126, 11773804660200139974, 18316473365498753087, 11425703203271360522, 1494260979827426979, 7257951427207711403, 11988969984388537235, 1636927417397113444, 13039696375362582352, 2004841306521734954, 16119623313637209976, 12133913441870415030, 695565190972308480, 16019422294774868240, 322843803334106926, 12282542977649702398, 745910359379479754, 2599049938205435586, 17590132843619942386, 5672846611701838594, 6236104566056751756, 15571820733531445022 }; 

int Board::m_rookShiftAmounts[64] = { 50, 51, 51, 51, 51, 51, 51, 50, 51, 53, 53, 53, 53, 53, 53, 52, 51, 53, 52, 52, 52, 52, 53, 52, 52, 53, 53, 52, 53, 52, 53, 52, 52, 53, 53, 53, 52, 52, 53, 52, 52, 53, 53, 52, 53, 53, 53, 52, 52, 53, 53, 53, 52, 53, 53, 52, 51, 52, 52, 52, 52, 52, 53, 51 };

uint64_t Board::m_bishopOccupancyBitmask[64] = { 18049651735527936, 70506452091904, 275415828992, 1075975168, 38021120, 8657588224, 2216338399232, 567382630219776, 9024825867763712, 18049651735527424, 70506452221952, 275449643008, 9733406720, 2216342585344, 567382630203392, 1134765260406784, 4512412933816832, 9024825867633664, 18049651768822272, 70515108615168, 2491752130560, 567383701868544, 1134765256220672, 2269530512441344, 2256206450263040, 4512412900526080, 9024834391117824, 18051867805491712, 637888545440768, 1135039602493440, 2269529440784384, 4539058881568768, 1128098963916800, 2256197927833600, 4514594912477184, 9592139778506752, 19184279556981248, 2339762086609920, 4538784537380864, 9077569074761728, 562958610993152, 1125917221986304, 2814792987328512, 5629586008178688, 11259172008099840, 22518341868716544, 9007336962655232, 18014673925310464, 2216338399232, 4432676798464, 11064376819712, 22137335185408, 44272556441600, 87995357200384, 35253226045952, 70506452091904, 567382630219776, 1134765260406784, 2832480465846272, 5667157807464448, 11333774449049600, 22526811443298304, 9024825867763712, 18049651735527936 }; //inizializzazione del vettore contenente le bitmask da usare nell'hashing per le magic bitboard relativo agli alfieri grazie ai valori precalcolati

uint64_t Board::m_bishopMagicNumbers[64] = { 1301458728482289659, 2480576671576751018, 10019474676587191777, 9968733218880549441, 10282291966949593424, 715088683708452943, 14590761865148562107, 11806229677758250851, 3625132614012968955, 16739331944649876474, 13392430790299613355, 5543623407150568148, 15163967460908598087, 8905791196360128586, 12531068692356923340, 14465687413383905156, 5120690120951539751, 4123175048549992398, 10191648675827122946, 2767230745509046865, 8736368379906216727, 10029236261004377954, 1005856890454163391, 8474861321512938386, 3983755699822008377, 9158074287147473603, 1415695568661738992, 7686206180567024438, 9947707072633712870, 15966256843794238982, 17470628480567804200, 2131544887437690907, 137395318718495892, 3541808489995232264, 17107402915822738976, 5689197996005913552, 3627739763300835353, 16429749946017513800, 11591235241407422405, 6126025282371325009, 1450091784425716907, 11592245020760917118, 8060315484380033032, 1126404720404547594, 4650367110558558720, 8917883794935786321, 13258493906644644162, 4226665651118442608, 6897257424288016484, 12961358619673518520, 12203791251348261336, 15431973664925455085, 16779642216255193537, 9104697014123889034, 15708331797815387792, 1711346512376401904, 11663196338124369846, 3663154921498436278, 10516323236185786404, 9345964734109555274, 8271288898437382665, 12182124706027931783, 11659397071542021733, 15425109614664615351 };

int Board::m_bishopShiftAmounts[64] = { 58, 60, 59, 59, 59, 59, 60, 58, 60, 60, 59, 59, 59, 59, 60, 60, 59, 59, 57, 56, 56, 57, 59, 59, 59, 59, 56, 53, 53, 57, 59, 59, 59, 59, 56, 53, 53, 57, 59, 59, 59, 59, 57, 57, 57, 56, 59, 59, 60, 60, 59, 59, 59, 59, 60, 60, 58, 60, 59, 59, 59, 59, 60, 58 };

uint64_t Board::m_positionBitBoards[nBitboards] = { 0 };
std::stack<uint32_t>* Board::m_previousPositionCharacteristics = new std::stack<uint32_t>;

uint64_t* Board::m_buffer;
uint64_t* Board::m_rookMagicBitboards[64];
uint64_t* Board::m_bishopMagicBitboards[64];

void Board::makeMove(std::string move) { //la mossa viene fornita nel formato <col><rank><col><rank>[<promotion>]
	int startSquare, endSquare, promotionPiece;
	uint32_t moveOut = 0;

	//estrazione di quadrato di partenza, quadrato di arrivo ed eventuale promozione dall'int che rappresenta la mossa
	startSquare = move[0] - 'a' + (move[1] - '1') * 8;
	endSquare = move[2] - 'a' + (move[3] - '1') * 8;
	
	if (move.length() == 5) {
		moveOut = moveOut | (1 << moveIsPromotionBitMask);

		switch (move[4]) {
		case 'q': {
			promotionPiece = nQueens;
			break;
		}
		case 'n': {
			promotionPiece = nKnights;
			break;
		}
		case 'r': {
			promotionPiece = nRooks;
			break;
		}
		case 'b': {
			promotionPiece = nBishops;
			break;
		}
		}
	}
	else {
		promotionPiece = 0;
	}

	moveOut = ((moveOut | startSquare) | (endSquare << moveEndSquareOffset)) | (promotionPiece << movePromotionPieceOffset);

	makeMove(moveOut);
}

void Board::makeMove(const uint32_t& move) { 
	int bitboardIndexStart = -895, bitboardIndexEnd = -1;
	int pieceColorStart = -1, pieceColorEnd;
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
		if ((m_positionBitBoards[i] >> startSquare) & 1) { 
			if (i >= 2) { //le bitboard di indice da 2 in poi sono le bitboard specifiche dei pezzi, le altre 2 sono le bitboard che identificano il colore del pezzo
				bitboardIndexStart = i;
			}
			else {
				pieceColorStart = i; //la bitboard contenente i pezzi bianchi ha indice 0
			}
		}
		
		if (((m_positionBitBoards[i] >> endSquare) & 1) && i >= 2) {
			bitboardIndexEnd = i;
		}
	}

	if (bitboardIndexEnd != -1) { // se c'e' un pezzo nella casella di arrivo lo tolgo (� una cattura) e salvo questa informazione per poter tornare indietro
		previousPositionCharacteristics |= (bitboardIndexEnd << prevPieceOnEndSquareOffset);
		previousPositionCharacteristics |= (!pieceColorStart << colorOfPrevePieceOnEndSquareOffset);

		pieceColorEnd = !pieceColorStart; //ovvero prendi il colore opposto a quello dela casella di partenza, in quanto non � legale catturare il proprio pezzo
		m_positionBitBoards[bitboardIndexEnd] = m_positionBitBoards[bitboardIndexEnd] = m_positionBitBoards[bitboardIndexEnd] ^ ((uint64_t)1 << endSquare); 
		m_positionBitBoards[pieceColorEnd] = m_positionBitBoards[pieceColorEnd] ^ ((uint64_t)1 << endSquare);
	}

	if (pieceColorStart == -1) {
		BoardHelper::printBoard();
		uciHandler::printMove(move);
	}

	m_positionBitBoards[pieceColorStart] = (m_positionBitBoards[pieceColorStart] ^ ((uint64_t)1 << startSquare)) | ((uint64_t)1 << endSquare); //aggiornamento della bitboard relativa al colore del pezzo mosso

	m_positionBitBoards[bitboardIndexStart] = (m_positionBitBoards[bitboardIndexStart] ^ ((uint64_t)1 << startSquare)); //tolgo il pezzo dalla sua casella precedente
	


	if (promotionPiece != 0) { //se la mossa � composta da 5 caratteri, essa � sicuramente la promozione di un pedone
		bitboardIndexStart = promotionPiece;

		m_positionBitBoards[bitboardIndexStart] = m_positionBitBoards[bitboardIndexStart] | ((uint64_t)1 << endSquare); //metto nella casella di arrivo il pezzo a cui il pedone � stato promosso

		bitboardIndexStart = nPawns; //riporto il tipo di pezzo iniziale a pedone perch� non so se cambiando il tipo di pezzo da pedone a regina/cavallo/torre/alfiere si pu� rompere qualcosa del codice sotto. Concettualmente ci� � corretto, in quanto il pezzo che � stato mosso effettivamente � un pedone
	}
	else {
		m_positionBitBoards[bitboardIndexStart] = m_positionBitBoards[bitboardIndexStart] | ((uint64_t)1 << endSquare); //metto nella casella di arrivo il pezzo (aggiorno la bitboard relativa al pezzo)
		

		if (endSquare == Engine::engineData.m_enPassantSquare && bitboardIndexStart == nPawns) { //se la mossa che � appena stata fatta e' un en passant
			Engine::engineData.m_enPassantSquare = 64;

			if (pieceColorStart == nBlack) { //se il pezzo che si � mosso � nero, il pezzo da rimuovere risieder� nella riga sopra rispetto alla casella di arrivo del pedone che si � mosso
				m_positionBitBoards[bitboardIndexStart] = m_positionBitBoards[bitboardIndexStart] ^ ((uint64_t)1 << (endSquare + 8)); //tolgo il pedone avversario dalla casella che ha subito un en passant
				m_positionBitBoards[!pieceColorStart] = m_positionBitBoards[!pieceColorStart] ^ ((uint64_t)1 << (endSquare + 8));
			}
			else {
				m_positionBitBoards[bitboardIndexStart] = m_positionBitBoards[bitboardIndexStart] ^ ((uint64_t)1 << (endSquare - 8)); //tolgo il pedone avversario dalla casella che ha subito un en passant
				m_positionBitBoards[!pieceColorStart] = m_positionBitBoards[!pieceColorStart] ^ ((uint64_t)1 << (endSquare - 8));
			}
		}
		else if (bitboardIndexStart == nKings) {
			if (pieceColorStart == nWhite) {
				Engine::engineData.m_whiteCanCastleShort = false;
				Engine::engineData.m_whiteCanCastleLong = false;
			}
			else {
				Engine::engineData.m_blackCanCastleShort = false;
				Engine::engineData.m_blackCanCastleLong = false;
			}

			if (startSquare == 4 && endSquare == 6) { //ovvero se la mossa e' un arrocco corto per il bianco
				m_positionBitBoards[nRooks] = (m_positionBitBoards[nRooks] ^ ((uint64_t)1 << 7)) | ((uint64_t)1 << 5);
				m_positionBitBoards[nWhite] = (m_positionBitBoards[nWhite] ^ ((uint64_t)1 << 7)) | ((uint64_t)1 << 5);
			}
			else if (startSquare == 4 && endSquare == 2) { //ovvero se la mossa e' un arrocco lungo per il bianco
				m_positionBitBoards[nRooks] = (m_positionBitBoards[nRooks] ^ ((uint64_t)1 << 0)) | ((uint64_t)1 << 3);
				m_positionBitBoards[nWhite] = (m_positionBitBoards[nWhite] ^ ((uint64_t)1 << 0)) | ((uint64_t)1 << 3);
			}
			else if (startSquare == 60 && endSquare == 62) { //se la mossa e' un arrocco corto per il nero
				m_positionBitBoards[nRooks] = (m_positionBitBoards[nRooks] ^ ((uint64_t)1 << 63)) | ((uint64_t)1 << 61);
				m_positionBitBoards[nBlack] = (m_positionBitBoards[nBlack] ^ ((uint64_t)1 << 63)) | ((uint64_t)1 << 61);
			}
			else if (startSquare == 60 && endSquare == 58) { //se la mossa e' un arrocco lungo per il nero
				m_positionBitBoards[nRooks] = (m_positionBitBoards[nRooks] ^ ((uint64_t)1 << 56)) | ((uint64_t)1 << 59);
				m_positionBitBoards[nBlack] = (m_positionBitBoards[nBlack] ^ ((uint64_t)1 << 56)) | ((uint64_t)1 << 59);

			}
		}
	}

	//aggiornamento relativo alle informazioni sulla posizione corrente, dopo aver eseguito la mossa
	Engine::engineData.m_enPassantSquare = 64;

	if (bitboardIndexStart == nPawns && abs(startSquare - endSquare) == 16) { //se un pedone � stato spinto di due quadrati, segnalo in quale casella e' possibile fare en passant
		if (pieceColorStart == nWhite) {
			Engine::engineData.m_enPassantSquare = endSquare - 8;
		}
		else {
			Engine::engineData.m_enPassantSquare = endSquare + 8;
		}

		//std::cout << "Casella di en passant: " << Engine::engineData.m_enPassantSquare;
	}
	else if (Engine::engineData.m_whiteCanCastleLong && endSquare == 0 || (startSquare == 0 && bitboardIndexStart == nRooks && pieceColorStart == nWhite)) { //se la torre bianca in a1 si � mossa o � stata catturata, tolgo il diritto di arrocco lungo al bianco
		Engine::engineData.m_whiteCanCastleLong = false;
		//std::cout << "Tolto il privilegio di arrocco lungo del bianco\n";
	}
	else if (Engine::engineData.m_whiteCanCastleShort && endSquare == 7 || (startSquare == 7 && bitboardIndexStart == nRooks && pieceColorStart == nWhite)) { //se la torre bianca in h1 si � mossa o � stata catturata, tolgo il diritto di arrocco corto al bianco
		Engine::engineData.m_whiteCanCastleShort = false;
		//std::cout << "Tolto il privilegio di arrocco corto del bianco\n";
	}
	else if (Engine::engineData.m_blackCanCastleLong && endSquare == 56 || (startSquare == 56 && bitboardIndexStart == nRooks && pieceColorStart == nBlack)) { //se la torre nera in a8 si � mossa o � stata catturata, tolgo il diritto di arrocco lungo al nero
		Engine::engineData.m_blackCanCastleLong = false;
		//std::cout << "Tolto il privilegio di arrocco lungo del nero\n";
	}
	else if (Engine::engineData.m_blackCanCastleShort && endSquare == 63 || (startSquare == 63 && bitboardIndexStart == nRooks && pieceColorStart == nBlack)) { //se la torre nera in h8 si � mossa o � stata catturata, tolgo il diritto di arrocco corto al nero
		//std::cout << "Tolto il privilegio di arrocco corto del nero\n";
		Engine::engineData.m_blackCanCastleShort = false;
	}
	else if (bitboardIndexStart == nKings) {
		if (pieceColorStart == nWhite && (Engine::engineData.m_whiteCanCastleLong || Engine::engineData.m_whiteCanCastleShort)) {
			Engine::engineData.m_whiteCanCastleLong = false;
			Engine::engineData.m_whiteCanCastleShort = false;
			//std::cout << "Rimossi tutti i diritti di arrocco del bianco\n";
		}
		else if (pieceColorStart == nBlack && (Engine::engineData.m_blackCanCastleLong || Engine::engineData.m_blackCanCastleShort)){
			Engine::engineData.m_blackCanCastleLong = false;
			Engine::engineData.m_blackCanCastleShort = false;
			//std::cout << "Rimossi tutti i diritti di arrocco del nero\n";
		}
	}

	(*m_previousPositionCharacteristics).push(previousPositionCharacteristics);

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

	for (int i = 0; i < nBitboards; i++) { //prima di impostare ogni posizione � necessario pulire tutte le bitboard, in quanto se ci� non viene fatto la posizione attuale verr� "sovrapposta" alla posizione precedente
		m_positionBitBoards[i] = 0;
	}

	for (int i = 0; i < fenSplit[0].size(); i++, column++) {

		switch (fenSplit[0][i]) {
		case 'r': { //� una torre nera
			m_positionBitBoards[nRooks] = (((m_positionBitBoards[nRooks] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_positionBitBoards[nRooks];
			m_positionBitBoards[nBlack] = (((m_positionBitBoards[nBlack] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_positionBitBoards[nBlack];
			break;
		}

		case 'n': { //� un cavallo nero
			m_positionBitBoards[nKnights] = (((m_positionBitBoards[nKnights] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_positionBitBoards[nKnights];
			m_positionBitBoards[nBlack] = (((m_positionBitBoards[nBlack] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_positionBitBoards[nBlack];
			break;
		}

		case 'b': { //� un alfiere nero
			m_positionBitBoards[nBishops] = (((m_positionBitBoards[nBishops] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_positionBitBoards[nBishops];
			m_positionBitBoards[nBlack] = (((m_positionBitBoards[nBlack] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_positionBitBoards[nBlack];
			break;
		}

		case 'q': { //� una regina nera
			m_positionBitBoards[nQueens] = (((m_positionBitBoards[nQueens] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_positionBitBoards[nQueens];
			m_positionBitBoards[nBlack] = (((m_positionBitBoards[nBlack] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_positionBitBoards[nBlack];
			break;
		}

		case 'k': { //� un re nero
			m_positionBitBoards[nKings] = (((m_positionBitBoards[nKings] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_positionBitBoards[nKings];
			m_positionBitBoards[nBlack] = (((m_positionBitBoards[nBlack] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_positionBitBoards[nBlack];
			break;
		}

		case 'p': { //� un pedone nero
			m_positionBitBoards[nPawns] = (((m_positionBitBoards[nPawns] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_positionBitBoards[nPawns];
			m_positionBitBoards[nBlack] = (((m_positionBitBoards[nBlack] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_positionBitBoards[nBlack];
			break;
		}

		case 'R': { //� una torre bianca
			m_positionBitBoards[nRooks] = (((m_positionBitBoards[nRooks] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_positionBitBoards[nRooks];
			m_positionBitBoards[nWhite] = (((m_positionBitBoards[nWhite] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_positionBitBoards[nWhite];
			break;
		}

		case 'N': { //� un cavallo bianco
			m_positionBitBoards[nKnights] = (((m_positionBitBoards[nKnights] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_positionBitBoards[nKnights];
			m_positionBitBoards[nWhite] = (((m_positionBitBoards[nWhite] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_positionBitBoards[nWhite];
			break;
		}

		case 'B': { //� un alfiere bianco
			m_positionBitBoards[nBishops] = (((m_positionBitBoards[nBishops] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_positionBitBoards[nBishops];
			m_positionBitBoards[nWhite] = (((m_positionBitBoards[nWhite] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_positionBitBoards[nWhite];
			break;
		}

		case 'Q': { //� una regina bianca
			m_positionBitBoards[nQueens] = (((m_positionBitBoards[nQueens] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_positionBitBoards[nQueens];
			m_positionBitBoards[nWhite] = (((m_positionBitBoards[nWhite] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_positionBitBoards[nWhite];
			break;
		}

		case 'K': { //� il re bianco
			m_positionBitBoards[nKings] = (((m_positionBitBoards[nKings] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_positionBitBoards[nKings];
			m_positionBitBoards[nWhite] = (((m_positionBitBoards[nWhite] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_positionBitBoards[nWhite];
			break;
		}

		case 'P': { //� il pedone bianco
			m_positionBitBoards[nPawns] = (((m_positionBitBoards[nPawns] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_positionBitBoards[nPawns];
			m_positionBitBoards[nWhite] = (((m_positionBitBoards[nWhite] >> (rank * 8 + column)) | 1) << (rank * 8 + column)) | m_positionBitBoards[nWhite];
			break;
		}

		case '1': case'2': case'3': case'4': case '5': case '6': case '7': case '8':{
			column += (fenSplit[0][i] - '0' - 1);
			break;
		}

		case '/':
			rank--;
			column = -1; //riporto a -1 perch� all'inizio del ciclo la variabile column verr� ulteriormente incrementata, portando il suo valore a 0
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
		Engine::engineData.m_enPassantSquare = 64; //64 equivale a dire che non � possibile effettuare un en passant
		//std::cout << "Non e' possibile fare en passant: " << Engine::getEnPassantSquare() << std::endl;
	}
	else {
		Engine::engineData.m_enPassantSquare = (fenSplit[3][0] - 'a') + (fenSplit[3][1] - '1') * 8;
		//std::cout << "Casella bersaglio dell'en passant: " << Engine::getEnPassantSquare() << std::endl;
	}

	if (fenSplit.size() > 4) {
		Engine::engineData.m_halfMoveClock = std::stoi(fenSplit[4]);
		//std::cout << "Half move clock: " << Engine::getHalfMoveClock() << std::endl;

		Engine::engineData.m_fullMoveClock = std::stoi(fenSplit[5]);
		//std::cout << "Full move clock: " << Engine::getFullMoveClock() << std::endl;
	}
	else {
		Engine::engineData.m_halfMoveClock = 0;
		//std::cout << "Half move clock: " << Engine::getHalfMoveClock() << std::endl;

		Engine::engineData.m_fullMoveClock = 0;
		//std::cout << "Full move clock: " << Engine::getFullMoveClock() << std::endl;
	}
	

	(*m_previousPositionCharacteristics).push(previousPositionInfo);
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
	std::shared_ptr<uint64_t[]> bitBoards(new uint64_t[nBitboards]); //� uno smart pointer, per cui non � necessario chiamare delete per liberare la memoria usata

	for (int i = 0; i < nBitboards; i++) {
		bitBoards[i] = m_positionBitBoards[i];
	}

	return bitBoards;
}

uint64_t Board::allPiecesBitboard() {
	uint64_t out = 0;

	for (int i = 0; i < 2; i++) {
		out = out | m_positionBitBoards[i];
	}

	return out;
}

void Board::unmakeMove(const uint32_t& move) {
	int pieceColor, pieceType = -1;

	int startSquare = move & moveStartSquareBitmask;
	int endSquare = (move >> moveEndSquareOffset) & moveEndSquareBitMask;
	int promotionPiece = (move >> movePromotionPieceOffset) & movePromotionPieceBitMask;

	//recupera le informazioni relative alla posizione precedente e rimuovile dallo stack relativo
	uint32_t previousPositionCharacteristics = (*m_previousPositionCharacteristics).top();
	(*m_previousPositionCharacteristics).pop();

	//reimposto le caratteristiche irreversibili della posizione
	Engine::engineData.m_whiteCanCastleLong = (previousPositionCharacteristics >> whiteLongCastleRightsOffset) & whiteLongCastleRightsBitMask;
	Engine::engineData.m_whiteCanCastleShort = (previousPositionCharacteristics >> whiteShortCastleRightsOffset) & whiteShortCastleRightsBitMask;
	Engine::engineData.m_blackCanCastleLong = (previousPositionCharacteristics >> blackLongCastleRightsOffset) & blackLongCastleRightsBitMask;
	Engine::engineData.m_blackCanCastleShort = (previousPositionCharacteristics >> blackShortCastleRightsOffset) & blackShortCastleRightsBitMask;
	Engine::engineData.m_isWhite = (previousPositionCharacteristics >> isWhiteOffset) & isWhiteBitMask;

	Engine::engineData.m_enPassantSquare = (previousPositionCharacteristics >> enPassantTargetSquareOffset) & enPassantTargetSquareBitMask;
	Engine::engineData.m_halfMoveClock = (previousPositionCharacteristics >> halfMoveClockOffset) & halfMoveClockBitMask;
	Engine::engineData.m_fullMoveClock = (previousPositionCharacteristics >> fullMoveClockOffset) & fullMoveClockBitMask;

	//il colore del pezzo che si � mosso � uguale al colore del giocatore che doveva giocare nella mossa precedente
	if (Engine::engineData.m_isWhite) {
		pieceColor = nWhite;
	}
	else {
		pieceColor = nBlack;
	}

	//identifico il tipo di pezzo che si � mosso
	for (int i = 2; i < nBitboards; i++) {
		if ((m_positionBitBoards[i] >> endSquare) & 1) {
			pieceType = i;
		}
	}

	if (pieceType == -1) {
		BoardHelper::printBoard();
		uciHandler::printMove(move);
	}

	//tolgo il pezzo dalla casella in cui � stato messo
	m_positionBitBoards[pieceType] = m_positionBitBoards[pieceType] ^ ((uint64_t)1 << endSquare);
	m_positionBitBoards[pieceColor] = m_positionBitBoards[pieceColor] ^ ((uint64_t)1 << endSquare);

	//reinserisco un eventuale pezzo catturato durante la mossa precedente
	if (((previousPositionCharacteristics >> prevPieceOnEndSquareOffset) & prevPieceOnEndSquareBitMask) != 0) {
		m_positionBitBoards[(previousPositionCharacteristics >> prevPieceOnEndSquareOffset) & prevPieceOnEndSquareBitMask] = m_positionBitBoards[(previousPositionCharacteristics >> prevPieceOnEndSquareOffset) & prevPieceOnEndSquareBitMask] | ((uint64_t)1 << endSquare);
		m_positionBitBoards[(previousPositionCharacteristics >> colorOfPrevePieceOnEndSquareOffset) & prevPieceOnEndSquareBitMask] = m_positionBitBoards[(previousPositionCharacteristics >> colorOfPrevePieceOnEndSquareOffset) & prevPieceOnEndSquareBitMask] | ((uint64_t)1 << endSquare);
	}

	if (promotionPiece != 0) { //se la mossa precedente � stata una promozione, il pezzo che si � precedentemente mosso in realt� � un pedone
		pieceType = nPawns;
	}

	//rimetto il pezzo nella casella da cui � partito
	m_positionBitBoards[pieceType] = m_positionBitBoards[pieceType] | ((uint64_t)1 << startSquare);
	m_positionBitBoards[pieceColor] = m_positionBitBoards[pieceColor] | ((uint64_t)1 << startSquare);

	if (pieceType == nKings) {
		if (startSquare == 4 && endSquare == 2) { //ovvero se la mossa � un arrocco lungo per il bianco
			m_positionBitBoards[nRooks] = (m_positionBitBoards[nRooks] ^ ((uint64_t)1 << 3)) | 1; //tolgo la torre bianca dalla casella d1 e la rimetto nella casella a1
			m_positionBitBoards[pieceColor] = (m_positionBitBoards[pieceColor] ^ ((uint64_t)1 << 3)) | 1;
		}
		else if (startSquare == 4 && endSquare == 6) { //ovvero se la mossa � un arrocco corto per il bianco
			m_positionBitBoards[nRooks] = (m_positionBitBoards[nRooks] ^ ((uint64_t)1 << 5)) | ((uint64_t)1 << 7); //tolgo la torre bianca dalla casella f1 e la rimetto nella casella h1
			m_positionBitBoards[pieceColor] = (m_positionBitBoards[pieceColor] ^ ((uint64_t)1 << 5)) | ((uint64_t)1 << 7);
		}
		else if (startSquare == 60 && endSquare == 58) { //ovvero se la mossa � un arrocco lungo per il nero
			m_positionBitBoards[nRooks] = (m_positionBitBoards[nRooks] ^ ((uint64_t)1 << 59)) | ((uint64_t)1 << 56); //tolgo la torre bianca dalla casella d8 e la rimetto nella casella a8
			m_positionBitBoards[pieceColor] = (m_positionBitBoards[pieceColor] ^ ((uint64_t)1 << 59)) | ((uint64_t)1 << 56);
		}
		else if (startSquare == 60 && endSquare == 62) { //ovvero se la mossa � un arrocco corto per il nero
			m_positionBitBoards[nRooks] = (m_positionBitBoards[nRooks] ^ ((uint64_t)1 << 61)) | ((uint64_t)1 << 63); //tolgo la torre bianca dalla casella f8 e la rimetto nella casella h8
			m_positionBitBoards[pieceColor] = (m_positionBitBoards[pieceColor] ^ ((uint64_t)1 << 61)) | ((uint64_t)1 << 63);
		}
	}
	else if (endSquare == Engine::engineData.m_enPassantSquare && pieceType == nPawns) { //se la mossa precedente � stata un en passant, rimetto il pedone catturato nella sua casella
		if (pieceColor == nWhite) {
			m_positionBitBoards[nPawns] = m_positionBitBoards[nPawns] | ((uint64_t)1 << (endSquare - 8)); 
			m_positionBitBoards[!pieceColor] = m_positionBitBoards[!pieceColor] | ((uint64_t)1 << (endSquare - 8));
		}
		else {
			m_positionBitBoards[nPawns] = m_positionBitBoards[nPawns] | ((uint64_t)1 << (endSquare + 8));
			m_positionBitBoards[!pieceColor] = m_positionBitBoards[!pieceColor] | ((uint64_t)1 << (endSquare + 8));
		}
	}

	//BoardHelper::printBoard();
}

uint64_t Board::generateRookMoves(const int& startSquare, const uint64_t& blockerBitboard) {
	uint64_t moves = 0;
	int currSquare; //questa variabile viene utilizzata come indice per controllare le possibili caselle in cui la torre pu� muoversi

	//la torre pu� muoversi soltanto in linea retta orizzontalmente o verticalmente.
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

uint64_t Board::generateBishopMoves(const int& startSquare, const uint64_t& blockerBitboard) {
	uint64_t moves = 0;
	int currSquare; //questa variabile viene utilizzata come indice per controllare le possibili caselle in cui l'alfiere pu� muoversi

	if ((startSquare + 1) % 8 != 0) {
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
	if (!BoardHelper::isOnRightBorder(startSquare)) {
		moves = moves | ((uint64_t)1 << (startSquare + 1));

		if (startSquare > 7) {
			moves = moves | ((uint64_t)1 << (startSquare - 7));
		}
		if (startSquare < 56) {
			moves = moves | ((uint64_t)1 << (startSquare + 9));
		}
	}
	if (!BoardHelper::isOnLeftBorder(startSquare)) {
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

	if (!BoardHelper::isOnLeftBorder(startSquare)) {
		if (isWhite) {
			if ((blockerBitboard >> (startSquare + 7)) & 1 || startSquare + 7 == Engine::engineData.m_enPassantSquare) { //controllo se il pedone bianco pu� catturare andando verso sinistra
				moves = moves | ((uint64_t)1 << (startSquare + 7));
			}
		}
		else {
			if ((blockerBitboard >> (startSquare - 9)) & 1 || startSquare - 9 == Engine::engineData.m_enPassantSquare) { //controllo se il pedone nero pu� catturare andando verso sinistra
				moves = moves | ((uint64_t)1 << (startSquare - 9));
			}
		}
	}

	if (!BoardHelper::isOnRightBorder(startSquare)) {
		if (isWhite) {
			if (blockerBitboard >> (startSquare + 9) & 1 || startSquare + 9 == Engine::engineData.m_enPassantSquare) { //controllo se il pedone bianco pu� catturare verso destra
				moves = moves | ((uint64_t)1 << (startSquare + 9));
			}
		}
		else { //controllo se il pedone nero pu� catturare verso destra
			if (blockerBitboard >> (startSquare - 7) & 1 || startSquare - 7 == Engine::engineData.m_enPassantSquare) {
				moves = moves | ((uint64_t)1 << (startSquare - 7));
			}
		}
	}

	if (isWhite) {
		if (!((blockerBitboard >> (startSquare + 8)) & 1)) {
			moves = moves | ((uint64_t)1 << (startSquare + 8));

			if (startSquare >= 8 && startSquare <= 15 && !((blockerBitboard >> (startSquare + 16)) & 1)) { //controllo se il pedone bianco pu� essere spinto
				moves = moves | ((uint64_t)1 << (startSquare + 16));
			}
		}
	}
	else {
		if (!((blockerBitboard >> (startSquare - 8)) & 1)) {
			moves = moves | ((uint64_t)1 << (startSquare - 8));

			if (startSquare >= 48 && startSquare <= 55 && !((blockerBitboard >> (startSquare - 16)) & 1)) { //controllo se il pedone nero pu� essere spinto
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

	//il colore del pezzo che si � mosso � uguale al colore del giocatore che doveva giocare nella mossa precedente
	if (Engine::engineData.m_isWhite) {
		pieceColor = nWhite;
	}
	else {
		pieceColor = nBlack;
	}

	//identifico il tipo di pezzo che si � mosso
	for (int i = 2; i < nBitboards; i++) {
		if ((m_positionBitBoards[i] >> endSquare) & 1) {
			pieceType = i;
		}
	}

	//tolgo il pezzo dalla casella in cui � stato messo
	m_positionBitBoards[pieceType] = m_positionBitBoards[pieceType] ^ ((uint64_t)1 << endSquare);
	m_positionBitBoards[pieceColor] = m_positionBitBoards[pieceColor] ^ ((uint64_t)1 << endSquare);

	//reinserisco un eventuale pezzo catturato durante la mossa precedente
	if (((previousPositionInfo >> prevPieceOnEndSquareOffset) & prevPieceOnEndSquareBitMask) != 0) {
		m_positionBitBoards[(previousPositionInfo >> prevPieceOnEndSquareOffset) & prevPieceOnEndSquareBitMask] = m_positionBitBoards[(previousPositionInfo >> prevPieceOnEndSquareOffset) & prevPieceOnEndSquareBitMask] | ((uint64_t)1 << endSquare);
		m_positionBitBoards[(previousPositionInfo >> colorOfPrevePieceOnEndSquareOffset) & prevPieceOnEndSquareBitMask] = m_positionBitBoards[(previousPositionInfo >> colorOfPrevePieceOnEndSquareOffset) & prevPieceOnEndSquareBitMask] | ((uint64_t)1 << endSquare);
	}

	if (promotionPiece != -1) { //se la mossa precedente � stata una promozione, il pezzo che si � precedentemente mosso in realt� � un pedone
		pieceType = nPawns;
	}

	//rimetto il pezzo nella casella da cui � partito
	m_positionBitBoards[pieceType] = m_positionBitBoards[pieceType] | ((uint64_t)1 << startSquare);
	m_positionBitBoards[pieceColor] = m_positionBitBoards[pieceColor] | ((uint64_t)1 << startSquare);

	if (pieceType == nKings) {
		if (startSquare == 4 && endSquare == 2) { //ovvero se la mossa � un arrocco lungo per il bianco
			m_positionBitBoards[nRooks] = (m_positionBitBoards[nRooks] ^ ((uint64_t)1 << 3)) | 1; //tolgo la torre bianca dalla casella d1 e la rimetto nella casella a1
			m_positionBitBoards[pieceColor] = (m_positionBitBoards[pieceColor] ^ ((uint64_t)1 << 3)) | 1;
		}
		else if (startSquare == 4 && endSquare == 6) { //ovvero se la mossa � un arrocco corto per il bianco
			m_positionBitBoards[nRooks] = (m_positionBitBoards[nRooks] ^ ((uint64_t)1 << 5)) | ((uint64_t)1 << 7); //tolgo la torre bianca dalla casella f1 e la rimetto nella casella h1
			m_positionBitBoards[pieceColor] = (m_positionBitBoards[pieceColor] ^ ((uint64_t)1 << 5)) | ((uint64_t)1 << 7);
		}
		else if (startSquare == 60 && endSquare == 58) { //ovvero se la mossa � un arrocco lungo per il nero
			m_positionBitBoards[nRooks] = (m_positionBitBoards[nRooks] ^ ((uint64_t)1 << 59)) | ((uint64_t)1 << 56); //tolgo la torre bianca dalla casella d8 e la rimetto nella casella a8
			m_positionBitBoards[pieceColor] = (m_positionBitBoards[pieceColor] ^ ((uint64_t)1 << 59)) | ((uint64_t)1 << 56);
		}
		else if (startSquare == 60 && endSquare == 62) { //ovvero se la mossa � un arrocco corto per il nero
			m_positionBitBoards[nRooks] = (m_positionBitBoards[nRooks] ^ ((uint64_t)1 << 61)) | ((uint64_t)1 << 63); //tolgo la torre bianca dalla casella f8 e la rimetto nella casella h8
			m_positionBitBoards[pieceColor] = (m_positionBitBoards[pieceColor] ^ ((uint64_t)1 << 61)) | ((uint64_t)1 << 63);
		}
	}
	else if (endSquare == Engine::engineData.m_enPassantSquare && pieceType == nPawns) { //se la mossa precedente � stata un en passant, rimetto il pedone catturato nella sua casella
		if (pieceColor == nWhite) {
			m_positionBitBoards[nPawns] = m_positionBitBoards[nPawns] | ((uint64_t)1 << (endSquare - 8));
			m_positionBitBoards[!pieceColor] = m_positionBitBoards[!pieceColor] | ((uint64_t)1 << (endSquare - 8));
		}
		else {
			m_positionBitBoards[nPawns] = m_positionBitBoards[nPawns] | ((uint64_t)1 << (endSquare + 8));
			m_positionBitBoards[!pieceColor] = m_positionBitBoards[!pieceColor] | ((uint64_t)1 << (endSquare + 8));
		}
	}

	BoardHelper::printBoard();
}

uint64_t Board::getBitboard(int bitboardIndex) {
	return m_positionBitBoards[bitboardIndex];
}

void Board::resetPreviousPositionCharacteristics() {
	while (!(*m_previousPositionCharacteristics).empty()) {
		(*m_previousPositionCharacteristics).pop();
	}
}


bool Board::findInconsistency(Position prevPos, Position newPos) {
	if ((prevPos.bitboards[nWhite] | prevPos.bitboards[nBlack]) != (newPos.bitboards[nWhite] | newPos.bitboards[nBlack])) {
		return true;
	}
	else {
		return false;
	}
}

void Board::generaterookOccupancyBitmasks() {
	uint64_t bitmask;
	uint64_t bitmasks[64];
	for (int i = 0; i < 64; i++) {
		bitmask = 0;

		for (int j = i - 8; j > 7; j -= 8) { //spostamento verticale verso il basso
			bitmask = bitmask | ((uint64_t)1 << j);
		}

		for (int j = i + 8; j < 56; j += 8) { //spostamento verticale verso l'alto
			bitmask = bitmask | ((uint64_t)1 << j);
		}

		for (int j = i - 1; j % 8 != 0 && j > -1 && i % 8 != 0; j--) { //spostamento in orizzontale verso sinistra
			bitmask = bitmask | ((uint64_t)1 << j);
		}

		for (int j = i + 1; (j + 1) % 8 != 0 && j < 64 && (i + 1) % 8 != 0; j++) { //spostamento in orizzontale verso destra
			bitmask = bitmask | ((uint64_t)1 << j);
		}

		BoardHelper::printLegalMoves(bitmask);
		bitmasks[i] = bitmask;
	}

	for (int i = 0; i < 64; i++) {
		std::cout << bitmasks[i] << ", ";

		if (i != 0 && i % 8 == 0) {
			std::cout << std::endl;
		}
	}
}

void Board::generateRookMagicNumbers() {
	uint64_t magicNumbers[64] = { 0 }; //vettore in cui salvo i magic number
	int shiftAmounts[64]; //non scendo sotto 49 come numero di cui effettuare uno shift, in quanto gi� cos� la hashtable richiederebbe 2 MB in memoria, che � enorme
	std::vector<uint64_t> map;
	map.reserve(exp2(64 - 49)); //prealloco il vettore
	bool isOk;
	uint64_t newMagic;
	int constructiveCollisions; //contatore del numero di collisioni costruttive
	std::chrono::steady_clock clock; //timer
	auto startTime = clock.now();
	auto currTime = clock.now();
	std::chrono::milliseconds timeElapsed;

	std::fill(std::begin(shiftAmounts), std::end(shiftAmounts), 48); //inizializza tutti gli elementi del vettore shiftAmounts a 48, che alla prima iterazione saranno incrementati a 49

	std::random_device rd;
	std::mt19937_64 e2(rd());
	std::uniform_int_distribution<unsigned long long int> dist(0, UINT64_MAX); //roba per generazione di numeri casuali


	do {
		for (int i = 0; i < 64; i++) { //per ogni singolo quadrato della scacchiera
			shiftAmounts[i]++; //incremento di uno la quanit� di bit di cui shifto il mio indice
			auto partialTime = clock.now(); //

		start:
			currTime = clock.now();
			timeElapsed = std::chrono::duration_cast<std::chrono::milliseconds> (currTime - partialTime); //tempo passato dall'inizio della ricerca di un magic number per il quadrato corrente

			if (timeElapsed.count() >= 3000) { //se dopo 5 secondi non ho ancora trovato un magic number che vada bene con il numero di bit per cui shiftare corrente
				shiftAmounts[i]--; //ridiminuisco lo shift amount di 1
				continue; //passo al prossimo quadrato
			}


			constructiveCollisions = 0;
			map.clear(); //svuota il vettore
			isOk = true;

			for (int j = 0; j < exp2(64 - shiftAmounts[i]); j++) { //riempio ogni cella del vettore con un valore inutile
				map.push_back(UINT64_MAX);
			}

			newMagic = dist(e2); //prendo un magic number a caso

			int numPatterns = 0; //numero di possibili pattern di blocker rilevanti per il quadrato attuale
			int cardinality = 0; //variabile usata per il conteggio dei possibili pattern

			//il conteggio viene fatto come se si stesse contando in binario: ogni quadrato della bitboardMask corrispondente al quadrato preso in considerazione � visto come un "bit", che pu� essere "acceso" (contiene un pezzo) o "spento". Ecco che, dati n quadrati rilevanti, il numero di patter di blocker da considerare sar� pari a 2^n
			for (int j = 0; j < 64; j++) {
				if ((m_rookOccupancyBitmask[i] >> j) & 1) {
					cardinality++;
				}
			}

			numPatterns = exp2(cardinality);

			//ora, dopo aver trovato il numero di possibili pattern, � necessario generare la blockerBitboard relativa a ogni singolo pattern
			for (int j = 0; j < numPatterns; j++) {
				uint64_t blockerBitboard = 0; //blocker bitboard da popolare
				bool finished = false;
				int num = j;
				int bitValue = cardinality - 1;

				//popolo la blocker bitboard esattamente come se stessi convertendo manualmente il numero j da decimale a binario, considerando come unici bit validi quelli relativi ai quadrati rilevanti della bitboardMask

				for (int k = 63; k >= 0 && num > 0; k--) { //itero sull'intero numero
					if ((m_rookOccupancyBitmask[i] >> k) & 1) { //quando incontro un bit rilevante
						if (exp2(bitValue) <= num) { //se il valore corrispondente al bit trovato sta all'interno del numero del pattern che sto considerando
							blockerBitboard = blockerBitboard | ((uint64_t)1 << k); //accendo questo bit nella blockerBitboard
							num -= exp2(bitValue); //decremento il numero da convertire in binario di una quantit� pari al valore del bit appena acceso
						}
						bitValue--; //il valore del bit rilevante successivo sar� inferiore di un fattore 2 rispetto a quello corrente
					}
				}

				//BoardHelper::printLegalMoves(blockerBitboard);
				uint64_t index = (blockerBitboard * newMagic) >> shiftAmounts[i]; //indice a cui l'attack set corrispondente alla blocker bitboard generata viene mappato
				uint64_t attackSet = generateRookMoves(i, blockerBitboard); //mosse legali della torre associate con la blocker bitboard appena generata
				if (map[index] == UINT64_MAX || map[index] == attackSet) { //se la casella che dovrebbe corrispondere alla blocker bitboard generata � vuota o se l'attack set salvato in essa corrisponde con l'attack set appena generato, � tutto a posto
					if (map[index] != UINT64_MAX) {
						constructiveCollisions++;
					}
					else {
						map[index] = attackSet;
					}
					//BoardHelper::printLegalMoves(map[index]);
				}
				else { //altrimenti, vuol dire che il magic number non ha le propriet� desiderate, quindi provo con un altro magic number
					goto start;
				}
			}

			//se arrivo qua, ho trovato un magic number che rispetti le propriet� desiderate
			magicNumbers[i] = newMagic;
			std::cout << "Magic number trovato per la casella numero " << i << ": " << newMagic << std::endl;
			std::cout << "Numero di collisioni costruttive: " << constructiveCollisions << "\n\n";
		}


		currTime = clock.now();
		timeElapsed = std::chrono::duration_cast<std::chrono::milliseconds> (currTime - startTime);
	} while (timeElapsed.count() < 2400000);

	std::cout << "Magic numbers: ";
	for (int i = 0; i < 64; i++) {
		std::cout << magicNumbers[i] << ", ";
	}

	std::cout << "\n\nShift amounts: ";
	for (int i = 0; i < 64; i++) {
		std::cout << shiftAmounts[i] << ", ";
	}

	std::cout << "\n\nRicerca terminata\n";
	std::cin.get();
}

void Board::generateBishopOccupancyBitmasks() {
	uint64_t bitmask;
	uint64_t bitmasks[64];
	for (int i = 0; i < 64; i++) {
		bitmask = 0;

		for (int j = i - 9; j > 7 && j % 8 != 0; j -= 9) { //spostamento diagonale verso il basso a sinistra
			if ((j + 1) % 8 == 0) {
				break;
			}

			bitmask = bitmask | ((uint64_t)1 << j);
		}

		for (int j = i + 9; j < 56 && (j + 1) % 8 != 0; j += 9) { //spostamento diagonale verso l'alto a destra
			if (j % 8 == 0) {
				break;
			}

			bitmask = bitmask | ((uint64_t)1 << j);
		}

		for (int j = i - 7; j > 7 && (j + 1) % 8 != 0; j -= 7) { //spostamento diagonale verso il basso a destra
			if (j % 8 == 0) {
				break;
			}

			bitmask = bitmask | ((uint64_t)1 << j);
		}

		for (int j = i + 7; j < 56 && j % 8 != 0; j += 7) { //spostamento diagonale verso l'alto a sinistra
			if ((j + 1) % 8 == 0) {
				break;
			}

			bitmask = bitmask | ((uint64_t)1 << j);
		}

		BoardHelper::printLegalMoves(bitmask);
		bitmasks[i] = bitmask;
	}

	for (int i = 0; i < 64; i++) {
		std::cout << bitmasks[i] << ", ";

		if (i != 0 && i % 8 == 0) {
			std::cout << std::endl;
		}
	}
}

void Board::generateBishopMagicNumbers() {
	uint64_t magicNumbers[64] = { 0 }; //vettore in cui salvo i magic number
	int shiftAmounts[64]; //non scendo sotto 49 come numero di cui effettuare uno shift, in quanto gi� cos� la hashtable richiederebbe 2 MB in memoria, che � enorme
	std::vector<uint64_t> map;
	map.reserve(exp2(64 - 49)); //prealloco il vettore
	bool isOk;
	uint64_t newMagic;
	int constructiveCollisions; //contatore del numero di collisioni costruttive
	std::chrono::steady_clock clock; //timer
	auto startTime = clock.now();
	auto currTime = clock.now();
	std::chrono::milliseconds timeElapsed;

	std::fill(std::begin(shiftAmounts), std::end(shiftAmounts), 48); //inizializza tutti gli elementi del vettore shiftAmounts a 48, che alla prima iterazione saranno incrementati a 49

	std::random_device rd;
	std::mt19937_64 e2(rd());
	std::uniform_int_distribution<unsigned long long int> dist(0, UINT64_MAX); //roba per generazione di numeri casuali


	do {
		for (int i = 0; i < 64; i++) { //per ogni singolo quadrato della scacchiera
			shiftAmounts[i]++; //incremento di uno la quanit� di bit di cui shifto il mio indice
			auto partialTime = clock.now(); //

		start:
			currTime = clock.now();
			timeElapsed = std::chrono::duration_cast<std::chrono::milliseconds> (currTime - partialTime); //tempo passato dall'inizio della ricerca di un magic number per il quadrato corrente

			if (timeElapsed.count() >= 3000) { //se dopo 5 secondi non ho ancora trovato un magic number che vada bene con il numero di bit per cui shiftare corrente
				shiftAmounts[i]--; //ridiminuisco lo shift amount di 1
				continue; //passo al prossimo quadrato
			}


			constructiveCollisions = 0;
			map.clear(); //svuota il vettore
			isOk = true;

			for (int j = 0; j < exp2(64 - shiftAmounts[i]); j++) { //riempio ogni cella del vettore con un valore inutile
				map.push_back(UINT64_MAX);
			}

			newMagic = dist(e2); //prendo un magic number a caso

			int numPatterns = 0; //numero di possibili pattern di blocker rilevanti per il quadrato attuale
			int cardinality = 0; //variabile usata per il conteggio dei possibili pattern

			//il conteggio viene fatto come se si stesse contando in binario: ogni quadrato della bitboardMask corrispondente al quadrato preso in considerazione � visto come un "bit", che pu� essere "acceso" (contiene un pezzo) o "spento". Ecco che, dati n quadrati rilevanti, il numero di patter di blocker da considerare sar� pari a 2^n
			for (int j = 0; j < 64; j++) {
				if ((m_bishopOccupancyBitmask[i] >> j) & 1) {
					cardinality++;
				}
			}

			numPatterns = exp2(cardinality);

			//ora, dopo aver trovato il numero di possibili pattern, � necessario generare la blockerBitboard relativa a ogni singolo pattern
			for (int j = 0; j < numPatterns; j++) {
				uint64_t blockerBitboard = 0; //blocker bitboard da popolare
				bool finished = false;
				int num = j;
				int bitValue = cardinality - 1;

				//popolo la blocker bitboard esattamente come se stessi convertendo manualmente il numero j da decimale a binario, considerando come unici bit validi quelli relativi ai quadrati rilevanti della bitboardMask

				for (int k = 63; k >= 0 && num > 0; k--) { //itero sull'intero numero
					if ((m_bishopOccupancyBitmask[i] >> k) & 1) { //quando incontro un bit rilevante
						if (exp2(bitValue) <= num) { //se il valore corrispondente al bit trovato sta all'interno del numero del pattern che sto considerando
							blockerBitboard = blockerBitboard | ((uint64_t)1 << k); //accendo questo bit nella blockerBitboard
							num -= exp2(bitValue); //decremento il numero da convertire in binario di una quantit� pari al valore del bit appena acceso
						}
						bitValue--; //il valore del bit rilevante successivo sar� inferiore di un fattore 2 rispetto a quello corrente
					}
				}

				//BoardHelper::printLegalMoves(blockerBitboard);
				uint64_t index = (blockerBitboard * newMagic) >> shiftAmounts[i]; //indice a cui l'attack set corrispondente alla blocker bitboard generata viene mappato
				uint64_t attackSet = generateBishopMoves(i, blockerBitboard); //mosse legali della torre associate con la blocker bitboard appena generata
				if (map[index] == UINT64_MAX || map[index] == attackSet) { //se la casella che dovrebbe corrispondere alla blocker bitboard generata � vuota o se l'attack set salvato in essa corrisponde con l'attack set appena generato, � tutto a posto
					if (map[index] != UINT64_MAX) {
						constructiveCollisions++;
					}
					else {
						map[index] = attackSet;
					}
					//BoardHelper::printLegalMoves(map[index]);
				}
				else { //altrimenti, vuol dire che il magic number non ha le propriet� desiderate, quindi provo con un altro magic number
					goto start;
				}
			}

			//se arrivo qua, ho trovato un magic number che rispetti le propriet� desiderate
			magicNumbers[i] = newMagic;
			std::cout << "Magic number trovato per la casella numero " << i << ": " << newMagic << std::endl;
			std::cout << "Numero di collisioni costruttive: " << constructiveCollisions << "\n\n";
		}


		currTime = clock.now();
		timeElapsed = std::chrono::duration_cast<std::chrono::milliseconds> (currTime - startTime);
	} while (timeElapsed.count() < 2400000);

	std::cout << "Magic numbers: ";
	for (int i = 0; i < 64; i++) {
		std::cout << magicNumbers[i] << ", ";
	}

	std::cout << "\n\nShift amounts: ";
	for (int i = 0; i < 64; i++) {
		std::cout << shiftAmounts[i] << ", ";
	}

	std::cout << "\n\nRicerca terminata\n";
	std::cin.get();
}

void Board::initMagicBitboards() {
	uint64_t bufferSize = 0; //variabile utilizzata per memorizzare la dimensione del buffer che � necessario allocare in memoria per le magic bitboard
	uint64_t offset = 0; //variabile utilizzata per assegnare ad ogni magic bitboard il suo indirizzo corretto
	int numPatterns; //variabile utilizzata per determinare il numero di possibili pattern di blocker bitboard a partire da un dato quadrato
	int cardinality;
	uint64_t* prevBitboardPointer; //puntatore all'indirizzo di memoria della magic bitboard precedente

	//conteggio del numero di caselle necessarie per le magic bitboard delle torri
	for (int i = 0; i < 64; i++) {
		bufferSize += exp2(64 - m_rookShiftAmounts[i]);
	}

	//conteggio del numero di caselle necessarie per le magic bitboard degli alfieri
	for (int i = 0; i < 64; i++) {
		bufferSize += exp2(64 - m_bishopShiftAmounts[i]);
	}

	m_buffer = new uint64_t[bufferSize]; //allocazione sull'heap del buffer di memoria

	for (int i = 0; i < bufferSize; i++) {
		m_buffer[i] = UINT64_MAX;
	}

	prevBitboardPointer = m_buffer;

	//inizializzazione delle magic bitboard delle torri. Si tenga presente che ogni quadrato ha il suo array dedicato
	for (int i = 0; i < 64; i++) {
		m_rookMagicBitboards[i] = prevBitboardPointer + offset;

		//conteggio del numero di possibili pattern per il quadrato corrente
		cardinality = 0;
		
		for (int j = 0; j < 64; j++) {
			if ((m_rookOccupancyBitmask[i] >> j) & 1) {
				cardinality++;
			}
		}

		numPatterns = exp2(cardinality);

		for (int j = 0; j < numPatterns; j++) {
			uint64_t blockerBitboard = 0; //blocker bitboard da popolare
			int num = j;
			int bitValue = cardinality - 1;

			//popolo la blocker bitboard esattamente come se stessi convertendo manualmente il numero j da decimale a binario, considerando come unici bit validi quelli relativi ai quadrati rilevanti della bitboardMask

			for (int k = 63; k >= 0 && num > 0; k--) { //itero sull'intero numero
				if ((m_rookOccupancyBitmask[i] >> k) & 1) { //quando incontro un bit rilevante
					if (exp2(bitValue) <= num) { //se il valore corrispondente al bit trovato sta all'interno del numero del pattern che sto considerando
						blockerBitboard = blockerBitboard | ((uint64_t)1 << k); //accendo questo bit nella blockerBitboard
						num -= exp2(bitValue); //decremento il numero da convertire in binario di una quantit� pari al valore del bit appena acceso
					}
					bitValue--; //il valore del bit rilevante successivo sar� inferiore di un fattore 2 rispetto a quello corrente
				}
			}

			uint64_t index = (blockerBitboard * m_rookMagicNumbers[i]) >> m_rookShiftAmounts[i]; //indice a cui l'attack set corrispondente alla blocker bitboard generata viene mappato
			uint64_t attackSet = generateRookMoves(i, blockerBitboard); //mosse legali della torre associate con la blocker bitboard appena generata

			if (m_rookMagicBitboards[i][index] != UINT64_MAX && m_rookMagicBitboards[i][index] != attackSet) {
				std::cout << "Errore" << std::endl;
				std::cin.get();
			}

			m_rookMagicBitboards[i][index] = attackSet; //inserisci nella casella corretta della magic bitboard l'attack set calcolato
		}

		offset = exp2(64 - m_rookShiftAmounts[i]); //imposto l'offest di cui spostarmi per assegnare il puntatore alla bitboard successiva
		prevBitboardPointer = m_rookMagicBitboards[i]; //imposto il puntatore alla bitboard appena inizializzata
	}

	offset = exp2(64 - m_rookShiftAmounts[63]);
	prevBitboardPointer = m_rookMagicBitboards[63] + offset;

	offset = 0;

	//inizializzazione delle magic bitboard degli alfieri
	for (int i = 0; i < 64; i++) {
		m_bishopMagicBitboards[i] = prevBitboardPointer + offset;

		//conteggio del numero di possibili pattern per il quadrato corrente
		cardinality = 0;

		for (int j = 0; j < 64; j++) {
			if ((m_bishopOccupancyBitmask[i] >> j) & 1) {
				cardinality++;
			}
		}

		numPatterns = exp2(cardinality);

		for (int j = 0; j < numPatterns; j++) {
			uint64_t blockerBitboard = 0; //blocker bitboard da popolare
			int num = j;
			int bitValue = cardinality - 1;

			//popolo la blocker bitboard esattamente come se stessi convertendo manualmente il numero j da decimale a binario, considerando come unici bit validi quelli relativi ai quadrati rilevanti della bitboardMask

			for (int k = 63; k >= 0 && num > 0; k--) { //itero sull'intero numero
				if ((m_bishopOccupancyBitmask[i] >> k) & 1) { //quando incontro un bit rilevante
					if (exp2(bitValue) <= num) { //se il valore corrispondente al bit trovato sta all'interno del numero del pattern che sto considerando
						blockerBitboard = blockerBitboard | ((uint64_t)1 << k); //accendo questo bit nella blockerBitboard
						num -= exp2(bitValue); //decremento il numero da convertire in binario di una quantit� pari al valore del bit appena acceso
					}
					bitValue--; //il valore del bit rilevante successivo sar� inferiore di un fattore 2 rispetto a quello corrente
				}
			}

			uint64_t index = (blockerBitboard * m_bishopMagicNumbers[i]) >> m_bishopShiftAmounts[i]; //indice a cui l'attack set corrispondente alla blocker bitboard generata viene mappato
			uint64_t attackSet = generateBishopMoves(i, blockerBitboard); //mosse legali della torre associate con la blocker bitboard appena generata

			if (m_bishopMagicBitboards[i][index] != UINT64_MAX && m_bishopMagicBitboards[i][index] != attackSet) {
				std::cout << "Errore" << std::endl;
				std::cin.get();
			}

			m_bishopMagicBitboards[i][index] = attackSet; //inserisci nella casella corretta della magic bitboard l'attack set calcolato
		}

		offset = exp2(64 - m_bishopShiftAmounts[i]); //imposto l'offest di cui spostarmi per assegnare il puntatore alla bitboard successiva
		prevBitboardPointer = m_bishopMagicBitboards[i]; //imposto il puntatore alla bitboard appena inizializzata
	}
}

uint64_t Board::bishopMoves(const int& startSquare, const uint64_t& blockerBitboard) {
	uint64_t relevantBlockerBitboard = blockerBitboard & m_bishopOccupancyBitmask[startSquare];

	uint64_t index = (relevantBlockerBitboard * m_bishopMagicNumbers[startSquare]) >> m_bishopShiftAmounts[startSquare];

	//BoardHelper::printLegalMoves(m_bishopMagicBitboards[startSquare][index]);

	return m_bishopMagicBitboards[startSquare][index];
}

uint64_t Board::rookMoves(const int& startSquare, const uint64_t& blockerBitboard) {
	uint64_t relevantBlockerBitboard = blockerBitboard & m_rookOccupancyBitmask[startSquare];

	uint64_t index = (relevantBlockerBitboard * m_rookMagicNumbers[startSquare]) >> m_rookShiftAmounts[startSquare];

	//BoardHelper::printLegalMoves(m_rookMagicBitboards[startSquare][index]);

	return m_rookMagicBitboards[startSquare][index];
}