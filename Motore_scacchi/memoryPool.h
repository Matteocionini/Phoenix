#pragma once
#include <vector>
#include <memory>
#include <thread>
#include <mutex>
#include <stack>
#include <unordered_set>

/// <summary>
/// MemoryPool � un namespace in cui � contenuto un array di std::vector. Nel momento in cui � necessario utilizzare un vector, � sufficiente chiamare la funzione deputata a ci� di memoryPool, che restituir� un puntatore ad un std::vector della dimensione desiderata. Tutti i vector sono preallocati all'atto dell'avvio del programma, in modo da evitare il costo aggiunto di dover allocare e deallocare ripetutamente dai sull'heap
/// </summary>

typedef std::vector<uint16_t>* vectorPointer;

namespace MemoryPool {
	extern std::stack<vectorPointer> freeVectors; //stack contenente i vettori non ancora in uso
	extern std::unordered_set<vectorPointer> vectorsInUse; //set contenente il numero di vettori in uso
	extern std::mutex mtx; //mutex utilizzato per fare accesso alla pool di vector

	void initMemoryPool(); //funzione per inizializzare la memoryPool
	vectorPointer getVector(int size);  //funzione utilizzata per ottenere un vettore di una data dimensione
	void increaseVectorPoolSize(int amount); //funzione usata per aumentare la capacit� della vector pool
	void freeVector(vectorPointer vecPointer); //funzione usata quando ho terminato di usare un vettore per reinserirlo nello stack di vettori utilizzabili
}