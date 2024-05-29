#include "memoryPool.h"

std::stack<vectorPointer> MemoryPool::freeVectors;
std::unordered_set<vectorPointer> MemoryPool::vectorsInUse;

void MemoryPool::initMemoryPool() {
	for (int i = 0; i < 5; i++) { //inserisci nello stack dei vettori disponibili per l'uso 5 vettori
		vectorPointer vecPointer = new std::vector<uint16_t>;
		vecPointer->reserve(256); //imposta la capacità iniziale di ciascun vettore a 256
		freeVectors.push(vecPointer);
	}
}

vectorPointer MemoryPool::getVector(int size) {
	vectorPointer freeVector = freeVectors.top(); //prendi il primo vettore libero disponibile dallo stack freeVectors
	freeVectors.pop(); //rimuovi il vettore appena trovato dallo stack freeVectors
	vectorsInUse.insert(freeVector); //aggiungi il vettore all'elenco dei vettori attualmente in uso

	if (freeVectors.size() == 0) { //se non sono più disponibili vettori liberi
		increaseVectorPoolSize(vectorsInUse.size()); //raddoppia la dimensione della pool
	}

	return freeVector;
}

void MemoryPool::increaseVectorPoolSize(int amount) {
	for (int i = 0; i < amount; i++) { //inserisci nello stack dei vettori disponibili per l'uso la quantità desiderata di vettori
		vectorPointer vecPointer = new std::vector<uint16_t>;
		vecPointer->reserve(256); //imposta la capacità iniziale di ciascun vettore a 256
		freeVectors.push(vecPointer);
	}
}

void MemoryPool::freeVector(vectorPointer vecPointer) {
	vectorsInUse.erase(vecPointer); //rimuovi il vettore dall'elenco dei vettori in uso
	delete vecPointer;

	vectorPointer newVector = new std::vector<uint16_t>;
	newVector->reserve(256); //per sicurezza, reimposta la capacità del vettore a 256
	freeVectors.push(newVector); //aggiungi il vettore allo stack dei vettori pronti per l'uso
}