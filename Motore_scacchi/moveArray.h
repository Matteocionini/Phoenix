#pragma once
#include <stdint.h>
#include <memory>


class moveArray { //array allocato sullo stack che verr� utilizzato per memorizzare le mosse generate dalla funzione di generazione mosse
public:
	moveArray();

	void pushBack(const uint16_t& val); //metodo che consente di aggiungere un elemento alla fine di un array
	void Reset(); //metodo che consente di riiniziare la scrittura da capo, ignorando eventuali dati scritti in precedenza
	const int& getSize() const; //metodo che restituisce la dimensione reale del mio array
	uint16_t* Begin(); //metodo che restituisce un puntatore all'inizio dell'array
	uint16_t* End(); //metodo che restituisce l'equivalente di un iteratore al termine dell'array
	void Append(uint16_t* begin, uint16_t* end); //metodo che permette di aggiungere elementi in coda ad un vettore
	uint16_t& getElem(int index); //metodo che permette di ottenere un elemento del vettore


	//uint16_t& operator [](const int& index); //overload dell'operatore [], che consente di utilizzare moveArray come un array

private:
	int m_size; //intero che tiene traccia della dimensione reale di un array
	uint16_t m_array[256];
};