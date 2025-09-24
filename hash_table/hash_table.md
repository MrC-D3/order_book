# 2. Hash Table
 Array di dimensione M in cui una hash function mappa un valore al suo indice.
## 2.1 Collisioni
  Avviene quando più valori vengono mappati sullo stesso indice.
## 2.1 Gestione
   - Chaining: gli elementi che collidono finiscono nello stesso bucket (e.g. lista, binary tree, altro).
   - Open Addressing: si fa il probing, cioè se un elemento collide, si cerca la prossima cella libera.
### 2.1.1 Probing
    - Linear: 
    - Quadratic
    - Double
    - Robin Hood
## 2.2 Load Factor (α)
   α = N/M, dove N è il numero di elemento nella Hash Table.
   Il Chaining ha performance accettabili per α in [.75, 1] ma può gestire anche
   α > 1.
   L'Open Addressing degrada rapidamente per α > .5 ed è impossibile se α >= 1.
## 2.3 Resize
  Necessario quando α supera una certa soglia.
  - Dinamico: crea un nuovo array dove sposta i valori del vecchio array.
  - Incrementale: sposta i valori a poco a poco, mantenendo quindi 2 array per un po'.
  - Consistente: per sistemi distribuiti.
## 2.2 Concorrenza
### 2.2.1 Problemi
   - ABA: Valore cambia e torna al valore originale.
### 2.2.2 Locking
   - Fine-Grained: bucket-level, read-write, spinlock.
   - Lock-Free: CAS (Compare&Swap), ordering delle istruzioni, hazard pointers.
   - Hybrid: RCU (Read-Copy-Update), Optimistic (rollout se necessario).
### 2.2.3 Altro
   - Memory Model: Sequential Consistency, Acquire-Release, Relaxed Ordering.
   - Cache Model: False Sharing, Cache Line Alignment, NUMA Awareness.
   - Performance: SIMD Instructions.
   - Varianti: Cuckoo Hashing, Hopscotch Hashing, Dense Hash Map.
   - Precision: Rounding Modes IEEE 754, 