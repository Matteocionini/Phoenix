# Phoenix

Phoenix è un motore scacchistico, sviluppato interamente in C++, compatibile con tutte le GUI che supportano lo standard UCI (https://gist.github.com/DOBRO/2592c6dad754ba67e6dcaec8c90165bf).  
## Funzionalità implementate
Il progetto è ancora incompleto, per cui non è ancora possibile utilizzabile con una GUI. Attualmente, il motore è solamente in grado di eseguire perft (https://www.chessprogramming.org/Perft). La generazione delle mosse risulta essere corretta rispetto a 
tutte le più celebri posizioni di test. Lavorando su un singolo thread, perft lavora a circa 26 milioni di nodi al secondo. E' in piano un'implementazione multithreaded. 
Il motore ha una rappresentazione interna della scacchiera completa mediante bitboard (https://www.chessprogramming.org/Bitboards), ed è in grado, a partire da qualsiasi posizione, di generare tutte le mosse fino ad una profondità arbitraria. 
E' presente un primitivo algoritmo per la valutazione delle posizioni e per la scelta della mossa migliore, attualmente però ancora piuttosto inefficiente. Per la generazione degli attack pattern degli sliding pieces (regina, alfiere e torre), 
viene utilizzata la tecnica delle magic bitboard (https://www.chessprogramming.org/Magic_Bitboards). Sono stati implementati, inoltre, i meccanismi di make move ed unmake move.
